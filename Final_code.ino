#include <ArduinoJson.h>
#include <WiFi.h>
#include <SPI.h>
#include <DHT.h>
#include <Wire.h>
#include <Adafruit_BMP085.h>
#include <LiquidCrystal.h>
#define DHTPIN 4   
#define DHTTYPE DHT11
#define SOUND_SPEED 0.034
#define CM_TO_INCH 0.393701

LiquidCrystal lcd(19, 23, 18, 17, 16, 15);
DHT dht(DHTPIN, DHTTYPE);
Adafruit_BMP085 bmp;
WiFiClient client;

const int trigPin = 5;
const int echoPin = 12;
long duration;
float distanceCm;
float distanceInch;

const char* ssid = "WI-FII";
const char* password =  "qwerty123";
const char* server = "api.openweathermap.org";
const char* resource = "/data/2.5/weather?q=Lviv,ua&APPID=a894d91e7a036212025c61739a68d658";
const unsigned long HTTP_TIMEOUT = 10000;  // max respone time from server
const size_t MAX_CONTENT_SIZE = 512;       // max size of the HTTP response
byte mac[] = {0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED};

struct clientData {
  char temp[8];
  char humidity[8];
  char pressure[8];
};

void setup() {
  lcd.begin(16, 2);
  lcd.clear();
  lcd.print("Local and Net");
  lcd.setCursor(0,2);
  lcd.print("Weather PLZ WAIT");
  
  pinMode(trigPin, OUTPUT); 
  pinMode(echoPin, INPUT);
  
  Serial.begin(115200);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi..");
  }
  Serial.println("Connected to the WiFi network");
  dht.begin();
  bmp.begin();
}

void loop() {
  
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  duration = pulseIn(echoPin, HIGH);
  distanceCm = duration * SOUND_SPEED/2;
  distanceInch = distanceCm * CM_TO_INCH;

  if(distanceCm < 15){
    if(connect(server)) {
      if(sendRequest(server, resource) && skipResponseHeaders()) {
        clientData clientData;
        if(readReponseContent(&clientData)) {
          printclientData(&clientData);
          delay(5000);
          Local_Sensors();
          delay(5000);
        }
      }
    }
  }
  else 
    Serial.print("Distance (cm): ");
    Serial.println(distanceCm);
    lcd.clear();
    lcd.print("Unlock Data");
    delay(500);
}

bool connect(const char* hostName) {
  Serial.print("Connect to ");
  Serial.println(hostName);
  bool ok = client.connect(hostName, 80);
  Serial.println(ok ? "Connected" : "Connection Failed!");
  return ok;
}

bool sendRequest(const char* host, const char* resource) {
  Serial.print("GET ");
  Serial.println(resource);
  client.print("GET ");
  client.print(resource);
  client.println(" HTTP/1.1");
  client.print("Host: ");
  client.println(host);
  client.println("Connection: close");
  client.println();
  return true;
}

bool skipResponseHeaders() {
  char endOfHeaders[] = "\r\n\r\n";
  client.setTimeout(HTTP_TIMEOUT);
  bool ok = client.find(endOfHeaders);
  if (!ok) {
    Serial.println("No response or invalid response!");
  }
  return ok;
}

bool readReponseContent(struct clientData* clientData) {
  const size_t bufferSize = 768;
  DynamicJsonBuffer jsonBuffer(bufferSize);
  JsonObject& root = jsonBuffer.parseObject(client);

  if (!root.success()) {
    Serial.println("JSON parsing failed!");
    return false;
  }

  strcpy(clientData->temp, root["main"]["temp"]);
  strcpy(clientData->humidity, root["main"]["humidity"]);
  strcpy(clientData->pressure, root["main"]["pressure"]);
  return true;
}

void printclientData(const struct clientData* clientData) {
  
  const char* tempnet = clientData->temp;
  const char* humnet = clientData->humidity;
  const char* presnet = clientData->pressure;

  int tempnet2 = tempnet[50];
  tempnet2 = tempnet2 - 255;
  Serial.print("TempnetNet = ");
  Serial.print(tempnet2);
  Serial.print("; ");

  Serial.print("HumidityNet = ");
  Serial.print(humnet);
  Serial.print("; ");
  
  Serial.print("PressureNet = ");
  Serial.print(presnet);
  Serial.println(";");

  lcd.clear();
  lcd.print("Net Weather:");
  delay(2000);
  
  lcd.clear();
  lcd.print("Hum   Temp  Pres");
  lcd.setCursor(0,2);
  lcd.print(humnet);
  lcd.setCursor(6,2);
  lcd.print (tempnet2);
  lcd.setCursor(12,2);
  lcd.print (presnet);
}

void Local_Sensors() {
  float humlocal = dht.readHumidity();
  float templocal = bmp.readTemperature();
  int preslocal = bmp.readPressure();
  if (isnan(humlocal)) {
    Serial.println(F("Failed to read from DHT sensor!"));
    return;
  }
  
  Serial.print(F("Humidity Local: "));
  Serial.print(humlocal);
  Serial.print("; ");
  
  Serial.print("Temperature Local = ");
  Serial.print(templocal);
  Serial.print(" *C; ");
  
  Serial.print("Pressure Local = ");
  Serial.print(preslocal);
  Serial.println(" Pa;");

  lcd.clear();
  lcd.print("Local Weather:");
  delay(2000);
  
  lcd.clear();
  lcd.print("Hum   Temp  Pres");
  lcd.setCursor(0,2);
  lcd.print(humlocal);
  lcd.setCursor(6,2);
  lcd.print (templocal);
  lcd.setCursor(12,2);
  lcd.print (preslocal/100);
}
