#! /bin/bash

mkdir -p build

gcc src/mkpart.c -Wall -Wextra -Werror -std=gnu99 -o build/mkpart -g
