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

#include "status.h"

#include <QByteArray>

#include <functional>
#include <memory>
#include <optional>
#include <variant>

namespace unboxer {

class Box {
public:
    using Ptr = std::shared_ptr<Box>;

    inline Box(bool          isContainer = false,
               QByteArray    type        = QByteArray(),
               std::uint64_t size        = 0,
               std::uint64_t fileOffset  = 0) :
        isContainer(isContainer),
        type(type), size(size), fileOffset(fileOffset)
    {
    }

    bool          isContainer = false;
    QByteArray    type;
    std::uint64_t size; // full size. 0 - all remaining
    std::uint64_t fileOffset;

    // callbacks to be set by a library user
    std::function<void(Box::Ptr)>             onSubBoxOpen;
    std::function<Status(const QByteArray &)> onDataRead;
    std::function<Status()>                   onClose;

private:
    friend class UnboxerImpl;
    bool          isClosed_ = false;
    std::uint64_t dataFed_  = 0;
};

}
