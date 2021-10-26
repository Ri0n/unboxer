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

#include "cacher.h"
#include "input.h"
#include "status.h"

#include <QByteArray>

#include <functional>
#include <string>

namespace unboxer {

class NullSource {
    std::function<void()>       openedCallback;
    std::function<void(Status)> closedCallback;

public:
    template <typename OpenedCB, typename DataReadCB, typename ClosedCB>
    NullSource([[maybe_unused]] const std::string &uri,
               OpenedCB                          &&openedCallback,
               [[maybe_unused]] DataReadCB       &&dataReadCallback,
               ClosedCB                          &&closedCallback)
    {
        this->openedCallback = openedCallback;
        this->closedCallback = std::move(closedCallback);
    }
    void open() { openedCallback(); }
    void read([[maybe_unused]] std::size_t size) { closedCallback(Status::Ok); };
    void reset() { }
};

class NullCache {
public:
    NullCache([[maybe_unused]] const std::string &uri) { }
    QByteArray read([[maybe_unused]] std::size_t size) { return QByteArray(); }
    void       write([[maybe_unused]] const QByteArray &data) { }
    void       reset() { }
};

template <class InputImpl, class CacherImpl> class InputStreamer {
public:
    using OpenedCallback   = std::function<void()>;
    using DataReadCallback = std::function<Status(const QByteArray &)>;
    using ClosedCallback   = std::function<void(Status)>;

    InputStreamer(const std::string &inputUri,
                  OpenedCallback   &&openedCallback,
                  DataReadCallback &&dataReadCallback,
                  ClosedCallback   &&closedCallback) :
        source(inputUri,
               std::bind(&InputStreamer::onStreamOpened, this),
               std::bind(&InputStreamer::onDataRead, this, std::placeholders::_1),
               std::bind(&InputStreamer::onStreamClosed, this, std::placeholders::_1)),
        openedCallback(std::move(openedCallback)), dataReadCallback(std::move(dataReadCallback)),
        closedCallback(std::move(closedCallback))
    {
    }

    void open() { source.open(); }
    void read(std::size_t size) { source.read(size); }

private:
    void onStreamOpened() { openedCallback(); }
    void onDataRead(const QByteArray &data)
    {
        if (dataReadCallback(data) != Status::Ok) {
            source.reset();
            cacher.reset();
            closedCallback(Status::Corrupted);
        }
    }
    void onStreamClosed(Status reason) { closedCallback(reason); }

private:
    Input<InputImpl>   source;
    Cacher<CacherImpl> cacher;

    OpenedCallback   openedCallback;
    DataReadCallback dataReadCallback;
    ClosedCallback   closedCallback;
};

}
