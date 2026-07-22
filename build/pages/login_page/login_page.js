let serverVersionMajor = 0; //to i tak tzeba zrobić preprocessorem więc użytkownika i opcje zrobimy tak samo
let serverVersionMinor = 0;


function setServerVersionInfo() {
    document.getElementById("versionField").innerText = serverVersionMajor.toString() + "." + serverVersionMinor.toString();
}

function mainPageOnClick() {
    window.location.href = "main_page.html";
}



function showLogiErrorText() {

}

const loginInfo = document.querySelector("#loginForm");

async function sendLoginInfo() {
    const loginInfoData = new FormData(loginInfo);

    try {
        const response = await fetch(window.location.origin, {
            method: "POST",
            body: loginInfoData,

        });

        if(!response.ok) {
            console.log("Login failed.");
            //logika do pokazywania wiadomości o niepoprawnym loginie lub haśle.
            return;
        }

        console.log("Logged in");

        
        ////////////////////////////////////////////////////////////
        //todo dynamiczny powrót do strony którą próbowano otworzyć
        window.location.href = "main_page.html";

    } catch (e) {
        
        console.error(e);
    }
    


} 


setServerVersionInfo();


