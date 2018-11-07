#include <FastLED.h>
#include <NTPtimeESP.h>

#define NUM_LEDS    49
CRGB leds[NUM_LEDS];

#define BRIGHTNESS          50
#define FRAMES_PER_SECOND  240

int FastLED_fade_counter = 0;
int litIndex = -1;

int targetIndex = -1;

NTPtime NTPch("ch.pool.ntp.org");   // Choose server pool as required
char *ssid      = "*****";               // Set you WiFi SSID
char *password  = "*****";               // Set you WiFi password
strDateTime dateTime;

void setup() {
  delay(500);
  Serial.begin(115200);

  // tell FastLED about the LED strip configuration
  FastLED.addLeds<WS2812, D2, GRB>(leds, NUM_LEDS).setCorrection(TypicalLEDStrip);
  FastLED.setBrightness(BRIGHTNESS);
  
  WiFi.mode(WIFI_STA);
  WiFi.begin (ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }
  Serial.println("WiFi connected");
  Serial.println(WiFi.localIP());

  NTPch.setRecvTimeout(5);
  NTPch.setSendInterval(5);
  getDate();
}

void loop()
{
  // send the 'leds' array out to the actual LED strip
  FastLED.show();

  if (litIndex < targetIndex) {
    fadeLedUp(litIndex + 1);
  }

  if (Serial.available()) {
    Serial.read();
    targetIndex++;
    if (targetIndex > 23) {
      litIndex = -1;
      targetIndex = -1;
    }
  }
  
  if (targetIndex == -1) {
    fadeAllToBlack();
  }

  FastLED.delay(1000 / FRAMES_PER_SECOND);

  EVERY_N_SECONDS(10) {
    getDate();
  }
}

void fadeLedUp (int index) {
  int indexA = 47 - (index * 2);
  int indexB = indexA + 1;

  if (index == 23) {
    indexA = 0;
    indexB = 2;
  }

  for ( int i = indexA; i <= indexB; i++ )
  {
    switch (dayOfWeek(dateTime.year, dateTime.month, index + 1)) {
      case 1:
        leds[i] = CRGB::Red;
        break;

      default:
        leds[i] = CRGB::Orange;
        break;
    }

    if (index == 23) {
      leds[i] = CRGB::Purple;
    }
    
    leds[i].maximizeBrightness(FastLED_fade_counter);  // 'FastLED_fade_counter' How high we want to fade up to 255 = maximum.
  }

  FastLED_fade_counter += 8 ;
  if (FastLED_fade_counter > 255) {
    litIndex ++;
    FastLED_fade_counter = 0;
  }
}

void fadeAllToBlack() {
  for ( int i = 0; i < NUM_LEDS; i++ ) {
    leds[i].fadeToBlackBy( 8 );
  }
}

void getDate() {
  dateTime = NTPch.getNTPtime(1.0, 1);

  // check dateTime.valid before using the returned time
  // Use "setSendInterval" or "setRecvTimeout" if required
  if (dateTime.valid) {
    NTPch.printDateTime(dateTime);

    if (dateTime.day <= 24 && dateTime.month == 12) {
      targetIndex = dateTime.day - 1;
    } else {
      targetIndex = -1;
      Serial.println("It's not december yet!");
    }
  } else {
    Serial.println("NTP request failed.");
  }
}

#define LEAP_YEAR(Y)     ( (Y>0) && !(Y%4) && ( (Y%100) || !(Y%400) ))     // from time-lib

int dayOfWeek(uint16_t year, uint8_t month, uint8_t day)
{
  uint16_t months[] =
  {
    0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334, 365 // days until 1st of month
  };

  uint32_t days = year * 365;   // days until year
  for (uint16_t i = 4; i < year; i += 4)
  {
    if (LEAP_YEAR(i)) days++;  // adjust leap years
  }

  days += months[month - 1] + day;
  if ((month > 2) && LEAP_YEAR(year)) days++;
  return days % 7;
}
