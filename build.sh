dest="bin"

debug=""

read -p "is it for 32-bit architecture?(Y/n): " $debug

mkdir $dest

set -e

if [[ $debug == "Y" ]]; then
    g++ -m32 -std=c++17 "GraphEditor/GraphEditor.cpp" -o "$dest/GraphEditor"
else
    g++ -std=c++17 "GraphEditor/GraphEditor.cpp" -o "$dest/GraphEditor"
fi

python_files=$(find GraphEditor -type f -name "*.py")

for file in $python_files; do
    filename=$(basename "$file")
    cp "$file" "$dest/$filename"
done

echo "done"
