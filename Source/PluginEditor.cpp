#include "PluginEditor.h"
#include <BinaryData.h>

namespace
{
    juce::Image loadImg (const void* data, int size)
    {
        return juce::ImageCache::getFromMemory (data, size);
    }

    #define IMG(name) loadImg (BinaryData::name, BinaryData::name##Size)

    juce::Font mono (float h, bool bold = true)
    {
        return juce::Font (juce::FontOptions (juce::Font::getDefaultMonospacedFontName(), h,
                                              bold ? juce::Font::bold : juce::Font::plain));
    }

    const juce::Colour lcdMain (126, 232, 255);
    const juce::Colour lcdDim  (70, 150, 185);
    const juce::Colour lcdDark (6, 28, 42);
}

// ===========================================================================
//  Utility panel (Audio I/O routing)
// ===========================================================================
UtilityPanel::UtilityPanel (ReverbFXProcessor& p) : proc (p)
{
    title.setText ("UTILITY", juce::dontSendNotification);
    title.setJustificationType (juce::Justification::centred);
    title.setColour (juce::Label::textColourId, juce::Colours::white);
    addAndMakeVisible (title);

    const juce::StringArray items  { "None", "Channel 1 (L)", "Channel 2 (R)" };
    const juce::StringArray names  { "Input 1 (L)", "Input 2 (R)", "Output 1 (L)", "Output 2 (R)" };

    for (int i = 0; i < 4; ++i)
    {
        choices[i].addItemList (items, 1);
        addAndMakeVisible (choices[i]);

        labels[i].setText (names[i], juce::dontSendNotification);
        labels[i].setColour (juce::Label::textColourId, juce::Colours::white);
        addAndMakeVisible (labels[i]);

        choices[i].onChange = [this, i]
        {
            const int v = choices[i].getSelectedId() - 1;      // 0 = None, 1 = ch1, 2 = ch2
            if      (i == 0) proc.input1Source.store (v);
            else if (i == 1) proc.input2Source.store (v);
            else if (i == 2) proc.output1Source.store (v);
            else             proc.output2Source.store (v);
        };
    }

    sectionLabel.setText ("PRESET", juce::dontSendNotification);
    sectionLabel.setColour (juce::Label::textColourId, juce::Colour (0xff7ee8ff));
    addAndMakeVisible (sectionLabel);

    activeLabel.setColour (juce::Label::textColourId, juce::Colours::white);
    activeLabel.setJustificationType (juce::Justification::centredRight);
    addAndMakeVisible (activeLabel);

    copyLabel.setText ("Copy ke no.", juce::dontSendNotification);
    copyLabel.setColour (juce::Label::textColourId, juce::Colours::white);
    addAndMakeVisible (copyLabel);
    addAndMakeVisible (copyTarget);

    statusLabel.setColour (juce::Label::textColourId, juce::Colour (0xff7ee8ff));
    statusLabel.setJustificationType (juce::Justification::centred);
    addAndMakeVisible (statusLabel);

    copyBtn.setButtonText ("COPY PRESET");
    copyBtn.onClick = [this]
    {
        disarm();
        const int dst = copyTarget.getSelectedId() - 1;
        if (dst >= 0 && onCopyPreset) statusLabel.setText (onCopyPreset (dst), juce::dontSendNotification);
        fillCopyChoices();
        refresh();
    };
    addAndMakeVisible (copyBtn);

    deleteBtn.onClick = [this]
    {
        if (armed != Armed::Delete) { armed = Armed::Delete; updateButtonTexts(); startTimer (3000); return; }
        disarm();
        if (onDeletePreset) statusLabel.setText (onDeletePreset(), juce::dontSendNotification);
        refresh();
    };
    addAndMakeVisible (deleteBtn);

    factoryBtn.onClick = [this]
    {
        if (armed != Armed::Factory) { armed = Armed::Factory; updateButtonTexts(); startTimer (3000); return; }
        disarm();
        if (onFactoryReset) statusLabel.setText (onFactoryReset(), juce::dontSendNotification);
        refresh();
    };
    addAndMakeVisible (factoryBtn);

    back.setButtonText ("BACK");
    back.onClick = [this] { disarm(); if (onBack) onBack(); };
    addAndMakeVisible (back);

    updateButtonTexts();
    refresh();
}

void UtilityPanel::timerCallback()   { disarm(); }

