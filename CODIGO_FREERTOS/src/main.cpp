#include <Arduino.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/semphr.h>
#include "SPIFFS.h"
#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <Arduino_JSON.h>

#define BOTAO_PINO 2
#define VIBRACAO_PINO 15
#define TEMPERATURA_PINO 25
#define RUIDO_PINO 32
#define BUZZER_PINO 27

SemaphoreHandle_t statusMutex;
int statusBotao = 0;
int statusVibracao = 0;
int statusTemperatura = 0;
int statusRuido = 0;
int statusRelogio = 0;
int statusBuzzer = 0;
int ultimaLeitura = 0;
int debounceDelay = 50;
const int limiteRuido = 70;
const int tempoLimite = 10;
bool buzzerAtivado = false;
bool relogioAtivado = false;
const int pinoDoSom = 17;         // substitua pelo pino que você conectou no ESP
const int frequenciaDoSom = 1000; // em Hertz
const int duracaoDoSom = 1000;    // em milissegundos
String ultimoResultado = " ";

// WEBSocket
// Credenciais da Internet
const char *ssid = "Isaac´s Galaxy A02s";
const char *password = "jspa9096";

// Cria um objeto AsyncWebServer na porta 80
AsyncWebServer server(80);

// Cria um objeto WebSocket
AsyncWebSocket ws("/ws");

// Json Variable to Hold Sensor Readings
JSONVar readings;

// Timer variables
unsigned long lastTime = 0;
unsigned long timerDelay = 30000;


void tarefaBotao(void *pvParameters) {
  pinMode(BOTAO_PINO, INPUT_PULLUP);

  while(1){
    int botaoPressionado = digitalRead(BOTAO_PINO);

    if (botaoPressionado == LOW){
      xSemaphoreTake(statusMutex, portMAX_DELAY);
      statusBotao = 1;
      xSemaphoreGive(statusMutex);
    }

    readings["status_botao"] = String(statusBotao);
    vTaskDelay(1000 / portTICK_PERIOD_MS);
  }
}

void tarefaVibracao(void *pvParameters){
  pinMode(VIBRACAO_PINO, INPUT);

  TickType_t xLastWakeTime = xTaskGetTickCount();

  while (1){
    int sensorVibracao = digitalRead(VIBRACAO_PINO);

    if(sensorVibracao != ultimaLeitura){
      vTaskDelay(debounceDelay / portTICK_PERIOD_MS);
      sensorVibracao = digitalRead(VIBRACAO_PINO);

      if(sensorVibracao == HIGH){
        xSemaphoreTake(statusMutex, portMAX_DELAY);
        statusVibracao = 1;
        xSemaphoreGive(statusMutex);
      }
    }

    ultimaLeitura = sensorVibracao;
    readings["status_vibracao"] = String(statusVibracao);
    vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(2000));
  }
}

void tarefaTemperatura(void *pvParameters){
  TickType_t xLastWakeTime = xTaskGetTickCount();

  while(1){
    float temperatura = analogRead(TEMPERATURA_PINO) * (3.3 / 4095) * 100;

    if(temperatura > 15.0){
      xSemaphoreTake(statusMutex, portMAX_DELAY);
      statusTemperatura = 1;
      xSemaphoreGive(statusMutex);
    }
    
    readings["status_temperatura"] = String(statusTemperatura);
    vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(10000));
  }
}

