FROM ubuntu:bionic

RUN apt-get update
RUN apt-get install -y cmake libssl-dev
RUN apt-get install -y clang lldb-9 lld-9
RUN apt-get install -y libllvm-9-ocaml-dev libllvm9 llvm-9 llvm-9-dev llvm-9-doc llvm-9-examples llvm-9-runtime
RUN touch /usr/lib/llvm-9/bin/yaml-bench
ENV LLVM_HOME /usr/lib/llvm-9/

WORKDIR /anselm

COPY Anselm.cpp Context.cpp Context.h CMakeLists.txt ./

RUN cmake .
RUN make

VOLUME /tests

COPY run.sh ./
RUN chmod +x run.sh

CMD ["/anselm/run.sh"]
