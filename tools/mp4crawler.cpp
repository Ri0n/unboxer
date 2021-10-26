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

#include <QCommandLineParser>
#include <QCoreApplication>
#include <QFile>
#include <QTimer>
#include <QUrl>
#include <QUuid>
#include <QtDebug>

#include <iostream>
#include <sstream>

#include <inttypes.h>

using namespace unboxer;
using FileUnboxer = unboxer::Unboxer<InputFileImpl, NullCache>;
using HttpUnboxer = unboxer::Unboxer<InputHttpImpl, NullCache>;
int spaces        = 0;

void setupBox(Box::Ptr box)
{
    std::stringstream ss;
    auto              type = box->type.isEmpty() ? QString("artificial root")
                     : box->type.size() > 4      ? QUuid::fromRfc4122(box->type).toString()
                                                 : QString::fromLatin1(box->type);
    ss << QString(spaces, ' ').toStdString() << type.toStdString() << " of size " << box->size << " with fileOffset "
       << box->fileOffset;
    std::cout << ss.str() << std::endl;
    box->onSubBoxOpen = setupBox;
    box->onClose      = []() {
        spaces -= 2;
        return Status::Ok;
    };
    box->onDataRead = [&](const QByteArray &) mutable { return Status::Ok; };
    spaces += 2;
}

template <class SpecificUnboxer> std::unique_ptr<SpecificUnboxer> makeUnboxer(const QString &uri, std::size_t readSize)
{
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
        QTimer::singleShot(0, QCoreApplication::instance(), [status]() { QCoreApplication::exit(int(status)); });
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
    parser.addOption(uriOption);
    parser.process(app);
    QString uri = parser.value(uriOption);

    auto url = QUrl::fromUserInput(uri, "", QUrl::AssumeLocalFile);
    if (url.scheme().isEmpty() || url.scheme() == "file") {
        uri          = url.toLocalFile();
        auto unboxer = makeUnboxer<FileUnboxer>(uri, 16384);
        return app.exec();
    } else {
        auto unboxer = makeUnboxer<HttpUnboxer>(uri, 2048);
        return app.exec();
    }
}
