#ifdef ESP32
#include <WiFi.h>
#include <ESPmDNS.h>
#include <HTTPClient.h>
#else
#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <ESP8266HTTPClient.h>
#endif

#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>
#include "DHT.h"
#include "time.h"
#include <PubSubClient.h>
#include <string.h>
#include <Adafruit_Sensor.h>
#include <OTAupdate.h>
#include "webConfig.h"

#define DHTPIN 4
#define led 34

// Wifi network station credentials
#define WIFI_SSID "Ferro"
#define WIFI_PASSWORD "12345678"
#define mqtt_server "ee.unsoed.ac.id"

//PubSubClient
WiFiClient espClient;
PubSubClient client(espClient);
unsigned long lastMsg = 0;
#define MSG_BUFFER_SIZE	(50)
char msg[MSG_BUFFER_SIZE];
int value = 0;

// Telegram BOT Token (Get from Botfather)
#define BOT_TOKEN "5863567021:AAE0jXupnwaHMrO0O5FI9w1FcDj47zWrSo8"
#define CHAT_ID "5277794106"

#ifdef ESP8266
X509List cert(TELEGRAM_CERTIFICATE_ROOT);
#endif

WiFiClientSecure secured_client;
UniversalTelegramBot bot(BOT_TOKEN, secured_client);

const char *ntpServer = "pool.ntp.org";
const long gmtOffset_sec = 19800;
const int daylightOffset_sec = 0;

// Google script ID and required credentials
String GOOGLE_SCRIPT_ID = "AKfycbxng6DJAHxfy1DJ3ohS0hTLqgSsZglW1I-_7BdQSzCdcXakOJEnzrH2IjMmXF3ESCqiLQ";
//DHT dht(DHTPIN, DHTTYPE);

const unsigned long BOT_MTBS = 1000; // mean time between scan messages
unsigned long bot_lasttime;          // last time messages' scan has been done
float temperatureC;
float temperatureF;
float humidity;

unsigned int myIP;

void handleNewMessages(int numNewMessages)
{
    Serial.print("handleNewMessages ");
    Serial.println(numNewMessages);
    for (int i = 0; i < numNewMessages; i++)
    {
        String chat_id = String(bot.messages[i].chat_id);
        if (chat_id != CHAT_ID)
        {
            bot.sendMessage(chat_id, "Unauthorized user", "");
        }
        else
        {
            String text = bot.messages[i].text;
            String from_name = bot.messages[i].from_name;
            if (from_name == "")
                from_name = "Guest";
            if (text == "/tempC")
            {
                String msg = "Temperature is ";
                msg += msg.concat(temperatureC);
                msg += "C";
                bot.sendMessage(chat_id, msg, "");
            }
            if (text == "/tempF")
            {
                String msg = "Temperature is ";
                msg += msg.concat(temperatureF);
                msg += "F";
                bot.sendMessage(chat_id, msg, "");
            }
            if (text == "/humidity")
            {
                String msg = "Humidity is ";
                msg += msg.concat(humidity);
                msg += "%";
                bot.sendMessage(chat_id, msg, "");
            }
            if (text == "/start")
            {
                String welcome = "DHTxx sensor readings.\n";
                welcome += "/tempC : Temperature in celcius \n";
                welcome += "/tempF : Temperature in faranthit\n";
                welcome += "/humidity : Humidity\n";
                bot.sendMessage(chat_id, welcome, "Markdown");
            }
        }
    }
}

