
var gateway = `ws://${window.location.hostname}/ws`;
var websocket;

// default units sent via websocket
var units = "Fahrenheit";

// Init web socket when the page loads
window.addEventListener('load', onload);

// Toggle ternary for units
function toggleUnits(units) {
    var something = document.getElementById('something');

    something.style.cursor = 'pointer';
    something.onclick = function() {
        // do something...
    };
}


function onload(event) {
    initWebSocket();
}

function getReadings(){
    websocket.send("getReadings");
}

function initWebSocket() {
    console.log('Trying to open a WebSocket connection…');
    websocket = new WebSocket(gateway);
    websocket.onopen = onOpen;
    websocket.onclose = onClose;
    websocket.onmessage = onMessage;
}

// When websocket is established, call the getReadings() function
function onOpen(event) {
    console.log('Connection opened');
    getReadings();
}

function onClose(event) {
    console.log('Connection closed');
    setTimeout(initWebSocket, 2000);
}

// Function that receives the message from the ESP32 with the readings
function onMessage(event) {
    var temperatureObj = JSON.parse(event.data);
    var temp = Object.values(temperatureObj)[0];

    if (temp > 200) {
        document.getElementById("temperature").innerHTML = "EEEEE Whoohoo! Bacterial death! I'm over 392F";
        document.getElementsByTagName("title")[0].innerHTML = "I'm really hot! OwO -Turkey Probe";
    } else if (temp < 4) {
        document.getElementById("temperature").innerHTML = "i too cold to measure accurately OwO";
        document.getElementsByTagName("title")[0].innerHTML = "I'm too cold :/ -Turkey Probe";
    // Using Celsius?
    } else if (units == "Celsius") { 
        document.getElementsByTagName("title")[0].innerHTML = temp.toString() + '&deg;C ';
        document.getElementById("temperature").innerHTML = temp.toString() + '&deg;C Probe';
    // Default to F:
    } else {
        document.getElementsByTagName("title")[0].innerHTML = (temp * 9/5 + 32).toString() + '&deg;F Probe'
        document.getElementById("temperature").innerHTML = (temp * 9/5 + 32).toString() + '&deg;F Probe'; 
    }
}