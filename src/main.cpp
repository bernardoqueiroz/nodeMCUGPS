#include <Arduino.h>
#include <SoftwareSerial.h>
//Pinos de comunicacao serial com a ST Núcleo
#define Pin_ST_NUCLEO_RX    5  //Pino D1 da placa Node MCU
#define Pin_ST_NUCLEO_TX    4  //Pino D2 da placa Node MCU
SoftwareSerial SSerial(Pin_ST_NUCLEO_RX, Pin_ST_NUCLEO_TX);

/**
 * Created by K. Suwatchai (Mobizt)
 *
 * Email: k_suwatchai@hotmail.com
 *
 * Github: https://github.com/mobizt/Firebase-ESP-Client
 *
 * Copyright (c) 2023 mobizt
 *
 */

/** This example will show how to authenticate using
 * the legacy token or database secret with the new APIs (using config and auth data).
 */

#if defined(ESP32) || defined(ARDUINO_RASPBERRY_PI_PICO_W)
#include <WiFi.h>
#elif defined(ESP8266)
#include <ESP8266WiFi.h>
#endif

#include <Firebase_ESP_Client.h>

// Provide the RTDB payload printing info and other helper functions.
#include <addons/RTDBHelper.h>

/* 1. Define the WiFi credentials */
#define WIFI_SSID "Bernardo"
#define WIFI_PASSWORD "wifibernardowifi"

/* 2. If work with RTDB, define the RTDB URL and database secret */
// #define DATABASE_URL "https://embarcados2023-default-rtdb.firebaseio.com/" //<databaseName>.firebaseio.com or <databaseName>.<region>.firebasedatabase.app
// #define DATABASE_SECRET "1trbY0S78WkvqXAjfz9EntHwPU1SvvKxJiauv0mK"

#define DATABASE_URL "https://gps-device-embarked-default-rtdb.firebaseio.com/" //<databaseName>.firebaseio.com or <databaseName>.<region>.firebasedatabase.app
#define DATABASE_SECRET "edwX731ToCaO6z2luZzsUx5xilXfH4wPvceO4OUY"

/* 3. Define the Firebase Data object */
FirebaseData fbdo;

/* 4, Define the FirebaseAuth data for authentication data */
FirebaseAuth auth;

/* Define the FirebaseConfig data for config data */
FirebaseConfig config;

unsigned long dataMillis = 0;
int count = 0;

char gps_buffer[512];
unsigned int buffer_index = 0;
char gps_data = '0';
char *latitude;
char *longitude;
int flagGps = 0;

#if defined(ARDUINO_RASPBERRY_PI_PICO_W)
WiFiMulti multi;
#endif

void setup()
{

  Serial.begin(115200);

  #if defined(ARDUINO_RASPBERRY_PI_PICO_W)
    multi.addAP(WIFI_SSID, WIFI_PASSWORD);
    multi.run();
  #else
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  #endif

    Serial.print("Connecting to Wi-Fi");
    unsigned long ms = millis();
    while (WiFi.status() != WL_CONNECTED)
    {
        Serial.print(".");
        delay(300);
  #if defined(ARDUINO_RASPBERRY_PI_PICO_W)
        if (millis() - ms > 10000)
            break;
  #endif
    }
    Serial.println();
    Serial.print("Connected with IP: ");
    Serial.println(WiFi.localIP());
    Serial.println();

    Serial.printf("Firebase Client v%s\n\n", FIREBASE_CLIENT_VERSION);

    /* Assign the certificate file (optional) */
    // config.cert.file = "/cert.cer";
    // config.cert.file_storage = StorageType::FLASH;

    /* Assign the database URL and database secret(required) */
    config.database_url = DATABASE_URL;
    config.signer.tokens.legacy_token = DATABASE_SECRET;

    // The WiFi credentials are required for Pico W
    // due to it does not have reconnect feature.
  #if defined(ARDUINO_RASPBERRY_PI_PICO_W)
    config.wifi.clearAP();
    config.wifi.addAP(WIFI_SSID, WIFI_PASSWORD);
  #endif

    Firebase.reconnectWiFi(true);

    /* Initialize the library with the Firebase authen and config */
    Firebase.begin(&config, &auth);

    // Or use legacy authenticate method
    // Firebase.begin(DATABASE_URL, DATABASE_SECRET);

     // Open serial communications and wait for port to open:
    SSerial.begin(115200);

    Serial.println("Serial by hardware!");

    // set the data rate for the SoftwareSerial port
    SSerial.println("Serial by software!");
}

void loop()
{
    if (millis() - dataMillis > 2000)
    {
        dataMillis = millis();
        int gpsEnabled = 0;
        Serial.printf("Get int ref... %s\n\r", Firebase.RTDB.getInt(&fbdo, "/locais/status", &gpsEnabled) ? String(gpsEnabled).c_str() : fbdo.errorReason().c_str());   
        digitalWrite(BUILTIN_LED, !gpsEnabled);
        SSerial.write(String(gpsEnabled).c_str());
    }

    if (SSerial.available()){   

      char c = SSerial.read();

      if (c == '\n') {
          gps_buffer[buffer_index] = '\0';

          char* latitude = strtok(gps_buffer, ",");
          char* longitude = strtok(NULL, ",");
          
          if (latitude != NULL && longitude != NULL) {
              Serial.printf("Set lat... %s\n\r", Firebase.RTDB.setString(&fbdo, "/locais/latitude", latitude) ? "ok" : fbdo.errorReason().c_str());
              Serial.printf("Set long... %s\n\r", Firebase.RTDB.setString(&fbdo, "/locais/longitude", longitude) ? "ok" : fbdo.errorReason().c_str());
          } else {
              Serial.printf("Invalid coordinates format\n\r");
          }

          buffer_index = 0; 
      } else {
          gps_buffer[buffer_index] = c;
          buffer_index++;

          if (buffer_index >= 512) {
              Serial.printf("Buffer overflow\n\r");
              buffer_index = 0; 
          }
      }

      delay(1);
    }  
}
