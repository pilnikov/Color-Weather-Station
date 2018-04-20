void updateData() // --------------   Update the internet based information and update screen
{
  gfx.fillBuffer(MINI_BLACK);
  gfx.setFont(ArialRoundedMTBold_14);

  msg = "Updating time...";
  if (RUS) msg = "Обновляем время...";
  drawProgress(10, msg);

  configTime(0, 0, NTP_SERVERS);
  time_t now = time(nullptr) + 3600 * UTC_OFFSET;
  while (now < 3600 * UTC_OFFSET + 1000)
  {
    delay(500);
    now = time(nullptr) + 3600 * UTC_OFFSET;
  }
  setTime(now);

  msg = "Updating conditions...";
  if (RUS) msg = "Обновляем содержимое...";
  drawProgress(50, msg);
  WundergroundConditions *conditionsClient = new WundergroundConditions(IS_METRIC);
  conditionsClient->updateConditions(&conditions, WUNDERGRROUND_API_KEY, WUNDERGRROUND_LANGUAGE, WUNDERGROUND_COUNTRY, WUNDERGROUND_CITY);
  delete conditionsClient;
  conditionsClient = nullptr;

  msg = "Updating forecasts...";
  if (RUS) msg = "Обновляем погоду...";
  drawProgress(70, msg);
  WundergroundForecast *forecastClient = new WundergroundForecast(IS_METRIC);
  forecastClient->updateForecast(forecasts, MAX_FORECASTS, WUNDERGRROUND_API_KEY, WUNDERGRROUND_LANGUAGE, WUNDERGROUND_COUNTRY, WUNDERGROUND_CITY);
  delete forecastClient;
  forecastClient = nullptr;

  msg = "Updating astronomy...";
  if (RUS) msg = "Обновляем астрономию...";
  drawProgress(80, msg);
  WundergroundAstronomy *astronomyClient = new WundergroundAstronomy(IS_STYLE_12HR);
  astronomyClient->updateAstronomy(&astronomy, WUNDERGRROUND_API_KEY, WUNDERGRROUND_LANGUAGE, WUNDERGROUND_COUNTRY, WUNDERGROUND_CITY);
  delete astronomyClient;
  astronomyClient = nullptr;
  moonAgeImage = String((char) (65 + 26 * (((15 + astronomy.moonAge.toInt()) % 30) / 30.0)));

  delay(1000);
}


void drawProgress(uint8_t percentage, String text)// ------------------Progress bar helper
{
  gfx.fillBuffer(MINI_BLACK);
  gfx.drawPalettedBitmapFromPgm(95, 10, MyLogo);
  gfx.setFont(ArialRoundedMTBold_14);
  gfx.setTextAlignment(TEXT_ALIGN_CENTER);
  gfx.setColor(MINI_WHITE);
  gfx.drawString(120, 90, "https://github.com/pilnikov/");
  gfx.drawString(120, 120, "Color-Weather-Station-рус");
  gfx.setColor(MINI_YELLOW);

  gfx.drawString(120, 146, text);
  gfx.setColor(MINI_WHITE);
  gfx.drawRect(10, 168, 240 - 20, 15);
  gfx.setColor(MINI_BLUE);
  gfx.fillRect(12, 170, 216 * percentage / 100, 11);

  gfx.commit();
}


