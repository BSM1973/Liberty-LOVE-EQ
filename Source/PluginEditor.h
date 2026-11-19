#pragma once
#include <JuceHeader.h>
#include "PluginProcessor.h"

class LoveEQAudioProcessorEditor final : public juce::AudioProcessorEditor, private juce::Timer {
public:
    explicit LoveEQAudioProcessorEditor(LoveEQAudioProcessor&);
    ~LoveEQAudioProcessorEditor() override;
    void paint(juce::Graphics&) override;
    void resized() override;
    void mouseDown(const juce::MouseEvent&) override;
    void mouseDrag(const juce::MouseEvent&) override;
private:
    using SliderAttachment=juce::AudioProcessorValueTreeState::SliderAttachment;
    using ComboAttachment=juce::AudioProcessorValueTreeState::ComboBoxAttachment;
    using ButtonAttachment=juce::AudioProcessorValueTreeState::ButtonAttachment;
    class Theme final : public juce::LookAndFeel_V4 {
    public:
        Theme();
    } theme;
    LoveEQAudioProcessor& processor;
    std::array<juce::TextButton,8> bandButtons;
    juce::TextEditor presetSearch;
    juce::ComboBox presetCombo, typeBox, channelBox, slopeBox;
    juce::TextButton buttonA {"A"}, buttonB {"B"}, undoButton {"ANNULER"}, redoButton {"RÉTABLIR"},
                     resetButton {"RESET / FLAT"}, bypassButton {"BYPASS"},
                     preButton {"PRE"}, postButton {"POST"};
    juce::ToggleButton enabledButton {"ACTIF"}, soloButton {"SOLO"}, dynamicButton {"DYNAMIQUE"};
    juce::Slider frequency, gain, q, threshold, range, attack, release, input, output;
    std::unique_ptr<SliderAttachment> hzAttach, gainAttach, qAttach, thresholdAttach, rangeAttach,
                                       attackAttach, releaseAttach, inputAttach, outputAttach;
    std::unique_ptr<ComboAttachment> typeAttach, channelAttach, slopeAttach;
    std::unique_ptr<ButtonAttachment> enabledAttach, soloAttach, dynamicAttach, bypassAttach;
    juce::ValueTree stateA, stateB;
    bool onB=false, showPre=true, showPost=true;
    int selectedBand=0, draggingBand=-1;
    juce::Rectangle<int> graph, meterBox, bandRow, controls;
    juce::dsp::FFT fft {11};
    std::array<float,4096> fftScratch{};
    std::array<float,1025> preSpectrum{}, postSpectrum{};
    void timerCallback() override;
    void attachBand(int);
    void refillPresets();
    void switchSnapshot(bool toB);
    void setParam(const juce::String&,float);
    void updateFFT(bool before,std::array<float,1025>&);
    void drawSpectrum(juce::Graphics&,const std::array<float,1025>&,juce::Colour);
    float xForHz(float hz) const;
    float yForDb(float db) const;
    float dbAtHz(float hz) const;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(LoveEQAudioProcessorEditor)
};
