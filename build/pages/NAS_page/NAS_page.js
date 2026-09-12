
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

function setServerVersionInfo() {
    document.getElementById("versionField").innerText = serverVersionMajor.toString() + "." + serverVersionMinor.toString();
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

let current_path = "";
let parent_path = null;

async function GETNASLIST(path) {
    if(!logged_in) {
        console.error("Function should not be used if user is not logged in");
        window.location.href = "main_page.html";
        return null;
    }

    try {
        const response = await fetch("/NAS/LIST/" + path, {
            method: "POST",
            credentials: "include"
        });

        const data = await response.json();

        return data;
    } catch(e) {
        console.error(e);
        return null;
    }
}

function find_extension(filename) {
    const dot = filename.lastIndexOf('.');
    return dot === -1 ? '' : filename.slice(dot + 1);
}

function make_objects(response) {
    console.log(response);

    files = [];
    for(var i = 0, size = response.length; i < size; i++) {
        //przechodzimy po elementach odpowiedzi
        let ext = "";
        if(response[i].type == 0) {
            ext = "dir";
        }
        else {
            ext = find_extension(response[i].name);
        }

        mod_date = new Date(response[i].mtime * 1000);

        obj = {
            filename: response[i].name,
            extension: ext,
            size: response[i].size,
            mtime: mod_date,
            type: response[i].type
        }

        files.push(obj);

    }
    return files;
}

function createButtons(files) {
    const container = document.getElementById("NAS-window-inner");

    files.forEach(file => {
        const button = document.createElement("button");

        button.textContent = file.filename;

        if(file.type === 1) {
            button.classList.add("NAS-dir-button");
            //button.addEventListener("dblclick", changeDir);
        }
        else {
            button.classList.add("NAS-file-button");
            //button.addEventListener("dblclick", getFile);
        }

        container.appendChild(button);
    })

}


async function show_list(path) {
    const response = await GETNASLIST(path);
    //obsługa błędu jeśli response = null
    if(response == null) {
        console.error("response is null");
        //tutaj kod do obsługi erroru
        return null;
    }

    files = make_objects(response);
    console.log(files);
    createButtons(files);


}




setServerVersionInfo();
setLoginInfo();
//GETNASLIST(current_pathpath);
show_list(current_path);
