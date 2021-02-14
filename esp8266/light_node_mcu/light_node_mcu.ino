#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClientSecure.h>
#include <time.h>
#include <EEPROM.h>

#define LED 5

const char* SSID_NAME = "PLS_NO_WARDRIVE";
const char* SSID_PASSWD = "PLS_NO_WARDRIVE";

const char* GCAL_URL = "https://www.googleapis.com/calendar/v3/freeBusy";
const char* GCAL_KEY = "TING";
const char* GCAL_OAUTH_ID = "TANG";
const char* GCAL_OAUTH_SECRET = "WALLA WALLA";

char* GCAL_REFRESH_TOKEN = "BING";

const char* EMAIL = "BANG";

int URL_BUFFER = 128;
int BODY_BUFFER = 512;
int MAX_GCAL_APPOINTMENTS = 24;

struct gcal_time {
  time_t start;
  time_t end;
};

struct gcal_token {
  char token[256];
  time_t expires_at;
};

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);  // Set up printer to computer for debugging
  delay(10);
  Serial.println('\n');

  wifi_set_sleep_type(LIGHT_SLEEP_T);
  EEPROM.begin(512);
  pinMode(LED, OUTPUT);

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
}

void loop() {
  // put your main code here, to run repeatedly:
  struct gcal_token token;
  struct gcal_time *times;
  int i;
  int awake = 0;
  time_t now;
  
  set_time();
  EEPROM.get(0, token);

  do { // Loop until sleep trigger fires
    awake = 0;
    if (time(NULL) >= token.expires_at)
      token = gcal_refresh(GCAL_REFRESH_TOKEN);
    
    times = get_gcal_times(token.token);
    now = time(NULL);
    
    for (i=0; i<MAX_GCAL_APPOINTMENTS; i++) {
      Serial.print("Time ");
      Serial.print(i);
      Serial.print(" starts at ");
      Serial.print(times[i].start);
      Serial.print(" and ends at ");
      Serial.print(times[i].end);
      Serial.println("\n");
      if (times[i].start == 0)
        break;
      else if (now >= times[i].start && now <= times[i].end) {
        awake = 1;
        digitalWrite(LED, HIGH);
      }
    }
    delay(1000 * 60);
  } while (awake);

  Serial.println("Not in an event, going to sleep now");
  
  digitalWrite(LED, LOW);
  EEPROM.put(0, token);
  EEPROM.commit();
  ESP.deepSleep(5 * 60 * 1e6); // Ensure you have GPIO16 wired to RST!
  return;
}

void set_time(void) {
  configTime(0, 0, "pool.ntp.org", "time.nist.gov");
  Serial.print("\nWaiting for time ");
  while (time(nullptr) < 3600) { 
    Serial.print(".");
    delay(1000);
  }
  Serial.println(" Time received");
  return;
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
  Serial.println("Getting times");
  
  char *url = (char *) malloc(URL_BUFFER * sizeof(char));
  char *bearer_header = (char *) malloc((strlen(token) + 8) * sizeof(char));
  char *response = (char *) malloc(BODY_BUFFER * sizeof(char));
  char *brkr, *brkt;  // strtok contextx
  struct gcal_time *times = (struct gcal_time *) calloc(MAX_GCAL_APPOINTMENTS, sizeof(struct gcal_time));
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
  free(response);
  free(body);
  free(url);
  free(bearer_header);

  Serial.println("Got times");
  return times;
}

struct gcal_token gcal_refresh(char *refresh_token) {
  HTTPClient http;
  WiFiClientSecure client;
  Serial.println("Refreshing token");
  
  char *url = "https://oauth2.googleapis.com/token";
  char *body = (char *) malloc(BODY_BUFFER * sizeof(char));
  int response_code;
  char *response = (char *) malloc(BODY_BUFFER * sizeof(char));
  gcal_token result;

  sprintf(body, "client_id=%s&client_secret=%s&grant_type=refresh_token&refresh_token=%s", GCAL_OAUTH_ID, GCAL_OAUTH_SECRET, refresh_token);

  client.setInsecure();
  client.connect(url, 443);

  http.begin(client, url);
  http.addHeader("Content-Type", "application/x-www-form-urlencoded");

  response_code = http.POST(body);
  http.getString().toCharArray(response, BODY_BUFFER);
  http.end();

  strtok(response, ":");
  strtok(NULL, "\"");
  strncpy(result.token, strtok(NULL, "\""), 256);
  strtok(NULL, ":");
  result.expires_at = time(NULL) + atoi(strtok(NULL, ","));

  Serial.println("Refreshed token");
  return result;
}