//PubSubClient
void setup_wifi() {

  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(WIFI_SSID);

  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  randomSeed(micros());

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (unsigned int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();

  // Switch on the LED if an 1 was received as first character
  if ((char)payload[0] == '1') {
    digitalWrite(led, LOW);   // Turn the LED on (Note that LOW is the voltage level
    // but actually the LED is on; this is because
    // it is active low on the ESP-01)
  } else {
    digitalWrite(led, HIGH);  // Turn the LED off by making the voltage HIGH
  }

}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Create a random client ID
    String clientId = "ESP8266Client-";
    clientId += String(random(0xffff), HEX);
    // Attempt to connect
    if (client.connect(clientId.c_str())) {
      Serial.println("connected");
      // Once connected, publish an announcement...
      client.publish("iot22231/kelompokc/outTopic", "Welcome to Weather");
      // ... and resubscribe
      client.subscribe("iot22231/kelompokc/inTopic");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void setup()
{
  Serial.begin(115200);
  Serial.println(F("DHT11 test!"));

  #ifdef ESP8266
    configTime(0, 0, "pool.ntp.org");
    //secured_client.setTrustAnchors(cert);
  #endif

  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);

  dht.begin();

  // attempt to connect to Wifi network:
  Serial.print("Connecting to Wifi SSID ");
  Serial.print(WIFI_SSID);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  #ifdef ESP32
    secured_client.setCACert(TELEGRAM_CERTIFICATE_ROOT); // Add root certificate for api.telegram.org
  #endif
  while (WiFi.status() != WL_CONNECTED)
  {
    Serial.print(".");
    delay(500);
  }
  Serial.print("\nWiFi connected. IP address: ");
  myIP = WiFi.localIP();
  Serial.println(myIP);
  
  /*OTA ESP32 Inisialization*/
  OTA_Update();

  //PubSubClient
    pinMode(led, OUTPUT);     // Initialize the led pin as an output
    //setup_wifi();
    client.setServer(mqtt_server, 1883);
    client.setCallback(callback);
  
  /*mDNS Inisialization*/
  if (MDNS.begin("esp8266-came")) {
    Serial.println("MDNS responder started");
  }
  server.begin();
  Serial.println("HTTP server started");
  MDNS.addService("http", "tcp", 80);
}


void loop()
{
    humidity = dht.readHumidity();
    // Read temperature as Celsius (the default)
    temperatureC = dht.readTemperature();
    // Read temperature as Fahrenheit (isFahrenheit = true)
    temperatureF = dht.readTemperature(true);

    if (millis() - bot_lasttime > BOT_MTBS)
    {
        int numNewMessages = bot.getUpdates(bot.last_message_received + 1);
        while (numNewMessages)
        {
            Serial.println("got response");
            handleNewMessages(numNewMessages);
            numNewMessages = bot.getUpdates(bot.last_message_received + 1);
        }
        bot_lasttime = millis();
    }

    if (WiFi.status() == WL_CONNECTED)
    {
        HTTPClient http;
        String url = "https://script.google.com/macros/s/" + GOOGLE_SCRIPT_ID + "/exec?read";
        Serial.println("Making a request");
        http.begin(secured_client, String(myIP)); // Specify the URL and certificate
        http.setFollowRedirects(HTTPC_STRICT_FOLLOW_REDIRECTS);
        int httpCode = http.GET();
        String payload;
        if (httpCode > 0)
        { // Check for the returning code
            payload = http.getString();
            Serial.println(httpCode);
            Serial.println(payload);
        }
        else
        {
            Serial.println("Error on HTTP request");
        }
        http.end();
    }
    delay(1000);
    if (!client.connected()) {
    reconnect();
  }
  client.loop();

  unsigned long now = millis();
  if (now - lastMsg > 5000) {
    lastMsg = now;
    client.publish("iot22231/kelompokc/outTopic", msg);
    Serial.print("Kondisi Lampu :");
    Serial.println(led);
    client.publish("iot22231/kelompokc/Lamp", String(value).c_str());
    
    //delay(dht.getMinimumSamplingPeriod());
    //humidity = dht.getHumidity();
    //temperature = dht.getTemperature();
    String IP = String(WiFi.localIP().toString()).c_str();
    String ID = String(ESP.getFlashChipMode()).c_str();
    Serial.print("Suhu :");
    Serial.println(temperatureC);
    client.publish("iot22231/kelompokc/Temperature", String(temperatureC).c_str());
    
    Serial.print("Kelembaban :");
    Serial.println(humidity);
    client.publish("iot22231/kelompokc/Humidity", String(humidity).c_str());
    
    Serial.print("Alamat IP :");
    Serial.println(IP);
    client.publish("iot22231/kelompokc/ipaddress", String(IP).c_str());
    Serial.print("ChipID :");
    Serial.println(ID);
    client.publish("iot22231/kelompokc/chipid", String(ID).c_str());
    ++value;
    snprintf (msg, MSG_BUFFER_SIZE, "Welcome to Weather #%d", value);
    Serial.print("Publish message: ");
    Serial.println(msg);
    client.publish("outTopic", msg);
  }
}
