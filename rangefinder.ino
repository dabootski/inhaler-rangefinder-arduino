#include <Console.h>
#include <Bridge.h>
#include <HttpClient.h>

//
// Constants
//
const int pingPin = 7;
HttpClient client;

//
// Settings
//
int loopTimeout; // In milliseconds
int settingsRefreshTimeoutInSec; // In seconds
int withinRangeTimeoutInSec; // In seconds
int rangeThreshold; // In inches
int notificationTimeout; // In seconds

//
// Variables
//
unsigned long lastSettingsRefresh = millis();
unsigned long secondsSinceLastRefresh;
float outOfRangeSince;
float withinRangeSince;

//
// Setup
//
void setup() {
  // Initialize serial communication
  Bridge.begin();
  Console.begin();
  
  while(!Console) {
    ; // Wait for Console port to connect
  }
  Console.println("You're connected to the Console!!!!");
  configureSettings();
}

//
// Main loop
//
void loop() {
  float inches = readDistance();
  
  Console.println(inches);
  
  // Determine if we need to fetch new settings from API
  secondsSinceLastRefresh = (millis() - lastSettingsRefresh) / 1000;
  if(secondsSinceLastRefresh >= settingsRefreshTimeoutInSec) {
    configureSettings();
  }

  delay(loopTimeout);
}

//
// Helper functions
//
float readDistance() {
  // The PING))) is triggered by a HIGH pulse of 2 or more microseconds.
  // Give a short LOW pulse beforehand to ensure a clean HIGH pulse.
  pinMode(pingPin, OUTPUT);
  digitalWrite(pingPin, LOW);
  delayMicroseconds(2);
  digitalWrite(pingPin, HIGH);
  delayMicroseconds(5);
  digitalWrite(pingPin, LOW);

  // The same pin is used to read the signal from the PING))): a HIGH
  // pulse whose duration is the time (in microseconds) from the sending
  // of the ping to the reception of its echo off of an object.
  pinMode(pingPin, INPUT);

  return microsecondsToInches(pulseIn(pingPin, HIGH));
}

void configureSettings() {
  Console.println("Configuring settings...");
  client.get("http://10.0.1.23:3000/api/devices/1/settings");
  
  int populatingKey = 1;
  String responseBody = "";
  String key = "";
  String value = "";

  while (client.available()) {
    char c = client.read();
    responseBody += c;

    if(populatingKey == 1) {
      if(c == ':') {
        populatingKey = 0;
      } else {
        key += c;
      }
    } else { // Populating value
      if(c == ':') {
        populatingKey = 1;
        assignSetting(key, value);
        key = "";
        value = "";
      } else {
        value += c;
      }
    }
  }

  assignSetting(key, value);
  lastSettingsRefresh = millis();
  debugSettings();
}

void debugSettings() {
  Console.println("***************************");  
  Console.println("SETTINGS:");
  Console.print("notificationTimeout: ");
  Console.println(notificationTimeout);
  Console.print("loopTimeout: ");
  Console.println(loopTimeout);
  Console.print("settingsRefreshTimeoutInSec: ");
  Console.println(settingsRefreshTimeoutInSec);
  Console.print("withinRangeTimeoutInSec: ");
  Console.println(withinRangeTimeoutInSec);
  Console.print("rangeThreshold: ");
  Console.println(rangeThreshold);
  Console.println("\n***************************"); 
}

void assignSetting(String key, String value) {
  Console.println("ASSIGNING SETTING!");
  Console.println("KEY: " + key);
  Console.println("VALUE: " + value);

  if(key == "notification_timeout") {
    notificationTimeout = value.toInt();
  }
  if(key == "loop_timeout") {
    loopTimeout = value.toInt();
  }
  if(key == "settings_refresh_timeout") {
    settingsRefreshTimeoutInSec = value.toInt();
  }
  if(key == "within_range_timeout_in_sec") {
    withinRangeTimeoutInSec = value.toInt();
  }
  if(key == "range_threshold") {
    rangeThreshold = value.toInt();
  }
}

float microsecondsToInches(float microseconds) {
  // According to Parallax's datasheet for the PING))), there are
  // 73.746 microseconds per inch (i.e. sound travels at 1130 feet per
  // second).  This gives the distance travelled by the ping, outbound
  // and return, so we divide by 2 to get the distance of the obstacle.
  // See: http://www.parallax.com/dl/docs/prod/acc/28015-PING-v1.3.pdf
  return microseconds / 74 / 2;
}

