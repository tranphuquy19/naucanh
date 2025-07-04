FROM ubuntu:22.04 AS builder

ENV DEBIAN_FRONTEND=noninteractive

# Install dependencies
RUN apt-get update && \
    apt-get install -y \
    gcc libc6-dev \
    musl-tools \
    autoconf automake libtool pkg-config make \
    # for strip
    binutils \
    upx

COPY . /src
WORKDIR /src

# Build the project
RUN make clean && \
    make

RUN strip --strip-all naucanh && \
    upx --best --lzma naucanh

FROM scratch
COPY --from=builder /src/naucanh /naucanh
ENTRYPOINT ["/naucanh"]
