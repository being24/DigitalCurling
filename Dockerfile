
FROM ghcr.io/being24/cmake_boost_ubuntu:latest

WORKDIR /work/

ENV LC_CTYPE='C.UTF-8'
ENV DEBIAN_FRONTEND=noninteractive

RUN set -x && \ 
    apt-get update && apt-get install -y && \
    cd /work/ && \
    git clone --recursive https://github.com/digitalcurling/DigitalCurling.git && \
    cd DigitalCurling && \
    git checkout develop && \
    git submodule update --init --recursive && \
    mkdir build && \
    cd build && \
    cmake -DCMAKE_BUILD_TYPE=RelWithDebInfo -DBOX2D_BUILD_TESTBED=OFF .. && \
    cmake --build . --config RelWithDebInfo

CMD ["/bin/bash"]
