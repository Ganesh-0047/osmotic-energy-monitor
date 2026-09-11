#include <WiFi.h>
#include <WebServer.h>
#include "DHT.h"

// ---------- WiFi ----------
const char* ssid = "GMU_Staff";
const char* password = "GMU@2025@";

// ---------- Pins ----------
#define DHTPIN 4
#define DHTTYPE DHT22

#define CURRENT_PIN 32
#define TDS_PIN 34
#define VOLTAGE_PIN 33
#define PRESSURE_PIN 35

DHT dht(DHTPIN, DHTTYPE);
WebServer server(80);

// ---------- Read sensors ----------
String getSensorData() {

  float temperature = dht.readTemperature();
  float humidity = dht.readHumidity();

  int tdsRaw = analogRead(TDS_PIN);
  float tdsVoltage = (tdsRaw / 4095.0) * 3.3;

  int pressureRaw = analogRead(PRESSURE_PIN);
  float pressureVoltage = (pressureRaw / 4095.0) * 3.3;

  int voltageRaw = analogRead(VOLTAGE_PIN);
  float voltage = (voltageRaw / 4095.0) * 3.3 * 5.0;

  int currentRaw = analogRead(CURRENT_PIN);
  float currentVoltage = (currentRaw / 4095.0) * 3.3;

  // Approximate ACS712 5A calculation
  float current = (currentVoltage - 2.5) / 0.185;

  if (current < 0) current = 0;

  float power = voltage * current;

  String data = "{";
  data += "\"temperature\":" + String(temperature, 1) + ",";
  data += "\"tdsVoltage\":" + String(tdsVoltage, 2) + ",";
  data += "\"pressureVoltage\":" + String(pressureVoltage, 2) + ",";
  data += "\"voltage\":" + String(voltage, 2) + ",";
  data += "\"current\":" + String(current, 2) + ",";
  data += "\"power\":" + String(power, 2);
  data += "}";

  return data;
}

// ---------- Dashboard ----------
void handleRoot() {

  String page = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
<meta name="viewport" content="width=device-width, initial-scale=1">
<title>Osmotic Energy Monitor</title>

<style>
body {
  font-family: Arial;
  background:#f4f6f8;
  margin:20px;
}

h1 {
  color:#123;
}

.card {
  background:white;
  padding:20px;
  margin:12px 0;
  border-radius:12px;
  box-shadow:0 2px 8px #ccc;
}

.value {
  font-size:28px;
  font-weight:bold;
}

.status {
  color:green;
  font-weight:bold;
}
</style>
</head>

<body>

<h1>Osmotic Energy Monitor</h1>

<div class="card">
<h2>ESP32 Dashboard</h2>
<p>ESP32 Status: <span class="status">CONNECTED</span></p>
<p>IP Address: )rawliteral";

  page += WiFi.localIP().toString();

  page += R"rawliteral(</p>
</div>

<h2>Sensor Monitoring</h2>

<div class="card">
Temperature
<div class="value" id="temperature">-- °C</div>
</div>

<div class="card">
TDS Sensor Voltage
<div class="value" id="tds">-- V</div>
</div>

<div class="card">
Pressure Sensor Voltage
<div class="value" id="pressure">-- V</div>
</div>

<div class="card">
Voltage
<div class="value" id="voltage">-- V</div>
</div>

<div class="card">
Current
<div class="value" id="current">-- A</div>
</div>

<div class="card">
Power
<div class="value" id="power">-- W</div>
</div>

<script>

function updateData() {

  fetch('/data')
  .then(response => response.json())
  .then(data => {

    document.getElementById("temperature").innerHTML =
      data.temperature + " °C";

    document.getElementById("tds").innerHTML =
      data.tdsVoltage + " V";

    document.getElementById("pressure").innerHTML =
      data.pressureVoltage + " V";

    document.getElementById("voltage").innerHTML =
      data.voltage + " V";

    document.getElementById("current").innerHTML =
      data.current + " A";

    document.getElementById("power").innerHTML =
      data.power + " W";

  });
}

setInterval(updateData, 2000);
updateData();

</script>

</body>
</html>
)rawliteral";

  server.send(200, "text/html", page);
}

void handleData() {
  server.send(200, "application/json", getSensorData());
}

// ---------- Setup ----------
void setup() {

  Serial.begin(115200);

  dht.begin();

  WiFi.begin(ssid, password);

  Serial.println("Connecting to WiFi...");

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println();
  Serial.println("WiFi Connected!");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());

  server.on("/", handleRoot);
  server.on("/data", handleData);

  server.begin();

  Serial.println("Web Server Started!");
}

// ---------- Loop ----------
void loop() {
  server.handleClient();
}