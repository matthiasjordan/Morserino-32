#include "MorseSystem.h"

using namespace MorseSystem;

//// measure battery voltage in mV
int16_t batteryVoltage()
{
    int32_t c, diff;

#if BOARDVERSION == 3
    WiFi.mode( WIFI_MODE_NULL );      // make sure WiFi is not running, as it uses the same ADC as battery measurement!
    const float XS = 1.95;//The returned reading is multiplied by this XS to get the battery voltage.

    analogSetClockDiv(128);//  this value was found by experimenting - no clue what it really does :-(
    analogSetPinAttenuation(batteryPin,ADC_11db);
    //delay(75);
    c = 0;
    for (int i = 0; i < 2048; ++i)
    c += analogRead(batteryPin);

    c = (int) c*XS;
    c = c / 2048;
    //printOnScroll(1, REGULAR, 1, String(c));
    analogSetClockDiv(1);// 5ms

#elif BOARDVERSION == 2     // probably buggy - but BOARDVERSION 2 is not supported anymore, was prototype only
    adcAttachPin(batteryPin);

    const float XS = 1.255;
    c = analogRead(batteryPin);
    delay(100);
    c = analogRead(batteryPin);
    c = (int) c*XS;

#endif

    return c;
}

void resetTOT()
{       //// reset the Time Out Timer - we do this whenever there is a screen update
    TOTcounter = millis();
}

void checkShutDown(boolean enforce)
{       /// if enforce == true, we shut donw even if there was no time-out
    // unsigend long timeOut = ((morseState == loraTrx) || (morseState == morseTrx)) ? 450000 : 300000;  /// 7,5 or 5 minutes
    unsigned long timeOut;
    switch (p_timeOut)
    {
        case 4:
            timeOut = ULONG_MAX;
            break;
        default:
            timeOut = 300000UL * p_timeOut;
            break;
    }

    if ((millis() - TOTcounter) > timeOut || enforce == true)
    {
        display.clear();
        printOnScroll(1, INVERSE_BOLD, 0, "Power OFF...");
        printOnScroll(2, REGULAR, 0, "RED to turn ON");
        display.display();
        delay(1500);
        shutMeDown();
    }
}

void shutMeDown()
{
    display.sleep();                //OLED sleep
    LoRa.sleep();                   //LORA sleep
    delay(50);
#if BOARDVERSION == 3
    digitalWrite(Vext,HIGH);
#endif
    delay(50);
    esp_deep_sleep_start();         // go to deep sleep
}

