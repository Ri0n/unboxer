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

#include "unboxer_impl.h"

#include <QVector>

namespace unboxer {

void UnboxerImpl::onStreamOpened()
{
    readers.emplace_back(
        BoxReader { std::bind(&UnboxerImpl::onBoxOpened, this, std::placeholders::_1, std::placeholders::_2),
                    std::bind(&UnboxerImpl::onBoxClosed, this),
                    std::bind(&UnboxerImpl::onDataRead, this, std::placeholders::_1) },
        std::make_shared<Box>(true));
    streamOpenedCallback();
}

void UnboxerImpl::onStreamClosed(Reason reason) { streamClosedCallback(reason); }

void UnboxerImpl::onBoxOpened(const QByteArray &type, std::uint64_t size)
{
    static QVector<QByteArray> containerBoxes { { "moof", "traf" } };

    auto parentBox = readers.back().box;
    readers.emplace_back(
        BoxReader(std::bind(&UnboxerImpl::onBoxOpened, this, std::placeholders::_1, std::placeholders::_2),
                  std::bind(&UnboxerImpl::onBoxClosed, this),
                  std::bind(&UnboxerImpl::onDataRead, this, std::placeholders::_1)),
        std::make_shared<Box>(containerBoxes.contains(type), type, size));
    if (parentBox->onSubBoxOpen) {
        parentBox->onSubBoxOpen(readers.back().box);
    }
}

Reason UnboxerImpl::onDataRead(const QByteArray &data)
{
    if (readers.back().box->isContainer) {
        return readers.back().reader.feed(data);
    } else {
        auto box = readers.back().box;
        if (box && box->onDataRead) {
            return box->onDataRead(data);
        }
    }
    return Reason::Ok;
}

void UnboxerImpl::onBoxClosed()
{
    if (readers.back().box->onClose) {
        readers.back().box->onClose();
    }
    readers.pop_back();
}

} // namespace unboxer
