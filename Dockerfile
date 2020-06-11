FROM ubuntu:20.04

ENV DEBIAN_FRONTEND=noninteractive
RUN apt-get -y update && \
    apt-get -y --no-install-recommends install \
        g++ cmake make git ca-certificates curl \
        autoconf automake libpcre3-dev python3-dev wget \
        gnupg software-properties-common lsb-release sudo \
        python3-pip

RUN ["/bin/bash", "-c", "wget -O - https://apt.llvm.org/llvm-snapshot.gpg.key|sudo apt-key add - && add-apt-repository \"deb http://apt.llvm.org/focal/     llvm-toolchain-focal-10 main\" && apt-get -y -qq install llvm-10 clang-10 libclang-10-dev"]

RUN pip3 install lit

COPY . /gt/code

WORKDIR /gt/

CMD ["code/.ci/run_tests.sh"]
