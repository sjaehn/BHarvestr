/* B.Harvestr
 * LV2 Plugin
 *
 * Copyright (C) 2018, 2019 by Sven Jähnichen
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#include "BHarvestrGUI.hpp"
#include <ctime>
#include "BUtilities/to_string.hpp"
#include "Envelope.hpp"

BHarvestrGUI::BHarvestrGUI (const char *bundle_path, const LV2_Feature *const *features, PuglNativeView parentWindow) :
	Window (1440, 880, "B.Harvestr", parentWindow, true, PUGL_MODULE, 0),
	controller (NULL), write_function (NULL),
	pluginPath (bundle_path ? std::string (bundle_path) : std::string ("")),
	sz (1.0), bgImageSurface (nullptr),
	uris (), forge (), samplePath ("."),
	mContainer (0, 0, 1440, 880, "main"),

	globalContainer (1060, 110, 360, 25, "box"),
	globalTitleIcon (100, 0, 160, 20, "widget", pluginPath + "inc/Global_settings.png"),
	globalMiniMaxiButton (330, 0, 20, 20, "redbutton"),
	globalMaxVoicesLabel (10, 25, 180, 20, "lflabel", "Max. voices:"),
	globalMaxGrainsLabel (10, 55, 180, 20, "lflabel", "Max. grains per voice:"),
	globalMaxVoicesSlider (220, 25, 130, 20, "dial", 10, 1, 16, 1, "%2.0f"),
	globalMaxGrainsSlider (220, 55, 130, 20, "dial", 15, 1, 63, 1, "%2.0f"),

	patchContainer (1060, 155, 360, 25, "box"),
	patchTitleIcon (100, 0, 160, 20, "widget", pluginPath + "inc/Patch_settings.png"),
	patchMiniMaxiButton (330, 0, 20, 20, "redbutton"),

	lfoContainer (20, 110, 360, 220, "widget"),
	seqContainer (20, 350, 360, 190, "widget"),
	rndContainer (20, 560, 360, 90, "widget"),
	envContainer (20, 670, 360, 190, "widget"),

	sampleContainer (400, 110, 640, 200, "widget"),
	sampleWidget (10, 34, 620, 122, "shape"),
	sampleSize (0.0f),
	sampleLoadButton (10, 170, 20, 20, "button"),
	sampleNameLabel (40, 170, 320, 20,"boxlabel", ""),
	sampleFileChooser (nullptr),
	sampleStartLine (7, 25, 6, 140, "marker", 0, 0, 1, 0),
	sampleEndLine (627, 25, 6, 140, "marker", 1, 0, 1, 0),
	sampleStartScreen (10, 32, 0, 126, "darkscreen"),
	sampleEndScreen (630, 32, 0, 126, "darkscreen"),
	sampleFrequencyDetectionButton (400, 170, 20, 20, "txtbutton", "?"),
	sampleFrequencySelect (440, 170, 120, 20, "select", 440, 10, 20000, 0.1, "%5.1f Hz"),
	sampleNoteListBox (),
	sampleSizeLabel (510, 0, 80, 12, "label", "Sample: 0.00 s"),
	sampleSelectionSizeLabel (510, 15, 80, 12, "label", "Selection: 0.00 s"),
	samplePlayButton (593, 0, 12, 12, "button"),
	sampleSelectionPlayButton (593, 15, 12, 12, "button"),

	patternContainer (400, 330, 640, 460, "widget"),
	patternWidget (10, 25, 620, 275, "pattern"),
	patternSelectionWidget (10, 25, 620, 275, "selection"),
	patternSizeSelect (350, 310, 90, 20, "select", 1, 0.01, 8, 0.01, "%1.2f"),
	patternTypeListBox
	(
		450, 310, 180, 20, 180, 180, "menu",
		BItems::ItemList
		({
			{0, "Forward playback"},
			{1, "Reverse playback"},
			{2, "Sine swinging playback"},
			{3, "Ping pong playback"},
			{4, "User pattern"}
		}),
		0
	),

	pianoRoll (400, 830, 640, 50, "piano", 0, 127),

	shapeContainer (1060, 200, 360, 240, "widget"),
	shapeScreen (2, 58, 356, 180, "screen"),
	shapeListBox
	(
		10, 30, 120, 20, 120, 200, "menu",
		BItems::ItemList
		({
			{0, "Sine"},
			{1, "Half sine"},
			{2, "Triangle"},
			{3, "Trapezoid"},
			{4, "User shape 1"},
			{5, "User shape 2"},
			{6, "User shape 3"},
			{7, "User shape 4"}
		}),
		0
	),
	clipboard (),

	grainContainer (1060, 460, 360, 490, "widget"),
	grainDisplay (1070, 700, 340, 150, "widget"),

	helpButton (1368, 68, 24, 24, "halobutton", "Help"),
	ytButton (1398, 68, 24, 24, "halobutton", "Video")

{
	// Init widgets
	// Patch

	patchLabel[PATCH_NAME] = BWidgets::Label (10, 30, 70, 12, "lflabel", "Name:");
	patchLabel[PATCH_TYPE] = BWidgets::Label (10, 50, 70, 12, "lflabel", "Type:");
	patchLabel[PATCH_DATE] = BWidgets::Label (210, 30, 40, 12, "lflabel", "Date:");
	patchLabel[PATCH_CREATOR] = BWidgets::Label (10, 70, 70, 12, "lflabel", "Creator:");
	patchLabel[PATCH_URI] = BWidgets::Label (10, 170, 70, 12, "lflabel", "URL:");
	patchLabel[PATCH_LICENSE] = BWidgets::Label (10, 90, 70, 12, "lflabel", "Licence:");
	patchLabel[PATCH_DESCRIPTION] = BWidgets::Label (10, 110, 70, 12, "lflabel", "Descript.:");
	patchInput[PATCH_NAME] = BWidgets::Label (80, 30, 120, 12, "lflabel", "<empty>");
	patchInput[PATCH_TYPE] = BWidgets::Label (80, 50, 270, 12, "lflabel", "<empty>");
	patchInput[PATCH_DATE] = BWidgets::Label (260, 30, 90, 12, "lflabel", "");
	patchInput[PATCH_CREATOR] = BWidgets::Label (80, 70, 760, 12, "lflabel", "<empty>");
	patchInput[PATCH_URI] = BWidgets::Label (80, 170, 270, 12, "lflabel", "<empty>");
	patchInput[PATCH_LICENSE] = BWidgets::Label (80, 90, 270, 12, "lflabel", "<empty>");
	patchInput[PATCH_DESCRIPTION] = BWidgets::Label (80, 110, 270, 12, "lflabel", "<empty>");

	// sampleNoteListBox
	{
		BItems::ItemList notes;
		std::array<std::string, 12> noteSymbols = {"C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B"};
		for (int i = 0; i < 128; ++i)
		{
			notes.push_back ({double (i), noteSymbols [i % 12] + std::to_string (int (i / 12) - 1)});
		}
		sampleNoteListBox = BWidgets::PopupListBox (570, 170, 60, 20, 60, 360, "menu", notes, 69);
	}

	// lfoWidgets
	for (int i = 0; i < NR_LFOS; ++i)
	{
		lfoTabIcons[i] = BWidgets::ImageIcon (i * 42, 0, 40, 20, (i == 0 ? "activetab" : "tab"), pluginPath + "inc/LFO" + std::to_string (i + 1) + ".png");

		lfoWidgets[i].container = BWidgets::Widget (0, 20, 360, 200, "widget");
		lfoWidgets[i].frequencyDial = Dial (295, 5, 45, 45, "dial", 1, 0.1, 10, 0, "%2.2f", "Hz");
		lfoWidgets[i].phaseDial = Dial (295, 70, 45, 45, "dial", 0, -1, 1, 0, "%1.2f");
		lfoWidgets[i].ampDial = Dial (295, 135, 45, 45, "dial", 1, 0, 1, 0, "%1.2f");
		lfoWidgets[i].displayWidget = CurveChart (10, 10, 260, 150, "shape");
		lfoWidgets[i].horizonWidget = VLine (0, 0, 2, 150, "widget");
		lfoWidgets[i].typeListBox = BWidgets::PopupListBox
		(
			10, 170, 260, 20, 260, 180, "menu",
			BItems::ItemList
			({
				{0, "Sine"},
				{1, "Triangle"},
				{2, "Triangle (2 bits)"},
				{3, "Triangle (3 bits)"},
				{4, "Triangle (4 bits)"},
				{5, "Sawtooth"},
				{6, "Sawtooth (2 bits)"},
				{7, "Sawtooth (3 bits)"},
				{8, "Sawtooth (4 bits)"},
				{9, "Reverse sawtooth"},
				{10, "Reverse sawtooth (2 bits)"},
				{11, "Reverse sawtooth (3 bits)"},
				{12, "Reverse sawtooth (4 bits)"},
				{13, "Square"}
			}),
			0
		);
	}

	// seqWidgets
	for (int i = 0; i < NR_SEQS; ++i)
	{
		seqTabIcons[i] = BWidgets::ImageIcon (i * 42, 0, 40, 20, (i == 0 ? "activetab" : "tab"), pluginPath + "inc/Seq" + std::to_string (i + 1) + ".png");

		seqWidgets[i].container = BWidgets::Widget (0, 20, 360, 200, "widget");
		seqWidgets[i].frequencyDial = Dial (295, 5, 45, 45, "dial", 1, 0.1, 10, 0, "%2.2f", "Hz");
		seqWidgets[i].phaseDial = Dial (295, 70, 45, 45, "dial", 0, -1, 1, 0, "%1.2f");
		seqWidgets[i].sizeSlider = HSlider (300, 135, 50, 20, "dial", 1, 1, 16, 1, "%2.0f");
		seqWidgets[i].sequencerWidget = SequencerWidget<NR_SEQ_STEPS> (10, 10, 260, 150, "shape");
	}

	// rndWidgets
	for (int i = 0; i < NR_RNDS; ++i)
	{
		rndTabIcons[i] = BWidgets::ImageIcon (i * 42, 0, 40, 20, (i == 0 ? "activetab" : "tab"), pluginPath + "inc/Rnd" + std::to_string (i + 1) + ".png");

		rndWidgets[i].container = BWidgets::Widget (0, 20, 360, 70, "widget");
		rndWidgets[i].minDial = Dial (80, 5, 45, 45, "dial", 0, 0.0, 1.0, 0, "%1.2f");
		rndWidgets[i].maxDial = Dial (235, 5, 45, 45, "dial", 0, 0.0, 1.0, 0, "%1.2f");
	}

	// envWidgets
	for (int i = 0; i < NR_ENVS; ++i)
	{
		envTabIcons[i] = BWidgets::ImageIcon (i * 42, 0, 40, 20, (i == 0 ? "activetab" : "tab"), pluginPath + "inc/Env" + std::to_string (i + 1) + ".png");

		envWidgets[i].container = BWidgets::Widget (0, 20, 360, 170, "widget");
		envWidgets[i].attackSlider = HSlider (300, 15, 50, 20, "dial", 0.02, 0, 4, 0, "%1.3f s", [] (double x) {return pow (x, 1.0 / 3.0);}, [] (double x) {return pow (x, 3.0);});
		envWidgets[i].decaySlider = HSlider (300, 55, 50, 20, "dial", 0.02, 0, 4, 0, "%1.3f s", [] (double x) {return pow (x, 1.0 / 3.0);}, [] (double x) {return pow (x, 3.0);});
		envWidgets[i].sustainSlider = HSlider (300, 95, 50, 20, "dial", 1, 0, 1, 0, "%1.3f");
		envWidgets[i].releaseSlider = HSlider (300, 135, 50, 20, "dial", 0.02, 0, 4, 0, "%1.3f s", [] (double x) {return pow (x, 1.0 / 3.0);}, [] (double x) {return pow (x, 3.0);});
		envWidgets[i].displayWidget = CurveChart (10, 10, 260, 150, "shape");
		for (int j = 0; j < MAXVOICES; ++j) envWidgets[i].horizonWidgets[j] = VLine (0, 0, 2, 150, "widget");
	}

	// processWidgets
	processWidgets[SYNTH_LEVEL].rangeDial = RangeDial (560, 390, 60, 60, "dial", 1.0, 0.0, 2.0, 0, "%1.2f");
	BItems::ItemList envItemList = BItems::ItemList
	({
		{1, "Env1"},
		{2, "Env2"},
		{3, "Env3"},
		{4, "Env4"}
	});
	BItems::ItemList modItemList = BItems::ItemList
	({
		{0, "None"},
		{1, "Env1"},
		{2, "Env2"},
		{3, "Env3"},
		{4, "Env4"},
		{5, "LFO1"},
		{6, "LFO2"},
		{7, "LFO3"},
		{8, "LFO4"},
		{9, "Seq1"},
		{10, "Seq2"},
		{11, "Seq3"},
		{12, "Seq4"},
		{13, "Rnd1"},
		{14, "Rnd2"},
		{15, "Rnd3"},
		{16, "Rnd4"}
	});

	for (int i = 0; i < NR_SYNTH_PROPERTIES; ++i)
	{
		processWidgets[i].modContainer = BWidgets::Widget (540 - i * 90, 455, 100, 20, "widget");
		processWidgets[i].miniMaxiButton = MiniMaximizeButton (75, 5, 10, 10, "redbutton");
		processWidgets[i].modBox = BWidgets::Widget (0, 20, 100, 160, "box");
		processWidgets[i].modLabel = BWidgets::Label (10, 10, 80, 20, "ctlabel", "Modulation");
		for (int j = 0; j < NR_PROPERTY_MODULATORS; ++j)
		{
			if ((i == SYNTH_LEVEL) && (j == 0)) processWidgets[i].modListBoxes[j] = BWidgets::PopupListBox (10, 40 + j * 30, 80, 20, 80, 120, "menu", envItemList, 0);
			else processWidgets[i].modListBoxes[j] = BWidgets::PopupListBox (10, 40 + j * 30, 80, 20, 80, 120, "menu", modItemList, 0);
		}
	}

	// Shape widgets
	for (int i = 0; i < USER_SHAPES + NR_USER_SHAPES; ++i) shapeWidgets[i] = ShapeWidget (8, 58, 344, 144, "shape");
	for (int i = 1; i < NR_TOOLS; ++i) shapeToolButtons[i - 1] = HaloToggleButton (8 + (i - 1) * 30, 210, 24, 24, "halobutton", toolLabels[i - 1]);
	for (int i = 0; i < NR_EDITS; ++i) shapeEditButtons[i] = HaloButton (168 + (i > 2 ? 10 : 0) + i * 30, 210, 24, 24, "halobutton", editLabels[i]);

	// grainWidgets
	grainWidgets[GRAIN_SIZE].rangeDial = RangeDial (10, 30, 60, 60, "dial", 50, 1, 1000, 0, "%4.0f", "ms", [] (double x) {return pow (x, 1.0 / 3.0);}, [] (double x) {return pow (x, 3.0);});
	grainWidgets[GRAIN_RATE].rangeDial = RangeDial (100, 30, 60, 60, "dial", 0.5, 0.1, 100, 0, "%2.2f", "", [] (double x) {return pow (x, 1.0 / 3.0);}, [] (double x) {return pow (x, 3.0);});
	grainWidgets[GRAIN_PHASE].rangeDial = RangeDial (190, 30, 60, 60, "dial", 0, -1, 1, 0, "%1.2f");
	grainWidgets[GRAIN_DRIVE].rangeDial = RangeDial (280, 30, 60, 60, "dial", 1, -1, 1, 0, "%1.2f");
	grainWidgets[GRAIN_LEVEL].rangeDial = RangeDial (10, 120, 60, 60, "dial", 1, 0, 2, 0, "%1.2f");
	grainWidgets[GRAIN_PAN].rangeDial = RangeDial (100, 120, 60, 60, "dial", 0, -1, 1, 0, "%1.2f");
	grainWidgets[GRAIN_TUNE].rangeDial = RangeDial (190, 120, 60, 60, "dial", 0, -12, 12, 1, "%2.0f", "st");
	grainWidgets[GRAIN_FINE].rangeDial = RangeDial (280, 120, 60, 60, "dial", 0, -100, 100, 0, "%3.1f", "ct");
	for (int i = 0; i < NR_GRAIN_PROPERTIES; ++i)
	{
		grainWidgets[i].modContainer = BWidgets::Widget (-10 + (i % 4) * 90, 95 + int (i / 4) * 90, 100, 20, "widget");
		grainWidgets[i].miniMaxiButton = MiniMaximizeButton (75, 5, 10, 10, "redbutton");
		grainWidgets[i].modBox = BWidgets::Widget (0, 20, 100, 160, "box");
		grainWidgets[i].modLabel = BWidgets::Label (10, 10, 80, 20, "ctlabel", "Modulation");
		for (int j = 0; j < NR_PROPERTY_MODULATORS; ++j)
		{
			grainWidgets[i].modListBoxes[j] = BWidgets::PopupListBox (10, 40 + j * 30, 80, 20, 80, 120, "menu", modItemList, 0);
		}
	}

	// Link controllerWidgets
	controllerWidgets[MAX_VOICES] = (BWidgets::ValueWidget*) &globalMaxVoicesSlider;
	controllerWidgets[MAX_GRAINS_PER_VOICE] = (BWidgets::ValueWidget*) &globalMaxGrainsSlider;

	controllerWidgets[SAMPLE_START] = (BWidgets::ValueWidget*) &sampleStartLine;
	controllerWidgets[SAMPLE_END] = (BWidgets::ValueWidget*) &sampleEndLine;
	controllerWidgets[SAMPLE_FREQ] = (BWidgets::ValueWidget*) &sampleFrequencySelect;

	controllerWidgets[PATTERN_SIZE] = (BWidgets::ValueWidget*) &patternSizeSelect;
	controllerWidgets[PATTERN_TYPE] = (BWidgets::ValueWidget*) &patternTypeListBox;

	for (int i = 0; i < NR_SYNTH_PROPERTIES; ++i)
	{
		controllerWidgets[SYNTH + i * PROPERTIES_SIZE + PROPERTY_VALUE_START] = (BWidgets::ValueWidget*) &processWidgets[i].rangeDial;
		controllerWidgets[SYNTH + i * PROPERTIES_SIZE + PROPERTY_VALUE_END] = (BWidgets::ValueWidget*) &processWidgets[i].rangeDial;
		for (int j = 0; j < NR_PROPERTY_MODULATORS; ++j)
		{
			controllerWidgets[SYNTH + i * PROPERTIES_SIZE + PROPERTY_MODULATORS + j] = (BWidgets::ValueWidget*) &processWidgets[i].modListBoxes[j];
		}
	}

	for (int i = 0; i < NR_GRAIN_PROPERTIES; ++i)
	{
		controllerWidgets[PROPERTIES + i * PROPERTIES_SIZE + PROPERTY_VALUE_START] = (BWidgets::ValueWidget*) &grainWidgets[i].rangeDial;
		controllerWidgets[PROPERTIES + i * PROPERTIES_SIZE + PROPERTY_VALUE_END] = (BWidgets::ValueWidget*) &grainWidgets[i].rangeDial;
		for (int j = 0; j < NR_PROPERTY_MODULATORS; ++j)
		{
			controllerWidgets[PROPERTIES + i * PROPERTIES_SIZE + PROPERTY_MODULATORS + j] = (BWidgets::ValueWidget*) &grainWidgets[i].modListBoxes[j];
		}
	}

	controllerWidgets[GRAIN_SHAPE] = (BWidgets::ValueWidget*) &shapeListBox;

	for (int i = 0; i < NR_LFOS; ++i)
	{
		controllerWidgets[LFOS + i * LFO_SIZE + LFO_TYPE] = (BWidgets::ValueWidget*) &lfoWidgets[i].typeListBox;
		controllerWidgets[LFOS + i * LFO_SIZE + LFO_FREQ] = (BWidgets::ValueWidget*) &lfoWidgets[i].frequencyDial;
		controllerWidgets[LFOS + i * LFO_SIZE + LFO_PHASE] = (BWidgets::ValueWidget*) &lfoWidgets[i].phaseDial;
		controllerWidgets[LFOS + i * LFO_SIZE + LFO_AMP] = (BWidgets::ValueWidget*) &lfoWidgets[i].ampDial;
	}

	for (int i = 0; i < NR_SEQS; ++i)
	{
		controllerWidgets[SEQS + i * SEQ_SIZE + SEQ_CHS] = (BWidgets::ValueWidget*) &seqWidgets[i].sizeSlider;
		controllerWidgets[SEQS + i * SEQ_SIZE + SEQ_FREQ] = (BWidgets::ValueWidget*) &seqWidgets[i].frequencyDial;
		controllerWidgets[SEQS + i * SEQ_SIZE + SEQ_PHASE] = (BWidgets::ValueWidget*) &seqWidgets[i].phaseDial;
		for (int j = 0; j < NR_SEQ_STEPS; ++j)
		{
			controllerWidgets[SEQS + i * SEQ_SIZE + SEQ_STEPS + j] = (BWidgets::ValueWidget*) &seqWidgets[i].sequencerWidget.sliders[j];
		}
	}

	for (int i = 0; i < NR_RNDS; ++i)
	{
		controllerWidgets[RNDS + i * RND_SIZE + RND_MIN] = (BWidgets::ValueWidget*) &rndWidgets[i].minDial;
		controllerWidgets[RNDS + i * RND_SIZE + RND_MAX] = (BWidgets::ValueWidget*) &rndWidgets[i].maxDial;
	}

	for (int i = 0; i < NR_ENVS; ++i)
	{
		controllerWidgets[ENVS + i * ENV_SIZE + ENV_ATTACK] = (BWidgets::ValueWidget*) &envWidgets[i].attackSlider;
		controllerWidgets[ENVS + i * ENV_SIZE + ENV_DECAY] = (BWidgets::ValueWidget*) &envWidgets[i].decaySlider;
		controllerWidgets[ENVS + i * ENV_SIZE + ENV_SUSTAIN] = (BWidgets::ValueWidget*) &envWidgets[i].sustainSlider;
		controllerWidgets[ENVS + i * ENV_SIZE + ENV_RELEASE] = (BWidgets::ValueWidget*) &envWidgets[i].releaseSlider;
	}

	controllerWidgets[PATTERN_TYPE] = (BWidgets::ValueWidget*) &patternTypeListBox;
	controllerWidgets[PATTERN_TYPE] = (BWidgets::ValueWidget*) &patternTypeListBox;
	controllerWidgets[PATTERN_TYPE] = (BWidgets::ValueWidget*) &patternTypeListBox;
	controllerWidgets[PATTERN_TYPE] = (BWidgets::ValueWidget*) &patternTypeListBox;

	// Set callback functions
	for (BWidgets::Label& l : patchInput) l.setCallbackFunction (BEvents::MESSAGE_EVENT, textChangedCallback);
	for (int i = 0; i < MAXCONTROLLERS; ++i) controllerWidgets[i]->setCallbackFunction (BEvents::VALUE_CHANGED_EVENT, valueChangedCallback);

	for (int i = 0; i < NR_LFOS; ++i) lfoTabIcons[i].setCallbackFunction (BEvents::BUTTON_PRESS_EVENT, lfoTabClickedCallback);
	for (int i = 0; i < NR_SEQS; ++i) seqTabIcons[i].setCallbackFunction (BEvents::BUTTON_PRESS_EVENT, seqTabClickedCallback);
	for (int i = 0; i < NR_RNDS; ++i) rndTabIcons[i].setCallbackFunction (BEvents::BUTTON_PRESS_EVENT, rndTabClickedCallback);
	for (int i = 0; i < NR_ENVS; ++i) envTabIcons[i].setCallbackFunction (BEvents::BUTTON_PRESS_EVENT, envTabClickedCallback);

	sampleStartLine.setCallbackFunction (BEvents::POINTER_DRAG_EVENT, sliderDraggedCallback);
	sampleEndLine.setCallbackFunction (BEvents::POINTER_DRAG_EVENT, sliderDraggedCallback);
	sampleNoteListBox.setCallbackFunction (BEvents::VALUE_CHANGED_EVENT, sampleNoteChangedCallback);
	sampleLoadButton.setCallbackFunction(BEvents::BUTTON_PRESS_EVENT, sampleLoadButtonClickedCallback);
	sampleNameLabel.setCallbackFunction(BEvents::BUTTON_PRESS_EVENT, sampleLoadButtonClickedCallback);
	samplePlayButton.setCallbackFunction (BEvents::VALUE_CHANGED_EVENT, samplePlayClickedCallback);
	sampleSelectionPlayButton.setCallbackFunction (BEvents::VALUE_CHANGED_EVENT, samplePlayClickedCallback);

	for (int i = 1; i < NR_TOOLS; ++i) shapeToolButtons[i - 1].setCallbackFunction (BEvents::BUTTON_PRESS_EVENT, shapeToolClickedCallback);
	for (int i = 0; i < NR_EDITS; ++i) shapeEditButtons[i].setCallbackFunction (BEvents::BUTTON_PRESS_EVENT, shapeEditClickedCallback);
	for (int i = USER_SHAPES; i < USER_SHAPES + NR_USER_SHAPES; ++i) shapeWidgets[i].setCallbackFunction (BEvents::VALUE_CHANGED_EVENT, shapeChangedCallback);

	patternWidget.setCallbackFunction(BEvents::BUTTON_PRESS_EVENT, patternClickedCallback);
	patternWidget.setCallbackFunction(BEvents::WHEEL_SCROLL_EVENT, patternClickedCallback);

	pianoRoll.setCallbackFunction(BEvents::MESSAGE_EVENT, pianoCallback);

	helpButton.setCallbackFunction(BEvents::BUTTON_PRESS_EVENT, helpButtonClickedCallback);
	ytButton.setCallbackFunction(BEvents::BUTTON_PRESS_EVENT, ytButtonClickedCallback);

	// Configure widgets
	globalMiniMaxiButton.setParentMinimizeArea ({1060, 110, 360, 25});
	globalMiniMaxiButton.setParentMaximizeArea ({1060, 110, 360, 90});

	patchMiniMaxiButton.setParentMinimizeArea ({1060, 155, 360, 25});
	patchMiniMaxiButton.setParentMaximizeArea ({1060, 155, 360, 190});
	for (BWidgets::Label& l : patchInput) l.setEditable (true);
	time_t t = time(NULL);
  	tm tm = *localtime(&t);
    	patchInput[PATCH_DATE].setText (std::to_string (tm.tm_year + 1900) + "-" + std::to_string (tm.tm_mon) + "-" + std::to_string (tm.tm_mday));

	for (int i = 0; i < NR_LFOS; ++i)
	{
		if (i >= 1) lfoWidgets[i].container.hide();
		lfoWidgets[i].displayWidget.setPoints (makeLfo (SINE_LFO, 0.0, 1.0));
		lfoWidgets[i].displayWidget.setScaleParameters (0.05, 0, 1.1);
		lfoWidgets[i].displayWidget.setFill (true);
		lfoWidgets[i].frequencyDial.setHardChangeable (false);
		lfoWidgets[i].phaseDial.setHardChangeable (false);
		lfoWidgets[i].ampDial.setHardChangeable (false);
	}

	for (int i = 0; i < NR_SEQS; ++i)
	{
		if (i >= 1) seqWidgets[i].container.hide();
		seqWidgets[i].sequencerWidget.setScaleParameters (0.05, 0, 1.1);
		seqWidgets[i].frequencyDial.setHardChangeable (false);
		seqWidgets[i].phaseDial.setHardChangeable (false);
	}

	for (int i = 0; i < NR_RNDS; ++i)
	{
		if (i >= 1) rndWidgets[i].container.hide();
		rndWidgets[i].minDial.setHardChangeable (false);
		rndWidgets[i].maxDial.setHardChangeable (false);
	}

	for (int i = 0; i < NR_ENVS; ++i)
	{
		if (i >= 1) envWidgets[i].container.hide();
		envWidgets[i].displayWidget.setPoints (makeEnv (0, 0, 0, 0));
		envWidgets[i].displayWidget.setScaleParameters (0.05, 0, 1.1);
		envWidgets[i].displayWidget.setFill (true);
		envWidgets[i].attackSlider.setHardChangeable (false);
		envWidgets[i].decaySlider.setHardChangeable (false);
		envWidgets[i].sustainSlider.setHardChangeable (false);
		envWidgets[i].releaseSlider.setHardChangeable (false);
		for (VLine& w : envWidgets[i].horizonWidgets) w.hide();
	}

	sampleWidget.setScaleParameters (0.5, 0, 2.2);
	sampleWidget.setMinorXSteps (10.0);
	sampleWidget.setMajorXSteps (60.0);
	sampleWidget.setLineWidth (1.0);
	sampleStartLine.setDraggable (true);
	sampleEndLine.setDraggable (true);

	patternWidget.setPattern (RISING_PATTERN);
	patternWidget.setSteps (16);
	patternWidget.setRows (16);
	patternWidget.setMergeable (BEvents::POINTER_DRAG_EVENT, false);

	patternSelectionWidget.setScaleVisible (false);
	patternSelectionWidget.setScaleParameters (0.5, 0, 2.2);
	patternSelectionWidget.setLineWidth (1.0);
	patternSelectionWidget.setClickable (false);
	patternSelectionWidget.setScrollable (false);

	for (int i = 0; i < NR_SYNTH_PROPERTIES; ++i)
	{
		processWidgets[i].rangeDial.setMergeable (BEvents::VALUE_CHANGED_EVENT, true);
		processWidgets[i].miniMaxiButton.setParentMinimizeArea ({540.0 - i * 90.0, 455, 100, 20});
		processWidgets[i].miniMaxiButton.setParentMaximizeArea ({540.0 - i * 90.0, 260, 100, 215});
		processWidgets[i].miniMaxiButton.moveTo ({75, 5}, {75, 200});
		processWidgets[i].modContainer.setStacking (BWidgets::STACKING_OVERSIZE);
	}

	shapeToolButtons[POINT_NODE_TOOL - 1].setValue (1.0);
	for (int i = 0; i < USER_SHAPES + NR_USER_SHAPES; ++i)
	{
		shapeWidgets[i].setMergeable (BEvents::POINTER_DRAG_EVENT, false);
		shapeWidgets[i].setTool (ToolType::POINT_NODE_TOOL);
		shapeWidgets[i].setDefaultShape ();
		if (i >= 1) shapeWidgets[i].hide();
		shapeWidgets[i].setScaleParameters (0.05, 0, 1.1);
	}
	shapeWidgets[SINE_SHAPE].insertNode ({CORNER_NODE, {0.001, 0.0}, {0.0, 0.0}, {0.1821, 0.0}});
	shapeWidgets[SINE_SHAPE].insertNode ({CORNER_NODE, {0.5, 1.0}, {-0.1821, 0.0}, {0.1821, 0.0}});
	shapeWidgets[SINE_SHAPE].insertNode ({CORNER_NODE, {0.999, 0.0}, {-0.1821, 0.0}, {0.0, 0.0}});
	shapeWidgets[HALF_SINE_SHAPE].insertNode ({CORNER_NODE, {0.001, 0.0}, {0.0, 0.0}, {0.1821, 0.5}});
	shapeWidgets[HALF_SINE_SHAPE].insertNode ({CORNER_NODE, {0.5, 1.0}, {-0.1821, 0.0}, {0.1821, 0.0}});
	shapeWidgets[HALF_SINE_SHAPE].insertNode ({CORNER_NODE, {0.999, 0.0}, {-0.1821, 0.5}, {0.0, 0.0}});
	shapeWidgets[TRIANGLE_SHAPE].insertNode ({POINT_NODE, {0.5, 1.0}, {0.0, 0.0}, {0.0, 0.0}});
	shapeWidgets[TRAPEZ_SHAPE].insertNode ({POINT_NODE, {0.25, 1.0}, {0.0, 0.0}, {0.0, 0.0}});
	shapeWidgets[TRAPEZ_SHAPE].insertNode ({POINT_NODE, {0.75, 1.0}, {0.0, 0.0}, {0.0, 0.0}});

	for (int i = 0; i < NR_GRAIN_PROPERTIES; ++i)
	{
		grainWidgets[i].miniMaxiButton.setParentMinimizeArea ({-10.0 + (i % 4) * 90.0, 95.0 + int (i / 4) * 90.0, 100, 20});
		grainWidgets[i].miniMaxiButton.setParentMaximizeArea ({-10.0 + (i % 4) * 90.0, 95.0 + int (i / 4) * 90.0, 100, 185});
		grainWidgets[i].modContainer.setStacking (BWidgets::STACKING_OVERSIZE);
	}

	// Load background & apply theme
	bgImageSurface = cairo_image_surface_create_from_png ((pluginPath + BG_FILE).c_str());
	widgetBg.loadFillFromCairoSurface (bgImageSurface);
	applyTheme (theme);

	// Pack widgets
	globalContainer.add (globalTitleIcon);
	globalContainer.add (globalMiniMaxiButton);
	globalContainer.add (globalMaxVoicesLabel);
	globalContainer.add (globalMaxGrainsLabel);
	globalContainer.add (globalMaxVoicesSlider);
	globalContainer.add (globalMaxGrainsSlider);

	patchContainer.add (patchTitleIcon);
	patchContainer.add (patchMiniMaxiButton);
	for (BWidgets::Label& l : patchLabel) patchContainer.add (l);
	for (BWidgets::Label& l : patchInput) patchContainer.add (l);

	for (int i = 0; i < NR_LFOS; ++i)
	{
		for (int j = 0; j < LFO_SIZE; ++j) lfoWidgets[i].container.add (*controllerWidgets[LFOS + i * LFO_SIZE + j]);
		lfoWidgets[i].container.add (lfoWidgets[i].displayWidget);
		lfoWidgets[i].displayWidget.add (lfoWidgets[i].horizonWidget);

		lfoContainer.add (lfoWidgets[i].container);
		lfoContainer.add (lfoTabIcons[i]);
	}

	for (int i = 0; i < NR_SEQS; ++i)
	{
		seqWidgets[i].container.add (seqWidgets[i].frequencyDial);
		seqWidgets[i].container.add (seqWidgets[i].phaseDial);
		seqWidgets[i].container.add (seqWidgets[i].sizeSlider);
		seqWidgets[i].container.add (seqWidgets[i].sequencerWidget);

		seqContainer.add (seqWidgets[i].container);
		seqContainer.add (seqTabIcons[i]);
	}

	for (int i = 0; i < NR_RNDS; ++i)
	{
		rndWidgets[i].container.add (rndWidgets[i].minDial);
		rndWidgets[i].container.add (rndWidgets[i].maxDial);

		rndContainer.add (rndWidgets[i].container);
		rndContainer.add (rndTabIcons[i]);
	}

	for (int i = 0; i < NR_ENVS; ++i)
	{
		for (int j = 0; j < ENV_SIZE; ++j) envWidgets[i].container.add (*controllerWidgets[ENVS + i * ENV_SIZE + j]);
		envWidgets[i].container.add (envWidgets[i].displayWidget);

		envContainer.add (envWidgets[i].container);
		envContainer.add (envTabIcons[i]);
		for (int j = 0; j < MAXVOICES; ++j) envWidgets[i].displayWidget.add (envWidgets[i].horizonWidgets[j]);
	}

	sampleWidget.add (sampleSizeLabel);
	sampleWidget.add (sampleSelectionSizeLabel);
	sampleWidget.add (samplePlayButton);
	sampleWidget.add (sampleSelectionPlayButton);
	sampleContainer.add (sampleWidget);
	sampleContainer.add (sampleLoadButton);
	sampleContainer.add (sampleNameLabel);
	sampleContainer.add (sampleStartScreen);
	sampleContainer.add (sampleEndScreen);
	sampleContainer.add (sampleStartLine);
	sampleContainer.add (sampleEndLine);
	sampleContainer.add (sampleFrequencyDetectionButton);
	sampleContainer.add (sampleFrequencySelect);
	sampleContainer.add (sampleNoteListBox);

	patternContainer.add (patternWidget);
	patternContainer.add (patternSelectionWidget);
	patternContainer.add (patternSizeSelect);
	patternContainer.add (patternTypeListBox);

	for (int i = NR_SYNTH_PROPERTIES - 1; i >= 0; --i)
	{
		patternContainer.add (processWidgets[i].rangeDial);
		processWidgets[i].modContainer.add (processWidgets[i].miniMaxiButton);
		processWidgets[i].modBox.add (processWidgets[i].modLabel);
		for (int j = 0; j < NR_PROPERTY_MODULATORS; ++j) processWidgets[i].modBox.add (processWidgets[i].modListBoxes[j]);
		processWidgets[i].modContainer.add (processWidgets[i].modBox);
		patternContainer.add (processWidgets[i].modContainer);
	}

	for (int i = 0; i < USER_SHAPES + NR_USER_SHAPES; ++i) shapeContainer.add (shapeWidgets[i]);
	for (int i = 1; i < NR_TOOLS; ++i) shapeContainer.add (shapeToolButtons[i - 1]);
	for (int i = 0; i < NR_EDITS; ++i) shapeContainer.add (shapeEditButtons[i]);
	shapeContainer.add (shapeScreen);
	shapeContainer.add (shapeListBox);

	for (int i = NR_GRAIN_PROPERTIES - 1; i >= 0; --i)
	{
		grainContainer.add (grainWidgets[i].rangeDial);
		grainWidgets[i].modContainer.add (grainWidgets[i].miniMaxiButton);
		grainWidgets[i].modBox.add (grainWidgets[i].modLabel);
		for (int j = 0; j < NR_PROPERTY_MODULATORS; ++j) grainWidgets[i].modBox.add (grainWidgets[i].modListBoxes[j]);
		grainWidgets[i].modContainer.add (grainWidgets[i].modBox);
		grainContainer.add (grainWidgets[i].modContainer);
	}

	mContainer.add (grainDisplay);
	mContainer.add (grainContainer);
	mContainer.add (shapeContainer);
	mContainer.add (pianoRoll);
	mContainer.add (patternContainer);
	mContainer.add (sampleContainer);
	mContainer.add (envContainer);
	mContainer.add (rndContainer);
	mContainer.add (seqContainer);
	mContainer.add (lfoContainer);
	mContainer.add (patchContainer);
	mContainer.add (globalContainer);
	mContainer.add (helpButton);
	mContainer.add (ytButton);
	add (mContainer);

	// Post exposure configurations
	for (int i = USER_SHAPES; i < USER_SHAPES + NR_USER_SHAPES; ++i) shapeWidgets[i].setValueEnabled (true);

	//Scan host features for URID map
	LV2_URID_Map* map = NULL;
	for (int i = 0; features[i]; ++i)
	{
		if (strcmp(features[i]->URI, LV2_URID__map) == 0)
		{
			map = (LV2_URID_Map*) features[i]->data;
		}
	}
	if (!map) throw std::invalid_argument ("Host does not support urid:map");

	//Map URIS
	getURIs (map, &uris);

	// Initialize forge
	lv2_atom_forge_init (&forge, map);
}

BHarvestrGUI::~BHarvestrGUI ()
{
	if (sampleFileChooser) delete sampleFileChooser;
	sendUiStatus (false);
}

void BHarvestrGUI::port_event(uint32_t port, uint32_t buffer_size,
	uint32_t format, const void* buffer)
{
	// Notify port
	if ((format == uris.atom_eventTransfer) && (port == NOTIFY))
	{
		const LV2_Atom* atom = (const LV2_Atom*) buffer;
		if ((atom->type == uris.atom_Blank) || (atom->type == uris.atom_Object))
		{
			const LV2_Atom_Object* obj = (const LV2_Atom_Object*) atom;

			// Playback stop notification from host => untoggle play buttons
			if (obj->body.otype == uris.bharvestr_sampleStop) samplePlayButton.setValue (0.0);
			if (obj->body.otype == uris.bharvestr_selectionStop) sampleSelectionPlayButton.setValue (0.0);

			// Pattern notification
			if (obj->body.otype == uris.bharvestr_patternEvent)
			{
				LV2_Atom *oPr = NULL, *oPs = NULL, *oPat = NULL;
				lv2_atom_object_get
				(
					obj,
					uris.bharvestr_patternRows, &oPr,
					uris.bharvestr_patternSteps, &oPs,
					uris.bharvestr_pattern, &oPat,
					NULL
				);

				if (oPr && (oPr->type == uris.atom_Int))
				{
					const int rows = ((const LV2_Atom_Int*)oPr)->body;
					patternWidget.setRows (rows);
				}

				if (oPs && (oPs->type == uris.atom_Int))
				{
					const int steps = ((const LV2_Atom_Int*)oPs)->body;
					patternWidget.setSteps (steps);
				}

				if (oPat && (oPat->type == uris.atom_Vector))
				{
					const LV2_Atom_Vector* vec = (const LV2_Atom_Vector*) oPat;
					if (vec->body.child_type == uris.atom_Int)
					{
						const int32_t size = (int32_t) ((oPat->size - sizeof(LV2_Atom_Vector_Body)) / sizeof (int));
						int* data = (int*) (&vec->body + 1);
						std::vector<int> values = {};
						for (int i = 0; i < size; ++i) values.push_back (data[i]);
						patternWidget.hide();
						patternWidget.setPattern (USER_PATTERN);
						patternWidget.setValues (values);
						patternWidget.show();
					}
				}
			}

			// Path notification
			else if (obj->body.otype == uris.bharvestr_sampleEvent)
			{
				const LV2_Atom* data = NULL;
				lv2_atom_object_get(obj, uris.bharvestr_samplePath, &data, 0);
				if (data && (data->type == uris.atom_Path))
				{
					sampleNameLabel.setText ((const char*)LV2_ATOM_BODY_CONST(data));
					// TODO Split to path and file name
				}
			}

			// Display notifications
			else if (obj->body.otype == uris.bharvestr_displayEvent)
			{
				LV2_Atom *oSize = NULL, *oSample = NULL, *oSelection = NULL;
				lv2_atom_object_get
				(
					obj,
					uris.bharvestr_sampleSize, &oSize,
					uris.bharvestr_sampleDisplayData, &oSample,
					uris.bharvestr_selectionDisplayData, &oSelection,
					NULL
				);

				// Sample notifications
				if (oSize && (oSize->type == uris.atom_Float) && oSample && (oSample->type == uris.atom_Vector))
				{
					sampleSize = ((const LV2_Atom_Float*)oSize)->body;
					const LV2_Atom_Vector* vec = (const LV2_Atom_Vector*) oSample;
					if (vec->body.child_type == uris.atom_Float)
					{
						int32_t size = (int32_t) ((oSample->size - sizeof(LV2_Atom_Vector_Body)) / sizeof (float));
						float* data = (float*) (&vec->body + 1);
						float max = 0;
						std::vector<BUtilities::Point> points;
						for (int i = 0; i < size; ++i)
						{
							points.push_back (BUtilities::Point (sampleSize * double (i) / double (size), data[i]));
							if (fabs (data[i]) > max) max = fabs (data[i]);
						}
						sampleWidget.hide();
						sampleWidget.setPoints (points);
						sampleWidget.setXRange (0.0, sampleSize);
						sampleWidget.setScaleParameters (0.5, 0.0, max * 2.2);
						sampleSizeLabel.setText ("Sample: " + BUtilities::to_string (sampleSize, "%5.2f s"));
						updateSelection();
						sampleWidget.show();
					}
				}

				// Selection notifications
				if (oSelection && (oSelection->type == uris.atom_Vector))
				{
					const LV2_Atom_Vector* vec = (const LV2_Atom_Vector*) oSelection;
					if (vec->body.child_type == uris.atom_Float)
					{
						int32_t size = (int32_t) ((oSelection->size - sizeof(LV2_Atom_Vector_Body)) / sizeof (float));
						float* data = (float*) (&vec->body + 1);
						float max = 0;
						std::vector<BUtilities::Point> points;
						for (int i = 0; i < size; ++i)
						{
							points.push_back (BUtilities::Point (double (i) / double (size), data[i]));
							if (fabs (data[i]) > max) max = fabs (data[i]);
						}
						patternSelectionWidget.hide();
						patternSelectionWidget.setPoints (points);
						patternSelectionWidget.setScaleParameters (0.5, 0.0, max * 2.2);
						patternSelectionWidget.show();
					}
				}
			}

			// Shape notification
			else if (obj->body.otype == uris.bharvestr_shapeEvent)
			{
				LV2_Atom *oId = NULL, *oData = NULL;
				lv2_atom_object_get
				(
					obj,
					uris.bharvestr_shapeIndex, &oId,
					uris.bharvestr_shapeData, &oData,
					NULL
				);

				if (oId && (oId->type == uris.atom_Int) && oData && (oData->type == uris.atom_Vector))
				{
					const int index = ((const LV2_Atom_Int*)oId)->body;
					if (index < MAXSHAPES)
					{
						const LV2_Atom_Vector* vec = (const LV2_Atom_Vector*) oData;
						if (vec->body.child_type == uris.atom_Float)
						{
							int32_t size = (int32_t) ((oData->size - sizeof(LV2_Atom_Vector_Body)) / (7 * sizeof (float)));
							float* data = (float*) (&vec->body + 1);

							shapeWidgets[index].setValueEnabled (false);
							shapeWidgets[index].clearShape ();
							for (int i = 0; (i < size) && (i < MAXSHAPENODES); ++i)
							{
								Node node;
								node.nodeType = NodeType (int (data[i * 7]));
								node.point.x = data[i * 7 + 1];
								node.point.y = data[i * 7 + 2];
								node.handle1.x = data[i * 7 + 3];
								node.handle1.y = data[i * 7 + 4];
								node.handle2.x = data[i * 7 + 5];
								node.handle2.y = data[i * 7 + 6];
								shapeWidgets[index].appendRawNode (node);
							}
							shapeWidgets[index].validateShape();
							shapeWidgets[index].pushToSnapshots ();
							shapeWidgets[index].update ();
							shapeWidgets[index].setValueEnabled (true);
						}
					}
				}
			}

			// Preset notification
			else if (obj->body.otype == uris.bharvestr_presetInfoEvent)
			{
				const LV2_Atom* oName = NULL, *oType = NULL, *oDate = NULL, *oCreator = NULL, *oUri = NULL, *oLicense = NULL, *oDescription = NULL ;
				lv2_atom_object_get
				(
					obj,
					uris.bharvestr_presetInfoName, &oName,
					uris.bharvestr_presetInfoType, &oType,
					uris.bharvestr_presetInfoDate, &oDate,
					uris.bharvestr_presetInfoCreator, &oCreator,
					uris.bharvestr_presetInfoURI, &oUri,
					uris.bharvestr_presetInfoLicense, &oLicense,
					uris.bharvestr_presetInfoDescription, &oDescription,
					NULL
				);

				if (oName && (oName->type == uris.atom_String)) patchInput[PATCH_NAME].setText ((const char*)LV2_ATOM_BODY_CONST(oName));
				if (oType && (oType->type == uris.atom_String)) patchInput[PATCH_TYPE].setText ((const char*)LV2_ATOM_BODY_CONST(oType));
				if (oDate && (oDate->type == uris.atom_Int))
				{
					const int date = ((const LV2_Atom_Int*)oDate)->body;
					const int day = (date % 31) + 1;
					const int mon = (int (date / 31) % 12) + 1;
					const int year = date / (31 * 12) + 1900;
					patchInput[PATCH_DATE].setText (std::to_string (year) + "-" + std::to_string (mon) + "-" + std::to_string (day));
				}
				if (oCreator && (oCreator->type == uris.atom_String)) patchInput[PATCH_CREATOR].setText ((const char*)LV2_ATOM_BODY_CONST(oCreator));
				if (oUri && (oUri->type == uris.atom_String)) patchInput[PATCH_URI].setText ((const char*)LV2_ATOM_BODY_CONST(oUri));
				if (oLicense && (oLicense->type == uris.atom_String)) patchInput[PATCH_LICENSE].setText ((const char*)LV2_ATOM_BODY_CONST(oLicense));
				if (oDescription && (oDescription->type == uris.atom_String)) patchInput[PATCH_DESCRIPTION].setText ((const char*)LV2_ATOM_BODY_CONST(oDescription));

				// Validate
				for (BWidgets::Label& l : patchInput) if (l.getText() == "") l.setText ("<empty>");
			}

			// Status notification
			else if (obj->body.otype == uris.bharvestr_statusEvent)
			{
				const LV2_Atom *oTime = NULL, *oLfo = NULL, *oSeq = NULL, *oEnv = NULL, *oGProp = NULL, *oPPos = NULL;
				lv2_atom_object_get
				(
					obj,
					uris.bharvestr_statusTime, &oTime,
					uris.bharvestr_statusLfoPositions, &oLfo,
					uris.bharvestr_statusSeqPositions, &oSeq,
					uris.bharvestr_statusEnvPositions, &oEnv,
					uris.bharvestr_statusGrainProperties, &oGProp,
					uris.bharvestr_statusPatternPositions, &oPPos,
					NULL);

				if (oTime && (oTime->type == uris.atom_Double))
				{
					// const double time = ((const LV2_Atom_Double*)oTime)->body;
					// TODO Find a use
				}

				if (oLfo && (oLfo->type == uris.atom_Vector))
				{
					const LV2_Atom_Vector* vec = (const LV2_Atom_Vector*) oLfo;
					if (vec->body.child_type == uris.atom_Float)
					{
						int32_t size = (int32_t) ((oLfo->size - sizeof(LV2_Atom_Vector_Body)) / sizeof (float));
						float* data = (float*) (&vec->body + 1);
						if (size == NR_LFOS)
						{
							for (int i = 0; i < NR_LFOS; ++i)
							{
								LfoWidget& l = lfoWidgets[i];
								double x0 = l.displayWidget.getXOffset();
								double w = l.displayWidget.getEffectiveWidth();
								l.horizonWidget.moveTo (x0 + data[i] * w - l.horizonWidget.getWidth() / 2, 0);
							}
						}
					}
				}

				if (oSeq && (oSeq->type == uris.atom_Vector))
				{
					const LV2_Atom_Vector* vec = (const LV2_Atom_Vector*) oSeq;
					if (vec->body.child_type == uris.atom_Float)
					{
						int32_t size = (int32_t) ((oSeq->size - sizeof(LV2_Atom_Vector_Body)) / sizeof (float));
						float* data = (float*) (&vec->body + 1);
						if (size == NR_SEQS)
						{
							for (int i = 0; i < NR_SEQS; ++i)
							{
								SequencerWidget<NR_SEQ_STEPS>& s = seqWidgets[i].sequencerWidget;
								size_t nr = data[i] * s.getSize();
								for (unsigned int j = 0; j < s.sliders.size(); ++j)
								{
									s.sliders[j].setState (j == nr ? BColors::ACTIVE : BColors::NORMAL);
								}
							}
						}
					}
				}

				if (oEnv && (oEnv->type == uris.atom_Vector))
				{
					const LV2_Atom_Vector* vec = (const LV2_Atom_Vector*) oEnv;
					if (vec->body.child_type == uris.atom_Float)
					{
						int32_t size = (int32_t) ((oEnv->size - sizeof(LV2_Atom_Vector_Body)) / sizeof (float) / 2);
						float* data = (float*) (&vec->body + 1);
						for (EnvWidget& env : envWidgets)
						{
							for (VLine& w : env.horizonWidgets) w.hide();

							for (int v = 0; v < size; ++v)
							{
								VLine& horizon = env.horizonWidgets[v];
								const bool released = data[2 * v];
								const float time = data[2 * v + 1];
								double susttime = env.attackSlider.getValue() +
										  env.decaySlider.getValue() +
										  4.0;
								double xrel = 0;

								if (released) xrel = (susttime + (time < 4.0 ? time : 4.0)) / 16.0;
								else xrel = (time < susttime ? time : susttime) / 16.0;

								double x0 = env.displayWidget.getXOffset();
								double w = env.displayWidget.getEffectiveWidth();
								horizon.moveTo (x0 + xrel * w - horizon.getWidth() / 2, 0);
								horizon.show();
							}
						}
					}
				}

				if (oGProp && (oGProp->type == uris.atom_Vector))
				{
					const LV2_Atom_Vector* vec = (const LV2_Atom_Vector*) oGProp;
					if (vec->body.child_type == uris.atom_Float)
					{
						int32_t size = (int32_t) ((oGProp->size - sizeof(LV2_Atom_Vector_Body)) / sizeof (float));
						float* data = (float*) (&vec->body + 1);
						if (size == NR_GRAIN_PROPERTIES)
						{
							for (int i = 0; i < NR_GRAIN_PROPERTIES; ++i)
							{
								grainWidgets[i].rangeDial.setValue (data[i]);
							}
						}
					}
				}

				if (oPPos && (oPPos->type == uris.atom_Vector))
				{
					const LV2_Atom_Vector* vec = (const LV2_Atom_Vector*) oPPos;
					if (vec->body.child_type == uris.atom_Int)
					{
						int32_t size = (int32_t) ((oPPos->size - sizeof(LV2_Atom_Vector_Body)) / sizeof (int));
						int* data = (int*) (&vec->body + 1);
						std::vector<int> sel (size);
						memcpy (&sel[0], &data[0], size * sizeof (int));
						patternWidget.setSelection (sel);
					}
				}
			}

			// Message notification
			else if (obj->body.otype == uris.bharvestr_messageEvent)
			{
				const LV2_Atom* data = NULL;
				lv2_atom_object_get(obj, uris.bharvestr_message, &data, 0);
				if (data && (data->type == uris.atom_String))
				{
					const std::string msg = (const char*)LV2_ATOM_BODY_CONST (data);
					mBox = BWidgets::MessageBox (200 * sz, 200 * sz, 400 * sz, 120 * sz, "boxlabel", "Error", msg);
					mBox.applyTheme (theme);
					add (mBox);
				}
			}
		}
	}

	// Scan remaining ports
	else if ((format == 0) && (port >= CONTROLLERS))
	{
		float* pval = (float*) buffer;
		int controllerNr = port - CONTROLLERS;

		// Grain properties
		if ((controllerNr >= PROPERTIES) && (controllerNr < PROPERTIES + NR_GRAIN_PROPERTIES * PROPERTIES_SIZE))
		{
			int property = (controllerNr - PROPERTIES) / PROPERTIES_SIZE;
			int param = (controllerNr - PROPERTIES) % PROPERTIES_SIZE;

			// Range sliders: Manually set start or end
			if (param == PROPERTY_VALUE_START) grainWidgets[property].rangeDial.setStartValue (*pval);
			else if (param == PROPERTY_VALUE_END) grainWidgets[property].rangeDial.setEndValue (*pval);
			else controllerWidgets[port - CONTROLLERS]->setValue (*pval);
		}

		// Process / Synth properties
		if ((controllerNr >= SYNTH) && (controllerNr < SYNTH + NR_SYNTH_PROPERTIES * PROPERTIES_SIZE))
		{
			int property = (controllerNr - SYNTH) / PROPERTIES_SIZE;
			int param = (controllerNr - SYNTH) % PROPERTIES_SIZE;

			// Range sliders: Manually set start or end
			if (param == PROPERTY_VALUE_START) processWidgets[property].rangeDial.setStartValue (*pval);
			else if (param == PROPERTY_VALUE_END) processWidgets[property].rangeDial.setEndValue (*pval);
			else controllerWidgets[port - CONTROLLERS]->setValue (*pval);
		}

		else controllerWidgets[port - CONTROLLERS]->setValue (*pval);
	}

}

void BHarvestrGUI::resize ()
{
	hide ();
	//Scale fonts
	ctLabelFont.setFontSize (12 * sz);
	lfLabelFont.setFontSize (12 * sz);
	smLabelFont.setFontSize (8 * sz);

	//Background
	cairo_surface_t* surface = cairo_image_surface_create (CAIRO_FORMAT_ARGB32, 1440 * sz, 880 * sz);
	cairo_t* cr = cairo_create (surface);
	cairo_scale (cr, sz, sz);
	cairo_set_source_surface(cr, bgImageSurface, 0, 0);
	cairo_paint(cr);
	widgetBg.loadFillFromCairoSurface(surface);
	cairo_destroy (cr);
	cairo_surface_destroy (surface);

	//Scale widgets
	RESIZE (mContainer, 0, 0, 1440, 880, sz);

	RESIZE (mBox, 200, 200, 400, 120, sz);

	//RESIZE (globalContainer, 20, 110, 360, 90, sz);
	RESIZE (globalTitleIcon, 100, 0, 160, 20, sz);
	RESIZE (globalMiniMaxiButton, 330, 0, 20, 20, sz);
	globalMiniMaxiButton.setParentMinimizeArea ({1060 * sz, 110 * sz, 360 * sz, 25 * sz});
	globalMiniMaxiButton.setParentMaximizeArea ({1060 * sz, 110 * sz, 360 * sz, 90 * sz});
	RESIZE (globalMaxVoicesLabel, 10, 25, 180, 20, sz);
	RESIZE (globalMaxGrainsLabel, 10, 55, 180, 20, sz);
	RESIZE (globalMaxVoicesSlider, 220, 25, 130, 20, sz);
	RESIZE (globalMaxGrainsSlider, 220, 55, 130, 20, sz);

	//RESIZE (patchContainer, 20, 155, 360, 190, sz);
	RESIZE (patchTitleIcon, 100, 0, 160, 20, sz);
	RESIZE (patchMiniMaxiButton, 330, 0, 20, 20, sz);
	patchMiniMaxiButton.setParentMinimizeArea ({1060 * sz, 155 * sz, 360 * sz, 25 * sz});
	patchMiniMaxiButton.setParentMaximizeArea ({1060 * sz, 155 * sz, 360 * sz, 190 * sz});
	RESIZE (patchLabel[PATCH_NAME], 10, 30, 70, 12, sz);
	RESIZE (patchLabel[PATCH_TYPE], 10, 50, 770, 12, sz);
	RESIZE (patchLabel[PATCH_DATE], 210, 30, 40, 12, sz);
	RESIZE (patchLabel[PATCH_CREATOR], 10, 70, 70, 12, sz);
	RESIZE (patchLabel[PATCH_URI], 10, 170, 70, 12, sz);
	RESIZE (patchLabel[PATCH_LICENSE], 10, 90, 70, 12, sz);
	RESIZE (patchLabel[PATCH_DATE], 10, 110, 270, 12, sz);
	RESIZE (patchInput[PATCH_NAME], 80, 30, 120, 12, sz);
	RESIZE (patchInput[PATCH_TYPE], 80, 50, 270, 12, sz);
	RESIZE (patchInput[PATCH_DATE], 260, 30, 90, 12, sz);
	RESIZE (patchInput[PATCH_CREATOR], 80, 70, 760, 12, sz);
	RESIZE (patchInput[PATCH_URI], 80, 170, 270, 12, sz);
	RESIZE (patchInput[PATCH_LICENSE], 80, 90, 270, 12, sz);
	RESIZE (patchInput[PATCH_DESCRIPTION], 80, 110, 270, 12, sz);

	RESIZE (lfoContainer, 20, 110, 360, 220, sz);
	for (int i = 0; i < NR_LFOS; ++i)
	{
		RESIZE (lfoTabIcons[i], i * 42, 0, 40, 20, sz);

		RESIZE (lfoWidgets[i].container, 0, 20, 360, 200, sz);
		RESIZE (lfoWidgets[i].frequencyDial, 295, 5, 45, 45, sz);
		RESIZE (lfoWidgets[i].phaseDial, 295, 70, 45, 45, sz);
		RESIZE (lfoWidgets[i].ampDial, 295, 135, 45, 45, sz);
		RESIZE (lfoWidgets[i].displayWidget, 10, 10, 260, 150, sz);
		RESIZE (lfoWidgets[i].horizonWidget, 0, 0, 2, 150, sz);
		RESIZE (lfoWidgets[i].typeListBox, 10, 170, 260, 20, sz);
		lfoWidgets[i].typeListBox.resizeListBox(BUtilities::Point (260 * sz, 180 * sz));
		lfoWidgets[i].typeListBox.moveListBox(BUtilities::Point (0, 20 * sz));
		lfoWidgets[i].typeListBox.resizeListBoxItems(BUtilities::Point (260 * sz, 20 * sz));
	}

	RESIZE (seqContainer, 20, 350, 360, 190, sz);
	for (int i = 0; i < NR_LFOS; ++i)
	{
		RESIZE (seqTabIcons[i], i * 42, 0, 40, 20, sz);

		RESIZE (seqWidgets[i].container, 0, 20, 360, 200, sz);
		RESIZE (seqWidgets[i].frequencyDial, 295, 5, 45, 45, sz);
		RESIZE (seqWidgets[i].phaseDial, 295, 70, 45, 45, sz);
		RESIZE (seqWidgets[i].sizeSlider, 300, 135, 50, 20, sz);
		RESIZE (seqWidgets[i].sequencerWidget, 10, 10, 260, 150, sz);
	}

	RESIZE (rndContainer, 20, 560, 360, 90, sz);
	for (int i = 0; i < NR_RNDS; ++i)
	{
		RESIZE (rndTabIcons[i], i * 42, 0, 40, 20, sz);

		RESIZE (rndWidgets[i].container, 0, 20, 360, 70, sz);
		RESIZE (rndWidgets[i].minDial, 80, 5, 45, 45, sz);
		RESIZE (rndWidgets[i].maxDial, 235, 5, 45, 45, sz);
	}

	RESIZE (envContainer, 20, 670, 360, 190, sz);
	for (int i = 0; i < NR_ENVS; ++i)
	{
		RESIZE (envTabIcons[i], i * 42, 0, 40, 20, sz);

		RESIZE (envWidgets[i].container, 0, 20, 360, 170, sz);
		RESIZE (envWidgets[i].attackSlider, 300, 15, 50, 20, sz);
		RESIZE (envWidgets[i].decaySlider, 300, 55, 50, 20, sz);
		RESIZE (envWidgets[i].sustainSlider, 300, 95, 50, 20, sz);
		RESIZE (envWidgets[i].releaseSlider, 300, 135, 50, 20, sz);
		RESIZE (envWidgets[i].displayWidget, 10, 10, 260, 150, sz);
	}

	RESIZE (sampleContainer, 400, 110, 640, 200, sz);
	RESIZE (sampleWidget, 10, 34, 620, 122, sz);
	RESIZE (sampleLoadButton, 10, 170, 20, 20, sz);
	RESIZE (sampleNameLabel, 40, 170, 320, 20, sz);
	sampleStartLine.resize (6 * sz, 140 * sz);
	sampleStartLine.moveTo
	(
		sampleWidget.getPosition().x - 0.5 * sampleStartLine.getWidth() + sampleStartLine.getValue() * sampleWidget.getWidth(),
		sampleWidget.getPosition().y - 1.5 * sampleStartLine.getWidth()
	);
	sampleEndLine.resize (6 * sz, 140 * sz);
	sampleEndLine.moveTo
	(
		sampleWidget.getPosition().x - 0.5 * sampleEndLine.getWidth() + sampleEndLine.getValue() * sampleWidget.getWidth(),
		sampleWidget.getPosition().y - 1.5 * sampleStartLine.getWidth()
	);
	RESIZE (sampleStartScreen, 10, 32, 0, 126, sz);
	RESIZE (sampleEndScreen, 630, 32, 0, 126, sz);
	RESIZE (sampleFrequencyDetectionButton, 400, 170, 20, 20, sz);
	RESIZE (sampleFrequencySelect, 440, 170, 110, 20, sz);
	RESIZE (sampleNoteListBox, 570, 170, 60, 20, sz);
	sampleNoteListBox.resizeListBox(BUtilities::Point (60 * sz, 360 * sz));
	sampleNoteListBox.moveListBox(BUtilities::Point (0, 20 * sz));
	sampleNoteListBox.resizeListBoxItems(BUtilities::Point (60 * sz, 20 * sz));
	if (sampleFileChooser) RESIZE ((*sampleFileChooser), 200, 120, 300, 400, sz);
	RESIZE (sampleSizeLabel, 510, 0, 80, 12, sz);
	RESIZE (sampleSelectionSizeLabel, 510, 15, 80, 12, sz);
	RESIZE (samplePlayButton, 593, 0, 12, 12, sz);
	RESIZE (sampleSelectionPlayButton, 593, 15, 12, 12, sz);

	RESIZE (patternContainer, 400, 330, 640, 460, sz);
	RESIZE (patternWidget, 10, 25, 620, 275, sz);
	RESIZE (patternSelectionWidget, 10, 25, 620, 275, sz);
	RESIZE (patternSizeSelect, 350, 310, 90, 20, sz);
	RESIZE (patternTypeListBox, 450, 310, 180, 20, sz);
	patternTypeListBox.resizeListBox(BUtilities::Point (180 * sz, 180 * sz));
	patternTypeListBox.moveListBox(BUtilities::Point (0, 20 * sz));
	patternTypeListBox.resizeListBoxItems(BUtilities::Point (180 * sz, 20 * sz));

	for (int i = 0; i < NR_SYNTH_PROPERTIES; ++i)
	{
		RESIZE (processWidgets[i].rangeDial, 560 - i * 90, 390, 60, 60, sz);
		processWidgets[i].miniMaxiButton.moveTo ({75 * sz, 5 * sz}, {75 * sz, 200 * sz});
		processWidgets[i].miniMaxiButton.resize (10 * sz, 10 * sz);
		processWidgets[i].miniMaxiButton.setParentMinimizeArea ({(540 - i * 90) * sz, 455 * sz, 100 * sz, 20 * sz});
		processWidgets[i].miniMaxiButton.setParentMaximizeArea ({(540 - i * 90) * sz, 260 * sz, 100 * sz, 215 * sz});
		RESIZE (processWidgets[i].modBox, 0, 20, 100, 160, sz);
		RESIZE (processWidgets[i].modLabel, 10, 10, 80, 20, sz);
		for (int j = 0; j < NR_PROPERTY_MODULATORS; ++j)
		{
			RESIZE (processWidgets[i].modListBoxes[j], 10, 40 + j * 30, 80, 20, sz);
			processWidgets[i].modListBoxes[j].resizeListBox(BUtilities::Point (80 * sz, 120 * sz));
			processWidgets[i].modListBoxes[j].moveListBox(BUtilities::Point (0, 20 * sz));
			processWidgets[i].modListBoxes[j].resizeListBoxItems(BUtilities::Point (80 * sz, 20 * sz));
		}
	}

	RESIZE (pianoRoll, 400, 830, 640, 50, sz);

	RESIZE (shapeContainer, 1060, 200, 360, 240, sz);
	RESIZE (shapeScreen, 2, 58, 356, 180, sz);
	RESIZE (shapeListBox, 10, 30, 120, 20, sz);
	shapeListBox.resizeListBox(BUtilities::Point (120 * sz, 200 * sz));
	shapeListBox.moveListBox(BUtilities::Point (0, 20 * sz));
	shapeListBox.resizeListBoxItems(BUtilities::Point (120 * sz, 20 * sz));
	for (int i = 0; i < USER_SHAPES + NR_USER_SHAPES; ++i) RESIZE (shapeWidgets[i], 8, 58, 344, 144, sz);
	for (int i = 1; i < NR_TOOLS; ++i) RESIZE (shapeToolButtons[i - 1], 8 + (i - 1) * 30, 210, 24, 24, sz);
	for (int i = 0; i < NR_EDITS; ++i) RESIZE (shapeEditButtons[i], 168 + (i > 2 ? 10 : 0) + i * 30, 210, 24, 24, sz);

	RESIZE (grainContainer, 1060, 460, 360, 490, sz);
	for (int i = 0; i < NR_GRAIN_PROPERTIES; ++i)
	{
		RESIZE (grainWidgets[i].rangeDial, 10 + (i % 4) * 90, 30 + int (i / 4) * 90, 60, 60, sz);
		RESIZE (grainWidgets[i].miniMaxiButton, 75, 5, 10, 10, sz);
		grainWidgets[i].miniMaxiButton.setParentMinimizeArea ({(-10.0 + (i % 4) * 90.0) * sz, (95.0 + int (i / 4) * 90.0) * sz, 100 * sz, 20 * sz});
		grainWidgets[i].miniMaxiButton.setParentMaximizeArea ({(-10.0 + (i % 4) * 90.0) * sz, (95.0 + int (i / 4) * 90.0) * sz, 100 * sz, 185 * sz});
		RESIZE (grainWidgets[i].modBox, 0, 20, 100, 160, sz);
		RESIZE (grainWidgets[i].modLabel, 10, 10, 80, 20, sz);
		for (int j = 0; j < NR_PROPERTY_MODULATORS; ++j)
		{
			RESIZE (grainWidgets[i].modListBoxes[j], 10, 40 + j * 30, 80, 20, sz);
			grainWidgets[i].modListBoxes[j].resizeListBox(BUtilities::Point (80 * sz, 120 * sz));
			grainWidgets[i].modListBoxes[j].moveListBox(BUtilities::Point (0, 20 * sz));
			grainWidgets[i].modListBoxes[j].resizeListBoxItems(BUtilities::Point (80 * sz, 20 * sz));
		}
	}
	RESIZE (grainDisplay, 1070, 700, 340, 150, sz);

	RESIZE (helpButton, 1368, 68, 24, 24, sz);
	RESIZE (ytButton, 1398, 68, 24, 24, sz);

	applyTheme (theme);
	show ();
}

void BHarvestrGUI::applyTheme (BStyles::Theme& theme)
{
	mContainer.applyTheme (theme);

	mBox.applyTheme (theme);

	globalContainer.applyTheme (theme);
	globalTitleIcon.applyTheme (theme);
	globalMiniMaxiButton.applyTheme (theme);
	globalMaxVoicesLabel.applyTheme (theme);
	globalMaxGrainsLabel.applyTheme (theme);
	globalMaxVoicesSlider.applyTheme (theme);
	globalMaxGrainsSlider.applyTheme (theme);

	patchContainer.applyTheme (theme);
	patchTitleIcon.applyTheme (theme);
	patchMiniMaxiButton.applyTheme (theme);
	for (BWidgets::Label& l : patchLabel) l.applyTheme (theme);
	for (BWidgets::Label& l : patchInput) l.applyTheme (theme);

	lfoContainer.applyTheme (theme);
	for (int i = 0; i < NR_LFOS; ++i)
	{
		lfoTabIcons[i].applyTheme (theme);

		lfoWidgets[i].container.applyTheme (theme);
		for (int j = 0; j < LFO_SIZE; ++j) controllerWidgets[LFOS + i * LFO_SIZE + j]->applyTheme (theme);
		lfoWidgets[i].displayWidget.applyTheme (theme);
		lfoWidgets[i].horizonWidget.applyTheme (theme);
	}

	seqContainer.applyTheme (theme);
	for (int i = 0; i < NR_LFOS; ++i)
	{
		seqTabIcons[i].applyTheme (theme);

		seqWidgets[i].container.applyTheme (theme);
		seqWidgets[i].frequencyDial.applyTheme (theme);
		seqWidgets[i].phaseDial.applyTheme (theme);
		seqWidgets[i].sizeSlider.applyTheme (theme);
		seqWidgets[i].sequencerWidget.applyTheme (theme);
	}

	rndContainer.applyTheme (theme);
	for (int i = 0; i < NR_RNDS; ++i)
	{
		rndTabIcons[i].applyTheme (theme);

		rndWidgets[i].container.applyTheme (theme);
		rndWidgets[i].minDial.applyTheme (theme);
		rndWidgets[i].maxDial.applyTheme (theme);
	}

	envContainer.applyTheme (theme);
	for (int i = 0; i < NR_ENVS; ++i)
	{
		envTabIcons[i].applyTheme (theme);

		envWidgets[i].container.applyTheme (theme);
		for (int j = 0; j < ENV_SIZE; ++j) controllerWidgets[ENVS + i * ENV_SIZE + j]->applyTheme (theme);
		envWidgets[i].displayWidget.applyTheme (theme);
	}

	sampleContainer.applyTheme (theme);
	sampleWidget.applyTheme (theme);
	sampleLoadButton.applyTheme (theme);
	sampleNameLabel.applyTheme (theme);
	sampleStartScreen.applyTheme (theme);
	sampleEndScreen.applyTheme (theme);
	sampleStartLine.applyTheme (theme);
	sampleEndLine.applyTheme (theme);
	sampleFrequencyDetectionButton.applyTheme (theme);
	sampleFrequencySelect.applyTheme (theme);
	sampleNoteListBox.applyTheme (theme);
	if (sampleFileChooser) sampleFileChooser->applyTheme (theme);
	sampleSizeLabel.applyTheme (theme);
	sampleSelectionSizeLabel.applyTheme (theme);
	samplePlayButton.applyTheme (theme);
	sampleSelectionPlayButton.applyTheme (theme);

	patternContainer.applyTheme (theme);
	patternWidget.applyTheme (theme);
	patternSelectionWidget.applyTheme (theme);
	patternSizeSelect.applyTheme (theme);
	patternTypeListBox.applyTheme (theme);

	for (int i = PROPERTIES; i < PROPERTIES + NR_SYNTH_PROPERTIES * PROPERTIES_SIZE; ++i) controllerWidgets[i]->applyTheme (theme);
	for (int i = 0; i < NR_SYNTH_PROPERTIES; ++i)
	{
		processWidgets[i].rangeDial.applyTheme (theme);
		processWidgets[i].modContainer.applyTheme (theme);
		processWidgets[i].miniMaxiButton.applyTheme (theme);
		processWidgets[i].modBox.applyTheme (theme);
		processWidgets[i].modLabel.applyTheme (theme);
		for (int j = 0; j < NR_PROPERTY_MODULATORS; ++j) processWidgets[i].modListBoxes[j].applyTheme (theme);
	}

	shapeContainer.applyTheme (theme);
	for (int i = 0; i < USER_SHAPES + NR_USER_SHAPES; ++i) shapeWidgets[i].applyTheme (theme);
	for (int i = 1; i < NR_TOOLS; ++i) shapeToolButtons[i - 1].applyTheme (theme);
	for (int i = 0; i < NR_EDITS; ++i) shapeEditButtons[i].applyTheme (theme);
	shapeScreen.applyTheme (theme);
	shapeListBox.applyTheme (theme);

	grainContainer.applyTheme (theme);
	for (int i = PROPERTIES; i < PROPERTIES + NR_GRAIN_PROPERTIES * PROPERTIES_SIZE; ++i) controllerWidgets[i]->applyTheme (theme);
	for (int i = 0; i < NR_GRAIN_PROPERTIES; ++i)
	{
		grainWidgets[i].rangeDial.applyTheme (theme);
		grainWidgets[i].modContainer.applyTheme (theme);
		grainWidgets[i].miniMaxiButton.applyTheme (theme);
		grainWidgets[i].modBox.applyTheme (theme);
		grainWidgets[i].modLabel.applyTheme (theme);
		for (int j = 0; j < NR_PROPERTY_MODULATORS; ++j) grainWidgets[i].modListBoxes[j].applyTheme (theme);
	}
	grainDisplay.applyTheme (theme);
	drawGrainDisplay();

	pianoRoll.applyTheme (theme);
	for (int i = 0; i < NR_ENVS; ++i) envWidgets[i].container.applyTheme (theme);
	for (int i = 0; i < NR_LFOS; ++i) lfoWidgets[i].container.applyTheme (theme);
	helpButton.applyTheme (theme);
	ytButton.applyTheme (theme);
}

void BHarvestrGUI::onConfigureRequest (BEvents::ExposeEvent* event)
{
	Window::onConfigureRequest (event);

	sz = (getWidth() / 1440 > getHeight() / 880 ? getHeight() / 880 : getWidth() / 1440);
	resize ();
}

void BHarvestrGUI::onCloseRequest (BEvents::WidgetEvent* event)
{
	if (!event) return;
	Widget* requestWidget = event->getRequestWidget ();
	if (!requestWidget) return;

	if (requestWidget == sampleFileChooser)
	{
		if (sampleFileChooser->getValue() == 1.0)
		{
			sampleNameLabel.setText (sampleFileChooser->getFileName());
			samplePath = sampleFileChooser->getPath();
			sendSamplePath ();
		}

		// Close fileChooser
		mContainer.release (sampleFileChooser);	// TODO Check why this is required
		delete sampleFileChooser;
		sampleFileChooser = nullptr;
		return;
	}

	Window::onCloseRequest (event);
}

void BHarvestrGUI::sendUiStatus (const bool on)
{
	uint8_t obj_buf[64];
	lv2_atom_forge_set_buffer(&forge, obj_buf, sizeof(obj_buf));

	LV2_Atom_Forge_Frame frame;
	LV2_Atom* msg = (LV2_Atom*)lv2_atom_forge_object(&forge, &frame, 0, (on ? uris.bharvestr_uiOn : uris.bharvestr_uiOff));
	lv2_atom_forge_pop(&forge, &frame);
	write_function(controller, CONTROL, lv2_atom_total_size(msg), uris.atom_eventTransfer, msg);
}

void BHarvestrGUI::sendPlaybackStatus (const PlaybackIndex index, const bool on)
{
	uint8_t obj_buf[64];
	lv2_atom_forge_set_buffer(&forge, obj_buf, sizeof(obj_buf));
	LV2_URID urid = (index == PLAY_SAMPLE	? (on ? uris.bharvestr_samplePlay : uris.bharvestr_sampleStop)
						: (on ? uris.bharvestr_selectionPlay : uris.bharvestr_selectionStop));

	LV2_Atom_Forge_Frame frame;
	LV2_Atom* msg = (LV2_Atom*)lv2_atom_forge_object(&forge, &frame, 0, urid);
	lv2_atom_forge_pop(&forge, &frame);
	write_function(controller, CONTROL, lv2_atom_total_size(msg), uris.atom_eventTransfer, msg);
}

void BHarvestrGUI::sendPatchInfo (const PatchInfoIndex index)
{
	uint8_t obj_buf[1096];
	lv2_atom_forge_set_buffer(&forge, obj_buf, sizeof(obj_buf));
	std::string infotext = patchInput[index].getText();
	size_t textsize = infotext.size();
	std::string sendinfo = infotext.substr (0, textsize > 1023 ? textsize : 1023);

	LV2_URID presetURID[7] =
	{
		uris.bharvestr_presetInfoName,
		uris.bharvestr_presetInfoType,
		uris.bharvestr_presetInfoDate,
		uris.bharvestr_presetInfoCreator,
		uris.bharvestr_presetInfoURI,
		uris.bharvestr_presetInfoLicense,
		uris.bharvestr_presetInfoDescription
	};

	LV2_Atom_Forge_Frame frame;
	LV2_Atom* msg = (LV2_Atom*)lv2_atom_forge_object(&forge, &frame, 0, uris.bharvestr_presetInfoEvent);
	lv2_atom_forge_key(&forge, presetURID[index]);

	if (index == PATCH_DATE)
	{
		int date = 0;
		// TODO Parse date from infotext
		lv2_atom_forge_int(&forge, date);
	}
	else lv2_atom_forge_string (&forge, sendinfo.c_str(), strlen (sendinfo.c_str()) + 1);
	lv2_atom_forge_pop(&forge, &frame);
	write_function(controller, CONTROL, lv2_atom_total_size(msg), uris.atom_eventTransfer, msg);
}

void BHarvestrGUI::sendSamplePath ()
{
	std::string path = samplePath + "/" + sampleNameLabel.getText();
	uint8_t obj_buf[1024];
	lv2_atom_forge_set_buffer(&forge, obj_buf, sizeof(obj_buf));

	LV2_Atom_Forge_Frame frame;
	LV2_Atom* msg = (LV2_Atom*)lv2_atom_forge_object(&forge, &frame, 0, uris.bharvestr_sampleEvent);
	lv2_atom_forge_key(&forge, uris.bharvestr_samplePath);
	lv2_atom_forge_path (&forge, path.c_str(), path.size() + 1);
	lv2_atom_forge_pop(&forge, &frame);
	write_function(controller, CONTROL, lv2_atom_total_size(msg), uris.atom_eventTransfer, msg);
}

void BHarvestrGUI::sendShape (size_t shapeNr)
{
	size_t size = shapeWidgets[shapeNr].size ();

	uint8_t obj_buf[4096];
	lv2_atom_forge_set_buffer(&forge, obj_buf, sizeof(obj_buf));

	// Load shapeBuffer
	float shapeBuffer[MAXNODES * 7];
	for (unsigned int i = 0; i < size; ++i)
	{
		Node node = shapeWidgets[shapeNr].getRawNode (i);
		shapeBuffer[i * 7 + 0] = (float)node.nodeType;
		shapeBuffer[i * 7 + 1] = (float)node.point.x;
		shapeBuffer[i * 7 + 2] = (float)node.point.y;
		shapeBuffer[i * 7 + 3] = (float)node.handle1.x;
		shapeBuffer[i * 7 + 4] = (float)node.handle1.y;
		shapeBuffer[i * 7 + 5] = (float)node.handle2.x;
		shapeBuffer[i * 7 + 6] = (float)node.handle2.y;
	}

	// Notify shapeBuffer
	LV2_Atom_Forge_Frame frame;
	LV2_Atom* msg = (LV2_Atom*)lv2_atom_forge_object (&forge, &frame, 0, uris.bharvestr_shapeEvent);
	lv2_atom_forge_key(&forge, uris.bharvestr_shapeIndex);
	lv2_atom_forge_int(&forge, shapeNr);
	lv2_atom_forge_key(&forge, uris.bharvestr_shapeData);
	lv2_atom_forge_vector(&forge, sizeof(float), uris.atom_Float, (uint32_t) (7 * size), &shapeBuffer);
	lv2_atom_forge_pop(&forge, &frame);
	write_function (controller, CONTROL, lv2_atom_total_size(msg), uris.atom_eventTransfer, msg);
}


void BHarvestrGUI::sendPattern ()
{
	uint8_t obj_buf[8192];
	lv2_atom_forge_set_buffer(&forge, obj_buf, sizeof(obj_buf));

	LV2_Atom_Forge_Frame frame;
	LV2_Atom* msg = (LV2_Atom*)lv2_atom_forge_object(&forge, &frame, 0, uris.bharvestr_patternEvent);
	lv2_atom_forge_key(&forge, uris.bharvestr_patternRows);
	lv2_atom_forge_int(&forge, patternWidget.getRows());
	lv2_atom_forge_key(&forge, uris.bharvestr_patternSteps);
	lv2_atom_forge_int(&forge, patternWidget.getSteps());
	lv2_atom_forge_key(&forge, uris.bharvestr_pattern);
	lv2_atom_forge_vector(&forge, sizeof(int), uris.atom_Int, MAXPATTERNSTEPS, (void*) patternWidget.getValues());
	lv2_atom_forge_pop(&forge, &frame);
	write_function(controller, CONTROL, lv2_atom_total_size(msg), uris.atom_eventTransfer, msg);
}

void BHarvestrGUI::sendKeyboard (const uint8_t note, const bool noteOn)
{
	uint8_t obj_buf[64];
	lv2_atom_forge_set_buffer(&forge, obj_buf, sizeof(obj_buf));
	LV2_URID urid = (noteOn	? uris.bharvestr_keyOn : uris.bharvestr_keyOff);
	LV2_Atom_Forge_Frame frame;
	LV2_Atom* msg = (LV2_Atom*)lv2_atom_forge_object(&forge, &frame, 0, uris.bharvestr_keyboardEvent);
	lv2_atom_forge_key (&forge, urid);
	lv2_atom_forge_int (&forge, note);
	lv2_atom_forge_pop (&forge, &frame);
	write_function(controller, CONTROL, lv2_atom_total_size (msg), uris.atom_eventTransfer, msg);
}

void BHarvestrGUI::valueChangedCallback(BEvents::Event* event)
{
	if (!event) return;
	BWidgets::ValueWidget* widget = (BWidgets::ValueWidget*) event->getWidget ();
	if (!widget) return;
	float value = widget->getValue();
	BHarvestrGUI* ui = (BHarvestrGUI*) widget->getMainWindow();
	if (!ui) return;

	int controllerNr = -1;

	// Identify controller
	for (int i = 0; i < MAXCONTROLLERS; ++i)
	{
		if (widget == ui->controllerWidgets[i])
		{
			controllerNr = i;
			break;
		}
	}

	// Controllers
	if (controllerNr >= 0)
	{
		if ((controllerNr == SAMPLE_START) || (controllerNr == SAMPLE_END))
		{
			// Set slider position
			double x0 = ui->sampleWidget.getPosition().x - 0.5 * widget->getWidth();
			double y0 = ui->sampleWidget.getPosition().y - 1.5 * widget->getWidth();
			double w = ui->sampleWidget.getWidth();
			if ((controllerNr == SAMPLE_START) && (value > ui->sampleEndLine.getValue())) value = ui->sampleEndLine.getValue();
			if ((controllerNr == SAMPLE_END) && (value < ui->sampleStartLine.getValue())) value = ui->sampleStartLine.getValue();
			widget->setValue (value);
			if (w) widget->moveTo (x0 + value * w, y0);

			// Apply changes to pattern
			ui->updateSelection();
		}

		else if (controllerNr == PATTERN_TYPE)
		{
			ui->patternWidget.setPattern (PatternIndex (int (value)));
			switch (PatternIndex (int (value)))
			{
				case RISING_PATTERN:
				case FALLING_PATTERN:	ui->controllerWidgets[PATTERN_SIZE]->setValue (1.0);
							break;

				case SINE_PATTERN:	ui->controllerWidgets[PATTERN_SIZE]->setValue (M_PI);
							break;

			 	case TRIANGLE_PATTERN:	ui->controllerWidgets[PATTERN_SIZE]->setValue (2.0);
							break;

				default:		break;
			}
		}

		else if (controllerNr == PATTERN_SIZE)
		{
			ui->patternWidget.setSteps (ceil (value * ui->patternWidget.getRows()));
		}

		else if (controllerNr == SAMPLE_FREQ)
		{
			int note = 69 + round (12.0 * log2 (value / 440.0));
			if (note != int (ui->sampleNoteListBox.getValue()))
			{
				// Release -> change value -> add (to prevent callback dead loops)
				Widget* parent = ui->sampleNoteListBox.getParent();
				if (parent)
				{
					BWidgets::PopupListBox* listBox = &ui->sampleNoteListBox;
					parent->release (listBox);
					listBox->setValue (note);
					parent->add (*listBox);
				}
			}
		}

		else if ((controllerNr >= LFOS) && (controllerNr < LFOS + LFO_SIZE * NR_LFOS))
		{
			int lfoNr = (controllerNr - LFOS) / LFO_SIZE;

			ui->lfoWidgets[lfoNr].displayWidget.setPoints
			(
				ui->makeLfo
				(
					LfoIndex (int (ui->lfoWidgets[lfoNr].typeListBox.getValue())),
					ui->lfoWidgets[lfoNr].phaseDial.getValue(),
					ui->lfoWidgets[lfoNr].ampDial.getValue()
				)
			);
		}

		else if ((controllerNr >= SEQS) && (controllerNr < SEQS + SEQ_SIZE * NR_SEQS))
		{
			int seqNr = (controllerNr - SEQS) / SEQ_SIZE;
			int featureNr = (controllerNr - SEQS) % SEQ_SIZE;

			if (featureNr == SEQ_CHS) ui->seqWidgets[seqNr].sequencerWidget.setSize (value);
		}

		else if ((controllerNr >= ENVS) && (controllerNr < ENVS + ENV_SIZE * NR_ENVS))
		{
			int envNr = (controllerNr - ENVS) / ENV_SIZE;

			ui->envWidgets[envNr].displayWidget.setPoints
			(
				ui->makeEnv
				(
					ui->envWidgets[envNr].attackSlider.getValue(),
					ui->envWidgets[envNr].decaySlider.getValue(),
					ui->envWidgets[envNr].sustainSlider.getValue(),
					ui->envWidgets[envNr].releaseSlider.getValue()
				)
			);
		}

		else if
		(
			(controllerNr == PROPERTIES + GRAIN_SIZE * PROPERTIES_SIZE + PROPERTY_VALUE_START) ||
			(controllerNr == PROPERTIES + GRAIN_SIZE * PROPERTIES_SIZE + PROPERTY_VALUE_END)
		)
		{
			ui->updateSelection();
		}

		else if (controllerNr == GRAIN_SHAPE)
		{
			if (int (value) >= USER_SHAPES) ui->shapeScreen.hide();
			else ui->shapeScreen.show();

			for (int i = 0; i < USER_SHAPES + NR_USER_SHAPES; ++i)
			{
				if (i == int (value)) ui->shapeWidgets[i].show();
				else ui->shapeWidgets[i].hide();
			}

			ui->drawGrainDisplay();
		}

		// ---------------

		// Grain properties
		if ((controllerNr >= PROPERTIES) && (controllerNr < PROPERTIES + NR_GRAIN_PROPERTIES * PROPERTIES_SIZE))
		{
			int property = (controllerNr - PROPERTIES) / PROPERTIES_SIZE;
			int param = (controllerNr - PROPERTIES) % PROPERTIES_SIZE;

			// Range sliders: Send both (start and end) values instead
			if ((param == PROPERTY_VALUE_START) || (param == PROPERTY_VALUE_END))
			{
				int pport = CONTROLLERS + PROPERTIES + property * PROPERTIES_SIZE;
				float start = ui->grainWidgets[property].rangeDial.getStartValue();
				float end = ui->grainWidgets[property].rangeDial.getEndValue();
				ui->write_function (ui->controller, pport + PROPERTY_VALUE_START, sizeof(float), 0, &start);
				ui->write_function (ui->controller, pport + PROPERTY_VALUE_END, sizeof(float), 0, &end);
				if (param == PROPERTY_VALUE_START) ui->drawGrainDisplay();
			}

			else ui->write_function(ui->controller, CONTROLLERS + controllerNr, sizeof(float), 0, &value);
		}

		else ui->write_function(ui->controller, CONTROLLERS + controllerNr, sizeof(float), 0, &value);
	}
}

void BHarvestrGUI::textChangedCallback (BEvents::Event* event)
{
	if (!event) return;
	BWidgets::Label* widget = (BWidgets::Label*) event->getWidget ();
	if (!widget) return;
	BEvents::MessageEvent* mev = (BEvents::MessageEvent*) event;
	BHarvestrGUI* ui = (BHarvestrGUI*) widget->getMainWindow();
	if (!ui) return;

	// Allow only one selected label
	if (mev->getName() == BWIDGETS_LABEL_EDIT_ENTERED_MESSAGE)
	{
		for (BWidgets::Label& p : ui->patchInput)
		{
			if ((&p != widget) && p.getEditMode ()) p.applyEdit ();
		}
	}

	// Label text changed
	else if (mev->getName() == BWIDGETS_LABEL_TEXT_CHANGED_MESSAGE)
	{
		for (int i = 0; i < NR_PATCHES; ++i)
		{
			if (widget == &ui->patchInput[i])
			{
				ui->sendPatchInfo (PatchInfoIndex (i));
				break;
			}
		}
	}
}

void BHarvestrGUI::lfoTabClickedCallback(BEvents::Event* event)
{
	if (!event) return;
	BWidgets::ImageIcon* widget = (BWidgets::ImageIcon*) event->getWidget ();
	if (!widget) return;
	BHarvestrGUI* ui = (BHarvestrGUI*) widget->getMainWindow();
	if (!ui) return;

	// Identify and change tab
	for (int i = 0; i < NR_LFOS; ++i)
	{
		if (widget == &ui->lfoTabIcons[i])
		{
			ui->lfoTabIcons[i].rename ("activetab");
			ui->lfoTabIcons[i].applyTheme (ui->theme);
			ui->lfoWidgets[i].container.show();
		}
		else
		{
			ui->lfoTabIcons[i].rename ("tab");
			ui->lfoTabIcons[i].applyTheme (ui->theme);
			ui->lfoWidgets[i].container.hide();
		}
	}
}

void BHarvestrGUI::seqTabClickedCallback(BEvents::Event* event)
{
	if (!event) return;
	BWidgets::ImageIcon* widget = (BWidgets::ImageIcon*) event->getWidget ();
	if (!widget) return;
	BHarvestrGUI* ui = (BHarvestrGUI*) widget->getMainWindow();
	if (!ui) return;

	// Identify and change tab
	for (int i = 0; i < NR_SEQS; ++i)
	{
		if (widget == &ui->seqTabIcons[i])
		{
			ui->seqTabIcons[i].rename ("activetab");
			ui->seqTabIcons[i].applyTheme (ui->theme);
			ui->seqWidgets[i].container.show();
		}
		else
		{
			ui->seqTabIcons[i].rename ("tab");
			ui->seqTabIcons[i].applyTheme (ui->theme);
			ui->seqWidgets[i].container.hide();
		}
	}
}

void BHarvestrGUI::rndTabClickedCallback(BEvents::Event* event)
{
	if (!event) return;
	BWidgets::ImageIcon* widget = (BWidgets::ImageIcon*) event->getWidget ();
	if (!widget) return;
	BHarvestrGUI* ui = (BHarvestrGUI*) widget->getMainWindow();
	if (!ui) return;

	// Identify and change tab
	for (int i = 0; i < NR_RNDS; ++i)
	{
		if (widget == &ui->rndTabIcons[i])
		{
			ui->rndTabIcons[i].rename ("activetab");
			ui->rndTabIcons[i].applyTheme (ui->theme);
			ui->rndWidgets[i].container.show();
		}
		else
		{
			ui->rndTabIcons[i].rename ("tab");
			ui->rndTabIcons[i].applyTheme (ui->theme);
			ui->rndWidgets[i].container.hide();
		}
	}
}

void BHarvestrGUI::envTabClickedCallback(BEvents::Event* event)
{
	if (!event) return;
	BWidgets::ImageIcon* widget = (BWidgets::ImageIcon*) event->getWidget ();
	if (!widget) return;
	BHarvestrGUI* ui = (BHarvestrGUI*) widget->getMainWindow();
	if (!ui) return;

	// Identify and change tab
	for (int i = 0; i < NR_ENVS; ++i)
	{
		if (widget == &ui->envTabIcons[i])
		{
			ui->envTabIcons[i].rename ("activetab");
			ui->envTabIcons[i].applyTheme (ui->theme);
			ui->envWidgets[i].container.show();
		}
		else
		{
			ui->envTabIcons[i].rename ("tab");
			ui->envTabIcons[i].applyTheme (ui->theme);
			ui->envWidgets[i].container.hide();
		}
	}
}

void BHarvestrGUI::sampleNoteChangedCallback (BEvents::Event* event)
{
	if (!event) return;
	BWidgets::ValueWidget* widget = (BWidgets::ValueWidget*) event->getWidget ();
	if (!widget) return;
	float value = widget->getValue();
	BHarvestrGUI* ui = (BHarvestrGUI*) widget->getMainWindow();
	if (!ui) return;

	ui->sampleFrequencySelect.setValue (pow (2.0, (value - 69) / 12.0) * 440.0);
}

void BHarvestrGUI::samplePlayClickedCallback (BEvents::Event* event)
{
	if (!event) return;
	BWidgets::ValueWidget* widget = (BWidgets::ValueWidget*) event->getWidget ();
	if (!widget) return;
	float value = widget->getValue();
	BHarvestrGUI* ui = (BHarvestrGUI*) widget->getMainWindow();
	if (!ui) return;

	if (widget == &ui->samplePlayButton)
	{
		ui->sendPlaybackStatus (PLAY_SAMPLE, value == 1.0);
		if (value == 1.0) ui->sampleSelectionPlayButton.setValue (0.0);
	}
	else if (widget == &ui->sampleSelectionPlayButton)
	{
		ui->sendPlaybackStatus (PLAY_SELECTION, value == 1.0);
		if (value == 1.0) ui->samplePlayButton.setValue (0.0);
	}
}

void BHarvestrGUI::pianoCallback (BEvents::Event* event)
{
	if (!event) return;
	BEvents::MessageEvent* mev = (BEvents::MessageEvent*) event;
	BWidgets::HPianoRoll* widget = (BWidgets::HPianoRoll*) event->getWidget ();
	if (!widget) return;
	BHarvestrGUI* ui = (BHarvestrGUI*) widget->getMainWindow();
	if (!ui) return;

	std::string msg = mev->getName ();
	int val = mev->getContent().get<int>();

	if (msg == BWIDGETS_PIANO_KEY_PRESSED_MESSAGE) ui->sendKeyboard (val, true);
	else if (msg == BWIDGETS_PIANO_KEY_RELEASED_MESSAGE) ui->sendKeyboard (val, false);
}

void BHarvestrGUI::shapeToolClickedCallback(BEvents::Event* event)
{
	if (!event) return;
	BWidgets::ValueWidget* widget = (BWidgets::ValueWidget*) event->getWidget ();
	if (!widget) return;
	float value = widget->getValue();
	BHarvestrGUI* ui = (BHarvestrGUI*) widget->getMainWindow();
	if (!ui) return;

	// Identify tool
	int widgetNr = 0;
	if (value)
	{
		for (int i = 1; i < NR_TOOLS; ++i)
		{
			if (widget == &ui->shapeToolButtons[i - 1])
			{
				widgetNr = i;
				break;
			}
		}
	}

	for (int i = 0; i < USER_SHAPES + NR_USER_SHAPES; ++i) ui->shapeWidgets[i].setTool (ToolType (widgetNr));

	// Allow only one button pressed
	for (int i = 1; i < NR_TOOLS; ++i)
	{
		if (i != widgetNr) ui->shapeToolButtons[i - 1].setValue (0.0);
	}
}

void BHarvestrGUI::shapeEditClickedCallback(BEvents::Event* event)
{
	if (!event) return;
	BWidgets::ValueWidget* widget = (BWidgets::ValueWidget*) event->getWidget ();
	if (!widget) return;
	float value = widget->getValue();
	if (value != 1.0) return;
	BHarvestrGUI* ui = (BHarvestrGUI*) widget->getMainWindow();
	if (!ui) return;

	// Identify editButtons
	int widgetNr = -1;
	for (int i = 0; i < NR_EDITS; ++i)
	{
		if (widget == &ui->shapeEditButtons[i])
		{
			widgetNr = i;
			break;
		}
	}

	// Action
	int shapeNr = ui->shapeListBox.getValue();
	switch (widgetNr)
	{
		case EDIT_CUT:		ui->clipboard = ui->shapeWidgets[shapeNr].cutSelection();
					break;

		case EDIT_COPY:		ui->clipboard = ui->shapeWidgets[shapeNr].copySelection();
					break;

		case EDIT_PASTE:	ui->shapeWidgets[shapeNr].pasteSelection (ui->clipboard);
					break;

		case EDIT_RESET:	ui->shapeWidgets[shapeNr].reset();
					break;

		case EDIT_UNDO:		ui->shapeWidgets[shapeNr].undo();
					break;

		case EDIT_REDO:		ui->shapeWidgets[shapeNr].redo();
					break;

		default:		break;
	}
}

void BHarvestrGUI::shapeChangedCallback (BEvents::Event* event)
{
	if (!event) return;
	ShapeWidget* widget = (ShapeWidget*) event->getWidget ();
	if (!widget) return;
	float value = widget->getValue();
	if (value != 1.0) return;
	BHarvestrGUI* ui = (BHarvestrGUI*) widget->getMainWindow();
	if (!ui) return;

	for (int i = 0; i < USER_SHAPES + NR_USER_SHAPES; ++i)
	{
		if (widget == &ui->shapeWidgets[i])
		{
			ui->sendShape (i);
			break;
		}
	}

	ui->drawGrainDisplay();
}

void BHarvestrGUI::sampleLoadButtonClickedCallback (BEvents::Event* event)
{
	if (!event) return;
	BWidgets::Widget* widget = event->getWidget ();
	if (!widget) return;
	BHarvestrGUI* ui = (BHarvestrGUI*) widget->getMainWindow();
	if (!ui) return;

	if (ui->sampleFileChooser) delete ui->sampleFileChooser;
	ui->sampleFileChooser = new BWidgets::FileChooser
	(
		200, 120, 300, 400, "filechooser", ui->samplePath,
		std::vector<BWidgets::FileFilter>
		{
			BWidgets::FileFilter {"All files", std::regex (".*")},
			BWidgets::FileFilter {"Audio files", std::regex (".*\\.((wav)|(wave)|(aif)|(aiff)|(au)|(sd2)|(flac)|(caf)|(ogg))$", std::regex_constants::icase)}
		},
		"Open");
	if (ui->sampleFileChooser)
	{
		RESIZE ((*ui->sampleFileChooser), 200, 120, 300, 400, ui->sz);
		ui->mContainer.add (*ui->sampleFileChooser);
	}
}

void BHarvestrGUI::sliderDraggedCallback (BEvents::Event* event)
{
	if (!event) return;
	BEvents::PointerEvent* pev = (BEvents::PointerEvent*) event;
	HSlider* widget = (HSlider*) event->getWidget ();
	if (!widget) return;
	BHarvestrGUI* ui = (BHarvestrGUI*) widget->getMainWindow();
	if (!ui) return;

	double w = ui->sampleWidget.getWidth();

	if (w)
	{
		double value = widget->getValue() + pev->getDelta().x / w;
		widget->setValue (value);
	}

}

void BHarvestrGUI::patternClickedCallback (BEvents::Event* event)
{
	if (!event) return;
	PatternWidget* widget = (PatternWidget*) event->getWidget ();
	if (!widget) return;
	BHarvestrGUI* ui = (BHarvestrGUI*) widget->getMainWindow();
	if (!ui) return;

	ui->patternTypeListBox.setValue (USER_PATTERN);
	ui->sendPattern();
}

void BHarvestrGUI::helpButtonClickedCallback (BEvents::Event* event)
{
	if (system(OPEN_CMD " " HELP_URL)) std::cerr << "BHarvestr.lv2#GUI: Can't open " << HELP_URL << ". You can try to call it maually.";
}

void BHarvestrGUI::ytButtonClickedCallback (BEvents::Event* event)
{
	if (system(OPEN_CMD " " YT_URL))  std::cerr << "BHarvestr.lv2#GUI: Can't open " << YT_URL << ". You can try to call it maually.";
}

void BHarvestrGUI::drawGrainDisplay ()
{
	double x0 = grainDisplay.getXOffset ();
	double y0 = grainDisplay.getYOffset ();
	double w = grainDisplay.getEffectiveWidth ();
	double h = grainDisplay.getEffectiveHeight ();
	double ymin = -0.05;
	double ymax = 1.05;
	//BColors::Color lineColor = *fgColors.getColor (BColors::NORMAL);
	BColors::Color gridColor = *bgColors.getColor (BColors::NORMAL);

	cairo_surface_clear (grainDisplay.getDrawingSurface());
	cairo_t* cr = cairo_create (grainDisplay.getDrawingSurface());

	if (cairo_status (cr) == CAIRO_STATUS_SUCCESS)
	{
		// Limit cairo-drawing area
		cairo_rectangle (cr, x0, y0, w, h);
		cairo_clip (cr);

		double ygrid = pow (10, floor (log10 (1.0 / 1.5)));
		int ldYgrid = log10 (ygrid);
		std::string nrformat = "%" + ((ygrid < 1) ? ("1." + std::to_string (-ldYgrid)) : (std::to_string (ldYgrid + 1) + ".0")) + "f";
		cairo_text_extents_t ext;
		cairo_select_font_face (cr, smLabelFont.getFontFamily ().c_str (), smLabelFont.getFontSlant (), smLabelFont.getFontWeight ());
		cairo_set_font_size (cr, smLabelFont.getFontSize ());

		// Draw Y steps
		for (double yp = ceil (ymin / ygrid) * ygrid; yp <= ymax; yp += ygrid)
		{
			cairo_move_to (cr, x0, y0 + h - h * (yp - ymin) / (ymax - ymin));
			cairo_line_to (cr, x0 + 0.02 * w, y0 + h - h * (yp - ymin) / (ymax - ymin));

			std::string label = BUtilities::to_string (yp, nrformat);
			cairo_text_extents (cr, label.c_str(), &ext);
			cairo_move_to (cr, x0 + 0.025 * w - ext.x_bearing, y0 + h - h * (yp - ymin) / (ymax - ymin) - ext.height / 2 - ext.y_bearing);
			cairo_set_source_rgba (cr, CAIRO_RGBA (gridColor));
			cairo_show_text (cr, label.c_str ());

			cairo_move_to (cr, x0 + 0.03 * w + ext.width, y0 + h - h * (yp - ymin) / (ymax - ymin));
			cairo_line_to (cr, x0 + w, y0 + h - h * (yp - ymin) / (ymax - ymin));
		}

		cairo_set_source_rgba (cr, CAIRO_RGBA (gridColor));
		cairo_set_line_width (cr, 1);
		cairo_stroke (cr);

		// Draw Y axis
		cairo_move_to (cr, 0, 0);
		cairo_line_to (cr, 0, h);
		cairo_set_source_rgba (cr, CAIRO_RGBA (gridColor));
		cairo_set_line_width (cr, 2.0);
		cairo_stroke (cr);

		// Draw curves
		int shapeNr = shapeListBox.getValue();
		double gSize = grainWidgets[GRAIN_SIZE].rangeDial.getStartValue();
		double gRate = grainWidgets[GRAIN_RATE].rangeDial.getStartValue();
		double gPhase = grainWidgets[GRAIN_PHASE].rangeDial.getStartValue();
		double gLevel = grainWidgets[GRAIN_LEVEL].rangeDial.getStartValue();

		int dGrains = (gRate <= 1.0 ? 2: ceil (gRate) + 1);
		double dSize = (gRate <= 1.0 ? gSize / gRate + gSize : 2.0 * gSize);

		BColors::Color lnColor = *fgColors.getColor (getState ());
		cairo_pattern_t* pat = cairo_pattern_create_linear (0, y0 + h, 0, y0);
		cairo_pattern_add_color_stop_rgba (pat, 0, lnColor.getRed (), lnColor.getGreen (), lnColor.getBlue (), 0);
		cairo_pattern_add_color_stop_rgba (pat, 1, lnColor.getRed (), lnColor.getGreen (), lnColor.getBlue (), 0.5 * lnColor.getAlpha ());

		for (int i = 0; i < dGrains; ++i)
		{
			double pos = (double (i) / gRate) * (gSize / dSize) + gPhase * (gSize / dSize);

			cairo_move_to (cr, x0 + pos * w, y0 + (1.0 - (shapeWidgets[shapeNr].getMapValue(0) + 0.05) / 1.1) * h);
			for (double x = 0.0; x < 1.0; x += 0.01)
			{
				cairo_line_to (cr, x0 + (pos + x * gSize / dSize) * w, y0 + (1.0 - gLevel * (shapeWidgets[shapeNr].getMapValue(x) + 0.05) / 1.1) * h);
			}
			cairo_set_source_rgba (cr, CAIRO_RGBA (lnColor));
			cairo_set_line_width (cr, 2.0);
			cairo_stroke_preserve (cr);

			cairo_line_to (cr, x0 + (pos + gSize / dSize) * w, y0 + h); // TODO
			cairo_line_to (cr, x0 + pos * w, y0 + h); // TODO
			cairo_close_path (cr);
			cairo_set_source (cr, pat);
			cairo_fill (cr);
		}

		cairo_destroy (cr);
	}

	grainDisplay.update();
}

std::vector<BUtilities::Point> BHarvestrGUI::makeLfo (const LfoIndex lfoId, const double phase, const double amp) const
{
	std::vector<BUtilities::Point> points = {};
	Lfo lfo = Lfo (lfoId, 1.0, phase);
	for (double d = 0; d < 1; d += 0.01) points.push_back (BUtilities::Point (d, amp * lfo.getValue (d)));
	return points;
}

std::vector<BUtilities::Point> BHarvestrGUI::makeEnv (const double attack, const double decay, const double sustain, const double release) const
{
	std::vector<BUtilities::Point> points = {};
	Envelope env = Envelope (attack, decay, sustain, release);
	env.releaseAt (attack + decay + 4.0);
	double size = 16.0;

	points.push_back (BUtilities::Point (0.0, 0.0));

	// Attack
	for (int i = 0; i < 10; ++i)
	{
		double t = double (i) * 0.1 * attack;
		points.push_back (BUtilities::Point (t / size, env.getValue (t)));
	}

	// Decay
	for (int i = 0; i < 10; ++i)
	{
		double t = attack + double (i) * 0.1 * decay;
		points.push_back (BUtilities::Point (t / size, env.getValue (t)));
	}

	// Sustain
	points.push_back (BUtilities::Point ((attack + decay) / size, sustain));

	// Release
	for (int i = 0; i < 10; ++i)
	{
		double t = attack + decay + 4.0 + double (i) * 0.1 * release;
		points.push_back (BUtilities::Point (t / size, env.getValue (t)));
	}

	points.push_back (BUtilities::Point ((attack + decay + 4.0 + release) / size, 0.0));
	return points;
}

void BHarvestrGUI::updateSelection ()
{
	float grainSize = (grainWidgets[GRAIN_SIZE].rangeDial.getStartValue() + grainWidgets[GRAIN_SIZE].rangeDial.getEndValue()) / 2.0f;
	float selectionSize = (sampleEndLine.getValue() - sampleStartLine.getValue()) * sampleSize * 1000.0f;
	int grainsPerSelection = ceil (selectionSize / grainSize);
	patternWidget.setRows (LIMIT (grainsPerSelection, 1, MAXPATTERNSTEPS));
	patternWidget.setSteps (patternSizeSelect.getValue() * patternWidget.getRows());

	sampleSelectionSizeLabel.setText ("Selection: " + BUtilities::to_string (selectionSize / 1000.0f, "%5.2f s"));
}

static LV2UI_Handle instantiate (const LV2UI_Descriptor *descriptor,
						  const char *plugin_uri,
						  const char *bundle_path,
						  LV2UI_Write_Function write_function,
						  LV2UI_Controller controller,
						  LV2UI_Widget *widget,
						  const LV2_Feature *const *features)
{
	PuglNativeView parentWindow = 0;
	LV2UI_Resize* resize = NULL;

	if (strcmp(plugin_uri, BHARVESTR_URI) != 0)
	{
		std::cerr << "BHarvestr.lv2#GUI: GUI does not support plugin with URI " << plugin_uri << std::endl;
		return NULL;
	}

	for (int i = 0; features[i]; ++i)
	{
		if (!strcmp(features[i]->URI, LV2_UI__parent)) parentWindow = (PuglNativeView) features[i]->data;
		else if (!strcmp(features[i]->URI, LV2_UI__resize)) resize = (LV2UI_Resize*)features[i]->data;
	}
	if (parentWindow == 0) std::cerr << "BHarvestr.lv2#GUI: No parent window.\n";

	// New instance
	BHarvestrGUI* ui;
	try {ui = new BHarvestrGUI (bundle_path, features, parentWindow);}
	catch (std::exception& exc)
	{
		std::cerr << "BHarvestr.lv2#GUI: Instantiation failed. " << exc.what () << std::endl;
		return NULL;
	}

	ui->controller = controller;
	ui->write_function = write_function;

	// Reduce min GUI size for small displays
	double sz = 1.0;
	int screenWidth  = getScreenWidth ();
	int screenHeight = getScreenHeight ();
	if ((screenWidth < 1000) || (screenHeight < 620)) sz = 0.5;
	else if ((screenWidth < 1480) || (screenHeight < 920)) sz = 0.66;

	if (resize) resize->ui_resize(resize->handle, 1440 * sz, 880 * sz);

	*widget = (LV2UI_Widget) puglGetNativeWindow (ui->getPuglView ());

	ui->sendUiStatus (true);

	return (LV2UI_Handle) ui;
}

static void cleanup(LV2UI_Handle ui)
{
	BHarvestrGUI* self = (BHarvestrGUI*) ui;
	delete self;
}

static void port_event(LV2UI_Handle ui, uint32_t port_index, uint32_t buffer_size,
	uint32_t format, const void* buffer)
{
	BHarvestrGUI* self = (BHarvestrGUI*) ui;
	self->port_event(port_index, buffer_size, format, buffer);
}

static int call_idle (LV2UI_Handle ui)
{
	BHarvestrGUI* self = (BHarvestrGUI*) ui;
	self->handleEvents ();
	return 0;
}

static int call_resize (LV2UI_Handle ui, int width, int height)
{
	BHarvestrGUI* self = (BHarvestrGUI*) ui;
	BEvents::ExposeEvent* ev = new BEvents::ExposeEvent (self, self, BEvents::CONFIGURE_REQUEST_EVENT, self->getPosition().x, self->getPosition().y, width, height);
	self->addEventToQueue (ev);
	return 0;
}

static const LV2UI_Idle_Interface idle = {call_idle};
static const LV2UI_Resize resize = {nullptr, call_resize} ;

static const void* extension_data(const char* uri)
{
	if (!strcmp(uri, LV2_UI__idleInterface)) return &idle;
	else if(!strcmp(uri, LV2_UI__resize)) return &resize;
	else return NULL;
}

static const LV2UI_Descriptor guiDescriptor = {
		BHARVESTR_GUI_URI,
		instantiate,
		cleanup,
		port_event,
		extension_data
};

// LV2 Symbol Export
LV2_SYMBOL_EXPORT const LV2UI_Descriptor *lv2ui_descriptor(uint32_t index)
{
	switch (index) {
	case 0: return &guiDescriptor;
	default:return NULL;
    }
}

/* End of LV2 specific declarations
 *
 * *****************************************************************************
 *
 *
 */
