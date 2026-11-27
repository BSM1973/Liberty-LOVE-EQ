#pragma once
#include <JuceHeader.h>
#include <algorithm>
#include <functional>
#include <vector>
#include <cmath>
#include "FactoryPresets.h"
#include "UserPresetLibrary.h"

// Custom in-plugin preset library with FACTORY and USER tabs.
// No modal OS file dialogs; works in AU, VST3 and standalone.
class LoveEQPresetBrowser final : public juce::Component, private juce::ListBoxModel {
public:
    explicit LoveEQPresetBrowser(LoveEQAudioProcessor& p)
        : processor(p), results("LOVE EQ Presets",nullptr) {
        setOpaque(true);
        results.setModel(this);
#if JUCE_IOS
        results.setRowHeight(44);
#else
        results.setRowHeight(30);
#endif
        results.setColour(juce::ListBox::backgroundColourId,juce::Colour(0xff121b24));
        results.setOutlineThickness(0);
        addAndMakeVisible(results);

        auto show=[this](juce::Component& c){addAndMakeVisible(c);};
        for(juce::Component* c : std::array<juce::Component*,15>{
             &factoryTab,&userTab,&search,&category,&tag,&favoriteOnly,
             &presetName,&presetCategory,&presetTags,&saveButton,&loadButton,
             &favoriteButton,&deleteButton,&refreshButton,&closeButton}) show(*c);

        factoryTab.onClick=[this]{setTab(false);};
        userTab.onClick=[this]{setTab(true);};
        search.onTextChange=[this]{filter();};
        category.onChange=[this]{filter();};
        tag.onChange=[this]{filter();};
        favoriteOnly.onClick=[this]{filter();};
        saveButton.onClick=[this]{saveUser();};
        loadButton.onClick=[this]{loadSelected();};
        favoriteButton.onClick=[this]{toggleFavorite();};
        deleteButton.onClick=[this]{deleteSelected();};
        refreshButton.onClick=[this]{reloadUser();};
        closeButton.onClick=[this]{setVisible(false);if(onClose)onClose();};

        userPresets=loveeq::UserPresetLibrary::list();
        juce::StringArray factoryCategories;
        for(const auto& item:loveeq::factoryPresets())
            factoryCategories.addIfNotAlreadyThere(juce::String(item.category));
        defaultCategories=factoryCategories;
        translate(false);
        setTab(false);
        setVisible(false);
    }
    ~LoveEQPresetBrowser() override {results.setModel(nullptr);}
    std::function<void(int)> onLoad;
    std::function<void()> onClose;

#if JUCE_IOS
    void setMobileCompact(bool compact) {mobileCompact=compact;resized();repaint();results.repaint();}
#endif
    void open() {
        reloadUser();
        setVisible(true);
        toFront(true);
#if !JUCE_IOS
        search.grabKeyboardFocus();
#endif
    }
    void translate(bool isEnglish) {
        en=isEnglish;
        factoryTab.setButtonText(en?"FACTORY":"USINE");
        userTab.setButtonText("USER");
        refreshButton.setButtonText(en?"REFRESH":"ACTUALISER");
        closeButton.setButtonText(en?"CLOSE":"FERMER");
        loadButton.setButtonText(en?"LOAD":"CHARGER");
        saveButton.setButtonText(en?"SAVE AS USER":"SAUVER DANS USER");
        favoriteButton.setButtonText(en?"FAVORITE":"FAVORI");
        deleteButton.setButtonText(en?"DELETE":"SUPPRIMER");
        favoriteOnly.setButtonText(en?"Favorites":"Favoris");
        search.setTextToShowWhenEmpty(en?"Search presets, tags...":"Rechercher presets, tags...",juce::Colours::grey);
        presetName.setTextToShowWhenEmpty(en?"Preset name":"Nom du preset",juce::Colours::grey);
        presetCategory.setTextToShowWhenEmpty(en?"User category":juce::String::fromUTF8(u8"Catégorie utilisateur"),juce::Colours::grey);
        presetTags.setTextToShowWhenEmpty(en?"Tags separated by commas":juce::String::fromUTF8(u8"Tags séparés par des virgules"),juce::Colours::grey);
        rebuildFilters();
        filter();
    }
    void paint(juce::Graphics& g) override {
        g.fillAll(juce::Colour(0xff151d26));
#if JUCE_IOS
        if(mobileCompact){
            g.setColour(juce::Colour(0xffe2c693));g.setFont(juce::Font(juce::FontOptions(14.f)));
            g.drawFittedText(status,getLocalBounds().removeFromBottom(30).reduced(5,1),juce::Justification::centredLeft,2);
            return;
        }
#endif
        auto area=getLocalBounds().toFloat().reduced(0.8f);
        g.setColour(juce::Colour(0xffc5a56d));
        g.drawRoundedRectangle(area,10.f,1.3f);
        g.setColour(juce::Colour(0xfff2e4c9));
        g.setFont(juce::Font(juce::FontOptions("Arial",23.f,juce::Font::bold)));
        g.drawText(en?"LOVE EQ - PRESET LIBRARY":juce::String::fromUTF8(u8"LOVE EQ - BIBLIOTHÈQUE DE PRESETS"),
                   22,13,getWidth()-160,37,juce::Justification::centredLeft);
        g.setColour(juce::Colour(0xff95a6b0));
        g.setFont(juce::Font(juce::FontOptions("Arial",11.f,juce::Font::plain)));
        g.drawText(en?"100 factory presets and your personal library":juce::String::fromUTF8(u8"100 presets d'usine et votre bibliothèque personnelle"),
                   24,50,getWidth()-48,20,juce::Justification::centredLeft);
        g.setColour(juce::Colour(0xff3b4752));
        g.drawHorizontalLine(157,22.f,(float)getWidth()-22.f);
        g.drawHorizontalLine(getHeight()-183,22.f,(float)getWidth()-22.f);

        const int lower=getHeight()-179;
        g.setColour(juce::Colour(0xffe2c693));
        g.setFont(juce::Font(juce::FontOptions("Arial",11.5f,juce::Font::bold)));
        g.drawText(en?"SAVE CURRENT SETTINGS AS A USER PRESET":juce::String::fromUTF8(u8"SAUVEGARDER LES RÉGLAGES ACTUELS DANS USER"),
                   25,lower,550,21,juce::Justification::centredLeft);
        g.setColour(juce::Colour(0xff93a6b1));
        g.setFont(juce::Font(juce::FontOptions("Arial",10.2f,juce::Font::plain)));
        g.drawText(en?"NAME":"NOM",25,lower+26,245,14,juce::Justification::centredLeft);
        g.drawText(en?"CATEGORY":juce::String::fromUTF8(u8"CATÉGORIE"),getWidth()/2-109,lower+26,204,14,juce::Justification::centredLeft);
        g.drawText("TAGS",getWidth()-258,lower+26,205,14,juce::Justification::centredLeft);
        g.setColour(juce::Colour(0xff93a6b1));
        g.drawText(status,24,getHeight()-38,getWidth()-48,21,juce::Justification::centredLeft);
    }
    void resized() override {
#if JUCE_IOS
        if(mobileCompact){
            auto a=getLocalBounds().reduced(4);
            auto row=[&](std::initializer_list<juce::Component*> cs){
                auto r=a.removeFromTop(44);const int count=(int)cs.size();int i=0;
                for(auto* c:cs){const int x=r.getWidth()*i/count,n=r.getWidth()*(++i)/count;c->setBounds(r.getX()+x,r.getY(),n-x-2,r.getHeight());}
                a.removeFromTop(3);
            };
            row({&factoryTab,&userTab,&closeButton});row({&search,&favoriteOnly});
            row({&category,&tag});
            a.removeFromBottom(30);auto bottom=a.removeFromBottom(44);
            loadButton.setBounds(bottom.removeFromLeft(bottom.getWidth()/2).reduced(1));
            favoriteButton.setBounds(bottom.reduced(1));results.setBounds(a.reduced(1));
            for(auto* c:std::initializer_list<juce::Component*>{&presetName,&presetCategory,&presetTags,&saveButton,&deleteButton,&refreshButton})c->setBounds(juce::Rectangle<int>{});
            return;
        }
#endif
        const int w=getWidth(),h=getHeight();
        closeButton.setBounds(w-106,20,81,30);
        factoryTab.setBounds(24,83,94,32);
        userTab.setBounds(125,83,94,32);
        search.setBounds(231,83,w-498,32);
        refreshButton.setBounds(w-256,83,105,32);
        favoriteOnly.setBounds(w-143,85,118,29);
        category.setBounds(24,123,238,30);
        tag.setBounds(270,123,w-294,30);
        results.setBounds(23,167,w-46,h-355);

        const int lower=h-179;
        presetName.setBounds(24,lower+42,w/2-139,31);
        presetCategory.setBounds(w/2-110,lower+42,w/4+37,31);
        presetTags.setBounds(w*3/4-65,lower+42,w/4+40,31);
        loadButton.setBounds(24,h-90,111,31);
        saveButton.setBounds(142,h-90,180,31);
        favoriteButton.setBounds(329,h-90,122,31);
        deleteButton.setBounds(458,h-90,127,31);
    }
private:
    LoveEQAudioProcessor& processor;
    bool en=false, userTabActive=false;
#if JUCE_IOS
    bool mobileCompact=false;
#endif
    std::vector<loveeq::UserPreset> userPresets;
    std::vector<int> matches;
    juce::StringArray defaultCategories, activeCategories;
    int selectedIndex=-1;
    juce::File awaitingOverwrite, awaitingDelete;
    juce::String status;

