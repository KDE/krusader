#!/bin/sh
cd ..
dirs="app plugins/iso plugins/krarc"

# This is usually added by scripty but the fact that this Messages.sh file
# is hidden away in subdir that doesn't contain the actual code breaks scripty's heuristic
echo 'i18nc("NAME OF TRANSLATORS","Your names");' >> rc.cpp
echo 'i18nc("EMAIL OF TRANSLATORS","Your emails");' >> rc.cpp

$EXTRACTRC `find . -name \*.rc -o -name \*.ui` >> rc.cpp || exit 11
$EXTRACTRC --tag-group=none --tag=title --tag=category --tag=tooltip --tag=description app/useraction_examples.xml >> rc.cpp
$XGETTEXT rc.cpp `find $dirs -name \*.cpp -o -name \*.cc -o -name \*.c -o -name \*.h` -o $podir/krusader.pot
rm -f rc.cpp
