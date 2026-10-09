# http_server
Repository for http server

Repo na część servera odpowiedzialną za http

Przebudowa do praw admina.
Nowe założenia do stron. Strony są albo ogólnodostępne(main, login) albo dostępne po zalogowaniu i mają treść dobraną do użytkownika (NAS). Specjalną kategeorią są strony dla admina (Status - testowa póki co). Tam użytkownik musi mieć flagę elevated.

TODO:
Poprawki
2. Komunikat o błędnym logowaniu wyświetlany na stronie
5. Poprawki w regexach z POST tak żeby były bardziej konkretne.
6. Czasowe usuwanie starych tokenów. Ważne jeśli serwer miałby dłużej działać nierestartowany.
7. W przypadku braku połączenia z NAS (error in connect) trzeba wysłaś jakieś info na stronę
8. Każdy error serwera NAS powinien być wyświetlany na stronie.
   Szczególnie errory wysyłane dosłownie przez NAS do http
10. Automatyczne przełączanie na stronę na którą użytkownik chciał wejść po zalogowaniu
11. Wysyłanie daty w odpowiedzi od serwa (Nie wszędzie jest a szkoda) SPRAWDZIĆ GDZIE BRAKUJE I DODAĆ
12. Graficzne zaznaczenie na stronie loginu po nieudanym logowaniu.
13. PUT
14. GET


Plan na NAS
1. inicjalizacja danych serwera NAS (IP + port) - DONE
2. TEST w main - DONE
3. Przekazywanie danych NAS do wątków obsługujących użykownika (w strukturze sockaddr_in ???) razem z wynikiem TEST
(jeśli niepowodzenie to np. wyłączamy możliwość wejścia na podstronę NAS) - DONE
4. Obsługa wpisywania IP i portu NAS jako argumentów main. - DONE
5. Zaplanowanie schematu żądania które będą przekazywane do NAS - DONE
6. Funkcja żądania listy w js na stronie uruchamiana po odpaleniu strony. - DONE
7. Parsowanie żądania i wyciąganie z db potrzebnego info do użycia funkcji z NAS_API. - DONE
8. Łączenie się z NAS gdy wymaga tego żądanie. - DONE
9. Parsowanie odpowiedzi z NAS i zmienianie jej w format json. - DONE
10. Wysyłanie odpowiedzi na stronę. - DONE

TERAZ napisać rotine dla każdego typu reqestu bazując na Cli
Dla GET i PUT nie piszamy gotowej funkcji. W trakcie odbierania,
w obu trzeba będzie co jakiś czas wysyłać jeśli nie chcemy zapisywać całego pliku. 

Plan endpointów:
Endpointy będą wysyłane z POST
Ścieżka będzie się zaczynać od NAS
Po NAS ścieżka będzie tłumaczyć komendę z jakiej chce korzystać.
Po komendzie podana będzie faktyczna ścieżka do pliku.
Przykład POST /NAS/LIST/thescorn
Albo POST /NAS/PUT/admin


Założenia dla logiki na stronie
Pliki mają własność name i typ
Funkcja show przyjmuje ścieżkę.

Jeśli kliknie się folder to do ścieżki jest doklejana nazwa folderu i odpalana jest znowu funkcja show

Ścieżka powinna się zmieniać dynamicznie.

Obiekty do pokazywania musimy zrobić oddzielnie tak żeby mogły zawierać też info o rozszerzeniu

Podczas show powinniśmy też stworzyć sztuczny obiekt rodzica przez którego będzie można wrócić wyżej (jako jakiś przycisk back)

