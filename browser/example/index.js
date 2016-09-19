/**
 * Created by cashbit on 17/09/16.
 */

var access_token = "" ;

var deviceId = "" ;


function login(){
    var username = document.getElementById('username').value ;
    var password = document.getElementById('password').value ;
    var url = "https://api.particle.io/oauth/token" ;
    
    var xhr = new XMLHttpRequest();
    xhr.open('POST', url);
    xhr.setRequestHeader('Content-Type', 'application/x-www-form-urlencoded');
    xhr.setRequestHeader("Authorization", "Basic " + btoa('particle' + ":" + 'particle'))
    xhr.onload = function() {
        addToConsole(xhr.response) ;
        var res = JSON.parse(xhr.response) ;
        access_token = res.access_token ;
        document.getElementById('access_token_field').value = access_token ;
    };
    xhr.send(encodeURI('grant_type=password&username=' + username + "&password=" + password));
}


function readDevices(){

    clearConsole();
    addToConsole("Reading devices...") ;

    var url = "https://api.particle.io/v1/devices/?access_token="+access_token ;

    var xhr = new XMLHttpRequest();
    xhr.open('GET', url);
    xhr.onload = function() {
        clearConsole();
        addToConsole("GET: " + url);
        addToConsole("") ;
        addToConsole("--------------- RESPONSE START ---------------") ;
        if (xhr.status === 200) {
            var obj = JSON.parse(xhr.response) ;
            addToConsole(obj) ;
            var x = document.getElementById("deviceSelect");
            while (x.options.length){
                x.options.remove(0);
            }
            var i = 0;
            obj.forEach(function(device){
                var option = document.createElement("option");
                option.value = device.id;
                option.text = device.name ;
                if (i==0) option.selected = true ;
                i++ ;
                x.add(option);
            });
            selectDevice();
        }
        else {
            addToConsole(xhr.responseText) ;
        }
        addToConsole("---------------- RESPONSE END ----------------") ;
    };
    xhr.send();
}

function selectDevice(){
    var x = document.getElementById("deviceSelect");
    var selected = x.options[x.selectedIndex] ;
    setDeviceId(x.value) ;
}

function readDeviceInfo(){

    clearConsole();
    addToConsole("Reading device info...") ;

    var url = "https://api.particle.io/v1/devices/"+deviceId+"?access_token="+access_token ;

    var xhr = new XMLHttpRequest();
    xhr.open('GET', url);
    xhr.onload = function() {
        clearConsole();
        addToConsole("GET: " + url);
        addToConsole("") ;
        addToConsole("--------------- RESPONSE START ---------------") ;
        if (xhr.status === 200) {
            var obj = JSON.parse(xhr.response) ;
            addToConsole(obj) ;
            var x = document.getElementById("variableSelect");
            while (x.options.length){
                x.options.remove(0);
            }
            var i=0 ;
            for (var k in obj.variables){
                var option = document.createElement("option");
                option.value = k;
                option.text = k ;
                if (i==0) option.selected = true ;
                i++ ;
                x.add(option);
            }
            selectVariable();

            x = document.getElementById("functionSelect");
            while (x.options.length){
                x.options.remove(0);
            }
            i = 0;
            obj.functions.forEach(function(functionname){
                var option = document.createElement("option");
                option.value = functionname;
                option.text = functionname ;
                if (i==0) option.selected = true ;
                i++ ;
                x.add(option);
            });

            selectFunction();

        }
        else {
            addToConsole(xhr.responseText) ;
        }
        addToConsole("---------------- RESPONSE END ----------------") ;
    };
    xhr.send();
}

function selectVariable(){
    var x = document.getElementById("variableSelect");
    var selected = x.options[x.selectedIndex] ;
    document.getElementById('variableNameField').value = x.value ;
}

function selectFunction(){
    var x = document.getElementById("functionSelect");
    var selected = x.options[x.selectedIndex] ;
    document.getElementById('functionNameInput').value = x.value ;
}

function readVariable(){

    var variableName = document.getElementById('variableNameField').value ;

    clearConsole();
    addToConsole("Reading variable: " + variableName + " ...") ;

    var url = "https://api.particle.io/v1/devices/"+deviceId+"/" + variableName + "?access_token="+access_token ;

    var xhr = new XMLHttpRequest();
    xhr.open('GET', url);
    xhr.onload = function() {
        clearConsole();
        addToConsole("GET: " + url);
        addToConsole("") ;
        addToConsole("--------------- RESPONSE START ---------------") ;
        if (xhr.status === 200) {
            var obj = JSON.parse(xhr.response) ;
            addToConsole(JSON.parse(obj.result)) ;
        }
        else {
            addToConsole(xhr.responseText) ;
        }
        addToConsole("---------------- RESPONSE END ----------------") ;
    };
    xhr.send();
}

function subscribeEvents(){
    // GET /v1/devices/{DEVICE_ID}/events
    if (deviceId) {
        var eventSourceUrl = "https://api.spark.io/v1/devices/" + deviceId + "/events/?access_token=" + access_token ;
    } else {
        var eventSourceUrl = "https://api.spark.io/v1/events/?access_token=" + access_token ;
    }

    var eventSource = new EventSource(eventSourceUrl);

    addToEvents('EVENTSOURCE URL: ' + eventSourceUrl) ;

    eventSource.addEventListener('open', function(e) {
        //addToEvents(e);
    },false);

    eventSource.addEventListener('error', function(e) {
        addToEvents(e);
    },false);

    var eventNames = document.getElementById('eventNamesInput').value ;
    if (eventNames) eventNames = eventNames.split(",") ;
    eventNames.forEach(function(eventName){
        eventSource.addEventListener(eventName.trim(), function(e) {
            addToEvents(e);
        },false);
    }) ;
}

function sendmessage() {

    var message = document.getElementById('messageInput').value ;
    var functionname = document.getElementById('functionNameInput').value ;

    var xhr = new XMLHttpRequest();
    var url = "https://api.particle.io/v1/devices/"+deviceId+"/" + functionname + "?access_token="+access_token ;

    clearConsole();
    addToConsole("POST: " + url);
    addToConsole("") ;
    addToConsole("POST DATA: " + 'args=' + message) ;
    addToConsole("") ;
    addToConsole("--------------- RESPONSE START ---------------") ;

    xhr.open('POST', url);
    xhr.setRequestHeader('Content-Type', 'application/x-www-form-urlencoded');
    xhr.onload = function() {
        addToConsole(xhr.response) ;
    };
    xhr.send(encodeURI('args=' + message));
}