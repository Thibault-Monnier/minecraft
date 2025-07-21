To run cmake, use:

```bash
cmake -G Ninja -B build -S .
cmake --build build
```

The executable will be `build/minecraft`.

Perf

```bash
sudo chrt -f 99 taskset -c 4 ./build-release/minecraft
```