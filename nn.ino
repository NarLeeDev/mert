#include <SPI.h> // Подключение библиотеки SPI для работы с SD-картой и дисплеем
#include <SD.h> // Подключение библиотеки для работы с SD-картой
#include <TFT_eSPI.h> // Подключение библиотеки для работы с TFT-дисплеем
#include <ArduinoJson.h> // Подключение библиотеки для работы с JSON
#include <RadioLib.h> // Подключение библиотеки для работы с радиомодулями

TFT_eSPI TFT = TFT_eSPI(); // Создание объекта для работы с TFT-дисплеем

// Параметры для настройки графиков
const int16_t speed = 300; // Скорость обновления данных
const float chartMax = 110; // Наибольшее значение в масштабе графика
const float chartMin = -110; // Наименьшее значение в масштабе графика
const int8_t chartPadding = 10; // Внутренний отступ области графика
const int8_t chartScale = 10; // Вертикальный масштаб графика
const int16_t chartDistance = 20; // Горизонтальное расстояние между точками
const int16_t chartXOffset = 35; // Позиция X области графика
const int16_t chartYOffset = 0; // Позиция Y области графика
const int16_t chartWidth = 320 - chartXOffset; // Ширина области графика
const int16_t chartHeight = 120; // Высота области графика для одного графика
const int16_t chartBackground = TFT_BLACK; // Цвет фона графика
const int16_t chartBorderColor = TFT_WHITE; // Цвет границы графика
const int16_t chartScaleColor = TFT_WHITE; // Цвет границы шкалы
const int16_t chartScaleTextColor = TFT_WHITE; // Цвет текста шкалы
const int16_t chartHGridColor = TFT_LIGHTGREY; // Цвет горизонтальной сетки
const int16_t chartVGridColor = TFT_DARKGREY; // Цвет вертикальной сетки
const int16_t chartLineColor1 = TFT_YELLOW; // Цвет линии первого графика
const int16_t chartLineColor2 = TFT_CYAN; // Цвет линии второго графика
const int16_t chartMarkerColor1 = TFT_RED; // Цвет маркера первого графика
const int16_t chartMarkerColor2 = TFT_BLUE; // Цвет маркера второго графика
const bool showValueBox = true; // Отображение поля с текущим значением

// Проверка деления на ноль, если разница между max и min равна нулю
const float delta = (chartMax - chartMin == 0) ? 1 : chartMax - chartMin; // Разница между наибольшим и наименьшим значением

// Параметры для отображения графика
const int16_t chartPXOffset = chartXOffset + chartPadding; // Позиция X области графика с учетом отступа
const int16_t chartPYOffset = chartYOffset + chartPadding; // Позиция Y области графика с учетом отступа
const int16_t chartPWidth = chartWidth - chartPadding * 2; // Ширина области графика с учетом отступа
const int16_t chartPHeight = chartHeight - chartPadding * 2; // Высота области графика с учетом отступа

// Количество данных, которые умещаются на графике
const int8_t dataQuantity = chartPWidth / chartDistance + 1; // Количество данных для отображения на графике

// Массивы для хранения данных графиков
int16_t data1[dataQuantity]; // Данные для первого графика
int16_t data2[dataQuantity]; // Данные для второго графика

// Документы для хранения данных из JSON файлов
DynamicJsonDocument doc1(16384); // Документ для данных из 500.json
DynamicJsonDocument doc2(16384); // Документ для данных из 1200.json

// Векторы для хранения частот из JSON файлов
std::vector<int> frequencies1; // Частоты для первого графика
std::vector<int> frequencies2; // Частоты для второго графика

