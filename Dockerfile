FROM ubuntu:20.04

ENV DEBIAN_FRONTEND=noninteractive
RUN apt-get -y update && \
    apt-get -y --no-install-recommends install \
        g++ cmake make git ca-certificates curl \
        autoconf automake libpcre3-dev python3-dev

COPY . /gt/code

WORKDIR /gt/

# Fetch the correct version of LLVM
RUN git clone https://github.com/llvm/llvm-project.git
RUN git --work-tree=llvm-project checkout llvmorg-10.0.0
# Build and install it
RUN mkdir llvm-build && pushd llvm-build && \
    cmake -G "Unix Makefiles" -DCMAKE_BUILD_TYPE=RelWithDebInfo -DLLVM_ENABLE_ASSERTIONS=1 -DLLVM_TARGETS_TO_BUILD=X86 -DLLVM_INSTALL_UTILS=1 -DLLVM_ENABLE_PROJECTS=clang -DLLVM_TOOL_CLANG_BUILD=1 -DCMAKE_INSTALL_PREFIX=$PWD/../llvm-install ../llvm-project/llvm && \
    cmake --build . && cmake --build . --target install && popd

CMD ["code/.ci/run_tests.sh"]
