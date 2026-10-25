cmake -B build
cmake --build build
ctest --test-dir build -V --output-on-failure

bende ctest hata verdi config olmadan calismiyor diye assagidakini kullaninca oldu.
ctest --test-dir build -C Debug -V --output-on-failure