#include <ESP8266WiFi.h>

const char* SSID_NAME = "PLS_NO_WARDRIVE";
const char* SSID_PASSWD = "PLS_NO_WARDRIVE";


void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);  // Set up printer to computer for debugging
  delay(10);
  Serial.println('\n');

  WiFi.begin(SSID_NAME, SSID_PASSWD);
  Serial.print("Connecting to ");
  Serial.print(SSID_NAME);
  Serial.print(" ");

  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print('.');
  }

  Serial.println('\n');
  Serial.println("Connection established");
  Serial.print("IP address:\t");
  Serial.println(WiFi.localIP());
  digitalWrite(LED_BUILTIN, LOW);  // Low is ON for the builtin LED for some reason
}

void loop() {
  // put your main code here, to run repeatedly:

}
