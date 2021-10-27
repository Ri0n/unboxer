/*
Copyright (c) 2021, Sergei Ilinykh <rion4ik@gmail.com>

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice, this
   list of conditions and the following disclaimer.
2. Redistributions in binary form must reproduce the above copyright notice,
   this list of conditions and the following disclaimer in the documentation
   and/or other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "inputfile_impl.h"
#include "inputhttp_impl.h"
#include "inputstreamer.h"
#include "status.h"
#include "unboxer.h"

#include "blobextractor.h"

#include <QCommandLineParser>
#include <QCoreApplication>
#include <QFile>
#include <QFileInfo>
#include <QImage>
#include <QMimeDatabase>
#include <QTimer>
#include <QUrl>
#include <QUuid>
#include <QXmlStreamReader>
#include <QtDebug>

#include <iostream>
#include <sstream>

#include <inttypes.h>

using namespace unboxer;
using FileUnboxer            = unboxer::Unboxer<InputFileImpl, NullCache>;
using HttpUnboxer            = unboxer::Unboxer<InputHttpImpl, NullCache>;
int            spaces        = 0;
BlobExtractor *blobExtractor = nullptr;
bool           verboseOutput = false;

void setupBox(Box::Ptr box)
{
    std::stringstream ss;
    auto              type = box->stringType();
    if (type.isEmpty()) {
        type = QString("artificial root");
    }
    ss << QString(spaces, ' ').toStdString() << type.toStdString() << " of size " << box->size << " with fileOffset "
       << box->fileOffset;
    std::cout << ss.str() << std::endl;
    box->onSubBoxOpen = setupBox;
    box->onClose      = [weakBox = std::weak_ptr<Box>(box)]() {
        spaces -= 2;
        blobExtractor->closeBox(weakBox.lock());
        return Status::Ok;
    };
    box->onDataRead = [weakBox = std::weak_ptr<Box>(box)](const QByteArray &data) mutable {
        if (verboseOutput) {
            std::stringstream ss;
            ss << QString(spaces + 2, ' ').toStdString() << data.data();
            std::cout << ss.str() << std::endl;
        }
        blobExtractor->addBoxData(weakBox.lock(), data);
        return Status::Ok;
    };
    blobExtractor->add(box);
    spaces += 2;
}

void extractImages([[maybe_unused]] Box::Ptr box, const QString &filename)
{
    QMimeDatabase db;
    auto          mimeType = db.mimeTypeForFile(filename);
    if (!mimeType.name().contains("xml") && !mimeType.name().contains("html")) {
        return;
    }
    qDebug() << "found xml in " << filename << ". extracting images";
    QFile file(filename);
    file.open(QIODevice::ReadOnly);
    QXmlStreamReader reader(&file);
    while (!reader.atEnd()) {
        if (reader.readNext() == QXmlStreamReader::StartElement && reader.name() == "image") {
            auto attrs     = reader.attributes();
            auto imagetype = attrs.value("imagetype").toString();
            if (imagetype.isEmpty() || attrs.value("encoding") != "Base64") {
                continue;
            }
            auto id       = attrs.value("xml:id").toString();
            auto dstFName = QString("%1.%2.%3")
                                .arg(QFileInfo(filename).fileName(), id, attrs.value("imagetype").toString().toLower());
            qDebug() << "found image: " << id << " saving to " << dstFName;
            auto text  = reader.readElementText();
            auto image = QImage::fromData(QByteArray::fromBase64(text.toLatin1()), imagetype.toLatin1().data());
            if (image.isNull()) {
                qWarning() << "failed to decode image: " << text;
            }
            image.save(dstFName);
        }
    }
}

template <class SpecificUnboxer>
std::unique_ptr<SpecificUnboxer> makeUnboxer(const QString &uri, std::size_t readSize, const QString registryTemplate)
{
    blobExtractor = new BlobExtractor(registryTemplate);
    blobExtractor->setParent(qApp);
    blobExtractor->setOnBoxClosedCallback(extractImages);

    std::unique_ptr<SpecificUnboxer> unboxer;
    unboxer = std::make_unique<SpecificUnboxer>(uri.toStdString());
    unboxer->setStreamOpenedCallback([unboxer = unboxer.get(), readSize](Box::Ptr rootBox) mutable {
        qDebug("stream opened");
        setupBox(rootBox);
        if (unboxer->stream().bytesAvailable()) {
            unboxer->read(readSize);
        }
    });
    unboxer->setStreamClosedCallback([](Status status) mutable {
        // let app start before exit
        QTimer::singleShot(0, QCoreApplication::instance(), [status]() {
            int ret = int(status);
            if (status == Status::Eof) {
                ret = 0;
            }
            QCoreApplication::exit(ret);
        });
    });
    unboxer->stream().setDataReadyCallback([unboxer = unboxer.get(), readSize]() mutable { unboxer->read(readSize); });
    unboxer->open();

    return unboxer;
}

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    QCoreApplication::setApplicationName("unboxer");
    QCoreApplication::setApplicationVersion("1.0");

    QCommandLineParser parser;
    parser.setApplicationDescription("Test helper");
    parser.addHelpOption();
    parser.addVersionOption();
    QCommandLineOption uriOption(QStringList() << "u"
                                               << "uri",
                                 "URI to open (local file if given w/o scheme)",
                                 "uri",
                                 "text0.mp4");
    QCommandLineOption verboseOption("verbose", "Enable verbose output (together with data blobs)");
    parser.addOption(uriOption);
    parser.addOption(verboseOption);
    parser.process(app);
    QString uri   = parser.value(uriOption);
    verboseOutput = parser.isSet(verboseOption);

    auto    url = QUrl::fromUserInput(uri, "", QUrl::AssumeLocalFile);
    QString registryTemplate;
    if (url.scheme().isEmpty() || url.scheme() == "file") {
        uri = url.toLocalFile();
        QFileInfo fi(uri);
        if (fi.isFile() && fi.isReadable()) {
            registryTemplate = fi.fileName() + ".%1.%2";
            qDebug() << "opening local file: " << uri;
            auto unboxer = makeUnboxer<FileUnboxer>(uri, 16384, registryTemplate);
            return app.exec();
        } else {
            qWarning() << "file " << uri << " is not readable";
        }
    } else {
        if (url.path().isEmpty()) {
            registryTemplate = url.host() + ".%1.%2";
        } else {
            registryTemplate = QFileInfo(url.path()).fileName() + ".%1.%2";
        }
        qDebug() << "opening http file: " << uri;
        auto unboxer = makeUnboxer<HttpUnboxer>(uri, 2048, registryTemplate);
        return app.exec();
    }
}
