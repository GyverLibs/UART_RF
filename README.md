[![latest](https://img.shields.io/github/v/release/GyverLibs/UART_RF.svg?color=brightgreen)](https://github.com/GyverLibs/UART_RF/releases/latest/download/UART_RF.zip)
[![PIO](https://badges.registry.platformio.org/packages/gyverlibs/library/UART_RF.svg)](https://registry.platformio.org/libraries/gyverlibs/UART_RF)
[![Foo](https://img.shields.io/badge/Website-AlexGyver.ru-blue.svg?style=flat-square)](https://alexgyver.ru/)
[![Foo](https://img.shields.io/badge/%E2%82%BD%24%E2%82%AC%20%D0%9F%D0%BE%D0%B4%D0%B4%D0%B5%D1%80%D0%B6%D0%B0%D1%82%D1%8C-%D0%B0%D0%B2%D1%82%D0%BE%D1%80%D0%B0-orange.svg?style=flat-square)](https://alexgyver.ru/support_alex/)
[![Foo](https://img.shields.io/badge/README-ENGLISH-blueviolet.svg?style=flat-square)](https://github-com.translate.goog/GyverLibs/UART_RF?_x_tr_sl=ru&_x_tr_tl=en)  

[![Foo](https://img.shields.io/badge/ПОДПИСАТЬСЯ-НА%20ОБНОВЛЕНИЯ-brightgreen.svg?style=social&logo=telegram&color=blue)](https://t.me/GyverLibs)

# UART_RF
Библиотека для передачи данных через 433 МГц радиомодули по UART
- Если UART аппаратный - не нагружает МК прерываниями и самостоятельно принимает данные
- Три варианта DC кодирования: отключено, Манчестер и 4b/6b
- Три варианта коррекции по Hamming: отключено, кодирование каждого байта и кодирование с перемешиванием
- Использование родного буфера UART-a - асинхронная отправка и приём
- Режим отправки сырых данных и пакетный режим с контролем целостности и типом пакета

### Совместимость
Совместима со всеми Arduino платформами (используются Arduino-функции)

### Зависимости
- [Hamming](https://github.com/GyverLibs/Hamming)

## Содержание
- [Документация](#usage)
- [Версии](#versions)
- [Установка](#install)
- [Баги и обратная связь](#feedback)

<a id="usage"></a>

## Документация
Библиотека отправляет данные через Arduino интерфейс `Print`, добавляя в начало пакета специальную последовательность символов для стабилизации радио канала и синхронизации старт-стоп UART. Передача и приём может быть дополнительно закодирована для улучшения качества связи:

- Балансное кодирование
  - `Encoder::DC4_6` (по умолч.) - 4b/6b, каждый ниббл кодируется 6 битами
  - `Encoder::Manchester` - ускоренная реализация Манчестера 4/4, отправляется `~nibble | nibble`
- Hamming - избыточный код для восстановления повреждённых данных
  - `Без кодирования` (по умолч.)
  - `Hamming::Simple` - алгоритм избыточности (8,4) для каждого байта по порядку
  - `Hamming::Mixed` - биты перемешиваются перед отправкой для повышения устойчивости к помехам

> [!WARNING]
> Режим кодирования должен совпадать на передатчике и приёмнике!

### Надёжность
|                   | `Encoder::None` | `Encoder::DC4_6` | `Encoder::Manchester` |
|-------------------|-----------------|------------------|-----------------------|
| `Hamming::None`   | Низкая          | Средняя          | Средняя               |
| `Hamming::Simple` | Средняя         | Высокая          | Высокая               |
| `Hamming::Mixed`  | Средняя         | Высокая          | Максимальная          |

### Скорость отправки 16 байт
|                   | `Encoder::None` | `Encoder::DC4_6` | `Encoder::Manchester` |
|-------------------|-----------------|------------------|-----------------------|
| `Hamming::None`   | 139 us          | 274 us           | 306 us                |
| `Hamming::Simple` | 279 us          | 549 us           | 610 us                |
| `Hamming::Mixed`  | 860 us          | 1070 us          | 1126 us               |

### Вес пакета
|                   | `Encoder::None` | `Encoder::DC4_6` | `Encoder::Manchester` |
|-------------------|-----------------|------------------|-----------------------|
| `Hamming::None`   | 100%            | 150%             | 200%                  |
| `Hamming::Simple` | 200%            | 300%             | 400%                  |
| `Hamming::Mixed`  | 200%            | 300%             | 400%                  |

> [!CAUTION]
> Библиотека принимает данные во встроенный буфер UART, свой буфер не создаёт. Это означает, что максимальная длина отправляемых данных ограничена буфером на приёмной стороне, например на AVR Arduino это в основном 64 байта, в ESP - 128. Размер буфера можно настроить, см. доку на конкретную платформу и реализацию UART. При расчёте размера пакета нужно смотреть на мультипликатор в таблице выше, т.е. для AVR Arduino при включенном `Hamming::Mixed` и `Encoder::Manchester` максимальный размер данных будет 16 байт, т.к. при передаче превратится в 64

### Скорость
Максимальная стабильная скорость для пары зелёных модулей (FS1000A и MX-RM-5V): ~15'000 бод. Рекомендуется использовать более низкие стандартные значения, например `9600`, `4800`, `1200`.

### Подключение
Радио-модуль нужно подключить к выводу UART TX на передатчике и RX на приёмнике. Таким образом на передатчике остаётся свободным RX, его можно использовать на приём в других целях, а на приёмнике - TX, его тоже можно использовать для своих целей, библиотека его не трогает.

На передатчике есть проблема - запущенный `Serial` подтягивает пин к VCC, поэтому радио будет активно передавать "единицу" всё свободное время и потреблять ток / засорять эфир. Это можно исправить несколькими способами:

- Запускать Serial перед отправкой и отключать после неё, в этом случае получится "ожидание отправки", т.к. `Serial.end()` вызывает `flush()` и ожидает отправки буфера. В то же время можно закрыть соединение по таймеру позже самостоятельно
- Поставить инвертер на транзисторе между радио и МК на приёмнике и на передатчике
- Отключать питание передатчика через некоторое время после отправки

### Описание классов
#### UART_RF_TX (отправка)
- `trainMs` - время в мс, которое будет активна "раскачка" радиоканала для синхронизации. Чем хуже потенциальное качество связи и чем хуже по качеству сами модули, тем дольше нужна раскачка. Хорошие модули (например зелёные FS1000A и MX-RM-5V) раскачиваются за 10 мс, плохие (например SYNxxx) - за 100 мс
- `baud` - скорость передачи, должна совпадать со скоростью UART. По сути используется для вычисления количества пакетов раскачки

```cpp
<class encoder_t = urf::Encoder::DC4_6, urf::Hamming hamming = urf::Hamming::None>
UART_RF_TX(Print& p, uint32_t baud, uint8_t trainMs = 20);

// ======== PACKET ========

// отправить пакет авто (тип URF_AUTO_TYPE)
void sendPacket(const Td& data);

// отправить пакет авто (тип URF_AUTO_TYPE)
void sendPacket(const void* data, size_t len);

// отправить пакет с типом
void sendPacketT(Tp type, const Td& data);

// отправить пакет с типом
void sendPacketT(Tp type, const void* data, size_t len);

// ======== RAW ========

// отправить сырые данные
void sendRaw(const T& data);

// отправить сырые данные
void sendRaw(const void* data, size_t len);

// прошло времени с конца последней отправки, мс
uint16_t lastSend();
```

#### UART_RF_RX (приём)
```cpp
<class decoder_t = urf::Decoder::DC4_6, urf::Hamming hamming = urf::Hamming::None>
UART_RF_RX(Stream& s, uint32_t baud);

// подключить обработчик пакета вида f(uint8_t type, void* data, size_t len)
void onPacket(PacketCallback cb);

// подключить обработчик сырых данных вида f(void* data, size_t len)
void onRaw(RawCallback cb);

// получить качество приёма в процентах (только для пакетов Packet)
uint8_t getRSSI();

// тикер, вызывать в loop
void tick();
```

### Отправка
Библиотека заканчивает приём по тайм-ауту, поэтому отправлять сообщения подряд нельзя, да и не имеет практического смысла.

#### Сырые данные
Библиотека может отправлять сырые данные: никак не контролирует размер и целостность - можно реализовать полностью свой протокол связи без оверхэда:

```cpp
UART_RF_TX<> tx(Serial, 4800);

struct Data {
  int i;
  float f;
};

uint32_t data_32 = 123456;             // целое
uint8_t data_arr[] = {1, 2, 3, 4, 5};  // массив
char cstr[10] = "hello";               // char array
String str = "hello";                  // String
Data data{1234, 3.1415};               // структура

// Serial.begin(4800);

// размер вручную
// tx.sendRaw(&data_32, sizeof(data_32));
// tx.sendRaw(data_arr, sizeof(data_arr));
// tx.sendRaw(cstr, strlen(cstr));
// tx.sendRaw(str.c_str(), str.length());
// tx.sendRaw(&data, sizeof(data));

// авто размер
// tx.sendRaw(data_32);
// tx.sendRaw(data_arr);
// tx.sendRaw(data);

// Serial.end();
```

#### Пакет
Библиотека также позволяет передавать данные по универсальному пакетному протоколу связи - метод `sendPacketT` - указывается тип передаваемых данных, чтобы на приёмном устройстве удобнее парсить пакет. В этом случае библиотека контролирует целостность и размер данных и не вызовет обработчик, если они повреждены. Например пакет типа `1` - 32 бит целое, пакет типа `2` - байтовый массив, `3` - строка произвольной длины, и так далее.

- Тип пакета - число от 0 до 30
- Размер данных - до 2047 Байт

```cpp
UART_RF_TX<> tx(Serial, 4800);

struct Data {
  int i;
  float f;
};

uint32_t data_32 = 123456;             // целое
uint8_t data_arr[] = {1, 2, 3, 4, 5};  // массив
char cstr[10] = "hello";               // char array
String str = "hello";                  // String
Data data{1234, 3.1415};               // структура

// noInterrupts();

// размер вручную
// tx.sendPacketT(0, &data_32, sizeof(data_32));
// tx.sendPacketT(1, data_arr, sizeof(data_arr));
// tx.sendPacketT(2, cstr, strlen(cstr));
// tx.sendPacketT(3, str.c_str(), str.length());
// tx.sendPacketT(4, &data, sizeof(data));

// авто размер
// tx.sendPacketT(0, data_32);
// tx.sendPacketT(1, data_arr);
// tx.sendPacketT(4, data);

// interrupts();
```

Тип для удобства может быть `enum`:

```cpp
enum class packet_t {
  Data32,
  Array,
  Cstring,
  String,
  Struct,
};

// tx.sendPacketT(packet_t::Data32, data_32);
// tx.sendPacketT(packet_t::Array, data_arr);
// tx.sendPacketT(packet_t::Cstring, cstr, strlen(cstr));
// tx.sendPacketT(packet_t::String, str.c_str(), str.length());
// tx.sendPacketT(packet_t::Struct, data);
```

Если не указывать тип пакета (метод `sendPacket`), то он будет равен типу `31` при парсинге (константа `URF_AUTO_TYPE`). Удобно, если в системе присутствует только один тип пакетов:

```cpp
// размер вручную
// tx.sendPacket(&data_32, sizeof(data_32));
// tx.sendPacket(data_arr, sizeof(data_arr));
// tx.sendPacket(cstr, strlen(cstr));
// tx.sendPacket(str.c_str(), str.length());
// tx.sendPacket(&data, sizeof(data));

// авто размер
// tx.sendPacket(data_32);
// tx.sendPacket(data_arr);
// tx.sendPacket(data);
```

### Приём
- Приём асинхронный в прерываниях UART
- Для получения данных нужно подключить функцию-обработчик
- В основном цикле программы нужно вызывать тикер `tick()` - в нём будет обработан пакет и вызван подключенный обработчик

```cpp
UART_RF_RX<> rx(Serial, 4800);

void setup() {
  Serial.begin(4800);

  // обработчик сырых данных, вызывается В ЛЮБОМ СЛУЧАЕ
  rx.onRaw([](void* data, size_t len) {
    // ...
  });

  // обработчик пакетов, вызывается только ПРИ КОРРЕКТНОМ ПАКЕТЕ
  rx.onPacket([](uint8_t type, void* data, size_t len) {
    // ...
  });
}

void loop() {
  rx.tick();
}
```

#### Парсинг пакетов
Если с сырыми данными всё понятно, то у пакета при получении нужно сверить тип и преобразовать данные в нужный формат. Пример с теми данными, которые отправляли выше:

```cpp
rx.onPacket([](uint8_t type, void* data, size_t len) {
    Serial.print("received type ");
    Serial.print(type);
    Serial.print(": ");

    switch (packet_t(type)) {
        case packet_t::Data32: {
            // можно дополнительно проверить длину данных, например тут if (len == 4)
            Serial.print(*((uint32_t*)data));
        } break;

        case packet_t::Array: {
            uint8_t* arr = (uint8_t*)data;
            for (size_t i = 0; i < len; i++) {
                Serial.print(arr[i]);
                Serial.print(',');
            }
        } break;

        case packet_t::Cstring: {
            Serial.write((uint8_t*)data, len);
        } break;

        case packet_t::String: {
            Serial.write((uint8_t*)data, len);
        } break;

        case packet_t::Struct: {
            Data& p = *((Data*)data);
            Serial.print(p.i);
            Serial.print(',');
            Serial.print(p.f);
        } break;
    }
    Serial.println();
});
```

## Примеры
### Отправка структуры
```cpp
#include <UART_RF_TX.h>

struct Data {
    int i;
    float f;
};

void setup() {
    Serial.begin(4800);
}

UART_RF_TX<> tx(Serial, 4800);

void loop() {
    static int i;
    Data data{i++, 3.14};

    // Serial.begin(4800);
    tx.sendPacket(data);
    // Serial.end();

    delay(1000);
}
```


### Приём структуры
```cpp
#include <UART_RF_RX.h>

struct Data {
    int i;
    float f;
};

UART_RF_RX<> rx(Serial, 4800);

void setup() {
    Serial.begin(4800);

    rx.onPacket([](uint8_t type, void* data, size_t len) {
        if (sizeof(Data) != len) return;  // проверка корректности длины
        Data& d = *((Data*)data);
        Serial.print(d.i);
        Serial.print(',');
        Serial.println(d.f);

        // или
        // Serial.println(static_cast<Data*>(data)->f);
    });
}

void loop() {
    rx.tick();
}
```

<a id="versions"></a>

## Версии
- v1.0

<a id="install"></a>
## Установка
- Библиотеку можно найти по названию **UART_RF** и установить через менеджер библиотек в:
    - Arduino IDE
    - Arduino IDE v2
    - PlatformIO
- [Скачать библиотеку](https://github.com/GyverLibs/UART_RF/archive/refs/heads/main.zip) .zip архивом для ручной установки:
    - Распаковать и положить в *C:\Program Files (x86)\Arduino\libraries* (Windows x64)
    - Распаковать и положить в *C:\Program Files\Arduino\libraries* (Windows x32)
    - Распаковать и положить в *Документы/Arduino/libraries/*
    - (Arduino IDE) автоматическая установка из .zip: *Скетч/Подключить библиотеку/Добавить .ZIP библиотеку…* и указать скачанный архив
- Читай более подробную инструкцию по установке библиотек [здесь](https://alexgyver.ru/arduino-first/#%D0%A3%D1%81%D1%82%D0%B0%D0%BD%D0%BE%D0%B2%D0%BA%D0%B0_%D0%B1%D0%B8%D0%B1%D0%BB%D0%B8%D0%BE%D1%82%D0%B5%D0%BA)
### Обновление
- Рекомендую всегда обновлять библиотеку: в новых версиях исправляются ошибки и баги, а также проводится оптимизация и добавляются новые фичи
- Через менеджер библиотек IDE: найти библиотеку как при установке и нажать "Обновить"
- Вручную: **удалить папку со старой версией**, а затем положить на её место новую. "Замену" делать нельзя: иногда в новых версиях удаляются файлы, которые останутся при замене и могут привести к ошибкам!

<a id="feedback"></a>

## Баги и обратная связь
При нахождении багов создавайте **Issue**, а лучше сразу пишите на почту [alex@alexgyver.ru](mailto:alex@alexgyver.ru)  
Библиотека открыта для доработки и ваших **Pull Request**'ов!

При сообщении о багах или некорректной работе библиотеки нужно обязательно указывать:
- Версия библиотеки
- Какой используется МК
- Версия SDK (для ESP)
- Версия Arduino IDE
- Корректно ли работают ли встроенные примеры, в которых используются функции и конструкции, приводящие к багу в вашем коде
- Какой код загружался, какая работа от него ожидалась и как он работает в реальности
- В идеале приложить минимальный код, в котором наблюдается баг. Не полотно из тысячи строк, а минимальный код