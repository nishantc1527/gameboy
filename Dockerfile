FROM ubuntu:24.04
RUN apt-get update && apt-get install -y build-essential git pkg-config python3 software-properties-common wget lsb-release gnupg apt-utils curl python3-venv python3-pip
RUN wget https://apt.llvm.org/llvm.sh && chmod +x llvm.sh && ./llvm.sh 21 && apt-get install -y clang-format-21 && ln -sf /usr/bin/clang-format-21 /usr/local/bin/clang-format
RUN curl --proto '=https' --tlsv1.2 -sSf https://sh.rustup.rs | sh -s -- -y
ENV PATH="/root/.cargo/bin:${PATH}"
RUN cargo install cbindgen
WORKDIR /gbemu
COPY . .
CMD make verify
