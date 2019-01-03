
#include <ESP8266WiFi.h>
#include <SocketIOClient.h>
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <ArduinoJson.h>
#include <WiFiManager.h>
#include <EEPROM.h>

#define AN_CHAN A0
#define LED_PIN 4   //D0

SocketIOClient client;

extern String RID;
extern String Rname;
extern String Rcontent;

String previewAnalog = "";

unsigned long previousMillis = 0, previousMillis2 = 0;
long interval = 10000, interval2 = 20000;
unsigned long lastreply = 0;
unsigned long lastsend = 0, lastsend2 = 0;

// HOST
char host[] = "controlwifi.herokuapp.com";
//char host[] = "192.168.1.52";

// PORT
int port = 80;

// Variables
String chipID = String(ESP.getChipId());
String UserID;

void setup() {
  // Reservar espacio EEPROM
  EEPROM.begin(512);

  // Iniciar puerto serial 115200 BAUD
  Serial.begin(115200);
  pinMode(LED_PIN, OUTPUT);


  int tamUserID = EEPROM.read(1);
  char lectura;
  int j = 1;
  // Serial.println("Tamano palabra");
  // Serial.println(String(tamUserID));
  if (tamUserID != 0) {
    while (tamUserID >= 0) {
      lectura = EEPROM.read(j);
      j++;
      tamUserID--;
      Serial.write(lectura);
      //delay(1000);
      UserID += lectura;
    }
  }

  Serial.println(UserID);
  //WiFiManager
  WiFiManager wifiManager;
  //String miUserID = String(wifiManager.getUserID());

  //Serial.println("Ohh yeah: " + String(wifiManager.getUserID()));

  //wifiManager.setUserID(String("Nuevo Nice !"));

  //Serial.println("El nuevo USER ID: " + String(wifiManager.getUserID()));
  // Reset saved settings
  // wifiManager.resetSettings();

  // Set custom ip for portal
  // wifiManager.setAPConfig(IPAddress(10,0,1,1), IPAddress(10,0,1,1), IPAddress(255,255,255,0));
  digitalWrite(LED_PIN, HIGH);

  //fetches ssid and pass from eeprom and tries to connect
  //if it does not connect it starts an access point with the specified name
  //here  "AutoConnectAP"
  //and goes into a blocking loop awaiting configuration
  wifiManager.autoConnect("AutoConnectAP");

  // If you get here you have connected to the WiFi
  Serial.println("Conexion exitosa WiFi");

  if (!client.connect(host, port)) {
    Serial.println("Fallo conexion");
    wifiManager.autoConnect("AutoConnectAP");
    //return;
  }

  if (client.connected()) {
    // Envio el id al servidor y datos de conexion inicial
    client.send("connection", "Bombillo001", "Mi nombre es ESP8266, Y ESTOY ENVIANDO DATOS AL SERVIDOR EN EL MOMENTO DE LA CONEXION");//en el momento de la conexion
    digitalWrite(LED_PIN, LOW);
  }

  digitalWrite(LED_PIN, LOW);
}

void loop() {
  static int i;
  static int state = 0;
  unsigned long currentMillis = millis();
  unsigned long currentMillis2 = millis();

  if (currentMillis2 - previousMillis2 > interval2) {
  


   
    previousMillis2 = currentMillis2;

    client.heartbeat(0);
    
  
  }

  if (client.monitor())
  {
    lastreply = millis();
    Serial.print("RID: ");
    Serial.println(RID);
    Serial.print("Rname: ");
    Serial.println(Rname);
    Serial.println("Rcontent");
    Serial.println(Rcontent);
    if (RID == "welcome" && Rname == "message") {
      Serial.println("gustavo este dato llego");
      Serial.println(Rcontent);
    }

    if (RID == "comando_resp" && Rname == "message") {
      /*  Serial.println("comando_resp");
        Serial.println(Rcontent);
        Serial.println("____________________________________");*/
      if (Rcontent == "On") {
        Serial.println("ya prendio");
        digitalWrite(LED_PIN, HIGH);
        client.send("confirmacion", "r", "on." + chipID);
      }

      if (Rcontent == "Off") {
        Serial.println("ya apague");
        digitalWrite(LED_PIN, LOW);
        client.send("confirmacion", "r", "off." + chipID);
      }
    }

    if (RID == "ADC" && Rname == "message") {
      /*
        Serial.println("gustavo llegaron datos del ADC desde el servidor");
        Serial.println(Rcontent);
      */
    }
  }

  if (!client.connected()) { ///si se desconeecto fel wifi
    Serial.println("connection terminated");
    Serial.println("reestarting communication");

    if (!client.connect(host, port)) { //si no se conecta al servidor  if (!client.connected())
      Serial.println("connection failed");
      delay(100);
      setup();
      return;
    }
  }

  Rcontent="";
}

