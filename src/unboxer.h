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

#include "box_reader.h"
#include "error.h"
#include "unboxer_export.h"

#include <QObject>

#include <variant>

namespace unboxer {

template <class Source> class Unboxer {
public:
    struct Box {
        int           type;
        std::uint64_t size;    // full size
        QByteArray    payload; // data of full size or less if incomplete
    };

    struct ProgressingBox {
        Box                                     box;
        std::function<void(const QByteArray &)> onDataRead;
    };

    using Data                 = std::variant<Box, std::shared_ptr<ProgressingBox>>;
    using Reader               = BoxReader<Source>;
    using StreamOpenedCallback = std::function<void()>;
    using DataReadCallback     = std::function<void(Data)>;
    using StreamClosedCallback = std::function<void(Reason)>;

    template <typename OpenedCB, typename DataReadCB, typename ClosedCB>
    Unboxer(const std::string &uri,
            OpenedCB         &&streamOpenedCallback,
            DataReadCB       &&dataReadCallback,
            ClosedCB         &&streamClosedCallback) :
        boxStream(uri,
                  std::bind(&Unboxer::onStreamOpened, this),
                  std::bind(&Unboxer::onBoxOpened, this, std::placeholders::_1, std::placeholders::_2),
                  std::bind(&Unboxer::onDataRead, this, std::placeholders::_1),
                  std::bind(&Unboxer::onStreamClosed, this, std::placeholders::_1)),
        streamOpenedCallback(streamOpenedCallback), dataReadCallback(dataReadCallback),
        streamClosedCallback(streamClosedCallback)
    {
    }
    void open() { boxStream.open(); }
    void read(std::size_t size) { boxStream.read(size); }

private:
    void onStreamOpened() { streamOpenedCallback(); }
    void onBoxOpened(int type, std::uint64_t) { }
    void onDataRead(const QByteArray &data) { }
    void onStreamClosed(Reason reason) { streamClosedCallback(reason); }

private:
    Reader                          boxStream;
    std::shared_ptr<ProgressingBox> progressingBox; // if any currently

    StreamOpenedCallback streamOpenedCallback;
    DataReadCallback     dataReadCallback;
    StreamClosedCallback streamClosedCallback;
};

}

UNBOXER_EXPORT void dummy_lib_fun();
