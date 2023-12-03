#include <Arduino.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/semphr.h>
#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <Arduino_JSON.h>

#define BOTAO_PINO 2
#define VIBRACAO_PINO 15
#define VIBRACAO_RESERVA_PINO 12
#define TEMPERATURA_PINO 37
#define RUIDO_PINO 32
#define BUZZER_PINO 38

SemaphoreHandle_t statusMutex;
int statusBotao = 0;
int statusVibracao = 0;
int statusTemperatura = 0;
int statusRuido = 0;
int statusRelogio = 0;
int statusBuzzer = 0;
int ultimaLeituraBotao = 1;
int ultimaLeituraVibracao = 0;
int debounceDelay = 50;
const int limiteRuido = 240;
const int tempoLimite = 10;
bool buzzerAtivado = false;
bool relogioAtivado = false;
String ultimoResultado = " ";

// WEBSocket
// Cria um objeto AsyncWebServer na porta 80
AsyncWebServer server(80);

// Cria um objeto WebSocket
AsyncWebSocket ws("/ws");

// Credenciais da Internet
const char *ssid = "ContruSensor STR";
const char *password = "jspa9096";

char index_html[] PROGMEM = R"rawliteral(
  <!DOCTYPE html>
  <html>
  <head>
    <title>ConstruSensor</title>
    <meta name="viewport" content="width=device-width, initial-scale=1">
    <style>
        * {padding: 0; margin: 0; box-sizing: border-box;}
        html {font-family: Arial, Helvetica, sans-serif; display: inline-block;}

        body {margin: 0;}

        h1 {font-size: 1.8rem; color: white;}

        .topnav {height: 10vh; display: flex; flex-direction: column; justify-content: center; align-items: center; background-color: #0A1128;}

        .content {height: 90vh; padding: 30px;}

        .content section {margin-bottom: 50px;}

        .content section h2 {margin-bottom: 10px;}

        .content section .alertas {margin-top: 50px;}

        .alert {
            padding: 20px;
            border-radius: 4px;
            border: 1px solid #ddd;
            background-color: #f8d7da;
            color: #721c24;
            position: relative;
            margin-bottom: 20px;
        }

        .alert:last-child {margin-bottom: 0px;}

        .alert-warning {border-color: #f5c6cb;}

        .close-btn {position: absolute; top: 10px; right: 10px; cursor: pointer; font-weight: bold;}

        .alert-title {font-size: 1.2em; font-weight: bold; margin-bottom: 10px;}

        .alert-description {font-size: 1em;}
    </style>
  </head>
  <body>
    <div class="topnav">
        <h1>ALERTAS - SENSORES (WEBSOCKET)</h1>
    </div>
    <div class="content">
        <section id="sensor-ruido">
            <h2>Sensor de Ruido</h2>
            <hr>
            <div class="alertas"></div>
        </section>

        <section id="sensor-temperatura">
            <h2>Sensor de Temperatura</h2>
            <hr>
            <div class="alertas"></div>
        </section>

        <section id="sensor-vibracao">
            <h2>Sensor de Vibracao</h2>
            <hr>
            <div class="alertas"></div>
        </section>

        <section id="sensor-poeira">
            <h2>Sensor de Poeira</h2>
            <hr>
            <div class="alertas"></div>
        </section>
    </div>

    <script>
        function addAlert(nomeEvento, tipoSensor) {
            var divAlertas;

            if(tipoSensor == "ruido") {
                divAlertas = document.querySelector('#sensor-ruido .alertas');
            }
            if(tipoSensor == "temperatura") {
                divAlertas = document.querySelector('#sensor-temperatura .alertas');
            }
            if(tipoSensor == "vibracao") {
                divAlertas = document.querySelector('#sensor-vibracao .alertas');
            }
            if(tipoSensor == "poeira") {
                divAlertas = document.querySelector('#sensor-poeira .alertas');
            }

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
          iniciarWebSocket();
        }

        function getReadings() {
          websocket.send("getReadings");
        }

        function iniciarWebSocket() {
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
          setTimeout(iniciarWebSocket, 2000);
        }

        // Função que recebe a mensagem do ESP32 com as leituras
        function onMessage(event) {
            console.log(event.data);
            var myObj = JSON.parse(event.data);

            if(myObj["status_ruido"] == "1") {
              addAlert("Deteccao de Ruido", "ruido");
            }

            if(myObj["status_temperatura"] == "1") {
              addAlert("Deteccao de Temperatura", "temperatura");
            }

            if(myObj["status_vibracao"] == "1") {
              addAlert("Deteccao de Vibracao", "vibracao");
            }

            if(myObj["status_poeira"] == "1") {
              addAlert("Deteccao de Poeira", "poeira");
            }
        }
    </script>
  </body>
</html>
)rawliteral";

// Tarefas WebSocket - Início

// Initialize WiFi
void iniciarWifi(){
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.print("Conectando ao WiFi..");
  while (WiFi.status() != WL_CONNECTED){
    Serial.print('.');
    delay(1000);
  }
  Serial.println(WiFi.localIP());
}

void notificarClientes(String sensorReadings){
  ws.textAll(sensorReadings);
}

void handleWebSocketMessage(void *arg, uint8_t *data, size_t len){
  AwsFrameInfo *info = (AwsFrameInfo *)arg;
  if(info->final && info->index == 0 && info->len == len && info->opcode == WS_TEXT){
    data[len] = 0;
    String message = (char *)data;
    // Verifica se a mensagem é "getReadings"
    if(strcmp((char *)data, "getReadings") == 0){
      JSONVar statusSensores;

      statusSensores["status_ruido"] = statusRuido;
      statusSensores["status_temperatura"] = statusTemperatura;
      statusSensores["status_vibracao"] = statusVibracao;
      statusSensores["status_poeira"] = statusBotao;

      // Se for, envie as leituras atuais do sensor
      String sensorReadings = JSON.stringify(statusSensores);
      Serial.println(sensorReadings);
      notificarClientes(sensorReadings);
    }
  }
}

void onEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len){
  switch (type){
  case WS_EVT_CONNECT:
    Serial.printf("WebSocket client #%u connected from %s\n", client->id(), client->remoteIP().toString().c_str());
    break;
  case WS_EVT_DISCONNECT:
    Serial.printf("WebSocket client #%u disconnected\n", client->id());
    break;
  case WS_EVT_DATA:
    handleWebSocketMessage(arg, data, len);
    break;
  case WS_EVT_PONG:
  case WS_EVT_ERROR:
    break;
  }
}

