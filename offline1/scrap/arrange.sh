name=$(ls 'Steven Rogers_1000007_assignsubmission_file_1805127.zip')
roll=$(ls "$name" | cut -d '_' -f5 | cut -d '.' -f1)
unzip "$name" -d  extracted
mkdir "$roll"
find -name '*.c' -exec bash -c 'cp "$1" "./$0/$0.c"' $roll  '{}' '+'