void tarefaRuido(void *pvParameters){
  TickType_t xLastWakeTime = xTaskGetTickCount();

  int contadorLimite = 0;
  const int periodoLeitura = 10000;
  const int numLeituras = tempoLimite * 1000 / periodoLeitura;

  while (1){
    float nivelRuido = analogRead(RUIDO_PINO);

    if (nivelRuido > limiteRuido){
      contadorLimite++;

      if (contadorLimite >= numLeituras){
        xSemaphoreTake(statusMutex, portMAX_DELAY);
        statusRuido = 1;
        xSemaphoreGive(statusMutex);
        contadorLimite = 0;
      }
    } else {
      contadorLimite = 0;
    }

    readings["status_ruido"] = String(statusRuido);
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

  while(1){
    xSemaphoreTake(statusMutex, portMAX_DELAY);
    if(statusVibracao == 1 && !buzzerAtivado){
      Serial.println("Buzzer ativado aqui!");
      statusBuzzer = 1;
      buzzerAtivado = true;
    }
    xSemaphoreGive(statusMutex);

    if(statusBuzzer == 1){
      // Ativa o buzzer
      // tone(BUZZER_PINO, frequenciaDoSom, duracaoDoSom);
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
      statusBotao = 0;
    }
    if (statusVibracao == 1){
      Serial.println("Vibração detectada!");
      statusVibracao = 0;
    }
    if (statusTemperatura == 1){
      Serial.println("Temperatura acima de 15 graus!");
      statusTemperatura = 0;
    }
    if (statusRuido == 1){
      Serial.println("Ruído acima de 30 dB!");
      statusRuido = 0;
    }
    xSemaphoreGive(statusMutex);
  }
}

// Tarefas WebSocket - Início

// Initialize SPIFFS
void initSPIFFS(){
  if (!SPIFFS.begin(true)){
    Serial.println("An error has occurred while mounting SPIFFS");
  }
  Serial.println("SPIFFS mounted successfully");
}

// Initialize WiFi
void initWiFi(){
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Conectando ao Wifi....");
  }
  Serial.print("IP para conectar: ");
  Serial.println(WiFi.localIP());
}

void notifyClients(String sensorReadings){
  ws.textAll(sensorReadings);
}

void handleWebSocketMessage(void *arg, uint8_t *data, size_t len){
  AwsFrameInfo *info = (AwsFrameInfo *)arg;
  if (info->final && info->index == 0 && info->len == len && info->opcode == WS_TEXT){
    data[len] = 0;
    String message = (char*)data;
    // Check if the message is "getReadings"
    if(strcmp((char*)data, "getReadings") == 0) {
    // if it is, send current sensor readings
      String sensorReadings = JSON.stringify(readings);
      Serial.print(sensorReadings);
      notifyClients(sensorReadings);
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

void initWebSocket(){
  ws.onEvent(onEvent);
  server.addHandler(&ws);
}

// Tarefas WebSocket - Fim

void setup(){
  Serial.begin(115200);
  statusMutex = xSemaphoreCreateMutex();

  xTaskCreatePinnedToCore(tarefaVibracao, "TarefaVibracao", 4096, NULL, 1, NULL, 1);
  xTaskCreatePinnedToCore(tarefaRuido, "TarefaRuido", 4096, NULL, 2, NULL, 1);
  xTaskCreatePinnedToCore(tarefaTemperatura, "TarefaTemperatura", 4096, NULL, 3, NULL, 1);
  xTaskCreatePinnedToCore(tarefaBotao, "TarefaBotao", 4096, NULL, 4, NULL, 1);

  xTaskCreatePinnedToCore(tarefaBuzzer, "TarefaBuzzer", 4096, NULL, 1, NULL, 0);
  xTaskCreatePinnedToCore(tarefaRelogio, "TarefaRelogio", 4096, NULL, 2, NULL, 0);
  xTaskCreatePinnedToCore(tarefaMonitor, "TarefaMonitor", 4096, NULL, 3, NULL, 0);

  initWiFi();
  initSPIFFS();
  initWebSocket();

  // Web Server Root URL
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){ request->send(SPIFFS, "/index.html", "text/html"); });
  server.serveStatic("/", SPIFFS, "/");

  // Start server
  server.begin();
}

void loop(){
  if ((millis() - lastTime) > timerDelay){
    String sensorReadings = JSON.stringify(readings);
    if (ultimoResultado == " "){
      Serial.println(sensorReadings);
      Serial.println("Primeira execucao");
      ultimoResultado = sensorReadings;
    }
    if (ultimoResultado != sensorReadings){
      ultimoResultado = sensorReadings;
      Serial.println(sensorReadings);
      notifyClients(sensorReadings);
    }

    lastTime = millis();
  }

  ws.cleanupClients();
}
