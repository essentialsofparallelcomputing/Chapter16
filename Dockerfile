# Part of the examples from the Parallel and High Performance Computing
# Robey and Zamora, Manning Publications
#   https://github.com/EssentialsofParallelComputing/Chapter8
#
# The built image can be found at:
#
#   https://hub.docker.com/r/essentialsofparallelcomputing/chapter8
#
# Author:
# Bob Robey <brobey@earthlink.net>

FROM ubuntu:20.04
LABEL maintainer Bob Robey <brobey@earthlink.net>

ARG DOCKER_LANG=en_US
ARG DOCKER_TIMEZONE=America/Denver

WORKDIR /tmp
RUN apt-get -qq update && \
    DEBIAN_FRONTEND=noninteractive \
    apt-get -qq install -y locales tzdata && \
    apt-get clean && rm -rf /var/lib/apt/lists/*

ENV LANG=$DOCKER_LANG.UTF-8 \
    LANGUAGE=$DOCKER_LANG:UTF-8

RUN ln -fs /usr/share/zoneinfo/$DOCKER_TIMEZONE /etc/localtime && \
    locale-gen $LANG && update-locale LANG=$LANG && \
    dpkg-reconfigure -f noninteractive locales tzdata

ENV LC_ALL=$DOCKER_LANG.UTF-8

RUN apt-get -qq update && \
    DEBIAN_FRONTEND=noninteractive \
    apt-get -qq install -y cmake git vim gcc g++ gfortran software-properties-common \
            openmpi-bin openmpi-doc libopenmpi-dev \
            libhdf5-openmpi-dev && \
    apt-get clean && rm -rf /var/lib/apt/lists/*
#   mpich libmpich-dev \
#   libhdf5-mpich-dev libhdf5-mpi-dev && \


# Installing latest GCC compiler (version 10)
RUN apt-get -qq update && \
    apt-get -qq install -y gcc-10 g++-10 gfortran-10 && \
    apt-get clean && rm -rf /var/lib/apt/lists/*

RUN update-alternatives \
      --install /usr/bin/gcc      gcc      /usr/bin/gcc-9       80 \
      --slave   /usr/bin/g++      g++      /usr/bin/g++-9          \
      --slave   /usr/bin/gfortran gfortran /usr/bin/gfortran-9     \
      --slave   /usr/bin/gcov     gcov     /usr/bin/gcov-9      && \
    update-alternatives \
      --install /usr/bin/gcc      gcc      /usr/bin/gcc-10      90 \
      --slave   /usr/bin/g++      g++      /usr/bin/g++-10         \
      --slave   /usr/bin/gfortran gfortran /usr/bin/gfortran-10    \
      --slave   /usr/bin/gcov     gcov     /usr/bin/gcov-10     && \
    chmod u+s /usr/bin/update-alternatives


SHELL ["/bin/bash", "-c"]

RUN groupadd chapter16 && useradd -m -s /bin/bash -g chapter16 chapter16

WORKDIR /home/chapter16
RUN chown -R chapter16:chapter16 /home/chapter16
USER chapter16

RUN git clone --recursive https://github.com/essentialsofparallelcomputing/Chapter16.git

WORKDIR /home/chapter16/Chapter16
#RUN make

ENTRYPOINT ["bash"]
