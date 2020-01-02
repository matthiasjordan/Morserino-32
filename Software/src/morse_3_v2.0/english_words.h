#ifndef ENGLISH_WORDS_H
#define ENGLISH_WORDS_H

/// The most common English Words in various lengths for CW Trainer
///////////////////////////////////////////////////////////////////

namespace EnglishWords
{

    const int WORDS_NUMBER_OF_ELEMENTS = 204;                                    // how many items all together?
    const int WORDS_MAX_SIZE = 14;                                                // longest item  +1
    const int WORDS_POINTER[] =
        {0, 201, 181, 146, 90, 46, 26};                                                // array where items start with length = index

    const String words[WORDS_NUMBER_OF_ELEMENTS] =
        {
            {"international"},
            {"university"},
            {"government"},
            {"including"},
            {"following"},
            {"national"},
            {"american"},
            {"released"},
            {"although"},
            {"district"},
            {"between"},        /// l = 7, pos = 10
                    {"however"},
                    {"through"},
                    {"several"},
                    {"history"},
                    {"against"},
                    {"because"},
                    {"located"},
                    {"company"},
                    {"general"},
                    {"another"},
                    {"century"},
                    {"station"},
                    {"british"},
                    {"college"},
                    {"members"},
                    {"during"},       /// l = 6, pos = 26
                    {"school"},
                    {"united"},
                    {"states"},
                    {"became"},
                    {"before"},
                    {"people"},
                    {"second"},
                    {"called"},
                    {"series"},
                    {"number"},
                    {"family"},
                    {"county"},
                    {"system"},
                    {"season"},
                    {"played"},
                    {"around"},
                    {"public"},
                    {"former"},
                    {"career"},
                    {"which"},      /// l = 5, pos = 46
                    {"first"},
                    {"their"},
                    {"after"},
                    {"other"},
                    {"there"},
                    {"years"},
                    {"would"},
                    {"where"},
                    {"later"},
                    {"these"},
                    {"about"},
                    {"under"},
                    {"world"},
                    {"known"},
                    {"while"},
                    {"state"},
                    {"three"},
                    {"being"},
                    {"early"},
                    {"since"},
                    {"until"},
                    {"south"},
                    {"north"},
                    {"music"},
                    {"album"},
                    {"group"},
                    {"often"},
                    {"those"},
                    {"house"},
                    {"began"},
                    {"could"},
                    {"found"},
                    {"major"},
                    {"river"},
                    {"named"},
                    {"still"},
                    {"place"},
                    {"local"},
                    {"party"},
                    {"large"},
                    {"small"},
                    {"along"},
                    {"based"},
                    {"with"},     /// l = 4, pos = 90
                    {"that"},
                    {"from"},
                    {"were"},
                    {"this"},
                    {"also"},
                    {"have"},
                    {"they"},
                    {"been"},
                    {"when"},
                    {"into"},
                    {"more"},
                    {"time"},
                    {"most"},
                    {"some"},
                    {"only"},
                    {"over"},
                    {"many"},
                    {"such"},
                    {"used"},
                    {"city"},
                    {"then"},
                    {"than"},
                    {"made"},
                    {"part"},
                    {"year"},
                    {"both"},
                    {"them"},
                    {"name"},
                    {"area"},
                    {"well"},
                    {"will"},
                    {"high"},
                    {"born"},
                    {"work"},
                    {"town"},
                    {"film"},
                    {"team"},
                    {"each"},
                    {"life"},
                    {"same"},
                    {"game"},
                    {"four"},
                    {"west"},
                    {"line"},
                    {"like"},
                    {"very"},
                    {"john"},
                    {"home"},
                    {"back"},
                    {"band"},
                    {"show"},
                    {"york"},
                    {"even"},
                    {"much"},
                    {"east"},
                    {"the"},      /// l = 3, pos = 146
                    {"and"},
                    {"was"},
                    {"for"},
                    {"his"},
                    {"are"},
                    {"has"},
                    {"had"},
                    {"one"},
                    {"not"},
                    {"but"},
                    {"its"},
                    {"new"},
                    {"who"},
                    {"her"},
                    {"two"},
                    {"she"},
                    {"all"},
                    {"can"},
                    {"may"},
                    {"out"},
                    {"him"},
                    {"war"},
                    {"age"},
                    {"now"},
                    {"use"},
                    {"any"},
                    {"end"},
                    {"day"},
                    {"did"},
                    {"own"},
                    {"due"},
                    {"won"},
                    {"sum"},  // new
                    {"usa"},  // new
                    {"of"},     /// l = 2, pos = 181
                    {"km"},   // new
                    {"mr"},   // new
                    {"us"},   // new
                    {"in"},
                    {"to"},
                    {"is"},
                    {"as"},
                    {"on"},
                    {"by"},
                    {"he"},
                    {"at"},
                    {"it"},
                    {"an"},
                    {"or"},
                    {"be"},
                    {"up"},
                    {"no"},
                    {"so"},
                    {"if"},
                    {"a"},
                    {"i"},   ///
                    {"m"}     // new
        };

    String getRandomWord(int maxLength);

}

#endif
