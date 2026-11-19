#pragma once
#include <JuceHeader.h>
#include <array>
#include <atomic>
#include "EqCore.h"

class LoveEQAudioProcessor final : public juce::AudioProcessor {
public:
    static constexpr int bandCount = 8;
    static constexpr int scopeSize = 2048;
    LoveEQAudioProcessor();
    ~LoveEQAudioProcessor() override = default;

    void prepareToPlay(double sampleRate, int maximumExpectedSamplesPerBlock) override;
    void releaseResources() override;
    void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }
    const juce::String getName() const override { return "LOVE EQ"; }
    bool acceptsMidi() const override { return false; }
    bool producesMidi() const override { return false; }
    bool isMidiEffect() const override { return false; }
    double getTailLengthSeconds() const override { return 0.0; }
    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram(int) override {}
    const juce::String getProgramName(int) override { return "Factory"; }
    void changeProgramName(int, const juce::String&) override {}
    void getStateInformation(juce::MemoryBlock&) override;
    void setStateInformation(const void*, int) override;
    bool isBusesLayoutSupported(const BusesLayout&) const override;

    juce::UndoManager undoManager;
    juce::AudioProcessorValueTreeState parameters;
    std::atomic<float> peakIn { 0.f }, peakOut { 0.f };
    std::array<std::atomic<float>, scopeSize> scopeBefore {}, scopeAfter {};
    std::atomic<unsigned int> scopeIndex { 0 };
    void loadFactoryPreset(size_t index);
    void resetToFlat();
    static juce::String bandId(int band, const juce::String& suffix);
    static juce::AudioProcessorValueTreeState::ParameterLayout makeParameterLayout();
private:
    struct BandSettings {
        float hz=1000.f, gain=0.f, q=1.f, threshold=-24.f, range=-4.f, attack=10.f, release=160.f;
        double attackCoefficient=0.99, releaseCoefficient=0.999;
        int type=0, channel=0, slope=1;
        bool enabled=true, dynamic=false, solo=false;
    };
    struct BandRuntime {
        std::array<std::array<loveeq::Biquad, 5>, 2> filters {};
        std::array<loveeq::Biquad, 2> detectors {};
        std::array<double,2> envelope {};
        juce::SmoothedValue<double> smoothedHz, smoothedGain, smoothedQ;
        juce::SmoothedValue<float> wet;
        int controlCountdown=0;
        void reset(double sampleRate, const BandSettings&);
        void updateCoefficients(double sampleRate, const BandSettings&);
        float filtered(int channel, float input) noexcept;
    };
    std::array<BandRuntime, bandCount> runtime;
    std::array<BandSettings, bandCount> settings;
    struct BandParameterPointers {
        std::atomic<float> *hz{}, *gain{}, *q{}, *type{}, *channel{}, *slope{},
                           *enabled{}, *solo{}, *dynamic{}, *threshold{}, *range{},
                           *attack{}, *release{};
    };
    std::array<BandParameterPointers, bandCount> bandParameters {};
    std::atomic<float> *inputParam{}, *outputParam{}, *bypassParam{}, *sidechainParam{};
    static float value(std::atomic<float>* p) noexcept { return p ? p->load(std::memory_order_relaxed):0.f; }
    juce::SmoothedValue<float> globalWet, inputGain, outputGain;
    double rate=48000.0;
    void refreshSettings();
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(LoveEQAudioProcessor)
};
