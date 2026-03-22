#!/bin/bash

DIR="/mnt/simple"

echo "Test fragmentacji i uszkodzenia entry"
echo "======================================"

# Wyczyść pliki
rm -f "$DIR/plik_1" "$DIR/plik_2" 2>/dev/null

# Krok 1: Wklej zawartość test.txt do plik_1 trzy razy
echo "1. Append test.txt do plik_1 (3 razy)..."
cat SimpleFS/test.txt >> "$DIR/plik_1"
cat SimpleFS/test.txt >> "$DIR/plik_1"
cat SimpleFS/test.txt >> "$DIR/plik_1"
SIZE_1=$(stat -c%s "$DIR/plik_1" 2>/dev/null || echo "ERROR")
echo "   Rozmiar plik_1: $SIZE_1 bajtów (powinno być ~11214)"

# Krok 2: Wklej zawartość test.txt do plik_2 raz
echo ""
echo "2. Append test.txt do plik_2 (1 raz)..."
cat SimpleFS/test.txt >> "$DIR/plik_2"
SIZE_2=$(stat -c%s "$DIR/plik_2" 2>/dev/null || echo "ERROR")
echo "   Rozmiar plik_2: $SIZE_2 bajtów (powinno być ~3738)"

# Krok 3: Wklej zawartość test.txt do plik_1 czwarty raz
echo ""
echo "3. Append test.txt do plik_1 (4 raz - powinno się fragmentować)..."
cat new_runs[run_idx].offset = curr_lcn;SimpleFS/test.txt >> "$DIR/plik_1"
SIZE_1=$(stat -c%s "$DIR/plik_1" 2>/dev/null || echo "ERROR")
echo "   Rozmiar plik_1: $SIZE_1 bajtów (powinno być ~14952)"

# Sprawdzenie czy plik_2 się uszkodził
echo ""
echo "4. Sprawdzenie spójności plików..."
SIZE_2_AFTER=$(stat -c%s "$DIR/plik_2" 2>/dev/null || echo "ERROR")
echo "   Rozmiar plik_2 po: $SIZE_2_AFTER bajtów (powinno być nadal ~3738, nie zmieniony)"

if [ "$SIZE_2_AFTER" != "$SIZE_2" ]; then
    echo "   ✗ BŁĄD: plik_2 się zmienił!"
else
    echo "   ✓ OK: plik_2 się nie zmienił"
fi

# Sprawdzenie first 8 bytes plik_2 (powinno być ENTR, nie BNFS)
echo ""
echo "5. Sprawdzenie magicu entry plik_2..."
HEXDUMP=$(hexdump -C "$DIR/plik_2" 2>/dev/null | head -1)
echo "   Hex: $HEXDUMP"
if [[ "$HEXDUMP" =~ "ENTR" ]]; then
    echo "   ✓ OK: Magic to ENTR"
elif [[ "$HEXDUMP" =~ "BNFS" ]]; then
    echo "   ✗ BŁĄD: Magic to BNFS (uszkodzona entry!)"
else
    echo "   ? UNKNOWN: Nieznany magic"
fi

echo ""
echo "==== Test zakończony ===="