void setup() {
  Serial.begin(115200); // Инициализация последовательного порта для отладки
  TFT.begin(); // Инициализация TFT-дисплея
  TFT.setRotation(1); // Установка ориентации дисплея
  randomSeed(micros()); // Инициализация генератора случайных чисел

  if (!SD.begin()) { // Инициализация SD-карты
    Serial.println("SD card initialization failed!"); // Сообщение об ошибке инициализации SD-карты
    return; // Выход из функции setup, если SD-карта не инициализирована
  }

  // Чтение данных из файла 500.json
  File jsonFile1 = SD.open("/500.json"); // Открытие файла 500.json
  if (!jsonFile1) { // Проверка, удалось ли открыть файл
    Serial.println("Failed to open file 500.json"); // Сообщение об ошибке открытия файла
    return; // Выход из функции setup, если файл не открыт
  }

  DeserializationError error1 = deserializeJson(doc1, jsonFile1); // Десериализация JSON-документа из файла 500.json
  if (error1) { // Проверка на ошибки десериализации
    Serial.println("Failed to parse file 500.json"); // Сообщение об ошибке десериализации
    jsonFile1.close(); // Закрытие файла
    return; // Выход из функции setup, если произошла ошибка десериализации
  }
  jsonFile1.close(); // Закрытие файла

  JsonArray array1 = doc1.as<JsonArray>(); // Преобразование JSON-документа в массив
  for (JsonObject obj : array1) { // Перебор объектов в массиве
    frequencies1.push_back(obj["freq"]); // Добавление частот в вектор frequencies1
  }

  // Чтение данных из файла 1200.json
  File jsonFile2 = SD.open("/1200.json"); // Открытие файла 1200.json
  if (!jsonFile2) { // Проверка, удалось ли открыть файл
    Serial.println("Failed to open file 1200.json"); // Сообщение об ошибке открытия файла
    return; // Выход из функции setup, если файл не открыт
  }

  DeserializationError error2 = deserializeJson(doc2, jsonFile2); // Десериализация JSON-документа из файла 1200.json
  if (error2) { // Проверка на ошибки десериализации
    Serial.println("Failed to parse file 1200.json"); // Сообщение об ошибке десериализации
    jsonFile2.close(); // Закрытие файла
    return; // Выход из функции setup, если произошла ошибка десериализации
  }
  jsonFile2.close(); // Закрытие файла

  JsonArray array2 = doc2.as<JsonArray>(); // Преобразование JSON-документа в массив
  for (JsonObject obj : array2) { // Перебор объектов в массиве
    frequencies2.push_back(obj["freq"]); // Добавление частот в вектор frequencies2
  }

  drawChartBox(chartYOffset); // Рисование рамки первого графика
  drawChartBox(chartYOffset + chartHeight); // Рисование рамки второго графика
  drawChartScale(chartYOffset); // Рисование вертикального масштаба для первого графика
  drawChartScale(chartYOffset + chartHeight); // Рисование вертикального масштаба для второго графика
}

void loop() {
  drawHorizontalGrid(chartYOffset); // Рисование горизонтальной сетки для первого графика
  drawHorizontalGrid(chartYOffset + chartHeight); // Рисование горизонтальной сетки для второго графика
  drawVerticalGrid(chartYOffset); // Рисование вертикальной сетки для первого графика
  drawVerticalGrid(chartYOffset + chartHeight); // Рисование вертикальной сетки для второго графика
  drawChartData(frequencies1, data1, chartYOffset, chartLineColor1, chartMarkerColor1); // Рисование данных первого графика
  drawChartData(frequencies2, data2, chartYOffset + chartHeight, chartLineColor2, chartMarkerColor2); // Рисование данных второго графика
  eraseChartData(data1, chartYOffset); // Стирание данных первого графика
  eraseChartData(data2, chartYOffset + chartHeight); // Стирание данных второго графика
  delay(1000); // Задержка для стабилизации работы
}

// Рисует рамку графика
void drawChartBox(int yOffset) {
  TFT.drawRect(chartXOffset - 1, yOffset - 1, chartWidth + 2, chartHeight + 2, chartBorderColor); // Рисование границы графика
  TFT.fillRect(chartXOffset, yOffset, chartWidth, chartHeight, chartBackground); // Заполнение фона графика
}

// Рисует вертикальный масштаб графика
void drawChartScale(int yOffset) {
  TFT.drawLine(chartXOffset - 5, yOffset + chartPYOffset, chartXOffset - 5, yOffset + chartYOffset + chartHeight - chartPadding, chartScaleColor); // Рисование вертикальной линии масштаба
  for (int8_t i = 0; i < (chartPHeight / chartScale + 1); i++) { // Перебор значений масштаба
    TFT.drawLine(
      chartXOffset - 5,
      yOffset + chartYOffset + chartHeight - chartPadding - chartScale * i,
      chartXOffset - 10,
      yOffset + chartYOffset + chartHeight - chartPadding - chartScale * i,
      chartScaleColor
    ); // Рисование горизонтальных линий масштаба
    // Центрирование текста шкалы
    TFT.setTextDatum(MC_DATUM); // Установка точки привязки текста по центру
    String text = String(round(delta / chartPHeight * (i * chartScale + chartMin))); // Формирование текста шкалы
    for (int8_t j = text.length(); j < 6; j++) text = " " + text; // Добавление пробелов для выравнивания текста
    TFT.drawString(text, chartXOffset - 47, yOffset + chartYOffset + chartHeight - chartPadding - chartScale * i - 4, chartScaleTextColor); // Рисование текста шкалы
  }
}

