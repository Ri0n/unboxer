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

#include "reason.h"
#include "unboxer_export.h"

#include <QByteArray>

#include <functional>

namespace unboxer {

class UNBOXER_EXPORT InputMemoryImpl {
    std::function<void()>                   openedCallback;
    std::function<void(const QByteArray &)> dataReadCallback;
    std::function<void(unboxer::Reason)>    closedCallback;

    std::size_t offset = 0;
    QByteArray  data;

public:
    template <typename OpenedCB, typename DataReadCB, typename ClosedCB>
    InputMemoryImpl(const std::string &base64data,
                    OpenedCB         &&openedCallback,
                    DataReadCB       &&dataReadCallback,
                    ClosedCB         &&closedCallback) :
        openedCallback(std::move(openedCallback)),
        dataReadCallback(std::move(dataReadCallback)), closedCallback(std::move(closedCallback)),
        data(QByteArray::fromBase64(QByteArray::fromRawData(base64data.data(), base64data.size())))
    {
    }
    void open() { openedCallback(); }
    void read(std::size_t size);
    void reset()
    {
        data.clear();
        offset = 0;
    }
};

} // namespace unboxer
