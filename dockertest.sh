#!/bin/sh
docker build --no-cache -t chapter16 .
#docker run -it --entrypoint /bin/bash chapter16
docker build --no-cache -f Dockerfile.Ubuntu20.04 -t chapter16 .
#docker run -it --entrypoint /bin/bash chapter16
docker build --no-cache -f Dockerfile.debian -t chapter16 .
#docker run -it --entrypoint /bin/bash chapter16
