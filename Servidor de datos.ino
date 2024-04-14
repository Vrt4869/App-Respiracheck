#include <WiFi.h>
#include <ArduinoJson.h>
#include <HTTPClient.h>
#include <Wire.h>
#include "MAX30105.h"
#include "spo2_algorithm.h"
#include "heartRate.h"

// Configura tus credenciales de WiFi
const char* ssid = "Contingencia";
const char* password = "Mar12345";

// Dirección del servidor Flask
const char* serverUrl = "http://127.0.0.1:5000";

MAX30105 particleSensor;

const byte RATE_SIZE = 4; //Increase this for more averaging. 4 is good.
byte rates[RATE_SIZE]; //Array of heart rates
byte rateSpot = 0;
long lastBeat = 0; //Time at which the last beat occurred

float beatsPerMinute;
int beatAvg;

const int LED_PIN = 18;

void setup()
{
  Serial.begin(115200);
  Serial.println("Initializing...");

  pinMode(LED_PIN, OUTPUT);

  // Conectar a WiFi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to WiFi");

  // Initialize sensor
  if (!particleSensor.begin(Wire, I2C_SPEED_FAST)) //Use default I2C port, 400kHz speed
  {
    Serial.println("MAX30102 was not found. Please check wiring/power. ");
    while (1);
  }
  Serial.println("Place your index finger on the sensor with steady pressure.");

  particleSensor.setup(); //Configure sensor with default settings
  particleSensor.setPulseAmplitudeRed(0x0A); //Turn Red LED to low to indicate sensor is running
  particleSensor.setPulseAmplitudeGreen(0); //Turn off Green LED
}

void loop()
{
  long irValue = particleSensor.getIR();
  long redValue = particleSensor.getRed();

  // Calcular la relación rojo/IR
  float redIRRatio = (float)redValue / (float)irValue;
  float SpO2 = -45.060 * pow(redIRRatio, 2) + 30.354 * redIRRatio + 94.845;
  int spo2 = round(SpO2);

  if (checkForBeat(irValue) == true)
  {
    //We sensed a beat!
    long delta = millis() - lastBeat;
    lastBeat = millis();

    beatsPerMinute = 60 / (delta / 1000.0);

    if (beatsPerMinute < 255 && beatsPerMinute > 20)
    {
      rates[rateSpot++] = (byte)beatsPerMinute; //Store this reading in the array
      rateSpot %= RATE_SIZE; //Wrap variable

      //Take average of readings
      beatAvg = 0;
      for (byte x = 0 ; x < RATE_SIZE ; x++)
        beatAvg += rates[x];
      beatAvg /= RATE_SIZE;
    }
  }
  if (irValue < 50000) {
    beatAvg = 0;
    spo2 = 0;
    digitalWrite(LED_PIN, HIGH); // Enciende el LED si no se detecta un dedo
  } else {
    digitalWrite(LED_PIN, LOW); // Apaga el LED si se detecta un dedo
  }
  Serial.print("BPM = ");
  Serial.print(beatAvg);
  Serial.println();

  // Imprimir la saturación de oxígeno
  Serial.print("SpO2 = ");
  Serial.print(spo2);
  Serial.println("%");  

  // Calcular la frecuencia respiratoria basada en la variabilidad de la frecuencia cardíaca (PROVISIONAL)
  int breathingRate =  (beatAvg / 5); // Supone un ciclo de respiración cada 4 o 5 latidos cardíacos
  Serial.print("Frecuencia Respiratoria = ");
  Serial.print(breathingRate); // Imprimir la frecuencia respiratoria con 2 decimales
  Serial.println(" resp/min");

  // Crear un objeto JSON con los datos
  StaticJsonDocument<200> doc;
  doc["breathingRate"] = breathingRate;
  doc["spo2"] = spo2;

  // Imprimir el JSON en el monitor serial
  serializeJsonPretty(jsonDocument, Serial);
  Serial.println();

  // Convertir el objeto JSON a una cadena
  String jsonString;
  serializeJson(jsonDocument, jsonString);

  // Enviar los datos al servidor Flask
  HTTPClient http;
  http.begin("http://127.0.0.1:5000/cedula_home");
  http.addHeader("Content-Type", "application/json");
  int httpResponseCode = http.POST(jsonString);

  if (httpResponseCode > 0) {
    String response = http.getString();
    Serial.println("Respuesta del servidor: " + response);
} else {
    Serial.print("Error en la solicitud HTTP, código: ");
    Serial.println(httpResponseCode);
}
  http.end();
}

String obtenerTimestamp() {
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    Serial.println("Error al obtener la hora local");
    return "No hay hora";
  }

  char days[3], months[3], years[5], hours[3], minutes[3], seconds[3];

  sprintf(days, "%02d", timeinfo.tm_mday);
  sprintf(months, "%02d", timeinfo.tm_mon + 1);
  sprintf(years, "%04d", timeinfo.tm_year + 1900);
  sprintf(hours, "%02d", timeinfo.tm_hour);
  sprintf(minutes, "%02d", timeinfo.tm_min);
  sprintf(seconds, "%02d", timeinfo.tm_sec);

  String timestamp = String(years) + "-" + String(months) + "-" + String(days) + " " + 
                     String(hours) + ":" + String(minutes) + ":" + String(seconds);

  return timestamp;
}