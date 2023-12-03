#include <FirebaseESP8266.h>
#include <ESP8266WiFi.h>
#include <Arduino.h>
#include <LittleFS.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <RFID.h>
#include <SPI.h>
#include <RFID.h>
#include <Servo.h> // servo library
Servo servo;
RFID rfid(D8, D0); // D8:pin of tag reader SDA.
#define FIREBASE_HOST "android-flutter-8e415-default-rtdb.firebaseio.com"
#define FIREBASE_AUTH "bZMt8bWQCQK7gfwxSwMI6YROwGVHOxHsoN07itAH"
#define WIFI_SSID "AE Chung Trọ"
#define WIFI_PASSWORD "baolauthicho"
unsigned char str[MAX_LEN]; // MAX_LEN is 16: size of the array

FirebaseData fbdo;
void connectWifi()
{
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
  }
  Serial.println("Connected to WiFi");
}

void setup()
{
  Serial.begin(9600);
  connectWifi();
  SPI.begin();
  rfid.init();
  Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH);
  Firebase.reconnectWiFi(true);
  servo.attach(0);
  servo.write(0);
}

bool state_key = false;
bool state_fbdo = false;

void loop()
{

  if (rfid.findCard(PICC_REQIDL, str) == MI_OK) // Wait for a tag to be placed near the reader
  {
    String temp = "";                // Temporary variable to store the read RFID number
    if (rfid.anticoll(str) == MI_OK) // Anti-collision detection, read tag serial number
    {
      for (int i = 0; i < 4; i++) // Record and display the tag serial number
      {
        temp = temp + (0x0F & (str[i] >> 4));
        temp = temp + (0x0F & str[i]);
      }
      if (state_key == false && temp == "1039684013")
      {
        servo.write(180);
        if (!Firebase.setBoolAsync(fbdo, "/door/state", true))
        {
          Serial.println(fbdo.errorReason());
        }
        state_key = true;
        Serial.print("mở của bằng key");
      }
      else if (state_key == true && temp == "1039684013")
      {
        servo.write(0);
        state_key = false;
        if (!Firebase.setBoolAsync(fbdo, "/door/state", false))
        {
          Serial.println(fbdo.errorReason());
        }
        Serial.print("đóng của bằng key");
      }
    }
    rfid.selectTag(str); // Lock card to prevent a redundant read, removing the line will make the sketch read cards continually
  }
  rfid.halt();
  check_door_user();
}
void check_door_user()
{
  if (Firebase.getBool(fbdo, "/door/state", &state_fbdo))
  {
    if (state_fbdo == false && state_key == true)
    {
      Serial.print("đóng của bằng app");

      servo.write(0);
      state_key = false;
    }
    else if (state_fbdo == true && state_key == false)
    {
      Serial.print("mở của bằng app");

      servo.write(180);
      state_key = true;
    }
    else
    {
      Serial.print("ok");
    }
  }
}
