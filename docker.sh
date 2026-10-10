#!/bin/bash

IMAGE_NAME=apollo-os-build:latest

if [ -z "$(docker images -q "${IMAGE_NAME}")" ]
then
    docker build -t "${IMAGE_NAME}" .
fi

docker run -it --rm -v /etc/passwd:/etc/passwd:ro -v /etc/group:/etc/group:ro -v "${PWD}":/usr/src -u "$(id -u):$(id -g)" "${IMAGE_NAME}" /bin/bash -c "./mkapollo.sh --ports=/usr/src/bin/Sources all"
