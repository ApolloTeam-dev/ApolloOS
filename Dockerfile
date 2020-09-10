FROM phusion/baseimage:master-amd64

LABEL maintainer="Georgios Sokianos <walkero@gmail.com>"

# Use baseimage-docker's init system.
CMD ["/sbin/my_init"]

RUN dpkg --add-architecture i386; \
    apt-get update && apt-get -y install \
    autoconf ccache make mmake automake cmake \
    gcc \
    wget \
    lhasa \
    git subversion mercurial \
    gawk bison flex netpbm genisoimage sshpass \
    python-mako libswitch-perl gperf gcc-multilib g++ \
    patch bzip2 ca-certificates xz-utils \
    libpng-dev zlib1g-dev libxcursor-dev libgl1-mesa-dev libasound2-dev \
    libx11-dev libxext-dev libc6-dev liblzo2-dev libxxf86vm-dev libsdl1.2-dev byacc libxxf86vm1:i386; \
    ln -s /usr/bin/genisoimage /usr/bin/mkisofs; \
    apt-get clean && rm -rf /var/lib/apt/lists/* /tmp/* /var/tmp/*;

# Add git branch name to bash prompt
RUN sed -i '4c\'"\nparse_git_branch() {\n\
  git branch 2> /dev/null | sed -e \'/^[^*]/d\' -e \'s/* \\\(.*\\\)/ (\\\1)/\'\n\
}\n" ~/.bashrc; \
    sed -i '43c\'"force_color_prompt=yes" ~/.bashrc; \
    sed -i '57c\'"    PS1=\'\${debian_chroot:+(\$debian_chroot)}\\\[\\\033[01;32m\\\]\\\u@\\\h\\\[\\\033[00m\\\]:\\\[\\\033[01;34m\\\]\\\w\\\[\\\033[01;31m\\\]\$(parse_git_branch)\\\[\\\033[00m\\\]\\\$ '" ~/.bashrc; \
    sed -i '59c\'"    PS1=\'\${debian_chroot:+(\$debian_chroot)}\\\u@\\\h:\\\w \$(parse_git_branch)\$ \'" ~/.bashrc;
# Clean up APT when done.
RUN apt-get clean && rm -rf /var/lib/apt/lists/* /tmp/* /var/tmp/*
WORKDIR /usr/src
