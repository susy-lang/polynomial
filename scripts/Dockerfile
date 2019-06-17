FROM alpine
MAINTAINER chrissof <chris@ethereum.org>
#Official polynomial docker image

#Establish working directory as polynomial
WORKDIR /polynomial
#Copy working directory on travis to the image
COPY / $WORKDIR

#Install dependencies, eliminate annoying warnings, and build release, delete all remaining points and statically link.
RUN ./scripts/install_deps.sh && sed -i -E -e 's/include <sys\/poll.h>/include <poll.h>/' /usr/include/boost/asio/detail/socket_types.hpp &&\
cmake -DCMAKE_BUILD_TYPE=Release -DTESTS=0 -DSTATIC_LINKING=1 &&\
make polc && install -s  polc/polc /usr/bin	&&\
cd / && rm -rf polynomial &&\
apk del sed build-base git make cmake gcc g++ musl-dev curl-dev boost-dev &&\
rm -rf /var/cache/apk/*
