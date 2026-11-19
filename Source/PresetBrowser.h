#pragma once
#include <JuceHeader.h>
#include <array>
#include <vector>
#include <functional>
#include "FactoryPresets.h"
#include "PluginProcessor.h"

// Embedded professional browser: no modal OS windows, no icon-font dependency.
class LoveEQPresetBrowser final : public juce::Component, private juce::ListBoxModel {
public:
    explicit LoveEQPresetBrowser(LoveEQAudioProcessor& p) : processor(p), results("Factory Presets",nullptr) {
        results.setModel(this);
        setOpaque(true);
        search.setTextToShowWhenEmpty("Rechercher un preset ou un tag...",juce::Colours::grey);
        search.onTextChange=[this]{filter();};
        addAndMakeVisible(search);
        category.onChange=[this]{filter();};
        tag.onChange=[this]{filter();};
        addAndMakeVisible(category);
        addAndMakeVisible(tag);
        favoriteOnly.onClick=[this]{filter();};
        addAndMakeVisible(favoriteOnly);
        results.setRowHeight(29);
        results.setOutlineThickness(0);
        results.setColour(juce::ListBox::backgroundColourId,juce::Colour(0xff111a20));
        addAndMakeVisible(results);
        loadButton.onClick=[this]{loadSelected();};
        favButton.onClick=[this]{
            if(selectedPreset>=0) {
                processor.setFavorite((size_t)selectedPreset,!processor.isFavorite((size_t)selectedPreset));
                filter();
                repaint();
            }
        };
        closeButton.onClick=[this]{setVisible(false); if(onClose) onClose();};
        for(auto* b:{&loadButton,&favButton,&closeButton})addAndMakeVisible(*b);
        juce::StringArray unique;
        for(const auto& item:loveeq::factoryPresets())
            unique.addIfNotAlreadyThere(juce::String(item.category));
        categories=unique;
        translate(false);
        filter();
        setVisible(false);
    }
    ~LoveEQPresetBrowser() override {results.setModel(nullptr);}
    std::function<void(int)> onLoad;
    std::function<void()> onClose;
    void translate(bool english) {
        en=english;
        const int prevCategory=category.getSelectedId();
        const int prevTag=tag.getSelectedId();
        category.clear(juce::dontSendNotification);
        category.addItem(en?"All categories":"Toutes les cat\u00e9gories",1);
        for(int i=0;i<categories.size();++i)
            category.addItem(categoryLabel(categories[i]),i+2);
        category.setSelectedId(prevCategory>0?prevCategory:1,juce::dontSendNotification);
        tag.clear(juce::dontSendNotification);
        const juce::StringArray tags {
            en?"All tags":"Tous les tags",
            en?"Dynamic":"Dynamique",
            "Mid/Side",
            en?"Warm":"Chaleur",
            en?"Bright":"Brillance",
            en?"Mastering":"Mastering",
            en?"Creative":"Cr\u00e9atif",
            en?"Percussive":"Percussif",
            en?"Dialogue":"Dialogue"
        };
        tag.addItemList(tags,1);
        tag.setSelectedId(prevTag>0?prevTag:1,juce::dontSendNotification);
        favoriteOnly.setButtonText(en?"Favorites only":"Favoris uniquement");
        loadButton.setButtonText(en?"LOAD PRESET":"CHARGER LE PRESET");
        favButton.setButtonText(en?"FAVORITE ON/OFF":"AJOUTER / RETIRER FAVORI");
        closeButton.setButtonText(en?"CLOSE":"FERMER");
        search.setTextToShowWhenEmpty(en?"Search names, categories and tags...":"Rechercher noms, cat\u00e9gories et tags...",juce::Colours::grey);
        filter();
        repaint();
    }
    void open() {
        filter();
        setVisible(true);
        toFront(true);
        grabKeyboardFocus();
    }
    void selectPreset(int index) {
        selectedPreset=juce::jlimit(0,99,index);
        filter();
    }
    bool isEnglish() const noexcept { return en; }
    void paint(juce::Graphics& g) override {
        g.fillAll(juce::Colour(0xff151d25));
        g.setColour(juce::Colour(0xffdfc28d));
        g.drawRoundedRectangle(getLocalBounds().toFloat().reduced(1.f),9.f,1.6f);
        g.setFont(juce::Font(juce::FontOptions(24.f,juce::Font::bold)));
        g.drawText(en?"FACTORY PRESET BROWSER":"BIBLIOTH\u00c8QUE DE PRESETS",24,15,getWidth()-170,34,juce::Justification::centredLeft);
        g.setColour(juce::Colour(0xff93a5af));
        g.setFont(juce::Font(juce::FontOptions(11.f)));
        g.drawText(en?"100 original presets - search, tags and favorites":"100 presets originaux - recherche, tags et favoris",
                   25,50,getWidth()-50,20,juce::Justification::centredLeft);
        g.setColour(juce::Colour(0xff293640));
        g.drawHorizontalLine(130,16.f,(float)getWidth()-16.f);
        g.drawHorizontalLine(getHeight()-115,16.f,(float)getWidth()-16.f);
        g.setColour(juce::Colour(0xffb6c5cb));
        g.setFont(juce::Font(juce::FontOptions(13.f,juce::Font::bold)));
        g.drawText(en?"SELECTED":"S\u00c9LECTION",25,getHeight()-106,160,22,juce::Justification::centredLeft);
        if(selectedPreset>=0) {
            const auto& info=loveeq::factoryPresets()[(size_t)selectedPreset];
            g.setColour(juce::Colour(0xffe5d6b5));
            g.drawText(juce::String(info.name),25,getHeight()-85,getWidth()-356,22,juce::Justification::centredLeft);
            g.setColour(juce::Colour(0xff93a5af));
            g.setFont(juce::Font(juce::FontOptions(11.f)));
            g.drawText(categoryLabel(info.category)+"   /   "+makeTags(selectedPreset),25,getHeight()-59,getWidth()-356,18,juce::Justification::centredLeft);
        } else {
            g.drawText(en?"No matching preset":"Aucun preset trouv\u00e9",25,getHeight()-81,getWidth()-50,22,juce::Justification::centredLeft);
        }
    }
    void resized() override {
        const int w=getWidth(),h=getHeight();
        closeButton.setBounds(w-105,22,80,30);
        search.setBounds(24,83,w/3,34);
        category.setBounds(36+w/3,83,w/5,34);
        tag.setBounds(44+w/3+w/5,83,w/5-7,34);
        favoriteOnly.setBounds(w-156,86,137,28);
        results.setBounds(24,143,w-48,h-270);
        favButton.setBounds(w-302,h-75,163,30);
        loadButton.setBounds(w-134,h-75,112,30);
    }
private:
    LoveEQAudioProcessor& processor;
    juce::TextEditor search;
    juce::ComboBox category,tag;
    juce::ToggleButton favoriteOnly;
    juce::ListBox results;
    juce::TextButton loadButton {"CHARGER LE PRESET"},favButton {"FAVORI"},closeButton {"FERMER"};
    juce::StringArray categories;
    std::vector<int> matches;
    int selectedPreset=0;
    bool en=false;

