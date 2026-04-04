FROM ubuntu:22.04
WORKDIR /usr/src

RUN apt-get -y update
RUN apt-get -y install \
    autoconf \
    automake \
    bison \
    byacc \
    bzip2 \
    cmake \
    flex \
    gawk \
    gcc \
    genisoimage \
    git \
    g++ \
    libc6-dev \
    liblzo2-dev \
    libpng-dev \
    libsdl1.2-dev \
    libx11-dev \
    libxcursor-dev \
    libxext-dev \
    libxxf86vm-dev \
    make \
    mmake \
    netpbm \
    wget
RUN apt-get clean
RUN rm -rf /var/lib/apt/lists/* /tmp/* /var/tmp/*

CMD ["/bin/bash"]
