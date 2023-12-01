function addAlert(nomeEvento) {
    var divAlertas = document.querySelector('#sensor-vibracao .alertas');

    var now = new Date();

    divAlertas.innerHTML += `
        <div class="alert alert-warning">
            <span class="close-btn" onclick="this.parentElement.style.display='none';">&times;</span>
            <div class="alert-title">${nomeEvento}</div>
            <div class="alert-description">
                <span class="hora">
                    <strong>Hora: ${now.toLocaleTimeString()}</strong>
                </span>
                <br>
                <span class="data">
                    <strong>Data: ${now.toLocaleDateString()}</strong>
                </span>
            </div>
        </div>
    `;
}

var gateway = `ws://${window.location.hostname}/ws`;
var websocket;

window.addEventListener('load', onload);

// Inicia o websocket quando a página carrega
function onload(event) {
    initWebSocket();
}

function getReadings() {
    websocket.send("getReadings");
}

function initWebSocket() {
    console.log('Tentando abrir uma conexão WebSocket…');
    websocket = new WebSocket(gateway);
    websocket.onopen = onOpen;
    websocket.onclose = onClose;
    websocket.onmessage = onMessage;
}

// Quando o websocket for estabelecido, chame a função getReadings()
function onOpen(event) {
    console.log('Conexão aberta');
    getReadings();
}

function onClose(event) {
    console.log('Conexão fechada');
    setTimeout(initWebSocket, 2000);
}

// Função que recebe a mensagem do ESP32 com as leituras
function onMessage(event) {
    console.log(event.data);
    var myObj = JSON.parse(event.data);

    addAlert("Detecção de Vibração");

    // Insere o valor de cada sensor no seu card correspondente
    // for (var i = 0; i < keys.length; i++){
    //     var key = keys[i];
    //     document.getElementById(key).innerHTML = myObj[key];
    // }

    // Exemplo de Ocorrência
    // if (myObj["sinalPresenca"] == "1" && !alarme) {
    //     alarme = true
    //     ativarAlarme()
    // }
}