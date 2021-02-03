#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClientSecure.h>
#include <time.h>

const char* SSID_NAME = "PLS_NO_WARDRIVE";
const char* SSID_PASSWD = "PLS_NO_WARDRIVE";

const char* GCAL_URL = "LOVE";
const char* GCAL_KEY = "EACH";
const char* EMAIL = "OTHER";

int URL_BUFFER = 128;
int BODY_BUFFER = 256;

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

  
  configTime(0, 0, "pool.ntp.org", "time.nist.gov");
  Serial.print("\nWaiting for time ");
  while (time(nullptr) < 3600) { 
    Serial.print(".");
    delay(1000);
  }
  Serial.println("Time received");
  
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);
  
  get_gcal("ITS_EXPIRED_ANYWAY");
}

void loop() {
  // put your main code here, to run repeatedly:

}

char *gcal_req_body(void) {
  time_t now = time(NULL);
  struct tm *timestruct = gmtime(&now);
  char start_time[50], end_time[50];
  char *body = (char *) malloc(BODY_BUFFER * sizeof(char));

  timestruct->tm_hour = 0;
  timestruct->tm_min = 0;
  strftime(start_time, 50, "%Y-%m-%dT%T.000Z", timestruct);
  
  timestruct->tm_hour = 23;
  timestruct->tm_min = 59;
  strftime(end_time, 50, "%Y-%m-%dT%T.000Z", timestruct);

  sprintf(body, "{\"timeMin\": \"%s\", \"timeMax\": \"%s\", \"timeZone\": \"UTC\", \"items\": [{\"id\": \"%s\"}]}", start_time, end_time, EMAIL);
  return body;
}

String get_gcal(char *token) {
  HTTPClient http;
  WiFiClientSecure client;
  
  char *url = (char *) malloc(URL_BUFFER * sizeof(char));
  char *bearer_header = (char *) malloc((strlen(token) + 8) * sizeof(char));
  char *body = gcal_req_body();
  int response_code;

  Serial.println(body);
  sprintf(url, "%s?key=%s", GCAL_URL, GCAL_KEY);
  sprintf(bearer_header, "Bearer %s", token);

  client.setInsecure(); // DANGER - Fine for the light but don't use on anything more serious
  client.connect(url, 443);
  
  http.begin(client, url);
  http.addHeader("Content-Type", "text/plain");
  http.addHeader("Accept", "text/plain");
  http.addHeader("Authorization", bearer_header);

  response_code = http.POST(body);
  String response = http.getString();

  Serial.println(response);
  http.end();
  free(body);
  free(url);
  free(bearer_header);
  return response;
}