void drawTime()  // ------------------------------------draws the clock
{
  char time_str[11];

  gfx.setTextAlignment(TEXT_ALIGN_CENTER);
  gfx.setFont(ArialRoundedMTBold_14);
  gfx.setColor(MINI_WHITE);
  char date[30];

  if (RUS)
  {
    sprintf(date, "%s %2d %s %4d г.", name_week[weekday()], day(), name_month[month()], year());
  }
  else
  {
    sprintf(date, "%s %s %2d %4d", name_week_e[weekday()], name_month_e[month()], day(), year());
  }
  gfx.drawString(SCREEN_WIDTH / 2, 6, date);

  gfx.setFont(ArialRoundedMTBold_36);

  if (IS_STYLE_12HR)
  {
    uint8_t hr = (hour() + 11) % 12 + 1; // take care of noon and midnight
    sprintf(time_str, "%2d:%02d:%02d\n", hr, minute(), second());
    gfx.drawString(SCREEN_WIDTH / 2, 20, time_str);
  }
  else
  {
    sprintf(time_str, "%02d:%02d:%02d\n", hour(), minute(), second());
    gfx.drawString(SCREEN_WIDTH / 2, 20, time_str);
  }

  gfx.setTextAlignment(TEXT_ALIGN_LEFT);
  gfx.setFont(ArialMT_Plain_10);
  gfx.setColor(MINI_BLUE);
  if (IS_STYLE_12HR)
  {
    sprintf(time_str, "%s\n%s", dstAbbrev, hour() >= 12 ? "PM" : "AM");
    gfx.drawString(195, 27, time_str);
  }
  else
  {
    sprintf(time_str, "%s", dstAbbrev);
    gfx.drawString(195, 27, time_str);  // Known bug: Cuts off 4th character of timezone abbreviation
  }
  drawWifiQuality();
}

void drawCurrentWeather()  // ----------------  draws current weather information
{
  gfx.setTransparentColor(MINI_BLACK);
  gfx.drawPalettedBitmapFromPgm(0, 55, getMeteoconIconFromProgmem(conditions.weatherIcon));
  // Weather Text

  gfx.setFont(ArialRoundedMTBold_14);
  gfx.setColor(MINI_BLUE);
  gfx.setTextAlignment(TEXT_ALIGN_RIGHT);
  gfx.drawString(220, 65, DISPLAYED_CITY_NAME);

  gfx.setFont(ArialRoundedMTBold_36);
  gfx.setColor(MINI_WHITE);
  gfx.setTextAlignment(TEXT_ALIGN_RIGHT);
  String degreeSign = "°F";
  if (IS_METRIC) degreeSign = "°C";

  String temp = conditions.currentTemp + degreeSign;

  gfx.drawString(220, 78, temp);

  gfx.setFont(ArialRoundedMTBold_14);
  gfx.setColor(MINI_YELLOW);
  gfx.setTextAlignment(TEXT_ALIGN_RIGHT);
  gfx.drawString(220, 118, conditions.weatherText);
}

void drawForecast1(MiniGrafx *display, CarouselState* state, int16_t x, int16_t y)  // ----------------  draws forecast detail
{
  drawForecastDetail(x + 10, y + 165, 0);
  drawForecastDetail(x + 95, y + 165, 2);
  drawForecastDetail(x + 180, y + 165, 4);
}

void drawForecast2(MiniGrafx *display, CarouselState* state, int16_t x, int16_t y)  // ----------------  draws forecast detail
{
  drawForecastDetail(x + 10, y + 165, 6);
  drawForecastDetail(x + 95, y + 165, 8);
  drawForecastDetail(x + 180, y + 165, 10);
}


// helper for the forecast columns
void drawForecastDetail(uint16_t x, uint16_t y, uint8_t dayIndex)
{
  gfx.setColor(MINI_YELLOW);
  gfx.setFont(ArialRoundedMTBold_14);
  gfx.setTextAlignment(TEXT_ALIGN_CENTER);
  String day = forecasts[dayIndex].forecastTitle.substring(0, 3);
  day.toUpperCase();
  gfx.drawString(x + 25, y - 15, day);

  gfx.setColor(MINI_WHITE);
  gfx.drawString(x + 25, y, forecasts[dayIndex].forecastLowTemp + "|" + forecasts[dayIndex].forecastHighTemp);

  gfx.drawPalettedBitmapFromPgm(x, y + 15, getMiniMeteoconIconFromProgmem(forecasts[dayIndex].forecastIcon));
  gfx.setColor(MINI_BLUE);
  gfx.drawString(x + 25, y + 60, forecasts[dayIndex].PoP + "%");
}


