#ifdef ESP32
#include <WiFi.h>
#include <ESPmDNS.h>
#else
#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#endif

#include "Arduino.h"
#include <WiFiUdp.h>
#include "OTAupdate.h"
#include "webConfig.h"
#include "MQTT_Config.h"
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>
#include <HTTPClient.h>
#include "DHT.h"
#include "time.h"

#define DHTPIN 13

#define DHTTYPE DHT11

// Wifi network station credentials
#ifndef STASSID
#define STASSID "DM"
#define STAPSK "kamardimana"
#endif

const char* host = "esp32-cyberfreak-webupdate";
const char* ssid = "DM";
const char* password = "kamardimana";
const char* mqtt_server = "ee.unsoed.ac.id";

//#define WIFI_SSID "MobilButut"
//#define WIFI_PASSWORD "123456789"

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
            /*if (text == "/tempF")
            {
                String msg = "Temperature is ";
                msg += msg.concat(temperatureF);
                msg += "F";
                bot.sendMessage(chat_id, msg, "");
            }*/
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

void setup()
{
    Serial.begin(9600);
    Serial.println(F("DHTxx test!"));
    delay(100);

#ifdef ESP8266
    configTime(0, 0, "pool.ntp.org");
    secured_client.setTrustAnchors(cert);
#endif

    configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);

    pinMode(led, INPUT);
    dht.begin();

    pinMode(led, OUTPUT);
    digitalWrite(led, 0);

    // attempt to connect to Wifi network:
    /*Serial.print("Connecting to Wifi SSID ");
    Serial.print(WIFI_SSID);
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);*/

    WiFi.mode(WIFI_STA);
    Serial.println("Connecting to ");
    Serial.println(ssid);
    WiFi.begin(ssid, password);
    Serial.println("");
#ifdef ESP32
    secured_client.setCACert(TELEGRAM_CERTIFICATE_ROOT); // Add root certificate for api.telegram.org
#endif
    while (WiFi.status() != WL_CONNECTED)
    {
        Serial.print(".");
        delay(500);
    }
    Serial.print("\nWiFi connected. IP address: ");
    Serial.println(WiFi.localIP());

    // web server
    server.on("/", handleRoot);

    server.on("/inline", [](){
        server.send(200, "text/plain", "this works as well");
    });

    server.on("/led_hidup", [](){
        server.send(200, "text/plain", "LED Hidup");
        Serial.println();
        Serial.println("LED Hidup");
        digitalWrite(led, HIGH);
    });

    server.on("/led_mati", [](){
        server.send(200, "text/plain", "LED Padam");
        Serial.println();
        Serial.println("LED Padam");
        digitalWrite(led, LOW);   
    });

    server.onNotFound(handleNotFound);

    //ota 32
    OTA_Update();

    //mDNS
    if (MDNS.begin("esp32-cyberfreak")){
        Serial.println("MDNS responder started");
    }
    server.begin();
    Serial.println("HTTP server started");

    for(int i=0; i<17; i=i+8){
        chipId |= ((ESP.getEfuseMac()>>(40 - i)) & 0xff) << i;
    };

    //http updater
    httpUpdater.setup(&server);
    server.begin();

    MDNS.addService("http", "tcp", 80);
    Serial.printf("HTTPUpdateServer ready! Open http://%s.local/update in your browser\n", host);

    //mqtt
    client.setServer(mqtt_server, 1883);
    client.setCallback(callback);
}

void loop(void)
{
    server.handleClient();

    ArduinoOTA.handle();

    value = digitalRead(led);

    humidity = dht.readHumidity();
    // Read temperature as Celsius (the default)
    temperatureC = dht.readTemperature();
    //Read temperature as Fahrenheit (isFahrenheit = true)
    temperatureF = dht.readTemperature(true);

    if (!client.connected()){
        reconnect();
    }
    client.loop();

    unsigned long now = millis();
    if (now - lastMsg > 1000) {
        lastMsg = now;
        snprintf(msg, MSG_BUFFER_SIZE, "STATUS LED = %d", status_led);
        Serial.print("Publish message: ");
        Serial.println(msg);
        client.publish("outTopic", msg);

        client.publish("iot22231/kelompokc/LED", String(value).c_str());
        
        readHumidity = dht.readHumidity();
        readTemperature = dht.readTemperature();
        
        String IP = String(WiFi.localIP().toString()).c_str();
        String ID = String(chipId).c_str();
        
        Serial.print("Suhu :");
        Serial.println(readTemperature);
        client.publish("iot22231/kelompokc/suhu", String(readTemperature).c_str());
        
        Serial.print("Kelembapan :");
        Serial.println(readHumidity);
        client.publish("iot22231/kelompokc/kelembaban", String(readHumidity).c_str());
        
        Serial.print("Alamat IP :");
        Serial.println(IP);
        client.publish("iot22231/kelompokc/ipaddress", String(IP).c_str());
        
        Serial.print("ChipID :");
        Serial.println(ID);
        client.publish("iot22231/kelompokc/chipid", String(ID).c_str());
        
        Serial.println("");
    }
    
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
        http.begin(url.c_str()); // Specify the URL and certificate
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
}
