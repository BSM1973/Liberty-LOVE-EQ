#pragma once
#include <JuceHeader.h>
#include <array>
#include <atomic>
#include "EqCore.h"
#include "RelationshipEQ.h"
#include "TimeLensCore.h"
#include "IntentCore.h"

class LoveEQAudioProcessor final : public juce::AudioProcessor {
public:
    static constexpr int bandCount = 8;
    static constexpr int scopeSize = 8192; // Longer analyzer history for Harmonic DNA. RT write count unchanged.
    static constexpr int graphicBandCount = 16;
    static constexpr int timeStageCount = 3;
    static juce::String timeStageId(int stage,const juce::String& param) {
        return "time_lens_" + juce::String(stage+1) + "_" + param;
    }
    std::atomic<int> timeLensPhase {0};
    std::atomic<float> timeLensEnvelope {0.f};
    static constexpr std::array<double,graphicBandCount> graphicHz {
        25.0,40.0,63.0,100.0,160.0,250.0,400.0,630.0,
        1000.0,1600.0,2500.0,4000.0,6300.0,10000.0,16000.0,20000.0
    };
    static juce::String graphicId(int index) { return "graphic_gain_" + juce::String(index+1); }
    LoveEQAudioProcessor();
    ~LoveEQAudioProcessor() override;

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
    loveeq::relationship::Frame relationshipOwnFrame() const noexcept {
        return loveeq::relationship::registry().snapshot(relationSlot);
    }
    loveeq::relationship::Frame relationshipPriorityFrame() const noexcept {
        return loveeq::relationship::registry().aggregate(
            (int)value(relationshipGroupParam),(int)value(relationshipPriorityParam)+1,relationSlot);
    }
    int relationshipInstanceId() const noexcept { return relationSlot; }
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
    // In-process cross-instance spectral communication, backed by fixed atomic slots.
    int relationSlot=-1;
    loveeq::relationship::Meter relationMeter;
    std::array<float,loveeq::relationship::bandCount> relationPreviousLevels {};
    std::atomic<float> *relationshipRoleParam{},*relationshipGroupParam{},
                       *relationshipPriorityParam{},*relationshipActiveParam{},
                       *relationshipHzParam{},*relationshipQParam{},*relationshipMaxCutParam{},*relationshipGuardHzParam{};
    std::array<loveeq::Biquad,2> relationshipFilters {};
    double relationshipGainDb=0.0,relationshipAttackCoeff=0.0,relationshipReleaseCoeff=0.0;
    int relationshipCoeffCountdown=0;
    struct TimeStageRuntime {
        std::array<loveeq::Biquad,2> channels {};
        juce::SmoothedValue<double> hz,gain,q;
        juce::SmoothedValue<float> weight;
        int coeffCountdown=0;
        void reset(double sr,float initHz,float initGain,float initQ) {
            for(auto& c:channels)c.reset();
            hz.reset(sr,0.020);gain.reset(sr,0.020);q.reset(sr,0.020);
            hz.setCurrentAndTargetValue(initHz);
            gain.setCurrentAndTargetValue(initGain);
            q.setCurrentAndTargetValue(initQ);
            weight.reset(sr,0.012);weight.setCurrentAndTargetValue(0.f);
            coeffCountdown=0;
        }
    };
    struct TimeStageParams {
        std::atomic<float>* hz{};
        std::atomic<float>* gain{};
        std::atomic<float>* q{};
    };
    loveeq::timelens::Detector timeDetector;
    std::array<TimeStageRuntime,timeStageCount> timeRuntime {};
    std::array<TimeStageParams,timeStageCount> timeParams {};
    std::atomic<float>* timeEnabledParam{},*timeAttackParam{},
                      *timeTailParam{},*timeThresholdParam{};
    juce::SmoothedValue<float> timeWet;
    // LOVE INTENT uses three independent non-destructive, automatable Bell bands.
    struct IntentBand {
        std::array<loveeq::Biquad,2> channels {};
        juce::SmoothedValue<double> frequency,gain,q;
        int countdown=0;
        void prepare(double sr,float hz,float db,float width) {
            for(auto& c:channels)c.reset();
            frequency.reset(sr,0.02);gain.reset(sr,0.02);q.reset(sr,0.02);
            frequency.setCurrentAndTargetValue(hz);
            gain.setCurrentAndTargetValue(db);
            q.setCurrentAndTargetValue(width);
            countdown=0;
        }
    };
    std::array<IntentBand,loveeq::intent::bandCount> intentRuntime {};
    struct IntentParameters {
        std::atomic<float>* hz{};std::atomic<float>* gain{};std::atomic<float>* q{};
    };
    std::array<IntentParameters,loveeq::intent::bandCount> intentParams {};
    std::atomic<float>* intentActiveParam{},*intentAmountParam{};
    juce::SmoothedValue<float> intentWet,intentDose;
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
