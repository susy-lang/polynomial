FROM alpine
MAINTAINER chrissof <chris@ethereum.org>

RUN \
  apk --no-cache --update add build-base cmake boost-dev git                                                && \
  sed -i -E -e 's/include <sys\/poll.h>/include <poll.h>/' /usr/include/boost/asio/detail/socket_types.hpp  && \
  git clone --depth 1 --recursive -b release https://octonion.institute/susy-lang/polynomial                           && \
  cd /polynomial && cmake -DCMAKE_BUILD_TYPE=Release -DTESTS=0 -DSTATIC_LINKING=1                             && \
  cd /polynomial && make polc && install -s  polc/polc /usr/bin                                               && \
  cd / && rm -rf polynomial                                                                                   && \
  apk del sed build-base git make cmake gcc g++ musl-dev curl-dev boost-dev                                 && \
  rm -rf /var/cache/apk/*
