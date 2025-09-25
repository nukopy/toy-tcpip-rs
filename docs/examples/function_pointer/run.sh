#! /bin/bash

# build C program
gcc function_pointer.c -o function_pointer_c

# run
./function_pointer_c

# build Rust program
rustc function_pointer.rs -o function_pointer_rs

# run
./function_pointer_rs

# clean
rm function_pointer_c function_pointer_rs
