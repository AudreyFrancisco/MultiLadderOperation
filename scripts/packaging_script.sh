#!/bin/bash

cd "$(git rev-parse --show-toplevel)"

git clean -x -f -d
git checkout .
git status > git_status.txt 2>&1
git log -1 > git_status.txt 2>&1
git diff   > git_status.txt 2>&1

zip ../new-alpide-software.zip -r . \
    -x \*.o -x \*.so -x test_\* -x startclk -x stopclk \
    -x analysis/\* -x GUI/\* -x .git\/* -x .gitignore \
    -x DataBaseSrc/\* -x exe/\*
