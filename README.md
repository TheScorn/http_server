# http_server
Repository for http server

Repo na część servera odpowiedzialną za http
Przeniesione zostaną tu wszyskie funkcjonalności z repo Server, które zostanie usunięte

Server będzie korzystał z mariadb do przechowywania danych użytkowników i info o każdej możliwej do wyświetlenia stronie.
Server będzie logował się do bazy poprzez konto 'http_server'
hasłem 'passwd' (sprawdzić opcje z hashowaniem).
Server http będzie miał jedną? bazę danych z dwoma? tabelmai.


Spostrzeżenie do dokmentacji:
    Pliki są wielokrotnie wysyłane tylko jeśli serwer nie odpowiada. Strona próbuje powtórzyć transmisję.
    Prawdopodobnie jest to wynik działania protokołów sieciowych.
