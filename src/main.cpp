#ifdef ESP32
#include <WiFi.h>
#else
#include <ESP8266WiFi.h>
#endif

#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>
#include <HTTPClient.h>
#include "DHT.h"
#include "time.h"

#define DHTPIN 13

#define DHTTYPE DHT11

// Wifi network station credentials
#define WIFI_SSID "MobilButut"
#define WIFI_PASSWORD "123456789"

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
DHT dht(DHTPIN, DHTTYPE);

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

void setup()
{
    Serial.begin(9600);
    Serial.println(F("DHTxx test!"));

#ifdef ESP8266
    configTime(0, 0, "pool.ntp.org");
    secured_client.setTrustAnchors(cert);
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
    Serial.println(WiFi.localIP());
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
