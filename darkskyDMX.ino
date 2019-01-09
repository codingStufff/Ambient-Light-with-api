#include <ArduinoJson.h>
#include <ESP8266WiFi.h>
#include <Sun.h>
////////////////Connection//////////////

#include <WiFiClientSecure.h>
const int _httpsPort = 443;
const char* _apiLink = "/forecast/413f75777aa56c4e92ec9005c751ae80/55.661934,12.402117?exclude=minutely,hourly,daily,flags&units=si";
const char* _apiHost = "api.darksky.net";
const char* _apiHostFingerprint = "16 D2 B8 63 6A B7 05 26 96 CF BD 55 79 2C 52 24 2D 51 2B 3A";
const char* ssid = "thormod";
const char* password = "Jt160790";
const int httpsPort = 443;
boolean DEBUG_DarkSkyAPI = true;


/////////////////////////////////////////////
String response;

/////////////////////DMX/////////////////////
#include <ESPDMX.h>
DMXESPSerial dmx;
/////////////////////////////////////////////
unsigned long unixTime = 0;
Sun sun(55.661934, 12.402117);
unsigned long startMillis;
unsigned long currentMillis;
unsigned int period = 100000;

float cloudcover;


void setup() {
  Serial.begin(115200);
  Serial.println();
  Serial.print("connecting to ");
  Serial.println(ssid);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  dmx.init(512);
  startMillis = millis();
      request();
}

void loop()
{
  currentMillis = millis();
  if (currentMillis - startMillis >= period)
  {
    request();

    startMillis = currentMillis;
  }
  unsigned long sunrise = sun.getRise(unixTime);
  unsigned long sunset = sun.getSet(unixTime);
  Serial.println("unixTime: " + (String)unixTime);
  Serial.println("Sunrise: " + (String)sunrise);
  Serial.println("Sunset: " + (String)sunset);
  int r = 255;
  int g = (int)mapf(cloudcover, 0, 1, 100, 120);
  int b = (int)mapf(cloudcover, 0, 1, 50, 80);
  int sAfterNoon = 36000;
  int dim;/// = constrain(map(evening, 3600, 0, 0, 255), 0, 255);

  int getNoon = (sunset - sunrise) * .5;
  unsigned long sum = sunrise + getNoon + sAfterNoon;
  int unixDiff = unixTime - sum;

  if (getNoon > unixTime)
  {

    Serial.println("Before noon");
    dim = constrain(map(unixTime, sunrise, getNoon, 255, 0), 0, 255);

  }
  if (getNoon < unixTime && unixTime < sunset)
  {
    Serial.println("After noon");
    dim = constrain(map(unixTime, getNoon + sunrise, sunset, 0, 255), 0, 255);
  }
  if (unixTime > sunset && unixDiff < 0)
  {
    Serial.println("Evening");
    //dim = constrain(map(unixTime, sunset, sum, 255, 0), 0, 255);
    g = 100;
    b = 50;
    dim = 255;
  }
  if (unixDiff > 0)
  {
    Serial.println("After Evening");
    g = 100;
    b = 50;
    dim = constrain(map(unixTime, sum, sum + 3600, 255, 0), 0, 255);
  }

  Serial.println("Cloudcover: " + (String)cloudcover);
  Serial.println("Red : " + (String)r);
  Serial.println("Green: " + (String)g);
  Serial.println("Blue: " + (String)b);
  Serial.println("Dim: " + (String)dim);

  lamp(0, r, g, b, dim);
  dmx.update();
}


void request()
{
  // Use WiFiClientSecure class to create TLS connection
  WiFiClientSecure client;
  if (DEBUG_DarkSkyAPI) Serial.print("connecting to ");
  if (DEBUG_DarkSkyAPI) Serial.println(_apiHost);
  if (!client.connect(_apiHost, _httpsPort))
  {
    Serial.println("connection failed");
    //   return false;
  }
  DynamicJsonBuffer jsonBuffer;

  String apiAddress = String(_apiLink);
  if (DEBUG_DarkSkyAPI) Serial.print("requesting URL: ");
  if (DEBUG_DarkSkyAPI) Serial.println(apiAddress);
  String clientGet = String("GET ") + apiAddress + " HTTP/1.1\r\n" +
                     "Host: " + _apiHost + "\r\n" +
                     "User-Agent: BuildFailureDetectorESP8266\r\n" +
                     "Connection: close\r\n\r\n" ;
  client.print(clientGet);
  Serial.println(clientGet);

  if (DEBUG_DarkSkyAPI) Serial.println("request sent");
  if (DEBUG_DarkSkyAPI) Serial.println("Free Heap: " + String(ESP.getFreeHeap()));
  String line;
  boolean createJson = false;
  String json = "";
  while (client.connected())
  {
    line = client.readStringUntil('\n');
    if (createJson)
    {
      json = json + line;
    }
    if (line == "\r")
    {
      if (DEBUG_DarkSkyAPI) Serial.println("- headers received -");
      createJson = true;
    }

  }

  char output [json.length() + 1];

  json.toCharArray(output, json.length() + 1);
  Serial.println(sizeof(output));
  JsonObject& root = jsonBuffer.parseObject(output);
  //Serial.println(root);
  Serial.println(jsonBuffer.size());
  if (!root.success())
  {
    Serial.println("parseObject() failed");
    return;
  }
  Serial.println(".. succesfully parsed.");

  unixTime = root["currently"]["time"];
  cloudcover = root["currently"]["cloudCover"];
}


double mapf(double val, double in_min, double in_max, double out_min, double out_max)
{
  return (val - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

void lamp(int a, int r, int g, int b, int dim)
{
  dmx.write((1 + a * 16), r);
  dmx.write((2 + a * 16), g);
  dmx.write((3 + a * 16), b);
  dmx.write((4 + a * 16), dim);
}
