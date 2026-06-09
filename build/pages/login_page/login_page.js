let serverVersionMajor = 0
let serverVersionMinor = 0

function setServerVersionInfo() {
    document.getElementById("versionField").innerText = serverVersionMajor.toString() + "." + serverVersionMinor.toString();
}

setServerVersionInfo();


