#!/bin/sh

set -e

gcc -Wall -D_TEST_ bmpfile.c barcode.c -o barcode
