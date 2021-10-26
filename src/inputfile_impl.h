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

#include "status.h"
#include "unboxer_export.h"

#include <QFile>

#include <functional>
#include <memory>
#include <string>

namespace unboxer {

class UNBOXER_EXPORT InputFileImpl : public QObject {
    Q_OBJECT
public:
    std::string                             fileName;
    std::function<void()>                   openedCallback;
    std::function<void()>                   dataReadyCallback;
    std::function<void(const QByteArray &)> dataReadCallback;
    std::function<void(Status)>             closedCallback;

    QFile  file;
    qint64 needToRead = 0;

public:
    template <typename OpenedCB, typename DataReadyCB, typename DataReadCB, typename ClosedCB>
    InputFileImpl(const std::string &fileName,
                  OpenedCB         &&openedCallback,
                  DataReadyCB      &&dataReadyCallback,
                  DataReadCB       &&dataReadCallback,
                  ClosedCB         &&closedCallback) :
        fileName(fileName),
        openedCallback(std::move(openedCallback)), dataReadyCallback(std::move(dataReadyCallback)),
        dataReadCallback(std::move(dataReadCallback)), closedCallback(std::move(closedCallback))
    {
    }
    void        open();
    void        read(std::size_t size);
    void        reset();
    std::size_t bytesAvailable() const { return file.isOpen() ? file.size() - file.pos() : 0; }

private:
    void tryRead();
};

} // namespace unboxer
