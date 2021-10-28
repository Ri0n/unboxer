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

UnboxerImpl::UnboxerImpl(std::vector<QByteArray> &&containerTypes) :
    containerTypes(std::move(containerTypes)), reader {
        std::bind(&UnboxerImpl::onBoxOpened, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3),
        std::bind(&UnboxerImpl::onBoxClosed, this),
        std::bind(&UnboxerImpl::onDataRead, this, std::placeholders::_1)
    }
{
}

void UnboxerImpl::onStreamOpened()
{
    boxes.emplace_back(std::make_shared<Box>(true));
    if (streamOpenedCallback) {
        streamOpenedCallback(boxes.back());
    }
}

Status UnboxerImpl::onStreamDataRead(const QByteArray &data) { return reader.feed(data); }

void UnboxerImpl::onStreamClosed(Status reason)
{
    reason = reader.close(reason);
    if (reason == Status::Eof) {
        while (!boxes.empty()) {
            // check for incomplete boxes like one having explicit size but still requiring more data to close
            auto box = boxes.back();

            // if box close wasn't handled by reader we need to close it explicitly
            // and let the the library's client to decide how valid it is
            box->isClosed_ = true;
            if (box->onClose) {
                auto status = box->onClose();
                if (status != Status::Ok) {
                    reason = status;
                    break;
                }
            }
            boxes.pop_back();
        }
    }
    boxes.clear();
    if (streamClosedCallback) {
        streamClosedCallback(reason);
    }
}

bool UnboxerImpl::onBoxOpened(const QByteArray &type, std::uint64_t size, uint64_t fileOffset)
{
    bool isContainer = std::find(containerTypes.begin(), containerTypes.end(), type) != containerTypes.end();
    auto parentBox   = boxes.back();
    auto box         = boxes.emplace_back(std::make_shared<Box>(isContainer, type, size, fileOffset));
    if (parentBox->onSubBoxOpen) {
        parentBox->onSubBoxOpen(box);
    }
    return box->isContainer;
}

Status UnboxerImpl::onDataRead(const QByteArray &data)
{
    auto box = boxes.back();
    box->dataFed_ += data.size();
    if (box->onDataRead) {
        return box->onDataRead(data);
    }
    return Status::Ok;
}

void UnboxerImpl::onBoxClosed()
{
    auto box       = boxes.back();
    box->isClosed_ = true;
    if (box->onClose) {
        box->onClose();
    }
    boxes.pop_back();
}

} // namespace unboxer
