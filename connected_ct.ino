/*
 *  https://learn.openenergymonitor.org/electricity-monitoring/ct-sensors/how-to-build-an-arduino-energy-monitor-measuring-current-only
 */
#include <ESP8266WiFi.h>
#include <PubSubClient.h>

const char* ssid = "cookie2";
const char* password = "0317137263";
const char* mqtt_server = "damoa.io";

WiFiClient espClient;
PubSubClient client(espClient);

#define PORT A0

#include "EmonLib.h"
EnergyMonitor emon1;
unsigned long mark, s = 0;

void setup_wifi() {
  Serial.printf("\nConnecting to ");
  Serial.print(ssid);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  randomSeed(micros());

  Serial.printf("\nWiFi connected");
  Serial.printf("\nIP address: ");
  Serial.println(WiFi.localIP());
}

void setup() {
  Serial.begin(115200);

  Serial.printf("\n\nBegin Connected CT");
  emon1.current(0, 111.1);
  for (int n=0;n<10;n++) emon1.calcIrms(1480);

  setup_wifi();
  Serial.printf("\ngot local IP address ");
  Serial.println(WiFi.localIP());
  client.setServer(mqtt_server, 1883);
  mark = millis() + 1000;
}

void reconnect() {
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    String clientId = "ewha-power-monitor-";
    clientId += String(random(0xffff), HEX);

    if (client.connect(clientId.c_str())) {
      Serial.println("connected");
      client.publish("ewha/power/washer/checkin/1", "starting");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);
    }
  }
}

void loop() {
  double w;
  if (!client.connected()) reconnect();
  client.loop();

  if( millis() > mark ) {
    mark = millis() + 1000;

    double w=emon1.calcIrms(1480)*220;
    String msg = String("{\"washer\":\"1\", \"watt\":")+String(w)+"}";
    client.publish("ewha/power/washer/data/1", msg.c_str());
    Serial.println(String("ewha/power/washer/1") + msg);
  }
}
