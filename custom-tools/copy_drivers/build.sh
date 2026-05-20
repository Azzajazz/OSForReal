#! /bin/bash

mkdir -p build

gcc src/copy_drivers.c -Wall -Wextra -Werror -std=gnu99 -o build/copy_drivers -g
