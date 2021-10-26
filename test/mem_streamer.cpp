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

#include "inputmemory_impl.h"
#include "inputstreamer.h"
#include "status.h"
#include "unboxer.h"

using namespace unboxer;
using MemStreamer = unboxer::InputStreamer<InputMemoryImpl, NullCache>;

class MemStreamerTest : public QObject {
    Q_OBJECT

    std::unique_ptr<MemStreamer> streamer;
    bool                         gotOpened    = false;
    bool                         gotDataRead  = false;
    bool                         gotClosed    = false;
    bool                         gotDataReady = false;
    QByteArray                   data;

private slots:

    void init()
    {
        data         = QByteArray();
        gotOpened    = false;
        gotDataRead  = false;
        gotClosed    = false;
        gotDataReady = false;

        streamer = std::make_unique<MemStreamer>(
            "SGVsbG8gV29ybGQ=",
            [&]() mutable { gotOpened = true; },
            [&](const QByteArray &data) mutable {
                gotDataRead = true;
                this->data  = data;
                return Status::Ok;
            },
            [&](Status) mutable { gotClosed = true; });
        streamer->setDataReadyCallback([&]() mutable { gotDataReady = true; });
    }

    void openTest()
    {
        streamer->open(); // will trigger opened immediatelly
        QCOMPARE(gotOpened, true);
        QCOMPARE(gotDataRead, false);
        QCOMPARE(gotDataReady, false);
        QCOMPARE(gotClosed, false);
        QCOMPARE(data, QByteArray());
    }

    void readTest()
    {
        streamer->open();
        streamer->read(5);
        QCOMPARE(gotOpened, true);
        QCOMPARE(gotDataRead, true);
        QCOMPARE(gotDataReady, true);
        QCOMPARE(gotClosed, false);
        QCOMPARE(data, QByteArray("Hello", 5));
        streamer->read(6);
        QCOMPARE(gotOpened, true);
        QCOMPARE(gotDataRead, true);
        QCOMPARE(gotClosed, true);
        QCOMPARE(data, QByteArray(" World", 6));
    }

    void cleanup() { streamer.reset(); }
};

QTEST_MAIN(MemStreamerTest)

#include "mem_streamer.moc"
