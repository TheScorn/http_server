
//info o użytkowniku
let logged_in = <<logged_in>>;

let username = <<username>>;
let user_email = null;

//wersja serwera
let serverVersionMajor = <<version_major>>;
let serverVersionMinor = <<version_minor>>;

function setLoginInfo() {
    if(logged_in && username !== null) {
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

async function loginButtonOnClick() {
    if(logged_in) {
        try {
            const response = await fetch("/logout", {
                method: "POST",
                credentials: "include"
            });
            window.location.href = "main_page.html";

        } catch(e) {
            console.error(e);
        }
    } 
    else {
        window.location.href = "login_page.html";
    }
}

function statusButtonOnClick() {
    window.location.href = "status_page.html";
}

async function sendLogOut() {
    try {
        const response = await fetch("/logout", {
        method: "POST",
        credentials: "include"    
        });

        

    } catch(e) {
        console.error(e);
    }

    
    
}

setLoginInfo();
setServerVersionInfo();