void drawAstronomy() // ----------------------------------------draw moonphase and sunrise/set and moonrise/set
{
  gfx.setFont(MoonPhases_Regular_36);
  gfx.setColor(MINI_WHITE);
  gfx.setTextAlignment(TEXT_ALIGN_CENTER);
  gfx.drawString(120, 275, moonAgeImage);

  gfx.setColor(MINI_WHITE);
  gfx.setFont(ArialRoundedMTBold_14);
  gfx.setTextAlignment(TEXT_ALIGN_CENTER);
  gfx.setColor(MINI_YELLOW);
  gfx.drawString(120, 250, astronomy.moonPhase);
  gfx.setTextAlignment(TEXT_ALIGN_LEFT);
  gfx.setColor(MINI_YELLOW);
  msg = "Sun";
  if (RUS) msg = "Солнце";
  gfx.drawString(5, 250, msg);
  gfx.setColor(MINI_WHITE);
  gfx.drawString(5, 276, astronomy.sunriseTime);
  gfx.drawString(5, 291, astronomy.sunsetTime);

  gfx.setTextAlignment(TEXT_ALIGN_RIGHT);
  gfx.setColor(MINI_YELLOW);
  msg = "Moon";
  if (RUS) msg = "Луна";
  gfx.drawString(235, 250, msg);
  gfx.setColor(MINI_WHITE);
  gfx.drawString(235, 276, astronomy.moonriseTime);
  gfx.drawString(235, 291, astronomy.moonsetTime);

}

void drawCurrentWeatherDetail() //----------------------------- draw Current Weather Detail
{
  gfx.setFont(ArialRoundedMTBold_14);
  gfx.setTextAlignment(TEXT_ALIGN_CENTER);
  gfx.setColor(MINI_WHITE);
  msg = "Current Conditions";
  if (RUS) msg = "Текущее состояние";
  gfx.drawString(120, 2, msg);

  //gfx.setTransparentColor(MINI_BLACK);
  //gfx.drawPalettedBitmapFromPgm(0, 20, getMeteoconIconFromProgmem(conditions.weatherIcon));

  String degreeSign = "°F";
  if (IS_METRIC) degreeSign = "°C";
  // String weatherIcon;
  // String weatherText;
  msg = "Temperature:";
  if (RUS) msg = "Температура:";
  drawLabelValue(0, msg, conditions.currentTemp + degreeSign);
  msg = "Feels Like:";
  if (RUS) msg = "Ощущается как:";
  drawLabelValue(1, msg, conditions.feelslike + degreeSign);
  msg = "Dew Point:";
  if (RUS) msg = "Точка росы:";
  drawLabelValue(2, msg, conditions.dewPoint + degreeSign);
  msg = "Wind Speed:";
  if (RUS) msg = "Скорость ветра:";
  drawLabelValue(3, msg, conditions.windSpeed);
  msg = "Wind Dir:";
  if (RUS) msg = "Направление ветра:";
  drawLabelValue(4, msg, conditions.windDir);
  msg = "Humidity:";
  if (RUS) msg = "Влажность:";
  drawLabelValue(5, "Влажность:", conditions.humidity);
  msg = "Pressure:";
  if (RUS) msg = "Давление:";
  drawLabelValue(6, msg, conditions.pressure);
  msg = "Precipitation:";
  if (RUS) msg = "Осадки:";
  drawLabelValue(7, msg, conditions.precipitationToday);
  msg = "UV:";
  if (RUS) msg = "Ультрафиолет:";
  drawLabelValue(8, msg, conditions.UV);

  gfx.setTextAlignment(TEXT_ALIGN_LEFT);
  gfx.setColor(MINI_YELLOW);
  msg = "Description: ";
  if (RUS) msg = "Подробно: ";
  gfx.drawString(15, 185, msg);
  gfx.setColor(MINI_WHITE);
  gfx.drawStringMaxWidth(15, 200, 240 - 2 * 15, forecasts[0].forecastText);
}

