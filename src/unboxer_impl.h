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
#include "reason.h"
#include "unboxer_export.h"

#include <memory>
#include <variant>

namespace unboxer {

class UNBOXER_EXPORT UnboxerImpl {
public:
    using StreamOpenedCallback = std::function<void()>;
    using DataReadCallback     = std::function<void(AnyBox)>;
    using StreamClosedCallback = std::function<void(Reason)>;

    UnboxerImpl(StreamOpenedCallback &&streamOpenedCallback,
                DataReadCallback     &&dataReadCallback,
                StreamClosedCallback &&streamClosedCallback) :
        streamOpenedCallback(std::move(streamOpenedCallback)),
        dataReadCallback(std::move(dataReadCallback)), streamClosedCallback(std::move(streamClosedCallback))
    {
    }

    // streaming
    void   onStreamOpened() { streamOpenedCallback(); }
    Reason onDataRead(const QByteArray &data);
    void   onStreamClosed(Reason reason) { streamClosedCallback(reason); }

    // unboxing
    void onBoxOpened(const QByteArray &type, std::uint64_t size);
    void onBoxClosed() { }

    std::shared_ptr<ProgressingBox> progressingBox; // if any currently

    StreamOpenedCallback streamOpenedCallback;
    DataReadCallback     dataReadCallback;
    StreamClosedCallback streamClosedCallback;
};

} // namespace unboxer
