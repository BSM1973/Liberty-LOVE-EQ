#include "Utf8Text.h"
#include <array>
#include <cstring>
#include <iostream>

int main() {
    // Must test what *JUCE actually decodes*, not just whether .cpp files
    // contain UTF-8 bytes. This would have caught PARAMÃ‰TRIQUE.
    constexpr std::array<const char*,14> labels {
        u8"PARAMÉTRIQUE",u8"FRÉQUENCE",u8"RÉPONSE - dB",
        u8"ÉCHELLE",u8"RELÂCHE",u8"ATTÉNUER",
        u8"RÉGLAGES PARAMÉTRIQUES",u8"ENTRÉE",
        u8"PRÊT POUR LE MIXAGE",u8"BIBLIOTHÈQUE DE PRESETS",
        u8"Catégorie utilisateur",u8"Sélectionner la bande",
        u8"CAPTURER RÉF",u8"ÉQUILIBRÉ"
    };
    for(const auto* label:labels) {
        const juce::String decoded=loveeq::i18n::utf8(label);
        if(std::strcmp(decoded.toRawUTF8(),label)!=0 ||
           decoded.contains(juce::String::fromUTF8(u8"Ã")) ||
           decoded.containsChar((juce_wchar)0xfffd)) {
            std::cerr<<"JUCE incorrectly decoded a French UI label\n";
            return 1;
        }
        if(decoded.isEmpty())return 2;
    }
    const auto french=loveeq::i18n::translate(false,u8"FRÉQUENCE","FREQUENCY");
    const auto english=loveeq::i18n::translate(true,u8"FRÉQUENCE","FREQUENCY");
    if(std::strcmp(french.toRawUTF8(),u8"FRÉQUENCE")!=0 ||
       std::strcmp(english.toRawUTF8(),"FREQUENCY")!=0)return 3;
    std::cout<<"JUCE native French text decoding passed (14 real labels, FR and EN)\n";
    return 0;
}
