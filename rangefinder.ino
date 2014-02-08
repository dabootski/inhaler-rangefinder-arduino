#include <Console.h>
#include <Bridge.h>
#include <HttpClient.h>

// this constant won't change.  It's the pin number
// of the sensor's output:
const int pingPin = 7;
int loopTimeout; // Every 5 seconds
const int settingsRefreshTimeout = 15000; // Every 15 seconds
unsigned long lastSettingsRefresh = millis();
unsigned long timeSinceLastRefresh;
const float minDistance = 12.0;
//int withinRangeTimeout = int(config[""]); # In seconds
//rangeThreshold = int(sys.argv[3]) # In centimeters
int notificationTimeout; // In seconds
//distanceCheckInterval = int(sys.argv[5])
//outOfRangeSince = None
//withinRangeSince = None
//lastNotifiedAt = time.time() - notificationTimeout
HttpClient client;

void setup() {
  // initialize serial communication:
  Bridge.begin();
  Console.begin();
  
  Console.println("You're connected to the Console!!!!");
  
  while (!Console){
    ; // wait for Console port to connect.
  }
  Console.println("You're connected to the Console!!!!");
  configureSettings();
}

void loop()
{
  // establish variables for duration of the ping, 
  // and the distance result in inches and centimeters:
  float duration, inches, cm;

  // The PING))) is triggered by a HIGH pulse of 2 or more microseconds.
  // Give a short LOW pulse beforehand to ensure a clean HIGH pulse:
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
  
  duration = pulseIn(pingPin, HIGH);
  inches = microsecondsToInches(duration);
  
  Console.println(inches);
  
  timeSinceLastRefresh = millis() - lastSettingsRefresh;
  
  if(timeSinceLastRefresh >= settingsRefreshTimeout)
  {
    configureSettings();
    lastSettingsRefresh = millis();
  }

  delay(loopTimeout);
}

void configureSettings()
{
  Console.println("Configuring settings...");
  client.get("http://10.0.1.23:3000/api/devices/1/settings");
  
  int populatingKey = 1;
  String responseBody = "";
  String key = "";
  String value = "";
  while (client.available()) {
    char c = client.read();
    responseBody += c;
    
    if(populatingKey == 1)
    {
      if(c == ':')
      {
        populatingKey = 0;
      } else
      {
        key += c;
      }
    } else // Populating value
    {
      if(c == ':')
      {
        populatingKey = 1;
        assignSetting(key, value);
        key = "";
        value = "";
      } else
      {
        value += c;
      }
    }
  }
  
  assignSetting(key, value);
  debugSettings();

  Console.println(sizeof(responseBody));
}

void debugSettings()
{
  Console.println("***************************");  
  Console.println("SETTINGS:");
  Console.print("notificationTimeout:");
  Console.print(notificationTimeout);
  Console.println("loopTimeout:");
  Console.print(loopTimeout);
  Console.println("\n***************************"); 
}

void assignSetting(String key, String value)
{
  Console.println("ASSIGNING SETTING!");
  Console.println("KEY: " + key);
  Console.println("VALUE: " + value);
  
  if(key == "notification_timeout")
  {
    Console.println("REMINDER TIMEOUT GETTING SET!!!");
    notificationTimeout = value.toInt();
  }
  if(key == "loop_timeout")
  {
    Console.println("LOOP TIMEOUT GETTING SET!!!");
    loopTimeout = value.toInt();
  }
  // TODO: CONFIGURE REST OF SETTINGS HERE!!!
  // TODO: CONFIGURE REST OF SETTINGS HERE!!!
  // TODO: CONFIGURE REST OF SETTINGS HERE!!!
}

float microsecondsToInches(float microseconds)
{
  // According to Parallax's datasheet for the PING))), there are
  // 73.746 microseconds per inch (i.e. sound travels at 1130 feet per
  // second).  This gives the distance travelled by the ping, outbound
  // and return, so we divide by 2 to get the distance of the obstacle.
  // See: http://www.parallax.com/dl/docs/prod/acc/28015-PING-v1.3.pdf
  return microseconds / 74 / 2;
}

