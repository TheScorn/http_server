
//info o użytkowniku
let logged_in = <<logged_in>>;

let username = <<username>>;
let user_email = null;

//wersja serwera
let serverVersionMajor = <<version_major>>;
let serverVersionMinor = <<version_minor>>;


function setServerVersionInfo() {
    document.getElementById("versionField").innerText = serverVersionMajor.toString() + "." + serverVersionMinor.toString();
}

function mainPageOnClick() {
    window.location.href = "main_page.html";
}


setServerVersionInfo();