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

#include "boxreader_impl.h"

#include <functional>
#include <memory>

namespace unboxer {

template <class InputStream> class BoxReader {
public:
    template <class... Args>
    BoxReader(const std::string &inputUri, Args &&...args) :
        impl(std::make_unique<BoxReaderImpl>(std::forward<Args>(args)...)),
        inputStream(inputUri,
                    std::bind(&BoxReaderImpl::onStreamOpened, impl.get()),
                    std::bind(&BoxReaderImpl::onStreamDataRead, impl.get(), std::placeholders::_1),
                    std::bind(&BoxReaderImpl::onStreamClosed, impl.get(), std::placeholders::_1))

    {
    }

    BoxReader(const BoxReader &) = delete;
    BoxReader(BoxReader &&)      = delete;

    void open() { inputStream.open(); }
    void read(std::size_t size) { inputStream.read(size); }

private:
    std::unique_ptr<BoxReaderImpl> impl;
    InputStream                    inputStream;
};

}
