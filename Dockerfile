FROM ubuntu:18.04 AS builder
WORKDIR /project
RUN apt-get update -q && DEBIAN_FRONTEND=noninteractive \
    apt-get install -q -y cmake git vim gcc g++ gfortran software-properties-common \
            mpich libmpich-dev \
            openmpi-bin openmpi-doc libopenmpi-dev && \
    apt-get clean && \
    rm -rf /var/lib/apt/lists/*

# Installing latest GCC compiler (version 10)
RUN add-apt-repository ppa:ubuntu-toolchain-r/test
RUN apt-get update -q && \
    apt-get install -q -y gcc-10 g++-10 gfortran-10 && \
    apt-get clean && \
    rm -rf /var/lib/apt/lists/*

# We are installing both OpenMPI and MPICH. We could use the update alternatives to switch between them
RUN update-alternatives --install /usr/bin/gcc gcc /usr/bin/gcc-10 90\
                        --slave /usr/bin/g++ g++ /usr/bin/g++-10\
                        --slave /usr/bin/gfortran gfortran /usr/bin/gfortran-10\
                        --slave /usr/bin/gcov gcov /usr/bin/gcov-10

SHELL ["/bin/bash", "-c"]

RUN groupadd chapter16 && useradd -m -s /bin/bash -g chapter16 chapter16

WORKDIR /home/chapter16
RUN chown -R chapter16:chapter16 /home/chapter16
USER chapter16

RUN git clone --recursive https://github.com/essentialsofparallelcomputing/Chapter16.git

WORKDIR /home/chapter16/Chapter16
#RUN make

ENTRYPOINT ["bash"]