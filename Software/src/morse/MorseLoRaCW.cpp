/******************************************************************************************************************************
 *  morse_3 Software for the Morserino-32 multi-functional Morse code machine, based on the Heltec WiFi LORA (ESP32) module ***
 *  Copyright (C) 2018  Willi Kraml, OE1WKL                                                                                 ***
 *
 *  This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License
 *  as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty
 *  of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License along with this program.
 *  If not, see <https://www.gnu.org/licenses/>.
 *****************************************************************************************************************************/

#include <Arduino.h>
#include "SPIFFS.h"
#include <SPI.h>           // library for SPI interface
#include <LoRa.h>          // library for LoRa transceiver

#include "MorseDisplay.h"
#include "MorsePreferencesMenu.h"
#include "MorseMachine.h"
#include "MorseLoRa.h"
#include "MorseLoRaCW.h"

using namespace MorseLoRaCW;

/////////////////// Variables for LoRa: Buffer management etc

char loraTxBuffer[32];


uint8_t loRaSerial = random(64);    /// a 6 bit serial number, start with some random value, will be incremented witch each sent LoRa packet
                                    /// the first two bits in teh byte will be the protocol id (CWLORAVERSION)

namespace internal
{
    MorseLoRaCW::Packet decodePacket(MorseLoRa::RawPacket &rp);
}

char* MorseLoRaCW::getTxBuffer() {
    return loraTxBuffer;
}


/// cwForLora packs element info (dit, dah, interelement space) into a String that can be sent via LoRA
///  element can be:
///  0: inter-element space
///  1: dit
///  2: dah
///  3: end of word -: cwForLora returns a string that is ready for sending to the LoRa transceiver

//  char loraTxBuffer[32];

void MorseLoRaCW::cwForLora(int element)
{
    static int pairCounter = 0;
    uint8_t temp;
    uint8_t header;

    if (pairCounter == 0)
    {   // we start a brand new word for LoRA - clear buffer and set speed first
        for (int i = 0; i < 32; ++i) {
            loraTxBuffer[i] = (char) 0;
        }

        /// 1st byte: version + serial number
        header = ++loRaSerial % 64;
        header += CWLORAVERSION * 64;        //// shift VERSION left 6 bits and add to the serial number
        loraTxBuffer[0] = header;

        temp = MorsePreferences::prefs.wpm * 4;                   /// shift left 2 bits
        loraTxBuffer[1] |= temp;
        pairCounter = 7;         /// so far we have used 7 bit pairs: 4 in the first byte (protocol version+serial); 3 in the 2nd byte (wpm)
    }

    temp = element & B011;      /// take the two left bits
    /// now store these two bits in the correct location in loraBuffer

    if (temp && (temp != 3))
    {                 /// no need to do the operation with 0, nor with B11
        temp = temp << (2 * (3 - (pairCounter % 4)));
        loraTxBuffer[pairCounter / 4] |= temp;
    }

    /// now increment, unless we got end of word
    /// have we get end of word, we got end of character (0) before

    if (temp != 3)
    {
        ++pairCounter;
    }
    else
    {
        --pairCounter; /// we are at end of word and step back to end of character
        if (pairCounter % 4 != 0)
        {           // do nothing if we have a zero in the topmost two bits already, as this was end of character
            temp = temp << (2 * (3 - (pairCounter % 4)));
            loraTxBuffer[pairCounter / 4] |= temp;
        }
        pairCounter = 0;
    }
}


MorseLoRaCW::Packet MorseLoRaCW::decodePacket()
{
    MorseLoRa::RawPacket rp = MorseLoRa::decodePacket();
    MorseLoRaCW::Packet packet = internal::decodePacket(rp);

    if (packet.protocolVersion() != CWLORAVERSION)
    {
        // invalid protocol version
        packet.valid = false;
        packet.payload = "";
    }

    if ((packet.rxWpm < 5) || (packet.rxWpm > 60))
    {
        // invalid speed
        packet.valid = false;
        packet.payload = "";
    }

    return packet;
}




/// decodePacket analyzes packet as received and stored in buffer
/// returns the header byte (protocol version*64 + 6bit packet serial number
//// byte 0 (added by receiver): RSSI
//// byte 1: header; first two bits are the protocol version (curently 01), plus 6 bit packet serial number (starting from random)
//// byte 2: first 6 bits are wpm (must be between 5 and 60; values 00 - 04 and 61 to 63 are invalid), the remaining 2 bits are already data payload!

MorseLoRaCW::Packet internal::decodePacket(MorseLoRa::RawPacket &rp)
{
    MorseLoRaCW::Packet p;
    p.rssi = rp.rssi;

    uint8_t l = rp.payloadLength;

    for (int i = 0; i < l; ++i)
    {     // decoding loop
        uint8_t c = rp.payload[i];

        switch (i)
        {
            case 0:
            {
                p.header = c;
                break;
            }
            case 1:
            {
                p.rxWpm = (uint8_t) (c >> 2); // the first data byte contains the wpm info in the first six bits, and actual morse in the remaining two bits
                                           // now take remaining two bits and store them in CWword as ASCII
                char cx = (char) ((c & B011) + 48);
                p.payload = cx;
                break;
            }
            default:
            {
                // decode the rest of the packet; we do this for all but the first byte  /// we need to handle end of word!!! therefore the break

                for (int j = 0; j < 4; ++j)
                {
                    char cc = ((c >> 2 * (3 - j)) & B011);                // we store them as ASCII characters 0,1,2,3 !
                    if (cc != 3)
                    {
                        char cx = (char) (cc + 48);
                        p.payload += cx;
                    }
                    else
                    {
                        break;
                    }
                }
                break;
            }
        } // end switch
    }   // end for
    return p;
}      // end decodePacket