    juce::String categoryLabel(const juce::String& raw) const {
        if(en) return raw;
        if(raw=="Vocals")return "Voix";
        if(raw=="Acoustic Guitar")return "Guitare acoustique";
        if(raw=="Electric Guitar")return "Guitare \u00e9lectrique";
        if(raw=="Bass")return "Basse";
        if(raw=="Kick and Snare")return "Grosse caisse et caisse claire";
        if(raw=="Drums and Percussion")return "Batterie et percussions";
        if(raw=="Piano and Keyboards")return "Piano et claviers";
        if(raw=="Synthesizers")return "Synth\u00e9tiseurs";
        if(raw=="Orchestral Instruments")return "Instruments orchestraux";
        if(raw=="Dialogue and Podcast")return "Dialogue et podcast";
        if(raw=="Mix Bus")return "Bus de mixage";
        if(raw=="Mastering")return "Mastering";
        if(raw=="Creative and Sound Design")return "Cr\u00e9ation sonore";
        return raw;
    }
    bool hasTag(int index,int which) const {
        if(which==1)return true;
        const auto info=loveeq::factoryPresets()[(size_t)index];
        const juce::String name(info.name),cat(info.category);
        if(which==2 || which==3) {
            const auto bands=loveeq::makePreset((size_t)index);
            for(const auto& b:bands) {
                if(which==2 && b.enabled && b.dynamic)return true;
                if(which==3 && b.enabled && (b.channel==3 || b.channel==4))return true;
            }
            return name.containsIgnoreCase(which==2?"dynamic":"mid/side");
        }
        if(which==4)return name.containsIgnoreCase("warm")||name.containsIgnoreCase("body")||name.containsIgnoreCase("smooth");
        if(which==5)return name.containsIgnoreCase("bright")||name.containsIgnoreCase("air")||name.containsIgnoreCase("clarity")||name.containsIgnoreCase("sparkle");
        if(which==6)return cat=="Mastering";
        if(which==7)return cat=="Creative and Sound Design"||name.containsIgnoreCase("creative");
        if(which==8)return cat.containsIgnoreCase("Drum")||cat.containsIgnoreCase("Kick");
        if(which==9)return cat=="Dialogue and Podcast";
        return false;
    }
    juce::String makeTags(int index) const {
        juce::StringArray names;
        if(hasTag(index,2)) names.add(en?"Dynamic":"Dynamique");
        if(hasTag(index,3)) names.add("Mid/Side");
        if(hasTag(index,4)) names.add(en?"Warm":"Chaleur");
        if(hasTag(index,5)) names.add(en?"Bright":"Brillance");
        if(hasTag(index,6)) names.add("Mastering");
        if(hasTag(index,7)) names.add(en?"Creative":"Cr\u00e9atif");
        if(hasTag(index,8)) names.add(en?"Percussive":"Percussif");
        if(hasTag(index,9)) names.add("Dialogue");
        if(names.isEmpty()) names.add(en?"Natural":"Naturel");
        return names.joinIntoString("  /  ");
    }
    void filter() {
        juce::StringArray terms;
        terms.addTokens(search.getText().trim().toLowerCase()," ","");
        terms.removeEmptyStrings();
        const int catId=category.getSelectedId()>0?category.getSelectedId():1;
        const int tagId=tag.getSelectedId()>0?tag.getSelectedId():1;
        matches.clear();
        for(int i=0;i<100;++i) {
            const auto& info=loveeq::factoryPresets()[(size_t)i];
            if(catId>1 && juce::String(info.category)!=categories[catId-2])continue;
            if(!hasTag(i,tagId))continue;
            if(favoriteOnly.getToggleState() && !processor.isFavorite((size_t)i))continue;
            const juce::String haystack=(juce::String(info.name)+" "+info.category+" "+categoryLabel(info.category)+" "+makeTags(i)).toLowerCase();
            bool all=true;
            for(auto word:terms) if(!haystack.contains(word)){all=false;break;}
            if(all) matches.push_back(i);
        }
        results.updateContent();
        int found=-1;
        for(size_t n=0;n<matches.size();++n)if(matches[n]==selectedPreset){found=(int)n;break;}
        if(found<0) {
            if(matches.empty()) selectedPreset=-1;
            else {found=0;selectedPreset=matches.front();}
        }
        if(found>=0) results.selectRow(found);
        favButton.setEnabled(selectedPreset>=0);
        loadButton.setEnabled(selectedPreset>=0);
        repaint();
    }
    int getNumRows() override {return (int)matches.size();}
    void paintListBoxItem(int row,juce::Graphics& g,int width,int height,bool selected) override {
        if(row<0||row>=getNumRows())return;
        const int index=matches[(size_t)row];
        if(selected)g.fillAll(juce::Colour(0xff324956));
        else if(row%2==1)g.fillAll(juce::Colour(0xff1b2831));
        const auto& item=loveeq::factoryPresets()[(size_t)index];
        g.setFont(juce::Font(juce::FontOptions(13.f)));
        g.setColour(selected?juce::Colour(0xfff3ddb6):juce::Colour(0xffd0dce1));
        g.drawText(juce::String(index+1).paddedLeft('0',3),10,0,38,height,juce::Justification::centredLeft);
        g.drawText(item.name,54,0,width/2-55,height,juce::Justification::centredLeft);
        g.setColour(juce::Colour(0xff9daeb9));
        g.setFont(juce::Font(juce::FontOptions(10.5f)));
        g.drawText(categoryLabel(item.category),width/2+2,0,width/4,height,juce::Justification::centredLeft);
        g.drawText(makeTags(index),width*3/4,0,width/4-28,height,juce::Justification::centredLeft);
        if(processor.isFavorite((size_t)index)) {
            // Draw an actual geometric star, not a font glyph (works in any locale).
            juce::Path star;
            const float cx=(float)width-15.f,cy=height*0.5f;
            for(int j=0;j<10;++j) {
                const float angle=-juce::MathConstants<float>::halfPi+j*juce::MathConstants<float>::pi/5.f;
                const float radius=j%2==0?9.f:4.f;
                const auto x=cx+radius*std::cos(angle),y=cy+radius*std::sin(angle);
                if(j==0)star.startNewSubPath(x,y);else star.lineTo(x,y);
            }
            star.closeSubPath();g.setColour(juce::Colour(0xffdcb778));g.fillPath(star);
        }
    }
    void selectedRowsChanged(int row) override {
        if(row>=0 && row<getNumRows()) {
            selectedPreset=matches[(size_t)row];
            favButton.setButtonText(processor.isFavorite((size_t)selectedPreset)?
                 (en?"REMOVE FAVORITE":"RETIRER DES FAVORIS"):
                 (en?"ADD FAVORITE":"AJOUTER AUX FAVORIS"));
            repaint();
        }
    }
    void listBoxItemDoubleClicked(int row,const juce::MouseEvent&) override {
        if(row>=0 && row<getNumRows()){selectedPreset=matches[(size_t)row];loadSelected();}
    }
    void loadSelected() {
        if(selectedPreset<0)return;
        processor.loadFactoryPreset((size_t)selectedPreset);
        setVisible(false);
        if(onLoad)onLoad(selectedPreset);
    }
};