#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <Wire.h>
#include <BH1750.h>

// --- Configuración WiFi ---
const char* ssid = "HONOR Magic7 Lite";           
const char* password = "Gonzalez14"; 

// --- URL del Google Apps Script ---
const char* server = "https://script.google.com/macros/s/AKfycbxeF4fHnBrjlYIHMT2W_BG7S7z6BLNba0F0MI2c8-xKzQ30sP1_BAwBTWONT3ckmtnInA/exec";

// --- Pines de sensores ---
#define PIN_PIR       26   // Sensor de movimiento (HC-SR501)
#define PIN_SONIDO    33   // Sensor de sonido (analógico)
#define PIN_OBSTACULO 35   // Sensor IR de obstáculo
#define PIN_NIVEL     32   // Sensor de nivel de agua (analógico)
#define LED_VERDE     25   // LED de estado OK
#define LED_ROJO      14   // LED de error
#define BUZZER        27   // Alarma sonora

BH1750 lightMeter;

void setup() {
  Serial.begin(115200);

  pinMode(PIN_PIR, INPUT);
  pinMode(PIN_OBSTACULO, INPUT);
  pinMode(LED_VERDE, OUTPUT);
  pinMode(LED_ROJO, OUTPUT);
  pinMode(BUZZER, OUTPUT);

  // --- Inicializar sensor de luz ---
  Wire.begin(21, 22); // SDA=21, SCL=22
  if (lightMeter.begin(BH1750::CONTINUOUS_HIGH_RES_MODE)) {
    Serial.println("Sensor BH1750 inicializado correctamente.");
  } else {
    Serial.println("Error al inicializar el sensor BH1750.");
  }

  // --- Conexión WiFi ---
  Serial.println("Conectando a WiFi...");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi conectado correctamente.");
  Serial.print("Dirección IP: ");
  Serial.println(WiFi.localIP());
}

void loop() {
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;

    // --- Lecturas de sensores ---
    float luz = lightMeter.readLightLevel(); // en lux
    int movimiento = digitalRead(PIN_PIR);
    int obstaculo = digitalRead(PIN_OBSTACULO);
    int nivel = analogRead(PIN_NIVEL);
    int sonido = analogRead(PIN_SONIDO);

    // --- Mostrar en Serial ---
    Serial.printf("Luz: %.2f lux | Mov: %d | Obst: %d | Nivel: %d | Sonido: %d\n",
                  luz, movimiento, obstaculo, nivel, sonido);

    // --- Construir URL con los parámetros ---
    String url = String(server) +
                 "?luz=" + String(luz, 2) +
                 "&mov=" + String(movimiento) +
                 "&obst=" + String(obstaculo) +
                 "&niv=" + String(nivel) +
                 "&son=" + String(sonido);

    // --- Enviar al script de Google ---
    http.begin(url);
    int httpCode = http.GET();

    if (httpCode == 200) {
      Serial.println("Datos enviados correctamente a Google Sheets.");
      digitalWrite(LED_VERDE, HIGH);
      delay(200);
      digitalWrite(LED_VERDE, LOW);
    } else {
      Serial.println("⚠️ Error al enviar datos.");
      digitalWrite(LED_ROJO, HIGH);
      delay(200);
      digitalWrite(LED_ROJO, LOW);
    }

    http.end();
  } else {
    Serial.println("No hay conexión WiFi.");
    digitalWrite(LED_ROJO, HIGH);
  }

  delay(5000); // cada 5 segundos
}