void iniciarWebSocket(){
  ws.onEvent(onEvent);
  server.addHandler(&ws);
}

// Tarefas WebSocket - Fim

void tarefaBotao(void *pvParameters){
  pinMode(BOTAO_PINO, INPUT_PULLUP);

  TickType_t xLastWakeTime = xTaskGetTickCount();

  while(1){
    int botaoPressionado = digitalRead(BOTAO_PINO);

    if(botaoPressionado != ultimaLeituraBotao){
      vTaskDelay(debounceDelay / portTICK_PERIOD_MS);
      botaoPressionado = digitalRead(BOTAO_PINO);

      if(botaoPressionado == LOW){
        xSemaphoreTake(statusMutex, portMAX_DELAY);
        statusBotao = 1;
        xSemaphoreGive(statusMutex);
      }
    }

    ultimaLeituraBotao = botaoPressionado;
    vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(2000));
  }
}

void tarefaVibracao(void *pvParameters){
  pinMode(VIBRACAO_PINO, INPUT);

  TickType_t xLastWakeTime = xTaskGetTickCount();

  while(1){
    int sensorVibracao = digitalRead(VIBRACAO_PINO);

    if(sensorVibracao != ultimaLeituraVibracao){
      vTaskDelay(debounceDelay / portTICK_PERIOD_MS);
      sensorVibracao = digitalRead(VIBRACAO_PINO);

      if(sensorVibracao == HIGH){
        xSemaphoreTake(statusMutex, portMAX_DELAY);
        statusVibracao = 1;
        xSemaphoreGive(statusMutex);
      }
    }

    ultimaLeituraVibracao = sensorVibracao;
    vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(2000));
  }
}

void tarefaTemperatura(void *pvParameters){
  TickType_t xLastWakeTime = xTaskGetTickCount();

  while (1){
    float temperatura = analogRead(TEMPERATURA_PINO) * (3.3 / 4095) * 100;

    if (temperatura > 15.0){
      xSemaphoreTake(statusMutex, portMAX_DELAY);
      statusTemperatura = 1;
      xSemaphoreGive(statusMutex);
    }

    vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(10000));
  }
}

void tarefaRuido(void *pvParameters){
  TickType_t xLastWakeTime = xTaskGetTickCount();

  int contadorLimite = 0;
  const int periodoLeitura = 10000;
  const int numLeituras = tempoLimite * 1000 / periodoLeitura;

  while(1){
    float nivelRuido = analogRead(RUIDO_PINO);

    if(nivelRuido > limiteRuido){
      contadorLimite++;

      if(contadorLimite >= numLeituras){
        xSemaphoreTake(statusMutex, portMAX_DELAY);
        statusRuido = 1;
        xSemaphoreGive(statusMutex);
        contadorLimite = 0;
      }
    } else {
      contadorLimite = 0;
    }

    vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(periodoLeitura));
  }
}

