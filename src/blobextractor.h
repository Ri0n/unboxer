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

#pragma once

#include "box.h"
#include "unboxer_export.h"

#include <QByteArray>
#include <QFile>
#include <QHash>
#include <QList>
#include <QObject>

namespace unboxer {

class UNBOXER_EXPORT BlobExtractor : public QObject {
    Q_OBJECT
public:
    using BoxClosedCallback = std::function<void(unboxer::Box::Ptr, const QString &filename)>;

    BlobExtractor(const QString &fnameTemplate, const QList<QByteArray> &boxTypes = {});
    ~BlobExtractor();

    void add(unboxer::Box::Ptr box);
    void addBoxData(unboxer::Box::Ptr box, const QByteArray &data);
    void closeBox(unboxer::Box::Ptr box);

    inline void setOnBoxClosedCallback(BoxClosedCallback callback) { boxClosedCallback = callback; }

private:
    QString                                        fnameTemplate;
    QList<QByteArray>                              boxTypes;
    std::unordered_map<unboxer::Box::Ptr, QFile *> files;
    int                                            fileIndex = 1;
    BoxClosedCallback                              boxClosedCallback;
};

}