// Рисует горизонтальную сетку
void drawHorizontalGrid(int yOffset) {
  for (int8_t i = (chartPadding == 0 ? 1 : 0); i < (chartHeight / chartScale); i++) { // Перебор значений сетки
    TFT.drawLine(
      chartXOffset,
      yOffset + chartYOffset + chartHeight - chartPadding - chartScale * i,
      chartXOffset + chartWidth - 1,
      yOffset + chartYOffset + chartHeight - chartPadding - chartScale * i,
      chartHGridColor
    ); // Рисование горизонтальных линий сетки
  }
}

// Рисует вертикальную сетку
void drawVerticalGrid(int yOffset) {
  for (int8_t i = 0; i < dataQuantity; i++) { // Перебор значений сетки
    TFT.drawLine(
      chartPXOffset + chartDistance * i,
      yOffset,
      chartPXOffset + chartDistance * i,
      yOffset + chartHeight - 1,
      chartVGridColor
    ); // Рисование вертикальных линий сетки
  }
}

// Рисует данные графика
void drawChartData(const std::vector<int>& frequencies, int16_t* data, int yOffset, int16_t lineColor, int16_t markerColor) {
  int16_t prevValue; // Переменная для хранения предыдущего значения
  for (int8_t column = 0; column < dataQuantity; column++) { // Перебор данных графика
    // Получение случайного значения для частоты
    int16_t freqIndex = random(0, frequencies.size()); // Случайный индекс частоты
    float value = frequencies[freqIndex]; // Получение значения частоты
    drawValueBox(value); // Рисование поля с текущим значением
    data[column] = value; // Сохранение значения в массив данных

    if (column == 0) { // Если это первая колонка
      prevValue = value; // Сохранение значения как предыдущего
      continue; // Переход к следующей итерации
    }
    const int16_t x1 = chartPXOffset + (column - 1 < 0 ? 0 : column - 1) * chartDistance; // Координата X первой точки
    const int16_t y1 = yOffset + chartPYOffset + chartPHeight - (chartPHeight / delta * (prevValue - chartMin)); // Координата Y первой точки
    const int16_t x2 = chartPXOffset + column * chartDistance; // Координата X второй точки
    const int16_t y2 = yOffset + chartPYOffset + chartPHeight - (chartPHeight / delta * (value - chartMin)); // Координата Y второй точки
    if (column * chartDistance > chartPWidth) break; // Прерывание цикла, если данные выходят за границы графика
    TFT.drawLine(x1, y1, x2, y2, lineColor); // Рисование линии между точками
    TFT.fillCircle(x1, y1, 2, markerColor); // Рисование маркера первой точки
    TFT.fillCircle(x2, y2, 2, markerColor); // Рисование маркера второй точки
    prevValue = value; // Сохранение текущего значения как предыдущего
    delay(speed); // Задержка для стабильности отображения
  }
}

// Стирает данные графика
void eraseChartData(const int16_t* data, int yOffset) {
  for (int8_t i = 1; i < dataQuantity; i++) { // Перебор данных графика
    const int16_t prevValue = data[i - 1]; // Получение предыдущего значения
    const float value = data[i]; // Получение текущего значения
    const int16_t x1 = chartPXOffset + (i - 1 < 0 ? 0 : i - 1) * chartDistance; // Координата X первой точки
    const int16_t y1 = yOffset + chartPYOffset + chartPHeight - (chartPHeight / delta * (prevValue - chartMin)); // Координата Y первой точки
    const int16_t x2 = chartPXOffset + i * chartDistance; // Координата X второй точки
    const int16_t y2 = yOffset + chartPYOffset + chartPHeight - (chartPHeight / delta * (value - chartMin)); // Координата Y второй точки
    TFT.drawLine(x1, y1, x2, y2, chartBackground); // Стирание линии между точками
    TFT.fillCircle(x1, y1, 2, chartBackground); // Стирание маркера первой точки
    TFT.fillCircle(x2, y2, 2, chartBackground); // Стирание маркера второй точки
  }
}

// Рисует коробку с текущим значением
void drawValueBox(float value) {
  if (!showValueBox) return; // Проверка, нужно ли отображать поле с текущим значением
  const int16_t width = 50; // Ширина поля
  const int16_t height = 20; // Высота поля
  const int16_t x = 320 - width; // Позиция X поля
  const int16_t y = 240 - height; // Позиция Y поля
  TFT.drawRect(x, y, width, height, TFT_WHITE); // Рисование границы поля
  TFT.fillRect(x + 1, y + 1, width - 2, height - 2, TFT_BLUE); // Заполнение фона поля
  TFT.setCursor(x + 4, y + height / 2 - 3); // Установка курсора для текста
  TFT.print(value); // Вывод значения в поле
}
