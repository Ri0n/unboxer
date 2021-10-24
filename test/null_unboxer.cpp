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

#include <QTest>

#include "error.h"
#include "input_streamer.h"
#include "unboxer.h"

class NullSource {
    std::function<void()>                openedCallback;
    std::function<void(unboxer::Reason)> closedCallback;

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
    void read([[maybe_unused]] std::size_t size) { closedCallback(unboxer::Reason::Ok); };
};

class NullCache {
};

using NullUnboxer = unboxer::Unboxer<unboxer::InputStreamer<NullSource, NullCache>>;

class NullUnboxerTest : public QObject {
    Q_OBJECT

    std::unique_ptr<NullUnboxer> unboxer;
    bool                         gotOpened    = false;
    bool                         gotDataReady = false;
    bool                         gotClosed    = false;

private slots:

    void init()
    {
        gotOpened    = false;
        gotDataReady = false;
        gotClosed    = false;

        unboxer = std::make_unique<NullUnboxer>(
            "file:///dev/null",
            [&]() mutable { gotOpened = true; },
            [&](NullUnboxer::Data) mutable { gotDataReady = true; },
            [&](unboxer::Reason) mutable { gotClosed = true; });
    }

    void openTest()
    {
        unboxer->open(); // will trigger opened immediatelly
        QCOMPARE(gotOpened, true);
        QCOMPARE(gotDataReady, false);
        QCOMPARE(gotClosed, false);
    }

    void readTest()
    {
        unboxer->open();
        unboxer->read(1); // we read 1 byte. but with null unboxer it means everything
        QCOMPARE(gotOpened, true);
        QCOMPARE(gotDataReady, false);
        QCOMPARE(gotClosed, true);
    }

    void cleanup() { unboxer.reset(); }
};

QTEST_MAIN(NullUnboxerTest)

#include "null_unboxer.moc"
