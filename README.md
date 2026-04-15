## standard c++ 17

```bash
(base) Jay_Y@MacBook-Pro-849:~/Desktop/cs274/project$ g++ --version
Apple clang version 21.0.0 (clang-2100.0.123.102)
Target: arm64-apple-darwin25.3.0
Thread model: posix
InstalledDir: /Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin
```

## setup

```bash
cd shewchuk && make trilibrary && cd ..
```

## normal command

```bash
cc -O -c shewchuk/predicates.c -o pred.o && g++ -O -o my_project my_project.cpp pred.o -lm && ./my_project
```

## warnings suppressed

```bash
cc -O -Wno-deprecated-non-prototype -c shewchuk/predicates.c -o pred.o && \
g++ -O -Wno-c++17-extensions -o my_project my_project.cpp pred.o -lm && \
./my_project
```

## usage

```bash
./my_project <input_node_file> [alternating_cuts_flag]
example1: ./my_project tests/spiral.node alternating
example2: ./my_project tests/ttimeu1000000.node vertical
```

## my timings

```
(base) Jay_Y@MacBook-Pro-849:~/Desktop/cs274/project$ cc -O -Wno-deprecated-non-prototype -c shewchuk/predicates.c -o pred.o && \
g++ -O -Wno-c++17-extensions -o my_project my_project.cpp pred.o -lm && \
./my_project tests/ttimeu1000000.node.gz
Using input path:  | "tests/ttimeu1000000.node.gz"
Using output path:  | "tests/ttimeu1000000.node.ele"

0: vertical cuts only (default), 1: alternating cuts
No alternating cuts flag provided. Using:  | 0

DT computed in  | 1607 |  ms
(base) Jay_Y@MacBook-Pro-849:~/Desktop/cs274/project$ cc -O -Wno-deprecated-non-prototype -c shewchuk/predicates.c -o pred.o && \
g++ -O -Wno-c++17-extensions -o my_project my_project.cpp pred.o -lm && \
./my_project tests/ttimeu100000.node.gz
Using input path:  | "tests/ttimeu100000.node.gz"
Using output path:  | "tests/ttimeu100000.node.ele"

0: vertical cuts only (default), 1: alternating cuts
No alternating cuts flag provided. Using:  | 0

DT computed in  | 125 |  ms
(base) Jay_Y@MacBook-Pro-849:~/Desktop/cs274/project$ cc -O -Wno-deprecated-non-prototype -c shewchuk/predicates.c -o pred.o && \
g++ -O -Wno-c++17-extensions -o my_project my_project.cpp pred.o -lm && \
./my_project tests/ttimeu10000.node.gz
Using input path:  | "tests/ttimeu10000.node.gz"
Using output path:  | "tests/ttimeu10000.node.ele"

0: vertical cuts only (default), 1: alternating cuts
No alternating cuts flag provided. Using:  | 0

DT computed in  | 10 |  ms
```
