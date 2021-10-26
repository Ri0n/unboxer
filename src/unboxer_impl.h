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
#include "boxreader.h"
#include "reason.h"
#include "unboxer_export.h"

#include <list>
#include <memory>
#include <variant>

namespace unboxer {

class UNBOXER_EXPORT UnboxerImpl {
public:
    using StreamOpenedCallback = std::function<void()>;
    using NewBoxCallback       = std::function<void(Box::Ptr)>;
    using StreamClosedCallback = std::function<void(Reason)>;

    UnboxerImpl(StreamOpenedCallback &&streamOpenedCallback, StreamClosedCallback &&streamClosedCallback) :
        streamOpenedCallback(std::move(streamOpenedCallback)), streamClosedCallback(std::move(streamClosedCallback))
    {
    }

    Box::Ptr rootBox() const { return readers.empty() ? Box::Ptr {} : readers.front().box; }

    // streaming
    void   onStreamOpened();
    void   onStreamClosed(Reason reason);
    Reason onDataRead(const QByteArray &data);

private:
    // unboxing
    void onBoxOpened(const QByteArray &type, std::uint64_t size);
    void onBoxClosed();

    StreamOpenedCallback streamOpenedCallback;
    StreamClosedCallback streamClosedCallback;

    struct BoxItem {
        inline BoxItem(BoxReader &&reader, Box::Ptr &&box) : reader(std::move(reader)), box(std::move(box)) { }

        BoxReader reader;
        Box::Ptr  box;
    };

    std::list<BoxItem> readers;
};

} // namespace unboxer
