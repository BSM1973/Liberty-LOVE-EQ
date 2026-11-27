#pragma once
#if JUCE_IOS
// Compact facade for the existing, guarded analysis workflow. No duplicate DSP.
class LoveEQMobileAdvanced final : public juce::Component, private juce::Timer {
public:
    explicit LoveEQMobileAdvanced(LoveEQAudioProcessorEditor& e):editor(e){
        addAndMakeVisible(back);back.onClick=[this]{close();if(onClose)onClose();};
        addAndMakeVisible(title);title.setJustificationType(juce::Justification::centred);
        addAndMakeVisible(status);status.setJustificationType(juce::Justification::centred);
        status.setMinimumHorizontalScale(.7f);
        for(auto& b:actions){addAndMakeVisible(b);b.setColour(juce::TextButton::buttonColourId,juce::Colour(0xff293b49));}
        for(auto& c:choices)addAndMakeVisible(c);
        for(auto& sl:values){addAndMakeVisible(sl);sl.setSliderStyle(juce::Slider::LinearHorizontal);sl.setTextBoxStyle(juce::Slider::TextBoxRight,false,76,35);}
    }
    ~LoveEQMobileAdvanced() override {stopTimer();close();}
    std::function<void()> onClose;
    void open(int requested,int band){
        close();page=requested;
        editor.attachBand(band);
        editor.mobileAuxVisible=true;
        if(page==1){editor.dnaEnabled=true;editor.dnaButton.setToggleState(true,juce::dontSendNotification);editor.updateDnaControls();}
        if(page==2)openMode(editor.relationModeButton,editor.relationMode);
        if(page==3)openMode(editor.timeLensModeButton,editor.timeLensMode);
        if(page==4)openMode(editor.intentModeButton,editor.intentMode);
        if(page==5)openMode(editor.matchModeButton,editor.matchMode);
        stage=0;configure();setVisible(true);startTimerHz(8);refresh();resized();
    }
    void close(){
        if(page==0)return;
        stopTimer();
        if(page==4 && editor.intentMode)editor.closeIntentView();
        if(page==5 && editor.matchMode)editor.closeMatchView();
        if(page==2 && editor.relationMode){editor.relationModeButton.setToggleState(false,juce::dontSendNotification);if(editor.relationModeButton.onClick)editor.relationModeButton.onClick();}
        if(page==3 && editor.timeLensMode){editor.timeLensModeButton.setToggleState(false,juce::dontSendNotification);if(editor.timeLensModeButton.onClick)editor.timeLensModeButton.onClick();}
        for(auto& a:attachments)a.reset();
        editor.mobileAuxVisible=false;page=0;setVisible(false);
    }
    void paint(juce::Graphics& g) override {g.fillAll(juce::Colour(0xff151d26));}
    void resized() override {
        auto r=getLocalBounds().reduced(5);auto top=r.removeFromTop(44);
        back.setBounds(top.removeFromLeft(82).reduced(1));title.setBounds(top);
        auto footer=r.removeFromBottom(42);status.setBounds(footer.reduced(2));
        const int nc=choiceCount,nb=actionCount,nv=valueCount;
        const int columns=getWidth()<500?2:3;
        for(int i=0;i<nc;++i){auto row=r.removeFromTop(44);choices[(size_t)i].setBounds(row.reduced(1));}
        for(int i=0;i<nb;i+=columns){
            auto row=r.removeFromTop(44);
            for(int k=0;k<columns&&i+k<nb;++k){int x=row.getWidth()*k/columns,n=row.getWidth()*(k+1)/columns;actions[(size_t)(i+k)].setBounds(row.getX()+x,row.getY(),n-x-2,row.getHeight()-2);}
        }
        for(int i=0;i<nv;++i){values[(size_t)i].setBounds(r.removeFromTop(44).reduced(1));}
    }
private:
    LoveEQAudioProcessorEditor& editor;
    int page=0,stage=0,actionCount=0,choiceCount=0,valueCount=0;
    juce::TextButton back{"EQ"};
    juce::Label title,status;
    std::array<juce::TextButton,8> actions;
    std::array<juce::ComboBox,3> choices;
    std::array<juce::Slider,3> values;
    std::array<std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment>,3> attachments;
    std::array<juce::TextButton*,8> originals{};
    std::array<juce::ComboBox*,3> originalChoices{};
    bool en() const{return editor.mobileEnglish();}
    void openMode(juce::TextButton& b,bool opened){if(!opened){b.setToggleState(true,juce::dontSendNotification);if(b.onClick)b.onClick();}}
    void button(juce::TextButton& original){
        const auto i=(size_t)actionCount++;originals[i]=&original;
        actions[i].onClick=[this,i]{auto* b=originals[i];if(b&&b->isEnabled()&&b->onClick)b->onClick();refresh();};
    }
    void choice(juce::ComboBox& original){
        const auto i=(size_t)choiceCount++;originalChoices[i]=&original;
        auto& c=choices[i];c.clear(juce::dontSendNotification);
        for(int k=0;k<original.getNumItems();++k)c.addItem(original.getItemText(k),original.getItemId(k));
        c.setSelectedId(original.getSelectedId(),juce::dontSendNotification);
        c.onChange=[this,i]{auto* b=originalChoices[i];if(b){b->setSelectedId(choices[i].getSelectedId(),juce::dontSendNotification);if(b->onChange)b->onChange();}refresh();};
    }
    void value(const juce::String& id,const juce::String& suffix){
        const auto i=(size_t)valueCount++;values[i].setTextValueSuffix(suffix);
        attachments[i]=std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(editor.processor.parameters,id,values[i]);
    }
    void configure(){
        for(auto& a:attachments)a.reset();
        for(auto& a:actions){a.onClick=nullptr;a.setBounds(juce::Rectangle<int>{});}
        for(auto& c:choices){c.onChange=nullptr;c.setBounds(juce::Rectangle<int>{});}
        for(auto& sl:values)sl.setBounds(juce::Rectangle<int>{});
        originals.fill(nullptr);originalChoices.fill(nullptr);actionCount=choiceCount=valueCount=0;
        if(page==1){title.setText("HARMONIC DNA",juce::dontSendNotification);button(editor.dnaFocusButton);button(editor.dnaCutButton);}
        if(page==2){title.setText("RELATIONSHIP EQ",juce::dontSendNotification);choice(editor.relationRoleBox);choice(editor.relationPriorityBox);choice(editor.relationGroupBox);button(editor.relationSuggestButton);button(editor.relationApplyButton);}
        if(page==3){
            title.setText("TIME LENS",juce::dontSendNotification);
            auto& c=choices[0];choiceCount=1;c.clear(juce::dontSendNotification);c.addItemList(en()?juce::StringArray{"Attack","Body","Tail"}:juce::StringArray{"Attaque","Corps","Fin"},1);
            c.setSelectedId(stage+1,juce::dontSendNotification);c.onChange=[this]{stage=juce::jlimit(0,2,choices[0].getSelectedId()-1);configure();refresh();resized();};
            actionCount=1;actions[0].onClick=[this]{auto* p=editor.processor.parameters.getParameter("time_lens_enabled");if(p){p->beginChangeGesture();p->setValueNotifyingHost(p->getValue()>.5f?0.f:1.f);p->endChangeGesture();}refresh();};
            value(LoveEQAudioProcessor::timeStageId(stage,"hz")," Hz");value(LoveEQAudioProcessor::timeStageId(stage,"gain")," dB");value(LoveEQAudioProcessor::timeStageId(stage,"q")," Q");
        }
        if(page==4){title.setText("LOVE INTENT",juce::dontSendNotification);choice(editor.intentGoalBox);button(editor.intentAnalyzeButton);for(auto& b:editor.intentVariantButtons)button(b);button(editor.intentOriginalButton);button(editor.intentPreviewButton);button(editor.intentCommitButton);button(editor.intentCancelButton);}
        if(page==5){title.setText("EQ MATCH",juce::dontSendNotification);button(editor.matchReferenceButton);button(editor.matchSourceButton);button(editor.matchProposeButton);button(editor.matchPreviewButton);button(editor.matchCommitButton);button(editor.matchCancelButton);}
    }
    void refresh(){
        for(int i=0;i<actionCount;++i)if(auto* b=originals[(size_t)i]){
            auto& a=actions[(size_t)i];a.setButtonText(b->getButtonText());a.setEnabled(b->isEnabled());a.setToggleState(b->getToggleState(),juce::dontSendNotification);
        }
        for(int i=0;i<choiceCount;++i)if(auto* c=originalChoices[(size_t)i])choices[(size_t)i].setSelectedId(c->getSelectedId(),juce::dontSendNotification);
        juce::String message;
        if(page==1)message=editor.dnaResult.reliable?juce::String("F0 : ")+juce::String(editor.dnaResult.fundamentalHz,1)+" Hz":en()?"Play audio for analysis":"Lancez la lecture pour analyser";
        if(page==2)message=editor.relationStatus;
        if(page==3){const bool on=editor.processor.parameters.getRawParameterValue("time_lens_enabled")->load()>.5f;actions[0].setButtonText(on?(en()?"ON":"ACTIF"):(en()?"OFF":"INACTIF"));actions[0].setToggleState(on,juce::dontSendNotification);message=en()?"Independent stage filters":"Filtres indépendants par phase";}
        if(page==4)message=editor.intentMessage;
        if(page==5)message=editor.matchStatus;
        status.setText(message,juce::dontSendNotification);
    }
    void timerCallback() override {if(isShowing())refresh();}
};
#endif
