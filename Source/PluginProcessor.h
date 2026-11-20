#pragma once
#include <JuceHeader.h>
#include <array>
#include <atomic>
#include "EqCore.h"

class LoveEQAudioProcessor final : public juce::AudioProcessor {
public:
    static constexpr int bandCount = 8;
    static constexpr int scopeSize = 8192; // Longer analyzer history for Harmonic DNA. RT write count unchanged.
    static constexpr int graphicBandCount = 16;
    static constexpr std::array<double,graphicBandCount> graphicHz {
        25.0,40.0,63.0,100.0,160.0,250.0,400.0,630.0,
        1000.0,1600.0,2500.0,4000.0,6300.0,10000.0,16000.0,20000.0
    };
    static juce::String graphicId(int index) { return "graphic_gain_" + juce::String(index+1); }
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
    std::atomic<float> peakHoldIn { 0.f }, peakHoldOut { 0.f };
    void resetPeakHold() noexcept { peakHoldIn.store(0.f); peakHoldOut.store(0.f); }
    bool isFavorite(size_t index) const;
    void setFavorite(size_t index,bool favorite);
    juce::String getUiLanguage() const;
    void setUiLanguage(const juce::String& language);
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
    struct GraphicRuntime {
        std::array<loveeq::Biquad,2> channels {};
        juce::SmoothedValue<float> gain;
        int countdown=0;
        void reset(double sr) {
            for(auto& f:channels) f.reset();
            gain.reset(sr,0.025);
            gain.setCurrentAndTargetValue(0.f);
            countdown=0;
        }
    };
    std::array<GraphicRuntime,graphicBandCount> graphicRuntime {};
    std::array<std::atomic<float>*,graphicBandCount> graphicParams {};
    std::atomic<float>* eqModeParam {};
    juce::SmoothedValue<float> graphicBlend;
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
