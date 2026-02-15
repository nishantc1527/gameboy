FROM ubuntu:24.04
RUN apt-get update && apt-get install -y build-essential git pkg-config python3 cmake software-properties-common wget lsb-release gnupg apt-utils curl python3-venv python3-pip
RUN wget https://apt.llvm.org/llvm.sh \
    && chmod +x llvm.sh \
    && ./llvm.sh 21 \
    && apt-get install -y clang-format-21 clang-21
RUN ln -s /usr/bin/clang-format-21 /usr/bin/clang-format \
    && ln -s /usr/bin/clang-21 /usr/bin/clang
RUN curl --proto '=https' --tlsv1.2 -sSf https://sh.rustup.rs | sh -s -- -y
ENV PATH="/root/.cargo/bin:${PATH}"
RUN cargo install cbindgen
RUN git clone --depth 1 https://github.com/libsdl-org/SDL \
    && cd SDL \
    && cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -DSDL_UNIX_CONSOLE_BUILD=ON \
    && cmake --build build -j$(nproc) \
    && cmake --install build \
    && cd .. && rm -rf SDL
WORKDIR /gbemu
COPY . .
CMD ["make", "verify"]
