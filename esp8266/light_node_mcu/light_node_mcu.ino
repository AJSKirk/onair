#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClientSecure.h>
#include <time.h>

const char* SSID_NAME = "PLS_NO_WARDRIVE";
const char* SSID_PASSWD = "PLS_NO_WARDRIVE";

const char* GCAL_URL = "https://www.googleapis.com/calendar/v3/freeBusy";
const char* GCAL_KEY = "LOVE";
const char* GCAL_FINGERPRINT = "EACH"; // SHA-1

const char* EMAIL = "OTHER";

int URL_BUFFER = 128;
int BODY_BUFFER = 512;
int MAX_GCAL_APPOINTMENTS = 24;

struct gcal_time {
  time_t start;
  time_t end;
};

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
  Serial.println(" Time received");
  
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);
  
  get_gcal_times("ITS_EXPIRED_ANYWAY");
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

struct gcal_time *get_gcal_times(char *token) {
  HTTPClient http;
  WiFiClientSecure client;
  
  char *url = (char *) malloc(URL_BUFFER * sizeof(char));
  char *bearer_header = (char *) malloc((strlen(token) + 8) * sizeof(char));
  char *response = (char *) malloc(BODY_BUFFER * sizeof(char));
  char *brkr, *brkt;  // strtok contextx
  struct gcal_time *times = (struct gcal_time *) malloc(MAX_GCAL_APPOINTMENTS * sizeof(struct gcal_time));
  char *next_time, *time_str;
  char *body = gcal_req_body();
  struct tm *timestruct = (struct tm *) malloc(sizeof(struct tm));
  int response_code, i;

  sprintf(url, "%s?key=%s", GCAL_URL, GCAL_KEY);
  sprintf(bearer_header, "Bearer %s", token);

  client.setInsecure(); // DANGER - Fine for the light but don't use on anything more serious
  client.connect(url, 443);
  
  http.begin(client, url);
  http.addHeader("Content-Type", "text/plain");
  http.addHeader("Accept", "text/plain");
  http.addHeader("Authorization", bearer_header);

  response_code = http.POST(body);
  http.getString().toCharArray(response, BODY_BUFFER);
  http.end();

  strtok_r(response, "[", &brkr);
  while (next_time=strtok_r(NULL, "}]", &brkr)) {
    strtok_r(next_time, ":", &brkt);
    strtok_r(NULL, "\"", &brkt);
    time_str = strtok_r(NULL, "\"", &brkt);
    if (time_str == NULL)
      break;
      
    strptime(time_str, "%Y-%m-%dT%TZ", timestruct);
    times[i].start = mktime(timestruct);
    
    strtok_r(NULL, ":", &brkt);
    strtok_r(NULL, "\"", &brkt);
    time_str = strtok_r(NULL, "\"", &brkt);
    strptime(time_str, "%Y-%m-%dT%TZ", timestruct);
    times[i].end = mktime(timestruct);
  
    i++;
  }
  Serial.println("Y");
  free(response);
  free(body);
  free(url);
  free(bearer_header);
  return times;
}
