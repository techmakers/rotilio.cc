/**
 * Created by cashbit on 17/09/16.
 */

function getConsoleElement(){
    return document.getElementById('console') ;
}

function getEventElement(){
    return document.getElementById('events') ;
}

function clearConsole(){
    getConsoleElement().innerText = "";
}

function addToConsole(value){
    if (typeof value != "string") value = JSON.stringify(value,null,4) ;
    var c = getConsoleElement() ;
    var innerText = c.value ? c.value + "\n" : "" ;
    innerText += value ;
    c.innerText = innerText ;
    c.scrollTop = c.scrollHeight;
}

function addToEvents(message){
    var c = getEventElement() ;
    var innerText = c.value ? c.value + "\n" : "" ;
    if (typeof message == "string") {
        innerText += message ;
    } else {
        innerText += "\n--------------- EVENT START ---------------\n" ;
        innerText += "TYPE: " + message.type ;
        if (message.data){
            innerText += "\n---------------\n" ;
            innerText += JSON.stringify(JSON.parse(message.data),null,4);
        }
        innerText += "\n--------------- EVENT END  ---------------" ;
    }
    c.innerText = innerText ;
    c.scrollTop = c.scrollHeight;
}


function setDeviceId(id){
    deviceId = id ;
    var deviceIdElement = document.getElementById('deviceIdInput') ;
    deviceIdElement.value = id ;
}

function updateDeviceId(){
    var deviceIdElement = document.getElementById('deviceIdInput') ;
    deviceId = deviceIdElement.value ;
}