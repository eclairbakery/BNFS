#!/bin/bash
set -e

# ====== Konfiguracja ======
OUTPUT="simplefs"
BUILD_DIR="build"

CC="gcc"
CFLAGS="-Wall -Wextra -O2"
LDFLAGS=""

# ====== Tworzenie katalogu build ======
mkdir -p "$BUILD_DIR"

# ====== Szukanie plików .c ======
echo "== Szukanie plików .c =="

SRC_FILES=$(find . -type f -name "*.c")

if [ -z "$SRC_FILES" ]; then
    echo "Brak plików .c!"
    exit 1
fi

# ====== Kompilacja ======
echo
echo "== Kompilacja =="

OBJ_FILES=""

for file in $SRC_FILES; do
    # src/main.c -> build/src_main.o
    obj_name=$(echo "$file" | sed 's|^\./||; s|/|_|g; s|\.c$|.o|')
    obj_path="$BUILD_DIR/$obj_name"

    echo "Kompiluję: $file -> $obj_path"
    $CC $CFLAGS -c "$file" -o "$obj_path"

    OBJ_FILES="$OBJ_FILES $obj_path"
done

# ====== Linkowanie ======
echo
echo "== Linkowanie =="

OUT_PATH="$BUILD_DIR/$OUTPUT"

echo "Tworzę: $OUT_PATH"
$CC $OBJ_FILES -o "$OUT_PATH" $LDFLAGS

# ====== Auto-clean ======
echo
echo "== Czyszczenie plików .o =="

rm -f $OBJ_FILES

echo "Usunięto pliki obiektowe."

# ====== Done ======
echo
echo "Build zakończony sukcesem!"
echo "Wynik: ./$OUT_PATH"
