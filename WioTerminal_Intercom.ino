// Wio Terminal Intercom
// Author: Jonathan Tan
// Feb 2021
// Written for Seeed Wio Terminal

#include <rpcWiFi.h>
#include <PubSubClient.h>
#include <RTC_SAMD51.h>
#include <DateTime.h>
#include <TFT_eSPI.h>
#include "Free_Fonts.h"

TFT_eSPI tft;
TFT_eSprite time_text(&tft);
TFT_eSprite date_text(&tft);
TFT_eSprite inmsg(&tft);
TFT_eSprite sp(&tft);
TFT_eSprite sp2(&tft);
TFT_eSprite sp3(&tft);

RTC_SAMD51 rtc;

// Hardware Setup
const auto clicker_button = WIO_5S_PRESS;
const auto audio_button = WIO_KEY_C;
int clicker_state = 0;

// Constant & Variable Declarations
int selection_choice = 0;
int num_messages = 4;
const char *messages[] = {
  "I need help!",
  "Food is ready!",
  "Give me 5 minutes!",
  "I'll take awhile."
};

String year, month, day, hour, minute, second;
const char *months[] = {
    "Jan", "Feb", "Mar", "Apr", "May", "Jun",
    "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"
};
DateTime now;

// Network Settings
const char *ssid = "YourWiFiSSID";
const char *password = "YourPassword";

const char *ID = "WioTerminal1";  // Must be unique
const char *PUBTOPIC = "WioTerminal1/messages";
const char *SUBTOPIC = "+/messages";

IPAddress broker("YourBrokerIP"); // IP address of your MQTT broker
WiFiClient wclient;
PubSubClient client(wclient); // Setup MQTT client

// MQTT & WiFi Functions

void setup_wifi() {
  tft.print("\nConnecting to ");
  tft.println(ssid);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
      millisDelay(2000);
      tft.print(".");
  }
  tft.println();
  tft.println("WiFi connected");
  tft.print("IP address: ");
  tft.println(WiFi.localIP());
  millisDelay(10000);
  tft.fillScreen(TFT_BLACK);
}

void reconnect() {
  while (!client.connected()) {
    tft.drawString("Attempting MQTT connection...", 0, 0);
    if (client.connect(ID)) {
      tft.fillRect(0,0,240,10,TFT_BLACK);
      tft.drawString("Status: Connected", 0, 0);
      client.subscribe(SUBTOPIC);
    } else {
      tft.drawString("Failed: ", 0, 0); 
      tft.drawString(String(client.state()), 20, 0);
      tft.drawString(". Try again in 5 seconds", 25, 0);
      millisDelay(5000);
      tft.fillRect(0,0,240,50,TFT_BLACK);
    }
  }
}

void callback(char* topic, byte* payload, unsigned int length) {
  String response, identity;
  if (payload && strcmp(topic,PUBTOPIC) != 0) {
    for (int i = 0; i < length; i++) {
      response += (char)payload[i];
    }
    for (int i = 0; i < (strlen(topic)-9) ; i++) {
      identity += (char)topic[i];
    }
    fadetxt("Message from: " + identity, FSS9, 40, 250, 240, TC_DATUM);
    fadetxt(response, FSS9, 40, 280, 240, TC_DATUM);
    alarm();
  }
}

void alarm() {
   for (int i=0; i<3; i++) {
      analogWrite(WIO_BUZZER, 150);
      delay(80);
      analogWrite(WIO_BUZZER, 0);
      delay(80);
  }
}

void millisDelay(int duration) {
    long timestart = millis();
    while ((millis()-timestart) < duration) {};
}

// Graphics Functions

