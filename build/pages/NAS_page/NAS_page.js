
let logged_in = <<logged_in>>;

let username = <<username>>;
let user_email = null;

let serverVersionMajor = <<version_major>>;
let serverVersionMinor = <<version_minor>>;


function setLoginInfo() {
    if(logged_in && username !== null) {
        document.getElementById("paragrahLoggedInAs").innerText = "Logged in as: " + username;
    }
    else {
        //jeśli użytkownik nie jest zalogowany a jest na tej stronie
        console.error("Unjustified page loaded.");
        window.location.href = "main_page.html";
    }
}

function mainPageOnClick() {
    window.location.href = "main_page.html";
}

async function logoutButtonOnClick() {
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
        console.error("Unjustified page loaded.");
        window.location.href = "main_page.html";
    }
}


setServerVersionInfo();
setLoginInfo();

