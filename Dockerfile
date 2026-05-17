FROM ubuntu:22.04

LABEL maintainer="Georgios Sokianos <walkero@gmail.com>"

ENV DEBIAN_FRONTEND=noninteractive

RUN dpkg --add-architecture i386 && \
    apt-get update && apt-get -y install \
    autoconf ccache make mmake automake cmake \
    gcc g++ gcc-multilib \
    wget \
    lhasa \
    git subversion mercurial \
    gawk bison flex netpbm genisoimage sshpass \
    python3-mako libswitch-perl gperf \
    patch bzip2 ca-certificates xz-utils \
    libpng-dev zlib1g-dev libxcursor-dev libgl1-mesa-dev libasound2-dev \
    libx11-dev libxext-dev libc6-dev liblzo2-dev libxxf86vm-dev libsdl2-dev \
    byacc libxxf86vm1:i386 \
    && [ -e /usr/bin/mkisofs ] || ln -s /usr/bin/genisoimage /usr/bin/mkisofs \
    && apt-get clean \
    && rm -rf /var/lib/apt/lists/* /tmp/* /var/tmp/*

WORKDIR /usr/src
CMD ["/bin/bash"]
