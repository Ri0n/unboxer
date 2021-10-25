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

#include <optional>

namespace unboxer {

class UNBOXER_EXPORT BoxReaderImpl {
public:
    using StreamOpenedCallback = std::function<void()>;
    using BoxOpenedCallback    = std::function<void(const QByteArray &, std::uint64_t)>;
    using BoxClosedCallback    = std::function<void()>;
    using DataReadCallback     = std::function<Reason(const QByteArray &)>;
    using StreamClosedCallback = std::function<void(Reason)>;

    template <typename StreamOpenedCB,
              typename BoxOpenedCB,
              typename BoxClosedCB,
              typename DataReadCB,
              typename StreamClosedCB>
    BoxReaderImpl(StreamOpenedCB &&streamOpened,
                  BoxOpenedCB    &&boxOpened,
                  BoxClosedCB    &&boxClosed,
                  DataReadCB     &&dataRead,
                  StreamClosedCB &&streamClosed) :
        openedCallback(streamOpened),
        boxOpenedCallback(boxOpened), boxClosedCallback(boxClosed), dataReadCallback(dataRead),
        streamClosedCallback(streamClosed)
    {
    }

    void   onStreamOpened() { openedCallback(); } // TODO if really nothing todo then pass cb directly to input stream
    Reason onStreamDataRead(const QByteArray &data);
    void   onStreamClosed(Reason reason);

private:
    Reason sendData();

private:
    std::optional<std::uint64_t> fullBoxSize; // if not set -> not enough data to parse size

    QByteArray    incompleteBox;           // a part of payload. could be somewhere in the middle of a box
    int           parsingOffset       = 0; // from the start of incompleteBox
    std::uint64_t boxPayloadBytesLeft = 0;

    StreamOpenedCallback openedCallback;
    BoxOpenedCallback    boxOpenedCallback;
    BoxClosedCallback    boxClosedCallback;
    DataReadCallback     dataReadCallback;
    StreamClosedCallback streamClosedCallback;
};

} // namespace unboxer
