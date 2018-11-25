#include <ArduinoJson.h>
#include <ESP8266WiFi.h>
#include <Sun.h>
////////////////Connection//////////////

#include <WiFiClientSecure.h>
const int _httpsPort = 443;
const char* _apiLink = "/forecast/6141a170f9761d774ab34b28bd03147b/55.6414,12.0804?exclude=minutely,hourly,daily,flags&units=si";
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
Sun sun(55.6414, 12.0804);
unsigned long startMillis;
unsigned long currentMillis;
unsigned int period = 10000;

unsigned long sunrise;

unsigned long sunset;

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
  //float cTemp = mapf(cloudcover, 0, 1, 0, 20); //problemer med mapping - blev konverteret til int.

  //Serial.println(cTemp);

  unsigned long evening = sunset - unixTime;

  int r = 255;
  int g = (int)mapf(cloudcover, 0, 1, 0, 150);
  int b = (int)mapf(cloudcover, 0, 1, 0, 100);
  int dim = map(evening, 3600, 0, 0, 255);

  Serial.println("Cloudcover: " + (String)cloudcover);
  Serial.print("UnixTime: ");
  Serial.println(unixTime);
  Serial.print("Sun rise: ");
  Serial.println(sunrise);
    Serial.print("Sun set: ");
  Serial.println(sunset);
  Serial.print("Sun sets in s: ");
  Serial.println(evening);

  Serial.print("blue: ");
  Serial.println(b);
  Serial.print("green: ");
  Serial.println(g);
    Serial.print("Dim: ");
  Serial.println(dim);
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
    // if (DEBUG_DarkSkyAPI) Serial.println(line);



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
