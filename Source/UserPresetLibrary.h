#pragma once
#include <JuceHeader.h>
#include <algorithm>
#include <vector>
#include "PluginProcessor.h"

// Presets are separate disk files, not tied to the DAW session. All file I/O
// is initiated by UI actions, never by the real-time audio callback.
namespace loveeq {
struct UserPreset {
    juce::File file;
    juce::String name, category, tags;
    bool favorite=false;
};
class UserPresetLibrary final {
public:
    static juce::File directory() {
        return juce::File::getSpecialLocation(juce::File::userApplicationDataDirectory)
            .getChildFile("Liberty Music Software").getChildFile("LOVE EQ").getChildFile("User Presets");
    }
    static juce::ValueTree read(const juce::File& file) {
        if (!file.existsAsFile() || file.getFileExtension()!=".loveeqpreset") return {};
        const auto xml=juce::XmlDocument::parse(file);
        if (!xml) return {};
        auto tree=juce::ValueTree::fromXml(*xml);
        if (!tree.isValid() || !tree.hasType("LOVE_EQ_USER_PRESET") ||
            !tree.getChildWithName("LOVE_EQ_STATE").isValid()) return {};
        return tree;
    }
    static std::vector<UserPreset> list() {
        juce::Array<juce::File> files;
        directory().findChildFiles(files,juce::File::findFiles,false,"*.loveeqpreset");
        std::vector<UserPreset> out;
        for (const auto& file:files) {
            const auto tree=read(file);
            if (!tree.isValid()) continue;
            UserPreset p;
            p.file=file;
            p.name=tree.getProperty("name",file.getFileNameWithoutExtension()).toString();
            p.category=tree.getProperty("category","User").toString();
            p.tags=tree.getProperty("tags","").toString();
            p.favorite=(bool)tree.getProperty("favorite",false);
            out.push_back(std::move(p));
        }
        std::sort(out.begin(),out.end(),[](const UserPreset& a,const UserPreset& b){
            return a.name.compareIgnoreCase(b.name)<0;
        });
        return out;
    }
    static juce::File fileFor(const juce::String& name) {
        auto safe=juce::File::createLegalFileName(name.trim());
        if (safe.isEmpty() || safe=="." || safe=="..") safe="User preset";
        return directory().getChildFile(safe+".loveeqpreset");
    }
    static bool save(const juce::File& file, const juce::String& name,
                     const juce::String& category,const juce::String& tags,
                     bool favorite,LoveEQAudioProcessor& processor) {
        if (name.trim().isEmpty() || file.getFileExtension()!=".loveeqpreset")
            return false;
        if (!directory().createDirectory().wasOk())return false;
        juce::ValueTree root("LOVE_EQ_USER_PRESET");
        root.setProperty("version",1,nullptr);
        root.setProperty("name",name.trim(),nullptr);
        root.setProperty("category",category.trim().isEmpty()?"User":category.trim(),nullptr);
        root.setProperty("tags",tags.trim(),nullptr);
        root.setProperty("favorite",favorite,nullptr);
        root.addChild(processor.parameters.copyState(),-1,nullptr);
        const auto xml=root.createXml();
        return xml!=nullptr && file.replaceWithText(xml->toString());
    }
    static bool load(const UserPreset& item,LoveEQAudioProcessor& processor) {
        auto root=read(item.file);
        if (!root.isValid())return false;
        auto saved=root.getChildWithName("LOVE_EQ_STATE");
        if (!saved.isValid())return false;
        const auto language=processor.getUiLanguage();
        const auto favorites=processor.parameters.state.getProperty("factoryFavorites");
        processor.parameters.replaceState(saved.createCopy());
        processor.parameters.state.setProperty("factoryFavorites",favorites,nullptr);
        processor.setUiLanguage(language);
        return true;
    }
    static bool setFavorite(const UserPreset& item,bool favorite) {
        auto root=read(item.file);
        if (!root.isValid())return false;
        root.setProperty("favorite",favorite,nullptr);
        const auto xml=root.createXml();
        return xml!=nullptr && item.file.replaceWithText(xml->toString());
    }
    static bool remove(const UserPreset& item) {
        if (!item.file.isAChildOf(directory()))return false;
        return item.file.deleteFile();
    }
};
} // namespace loveeq
