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
using MemUnboxer = unboxer::Unboxer<InputMemoryImpl, NullCache>;

class MemUnboxerTest : public QObject {
    Q_OBJECT

    std::unique_ptr<MemUnboxer> unboxer;
    bool                        gotStreamOpened    = false;
    bool                        gotDataRead        = false;
    bool                        gotSubBox          = false;
    bool                        gotStreamClosed    = false;
    bool                        gotStreamDataReady = false;
    Status                      streamCloseStatus  = Status::Ok;

    void setupBox(Box::Ptr box)
    {
        gotSubBox         = true;
        box->onSubBoxOpen = std::bind(&MemUnboxerTest::setupBox, this, std::placeholders::_1);
        box->onDataRead   = [&](const QByteArray &) mutable {
            gotDataRead = true;
            return Status::Ok;
        };
    }

private slots:

    void init()
    {
        gotStreamOpened    = false;
        gotDataRead        = false;
        gotSubBox          = false;
        gotStreamClosed    = false;
        gotStreamDataReady = false;
        streamCloseStatus  = Status::Ok;

        // let's parse first 256 bytes of the demo file
        unboxer = std::make_unique<MemUnboxer>(
            "AAAAtW1vb2YAAAAQbWZoZAAAAAAAD+OzAAAAnXRyYWYAAAAYdGZoZAAAABgAAAABATEtAAAARewA"
            "AAAUdHJ1bgAAAAEAAAABAAAAgAAAACx1dWlkbR2bBULVROaA4hQdr/dXsgEAAAAAADCvZXt2AAAA"
            "AAABMS0AAAAAPXV1aWTUgH7yyjlGlY5UJsueRqefAQAAAAIAADCvZqyjAAAAAAABMS0AAAAwr2fd"
            "0AAAAAAAATEtAAAARfRtZGF0PD94bWwgdmVyc2lvbj0iMS4wIiBlbmNvZGluZz0iVVRGLTgiPz48"
            "dHQgeG1sOmxhbmc9InNwYSIgeG1sbnM9Imh0dA==");
        unboxer->setStreamOpenedCallback([this](Box::Ptr root) mutable {
            gotStreamOpened = true;
            setupBox(root);
        });
        unboxer->setStreamClosedCallback([this](Status status) mutable {
            streamCloseStatus = status;
            gotStreamClosed   = true;
        });
        unboxer->stream().setDataReadyCallback([&]() mutable { gotStreamDataReady = true; });
    }

    void openTest()
    {
        unboxer->open(); // will trigger opened immediatelly
        QCOMPARE(gotStreamOpened, true);
        QCOMPARE(gotDataRead, false);
        QCOMPARE(gotStreamClosed, false);
    }

    void readTest()
    {
        unboxer->open();
        unboxer->read(256); // read all
        QCOMPARE(gotStreamOpened, true);
        QCOMPARE(gotDataRead, true);
        QCOMPARE(gotStreamClosed, true);
        QCOMPARE(streamCloseStatus, Status::Corrupted);
    }

    void cleanup() { unboxer.reset(); }
};

QTEST_MAIN(MemUnboxerTest)

#include "mem_unboxer.moc"
