#!/bin/bash
#sudo docker run -it --rm -v "$PWD":/git ubuntu "/bin/bash -c \"apt-get install git-core gcc g++ make cmake gawk bison flex bzip2 netpbm autoconf automake libx11-dev libxext-dev libc6-dev liblzo2-dev libxxf86vm-dev libpng-dev libsdl1.2-dev byacc python-mako libxcursor-dev gcc-multilib\"; /bin/bash"
IMAGE_NAME=apolloos/builder


#Do we have this image already?
DOCKER_IMAGE_COUNT=$(docker image ls | awk -F\  '{ print $1 }' | grep "${IMAGE_NAME} | wc -l")
echo ${DOCKER_IMAGE_COUNT}
if [ "${DOCKER_IMAGE_COUNT}" == "" ]
then
	docker build -t ${IMAGE_NAME} .
fi

#Run the docker and start the build
docker run -it --rm -v "$PWD":/usr/src ${IMAGE_NAME} /bin/bash -c "cd /usr/src; ./rebuild_all.sh"
