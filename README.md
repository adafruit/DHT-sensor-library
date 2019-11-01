# Adafruit DHT Humidity & Temperature Sensor Library [![Build Status](https://travis-ci.com/adafruit/DHT-sensor-library.svg?branch=master)](https://travis-ci.com/adafruit/DHT-sensor-library)

## Description

An Arduino library for the DHT series of low-cost temperature/humidity sensors.

You can find DHT tutorials [here](https://learn.adafruit.com/dht).

## Installation

### First Method

![image](https://user-images.githubusercontent.com/36513474/67982415-773d6a00-fc44-11e9-8741-8185da71e785.png)

1. In the Arduino IDE, navigate to Sketch > Include Library > Manage Libraries
1. Then the Library Manager will open and you will find a list of libraries that are already installed or ready for installation.
1. Then search for DHT-sensor using the search bar.
1. Click on the text area and then select the specific version and install it.

### Second Method

1. Navigate to the Releases page.
1. Download the latest release.
1. Extract the zip file
1. In the Arduino IDE, navigate to Sketch > Include Library > Add .ZIP Library

## Requirements

This library depends on [Adafruit Unified Sensor Library](https://github.com/adafruit/Adafruit_Sensor). To use this library the user must download the required library.

## Features

- ### Inexpensive

     This library is used with low-cost temperature and humidity sensors, for example, DHT11 and DHT22. This library is free of cost and the only cost is of the sensors.

- ### Compatible

    DHT sensor library is compatible with multiple low-cost temperature and humidity sensors like DHT11 and DHT22. A few examples are implemented just to demonstrate how to modify the code for different sensors.

- ### Function calls

    Basic functions of the low-cost temperature/humidity sensors have been implemented in this library. There's no need to re-implement these functions from scratch. The user simply has to import the library in the project and can use any of its functions by just calling it.

- ### Give back

    The library is free, you don’t have to pay for anything. However, if you want to support the development, or just thank the author of the library by purchasing products from Adafruit!

    Not only you’ll encourage the development of the library, but you’ll also learn how to best use the library and probably some C++ too

- ### MIT License

    DHT sensor library is open-source and uses one of the most permissive licenses so you can use it on any project.

  - Commercial use
  - Modification
  - Distribution
  - Private use

## Functions

- begin()
- readTemperature()
- convertCtoF()
- convertFtoC()
- readHumidity()
- computeHeatIndex()
- read()
- expectPulse()

## Example

Examples include both a "standalone" DHT example and one that works along with the Adafruit Unified Sensor Library. A Unified sensor library is required even if using the standalone version. You can find other examples from [Github-DHT-sensor-library](https://github.com/adafruit/DHT-sensor-library/tree/master/examples).

```Cpp
#include "DHT.h"

#define DHTPIN 2
#define DHTTYPE DHT22
DHT dht(DHTPIN, DHTTYPE);

void setup() {
  Serial.begin(9600);
  Serial.println(F("DHTxx test!"));

  dht.begin();
}

void loop() {
  delay(2000);

  float h = dht.readHumidity();
  float t = dht.readTemperature();
  float f = dht.readTemperature(true);

  if (isnan(h) || isnan(t) || isnan(f)) {
    Serial.println(F("Failed to read from DHT sensor!"));
    return;
  }

  float hif = dht.computeHeatIndex(f, h);
  float hic = dht.computeHeatIndex(t, h, false);

  Serial.print(F("Humidity: "));
  Serial.print(h);
  Serial.print(F("%  Temperature: "));
  Serial.print(t);
  Serial.print(F("°C "));
  Serial.print(f);
  Serial.print(F("°F  Heat index: "));
  Serial.print(hic);
  Serial.print(F("°C "));
  Serial.print(hif);
  Serial.println(F("°F"));
}
```

## Contributing

If you want to contribute to this project:

- Report bugs and errors
- Ask for enhancements
- Create issues and pull requests
- Tell others about this library
- Contribute new protocols

Please read [CONTRIBUTING.md](https://github.com/adafruit/DHT-sensor-library/blob/master/CONTRIBUTING.md) for details on our code of conduct, and the process for submitting pull requests to us.

## Credits

The author and maintainer of this library is Adafruit <info@adafruit.com>

Based on previous work by:

- T. DiCola
- P. Y. Dragon
- L. Fried
- J. Hoffmann
- M. Kooijman
- J. M. Dana
- S. Conaway
- S. IJskes
- T. Forbes
- B. C
- T. J Myers
- L. Sørup
- per1234
- O. Duffy
- matthiasdanner
- J. Lim
- G. Ambrozio
- chelmi
- adams13x13
- Spacefish
- I. Scheller
- C. Miller
- 7eggert

## License

This library is licensed under [MIT license](https://opensource.org/licenses/MIT).
