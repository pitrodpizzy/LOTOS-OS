#!/bin/bash

# 1. Dodaj wszystkie pliki i zmiany do kolejki
git add .

# 2. Zapytaj użytkownika o opis zmian
echo "=== Wrzucanie zmian na GitHub ==="
echo "Wpisz opis zmian (nazwę commita) i naciśnij ENTER:"
read commit_message

# 3. Jeśli użytkownik nic nie wpisał, ustaw domyślny opis
if [ -z "$commit_message" ]; then
    commit_message="Aktualizacja kodu LOTOS OS"
fi

# 4. Zatwierdź zmiany w pamięci Git
git commit -m "$commit_message"

# 5. Wyślij pliki na serwer do gałęzi main
echo ""
echo "Wysyłam pliki na GitHub..."
git push origin main

echo ""
echo "=== Gotowe! ==="
