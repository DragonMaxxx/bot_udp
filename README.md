# UDP BOT – termometr cyfrowy (C)

Prosta aplikacja konsolowa w języku C, która symuluje termometr cyfrowy i cyklicznie wysyła dane UDP do wskazanego serwera. Dodatkowo zapisuje logi w formacie tekstowym.

## Parametry uruchomienia

./udp_bot <IP> <PORT> <CYKL_WYSLANIA> <CYKL_LOGOWANIA> <ŚCIEŻKA_LOGU>


Przykład:

./udp_bot 127.0.0.1 5000 3 10 ./log.txt


## Przesyłane dane

Każdy pakiet ma postać struktury binarnej:

| Pole           | Typ        | Opis                                      |
|----------------|------------|-------------------------------------------|
| timestamp      | uint32_t   | Znacznik czasowy (UNIX)                   |
| temperature    | uint16_t   | Temperatura × 10 (dokładność 0.1°C)       |
| power_status   | uint8_t    | 0 – zasilanie sieciowe, 1 – bateryjne     |
| measurement_id | uint8_t    | Kolejny identyfikator pomiaru             |
| checksum       | uint8_t    | Prosta suma kontrolna bajtów              |

## Logowanie

Zapisuje dane cyklicznie do pliku w formacie:

Czas: 1716059241, Temp: 36.5°C, Zasilanie: zasilanie bateryjne, ID: 42, CRC: 123


## Testowanie

- Uruchom serwer UDP do testów, np. `gcc -o udp_bot udp_bot.c'
- Otwórz logi (`tail -f log.txt`), aby podglądać wpisy