void UtilityPanel::disarm()
{
    stopTimer();
    if (armed != Armed::None) { armed = Armed::None; updateButtonTexts(); }
}

void UtilityPanel::updateButtonTexts()
{
    deleteBtn.setButtonText  (armed == Armed::Delete  ? "KLIK LAGI = HAPUS?" : "DELETE PRESET");
    factoryBtn.setButtonText (armed == Armed::Factory ? "KLIK LAGI = RESET?" : "FACTORY RESET");
    deleteBtn.setColour  (juce::TextButton::buttonColourId, armed == Armed::Delete  ? juce::Colour (0xffa02020) : juce::Colour (0xff3a3f45));
    factoryBtn.setColour (juce::TextButton::buttonColourId, armed == Armed::Factory ? juce::Colour (0xffa02020) : juce::Colour (0xff3a3f45));
}

void UtilityPanel::fillCopyChoices()
{
    const int keep = copyTarget.getSelectedId();
    copyTarget.clear (juce::dontSendNotification);
    for (int i = 0; i < PresetManager::numSlots; ++i)
        copyTarget.addItem (juce::String (i + 1).paddedLeft ('0', 2) + "  "
                              + (proc.presets.isUsed (i) ? proc.presets.getName (i) : juce::String ("(kosong)")),
                            i + 1);
    copyTarget.setSelectedId (keep > 0 ? keep : 1, juce::dontSendNotification);
}

void UtilityPanel::refresh()
{
    choices[0].setSelectedId (proc.input1Source.load()  + 1, juce::dontSendNotification);
    choices[1].setSelectedId (proc.input2Source.load()  + 1, juce::dontSendNotification);
    choices[2].setSelectedId (proc.output1Source.load() + 1, juce::dontSendNotification);
    choices[3].setSelectedId (proc.output2Source.load() + 1, juce::dontSendNotification);

    fillCopyChoices();

    const int cur = juce::jlimit (0, PresetManager::numSlots - 1, proc.currentPreset.load());
    activeLabel.setText ("Aktif: " + juce::String (cur + 1) + " "
                           + (proc.presets.isUsed (cur) ? proc.presets.getName (cur) : juce::String ("(kosong)")),
                         juce::dontSendNotification);
}

void UtilityPanel::paint (juce::Graphics& g)
{
    const float s = (float) getWidth() / 820.f;
    auto r = getLocalBounds().toFloat().reduced (1.f);
    g.setColour (juce::Colour (0xf20a1724));
    g.fillRoundedRectangle (r, 10.f * s);
    g.setColour (juce::Colour (0xff2b8dc4));
    g.drawRoundedRectangle (r, 10.f * s, 2.f * s);

    g.setColour (juce::Colour (0xff2b8dc4).withAlpha (0.6f));
    g.fillRect (juce::Rectangle<float> (30.f * s, 178.f * s, 760.f * s, 1.5f * s));
}

void UtilityPanel::resized()
{
    const float s = (float) getWidth() / 820.f;
    auto R = [s] (float x, float y, float w, float h)
    {
        return juce::Rectangle<float> (x * s, y * s, w * s, h * s).toNearestInt();
    };

    title.setFont (juce::Font (juce::FontOptions (26.f * s).withStyle ("Bold")));
    title.setBounds (R (20, 10, 780, 40));

    for (int i = 0; i < 4; ++i)
    {
        const int col = i % 2, row = i / 2;
        const float x = 30.f + (float) col * 395.f, y = 58.f + (float) row * 52.f;
        labels[i].setFont (juce::Font (juce::FontOptions (17.f * s)));
        labels[i].setBounds (R (x, y, 140, 38));
        choices[i].setBounds (R (x + 145, y, 215, 38));
    }

    sectionLabel.setFont (juce::Font (juce::FontOptions (17.f * s).withStyle ("Bold")));
    sectionLabel.setBounds (R (30, 184, 200, 28));
    activeLabel.setFont (juce::Font (juce::FontOptions (17.f * s)));
    activeLabel.setBounds (R (300, 184, 490, 28));

    copyLabel.setFont (juce::Font (juce::FontOptions (17.f * s)));
    copyLabel.setBounds (R (30, 218, 130, 38));
    copyTarget.setBounds (R (165, 218, 330, 38));
    copyBtn.setBounds (R (510, 218, 280, 38));

    deleteBtn.setBounds (R (30, 264, 375, 38));
    factoryBtn.setBounds (R (415, 264, 375, 38));

    statusLabel.setFont (juce::Font (juce::FontOptions (15.f * s)));
    statusLabel.setBounds (R (30, 306, 560, 28));
    back.setBounds (R (610, 306, 180, 30));
}

