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
14. Czemu czasem psuje się LIST. Jeśli folder jest pusty trzeba lepiej obsłużyć ten null.
15. Czasem nie wstawia się w ścieżkę "/". Sprawdzić czemu - sprawdzić kolejność zmieniania ścieżki (może jakaś ścieżka tymczasowa i przeniesienie zmiany ścieżki na start) (może da się jakoś blokować ścieżkę w stylu mutexów).
19. Czyszczenie malloców i free w login i logout
20. Czy na początku HTTP GET kopiowanie buffora do page_buffer jest potrzebne??
21. zabijanie procesów fetch na stronie.

22. Szyfrowana baza danych
23. Szyfrowane połączenie z NAS by TLS


13. PUT !!!!!!!!!!!!!!!!!!!
Działa dla małego pliku, zepsuło się dla pdf-a
znaleźć przyczynę


Testy:
1. Testy jednostkowe dla prostych funkcji
a) build_http_response
b) helper_functions

2. Testy działającego serwera 
Jak zbudować sensowną procedurę testowania?? 