    juce::TextButton factoryTab{"USINE"},userTab{"USER"},saveButton{"SAUVER DANS USER"},
                     loadButton{"CHARGER"},favoriteButton{"FAVORI"},deleteButton{"SUPPRIMER"},
                     refreshButton{"ACTUALISER"},closeButton{"FERMER"};
    juce::TextEditor search,presetName,presetCategory,presetTags;
    juce::ComboBox category,tag;
    juce::ToggleButton favoriteOnly;
    juce::ListBox results;

    juce::String categoryLabel(const juce::String& raw) const {
        if(en)return raw;
        if(raw=="Vocals")return "Voix";
        if(raw=="Acoustic Guitar")return "Guitare acoustique";
        if(raw=="Electric Guitar")return juce::String::fromUTF8(u8"Guitare électrique");
        if(raw=="Bass")return "Basse";
        if(raw=="Kick and Snare")return "Grosse caisse et caisse claire";
        if(raw=="Drums and Percussion")return "Batterie et percussions";
        if(raw=="Piano and Keyboards")return "Piano et claviers";
        if(raw=="Synthesizers")return juce::String::fromUTF8(u8"Synthétiseurs");
        if(raw=="Orchestral Instruments")return "Instruments orchestraux";
        if(raw=="Dialogue and Podcast")return "Dialogue et podcast";
        if(raw=="Mix Bus")return "Bus de mixage";
        if(raw=="Creative and Sound Design")return juce::String::fromUTF8(u8"Création sonore");
        return raw;
    }
    void reloadUser() {
        const auto oldName=selectedIndex>=0 && userTabActive && selectedIndex<(int)userPresets.size()
                         ?userPresets[(size_t)selectedIndex].name:juce::String();
        userPresets=loveeq::UserPresetLibrary::list();
        if(userTabActive && oldName.isNotEmpty()) {
            selectedIndex=-1;
            for(int i=0;i<(int)userPresets.size();++i)
                if(userPresets[(size_t)i].name==oldName){selectedIndex=i;break;}
        }
        rebuildFilters();
        filter();
    }
    void setTab(bool user) {
        userTabActive=user;
        awaitingDelete=juce::File();
        awaitingOverwrite=juce::File();
        factoryTab.setToggleState(!user,juce::dontSendNotification);
        userTab.setToggleState(user,juce::dontSendNotification);
        selectedIndex=-1;
        if(user) userPresets=loveeq::UserPresetLibrary::list();
        category.setSelectedId(1,juce::dontSendNotification);
        tag.setSelectedId(1,juce::dontSendNotification);
        rebuildFilters();
        filter();
        status=user?
            (en?"USER presets are stored on disk and survive DAW restarts.":juce::String::fromUTF8(u8"Les presets USER sont enregistrés sur disque et conservés après fermeture du DAW.")):
            (en?"Choose a factory preset or save your current settings to USER.":juce::String::fromUTF8(u8"Choisissez un preset d'usine ou sauvegardez vos réglages dans USER."));
        repaint();
    }
    void rebuildFilters() {
        const auto selectedCat=category.getText();
        const int selectedTag=tag.getSelectedId();
        activeCategories.clear();
        if(userTabActive) {
            for(const auto& item:userPresets)
                activeCategories.addIfNotAlreadyThere(item.category);
        } else activeCategories=defaultCategories;
        category.clear(juce::dontSendNotification);
        category.addItem(en?"All categories":juce::String::fromUTF8(u8"Toutes les catégories"),1);
        for(int i=0;i<activeCategories.size();++i)
            category.addItem(categoryLabel(activeCategories[i]),i+2);
        int match=1;
        for(int i=0;i<activeCategories.size();++i)
            if(categoryLabel(activeCategories[i])==selectedCat)match=i+2;
        category.setSelectedId(match,juce::dontSendNotification);
        tag.clear(juce::dontSendNotification);
        const juce::StringArray values {
            en?"All tags":"Tous les tags",en?"Dynamic":"Dynamique",
            "Mid/Side",en?"Warm":"Chaleur",en?"Bright":"Brillance",
            "Mastering",en?"Creative":juce::String::fromUTF8(u8"Créatif"),en?"Percussive":"Percussif","Dialogue"
        };
        tag.addItemList(values,1);
        tag.setSelectedId(juce::jlimit(1,values.size(),selectedTag>0?selectedTag:1),juce::dontSendNotification);
    }
    bool factoryHasTag(int index,int which) const {
        if(which<=1)return true;
        const auto& info=loveeq::factoryPresets()[(size_t)index];
        const juce::String name(info.name),cat(info.category);
        if(which==2 || which==3) {
            const auto bands=loveeq::makePreset((size_t)index);
            for(const auto& b:bands) {
                if(which==2 && b.enabled && b.dynamic)return true;
                if(which==3 && b.enabled && (b.channel==3||b.channel==4))return true;
            }
            return false;
        }
        if(which==4)return name.containsIgnoreCase("warm")||name.containsIgnoreCase("body")||name.containsIgnoreCase("smooth");
        if(which==5)return name.containsIgnoreCase("bright")||name.containsIgnoreCase("air")||name.containsIgnoreCase("clarity");
        if(which==6)return cat=="Mastering";
        if(which==7)return cat=="Creative and Sound Design";
        if(which==8)return cat.containsIgnoreCase("Drum")||cat.containsIgnoreCase("Kick");
        if(which==9)return cat=="Dialogue and Podcast";
        return false;
    }
    bool userHasTag(const loveeq::UserPreset& item,int which) const {
        if(which<=1)return true;
        const juce::StringArray tags {
            "","", "dynamic", "mid/side", "warm", "bright", "mastering",
            "creative", "percussive", "dialogue"
        };
        if(item.tags.containsIgnoreCase(tags[which]))return true;
        if(which==2 && item.tags.containsIgnoreCase("dynamique"))return true;
        if(which==4 && item.tags.containsIgnoreCase("chaleur"))return true;
        if(which==5 && item.tags.containsIgnoreCase("brillance"))return true;
        if(which==7 && item.tags.containsIgnoreCase(juce::String::fromUTF8(u8"créatif")))return true;
        return false;
    }
    juce::String rowName(int index) const {
        return userTabActive?userPresets[(size_t)index].name:
               juce::String(loveeq::factoryPresets()[(size_t)index].name);
    }
    juce::String rowCategory(int index) const {
        return userTabActive?userPresets[(size_t)index].category:
               juce::String(loveeq::factoryPresets()[(size_t)index].category);
    }
    juce::String rowTags(int index) const {
        if(userTabActive)return userPresets[(size_t)index].tags;
        juce::StringArray out;
        if(factoryHasTag(index,2))out.add(en?"Dynamic":"Dynamique");
        if(factoryHasTag(index,3))out.add("Mid/Side");
        if(factoryHasTag(index,4))out.add(en?"Warm":"Chaleur");
        if(factoryHasTag(index,5))out.add(en?"Bright":"Brillance");
        if(factoryHasTag(index,6))out.add("Mastering");
        if(factoryHasTag(index,7))out.add(en?"Creative":juce::String::fromUTF8(u8"Créatif"));
        if(factoryHasTag(index,8))out.add(en?"Percussive":"Percussif");
        if(factoryHasTag(index,9))out.add("Dialogue");
        return out.joinIntoString(", ");
    }
    bool rowFavorite(int index) const {
        return userTabActive?userPresets[(size_t)index].favorite:processor.isFavorite((size_t)index);
    }
    void filter() {
        matches.clear();
        juce::StringArray terms;
        terms.addTokens(search.getText().trim().toLowerCase()," ","");
        terms.removeEmptyStrings();
        const int n=userTabActive?(int)userPresets.size():100;
        const int categoryId=juce::jmax(1,category.getSelectedId());
        const int tagId=juce::jmax(1,tag.getSelectedId());
        for(int i=0;i<n;++i) {
            if(categoryId>1 && rowCategory(i)!=activeCategories[categoryId-2])continue;
            if(tagId>1 && !(userTabActive?userHasTag(userPresets[(size_t)i],tagId):factoryHasTag(i,tagId)))continue;
            if(favoriteOnly.getToggleState() && !rowFavorite(i))continue;
            const auto haystack=(rowName(i)+" "+rowCategory(i)+" "+categoryLabel(rowCategory(i))+" "+rowTags(i)).toLowerCase();
            bool found=true;
            for(auto term:terms) if(!haystack.contains(term)){found=false;break;}
            if(found)matches.push_back(i);
        }
        results.updateContent();
        int selectedRow=-1;
        for(int i=0;i<(int)matches.size();++i)if(matches[(size_t)i]==selectedIndex){selectedRow=i;break;}
        if(selectedRow<0) {
            if(matches.empty())selectedIndex=-1;
            else {selectedRow=0;selectedIndex=matches.front();}
        }
        if(selectedRow>=0)results.selectRow(selectedRow);
        loadButton.setEnabled(selectedIndex>=0);
        favoriteButton.setEnabled(selectedIndex>=0);
        deleteButton.setEnabled(userTabActive && selectedIndex>=0);
        repaint();
    }
    int getNumRows() override {return (int)matches.size();}
    void paintListBoxItem(int row,juce::Graphics& g,int width,int height,bool selected) override {
        if(row<0||row>=getNumRows())return;
        const int index=matches[(size_t)row];
        if(selected)g.fillAll(juce::Colour(0xff354756));
        else if(row%2==1)g.fillAll(juce::Colour(0xff1d2933));
        g.setColour(selected?juce::Colour(0xfff0dec1):juce::Colour(0xffd6e0e5));
        g.setFont(juce::Font(juce::FontOptions("Arial",12.2f,juce::Font::bold)));
#if JUCE_IOS
        if(mobileCompact){
            g.setFont(juce::Font(juce::FontOptions(15.f,juce::Font::bold)));
            g.drawFittedText(rowName(index),8,1,width-36,height/2,juce::Justification::centredLeft,1);
            g.setFont(juce::Font(juce::FontOptions(12.f)));
            g.setColour(juce::Colour(0xff9bafbd));
            g.drawFittedText(categoryLabel(rowCategory(index)),8,height/2,width-36,height/2,juce::Justification::centredLeft,1);
            if(rowFavorite(index)){g.setColour(juce::Colour(0xffe5c184));g.drawText(juce::String::fromUTF8(u8"★"),width-30,0,26,height,juce::Justification::centred);}
            return;
        }
#endif
        g.drawText(userTabActive?"USR":juce::String(index+1).paddedLeft('0',3),
                   12,0,43,height,juce::Justification::centredLeft);
        g.drawText(rowName(index),58,0,width/2-55,height,juce::Justification::centredLeft);
        g.setColour(juce::Colour(0xffa4b5bf));
        g.setFont(juce::Font(juce::FontOptions("Arial",10.8f,juce::Font::plain)));
        g.drawText(categoryLabel(rowCategory(index)),width/2+7,0,width/4-8,height,juce::Justification::centredLeft);
        g.drawText(rowTags(index),width*3/4+1,0,width/4-34,height,juce::Justification::centredLeft);
        if(rowFavorite(index)) {
            juce::Path star;
            const float cx=(float)width-16.f,cy=(float)height/2.f;
            for(int j=0;j<10;++j) {
                const float angle=-juce::MathConstants<float>::halfPi+
                                  (float)j*juce::MathConstants<float>::pi/5.f;
                const float radius=(j%2)==0?9.f:4.f;
                if(j==0)star.startNewSubPath(cx+radius*std::cos(angle),cy+radius*std::sin(angle));
                else star.lineTo(cx+radius*std::cos(angle),cy+radius*std::sin(angle));
            }
            star.closeSubPath();
            g.setColour(juce::Colour(0xffe5c184));
            g.fillPath(star);
        }
    }
    void selectedRowsChanged(int row) override {
        if(row<0||row>=getNumRows())return;
        selectedIndex=matches[(size_t)row];
        awaitingDelete=juce::File();
        awaitingOverwrite=juce::File();
        const juce::String name=rowName(selectedIndex);
        presetName.setText(userTabActive?name:("Mon "+name),false);
        presetCategory.setText(userTabActive?rowCategory(selectedIndex):categoryLabel(rowCategory(selectedIndex)),false);
        presetTags.setText(rowTags(selectedIndex),false);
        deleteButton.setButtonText(en?"DELETE":"SUPPRIMER");
        favoriteButton.setButtonText(rowFavorite(selectedIndex)?
            (en?"UNFAVORITE":"RETIRER FAVORI"):(en?"FAVORITE":"FAVORI"));
        repaint();
    }
    void listBoxItemDoubleClicked(int row,const juce::MouseEvent&) override {
        if(row>=0 && row<getNumRows()) {selectedIndex=matches[(size_t)row];loadSelected();}
    }
    void loadSelected() {
        if(selectedIndex<0)return;
        bool ok=true;
        if(userTabActive)ok=loveeq::UserPresetLibrary::load(userPresets[(size_t)selectedIndex],processor);
        else processor.loadFactoryPreset((size_t)selectedIndex);
        if(!ok) {
            status=en?"Unable to open this user preset.":"Impossible de charger ce preset utilisateur.";
            repaint();return;
        }
        setVisible(false);
        if(onLoad)onLoad(userTabActive?-1:selectedIndex);
    }
    void saveUser() {
        const auto name=presetName.getText().trim();
        if(name.isEmpty()) {
            status=en?"Enter a preset name.":"Saisissez un nom de preset.";
            repaint();return;
        }
        const auto file=loveeq::UserPresetLibrary::fileFor(name);
        if(file.existsAsFile() && file!=awaitingOverwrite) {
            awaitingOverwrite=file;
            status=en?"Preset exists. Click SAVE again to replace it.":juce::String::fromUTF8(u8"Ce nom existe déjà. Cliquez encore sur SAUVER pour remplacer.");
            repaint();return;
        }
        const bool favorite=selectedIndex>=0 && userTabActive &&
                            userPresets[(size_t)selectedIndex].file==file ?
                            userPresets[(size_t)selectedIndex].favorite:false;
        const bool ok=loveeq::UserPresetLibrary::save(
                 file,name,presetCategory.getText(),presetTags.getText(),favorite,processor);
        awaitingOverwrite=juce::File();
        if(!ok) {
            status=en?"Save failed. Check folder permissions.":juce::String::fromUTF8(u8"Échec de sauvegarde : vérifiez les autorisations du dossier.");
            repaint();return;
        }
        setTab(true);
        int row=-1;
        for(int i=0;i<(int)userPresets.size();++i)
            if(userPresets[(size_t)i].file==file){row=i;break;}
        if(row>=0) {
            selectedIndex=row;
            filter();
        }
        status=en?"User preset saved to disk.":juce::String::fromUTF8(u8"Preset USER enregistré sur le disque.");
        repaint();
    }
    void toggleFavorite() {
        if(selectedIndex<0)return;
        const bool value=!rowFavorite(selectedIndex);
        if(userTabActive) {
            if(!loveeq::UserPresetLibrary::setFavorite(userPresets[(size_t)selectedIndex],value)){
                status=en?"Cannot update favorite.":"Impossible de modifier le favori.";
                repaint();return;
            }
            userPresets[(size_t)selectedIndex].favorite=value;
        } else processor.setFavorite((size_t)selectedIndex,value);
        favoriteButton.setButtonText(value?(en?"UNFAVORITE":"RETIRER FAVORI"):(en?"FAVORITE":"FAVORI"));
        filter();
    }
    void deleteSelected() {
        if(!userTabActive || selectedIndex<0)return;
        const auto file=userPresets[(size_t)selectedIndex].file;
        if(file!=awaitingDelete) {
            awaitingDelete=file;
            deleteButton.setButtonText(en?"CONFIRM DELETE":"CONFIRMER");
            status=en?"Click CONFIRM DELETE to permanently remove the preset.":
                      juce::String::fromUTF8(u8"Cliquez sur CONFIRMER pour supprimer définitivement le preset.");
            repaint();return;
        }
        const bool ok=loveeq::UserPresetLibrary::remove(userPresets[(size_t)selectedIndex]);
        awaitingDelete=juce::File();
        if(!ok) {
            status=en?"Unable to delete preset.":"Impossible de supprimer le preset.";
            repaint();return;
        }
        reloadUser();
        deleteButton.setButtonText(en?"DELETE":"SUPPRIMER");
        status=en?"User preset deleted.":juce::String::fromUTF8(u8"Preset USER supprimé.");
        repaint();
    }
};