// ===========================================================================
//  LCD
// ===========================================================================
LcdComponent::LcdComponent()
{
    screenBg = IMG (lcd_screen_bg_png);
    glass    = IMG (lcd_glass_overlay_png);
    bezel    = IMG (lcd_bezel_png);
    redOn    = IMG (beat_red_on_png);
    redOff   = IMG (beat_red_off_png);
    greenOn  = IMG (beat_green_on_png);
    greenOff = IMG (beat_green_off_png);

    nameEditor.setMultiLine (false);
    nameEditor.setReturnKeyStartsNewLine (false);
    nameEditor.setInputRestrictions (16);
    nameEditor.setJustification (juce::Justification::centredLeft);
    nameEditor.setIndents (6, 0);
    nameEditor.setColour (juce::TextEditor::backgroundColourId, lcdDark.withAlpha (0.9f));
    nameEditor.setColour (juce::TextEditor::textColourId, lcdMain);
    nameEditor.setColour (juce::TextEditor::outlineColourId, lcdDim);
    nameEditor.setColour (juce::TextEditor::focusedOutlineColourId, lcdMain);
    nameEditor.setColour (juce::TextEditor::highlightColourId, lcdMain.withAlpha (0.35f));
    nameEditor.setColour (juce::TextEditor::highlightedTextColourId, lcdMain);
    nameEditor.setColour (juce::CaretComponent::caretColourId, lcdMain);
    nameEditor.onReturnKey = [this] { if (onSubmit) onSubmit(); };
    nameEditor.onEscapeKey = [this] { if (onCancel) onCancel(); };
    nameEditor.setVisible (false);
    addChildComponent (nameEditor);
}

void LcdComponent::setModel (const LcdModel& m)
{
    model = m;
    repaint();
}

void LcdComponent::showStore (const juce::String& initialName)
{
    nameEditor.setText (initialName, false);
    nameEditor.setVisible (true);
    nameEditor.grabKeyboardFocus();
    nameEditor.selectAll();
}

void LcdComponent::hideStore()
{
    nameEditor.setVisible (false);
}

void LcdComponent::resized()
{
    const float s = (float) getWidth() / 300.f;
    nameEditor.setBounds (juce::Rectangle<float> (22.f * s, 66.f * s, 256.f * s, 34.f * s).toNearestInt());
    const auto f = mono (18.f * s);
    nameEditor.setFont (f);
    nameEditor.applyFontToAllText (f);
}