void drawLabelValue(uint8_t line, String label, String value)
{
  const uint8_t labelX = 15;
  const uint8_t valueX = 150;
  gfx.setTextAlignment(TEXT_ALIGN_LEFT);
  gfx.setColor(MINI_YELLOW);
  gfx.drawString(labelX, 30 + line * 15, label);
  gfx.setColor(MINI_WHITE);
  gfx.drawString(valueX, 30 + line * 15, value);
}

// converts the dBm to a range between 0 and 100%
int8_t getWifiQuality()
{
  int32_t dbm = WiFi.RSSI();
  if (dbm <= -100) return 0;
  else if (dbm >= -50) return 100;
  else return 2 * (dbm + 100);

}

void drawWifiQuality()
{
  int8_t quality = getWifiQuality();
  gfx.setColor(MINI_WHITE);
  gfx.setTextAlignment(TEXT_ALIGN_RIGHT);
  gfx.drawString(228, 9, String(quality) + "%");
  for (int8_t i = 0; i < 4; i++)
  {
    for (int8_t j = 0; j < 2 * (i + 1); j++) if (quality > i * 25 || j == 0) gfx.setPixel(230 + 2 * i, 18 - j);
  }
}

void drawForecastTable(uint8_t start)
{
  gfx.setFont(ArialRoundedMTBold_14);
  gfx.setTextAlignment(TEXT_ALIGN_CENTER);
  gfx.setColor(MINI_WHITE);
  msg = "Forecasts";
  if (RUS) msg = "Погода подробно:";
  gfx.drawString(120, 2, msg);
  uint16_t y = 0;

  String degreeSign = "°F";
  if (IS_METRIC) degreeSign = "°C";
  for (uint8_t i = start; i < start + 6; i++)
  {
    gfx.setTextAlignment(TEXT_ALIGN_LEFT);
    y = 30 + (i - start) * 45;
    if (y > 320) break;
    gfx.drawPalettedBitmapFromPgm(0, y, getMiniMeteoconIconFromProgmem(forecasts[i].forecastIcon));

    gfx.setColor(MINI_YELLOW);
    gfx.setFont(ArialRoundedMTBold_14);

    gfx.drawString(50, y, forecasts[i].forecastTitle);
    gfx.setColor(MINI_WHITE);
    gfx.drawString(50, y + 15, getShortText(forecasts[i].forecastIcon));
    gfx.setColor(MINI_WHITE);
    gfx.setTextAlignment(TEXT_ALIGN_RIGHT);

    if (i % 2 == 0) msg = forecasts[i].forecastHighTemp;
    else msg = forecasts[i - 1].forecastLowTemp;
    gfx.drawString(235, y, msg + degreeSign);
    /*gfx.setColor(MINI_WHITE);
      gfx.drawString(x + 25, y, forecasts[dayIndex].forecastLowTemp + "|" + forecasts[dayIndex].forecastHighTemp);

      gfx.drawPalettedBitmapFromPgm(x, y + 15, getMiniMeteoconIconFromProgmem(forecasts[dayIndex].forecastIcon));*/
    gfx.setColor(MINI_BLUE);
    gfx.drawString(235, y + 15, forecasts[i].PoP + "%");

  }
}

