#ifndef MORSEMENU_H_
#define MORSEMENU_H_

#include <Arduino.h>

#include "MorseText.h"
#include "MorseMode.h"
#include "MorsePreferences.h"

namespace MorseMenu
{

    enum menuNo
    {
        _dummy,
        _keyer,
        _gen,
        _genRand,
        _genAbb,
        _genWords,
        _genCalls,
        _genMixed,
        _genPlayer,
        _echo,
        _echoRand,
        _echoAbb,
        _echoWords,
        _echoCalls,
        _echoMixed,
        _echoPlayer,
        _koch,
        _kochSel,
        _kochLearn,
        _kochGen,
        _kochGenRand,
        _kochGenAbb,
        _kochGenWords,
        _kochGenMixed,
        _kochEcho,
        _kochEchoRand,
        _kochEchoAbb,
        _kochEchoWords,
        _kochEchoMixed,
        _head,
        _headRand,
        _headAbb,
        _headWords,
        _headCalls,
        _headMixed,
        _headPlayer,
        _tennis,
        _trx,
        _trxLora,
        _trxIcw,
        _decode,
        _wifi,
        _wifi_mac,
        _wifi_config,
        _wifi_check,
        _wifi_upload,
        _wifi_update,
        _goToSleep
    };

    typedef struct menuItem_t
    {
            /*
             * The text to display in the menu.
             */
            String text;
            /**
             * The menu item's ID.
             */
            menuNo no;
            /*
             * An array with navigation information. See enum navi.
             */
            uint8_t nav[5];
            /**
             * The mode the MorseGenerator is operated in when selecting this menu item.
             * This member will likely go away soon.
             */
            MorseText::GEN_TYPE generatorMode;
            /**
             * The array of preferences available when selecting this menu item.
             */
            MorsePreferences::prefPos *options;
            /**
             * If Morselino is supposed to remember this menu item when rebooting.
             */
            boolean remember;
            /**
             * A function that is called when the menu item is selected.
             */
            boolean (*menufx)(String);
            /**
             * A string ID passed to menufx.
             */
            String menufxParam;
            /**
             * A pointer to a MorseMode object handling this item.
             * If this pointer is not 0, the mode will also take precedence over the above defined menufx function.
             */
            MorseMode *mode;
    } MenuItem;

    void setup();
    void menu_();
    void cleanStartSettings();

    boolean isCurrentMenuItem(menuNo test);
    const MenuItem* getCurrentMenuItem();
}

#endif /* MORSEMENU_H_ */
