
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

let current_path = "/";
let parent_path = null;

const list_req = "/NAS/LIST";

async function GETNASLIST(path) {
    if(!logged_in) {
        console.error("Function should not be used if user is not logged in");
        window.location.href = "main_page.html";
        return null;
    }

    try {
        const response = await fetch(list_req.concat(path), {
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

function format_date(timestamp) {
    const date = new Date(timestamp);

    return(
        String(date.getDate()).padStart(2, '0') + '/' +
        String(date.getMonth() + 1).padStart(2, '0') + '/' + 
        date.getFullYear() + ' ' +
        String(date.getHours()).padStart(2, '0') + ':' + 
        String(date.getMinutes()).padStart(2, '0') + ':' +
        String(date.getSeconds()).padStart(2, '0')
    );
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

        mod_date = format_date(response[i].mtime * 1000);

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

        //tutaj trzeba wstawić całego diva z konkretną klasą w przycisk
        //ma być:
        //ikonka nazwa                            (ostatnia modyfikacja) (wielkość)

        const divFront = document.createElement('div');
        const divBack = document.createElement('div');
        divBack.className = 'NAS-button-back';
        divFront.className = 'NAS-button-front';

        
        //button.textContent = file.filename; to wstawiamy w diva
        divFront.textContent = file.filename;
        divBack.textContent = file.size + "B" + "        " + file.mtime;


        button.appendChild(divFront);
        button.appendChild(divBack);


        if(file.type === 1) {
            button.classList.add("NAS-dir-button");
            button.addEventListener("dblclick", function() {
                parent_path = current_path;
                current_path = current_path + file.filename + "/";
                console.log(current_path);
                setCurrentPathField(current_path);
                refresh_list(current_path);
            });
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

function clear_list() {
    const container = document.getElementById("NAS-window-inner");
    container.innerHTML = "";
}

async function refresh_list(path) {
    clear_list();
    await show_list(path);
}

async function backToParent() {
    if(parent_path == null) { //current "/", parent null
        return null;
    }
    //wszedzie poza przypadkiem powyżej gdzie nie możemy się już cofnąć robimy
    //current = parent a parent zmieniamy w zależności gdzie jesteśmy
    current_path = parent_path;

    if(parent_path == "/") {
        parent_path = null;
        await refresh_list(current_path);
        setCurrentPathField(current_path);
        return null;
    }

    parent_path = parent_path.slice(0, -1);
    const slash = parent_path.lastIndexOf('/');
    parent_path = parent_path.slice(0, slash);

    
    //przykład     current /admin/, parent "/"
    //             current /admin/dir1/, parent "/admin/"
    //             current admin/dir1/dir2 parent admin/dir1

    //jeśli znajdziemy / w parent to ustawiamy current = parent a 

    await refresh_list(current_path);
    setCurrentPathField(current_path);
}

function setCurrentPathField(path) {
    const field = document.getElementById("NAS-path-span");
    field.innerText = path;
}

setServerVersionInfo();
setLoginInfo();
setCurrentPathField(current_path);

refresh_list(current_path);
