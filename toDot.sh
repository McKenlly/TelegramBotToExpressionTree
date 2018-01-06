for file in `ls ./*.dot`; do
    ans_file="${file%.*}"
    dot -Tpng  $file  -o "./Picture/${ans_file}.png"
    rm $file
done   
