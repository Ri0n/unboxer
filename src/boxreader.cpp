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

#include "boxreader.h"

#include <QtEndian>

#include <optional>

namespace unboxer {

constexpr std::uint64_t MAX_BOX_SIZE      = 50ull * 1024 * 1024 * 1024;
constexpr std::uint8_t  MINIMAL_HEADER_SZ = 8;
constexpr std::uint8_t  EXTENDED_TYPE_SZ  = 16;

/* // from standard (mp4 4.2 Object Structure)
aligned(8) class Box (unsigned int(32) boxtype,
                      optional unsigned int(8)[16] extended_type) {

 unsigned int(32) size;
 unsigned int(32) type = boxtype;

 if (size==1) {
  unsigned int(64) largesize;
 } else if (size==0) {
  // box extends to end of file
 }
 if (boxtype==‘uuid’) {
  unsigned int(8)[16] usertype = extended_type;
 }
}
*/

class BoxReaderImpl {
public:
    Reason feed(const QByteArray &data);
    Reason close(Reason reason);

    Reason sendData();

    std::optional<std::uint64_t> fullBoxSize; // if not set -> not enough data to parse size

    QByteArray    incompleteBox;           // a part of payload. could be somewhere in the middle of a box
    int           parsingOffset       = 0; // from the start of incompleteBox
    std::uint64_t boxPayloadBytesLeft = 0;

    BoxReader::BoxOpenedCallback boxOpenedCallback;
    BoxReader::BoxClosedCallback boxClosedCallback;
    BoxReader::DataReadCallback  dataReadCallback;
};

Reason BoxReaderImpl::feed(const QByteArray &data)
{
    incompleteBox += data;
    while (incompleteBox.size() - parsingOffset > 0) { // iterate over boxes
        if (!fullBoxSize) {                            // either very beginning of a box or not enough data to parse
            // C++20 span would work better here
            char       *parseStart = incompleteBox.data() + parsingOffset;
            std::size_t bytesLeft  = incompleteBox.size() - parsingOffset;

            std::uint64_t payloadOffset = 0;
            if (bytesLeft < MINIMAL_HEADER_SZ) // size + type
                break;                         // will wait for more data
            std::uint64_t boxSize = qFromBigEndian<quint32>(parseStart);
            payloadOffset         = MINIMAL_HEADER_SZ; // skip minimal header (type + size)
            bool hasExtendedSize  = false;
            if (boxSize == 1) { // have extended size
                if (bytesLeft < payloadOffset + sizeof(std::uint64_t)) {
                    break;
                }
                boxSize = qFromBigEndian<quint64>(parseStart + payloadOffset);
                payloadOffset += sizeof(std::uint64_t); // skip extended size
                hasExtendedSize = true;
            }
            auto boxType = QByteArray(parseStart + 4, 4);
            if (boxType == QByteArray("uuid")) {
                if (bytesLeft < payloadOffset + EXTENDED_TYPE_SZ) { // have uuid already?
                    break;                                          // need more data
                }
                boxType = QByteArray(parseStart + payloadOffset, EXTENDED_TYPE_SZ);
                payloadOffset += EXTENDED_TYPE_SZ;
                // REVIEW we can validate uuid too.
            }
            // validate size
            if (boxSize > MAX_BOX_SIZE || (hasExtendedSize && boxSize < payloadOffset)
                || (!hasExtendedSize && boxSize > 0 && boxSize < payloadOffset)) { // size 0 - is all remaining
                // looks like something invalid
                return Reason::Corrupted;
            }

            boxOpenedCallback(boxType, boxSize);
            parsingOffset += payloadOffset;
            fullBoxSize         = boxSize;
            boxPayloadBytesLeft = boxSize ? boxSize - payloadOffset : 0;
        }

        auto status = sendData();
        if (status == Reason::NeedMoreData) {
            break;
        }
        if (status != Reason::Ok) {
            return Reason::Corrupted;
        }
    }
    // we sent as much data as we could. drop beginning of the buffer
    incompleteBox.remove(0, parsingOffset);
    parsingOffset = 0;
    return Reason::Ok;
}

Reason BoxReaderImpl::close(Reason reason)
{
    if (reason == Reason::Eof && fullBoxSize) { // we have unfinished box on the end of the stream
        if (*fullBoxSize) {                     // and its data wasn't consumed for some reason. looks wrong
            return Reason::Corrupted;
        } else {
            boxClosedCallback();
        }
    }
    return reason;
}

Reason BoxReaderImpl::sendData()
{
    char       *parseStart = incompleteBox.data() + parsingOffset;
    std::size_t bytesLeft  = incompleteBox.size() - parsingOffset;

    // send data to callback (TODO we need the same on eof in case of zero size box)
    auto sendSz = *fullBoxSize ? qMin(boxPayloadBytesLeft, std::uint64_t(bytesLeft)) : incompleteBox.size();
    if (sendSz) {
        auto status = dataReadCallback(QByteArray::fromRawData(parseStart, sendSz));
        if (status == Reason::Ok) {
            parsingOffset += sendSz;
            boxPayloadBytesLeft -= sendSz;
        } else {
            return status;
        }
    }
    // the data was consumed. check if we finished sending all the data of the box
    if (*fullBoxSize && !boxPayloadBytesLeft) {
        fullBoxSize = std::nullopt; // mark as the start of the next box
        boxClosedCallback();
    }
    return Reason::Ok;
}

BoxReader::BoxReader(BoxOpenedCallback &&boxOpened, BoxClosedCallback &&boxClosed, DataReadCallback &&dataRead) :
    impl(std::make_unique<BoxReaderImpl>())

{
    impl->boxOpenedCallback = std::move(boxOpened);
    impl->boxClosedCallback = std::move(boxClosed);
    impl->dataReadCallback  = std::move(dataRead);
}

BoxReader::~BoxReader() { } // just to know how to destroy impl

Reason BoxReader::feed(const QByteArray &inputData) { return impl->feed(inputData); }

Reason BoxReader::close(Reason reason) { return impl->close(reason); }

} // namespace unboxer
