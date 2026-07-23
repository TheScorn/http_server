
//info o użytkowniku
let logged_in = false;

let username = null;
let user_email = null;

//wersja serwera
let serverVersionMajor = 0;
let serverVersionMinor = 0;


function setServerVersionInfo() {
    document.getElementById("versionField").innerText = serverVersionMajor.toString() + "." + serverVersionMinor.toString();
}

function mainPageOnClick() {
    window.location.href = "main_page.html";
}


setServerVersionInfo();