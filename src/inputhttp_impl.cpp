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

#include "inputhttp_impl.h"

#include <QUrl>

namespace unboxer {

void InputHttpImpl::open()
{
    reply.reset(nam.get(QNetworkRequest(QUrl(QString::fromStdString(url)))));
    connect(reply.get(), &QNetworkReply::metaDataChanged, this, [&]() { openedCallback(); });
    connect(reply.get(), &QNetworkReply::readyRead, this, &InputHttpImpl::tryRead);
    connect(reply.get(), &QNetworkReply::finished, this, [this]() { tryReportClose(); });
}

void InputHttpImpl::read(std::size_t size)
{
    needToRead += size;
    tryRead();
}

void InputHttpImpl::reset() { reply.reset(); }

void InputHttpImpl::tryRead()
{
    if (reply && reply->bytesAvailable()) {
        auto dataSz = qMin(needToRead, reply->bytesAvailable());
        if (dataSz) {
            auto data = reply->read(dataSz);
            needToRead -= data.size();
            dataReadCallback(data);
        }
        if (!reply->bytesAvailable() && !reply->isRunning()) {

            tryReportClose();
        } else if (reply->bytesAvailable()) {
            dataReadyCallback();
        }
    }
}

void InputHttpImpl::tryReportClose()
{
    if (closed) {
        return;
    }
    if (reply && !reply->bytesAvailable() && reply->isFinished()) {
        auto status = Status::Ok;
        switch (reply->error()) {
        case QNetworkReply::NoError:
            status = Status::Eof;
            break;
        case QNetworkReply::TimeoutError:
            status = Status::Timeout;
            break;
        default:
            status = Status::Corrupted;
            break;
        }
        closedCallback(status);
        closed = true;
        reply.release()->deleteLater();
    }
}

} // namespace unboxer
