FROM ubuntu:20.04 as builder

ENV DEBIAN_FRONTEND="noninteractive" TZ="UTC"

RUN apt update -y \
    && apt -y install \
    build-essential \
    cmake \
    qtbase5-dev

WORKDIR /src

COPY ./ /src
RUN rm -rf build && mkdir build && cd build && cmake -DCMAKE_BUILD_TYPE=Release .. &&  cmake --build . -j 8 && mv tools/mp4crawler ../ && rm -rf ../build

FROM ubuntu:20.04 as runtime

ENV DEBIAN_FRONTEND="noninteractive" TZ="UTC"

RUN apt update -y \
    && apt -y install \
    libqt5core5a \
    libqt5gui5 \
    ca-certificates

WORKDIR /unboxer
COPY --from=builder /src/mp4crawler .

ENTRYPOINT ["./mp4crawler"]
