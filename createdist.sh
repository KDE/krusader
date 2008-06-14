#!/bin/bash

VERSION=`cat CMakeLists.txt | grep add_definitions | grep 'DVERSION' | awk -F '"' '{print $2; }'`
DIST_NAME="krusader-${VERSION}"
OUTPUT_FILE_NAME="${DIST_NAME}.tar.gz"

rm -rf __KrDist__
mkdir __KrDist__
mkdir __KrDist__/$DIST_NAME

echo 'try SVN listing...'
if svn status -v >__KrDist__/svnfiles; then
  FILES=`cat __KrDist__/svnfiles | grep -v '^\? ' | awk '{print $NF;}'`
fi

if [ -z "$FILES" ]; then
  echo 'try git listing...'
  if git ls-files >__KrDist__/gitfiles; then
    FILES=`cat __KrDist__/gitfiles`
  fi
fi

if [ -z "$FILES" ]; then
  echo "Something went wrong! Can't find version controlled source files"
  exit 1
fi

echo 'Copying files for disting...'
for FILE2 in $FILES;
do
   if [ ! -d $FILE2 ]; then
     target_dir="__KrDist__/$DIST_NAME/`dirname $FILE2`"
     if [ ! -d $target_dir ]; then mkdir -p $target_dir; fi
     cp -p $FILE2 $target_dir;
   fi
done

echo 'Creating the tar.gz archive...'
cd __KrDist__
tar cvfz ../$OUTPUT_FILE_NAME $DIST_NAME >/dev/null
cd ..

echo 'Removing temporary files...'
rm -rf __KrDist__

echo 'OK.'
