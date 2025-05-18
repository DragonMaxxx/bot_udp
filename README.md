v# UDP BOT – termometr cyfrowy (C)

Prosta aplikacja konsolowa w języku C, która symuluje termometr cyfrowy i cyklicznie wysyła dane UDP do wskazanego serwera.

## Parametry uruchomienia

./udp_bot <IP> <PORT> <CYKL_WYSLANIA> <CYKL_LOGOWANIA> <ŚCIEŻKA_LOGU>

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

Czas: 2025-05-18 23:28:40, Temp: 101.8°C, Zasilanie: zasilanie bateryjne, ID: 94, CRC: 71

## Testowanie

- Skompiluj program `gcc -o udp_bot udp_bot.c`
- Uruchom serwer UDP do testów, np. `nc -lu 5000`
- W osobnym terminalu uruchom aplikację np.`./udp_bot 127.0.0.1 5000 3 10 ./log.txt`
- Otwórz logi (`tail -f log.txt`)

