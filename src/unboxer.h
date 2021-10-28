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

#include "boxreader.h"
#include "inputstreamer.h"
#include "status.h"
#include "unboxer_export.h"
#include "unboxer_impl.h"

#include <QObject>

#include <variant>

namespace unboxer {

template <class Source, class Cache> class Unboxer {
public:
    using StreamType = InputStreamer<Source, Cache>;

    Unboxer(const std::string &uri, std::vector<QByteArray> &&containerTypes = { { "moof", "traf" } }) :
        impl(std::make_unique<UnboxerImpl>(std::move(containerTypes))),
        stream_(uri,
                std::bind(&UnboxerImpl::onStreamOpened, impl.get()),
                std::bind(&UnboxerImpl::onStreamDataRead, impl.get(), std::placeholders::_1),
                std::bind(&UnboxerImpl::onStreamClosed, impl.get(), std::placeholders::_1))

    {
    }

    Unboxer(Unboxer &&other)      = delete;
    Unboxer(const Unboxer &other) = delete;

    void setStreamOpenedCallback(UnboxerImpl::StreamOpenedCallback &&callback)
    {
        impl->streamOpenedCallback = std::move(callback);
    }
    void setStreamClosedCallback(UnboxerImpl::StreamClosedCallback &&callback)
    {
        impl->streamClosedCallback = std::move(callback);
    }

    Box::Ptr    rootBox() const { return impl->rootBox(); }
    void        open() { stream_.open(); }
    void        read(std::size_t size) { stream_.read(size); }
    StreamType &stream() { return stream_; }

private:
private:
    std::unique_ptr<UnboxerImpl> impl;
    StreamType                   stream_;
};

}
