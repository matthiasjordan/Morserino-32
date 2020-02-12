/*
 * arduino.h
 *
 *  Created on: 31.01.2020
 *      Author: mj
 */

#ifdef ESP_PLATFORM
#include <Arduino.h>
#else
#include "mock_arduino.h"
#endif /* ARDUINO_H_ */
