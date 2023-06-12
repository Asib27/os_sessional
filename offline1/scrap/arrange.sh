#!/bin/bash

submission="$1"
target="$2"
test="$3"
answer="$4"

mkdir -p "$target/c" "$target/java" "$target/python"
result="$target/result.csv"

echo "student_id,type,matched,not_matched" > "$result"

noTest=$(ls "$test/" | wc -l)
echo $noTest

for zipname in "$submission"/*.zip ; do 
    roll=$( echo "$zipname" | cut -d '_' -f5 | cut -d '.' -f1 )
    echo $zipname $roll

    unzip "$zipname" -d extracted > /dev/null

    rightCnt=0
    wrongCnt=0

    if (( $(find -path '*extracted/*.c' | wc -l) != 0 )); then
        mkdir -p "$target/c/$roll";
        filename="$target/c/$roll/main.c";
        outfile="$target/c/$roll/main.out"
        find -name '*.c' -exec bash -c 'cp "$1" "$0"' $filename '{}' '+';

        gcc "$filename" -o "$outfile" ;

        for i in $(seq 1 $noTest) ; do 
            "./$outfile" < "$test/test$i.txt" > "$target/c/$roll/out$i.txt"
            
            if (( $(diff "$target/c/$roll/out$i.txt" "$answer/ans$i.txt" | wc -l) == 0 )) ; then
                rightCnt=$(( rightCnt + 1 ))
            else
                wrongCnt=$(( wrongCnt + 1 ))
            fi
        done

        echo "$roll,C,$rightCnt,$wrongCnt" >> "$result";
    fi

    if (( $(find -path '*extracted/*.java' | wc -l) != 0 )); then
        mkdir -p "$target/java/$roll";
        filename="$target/java/$roll/main.java";
        find -name '*.java' -exec bash -c 'cp "$1" "$0"' $filename '{}' '+';    

        javac "$filename"  

        for i in $(seq 1 $noTest) ; do 
            java -cp "$target/java/$roll" Main < "$test/test$i.txt" > "$target/java/$roll/out$i.txt"
            
            if (( $(diff "$target/java/$roll/out$i.txt" "$answer/ans$i.txt" | wc -l) == 0 )) ; then
                rightCnt=$(( rightCnt + 1 ))
            else
                wrongCnt=$(( wrongCnt + 1 ))
            fi
        done    

        echo "$roll,Java,$rightCnt,$wrongCnt" >> "$result";
    fi

    if (( $(find -path '*extracted/*.py' | wc -l) != 0 )); then
        mkdir -p "$target/python/$roll";
        filename="$target/python/$roll/main.py";
        find -name '*.py' -exec bash -c 'cp "$1" "$0"' $filename '{}' '+';

        chmod +x "$filename"

        for i in $(seq 1 $noTest) ; do 
            python3 "$filename"  < "$test/test$i.txt" > "$target/python/$roll/out$i.txt"
            
            if (( $(diff "$target/python/$roll/out$i.txt" "$answer/ans$i.txt" | wc -l) == 0 )) ; then
                rightCnt=$(( rightCnt + 1 ))
            else
                wrongCnt=$(( wrongCnt + 1 ))
            fi
        done   

        echo "$roll,Python,$rightCnt,$wrongCnt" >> "$result";
    fi

    rm -rf extracted/*
done

rmdir extracted;