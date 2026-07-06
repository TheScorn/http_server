let serverVersionMajor = 0;
let serverVersionMinor = 0;

let authServerIP = "192.168.0.129";
let authServerPort = "54003";

function setServerVersionInfo() {
    document.getElementById("versionField").innerText = serverVersionMajor.toString() + "." + serverVersionMinor.toString();
}

function mainPageOnClick() {
    window.location.href = "main_page.html";
}


const loginInfo = document.querySelector("#loginForm");

async function sendLoginInfo() {
    
    const loginInfoData = new FormData(loginInfo);

    try {
        const response = await fetch("http://" + authServerIP + ":" + authServerPort, {
            method: "POST",
            body: loginInfoData,

        });

        console.log(await response.json());


    } catch (e) {
        console.error(e);
    }
    


} 


setServerVersionInfo();