void displayDateTime() {
  now = rtc.now();
  year = String(now.year(), DEC);
  month = months[String(now.month(), DEC).toInt()-1];
  day = String(now.day(), DEC);
  hour = String(now.hour(), DEC);
  minute = String(now.minute(), DEC);
  
  if (hour.length() < 2) hour = "0" + hour;
  if (minute.length() < 2) minute = "0" + minute;
  
  time_text.createSprite(120, 40);
  time_text.setTextDatum(TC_DATUM);
  time_text.setTextPadding(120);
  time_text.setFreeFont(FSSB24);
  time_text.drawString(hour + ":" + minute, 0 ,0);
  time_text.pushSprite(60,100);

  date_text.createSprite(120, 30);
  date_text.setTextDatum(TC_DATUM);
  date_text.setTextPadding(120);
  date_text.setFreeFont(FSSB12);
  date_text.drawString(day + " " + month + " " + year, 0 ,0);
  date_text.pushSprite(60,150);
}

void fadetxt(String text, auto font, int x, int y, int sprite_length, byte datum) {
  sp.createSprite(sprite_length, 20);
  sp.setTextDatum(datum);
  sp.setFreeFont(font);
  sp.setTextColor(TFT_BLACK);
  
  int r = 0; int g = 0; int b = 0;
  
  for (int j = 0; j < 127; j++) {
    int color = sp.color565(r += 2, g += 2, b += 2);
    sp.setTextColor(color);
    if (datum == TL_DATUM) sp.drawString(text, 0, 0);
    if (datum == TC_DATUM) sp.drawString(text, 80, 0);
    sp.pushSprite(x,y);
  }
  sp.deleteSprite();
}

void fadeouttxt(String text, auto font, int x, int y, int sprite_length, byte datum) {
  sp.createSprite(sprite_length, 20);
  sp.setTextDatum(datum);
  sp.setFreeFont(font);
  sp.setTextColor(TFT_WHITE);
  
  int r = 255; int g = 255; int b = 255;
  
  for (int j = 0; j < 127; j++) {
    int color = sp.color565(r -= 2, g -= 2, b -= 2);
    sp.setTextColor(color);
    if (datum == TL_DATUM) sp.drawString(text, 0, 0);
    if (datum == TC_DATUM) sp.drawString(text, 80, 0);
    sp.pushSprite(x,y);
  }
  sp.deleteSprite();
}

void triangles() {
  if (selection_choice == 0) {
    sp2.createSprite(13,13);
    sp2.fillTriangle(0,6,6,0,6,12,TFT_BLACK);
    sp2.pushSprite(10,281);
    sp3.createSprite(13,13);
    sp3.fillTriangle(12,6,6,0,6,12,TFT_WHITE);
    sp3.pushSprite(218,281);
  } else if (selection_choice == num_messages-1) {
    sp2.createSprite(13,13);
    sp2.fillTriangle(0,6,6,0,6,12,TFT_WHITE);
    sp2.pushSprite(10,281);
    sp3.createSprite(13,13);
    sp3.fillTriangle(12,6,6,0,6,12,TFT_BLACK);
    sp3.pushSprite(218,281);
  } else {
    sp2.createSprite(13,13);
    sp2.fillTriangle(0,6,6,0,6,12,TFT_WHITE);
    sp2.pushSprite(10,281);
    sp3.createSprite(13,13);
    sp3.fillTriangle(12,6,6,0,6,12,TFT_WHITE);
    sp3.pushSprite(218,281);
  }
  sp2.deleteSprite();
  sp3.deleteSprite();
}

void deletetriangles() {
  sp2.createSprite(13,13);
  sp3.createSprite(13,13);
  sp2.fillTriangle(0,6,6,0,6,12,TFT_BLACK);
  sp3.fillTriangle(12,6,6,0,6,12,TFT_BLACK);
  sp2.pushSprite(10,281);
  sp3.pushSprite(218,281);
}

