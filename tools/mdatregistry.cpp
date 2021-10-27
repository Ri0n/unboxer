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

#include "mdatregistry.h"

#include "status.h"

#include <QDebug>
#include <QPointer>
#include <QUuid>

using namespace unboxer;

MdatRegistry::MdatRegistry(const QString &fnameTemplate, const QList<QByteArray> &boxTypes) :
    fnameTemplate(fnameTemplate), boxTypes(boxTypes)
{
    Q_ASSERT(!fnameTemplate.isEmpty());
    if (boxTypes.isEmpty()) {
        this->boxTypes << "mdat";
    }
}

MdatRegistry::~MdatRegistry()
{
    for (auto const &[_, file] : files) {
        if (file->isOpen()) {
            file->close();
        }
        delete file;
    }
}

void MdatRegistry::add(Box::Ptr box)
{
    if (!boxTypes.contains(box->type) || box->type.isEmpty()) {
        return;
    }

    auto it = files.find(box);
    Q_ASSERT(it == files.end());

    auto file = new QFile(fnameTemplate.arg(box->stringType(), QString::number(fileIndex++)));
    bool inserted;
    std::tie(it, inserted) = files.insert(std::make_pair(box, file));
    if (!file->open(QIODevice::WriteOnly)) {
        qWarning() << "Failed to open " << file->fileName() << " for writing";
        delete file;
        files.erase(it);
    }
}

void MdatRegistry::addBoxData(unboxer::Box::Ptr box, const QByteArray &data)
{
    auto it = files.find(box);
    if (it == files.end()) {
        return;
    }
    auto file = it->second;
    if (file->write(data) != data.size()) {
        qWarning() << "Failed to write to " << file->fileName();
        file->close();
        delete file;
        files.erase(it);
    }
}

void MdatRegistry::closeBox(unboxer::Box::Ptr box)
{
    auto it = files.find(box);
    if (it == files.end()) {
        return;
    }
    it->second->close();
    if (boxClosedCallback) {
        boxClosedCallback(box, it->second->fileName());
    }
}
