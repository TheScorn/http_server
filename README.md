# http_server
Repository for http server

Repo na część servera odpowiedzialną za http

Przebudowa do praw admina.
Nowe założenia do stron. Strony są albo ogólnodostępne(main, login) albo dostępne po zalogowaniu i mają treść dobraną do użytkownika (NAS). Specjalną kategeorią są strony dla admina (Status - testowa póki co). Tam użytkownik musi mieć flagę elevated.

TODO:
Poprawki
1. W sesji zapisywanie tylko user_id zamiast username - DONE
2. Komunikat o błędnym logowaniu wyświetlany na stronie

Plan na NAS
1. inicjalizacja danych serwera NAS (IP + port) - DONE
2. TEST w main - DONE
2. Przekazywanie danych NAS do wątków obsługujących użykownika (w strukturze sockaddr_in ???) razem z wynikiem TEST
(jeśli niepowodzenie to np. wyłączamy możliwość wejścia na podstronę NAS)
3. Obsługa wpisywania IP i portu NAS jako argumentów main.
4. Zaplanowanie schematu żądania które będą przekazywane do NAS
5. Łączenie się z NAS gdy wymaga tego żądanie.

