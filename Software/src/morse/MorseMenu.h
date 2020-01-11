#ifndef MORSEMENU_H_
#define MORSEMENU_H_

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

    void setup();
    void menu_();
    void cleanStartSettings();

    boolean isCurrentMenuItem(menuNo test);

}

#endif /* MORSEMENU_H_ */
