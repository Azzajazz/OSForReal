#! /bin/bash

mkdir -p build

gcc src/tfsfmt.c -Wall -Wextra -Werror -std=gnu99 -o build/tfsfmt -g
gcc tests/runner.c -Wall -Wextra -Werror -std=gnu99 -o build/runner -g