void LcdComponent::paint (juce::Graphics& g)
{
    const float s = (float) getWidth() / 300.f;
    g.addTransform (juce::AffineTransform::scale (s));
    g.setImageResamplingQuality (juce::Graphics::highResamplingQuality);

    const float sx = 10.f, sy = 10.f;
    g.drawImage (screenBg, juce::Rectangle<float> (sx, sy, 280.f, 160.f));

    {
        juce::Graphics::ScopedSaveState ss (g);
        g.reduceClipRegion (juce::Rectangle<float> (sx + 4, sy + 4, 272.f, 152.f).toNearestInt());

        if (model.mode == LcdModel::Home)
        {
            g.setColour (lcdMain);
            g.setFont (mono (38.f));
            g.drawText (model.typeName, juce::Rectangle<float> (sx, sy + 14.f, 280.f, 50.f),
                        juce::Justification::centred, false);

            g.setFont (mono (17.f));
            g.drawText (model.presetLine, juce::Rectangle<float> (sx + 14.f, sy + 76.f, 252.f, 24.f),
                        juce::Justification::centredLeft, true);

            if (model.hint.isNotEmpty())
            {
                g.setColour (lcdDim.brighter (0.3f));
                g.setFont (mono (11.f, false));
                g.drawText (model.hint, juce::Rectangle<float> (sx + 14.f, sy + 99.f, 252.f, 16.f),
                            juce::Justification::centredLeft, true);
            }

            for (int i = 0; i < 4; ++i)
            {
                const bool on = (model.led == i);
                const auto& img = (i == 0) ? (on ? redOn : redOff) : (on ? greenOn : greenOff);
                g.drawImage (img, juce::Rectangle<float> (sx + 14.f + 22.f * (float) i, sy + 128.f, 20.f, 20.f));
            }

            g.setColour (model.led >= 0 ? lcdMain : lcdDim);
            g.setFont (mono (20.f));
            g.drawText (model.bpmText, juce::Rectangle<float> (sx + 106.f, sy + 126.f, 160.f, 24.f),
                        juce::Justification::centredRight, false);
        }
        else if (model.mode == LcdModel::Menu)
        {
            g.setColour (lcdDim.brighter (0.4f));
            g.setFont (mono (14.f));
            g.drawText (model.title, juce::Rectangle<float> (sx, sy + 7.f, 280.f, 22.f),
                        juce::Justification::centred, false);

            const int count = model.labels.size();
            for (int r = 0; r < 4; ++r)
            {
                const int idx = model.first + r;
                if (idx >= count) break;

                const float y = sy + 34.f + 27.f * (float) r;
                const bool sel = (idx == model.selected);
                if (sel)
                {
                    g.setColour (lcdMain.withAlpha (0.92f));
                    g.fillRoundedRectangle (sx + 8.f, y, 258.f, 25.f, 4.f);
                }
                g.setColour (sel ? lcdDark : lcdMain);
                g.setFont (mono (17.f));
                g.drawText (model.labels[idx], juce::Rectangle<float> (sx + 16.f, y, 130.f, 25.f),
                            juce::Justification::centredLeft, false);
                g.drawFittedText (model.values[idx], juce::Rectangle<float> (sx + 120.f, y, 138.f, 25.f).toNearestInt(),
                                  juce::Justification::centredRight, 1, 0.7f);
            }

            g.setColour (lcdDim);
            if (model.first > 0)
            {
                juce::Path p; p.addTriangle (sx + 270.f, sy + 40.f, sx + 275.f, sy + 40.f, sx + 272.5f, sy + 35.f);
                g.fillPath (p);
            }
            if (model.first + 4 < count)
            {
                juce::Path p; p.addTriangle (sx + 270.f, sy + 138.f, sx + 275.f, sy + 138.f, sx + 272.5f, sy + 143.f);
                g.fillPath (p);
            }
        }
        else // Store
        {
            g.setColour (lcdMain);
            g.setFont (mono (16.f));
            g.drawText (model.storeTitle, juce::Rectangle<float> (sx, sy + 12.f, 280.f, 24.f),
                        juce::Justification::centred, false);

            g.setColour (lcdDim.brighter (0.3f));
            g.setFont (mono (12.f));
            g.drawText ("NAMA PRESET:", juce::Rectangle<float> (sx + 14.f, sy + 40.f, 252.f, 18.f),
                        juce::Justification::centredLeft, false);

            g.setFont (mono (11.f, false));
            g.drawText ("KLIK KNOP = SIMPAN", juce::Rectangle<float> (sx, sy + 112.f, 280.f, 16.f),
                        juce::Justification::centred, false);
            g.drawText ("UP/DN/KNOP = PILIH NO | EXIT = BATAL", juce::Rectangle<float> (sx, sy + 128.f, 280.f, 16.f),
                        juce::Justification::centred, false);
        }
    }

    g.drawImage (glass, juce::Rectangle<float> (sx, sy, 280.f, 160.f));
    g.drawImage (bezel, juce::Rectangle<float> (0.f, 0.f, 300.f, 180.f));
}

