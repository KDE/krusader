#!/bin/bash

# When you do a merge from the stable branch to the master branch,
# you may see a lot of conflicts and changes in localization-enabled files,
# such as in 'po' dir and .desktop, .appdata.xml files.
# This happens since the branches have separate localization injection streams.
# The script helps to restore these files to the state before the merge,
# with the exception to .appdata.xml that often contains version changes.


files_with_status()
{
    git status -s | grep '^'"$1" | awk '{ print $2; }'
}

with_translations()
{
    grep -P '(^po/)|(^app/.*\.desktop)'
}

restore_files()
{
    while read f; do
        git restore --source=HEAD --staged --worktree "$f"
    done
}


files_with_status '' | with_translations | restore_files
