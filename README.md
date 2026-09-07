# http_server
Repository for http server

Repo na część servera odpowiedzialną za http

Przebudowa do praw admina.
Nowe założenia do stron. Strony są albo ogólnodostępne(main, login) albo dostępne po zalogowaniu i mają treść dobraną do użytkownika (NAS). Specjalną kategeorią są strony dla admina (Status - testowa póki co). Tam użytkownik musi mieć flagę elevated.

TODO:
1. Przebudowa bazy danych tak żeby użytkownicy zawierali flagę elevated + usunięcie access flags - będą zbędne.
2. Dodanie tabeli od użytkowników NAS. Każdy użytkownik http może mieć przypisane jedno konto użytkownika NAS.
3. Zmiana sprawdzania praw na stronach.
4. Dodanie strony NAS i podstawowa konfiguracja.
