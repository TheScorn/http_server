# http_server
Repository for http server

Repo na część servera odpowiedzialną za http

Przebudowa do praw admina.
Nowe założenia do stron. Strony są albo ogólnodostępne(main, login) albo dostępne po zalogowaniu i mają treść dobraną do użytkownika (NAS). Specjalną kategeorią są strony dla admina (Status - testowa póki co). Tam użytkownik musi mieć flagę elevated.

TODO:
Poprawki
1. W sesji zapisywanie tylko user_id zamiast username - DONE
2. Komunikat o błędnym logowaniu wyświetlany na stronie
3. Dodanie komunikatów o błędach do funkcji obsługującej argumenty w main (obecna wersja jest okropnie leniwa) - DONE
4. Ulepszenie PPHP tak by nie korzystało z regexa (mniejsze zużycie pamięci).
5. Poprawki w regexach z POST tak żeby były bardziej konkretne.
6. Czasowe usuwanie starych tokenów. Ważne jeśli serwer miałby dłużej działać nierestartowany.

Plan na NAS
1. inicjalizacja danych serwera NAS (IP + port) - DONE
2. TEST w main - DONE
3. Przekazywanie danych NAS do wątków obsługujących użykownika (w strukturze sockaddr_in ???) razem z wynikiem TEST
(jeśli niepowodzenie to np. wyłączamy możliwość wejścia na podstronę NAS) - DONE
4. Obsługa wpisywania IP i portu NAS jako argumentów main. - DONE
5. Zaplanowanie schematu żądania które będą przekazywane do NAS - DONE

6. Funkcja żądania listy w js na stronie uruchamiana po odpaleniu strony.
7. Parsowanie żądania i wyciąganie z db potrzebnego info do użycia funkcji z NAS_API.
8. Łączenie się z NAS gdy wymaga tego żądanie.
9. Parsowanie odpowiedzi z NAS i zmienianie jej w format json.
10. Wysyłanie odpowiedzi na stronę.



Plan endpointów
Endpointy będą wysyłane z POST
Ścieżka będzie się zaczynać od NAS
Po NAS ścieżka będzie tłumaczyć komendę z jakiej chce korzystać.
Po komendzie podana będzie faktyczna ścieżka do pliku.
Przykład POST /NAS/LIST/thescorn
Albo POST /NAS/PUT/admin