void tarefaRelogio(void *pvParameters){
  while (1){
    xSemaphoreTake(statusMutex, portMAX_DELAY);
    if (statusRuido == 1 && !relogioAtivado){
      Serial.println("Relógio ativado aqui!");
      statusRelogio = 1;
      relogioAtivado = true;
    }
    xSemaphoreGive(statusMutex);

    vTaskDelay(100 / portTICK_PERIOD_MS);
  }
}

void tarefaBuzzer(void *pvParameters){
  pinMode(BUZZER_PINO, OUTPUT);

  // Configuração do canal LEDC para o buzzer
  ledcSetup(0, 3000, 8);         // Canal 0, frequência, resolução
  ledcAttachPin(BUZZER_PINO, 0); // Pino do buzzer para o canal 0

  while (1){
    xSemaphoreTake(statusMutex, portMAX_DELAY);
    if (statusVibracao == 1 && !buzzerAtivado){
      Serial.println("Buzzer ativado aqui!");
      statusBuzzer = 1;
      buzzerAtivado = true;
    }
    xSemaphoreGive(statusMutex);

    if (statusBuzzer == 1){
      // Ativa o buzzer
      ledcWriteTone(0, 3000); // Ativa a frequência no canal 0
      delay(3000);
      ledcWriteTone(0, 0); // Desativa o canal 0
      Serial.println("Buzzer ativado!");

      // Marca como inativo para evitar que o buzzer seja ativado continuamente
      xSemaphoreTake(statusMutex, portMAX_DELAY);
      statusBuzzer = 0;
      xSemaphoreGive(statusMutex);
    }

    vTaskDelay(100 / portTICK_PERIOD_MS);
  }
}

void tarefaMonitor(void *pvParameters){
  while (1){
    vTaskDelay(15000 / portTICK_PERIOD_MS); // Ajuste para 15 segundos

    xSemaphoreTake(statusMutex, portMAX_DELAY);

    if (statusBotao == 1){
      Serial.println("Botão pressionado!");

      JSONVar statusSensorPoeira;
      statusSensorPoeira["status_poeira"] = statusBotao;

      String statusSensor = JSON.stringify(statusSensorPoeira);
      Serial.println(statusSensor);
      notificarClientes(statusSensor);
      statusBotao = 0;
    }

    if (statusVibracao == 1){
      Serial.println("Vibração detectada!");

      JSONVar statusSensorVibracao;
      statusSensorVibracao["status_vibracao"] = statusVibracao;

      String statusSensor = JSON.stringify(statusSensorVibracao);
      Serial.println(statusSensor);
      notificarClientes(statusSensor);
      statusVibracao = 0;
    }

    if (statusTemperatura == 1){
      Serial.println("Temperatura acima de 15 graus!");

      JSONVar statusSensorTemperatura;
      statusSensorTemperatura["status_temperatura"] = statusTemperatura;

      String statusSensor = JSON.stringify(statusSensorTemperatura);
      Serial.println(statusSensor);
      notificarClientes(statusSensor);
      statusTemperatura = 0;
    }

    if (statusRuido == 1){
      Serial.println("Ruído acima de 30 dB!");

      JSONVar statusSensorRuido;
      statusSensorRuido["status_ruido"] = statusRuido;

      String statusSensor = JSON.stringify(statusSensorRuido);
      Serial.println(statusSensor);
      notificarClientes(statusSensor);
      statusRuido = 0;
    }
    xSemaphoreGive(statusMutex);
  }
}

void setup(){
  iniciarWebSocket();

  Serial.begin(115200);
  iniciarWifi();
  statusMutex = xSemaphoreCreateMutex();

  xTaskCreatePinnedToCore(tarefaVibracao, "TarefaVibracao", 4096, NULL, 1, NULL, 1);
  xTaskCreatePinnedToCore(tarefaRuido, "TarefaRuido", 4096, NULL, 2, NULL, 1);
  xTaskCreatePinnedToCore(tarefaTemperatura, "TarefaTemperatura", 4096, NULL, 3, NULL, 1);
  xTaskCreatePinnedToCore(tarefaBotao, "TarefaBotao", 4096, NULL, 4, NULL, 1);

  xTaskCreatePinnedToCore(tarefaBuzzer, "TarefaBuzzer", 4096, NULL, 1, NULL, 0);
  xTaskCreatePinnedToCore(tarefaRelogio, "TarefaRelogio", 4096, NULL, 2, NULL, 0);
  xTaskCreatePinnedToCore(tarefaMonitor, "TarefaMonitor", 4096, NULL, 3, NULL, 0);

  // URL raiz do servidor Web
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){ request->send_P(200, "text/html", index_html); });

  // Iniciar servidor
  server.begin();
}

void loop(){
  ws.cleanupClients();
}
