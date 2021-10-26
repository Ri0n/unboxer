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

#include <QByteArray>

#include <functional>
#include <memory>

namespace unboxer {

class BoxReaderImpl;

class BoxReader {
public:
    using BoxOpenedCallback = std::function<void(const QByteArray &, std::uint64_t)>;
    using BoxClosedCallback = std::function<void()>;
    using DataReadCallback  = std::function<Reason(const QByteArray &)>;

    BoxReader(BoxOpenedCallback &&boxOpened, BoxClosedCallback &&boxClosed, DataReadCallback &&dataRead);
    ~BoxReader();

    BoxReader(const BoxReader &) = delete;
    BoxReader(BoxReader &&other);

    /**
     * @brief feed fresh input data either from input stream or from another box
     * @param inputData
     * @return In most cases Reason::Ok
     */
    Reason feed(const QByteArray &inputData);

    /**
     * @brief close the box by some reason (stream eof for example).
     * @param reason of closing
     * @return another reason if the box disagree with the close
     */
    Reason close(Reason reason);

private:
    std::unique_ptr<BoxReaderImpl> impl;
};

}
