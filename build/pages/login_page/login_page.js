let serverVersionMajor = 0;
let serverVersionMinor = 0;


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
        const response = await fetch(window.location.origin, {
            method: "POST",
            body: loginInfoData,

        });

        console.log(await response.json());


    } catch (e) {
        console.error(e);
    }
    


} 


setServerVersionInfo();


