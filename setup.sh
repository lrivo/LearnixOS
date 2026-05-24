#!/bin/bash

rm -rf build/
meson setup build/ --cross-file cross_file.txt