// ===========================================================================
//  Editor
// ===========================================================================
ReverbFXEditor::ReverbFXEditor (ReverbFXProcessor& p)
    : juce::AudioProcessorEditor (&p), proc (p),
      background (IMG (background_panel_png)),
      btnDown    (IMG (btn_down_up_png),    IMG (btn_down_down_png)),
      btnUp      (IMG (btn_up_up_png),      IMG (btn_up_down_png)),
      btnUtility (IMG (btn_utility_up_png), IMG (btn_utility_down_png)),
      btnMenu    (IMG (btn_menu_up_png),    IMG (btn_menu_down_png)),
      btnExit    (IMG (btn_exit_up_png),    IMG (btn_exit_down_png)),
      btnStore   (IMG (btn_store_up_png),   IMG (btn_store_down_png)),
      btnTap     (IMG (btn_tap_up_png),     IMG (btn_tap_down_png), IMG (btn_tap_led_glow_png)),
      typeKnob   (IMG (knob_large_strip_64f_png)),
      knobInput  (IMG (knob_small_strip_64f_png)),
      knobMix    (IMG (knob_small_strip_64f_png)),
      knobOutput (IMG (knob_small_strip_64f_png)),
      knobLow    (IMG (knob_small_strip_64f_png)),
      knobHigh   (IMG (knob_small_strip_64f_png)),
      utilityPanel (p)
{
    setWantsKeyboardFocus (true);
    browseIdx = juce::jlimit (0, PresetManager::numSlots - 1, proc.currentPreset.load());

    for (auto* c : std::initializer_list<juce::Component*> { &lcd, &btnDown, &btnUp, &btnUtility, &btnMenu,
                                                             &btnExit, &btnStore, &btnTap, &typeKnob,
                                                             &knobInput, &knobMix, &knobOutput, &knobLow, &knobHigh })
        addAndMakeVisible (c);

    addChildComponent (utilityPanel);        // added last = drawn on top of knobs; hidden until Utility is pressed
    utilityPanel.onBack = [this] { utilityPanel.setVisible (false); grabKeyboardFocus(); };

    utilityPanel.onDeletePreset = [this] () -> juce::String
    {
        const int cur = juce::jlimit (0, PresetManager::numSlots - 1, proc.currentPreset.load());
        if (! proc.presets.remove (cur))
            return "Preset " + juce::String (cur + 1) + " sudah kosong";
        flash ("PRESET DELETED");
        return "Preset " + juce::String (cur + 1) + " dihapus";
    };

    utilityPanel.onCopyPreset = [this] (int dst) -> juce::String
    {
        const int cur = juce::jlimit (0, PresetManager::numSlots - 1, proc.currentPreset.load());
        if (! proc.presets.isUsed (cur))
            return "Preset aktif kosong - tidak ada yang disalin";
        if (dst == cur)
            return "Tujuan sama dengan preset aktif";
        proc.presets.copy (cur, dst);
        flash ("PRESET COPIED");
        return "Preset " + juce::String (cur + 1) + " disalin ke " + juce::String (dst + 1);
    };

    utilityPanel.onFactoryReset = [this] () -> juce::String
    {
        proc.presets.factoryReset();
        proc.input1Source.store (1); proc.input2Source.store (2);
        proc.output1Source.store (1); proc.output2Source.store (2);
        proc.presets.recall (0);
        proc.currentPreset.store (0);
        browseIdx = 0;
        flash ("FACTORY RESET");
        return "Factory reset selesai (preset 1-5 pabrik, 6-25 kosong)";
    };

    attInput  = std::make_unique<SliderAtt> (proc.apvts, "input",  knobInput);
    attMix    = std::make_unique<SliderAtt> (proc.apvts, "mix",    knobMix);
    attOutput = std::make_unique<SliderAtt> (proc.apvts, "output", knobOutput);
    attLow    = std::make_unique<SliderAtt> (proc.apvts, "eqlow",  knobLow);
    attHigh   = std::make_unique<SliderAtt> (proc.apvts, "eqhigh", knobHigh);

    for (auto* k : { &knobInput, &knobMix, &knobOutput, &knobLow, &knobHigh })
        k->setPopupDisplayEnabled (true, true, this);

    btnDown.onClick    = [this] { navigate (-1); };
    btnUp.onClick      = [this] { navigate (+1); };
    btnUtility.onClick = [this] { toggleUtility(); };
    btnMenu.onClick    = [this] { utilityPanel.setVisible (false); if (view == View::Store) cancelStore(); view = View::Menu; };
    btnExit.onClick    = [this]
    {
        utilityPanel.setVisible (false);
        if (view == View::Store) cancelStore();
        view = View::Home;
        browseIdx = proc.currentPreset.load();
    };
    btnStore.onClick   = [this] { if (view != View::Store) startStore(); };
    btnTap.setTriggeredOnMouseDown (true);
    btnTap.onClick     = [this] { doTap(); };

    typeKnob.onStep  = [this] (int n) { onKnobStep (n); };
    typeKnob.onClick = [this] { onKnobClick(); };

    lcd.onSubmit = [this] { commitStore(); };
    lcd.onCancel = [this] { cancelStore(); };

    addMouseListener (this, true);

    setResizable (true, true);
    setResizeLimits (800, 200, 1600, 400);
    getConstrainer()->setFixedAspectRatio (4.0);
    setSize (1200, 300);

    startTimerHz (30);
}

