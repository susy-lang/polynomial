FROM alpine AS build
MAINTAINER chrissof <chris@ethereum.org>
#Official polynomial docker image

#Establish working directory as polynomial
WORKDIR /polynomial

# Build dependencies
ADD /scripts/install_deps.sh /polynomial/scripts/install_deps.sh
RUN ./scripts/install_deps.sh

#Copy working directory on travis to the image
COPY / $WORKDIR

# Number of parallel jobs during build
# or 0 for auto-computing (max(1, CPU_core_count * 2/3), a greedy value)
ARG BUILD_CONCURRENCY="0"

#Install dependencies, eliminate annoying warnings
RUN sed -i -E -e 's/include <sys\/poll.h>/include <poll.h>/' /usr/include/boost/asio/detail/socket_types.hpp
RUN cmake -DCMAKE_BUILD_TYPE=Release -DTESTS=0 -DPOLC_LINK_STATIC=1
RUN make polc \
    -j$(awk "BEGIN {                                       \
        if (${BUILD_CONCURRENCY} != 0) {                   \
            print(${BUILD_CONCURRENCY});                   \
        } else {                                           \
            x=($(grep -c ^processor /proc/cpuinfo) * 2/3); \
            if (x > 1) {                                   \
                printf(\"%d\n\", x);                       \
            } else {                                       \
                print(1);                                  \
            }                                              \
        }                                                  \
    }")
RUN strip polc/polc

FROM scratch
COPY --from=build /polynomial/polc/polc /usr/bin/polc
ENTRYPOINT ["/usr/bin/polc"]
