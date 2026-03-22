#!/usr/bin/env -S bash --posix

set -e

shopt -s globstar

# preparation
OUTPUT="simplefs"
BUILD_DIR="build"

CC="gcc"
CFLAGS="-Wall -Wextra -O2 -g `pkg-config fuse3 --cflags --libs` -D_FILE_OFFSET_BITS=64"
LDFLAGS="-g `pkg-config fuse3 --cflags --libs` -D_FILE_OFFSET_BITS=64"

mkdir -p "$BUILD_DIR"

echo "== Searching for sources =="

SRC_FILES=$(find "fuse" -type f -name "*.c")

if [ -z "$SRC_FILES" ]; then
    echo "no c files"
    exit 1
fi

# compiling
echo
echo "== Compiling =="

OBJ_FILES=""

for file in $SRC_FILES; do
    obj_name=$(echo "$file" | sed 's|^\./||; s|/|_|g; s|\.c$|.o|')
    obj_path="$BUILD_DIR/$obj_name"

    echo "Compiling: $file -> $obj_path"
    $CC $CFLAGS -c "$file" -o "$obj_path"

    OBJ_FILES="$OBJ_FILES $obj_path"
done

# linking
echo
echo "== Linking =="

OUT_PATH="$BUILD_DIR/$OUTPUT"

echo "Creating: $OUT_PATH"
$CC $OBJ_FILES -o "$OUT_PATH" $LDFLAGS

# cleanup
echo
echo "== Cleaning up =="

rm -f $OBJ_FILES

echo "Object files were removed"

# done
echo
echo -e "\033[32mSuccess!\033[0m"
echo "Output binary: ./$OUT_PATH"
