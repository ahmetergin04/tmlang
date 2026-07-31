cmake -B build
cmake --build build
ctest --test-dir build -V --output-on-failure
