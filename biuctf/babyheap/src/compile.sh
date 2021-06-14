#!/bin/bash
glibc_install="$(pwd)/glibc/build/install"
set -eux
gcc \
  -no-pie \
  -L "${glibc_install}/lib" \
  -I "${glibc_install}/include" \
  -Wl,--rpath="${glibc_install}/lib" \
  -Wl,--dynamic-linker="${glibc_install}/lib/ld-linux-x86-64.so.2" \
  -std=c11 \
  -o babyheap.out \
  -v \
  babyheap.c \
;
ldd ./babyheap.out
