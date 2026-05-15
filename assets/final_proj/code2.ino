#include <Arduino.h>
#include <WiFi.h>
#include <esp_now.h>
#include <esp_wifi.h>
#include <ESP32Servo.h>

Servo servo26;
Servo servo27;

// Motor driver pins
const int motorIn1 = 32;
const int motorIn2 = 33;

// Servo pins
const int servo26Pin = 26;
const int servo27Pin = 27;

// Angles
const int centerAngle = 100;
const int lowAngle = 85;
const int highAngle = 115;

// ESP-NOW channel
const int espNowChannel = 1;

void motorForward() {
  digitalWrite(motorIn1, HIGH);
  digitalWrite(motorIn2, LOW);
}

void motorStop() {
  digitalWrite(motorIn1, LOW);
  digitalWrite(motorIn2, LOW);
}

void setConfig(int config) {
  Serial.print("Received config: ");
  Serial.println(config);

  // Stop motor before changing servo positions
  motorStop();

  if (config == 5) {
    Serial.println("Config 5: STOP motor, no servo movement");
    return;
  }

  if (config == 0) {
    Serial.println("Config 0: both servos 90");
    servo26.write(90);
    servo27.write(90);
  } 
  else if (config == 1) {
    Serial.println("Config 1: servo26 = 80, servo27 = 90");
    servo26.write(80);
    servo27.write(90);
  } 
  else if (config == 2) {
    Serial.println("Config 2: servo26 = 100, servo27 = 90");
    servo26.write(100);
    servo27.write(90);
  } 
  else if (config == 3) {
    Serial.println("Config 3: servo26 = 90, servo27 = 80");
    servo26.write(90);
    servo27.write(80);
  } 
  else if (config == 4) {
    Serial.println("Config 4: servo26 = 90, servo27 = 100");
    servo26.write(90);
    servo27.write(100);
  } 
  else {
    Serial.println("Invalid config");
    return;
  }

  Serial.println("Waiting 1 second before starting DC motor");
  delay(1000);

  Serial.println("Starting DC motor indefinitely");
  motorForward();
}

void onDataRecv(const esp_now_recv_info_t *info, const uint8_t *incomingData, int len) {
  if (len <= 0) return;

  int config = incomingData[0] - '0'; // converts char '0' to int 0
  setConfig(config);
}

void setup() {
  Serial.begin(115200);
  delay(1000);

  Serial.println("ESP32 Robot Receiver Starting");

  pinMode(motorIn1, OUTPUT);
  pinMode(motorIn2, OUTPUT);
  motorStop();

  servo26.setPeriodHertz(50);
  servo27.setPeriodHertz(50);

  servo26.attach(servo26Pin, 500, 2400);
  servo27.attach(servo27Pin, 500, 2400);

  servo26.write(centerAngle);
  servo27.write(centerAngle);

  WiFi.mode(WIFI_STA);
  WiFi.disconnect();

  esp_wifi_set_channel(espNowChannel, WIFI_SECOND_CHAN_NONE);

  Serial.print("Robot MAC: ");
  Serial.println(WiFi.macAddress());

  if (esp_now_init() != ESP_OK) {
    Serial.println("ESP-NOW init failed");
    return;
  }

  esp_now_register_recv_cb(onDataRecv);

  Serial.println("Ready. Waiting for controller input...");
}

void loop() {
  // Nothing here. ESP-NOW callback handles movement.
}