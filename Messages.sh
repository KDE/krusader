#!/bin/sh
dirs="krusader krArc tar iso"
$EXTRACTRC `find . -name \*.rc` >> rc.cpp || exit 11
$XGETTEXT `find $dirs -name \*.cpp -o -name \*.cc -o -name \*.c -o -name \*.h` -o $podir/krusader.pot
rm -f rc.cpp
