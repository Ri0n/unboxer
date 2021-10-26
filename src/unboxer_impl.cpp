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

void UnboxerImpl::onStreamClosed(Status reason)
{
    if (reason == Status::Eof) {
        while (!readers.empty()) {
            // check for incomplete boxes like one having explicit size but still requiring more data to close
            auto box    = readers.back().box;
            auto status = readers.back().reader.close(reason);
            if (status != Status::Eof) { // only Eof or error statuses are expected
                reason = status;
                break;
            }
            if (box->isClosed_) {
                continue; // we handled closed. So readers.back() has changed (see handler below)
            }

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
            readers.pop_back();
        }
    }
    readers.clear();
    streamClosedCallback(reason);
}

void UnboxerImpl::onBoxOpened(const QByteArray &type, std::uint64_t size)
{
    static QVector<QByteArray> containerBoxes { { "moof", "traf" } };

    auto parentBox = readers.back().box;
    // we always create reader to unbox payload. but set flag isContainer only
    // for known containers. The library's client may change this flag in onSubBoxOpen
    // or just ignore it. So if flag is set there won't be onDataRead events from the box,
    // or otherwise (if not set) there won't be onSubBoxOpen
    readers.emplace_back(
        BoxReader { std::bind(&UnboxerImpl::onBoxOpened, this, std::placeholders::_1, std::placeholders::_2),
                    std::bind(&UnboxerImpl::onBoxClosed, this),
                    std::bind(&UnboxerImpl::onDataRead, this, std::placeholders::_1) },
        std::make_shared<Box>(containerBoxes.contains(type), type, size));
    if (parentBox->onSubBoxOpen) {
        parentBox->onSubBoxOpen(readers.back().box);
    }
}

Status UnboxerImpl::onDataRead(const QByteArray &data)
{
    QByteArray remainder = data;
    auto       status    = Status::Ok;
    while (!remainder.isEmpty()) {
        auto          box      = readers.back().box;
        std::uint64_t toFeedSz = remainder.size();
        if (box->size) {
            toFeedSz = qMin(*box->size - box->dataFed_, toFeedSz);
        }
        box->dataFed_ += toFeedSz;
        if (box->isContainer) { // need to parse boxes insize payload
            status = readers.back().reader.feed(remainder.left(toFeedSz));
            if (status != Status::Ok) {
                break;
            }
        } else { // if not then just send data to the client
            if (box->type == "mdat") {
                qDebug("got data from mdat");
            }
            if (box->onDataRead) {
                status = box->onDataRead(remainder.left(toFeedSz));
                if (status != Status::Ok) {
                    break;
                }
            }
        }
        remainder.remove(0, toFeedSz);
    }
    return status;
}

void UnboxerImpl::onBoxClosed()
{
    auto box       = readers.back().box;
    box->isClosed_ = true;
    if (box->onClose) {
        box->onClose();
    }
    readers.pop_back();
}

} // namespace unboxer
