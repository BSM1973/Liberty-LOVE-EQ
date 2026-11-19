#pragma once
#include <array>
#include <algorithm>
#include <cmath>
#include <cstddef>
#include <string_view>
namespace loveeq {
struct PresetInfo { const char* category; const char* name; };
struct PresetBand {
    float hz=1000.f, gain=0.f, q=1.f, threshold=-24.f, range=-4.f;
    int type=0, channel=0, slope=1;
    bool enabled=false, dynamic=false;
};
inline const std::array<PresetInfo,100>& factoryPresets() {
    static constexpr std::array<PresetInfo,100> presets {{
        {"Vocals","Lead Vocal Clarity"},
        {"Vocals","Warm Vocal Body"},
        {"Vocals","Female Vocal Air"},
        {"Vocals","Male Vocal Depth"},
        {"Vocals","Vocal De-Ess Dynamic"},
        {"Vocals","Intimate Whisper"},
        {"Vocals","Rock Vocal Presence"},
        {"Vocals","R&B Vocal Polish"},
        {"Vocals","Vocal Mid/Side Focus"},
        {"Vocals","Backing Vocal Separation"},
        {"Acoustic Guitar","Steel String Detail"},
        {"Acoustic Guitar","Acoustic Warmth"},
        {"Acoustic Guitar","Fingerstyle Definition"},
        {"Acoustic Guitar","Strummed Guitar Space"},
        {"Acoustic Guitar","Nylon Guitar Smooth"},
        {"Acoustic Guitar","Acoustic Resonance Tame"},
        {"Acoustic Guitar","Bright Acoustic Pick"},
        {"Acoustic Guitar","Natural Acoustic Recording"},
        {"Electric Guitar","Clean Guitar Sparkle"},
        {"Electric Guitar","Crunch Guitar Focus"},
        {"Electric Guitar","High Gain Mud Removal"},
        {"Electric Guitar","Blues Guitar Warmth"},
        {"Electric Guitar","Rhythm Guitar Pocket"},
        {"Electric Guitar","Lead Guitar Forward"},
        {"Electric Guitar","Guitar Amp Fizz Control"},
        {"Electric Guitar","Wide Double Guitars"},
        {"Bass","Electric Bass Fundamental"},
        {"Bass","Bass Pick Definition"},
        {"Bass","Deep Sub Focus"},
        {"Bass","Bass Growl Shape"},
        {"Bass","Bass Low Mid Clean"},
        {"Bass","Slap Bass Punch"},
        {"Bass","Synth Bass Separation"},
        {"Bass","Bass Dynamic Control"},
        {"Kick and Snare","Kick Low End Punch"},
        {"Kick and Snare","Kick Click Definition"},
        {"Kick and Snare","Kick Sub Tight"},
        {"Kick and Snare","Kick Boxiness Cut"},
        {"Kick and Snare","Snare Snap Presence"},
        {"Kick and Snare","Snare Body and Crack"},
        {"Kick and Snare","Snare Ring Control"},
        {"Kick and Snare","Kick Snare Space"},
        {"Drums and Percussion","Drum Bus Tighten"},
        {"Drums and Percussion","Overhead Cymbal Smooth"},
        {"Drums and Percussion","Toms Weight"},
        {"Drums and Percussion","Hi Hat Harshness Cut"},
        {"Drums and Percussion","Percussion Transient Clarity"},
        {"Drums and Percussion","Room Mics Character"},
        {"Drums and Percussion","Full Kit Balance"},
        {"Drums and Percussion","Drum Bus Dynamic Low End"},
        {"Piano and Keyboards","Grand Piano Natural"},
        {"Piano and Keyboards","Grand Piano Sparkle"},
        {"Piano and Keyboards","Warm Upright Piano"},
        {"Piano and Keyboards","Electric Piano Silk"},
        {"Piano and Keyboards","Electric Piano Attack"},
        {"Piano and Keyboards","Organ Midrange Space"},
        {"Piano and Keyboards","Piano Pop Presence"},
        {"Piano and Keyboards","Keyboard Bus Cleanup"},
        {"Synthesizers","Synth Lead Cut Through"},
        {"Synthesizers","Analog Pad Warmth"},
        {"Synthesizers","Wavetable Motion Space"},
        {"Synthesizers","Arpeggio Definition"},
        {"Synthesizers","Ambient Texture Air"},
        {"Synthesizers","Pluck Transient Tone"},
        {"Synthesizers","Synth Bass Low End"},
        {"Synthesizers","Wide Pad Mid Side"},
        {"Orchestral Instruments","String Section Silk"},
        {"Orchestral Instruments","Solo Violin Presence"},
        {"Orchestral Instruments","Cello Woody Resonance"},
        {"Orchestral Instruments","Brass Section Focus"},
        {"Orchestral Instruments","Woodwinds Natural"},
        {"Orchestral Instruments","Orchestra Bus Gentle"},
        {"Dialogue and Podcast","Podcast Speech Clean"},
        {"Dialogue and Podcast","Broadcast Voice Warmth"},
        {"Dialogue and Podcast","Narration Intelligibility"},
        {"Dialogue and Podcast","Dialogue Low Cut"},
        {"Dialogue and Podcast","Dialogue Boxiness Control"},
        {"Dialogue and Podcast","Interview Dynamic Control"},
        {"Dialogue and Podcast","Voice Over Air"},
        {"Dialogue and Podcast","Location Dialogue Rescue"},
        {"Mix Bus","Mix Bus Sub Control"},
        {"Mix Bus","Mix Bus Gentle Smile"},
        {"Mix Bus","Mix Bus Presence"},
        {"Mix Bus","Mix Bus Warmth"},
        {"Mix Bus","Mix Bus Low Mid Clean"},
        {"Mix Bus","Mix Bus Mid Side Glue"},
        {"Mix Bus","Mix Bus Transient Space"},
        {"Mix Bus","Mix Bus Dynamic Balance"},
        {"Mastering","Master Transparent Tilt"},
        {"Mastering","Master Subsonic Cleanup"},
        {"Mastering","Master Low End Definition"},
        {"Mastering","Master Gentle Air"},
        {"Mastering","Master Mid Side Widening"},
        {"Mastering","Master Harshness Control"},
        {"Mastering","Master Dynamic Resonance"},
        {"Mastering","Master Streaming Balance"},
        {"Creative and Sound Design","Telephone Vintage"},
        {"Creative and Sound Design","Lo Fi Radio Narrow"},
        {"Creative and Sound Design","Underwater Darkness"},
        {"Creative and Sound Design","Extreme Resonant Sweep"},
    }};
    return presets;
}
inline std::array<PresetBand,8> makePreset(std::size_t index) {
    const auto& info=factoryPresets()[std::min(index,std::size_t{99})];
    const std::string_view cat=info.category, name=info.name;
    std::array<PresetBand,8> b{};
    auto set=[&](int i,float hz,float gain,float q,int type=0,int channel=0,int slope=1){
        auto& x=b.at(static_cast<std::size_t>(i));
        x.hz=hz; x.gain=gain; x.q=q; x.type=type; x.channel=channel;
        x.slope=slope; x.enabled=true;
    };
    // Every category starts from a deliberate, conservative, musical EQ template.
    if (cat=="Vocals") {
        set(0,75,0,0.7f,3);set(1,230,-2,1.1f);set(2,850,-1,1.2f);
        set(3,3200,2.2f,0.8f);set(4,7000,-1.2f,2.5f);set(5,12000,1.5f,0.7f,2);
    } else if(cat=="Acoustic Guitar") {
        set(0,70,0,0.7f,3);set(1,190,-1.6f,1.1f);set(2,520,-1.8f,1.3f);
        set(3,2800,2.0f,1.0f);set(4,9000,1.1f,0.7f,2);
    } else if(cat=="Electric Guitar") {
        set(0,78,0,0.7f,3);set(1,280,-1.8f,1.2f);set(2,1500,1.8f,0.95f);
        set(3,4200,1.3f,1.1f);set(4,11500,0,0.7f,4);
    } else if(cat=="Bass") {
        set(0,34,0,0.7f,3);set(1,85,2.0f,0.8f);set(2,250,-2.0f,1.2f);
        set(3,1050,1.5f,1.0f);set(4,4500,0,0.7f,4);
    } else if(cat=="Kick and Snare") {
        set(0,32,0,0.7f,3);set(1,75,2.5f,0.9f);set(2,340,-2.0f,1.4f);
        set(3,3200,2.0f,0.9f);set(4,10500,0.9f,0.7f,2);
    } else if(cat=="Drums and Percussion") {
        set(0,30,0,0.7f,3);set(1,90,1.4f,0.85f);set(2,350,-1.3f,1.0f);
        set(3,2900,1.0f,0.9f);set(4,10200,1.4f,0.7f,2);
    } else if(cat=="Piano and Keyboards") {
        set(0,38,0,0.7f,3);set(1,220,-1.1f,1.0f);set(2,850,-0.8f,1.2f);
        set(3,3400,1.1f,0.8f);set(4,11000,1.2f,0.7f,2);
    } else if(cat=="Synthesizers") {
        set(0,32,0,0.7f,3);set(1,150,1.3f,0.9f);set(2,500,-1.4f,1.3f);
        set(3,2600,1.8f,0.9f);set(4,9500,1.5f,0.7f,2);
    } else if(cat=="Orchestral Instruments") {
        set(0,42,0,0.7f,3);set(1,240,-0.8f,1.1f);set(2,1150,-0.8f,1.3f);
        set(3,4100,1.1f,0.9f);set(4,10500,0.9f,0.7f,2);
    } else if(cat=="Dialogue and Podcast") {
        set(0,90,0,0.7f,3);set(1,260,-2.2f,1.1f);set(2,720,-1.5f,1.0f);
        set(3,2800,2.0f,0.8f);set(4,6500,-1.2f,2.0f);set(5,11500,1.0f,0.7f,2);
    } else if(cat=="Mix Bus") {
        set(0,25,0,0.7f,3);set(1,100,0.7f,0.75f,1);set(2,330,-0.7f,0.9f);
        set(3,3000,0.6f,0.85f);set(4,11000,0.6f,0.7f,2);
    } else if(cat=="Mastering") {
        set(0,22,0,0.7f,3);set(1,85,0.45f,0.7f,1);set(2,260,-0.5f,1.0f);
        set(3,3500,0.45f,0.85f);set(4,11500,0.6f,0.7f,2);
    } else {
        set(0,450,0,0.7f,3,0,3);set(1,1100,2.5f,1.1f);set(2,3300,2.0f,1.2f);
        set(3,4500,0,0.7f,4,0,3);
    }
    // Small reproducible per-preset variations: every named starting point has its own curve.
    const float v=static_cast<float>(index%8)-3.5f;
    for (auto& x:b) if(x.enabled && x.type!=3 && x.type!=4) {
        x.hz=std::clamp(x.hz*(1.f+v*0.045f),20.f,19500.f);
        x.gain=std::clamp(x.gain*(1.f+v*0.07f),-18.f,18.f);
    }
    if(name.find("Dynamic")!=std::string_view::npos || name.find("De-Ess")!=std::string_view::npos || name.find("Harshness")!=std::string_view::npos) {
        const int i=(cat=="Vocals" || cat=="Dialogue and Podcast")?4:2;
        auto& x=b[static_cast<std::size_t>(i)];
        x.enabled=true;x.dynamic=true;x.range=-3.f;x.threshold=-28.f;
        if(name.find("De-Ess")!=std::string_view::npos){x.hz=7200.f;x.q=2.8f;x.gain=0.f;x.type=0;}
    }
    if(name.find("Mid Side")!=std::string_view::npos || name.find("Mid/Side")!=std::string_view::npos || name.find("Wide")!=std::string_view::npos) {
        b[3].channel=3;
        b[4].channel=4;
    }
    if(name.find("Telephone")!=std::string_view::npos || name.find("Radio")!=std::string_view::npos) {
        b[0].hz=350.f;b[3].hz=4000.f;
    }
    if(name.find("Underwater")!=std::string_view::npos) {
        b[0].enabled=false;b[1].gain=-8.f;b[2].gain=-6.f;b[3].hz=650.f;
    }
    return b;
}
} // namespace loveeq