void msgselection() {
  sp2.createSprite(13,13);
  sp3.createSprite(13,13);
  int r = 0; int g = 0; int b = 0;
  if (selection_choice == 0) {
    for (int j = 0; j < 127; j++) {
      int color = sp.color565(r += 2, g += 2, b += 2);
      sp3.fillTriangle(12,6,6,0,6,12,color);
      sp3.pushSprite(218,281);
    }
  } else if (selection_choice == num_messages-1) {
    for (int j = 0; j < 127; j++) {
      int color = sp.color565(r += 2, g += 2, b += 2);
      sp2.fillTriangle(0,6,6,0,6,12,color);
      sp2.pushSprite(10,281);
    }
  } else {
    for (int j = 0; j < 127; j++) {
      int color = sp.color565(r += 2, g += 2, b += 2);
      sp2.fillTriangle(0,6,6,0,6,12,color);
      sp3.fillTriangle(12,6,6,0,6,12,color);
      sp2.pushSprite(10,281);
      sp3.pushSprite(218,281);
    }
  }
  sp2.deleteSprite();
  sp3.deleteSprite();
}

void setup() {
  pinMode(WIO_5S_PRESS,INPUT_PULLUP);
  pinMode(WIO_5S_UP,INPUT_PULLUP);
  pinMode(WIO_5S_DOWN,INPUT_PULLUP);
  pinMode(WIO_5S_LEFT,INPUT_PULLUP);
  pinMode(WIO_5S_RIGHT,INPUT_PULLUP);
  pinMode(WIO_BUZZER, OUTPUT);
  
  // Configure SWITCH_Pin as an input
  // Display Setup
  tft.init();
  tft.setRotation(2);
  tft.fillScreen(TFT_BLACK);

  // Connect to WiFi & Establish Server
  setup_wifi(); // Connect to network

  rtc.begin();
  now = DateTime(F(__DATE__), F(__TIME__));
  rtc.adjust(now);
  client.setServer(broker, 1883);
  client.setCallback(callback);
}

void loop() {
  
  if (!client.loop()) reconnect();
  displayDateTime();
  
  // Send Message Menu
  if (digitalRead(WIO_5S_LEFT) == LOW) {
    tft.fillRect(0,250,240,200,TFT_BLACK);
    fadetxt("Send Message:", FSS9, 10, 250, 150, TL_DATUM);
    fadetxt(messages[selection_choice], FSS9, 40, 280, 160, TC_DATUM);
    msgselection();
    while (clicker_state == 0) {
      client.loop();
      displayDateTime();
      if (digitalRead(WIO_5S_DOWN) == LOW) {
        selection_choice --;
        if (selection_choice < 0) { selection_choice = 0; continue; }
        triangles();
        fadetxt(messages[selection_choice], FSS9, 40, 280, 160, TC_DATUM);
        millisDelay(100);
      }
      if (digitalRead(WIO_5S_UP) == LOW) {
        selection_choice ++;
        if (selection_choice == num_messages) { selection_choice = num_messages-1; continue; }
        triangles();
        fadetxt(messages[selection_choice], FSS9, 40, 280, 160, TC_DATUM);
        millisDelay(100);
      }
      if (digitalRead(WIO_5S_RIGHT) == LOW) {
        clicker_state = 1;
        deletetriangles();
        fadeouttxt(messages[selection_choice], FSS9, 40, 280, 160, TC_DATUM);
        fadeouttxt("Send Message:", FSS9, 10, 250, 150, TL_DATUM);
        millisDelay(100);
      }
      if (digitalRead(WIO_5S_PRESS) == LOW) {
        clicker_state = 1;
        // MQTT Communication
        deletetriangles();
        client.publish(PUBTOPIC, messages[selection_choice]);
        fadeouttxt("Send Message:", FSS9, 10, 250, 150, TL_DATUM);
        fadetxt("Message Sent!", FSS9, 40, 280, 160, TC_DATUM);
        millisDelay(1000);
        fadeouttxt("Message Sent!", FSS9, 40, 280, 160, TC_DATUM);
      }
    }
    clicker_state = 0;
    tft.fillRect(0,250,240,200,TFT_BLACK);
  }
  millisDelay(200);
}
