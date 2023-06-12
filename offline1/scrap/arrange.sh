# name=$(ls 'Steven Rogers_1000007_assignsubmission_file_1805127.zip')
# roll=$(ls "$name" | cut -d '_' -f5 | cut -d '.' -f1)
# unzip "$name" -d  extracted
# mkdir "$roll"
# find -name '*.c' -exec bash -c 'cp "$1" "./$0/$0.c"' $roll  '{}' '+'

#!/bin/bash

submission="$1"
target="$2"
test="$3"
answer="$4"

mkdir -p "$target/c" "$target/java" "$target/python"
touch "$target/result.csv"

# if (( $(find -name '*.csv' | wc -l) != 0 )); then
#     echo "exists";
# else
#     echo "DOES NOT";
# fi

for zipname in "$submission"/*.zip ; do 
    roll=$( echo "$zipname" | cut -d '_' -f5 | cut -d '.' -f1 )
    echo $zipname $roll

    unzip "$zipname" -d extracted > /dev/null

    if (( $(find -path '*extracted/*.c' | wc -l) != 0 )); then
      mkdir -p "$target/c/$roll";
      filename="$target/c/$roll/$roll.c";
      find -name '*.c' -exec bash -c 'cp "$1" "$0"' $filename '{}' '+';
    fi

    if (( $(find -path '*extracted/*.java' | wc -l) != 0 )); then
      mkdir -p "$target/java/$roll";
      filename="$target/java/$roll/$roll.java";
      find -name '*.java' -exec bash -c 'cp "$1" "$0"' $filename '{}' '+';    fi

    if (( $(find -path '*extracted/*.py' | wc -l) != 0 )); then
      mkdir -p "$target/python/$roll";
      filename="$target/python/$roll/$roll.py";
      find -name '*.py' -exec bash -c 'cp "$1" "$0"' $filename '{}' '+';
    fi

    rm -rf extracted/*
done

rmdir extracted;