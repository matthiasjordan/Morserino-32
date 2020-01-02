/*
 * MorseUI.h
 *
 *  Created on: 27.12.2019
 *      Author: mj
 */

#ifndef MORSEUI_H_
#define MORSEUI_H_

#include "ClickButton.h"   // button control library


namespace MorseUI {

    // define the buttons for the clickbutton library
    extern ClickButton modeButton;
    extern ClickButton volButton;


    void setup();
    void click();
    void audioLevelAdjust();

}



#endif /* MORSEUI_H_ */
