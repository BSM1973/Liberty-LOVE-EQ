#include <array>
#include <cassert>
#include <cstring>

// The source must contain actual UTF-8 literals, not escaped glyph placeholders.
// Tests execute on Linux and macOS with the same C++17 source encoding.
int main() {
    constexpr std::array<const char*,12> accents = {
        u8"é",u8"è",u8"ê",u8"ë",u8"à",u8"â",
        u8"ù",u8"û",u8"î",u8"ï",u8"ô",u8"ç"
    };
    for(const auto* text:accents) {
        if(std::strlen(text)!=2)return 1;
        if((static_cast<unsigned char>(text[0])&0xe0)!=0xc0)return 2;
        if((static_cast<unsigned char>(text[1])&0xc0)!=0x80)return 3;
    }
    constexpr auto title=u8"BIBLIOTHÈQUE DE PRESETS";
    if(std::strlen(title)<=20)return 4;
    constexpr auto accent=u8"É";
    if(static_cast<unsigned char>(accent[0])!=0xc3)return 5;
    if(static_cast<unsigned char>(accent[1])!=0x89)return 6;
    return 0;
}
