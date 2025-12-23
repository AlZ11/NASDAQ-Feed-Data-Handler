# Nasdaq Feed Data Handler

![Language](https://img.shields.io/badge/language-C%2B%2B17-blue)
![Architecture](https://img.shields.io/badge/architecture-Zero--Copy-green)
![Performance](https://img.shields.io/badge/throughput-11M-orange)

A low latency market data feed handler built in C++ to parse the NASDAQ TotalView-ITCH 5.0 binary protocol. This engine processes **11.5M messages/s** on a single core by utilizing systems programming techniques such as memory mapping (`mmap`) and cache friendly data structures.

## 🚀 Key Features

* **Zero Copy Architecture:** Utilizes `mmap` to load 11GB+ datasets into virtual memory and processes messages via pointer casting (`reinterpret_cast`) to eliminate copies from kernel to user RAM.
* **Contiguous Memory Order Book:** Replaces standard trees data structure (`std::map`) with sorted flat vectors (`std::vector`) to minimize cache misses and memory fragmentation.
* **Protocol Compliance:** Functional implementation of NASDAQ ITCH 5.0 specs, handling System Events, Order Addition, Execution, Cancellation, and Deletion.
* **Endianness Handling:** Uses compiler intrinsics (`__builtin_bswap`) for efficient Big to Little Endian conversion.

## 📊 Performance Benchmarks

Benchmarks were run on a Apple M2 Pro processing a 11GB ITCH 5.0 binary file (~368 million messages).

| Optimization Stage | Throughput | Implementation Details |
| :--- | :--- | :--- |
| **Baseline** | ~7.3M msgs/sec | `mmap` + `std::map` (Red Black Tree) + Debug Build |
| **Optimized** | ~11.5M msgs/sec | `std::vector` (Flat Map) + `-O3` + `-march=native` + Binary Search  |
<img width="362" height="167" alt="Screenshot 2025-12-23 at 7 21 55 pm" src="https://github.com/user-attachments/assets/76558085-af11-44f5-815e-60548044a63e" />  

**Performance Gain:** +57% increase in throughput by optimizing for L1 Cache locality.


## 💻 Build & Run

**Prerequisites:**
* C++17 Compiler (GCC/Clang/MSVC)
* Make or CMake (Optional)

**Compilation:**
```bash
# Compile with maximum optimization and native architecture tuning
cd src
clang++ -std=c++17 -O3 -march=native main.cpp MessageHandler.cpp MMapReader.cpp OrderBook.cpp -o main
