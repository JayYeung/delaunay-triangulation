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
