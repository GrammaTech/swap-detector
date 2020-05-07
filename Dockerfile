FROM ubuntu:20.04

ENV DEBIAN_FRONTEND=noninteractive
RUN apt-get -y update && \
    apt-get -y --no-install-recommends install \
        g++ cmake make git ca-certificates curl \
        autoconf automake libpcre3-dev python3-dev \
        python3-pip
RUN pip3 install pytest

COPY . /gt/code

WORKDIR /gt/

CMD ["code/.ci/run_tests.sh"]
