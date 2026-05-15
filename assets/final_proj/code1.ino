#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include <esp_now.h>
#include <esp_wifi.h>

WebServer server(80);

// Broadcast address
uint8_t receiverAddress[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

const int espNowChannel = 1;

void onDataSent(const wifi_tx_info_t *tx_info, esp_now_send_status_t status) {
  Serial.print("Send status: ");
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Success" : "Fail");
}

void sendConfig(char config) {
  Serial.print("Sending config: ");
  Serial.println(config);

  esp_err_t result = esp_now_send(receiverAddress, (uint8_t *)&config, 1);

  if (result == ESP_OK) {
    Serial.println("ESP-NOW send queued");
  } else {
    Serial.print("ESP-NOW send error: ");
    Serial.println(result);
  }
}

String pageHtml() {
  return R"rawliteral(
<!DOCTYPE html>
<html>
<head>
  <title>ESP32 Gear Controller</title>
  <style>
    body {
      font-family: Arial, sans-serif;
      text-align: center;
      padding: 24px;
      background: #f5f5f5;
    }

    h1 {
      font-size: 28px;
      margin-bottom: 8px;
    }

    .status {
      font-size: 18px;
      margin: 12px 0 24px;
    }

    .wheel-wrap {
      display: flex;
      justify-content: center;
      margin: 20px 0;
    }

    .wheel {
      width: 110px;
      height: 110px;
      border: 12px solid #222;
      border-radius: 50%;
      position: relative;
      animation-name: spin;
      animation-timing-function: linear;
      animation-iteration-count: infinite;
      animation-duration: 0s;
      animation-play-state: paused;
      background: white;
    }

    .wheel::before {
      content: "";
      position: absolute;
      top: 47px;
      left: -10px;
      width: 130px;
      height: 8px;
      background: #222;
      border-radius: 8px;
    }

    .wheel::after {
      content: "";
      position: absolute;
      top: -10px;
      left: 47px;
      width: 8px;
      height: 130px;
      background: #222;
      border-radius: 8px;
    }

    .hub {
      position: absolute;
      width: 26px;
      height: 26px;
      background: #222;
      border-radius: 50%;
      top: 42px;
      left: 42px;
      z-index: 2;
    }

    @keyframes spin {
      from { transform: rotate(0deg); }
      to { transform: rotate(360deg); }
    }

    button {
      width: 260px;
      height: 58px;
      margin: 8px;
      font-size: 18px;
      border-radius: 12px;
      border: none;
      background: #222;
      color: white;
    }

    .base {
      background: #555;
    }

    .stop {
      background: #b00020;
    }
  </style>
</head>
<body>
  <h1>ESP32 Gear Controller</h1>

  <div class="status" id="status">Current: 0 - Base, no gear engaged</div>

  <div class="wheel-wrap">
    <div class="wheel" id="wheel">
      <div class="hub"></div>
    </div>
  </div>

  <button class="base" onclick="sendConfig(0, 0, '0 - Base / both 90 / stopped')">
    0: Base / Both 90
  </button><br>

  <button onclick="sendConfig(2, 0.33, '2 - GR 0.33')">
    2: GR 0.33
  </button><br>

  <button onclick="sendConfig(1, 0.56, '1 - GR 0.56')">
    1: GR 0.56
  </button><br>

  <button onclick="sendConfig(4, 1.00, '4 - GR 1.00')">
    4: GR 1.00
  </button><br>

  <button onclick="sendConfig(3, 1.67, '3 - GR 1.67')">
    3: GR 1.67
  </button><br>

  <button class="stop" onclick="sendConfig(5, 0, '5 - STOP')">
    5: STOP
  </button>

  <script>
    function setWheelSpeed(gearRatio) {
      const wheel = document.getElementById('wheel');

      if (gearRatio <= 0) {
        wheel.style.animationPlayState = 'paused';
        wheel.style.animationDuration = '0s';
        return;
      }

      // Higher gear ratio = faster visual rotation.
      // Duration is inversely proportional to gear ratio.
      const baseDuration = 2.5;
      const duration = baseDuration / gearRatio;

      wheel.style.animationDuration = duration + 's';
      wheel.style.animationPlayState = 'running';
    }

    function sendConfig(num, gearRatio, label) {
      document.getElementById('status').innerText = 'Current: ' + label;
      setWheelSpeed(gearRatio);

      fetch('/send?config=' + num)
        .then(response => response.text())
        .then(data => console.log(data))
        .catch(error => {
          console.log(error);
          document.getElementById('status').innerText = 'Send failed';
        });
    }
  </script>
</body>
</html>
)rawliteral";
}

void handleRoot() {
  server.send(200, "text/html", pageHtml());
}

void handleSend() {
  if (!server.hasArg("config")) {
    server.send(400, "text/plain", "Missing config");
    return;
  }

  String config = server.arg("config");

  if (config.length() != 1 || config[0] < '0' || config[0] > '5') {
    server.send(400, "text/plain", "Invalid config");
    return;
  }

  sendConfig(config[0]);

  server.send(200, "text/plain", "Sent config " + config);
}

void setup() {
  Serial.begin(115200);
  delay(1000);

  Serial.println("Xiao ESP32-C3 Controller Starting");

  WiFi.mode(WIFI_AP_STA);

  WiFi.softAP("ESP32_Controller", "12345678", espNowChannel);

  esp_wifi_set_channel(espNowChannel, WIFI_SECOND_CHAN_NONE);

  Serial.print("Controller AP IP: ");
  Serial.println(WiFi.softAPIP());

  Serial.print("Controller MAC: ");
  Serial.println(WiFi.macAddress());

  if (esp_now_init() != ESP_OK) {
    Serial.println("ESP-NOW init failed");
    return;
  }

  esp_now_register_send_cb(onDataSent);

  esp_now_peer_info_t peerInfo = {};
  memcpy(peerInfo.peer_addr, receiverAddress, 6);
  peerInfo.channel = espNowChannel;
  peerInfo.encrypt = false;
  peerInfo.ifidx = WIFI_IF_STA;

  if (esp_now_add_peer(&peerInfo) != ESP_OK) {
    Serial.println("Failed to add ESP-NOW peer");
    return;
  }

  server.on("/", handleRoot);
  server.on("/send", handleSend);
  server.begin();

  Serial.println("Web controller ready.");
  Serial.println("Connect to WiFi: ESP32_Controller");
  Serial.println("Open: http://192.168.4.1");
}

void loop() {
  server.handleClient();
}