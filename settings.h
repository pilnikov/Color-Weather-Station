/**The MIT License (MIT)
Copyright (c) 2015 by Daniel Eichhorn
Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:
The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.
THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
See more at http://blog.squix.ch
*/

#include <Arduino.h>


#if defined(ESP8266)
#include <ESP8266WiFi.h>
#include <time.h>                       // time() ctime()
#include <sys/time.h>                   // struct timeval
#endif

#if defined(ESP32)
#include <WiFi.h>
#endif


#include <SPI.h>
#include <TimeLib.h>

#ifdef HAVE_TOUCHPAD
#include <XPT2046_Touchscreen.h>
#include "TouchControllerWS.h"
#endif

/***
   Install the following libraries through Arduino Library Manager
   - Mini Grafx by Daniel Eichhorn
   - ESP8266 WeatherStation by Daniel Eichhorn
   - Json Streaming Parser by Daniel Eichhorn
   - simpleDSTadjust by neptune2
 ***/

#include <JsonListener.h>
#include <WundergroundConditions.h>
#include <WundergroundForecast.h>
#include <WundergroundAstronomy.h>
#include <MiniGrafx.h>
#include <Carousel.h>
#include <ILI9341_SPI.h>

#include "fonts\ArialRounded.h"
#include "moonphases.h"
#include "weathericons.h"
#include "logo.h"
#include "pers.h"

#define MINI_BLACK 0
#define MINI_WHITE 1
#define MINI_YELLOW 2
#define MINI_BLUE 3

#define MAX_FORECASTS 12


//#define TTGO

#ifndef TTGO
// Pins for the ILI9341
#define TFT_DC     2 //D2
#define TFT_CS     5 //D13
#define TFT_LED   15 //D10
#define TFT_RST    0 //D8

//#define HAVE_TOUCHPAD

#ifdef HAVE_TOUCHPAD
#define TOUCH_CS   0 //D8
#define TOUCH_IRQ  4 //D14
#endif    

#else

#define TFT_DC    16 //D2
#define TFT_CS     5 //D1
#define TFT_LED   15 //D8
#define TFT_RST   17 //D8

#endif    

// defines the colors usable in the paletted 16 color frame buffer
// defines the colors usable in the paletted 16 color frame buffer
uint16_t palette[] = {ILI9341_BLACK, // 0
                      ILI9341_WHITE, // 1
                      ILI9341_YELLOW, // 2
                      0x7E3C //3
                     };

uint16_t SCREEN_WIDTH = 240;
uint16_t SCREEN_HEIGHT = 320;
// Limited to 4 colors due to memory constraints
uint8_t BITS_PER_PIXEL = 2; // 2^2 =  4 colors

#define uS_TO_S_FACTOR 1000000  /* Conversion factor for micro seconds to seconds */
#define TIME_TO_SLEEP  5        /* Time ESP32 will go to sleep (in seconds) */


//ADC_MODE(ADC_VCC);

ILI9341_SPI tft = ILI9341_SPI(TFT_CS, TFT_DC, TFT_RST);

MiniGrafx gfx = MiniGrafx(&tft, BITS_PER_PIXEL, palette);

Carousel carousel(&gfx, 0, 0, 240, 100);



#ifdef HAVE_TOUCHPAD
XPT2046_Touchscreen ts(TOUCH_CS, TOUCH_IRQ);
TouchControllerWS touchController(&ts);


void calibrationCallback(int16_t x, int16_t y);
CalibrationCallback calibration = &calibrationCallback;
#endif


WGConditions conditions;
WGForecast forecasts[MAX_FORECASTS];
WGAstronomy astronomy;

void updateData();
void drawProgress(uint8_t percentage, String text);
void drawTime();
void drawCurrentWeather();
void drawForecast();
void drawForecastDetail(uint16_t x, uint16_t y, uint8_t dayIndex);
void drawAstronomy();
void drawCurrentWeatherDetail();
void drawLabelValue(uint8_t line, String label, String value);
void drawForecastTable(uint8_t start);
void drawAbout();
void drawSeparator(uint16_t y);
const char* getMeteoconIconFromProgmem(String iconText);
const char* getMiniMeteoconIconFromProgmem(String iconText);
void drawForecast1(MiniGrafx *display, CarouselState* state, int16_t x, int16_t y);
void drawForecast2(MiniGrafx *display, CarouselState* state, int16_t x, int16_t y);
FrameCallback frames[] = { drawForecast1, drawForecast2 };
int8_t frameCount = 2;

