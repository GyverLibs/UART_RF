This is an automatic translation, may be incorrect in some places. See sources and examples!

# Uart_rf
Library for data transmission through 433 MHz Radio Moduli by UART
- if UART is hardware - does not load MK interrupts and independently accepts data
- Three options for DC Coding: Disconnected, Manchester and 4B/6B
- Three options for correction by Hamming: disabled, coding of each byte and coding with mixing
- the use of the native buffer UART -A - asynchronous shipping and reception
- Sending the raw data and package mode with the control of integrity and the type of package

## compatibility
Compatible with all arduino platforms (used arduino functions)

### Dependencies
- [Hamming] (https://github.com/gyverlibs/hamming)

## Content
- [documentation] (#usage)
- [versions] (#varsions)
- [installation] (# Install)
- [bugs and feedback] (#fedback)

<a id = "USAGE"> </A>

## Documentation
The library sends data via the Arduino interface `Print`, adding a special sequence of characters to the beginning of the package to stabilize the radio channel and synchronize the start-stop UART.The transfer and reception may be additionally encoded to improve the quality of communication:

- Balanced coding
- `Encoder :: dc4_6` (by the silence) - 4b/6b, each nibble is encoded 6 bits
- `Encoder :: Manchester` - the accelerated implementation of Manchester 4/4,` ~ nible |Nibble`
- Hamming - Excessive code to restore damaged data
- `without coding` (by default)
- `Hamming :: Simple` - an excessive algorithm (8.4) for each byte in order
- `Hamming :: mixed` - the bats are mixed before sending to increase resistance to interference

> [! Warning]
> The coding mode must coincide on the transmitter and receiver!

## reliability
||`Encoder :: None` |`Encoder :: dc4_6` |`Encoder :: Manchester` |
| ----------------- | ----------------- | -----------------------------------------------------------------------------------------------------------------------------
|`Hamming :: None` |Low |Average |Average |
|`Hamming :: Simple` |Average |High |High |
|`Hamming :: mixed` |Average |High |Maximum |

### Shipping speed 16 bytes
||`Encoder :: None` |`Encoder :: dc4_6` |`Encoder :: Manchester` |
| ----------------- | ----------------- | -----------------------------------------------------------------------------------------------------------------------------
|`Hamming :: None` |139 US |274 US |306 US |
|`Hamming :: Simple` |279 US |549 US |610 US |
|`Hamming :: mixed` |860 us |1070 US |1126 US |

### weight package
||`Encoder :: None` |`Encoder :: dc4_6` |`Encoder :: Manchester` |
| ----------------- | ----------------- | -----------------------------------------------------------------------------------------------------------------------------
|`Hamming :: None` |100% |150% |200% |
|`Hamming :: Simple` |200% |300% |400% |
|`Hamming :: mixed` |200% |300% |400% |

> [! Caution]
> The library accepts data in the built -in UART buffer, does not create its own buffer.ThisCranberries mean that the maximum length of the data sent is limited by the buffer on the receiving side, for example, on AVR Arduino, this is mainly 64 bytes, in ESP - 128. The size of the buffer can be configured, see Doks for a specific platform and UART implementation.When calculating the size of the package, you need to look at the multiplier in the table above, i.e.For AVR Arduino, with the `Hamming :: mixed` and` encoder :: Manchester 'maximum data size will be 16 bytes, becausewhen transmitting it will turn into 64

### Speed
The maximum stable speed for a pair of green modules (FS1000A and MX-RM-5V): ~ 15'000 BOD.It is recommended to use lower standard values, for example, `9600`,` 4800`, `1200`.

### connection
The radio module must be connected to the UART TX output on the transmitter and RX at the receiver.Thus, the transmitter remains free RX, it can be used to receive for other purposes, and at the receiver - TX, it can also be used for its purposes, the library does not touch it.

There is a problem on the transmitter - the neglected `serial` pulls the pin to the VCC, so the radio will actively transfer the“ unit ”all free time and consume current / clog the ether.This can be fixed in several ways:

- Run Serial before sending and disconnecting after it, in this case you will get a "expectation of sending", because`Serial.end ()` calls `flush ()` and expects to send the buffer.At the same time, you can close the tier connection later on your own
- put an inverter on a transistor between the radio and MK on the receiver and on the transmitter
- turn off the power of the transmitter some time after sending

### description of classes
### uart_rf_tx (Sending)
- `Trainms` - the time in the MS, which will be active in the" swing "of the radio channel for synchronization.The worse the potential quality of communication and the worse the quality of the modules themselves, the longer the swing is needed.Good modules (for example, green FS1000A and MX-RM-5V) swing for 10 ms, bad (for example, Synxxx)-for 100 ms
- `Baud` - the speed of transmission should coincide with the speed of UART.In fact, it is used to calculate the number of buildup packages

`` `CPP
<class encoder_t = URF :: Encoder :: DC4_6, URF :: Hamming Hamming = URF :: Hamming :: None>
Uart_rf_tx (Print & p, uint32_t baud, uint8_t Trainms = 20);

// ======== Packet ==========

// Send a car package (type URF_AUTO_TYPE)
Void Sendpacket (Consta TD & DATA);

// Send a car package (type URF_AUTO_TYPE)
VOID SENDPACket (Consta* Data, Size_t Len);

// Send a package with a type
Void Sendpackett (TP Type, Consta TD & DATA);

// Send a package with a type
Void Sendpackett (TP Type, Consta* Data, Size_t Len);

// ======== RAW =============

// Send raw data
VOID SENDRAW (Consta T & Data);

// Send raw data
VOID SENDRAW (const VOID* DATA, SIZE_T LEN);

// time has passed since the end of the last shipment, ms
uint16_t LastSend ();
`` `

### uart_rf_rx (reception)
`` `CPP
<class decoder_t = URF :: decoder :: DC4_6, URF :: Hamming Hamming = URF :: Hamming :: None>
Uart_rf_rx (Stream & s, uint32_t baud);

// Connect the packet handler F (Uint8_t Type, Void* Data, Size_t Len)
VOID Onpacket (PacketCallback CB);

// Connect the raw data processor F (VOID* DATA, Size_t Len)
VOID online (RAWCALLBACK CB);

// Get the quality of reception as a percentage (only for Packet packages)
uint8_t getrssi ();

// ticker, call in loop
VOID Tick ();
`` `

### Sending
The library finishes a time-out reception, so you cannot send messages in a row, and does not make practical sense.

### raw data
The library can send raw data: it does not control the size and integrity in any way - you can fully realize your communication protocol without over edges:

`` `CPP
Uart_rf_tx <> tx (Serial, 4800);

Struct Data {
int i;
Float F;
};

uint32_t data_32 = 123456;// whole
uint8_t data_arr [] = {1, 2, 3, 4, 5};// Massive
Char CSTR [10] = "HELLO";// char Array
String str = "Hello";// String
Data Data {1234, 3.1415};// Structure

// serial.Begin (4800);

// Size manually
// TX.SendRAW (& DATA_32, SIZEOF (Data_Cranberries 32));
// tx.sendraw (Data_arr, Sizeof (Data_arr));
// tx.sendraw (CSTR, Strlen (CSTR));
// tx.sendraw (str.c_str (), str.length ());
// TX.SendRAW (& DATA, SIZEOF (DATA));

// Auto size
// tx.sendraw (Data_32);
// tx.sendraw (Data_arr);
// tx.Sendraw (Data);

// serial.end ();
`` `

#### Plastic bag
The library also allows you to transmit data on the universal bag of communication - the `sendpackett` method - the type of data is indicated so that it is more convenient to parse the package on the receiving device.In this case, the library controls the integrity and size of the data and does not cause the processor if they are damaged.For example, a package of type `1` - 32 bits the whole, a package of the` 2` - byte array, `3` - a string of arbitrary length, and so on.

- Type of package - number from 0 to 30
- Data size - up to 2047 bytes

`` `CPP
Uart_rf_tx <> tx (Serial, 4800);

Struct Data {
int i;
Float F;
};

uint32_t data_32 = 123456;// whole
uint8_t data_arr [] = {1, 2, 3, 4, 5};// Massive
Char CSTR [10] = "HELLO";// char Array
String str = "Hello";// String
Data Data {1234, 3.1415};// Structure

// nointerrupts ();

// Size manually
// tx.Sendpackett (0, & Data_32, Sizeof (Data_32));
// tx.Sendpackett (1, Data_arr, Sizeof (Data_arr));
// tx.Sendpackett (2, CSTR, StREN (CSTR));
// tx.sendpackett (3, str.c_str (), str.length ());
// TX.Sendpackett (4, & Data, Sizeof (Data));

// Auto size
// tx.sendpackett (0, Data_32);
// tx.Sendpackett (1, Data_arr);
// tx.Sendpackett (4, Data);

// Interrupts ();
`` `

The type for convenience can be `enum`:

`` `CPP
enum class packet_t {
Data32,
Array,
CSTring,
String,
Struct,
};

// tx.Sendpackett (Packet_t :: Data32, Data_32);
// tx.Sendpackett (Packet_t :: Array, Data_arr);
// tx.Sendpackett (Packet_t :: Cstring, Cstr, Strlen (CSTR));
// tx.Sendpackett (packet_t :: string, str.c_str (), str.length ());
// tx.Sendpackett (Packet_t :: Struct, Data);
`` `

If you do not specify the type of package (the `sendpacket` method), then it will be equal to the type` 31` during parsing (constant `urf_auto_type`).It is convenient if the system contains only one type of package:

`` `CPP
// Size manually
// TX.Sendpacket (& Data_32, Sizeof (Data_32));
// tx.sendpacket (Data_arr, Sizeof (Data_arr));
// tx.Sendpacket (CSTR, Strlen (CSTR));
// tx.sendpacket (str.c_str (), str.length ());
// TX.Sendpacket (& Data, Sizeof (Data));

// Auto size
// tx.Sendpacket (Data_32);
// tx.Sendpacket (Data_arr);
// tx.Sendpacket (Data);
`` `

### Reception
- Asynchronous reception in UART interruptions
- To obtain data, you need to connect a processing function
- In the main cycle of the program, you need to call a ticker `tick ()` - a package will be processed in it and a connected processor will be called

`` `CPP
Uart_rf_rx <> rx (Serial, 4800);

VOID setup () {
Serial.Begin (4800);

// processor of raw data, is called in any case
rx.onraw ([] (void* data, size_t len) {
// ...
});

// packet handler, caused only with the correct package
rx.onpacket ([] (Uint8_t Type, VOID* DATA, SIZE_T LEN) {
// ...
});
}

VOID loop () {
rx.tick ();
}
`` `

#### Parsing packages
If everything is clear with raw data, then at the package you need to verify the type and convert the data into the desired format.An example with the data that sent above:

`` `CPP
rx.onpacket ([] (Uint8_t Type, VOID* DATA, SIZE_T LEN) {
Serial.print ("Receved Type");
Serial.print (type);
Serial.print (":");

Switch (Packet_t (Type)) {
Case Packet_t :: Data32: {
// you can additionally check the data length, for example, here if (Len == 4)
Serial.print (*((uint32_t*) data));
} Break;

Case Packet_t :: Array: {
uint8_t* arr = (uint8_t*) Data;
for (size_t i = 0; i <len; i ++) {
Serial.print (arr [i]);
Serial.print (',');
}
} Break;

Case Packet_t :: Cstring: {
Serial.write ((uint8_t*) Data, Len);
} Break;

Case Packet_t :: String: {
Serial.write ((uint8_t*) Data, Len);
} Break;

Case PAcket_t :: struct: {
Data & P = *((Data *) Data);
Serial.print (P.I);
Serial.print (',');
Serial.print (P.F);
} Break;
}
Serial.println ();
});
`` `

## Examples
### Sending the structure
`` `CPP
#include <uart_rf_tx.h>

Struct Data {
int i;
Float F;
};

VOID setup () {
Serial.Begin (4800);
}

Uart_rf_tx <> tx (Serial, 4800);

VOID loop () {
static int i;
Data Data {I ++, 3.14};

// serial.Begin (4800);
tx.Sendpacket (Data);
// serial.end ();

DELAY (1000);
}
`` `


### Establishment of the structure
`` `CPP
#include <uart_rf_rx.h>

Struct Data {
int i;
Float F;
};

Uart_rf_rx <> rx (Serial, 4800);

VOID setup () {
Serial.Begin (4800);

rx.onpacket ([] (Uint8_t Type, VOID* DATA, SIZE_T LEN) {
if (sizeof (Data)! = Len) return;// Checking the correctness of the length
Data & D = *((Data *) Data);
Serial.print (D.I);
Serial.print (',');
Serial.println (D.F);

// or
// serial.println (static_cast <data*> (data)-> f);
});
}

VOID loop () {
rx.tick ();
}
`` `

<a ID = "Versions"> </a>

## versions
- V1.0

<a id = "Install"> </a>
## Installation
- The library can be found by the name ** uart_rf ** and installed through the library manager in:
- Arduino ide
- Arduino ide v2
- Platformio
- [download the library] (https://github.com/gyverlibs/uart_rf/archive/refs/heads/main.zip) .Zip archive for manual installation:
- unpack and put in * C: \ Program Files (X86) \ Arduino \ Libraries * (Windows X64)
- unpack and put in * C: \ Program Files \ Arduino \ Libraries * (Windows X32)
- unpack and put in *documents/arduino/libraries/ *
- (Arduino id) Automatic installation from. Zip: * sketch/connect the library/add .Zip library ... * and specify downloaded archive
- Read more detailed instructions for installing libraries[here] (https://alexgyver.ru/arduino-first/#%D0%A3%D1%81%D1%82%D0%B0%D0%BD%D0%BE%D0%B2%D0%BA%D0%B0_%D0%B1%D0%B8%D0%B1%D0%B8%D0%BE%D1%82%D0%B5%D0%BA)
### Update
- I recommend always updating the library: errors and bugs are corrected in the new versions, as well as optimization and new features are added
- through the IDE library manager: find the library how to install and click "update"
- Manually: ** remove the folder with the old version **, and then put a new one in its place.“Replacement” cannot be done: sometimes in new versions, files that remain when replacing are deleted and can lead to errors!

<a id = "Feedback"> </a>

## bugs and feedback
Create ** Issue ** when you find the bugs, and better immediately write to the mail [alex@alexgyver.ru] (mailto: alex@alexgyver.ru)
The library is open for refinement and your ** pull Request ** 'ow!

When reporting about bugs or incorrect work of the library, it is necessary to indicate:
- The version of the library
- What is MK used
- SDK version (for ESP)
- version of Arduino ide
- whether the built -in examples work correctly, in which the functions and designs are used, leading to a bug in your code
- what code has been loaded, what work was expected from it and how it works in reality
- Ideally, attach the minimum code in which the bug is observed.Not a canvas of a thousand lines, but a minimum code