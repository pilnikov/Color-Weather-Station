/**The MIT License (MIT)
  Copyright (c) 2017 by Daniel Eichhorn
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
  See more at https://blog.squix.org
*/


/*****************************
   Important: see settings.h to configure your settings!!!
 * ***************************/

#include "settings.h"



void connectWifi() {
  WiFi.disconnect();
  WiFi.mode( WIFI_STA );
  if (WiFi.status() == WL_CONNECTED) return;
  //Manual Wifi
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  uint8_t i = 0;
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    if (i > 80) i = 0;
    msg = "Connecting to WiFi...";
    if (RUS) msg = "Подключаемся к WiFi...";
    drawProgress(i, msg);
    i += 10;
    Serial.print(".");
  }
}

void setup() {
  Serial.begin(115200);

  // The LED pin needs to set HIGH
  // Use this pin to save energy
  // Turn on the background LED
  Serial.println(TFT_LED);
  pinMode(TFT_LED, OUTPUT);
  digitalWrite(TFT_LED, HIGH);    // HIGH to Turn on;

  pinMode(button1Pin, INPUT);
  pinMode(button2Pin, INPUT);
  pinMode(button3Pin, INPUT);

  gfx.init();
#ifdef TTGO
  gfx.setRotation(4);
#endif
  gfx.fillBuffer(MINI_BLACK);
  gfx.commit();

  connectWifi();

#ifdef HAVE_TOUCHPAD
  ts.begin();

  bool isFSMounted = SPIFFS.begin();
  if (!isFSMounted) {
    SPIFFS.format();
  }
  //SPIFFS.remove("/calibration.txt");
  boolean isCalibrationAvailable = touchController.loadCalibration();
  if (!isCalibrationAvailable) {
    Serial.println("Calibration not available");
    touchController.startCalibration(&calibration);
    while (!touchController.isCalibrationFinished()) {
      gfx.fillBuffer(0);
      gfx.setColor(MINI_YELLOW);
      gfx.setTextAlignment(TEXT_ALIGN_CENTER);
      gfx.drawString(120, 160, "Пожалуйстаоткалибруйте\nТачскрин с\nточками касания");
      touchController.continueCalibration();
      gfx.commit();
      yield();
    }
    touchController.saveCalibration();
  }
#endif

  carousel.setFrames(frames, frameCount);
  carousel.disableAllIndicators();

  // update the weather information
  updateData();
}

void loop()
{
#ifdef HAVE_TOUCHPAD
  if (touchController.isTouched(500))
  {
    TS_Point p = touchController.getPoint();
    if (p.y < 80)
    {
      IS_STYLE_12HR = !IS_STYLE_12HR;
    }
    else
    {
      //screen = (screen + 1) % screenCount;
    }
  }
#endif

  gfx.fillBuffer(MINI_BLACK);
  if (screen == 0) {
    drawTime();
    int remainingTimeBudget = carousel.update();
    if (remainingTimeBudget > 0) {
      // You can do some work here
      // Don't do stuff if you are below your
      // time budget.
      delay(remainingTimeBudget);
    }
    drawCurrentWeather();
    drawAstronomy();
  } else if (screen == 1) {
    drawCurrentWeatherDetail();
  } else if (screen == 2) {
    drawForecastTable(0);
  } else if (screen == 3) {
    drawForecastTable(6);
  } else if (screen == 4) {
    drawAbout();
  }
  gfx.commit();

  if (millis() - lastDownloadUpdate > 1000 * UPDATE_INTERVAL_SECS) //------ Check if we should update weather information
  {
    updateData();
    lastDownloadUpdate = millis();
  }

  if (SLEEP_INTERVAL_SECS && millis() - timerPress >= SLEEP_INTERVAL_SECS * 1000) { // after 2 minutes go to sleep
    drawProgress(25, "Засыпаем!");
    delay(1000);
    drawProgress(50, "Засыпаем!");
    delay(1000);
    drawProgress(75, "Засыпаем!");
    delay(1000);
    drawProgress(100, "Going to Sleep!");
    // go to deepsleep for xx minutes or 0 = permanently
#if defined(ESP8266)
    ESP.deepSleep(0,  WAKE_RF_DEFAULT);                       // 0 delay = permanently to sleep
#endif
#if defined(ESP32)
    esp_sleep_enable_timer_wakeup(TIME_TO_SLEEP * uS_TO_S_FACTOR);
    esp_deep_sleep_start();
#endif
  }
  bounce();
}

