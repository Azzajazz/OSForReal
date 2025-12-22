#! /bin/bash

gcc src/tagfs.c -I libfuse/include/ -I libfuse/build -L libfuse/build/lib -lfuse3 -o build/tagf