ReverbFXEditor::~ReverbFXEditor()
{
    stopTimer();
    removeMouseListener (this);
}

void ReverbFXEditor::paint (juce::Graphics& g)
{
    g.setImageResamplingQuality (juce::Graphics::highResamplingQuality);
    g.drawImage (background, getLocalBounds().toFloat());
}

void ReverbFXEditor::resized()
{
    const float s = (float) getWidth() / 1600.f;
    auto place = [s] (juce::Component& c, float x, float y, float w, float h)
    {
        c.setBounds (juce::Rectangle<float> (x * s, y * s, w * s, h * s).toNearestInt());
    };

    place (lcd,        62.f, 110.f, 300.f, 180.f);
    place (btnDown,   386.f, 131.f, 52.f, 42.f);
    place (btnUp,     440.f, 131.f, 52.f, 42.f);
    place (btnUtility, 386.f, 179.f, 52.f, 42.f);
    place (btnMenu,   440.f, 179.f, 52.f, 42.f);
    place (btnExit,   386.f, 227.f, 52.f, 42.f);
    place (btnStore,  440.f, 227.f, 52.f, 42.f);
    place (typeKnob,  510.f, 130.f, 140.f, 140.f);
    place (knobInput,  728.f, 168.f, 64.f, 64.f);
    place (knobMix,    813.f, 168.f, 64.f, 64.f);
    place (knobOutput, 898.f, 168.f, 64.f, 64.f);
    place (knobLow,   1058.f, 168.f, 64.f, 64.f);
    place (knobHigh,  1158.f, 168.f, 64.f, 64.f);
    place (btnTap,    1345.f, 164.f, 110.f, 72.f);
    place (utilityPanel, 510.f, 40.f, 820.f, 340.f);
}

void ReverbFXEditor::mouseDown (const juce::MouseEvent&)
{
    if (view != View::Store)
        grabKeyboardFocus();
}

bool ReverbFXEditor::keyPressed (const juce::KeyPress& k)
{
    if (k == juce::KeyPress::spaceKey)
    {
        if (! spaceHeld)
        {
            spaceHeld = true;
            btnTap.setForcedDown (true);
            doTap();
        }
        return true;
    }
    return false;
}

bool ReverbFXEditor::keyStateChanged (bool)
{
    if (spaceHeld && ! juce::KeyPress::isKeyCurrentlyDown (juce::KeyPress::spaceKey))
    {
        spaceHeld = false;
        btnTap.setForcedDown (false);
    }
    return false;
}

// ---------------------------------------------------------------------------
const ReverbFXEditor::MenuItem* ReverbFXEditor::menuItems (int& count) const
{
    static const MenuItem reverbItems[] = {
        { "decay",     "Decay",    0.05, true  },
        { "predelay",  "PreDelay", 1.0,  false },
        { "room",      "Room",     1.0,  false },
        { "diffusion", "Diffusion",1.0,  false },
        { "damping",   "Damping",  1.0,  false },
        { "lowcut",    "Low Cut",  0.06, true  },
        { "hicut",     "Hi Cut",   0.06, true  }
    };
    static const MenuItem delayItems[] = {
        { "dlytime", "Time",     1.0,  false },
        { "dlyfb",   "Feedback", 1.0,  false },
        { "dlyhpf",  "Hpf",      0.06, true  },
        { "dlylpf",  "Lpf",      0.06, true  }
    };

    static const MenuItem harmonyItems[] = {
        { "hkey",   "Key",   1.0, false },
        { "hmode",  "Mode",  1.0, false },
        { "hscale", "Sharp", 1.0, false },
        { "htone",  "Tone",  1.0, false },
        { "hout",   "Out",   0.5, false }
    };

    if (proc.getFxType() == 3) { count = 5; return harmonyItems; }
    if (proc.getFxType() == 2) { count = 4; return delayItems; }
    count = 7;
    return reverbItems;
}

void ReverbFXEditor::setStoreSlot (int slot)
{
    storeSlot = juce::jlimit (0, PresetManager::numSlots - 1, slot);
    lcd.showStore (proc.presets.isUsed (storeSlot) ? proc.presets.getName (storeSlot)
                                                    : "Preset " + juce::String (storeSlot + 1));
}

