#!/bin/bash

set -e

make

if [[ -z "$1" ]]; then
  echo "Which test lib?"
  exit 1
fi

for f in $(ls ${1}*.txt); do
  echo -ne "$f\t"
  ./facility_location/runner $f
done > "res-$(git rev-parse --short HEAD)-$(basename $1).txt"
