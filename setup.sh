#!/bin/bash

rm -rf target/
meson setup target --cross-file cross_file.txt