void ReverbFXEditor::navigate (int dir)
{
    switch (view)
    {
        case View::Home:
        {
            // Tombol Up / Down = ganti tipe FX (Plate, Hall, Delay, Harmony, SPX Plate), berputar
            const int n = 5;
            setType ((proc.getFxType() + dir + n) % n);
            break;
        }

        case View::Menu:
        {
            int count = 0; menuItems (count);
            menuSel = juce::jlimit (0, count - 1, menuSel - dir);   // Up = baris atas, Down = baris bawah
            break;
        }

        case View::Store:
            setStoreSlot (storeSlot + dir);
            break;
    }
}

void ReverbFXEditor::setType (int idx)
{
    if (auto* p = proc.apvts.getParameter ("type"))
    {
        p->beginChangeGesture();
        p->setValueNotifyingHost (p->convertTo0to1 ((float) juce::jlimit (0, 4, idx)));
        p->endChangeGesture();
    }
}

void ReverbFXEditor::stepParam (const MenuItem& item, int steps)
{
    auto* p = proc.apvts.getParameter (item.id);
    if (p == nullptr) return;

    if (dynamic_cast<juce::AudioParameterChoice*> (p) != nullptr)   // pilihan: 1 langkah per putaran
        steps = steps > 0 ? 1 : (steps < 0 ? -1 : 0);

    float real = p->convertFrom0to1 (p->getValue());
    if (item.mult) real *= std::pow (1.0f + (float) item.step, (float) steps);
    else           real += (float) item.step * (float) steps;

    real = p->getNormalisableRange().snapToLegalValue (real);

    p->beginChangeGesture();
    p->setValueNotifyingHost (p->convertTo0to1 (real));
    p->endChangeGesture();
}

void ReverbFXEditor::onKnobStep (int steps)
{
    const double now = juce::Time::getMillisecondCounterHiRes();
    const double dt = now - lastStepMs;
    lastStepMs = now;
    const int accel = dt < 35.0 ? 8 : (dt < 90.0 ? 3 : 1);

    if (view == View::Home)
    {
        // Knop PROGRAM = pilih preset tersimpan (1 langkah per klik), klik knop = load
        browseIdx = juce::jlimit (0, PresetManager::numSlots - 1, browseIdx + (steps > 0 ? 1 : -1));
    }
    else if (view == View::Store)
    {
        setStoreSlot (storeSlot + (steps > 0 ? 1 : -1));
    }
    else if (view == View::Menu)
    {
        int count = 0;
        const auto* items = menuItems (count);
        menuSel = juce::jlimit (0, count - 1, menuSel);
        stepParam (items[menuSel], steps * accel);
    }
}

void ReverbFXEditor::onKnobClick()
{
    if (view == View::Home)
    {
        if (proc.presets.recall (browseIdx))
        {
            proc.currentPreset.store (browseIdx);
            flash ("PRESET LOADED");
        }
        else
        {
            flash ("SLOT KOSONG");
        }
    }
    else if (view == View::Store)
    {
        commitStore();
    }
}

void ReverbFXEditor::doTap()
{
    const bool ok = proc.tapTempo();
    tapFlashUntil = juce::Time::getMillisecondCounterHiRes() + 120.0;
    if (! ok)
        flash ("TAP: DELAY / HARMONY");
    else if (proc.getFxType() == 3)
        if (auto* p = proc.apvts.getParameter ("hscale"))
            flash ("SHARP: " + p->getCurrentValueAsText().toUpperCase());
}

void ReverbFXEditor::startStore()
{
    view = View::Store;
    storeSlot = browseIdx;
    lcd.showStore (proc.presets.isUsed (storeSlot) ? proc.presets.getName (storeSlot)
                                                    : "Preset " + juce::String (storeSlot + 1));
}

void ReverbFXEditor::commitStore()
{
    if (view != View::Store) return;

    auto name = lcd.getStoreName().trim();
    if (name.isEmpty()) name = "Preset " + juce::String (storeSlot + 1);

    proc.presets.store (storeSlot, name);
    proc.currentPreset.store (storeSlot);
    browseIdx = storeSlot;

    lcd.hideStore();
    view = View::Home;
    flash ("PRESET SAVED");
    grabKeyboardFocus();
}

