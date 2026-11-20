#pragma once
#include <juce_core/juce_core.h>

// Mandatory C++17 text boundary for every Liberty plugin built with JUCE.
// In C++17 a u8"..." literal is still a const char*; JUCE's implicit
// String(const char*) constructor interprets it as ASCII rather than UTF-8.
// ALL French text must cross this boundary explicitly to preserve accents.
namespace loveeq::i18n {
inline juce::String utf8(const char* encodedText) {
    return juce::String::fromUTF8(encodedText);
}
inline juce::String translate(bool english,const char* french,const char* englishText) {
    return utf8(english?englishText:french);
}
}
