#!/bin/bash
files=()
for f in $(find src/ -type f -name \*.h -o -name \*.cpp -o -name \*.hpp -o -name \*.ini)
do
    files+=("$f");
done
if [ -t 1 ]; then inpipe=no; else inpipe=yes; fi
egrep -Hn $(test "$inpipe" = "no" && echo -n --color=always) "${@}" ${files[@]}