void ReverbFXEditor::cancelStore()
{
    lcd.hideStore();
    view = View::Home;
    grabKeyboardFocus();
}

void ReverbFXEditor::flash (const juce::String& text)
{
    flashText = text;
    flashUntil = juce::Time::getMillisecondCounterHiRes() + 1500.0;
}

void ReverbFXEditor::toggleUtility()
{
    if (utilityPanel.isVisible())
    {
        utilityPanel.setVisible (false);
        return;
    }

    if (view == View::Store) cancelStore();
    utilityPanel.refresh();
    utilityPanel.setVisible (true);
    utilityPanel.toFront (false);
}

// ---------------------------------------------------------------------------
void ReverbFXEditor::timerCallback()
{
    const double now = juce::Time::getMillisecondCounterHiRes();

    if (spaceHeld && ! juce::KeyPress::isKeyCurrentlyDown (juce::KeyPress::spaceKey))
    {
        spaceHeld = false;
        btnTap.setForcedDown (false);
    }

    const int type = proc.getFxType();
    int count = 0;
    const auto* items = menuItems (count);
    menuSel = juce::jlimit (0, count - 1, menuSel);
    if (menuSel < menuFirst) menuFirst = menuSel;
    if (menuSel >= menuFirst + 4) menuFirst = menuSel - 3;

    static const char* typeNames[] = { "PLATE", "HALL", "DELAY", "HARMONY", "SPX PLATE" };

    LcdModel m;
    m.mode = (view == View::Home) ? LcdModel::Home : (view == View::Menu ? LcdModel::Menu : LcdModel::Store);
    m.typeName = typeNames[type];

    // --- preset ---
    const juce::String nm = proc.presets.isUsed (browseIdx) ? proc.presets.getName (browseIdx) : juce::String ("Empty");
    m.presetLine = juce::String (browseIdx + 1) + " " + nm;

    const bool pending = browseIdx != proc.currentPreset.load();
    const bool blinkOn = ((int) (now / 500.0) % 2) == 0;
    if (now < flashUntil)      m.hint = flashText;
    else if (pending && blinkOn) m.hint = "[KLIK KNOP = LOAD]";

    // --- LED tempo & BPM ---
    double beatFrac = 1.0;
    if (type == 2)
    {
        const double period = juce::jmax (30.0, (double) proc.getParamReal ("dlytime"));
        const double el = juce::jmax (0.0, now - proc.beatOriginMs.load());
        m.led = (int) ((long long) (el / period) % 4);
        beatFrac = std::fmod (el, period) / period;
        m.bpmText = juce::String (juce::roundToInt (60000.0 / period)) + " BPM";
    }
    else if (type == 3)
    {
        static const char* keyNames[] = { "C", "D", "E", "F", "G", "A", "B" };
        const int k = juce::jlimit (0, 6, (int) std::lround (proc.getParamReal ("hkey")));
        m.led = -1;
        m.bpmText = juce::String (keyNames[k]) + (proc.getParamReal ("hscale") > 0.5f ? " MIN" : " MAJ");
    }
    else
    {
        m.led = -1;
        m.bpmText = "--- BPM";
    }

    // --- menu ---
    m.title = juce::String (typeNames[type]) + " MENU";
    for (int i = 0; i < count; ++i)
    {
        m.labels.add (items[i].label);
        if (auto* p = proc.apvts.getParameter (items[i].id))
            m.values.add (p->getCurrentValueAsText());
        else
            m.values.add ("");
    }
    m.selected = menuSel;
    m.first = menuFirst;

    // --- store ---
    m.storeTitle = "STORE > PRESET " + juce::String (storeSlot + 1);

    if (! modelValid || m != lastModel)
    {
        lastModel = m;
        modelValid = true;
        lcd.setModel (m);
    }

    // --- knob TYPE/PROGRAM ---
    float knobNorm = (float) browseIdx / (float) (PresetManager::numSlots - 1);
    if (view == View::Store)
        knobNorm = (float) storeSlot / (float) (PresetManager::numSlots - 1);
    if (view == View::Menu)
        if (auto* p = proc.apvts.getParameter (items[menuSel].id))
            knobNorm = p->getValue();
    typeKnob.setNormalised (knobNorm);

    // --- LED di tombol TAP ---
    btnTap.setGlow (now < tapFlashUntil || (type == 2 && m.led == 0 && beatFrac < 0.15));
}
