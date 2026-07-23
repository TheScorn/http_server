
//info o użytkowniku
let logged_in = false;

let username = null;
let user_email = null;

//wersja serwera
let serverVersionMajor = 0;
let serverVersionMinor = 0;

function setLoginInfo() {
    if(logged_in && username !== null && user_email !== null) {
        document.getElementById("buttonLogin").innerText = "Log out";
        document.getElementById("paragrahLoggedInAs").innerText = "Logged in as: " + username;
    }
    else if(!logged_in && username == null && user_email == null) {
        document.getElementById("buttonLogin").innerText = "Login";
        document.getElementById("paragrahLoggedInAs").innerText = "";
    }
}

function setServerVersionInfo() {
    document.getElementById("versionField").innerText = serverVersionMajor.toString() + "." + serverVersionMinor.toString();
}

function mainPageOnClick() {
    window.location.href = "main_page.html";
}

function loginButtonOnClick() {
    if(logged_in) {

        window.location.href = "main_page.html";
    } 
    else {
        window.location.href = "login_page.html";
    }
}

function statusButtonOnClick() {
    window.location.href = "status_page.html";
}

function sendLogOut() {
    return 0;
}

setLoginInfo();
setServerVersionInfo();