void drawAbout()
{
  gfx.fillBuffer(MINI_BLACK);
  gfx.drawPalettedBitmapFromPgm(95, 10, MyLogo);
  gfx.setFont(ArialRoundedMTBold_14);
  gfx.setTextAlignment(TEXT_ALIGN_CENTER);
  gfx.setColor(MINI_WHITE);
  gfx.drawString(120, 80, "https://github.com/pilnikov/");
  gfx.drawString(120, 100, "Color-Weather-Station-РУС");

  gfx.setFont(ArialRoundedMTBold_14);
  gfx.setTextAlignment(TEXT_ALIGN_CENTER);
  msg = "Heap Mem:";
  if (RUS) msg = "Свободно в Heap:";
  drawLabelValue(7, "Свободно в Heap:", String(ESP.getFreeHeap() / 1024) + "kb");
  msg = "Flash Mem:";
  if (RUS) msg = "Свободно Flash:";
  drawLabelValue(8, msg, String(ESP.getFlashChipSize() / 1024 / 1024) + "MB");
  msg = "WiFi Strength:";
  if (RUS) msg = "Уровень WiFi:";
  drawLabelValue(9, msg, String(WiFi.RSSI()) + "dB");
  msg = "Chip ID:=";
  if (RUS) msg = "Чип ID:=";
#if defined(ESP8266)
  drawLabelValue(10, msg, String(ESP.getChipId()));
#endif
#if defined(ESP32)
  uint64_t chipid = ESP.getEfuseMac(); //The chip ID is essentially its MAC address(length: 6 bytes).
  drawLabelValue(10, msg, String((uint16_t)(chipid >> 32)));
  drawLabelValue(11, "", String((uint32_t)chipid));
#endif
#if defined(ESP8266)
  msg = "CPU Freq.: ";
  if (RUS) msg = "Подробно: ";
  drawLabelValue(12, "VCC: ", String(ESP.getVcc() / 1024.0) + "V");
#endif
  msg = "Description: ";
  if (RUS) msg = "Частота CPU: ";
  drawLabelValue(13, msg, String(ESP.getCpuFreqMHz()) + "MHz");
  char time_str[15];
  const uint32_t millis_in_day = 1000 * 60 * 60 * 24;
  const uint32_t millis_in_hour = 1000 * 60 * 60;
  const uint32_t millis_in_minute = 1000 * 60;
  uint8_t days = millis() / (millis_in_day);
  uint8_t hours = (millis() - (days * millis_in_day)) / millis_in_hour;
  uint8_t minutes = (millis() - (days * millis_in_day) - (hours * millis_in_hour)) / millis_in_minute;
  sprintf(time_str, "%2dd%2dh%2dm", days, hours, minutes);
  msg = "Uptime: ";
  if (RUS) msg = "Время работы: ";
  drawLabelValue(14, msg, time_str);
  gfx.setTextAlignment(TEXT_ALIGN_LEFT);
  gfx.setColor(MINI_YELLOW);
#if defined(ESP8266)
  msg = "Last Reset: ";
  if (RUS) msg = "Последний перезапуск: ";
  gfx.drawString(16, 250, msg);
  gfx.setColor(MINI_WHITE);
  gfx.drawStringMaxWidth(17, 265, 240 - 2 * 15, ESP.getResetInfo());
#endif
}

void calibrationCallback(int16_t x, int16_t y)
{
  gfx.setColor(1);
  gfx.fillCircle(x, y, 10);
}

void bounce()
{
  // read the state of the switch into a local variable:
  int8_t reading1 = digitalRead(button1Pin);
  int8_t reading2 = digitalRead(button2Pin);
  int8_t reading3 = digitalRead(button3Pin);

  // If the switch changed, due to noise or pressing:
  // reset the debouncing timer
  if (reading1 != lastButton1State) lastDebounce1Time = millis();
  if (reading2 != lastButton2State) lastDebounce2Time = millis();
  if (reading3 != lastButton3State) lastDebounce3Time = millis();

  if (((millis() - lastDebounce1Time) > debounceDelay) && (reading1 != button1State))
  {
    button1State = reading1;
    if (button1State == 0) screen = (screen + 1) % screenCount;
  }

  if (((millis() - lastDebounce2Time) > debounceDelay) && (reading2 != button2State))
  {
    button2State = reading2;
    if (button2State == 0) screen = (screen - 1) % screenCount;
  }

  if (((millis() - lastDebounce3Time) > debounceDelay) && (reading3 != button3State))
  {
    button3State = reading3;
    if (button3State == 0) screen = 0;
  }

  // save the reading. Next time through the loop, it'll be the lastButtonState:
  lastButton1State = reading1;
  lastButton2State = reading2;
  lastButton3State = reading3;
}

