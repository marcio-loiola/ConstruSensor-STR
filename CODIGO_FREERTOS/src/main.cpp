#include <Arduino.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/semphr.h>

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
const int limiteRuido = 70;
const int tempoLimite = 10;
bool buzzerAtivado = false;
bool relogioAtivado = false;

void tarefaBotao(void *pvParameters) {
  pinMode(BOTAO_PINO, INPUT_PULLUP);

  while (1) {
    int botaoPressionado = digitalRead(BOTAO_PINO);

    if (botaoPressionado == LOW) {
      xSemaphoreTake(statusMutex, portMAX_DELAY);
      statusBotao = 1;
      xSemaphoreGive(statusMutex);
    }

    vTaskDelay(1000 / portTICK_PERIOD_MS);
  }
}

void tarefaVibracao(void *pvParameters) {
  pinMode(VIBRACAO_PINO, INPUT);

  TickType_t xLastWakeTime = xTaskGetTickCount();

  while (1) {
    int sensorVibracao = digitalRead(VIBRACAO_PINO);

    if (sensorVibracao != ultimaLeitura) {
      vTaskDelay(debounceDelay / portTICK_PERIOD_MS);
      sensorVibracao = digitalRead(VIBRACAO_PINO);

      if (sensorVibracao == HIGH) {
        xSemaphoreTake(statusMutex, portMAX_DELAY);
        statusVibracao = 1;
        xSemaphoreGive(statusMutex);
      }
    }

    ultimaLeitura = sensorVibracao;
    vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(2000));
  }
}


void tarefaTemperatura(void *pvParameters) {
  TickType_t xLastWakeTime = xTaskGetTickCount();

  while (1) {
    float temperatura = analogRead(TEMPERATURA_PINO) * (3.3 / 4095) * 100;

    if (temperatura > 15.0) {
      xSemaphoreTake(statusMutex, portMAX_DELAY);
      statusTemperatura = 1;
      xSemaphoreGive(statusMutex);
    }

    vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(10000));
  }
}

void tarefaRuido(void *pvParameters) {
  TickType_t xLastWakeTime = xTaskGetTickCount();

  int contadorLimite = 0;
  const int periodoLeitura = 4000;
  const int numLeituras = tempoLimite * 1000 / periodoLeitura;

  while (1) {
    float nivelRuido = analogRead(RUIDO_PINO);

    if (nivelRuido > limiteRuido) {
      contadorLimite++;

      if (contadorLimite >= numLeituras) {
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

void tarefaRelogio(void *pvParameters) {
  while (1) {
    xSemaphoreTake(statusMutex, portMAX_DELAY);
    if (statusRuido == 1 && !relogioAtivado) {
      Serial.println("Relógio ativado aqui!");
      statusRelogio = 1;
      relogioAtivado = true;
    }
    xSemaphoreGive(statusMutex);

    vTaskDelay(100 / portTICK_PERIOD_MS);
  }
}

void tarefaBuzzer(void *pvParameters) {
  while (1) {
    xSemaphoreTake(statusMutex, portMAX_DELAY);
    if (statusVibracao == 1 && !buzzerAtivado) {
      Serial.println("Buzzer ativado aqui!");
      statusBuzzer = 1;
      buzzerAtivado = true;
    }
    xSemaphoreGive(statusMutex);

    vTaskDelay(100 / portTICK_PERIOD_MS);
  }
}

void tarefaMonitor(void *pvParameters) {
  while (1) {
    vTaskDelay(15000 / portTICK_PERIOD_MS);  // Ajuste para 15 segundos

    xSemaphoreTake(statusMutex, portMAX_DELAY);
    if (statusBotao == 1) {
      Serial.println("Botão pressionado!");
      statusBotao = 0;
    }
    if (statusVibracao == 1) {
      Serial.println("Vibração detectada!");
      statusVibracao = 0;
    }
    if (statusTemperatura == 1) {
      Serial.println("Temperatura acima de 15 graus!");
      statusTemperatura = 0;
    }
    if (statusRuido == 1) {
      Serial.println("Ruído acima de 30 dB!");
      statusRuido = 0;
    }
    xSemaphoreGive(statusMutex);
  }
}

void setup() {
  Serial.begin(115200);
  statusMutex = xSemaphoreCreateMutex();

  xTaskCreatePinnedToCore(tarefaVibracao, "TarefaVibracao", 4096, NULL, 1, NULL, 1);
  xTaskCreatePinnedToCore(tarefaRuido, "TarefaRuido", 4096, NULL, 2, NULL, 1);
  xTaskCreatePinnedToCore(tarefaTemperatura, "TarefaTemperatura", 4096, NULL, 3, NULL, 1);
  xTaskCreatePinnedToCore(tarefaBotao, "TarefaBotao", 4096, NULL, 4, NULL, 1);

  xTaskCreatePinnedToCore(tarefaBuzzer, "TarefaBuzzer", 4096, NULL, 1, NULL, 0);
  xTaskCreatePinnedToCore(tarefaRelogio, "TarefaRelogio", 4096, NULL, 2, NULL, 0);
  xTaskCreatePinnedToCore(tarefaMonitor, "TarefaMonitor", 4096, NULL, 3, NULL, 0);
}

void loop() {
  // loop vazio
}