// how many different screens do we have?
int8_t screenCount = 5;
long lastDownloadUpdate = millis();

String moonAgeImage = "";
String msg = "";
int8_t screen = 0;
long timerPress;

#if defined(ESP8266)

const uint8_t button3Pin = 0;    // the number of the pushbutton pin
const uint8_t button2Pin = 2;    // the number of the pushbutton pin
const uint8_t button1Pin = 15;    // the number of the pushbutton pin
#endif

#if defined(ESP32)

const uint8_t button3Pin = 37;    // the number of the pushbutton pin
const uint8_t button2Pin = 38;    // the number of the pushbutton pin
const uint8_t button1Pin = 39;    // the number of the pushbutton pin
#endif

// Variables will change:
int8_t button1State;             // the current reading from the input pin
int8_t lastButton1State = LOW;   // the previous reading from the input pin
int8_t button2State;             // the current reading from the input pin
int8_t lastButton2State = LOW;   // the previous reading from the input pin
int8_t button3State;             // the current reading from the input pin
int8_t lastButton3State = LOW;   // the previous reading from the input pin

// the following variables are unsigned longs because the time, measured in
// milliseconds, will quickly become a bigger number than can be stored in an int.
unsigned long lastDebounce1Time = 0;  // the last time the output pin was toggled
unsigned long lastDebounce2Time = 0;  // the last time the output pin was toggled
unsigned long lastDebounce3Time = 0;  // the last time the output pin was toggled
unsigned long debounceDelay = 50;    // the debounce time; increase if the output flickers

static const char* name_week [] = {"", "Вc", "Пн", "Вт", "Ср", "Чт", "Пт", "Сб"};
static const char* name_month[] = {"", "Января", "Февраля", "Марта", "Апреля", "Мая", "Июня", "Июля", "Августа", "Сентября", "Октября", "Ноября", "Декабря"};
static const char* name_week_e [] = {"", "Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};
static const char* name_month_e[] = {"", "Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};

// move to pers.h
//#define WIFI_SSID "MY_SSID"
//#define WIFI_PASS "MY_PASS"

const int UPDATE_INTERVAL_SECS = 15 * 60; // Update every 15 minutes
const int SLEEP_INTERVAL_SECS = 0;   // Going to Sleep after idle times, set 0 for dont sleep


// Wunderground Settings
// To check your settings first try them out in your browser:
// http://api.wunderground.com/api/WUNDERGROUND_API_KEY/conditions/q/WUNDERGROUND_COUNTTRY/WUNDERGROUND_CITY.json
// e.g. http://api.wunderground.com/api/808ba87ed77c4511/conditions/q/CH/Zurich.json
// e.g. http://api.wunderground.com/api/808ba87ed77c4511/conditions/q/CA/SAN_FRANCISCO.json <- note that in the US you use the state instead of country code

const String DISPLAYED_CITY_NAME = "Забанск";
//const String WUNDERGRROUND_API_KEY = "API_KEY"; //move to pers.h
const String WUNDERGRROUND_LANGUAGE = "RU";
const String WUNDERGROUND_COUNTRY = "RU";
const String WUNDERGROUND_CITY = "Yekaterinburg";

#define UTC_OFFSET 5

const char* dstAbbrev = "RTZ+5";

bool RUS = false; //Говорим по русски

// values in metric or imperial system?
bool IS_METRIC = true;

// Change for 12 Hour/ 24 hour style clock
bool IS_STYLE_12HR = false;

// change for different ntp (time servers)
#define NTP_SERVERS "ru.pool.ntp.org", "1.ch.pool.ntp.org", "2.ch.pool.ntp.org"

/***************************
 * End Settings
 **************************/

