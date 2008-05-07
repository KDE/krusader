#!/bin/bash

VERSION=`cat CMakeLists.txt | grep add_definitions | grep 'DVERSION' | awk -F '"' '{print $2; }'`
DIST_NAME="krusader-kde4-${VERSION}"
OUTPUT_FILE_NAME="${DIST_NAME}.tar.gz"

rm -rf __KrDist__
mkdir __KrDist__
mkdir __KrDist__/$DIST_NAME

echo 'SVN listing...'
svn status -v >__KrDist__/svnfiles
FILES=`cat __KrDist__/svnfiles | grep -v '^\? ' | awk '{print $NF;}'`

echo 'Creating directory structure...'
for FILE in $FILES;
do
   if [ $FILE != "." ]; then
     if [ -d $FILE ]; then
        mkdir __KrDist__/$DIST_NAME/$FILE;
     fi
   fi
done

echo 'Copying files for disting...'
for FILE2 in $FILES;
do
   if [ ! -d $FILE2 ]; then
     cp --preserve=all $FILE2 __KrDist__/$DIST_NAME/$FILE2;
   fi
done

echo 'Creating the tar.gz archive...'
cd __KrDist__
tar cvfz ../$OUTPUT_FILE_NAME $DIST_NAME >/dev/null
cd ..

echo 'Removing temporary files...'
rm -rf __KrDist__

echo 'OK.'
