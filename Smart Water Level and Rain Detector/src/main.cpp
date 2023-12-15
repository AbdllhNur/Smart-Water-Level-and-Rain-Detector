#include <Arduino.h>
#include <WiFi.h>
#include <Wire.h>
#include <Firebase_ESP_Client.h>
#include "addons/TokenHelper.h"
#include "addons/RTDBHelper.h"

#define WIFI_SSID "racist"
#define WIFI_PASSWORD "12345678"

#define API_KEY "AIzaSyAQtZqFYNN59WpOfr73ZMn6MNQJP7YDwl8"
#define DATABASE_URL "https://duarrr-proyektor-default-rtdb.asia-southeast1.firebasedatabase.app/"

FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;
bool signupOK = false;

void firebaseSetInt(String, int);
void firebaseSetFloat(String, float);
void firebaseSetString(String, String);

String status_hujan;
String status_sungai;

const int trigPin = 14;
const int echoPin = 12;
long duration;
float ketinggian_air;
#define sound_speed 0.034

const int AO_PIN = 36;

const int buzz = 13;

void setup()
{
  Serial.begin(115200);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("connecting to wifi.....");

  while (WiFi.status() != WL_CONNECTED)
  {
    Serial.print(".");
    delay(500);
  }

  Serial.println();
  Serial.print("connected with IP : ");
  Serial.print(WiFi.localIP());
  Serial.println();

  Serial.printf("Firebase Client v%s\n\n", FIREBASE_CLIENT_VERSION);
  config.api_key = API_KEY;

  if (Firebase.signUp(&config, &auth, "", ""))
  {
    Serial.println("Firebase success");
    signupOK = true;
  }
  else
  {
    String firebaseErrorMessage = config.signer.signupError.message.c_str();
    Serial.printf("%s\n", firebaseErrorMessage);
  }

  config.database_url = DATABASE_URL;
  config.token_status_callback = tokenStatusCallback;

  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);

  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  pinMode(buzz, OUTPUT);
}

void loop()
{

  // done : ambil data sensor raindrop + kirim ke firebase
  int rain_value = analogRead(AO_PIN);
  rain_value = rain_value / 4;
  firebaseSetInt("sensor/sensor_raindrop", rain_value);

  // done : ambil data sensor ultrasonik + kirim ke firebase
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  duration = pulseIn(echoPin, HIGH);
  ketinggian_air = duration * sound_speed / 2;
  firebaseSetFloat("sensor/sensor_ultrasonik", ketinggian_air);
  delay(100);

  // done : kirim status hujan berdasarkan analog value sensor raindrop
  if (rain_value>=1000)
  {
    status_hujan = "tidak hujan";
    firebaseSetString("status/status hujan", status_hujan);
  }
  else if (rain_value<=999 && rain_value>=100)
  {
    status_hujan = "hujan";
    firebaseSetString("status/status hujan", status_hujan);
  }
  else if (99 > rain_value)
  {
    status_hujan = "hujan deras";
    firebaseSetString("status/status hujan", status_hujan);
  }

  digitalWrite(buzz, LOW);
  delay(100);
  // done : kirim status ketinggian air berdasarkan jarak yg diterima sensor ultrasonik
  if (ketinggian_air >= 100)
  {
    status_sungai = "aman";
    firebaseSetString("status/status sungai", status_sungai);
  }
  else if (ketinggian_air>40 && ketinggian_air<100)
  {
    status_sungai = "waspada";
    firebaseSetString("status/status sungai", status_sungai);
    for (int i = 0; i < 5; i++)
    {
      digitalWrite(buzz, HIGH);
      delay(100);
      digitalWrite(buzz, LOW);
      delay(100);
    }
  }
  else
  {
    status_sungai = "bahaya";
    firebaseSetString("status/status sungai", status_sungai);
    digitalWrite(buzz, HIGH);
    delay(100);
  }
}

void firebaseSetFloat(String databaseDirectory, float value)
{
  if (Firebase.RTDB.setFloat(&fbdo, databaseDirectory, value))
  {
    Serial.print("PASSED: ");
    Serial.println(value);
  }
  else
  {
    Serial.println("FAILED");
    Serial.println("REASON: " + fbdo.errorReason());
  }
}

void firebaseSetString(String databaseDirectory, String value)
{
  if (Firebase.RTDB.setString(&fbdo, databaseDirectory, value))
  {
    Serial.print("PASSED: ");
    Serial.println(value);
  }
  else
  {
    Serial.println("FAILED");
    Serial.println("REASON: " + fbdo.errorReason());
  }
}

void firebaseSetInt(String databaseDirectory, int value)
{
  if (Firebase.RTDB.setInt(&fbdo, databaseDirectory, value))
  {
    Serial.print("PASSED: ");
    Serial.println(value);
  }
  else
  {
    Serial.println("FAILED");
    Serial.println("REASON: " + fbdo.errorReason());
  }
}
