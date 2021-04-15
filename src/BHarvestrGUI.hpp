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

#ifndef BHARVESTRGUI_HPP_
#define BHARVESTRGUI_HPP_

#include <lv2/lv2plug.in/ns/lv2core/lv2.h>
#include <lv2/lv2plug.in/ns/extensions/ui/ui.h>
#include <lv2/lv2plug.in/ns/ext/atom/atom.h>
#include <lv2/lv2plug.in/ns/ext/atom/forge.h>
#include <lv2/lv2plug.in/ns/ext/time/time.h>
#include <lv2/lv2plug.in/ns/ext/midi/midi.h>
#include <iostream>
#include <algorithm>

#include "BWidgets/Widget.hpp"
#include "BWidgets/Window.hpp"
#include "BWidgets/Label.hpp"
#include "BWidgets/DrawingSurface.hpp"
#include "BWidgets/PopupListBox.hpp"
#include "BWidgets/ImageIcon.hpp"
#include "BWidgets/HPianoRoll.hpp"
#include "BWidgets/FileChooser.hpp"
#include "BWidgets/MessageBox.hpp"
#include "screen.h"


#include "definitions.h"
#include "LFO.hpp"
#include "Ports.hpp"
#include "Urids.hpp"
#include "ValueSelect.hpp"
#include "HSlider.hpp"
#include "RangeDial.hpp"
#include "Dial.hpp"
#include "VLine.hpp"
#include "VMarkerLine.hpp"
#include "HaloButton.hpp"
#include "HaloToggleButton.hpp"
#include "LoadButton.hpp"
#include "PlayButton.hpp"
#include "StopButton.hpp"
#include "MiniMaximizeButton.hpp"
#include "ShapeWidget.hpp"
#include "CurveChart.hpp"
#include "SequencerWidget.hpp"
#include "PatternWidget.hpp"
//#include "Journal.hpp"

#define BG_FILE "inc/surface.png"
#define HELP_URL "https://github.com/sjaehn/BHarvestr/blob/master/README.md"
#define YT_URL ""
#define WWW_BROWSER_CMD "x-www-browser"
#define MAXUNDO 20

#define RESIZE(widget, x, y, w, h, sz) {widget.moveTo ((x) * (sz), (y) * (sz)); widget.resize ((w) * (sz), (h) * (sz));}

enum editIndex
{
	EDIT_CUT	= 0,
	EDIT_COPY	= 1,
	EDIT_PASTE	= 2,
	EDIT_RESET	= 3,
	EDIT_UNDO	= 4,
	EDIT_REDO	= 5,
	NR_EDITS	= 6
};

const std::string editLabels[NR_EDITS] = {"Cut", "Copy", "Paste", "Reset", "Undo", "Redo"};
const std::string toolLabels[NR_TOOLS - 1] = {"Select", "Point node", "Auto smooth node", "Symmetric smooth node", "Corner node"};

class BHarvestrGUI : public BWidgets::Window
{
public:
	BHarvestrGUI (const char *bundle_path, const LV2_Feature *const *features, PuglNativeView parentWindow);
	~BHarvestrGUI ();
	void port_event (uint32_t port_index, uint32_t buffer_size, uint32_t format, const void *buffer);
	void sendUiStatus (const bool on);
	void sendPlaybackStatus (const PlaybackIndex index, const bool on);
	void sendPatchInfo (const PatchInfoIndex index);
	void sendSamplePath ();
	void sendShape (size_t shapeNr);
	void sendPattern ();
	void sendKeyboard (const uint8_t note, const bool noteOn);
	virtual void onConfigureRequest (BEvents::ExposeEvent* event) override;
	virtual void onCloseRequest (BEvents::WidgetEvent* event) override;
	void applyTheme (BStyles::Theme& theme) override;

	LV2UI_Controller controller;
	LV2UI_Write_Function write_function;

private:
	static void valueChangedCallback (BEvents::Event* event);
	static void textChangedCallback (BEvents::Event* event);
	static void lfoTabClickedCallback (BEvents::Event* event);
	static void seqTabClickedCallback (BEvents::Event* event);
	static void rndTabClickedCallback (BEvents::Event* event);
	static void envTabClickedCallback (BEvents::Event* event);
	static void sampleNoteChangedCallback (BEvents::Event* event);
	static void shapeToolClickedCallback (BEvents::Event* event);
	static void shapeEditClickedCallback (BEvents::Event* event);
	static void shapeChangedCallback (BEvents::Event* event);
	static void sampleLoadButtonClickedCallback (BEvents::Event* event);
	static void samplePlayClickedCallback (BEvents::Event* event);
	static void pianoCallback (BEvents::Event* event);
	static void sliderDraggedCallback (BEvents::Event* event);
	static void patternClickedCallback (BEvents::Event* event);
	static void helpButtonClickedCallback (BEvents::Event* event);
	static void ytButtonClickedCallback (BEvents::Event* event);
	void drawGrainDisplay();
	virtual void resize () override;
	std::vector<BUtilities::Point> makeLfo (const LfoIndex lfoId, const double phase, const double amp) const;
	std::vector<BUtilities::Point> makeEnv (const double attack, const double decay, const double sustain, const double release) const;
	void updateSelection ();

	std::string pluginPath;
	double sz;
	cairo_surface_t* bgImageSurface;

	BHarvestrURIs uris;
	LV2_Atom_Forge forge;

	// Controllers
	std::array<BWidgets::ValueWidget*, MAXCONTROLLERS> controllerWidgets;

	std::string samplePath;

	//Widgets
	BWidgets::Widget mContainer;

	BWidgets::MessageBox mBox;

	BWidgets::Widget globalContainer;
	BWidgets::ImageIcon globalTitleIcon;
	MiniMaximizeButton globalMiniMaxiButton;
	BWidgets::Label globalMaxVoicesLabel;
	BWidgets::Label globalMaxGrainsLabel;
	HSlider globalMaxVoicesSlider;
	HSlider globalMaxGrainsSlider;

	BWidgets::Widget patchContainer;
	BWidgets::ImageIcon patchTitleIcon;
	MiniMaximizeButton patchMiniMaxiButton;
	std::array<BWidgets::Label, NR_PATCHES> patchLabel;
	std::array<BWidgets::Label, NR_PATCHES> patchInput;

	BWidgets::Widget lfoContainer;
	struct LfoWidget
	{
		BWidgets::Widget container;
		Dial frequencyDial;
		Dial phaseDial;
		Dial ampDial;
		CurveChart displayWidget;
		VLine horizonWidget;
		BWidgets::PopupListBox typeListBox;
	};
	std::array <LfoWidget, NR_LFOS> lfoWidgets;
	std::array <BWidgets::ImageIcon, NR_LFOS> lfoTabIcons;

	BWidgets::Widget seqContainer;
	struct SeqWidget
	{
		BWidgets::Widget container;
		Dial frequencyDial;
		Dial phaseDial;
		HSlider sizeSlider;
		SequencerWidget<NR_SEQ_STEPS> sequencerWidget;
	};
	std::array <SeqWidget, NR_LFOS> seqWidgets;
	std::array <BWidgets::ImageIcon, NR_LFOS> seqTabIcons;

	BWidgets::Widget rndContainer;
	struct RndWidget
	{
		BWidgets::Widget container;
		Dial minDial;
		Dial maxDial;
	};
	std::array <RndWidget, NR_RNDS> rndWidgets;
	std::array <BWidgets::ImageIcon, NR_RNDS> rndTabIcons;

	BWidgets::Widget envContainer;
	struct EnvWidget
	{
		BWidgets::Widget container;
		HSlider attackSlider;
		HSlider decaySlider;
		HSlider sustainSlider;
		HSlider releaseSlider;
		CurveChart displayWidget;
		std::array <VLine, MAXVOICES> horizonWidgets;
	};
	std::array <EnvWidget, NR_LFOS> envWidgets;
	std::array <BWidgets::ImageIcon, NR_LFOS>  envTabIcons;

	BWidgets::Widget sampleContainer;
	CurveChart sampleWidget;
	float sampleSize;
	LoadButton sampleLoadButton;
	BWidgets::Label sampleNameLabel;
	BWidgets::FileChooser* sampleFileChooser;
	VMarkerLine sampleStartLine;
	VMarkerLine sampleEndLine;
	BWidgets::Widget sampleStartScreen;
	BWidgets::Widget sampleEndScreen;
	BWidgets::TextButton sampleFrequencyDetectionButton;
	ValueSelect sampleFrequencySelect;
	BWidgets::PopupListBox sampleNoteListBox;
	BWidgets::Label sampleSizeLabel;
	BWidgets::Label sampleSelectionSizeLabel;
	PlayButton samplePlayButton;
	PlayButton sampleSelectionPlayButton;

	BWidgets::Widget patternContainer;
	PatternWidget patternWidget;
	CurveChart patternSelectionWidget;
	ValueSelect patternSizeSelect;
	BWidgets::PopupListBox patternTypeListBox;

	struct ProcessWidget
	{
		RangeDial rangeDial;
		BWidgets::Widget modContainer;
		MiniMaximizeButton miniMaxiButton;
		BWidgets::Widget modBox;
		BWidgets::Label modLabel;
		std::array<BWidgets::PopupListBox, NR_PROPERTY_MODULATORS> modListBoxes;
	};
	std::array<ProcessWidget, NR_SYNTH_PROPERTIES> processWidgets;

	BWidgets::HPianoRoll pianoRoll;

	BWidgets::Widget shapeContainer;
	std::array<ShapeWidget, USER_SHAPES + NR_USER_SHAPES> shapeWidgets;
	BWidgets::Widget shapeScreen;
	std::array<HaloToggleButton, NR_TOOLS> shapeToolButtons;
	std::array <HaloButton, NR_EDITS> shapeEditButtons;
	BWidgets::PopupListBox shapeListBox;
	std::vector<Node> clipboard;


	BWidgets::Widget grainContainer;
	struct GrainWidget
	{
		RangeDial rangeDial;
		BWidgets::Widget modContainer;
		MiniMaximizeButton miniMaxiButton;
		BWidgets::Widget modBox;
		BWidgets::Label modLabel;
		std::array<BWidgets::PopupListBox, NR_PROPERTY_MODULATORS> modListBoxes;
	};
	std::array<GrainWidget, NR_GRAIN_PROPERTIES> grainWidgets;
	BWidgets::DrawingSurface grainDisplay;

	HaloButton helpButton;
	HaloButton ytButton;


	// Definition of styles
	BColors::ColorSet fgColors = {{{0.85, 0.0, 0.0, 1.0}, {1.0, 0.05, 0.05, 1.0}, {0.1, 0.0, 0.0, 1.0}, {0.0, 0.0, 0.0, 0.0}}};
	BColors::ColorSet txColors = {{{1.0, 1.0, 1.0, 1.0}, {1.0, 1.0, 1.0, 1.0}, {0.25, 0.25, 0.25, 1.0}, {0.0, 0.0, 0.0, 1.0}}};
	BColors::ColorSet selColors = {{{0.5, 0.5, 0.5, 0.333}, {1.0, 1.0, 1.0, 0.333}, {0.1, 0.1, 0.1, 0.333}, {0.0, 0.0, 0.0, 0.333}}};
	BColors::ColorSet ltBgColors = {{{0.4, 0.4, 0.4, 1.0}, {0.6, 0.6, 0.6, 1.0}, {0.05, 0.05, 0.05, 1.0}, {0.0, 0.0, 0.0, 1.0}}};
	BColors::ColorSet bgColors = {{{0.15, 0.15, 0.15, 1.0}, {0.3, 0.3, 0.3, 1.0}, {0.075, 0.075, 0.075, 1.0}, {0.0, 0.0, 0.0, 1.0}}};
	BColors::ColorSet ptBgColors = {{{0.03, 0.0, 0.0, 1.0}, {0.1, 0.1, 0.1, 1.0}, {0.0, 0.0, 0.0, 1.0}, {0.0, 0.0, 0.0, 1.0}}};
	BColors::ColorSet wvColors = {{{1.0, 1.0, 1.0, 0.15}, {1.0, 1.0, 1.0, 0.15}, {0.25, 0.25, 0.25, 0.15}, {0.0, 0.0, 0.0, 0.15}}};
	BColors::Color evenPadBgColor = {0.0, 0.05, 0.1, 1.0};
	BColors::Color oddPadBgColor = {0.0, 0.0, 0.0, 1.0};

	BStyles::Border screenBorder = BStyles::Border (BStyles::Line (BColors::Color (0.0, 0.0, 0.0, 0.75), 4.0));
	BStyles::Border boxBorder = {{{0.85, 0.0, 0.0, 0.75}, 1.0}, 0.0, 0.0, 0.0};
	BStyles::Border menuBorder = {{BColors::grey, 1.0}, 0.0, 0.0, 0.0};
	BStyles::Border labelborder = {BStyles::noLine, 4.0, 0.0, 0.0};
	BStyles::Border boxlabelborder = {{BColors::grey, 1.0}, 0.0, 3.0, 0.0};
	BStyles::Border focusborder = BStyles::Border (BStyles::Line (BColors::Color (0.0, 0.0, 0.0, 0.5), 2.0));
	BStyles::Fill widgetBg = BStyles::noFill;
	BStyles::Fill menuBg = BStyles::Fill (BColors::Color (0.02, 0.0, 0.0, 1.0));
	BStyles::Fill screenBg = BStyles::Fill (BColors::Color (0.0, 0.0, 0.0, 0.8));
	BStyles::Fill tabBg = BStyles::Fill (BColors::Color (1.0, 0, 0.0, 0.5));
	BStyles::Fill activeTabBg = BStyles::Fill (BColors::Color (1.0, 0, 0.0, 0.75));
	BStyles::Fill boxBg = BStyles::Fill (BColors::Color (0.0, 0.0, 0.0, 0.9));
	BStyles::Font ctLabelFont = BStyles::Font ("Sans", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_NORMAL, 12.0,
						   BStyles::TEXT_ALIGN_CENTER, BStyles::TEXT_VALIGN_MIDDLE);
	BStyles::Font lfLabelFont = BStyles::Font ("Sans", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_NORMAL, 12.0,
						   BStyles::TEXT_ALIGN_LEFT, BStyles::TEXT_VALIGN_MIDDLE);
	BStyles::Font smLabelFont = BStyles::Font ("Sans", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_NORMAL, 8.0,
						   BStyles::TEXT_ALIGN_LEFT, BStyles::TEXT_VALIGN_MIDDLE);
	BStyles::StyleSet defaultStyles = {"default", {{"background", STYLEPTR (&BStyles::noFill)},
					  {"border", STYLEPTR (&BStyles::noBorder)}}};
	BStyles::StyleSet labelStyles = {"labels", {{"background", STYLEPTR (&BStyles::noFill)},
					{"border", STYLEPTR (&labelborder)},
					{"textcolors", STYLEPTR (&fgColors)},
					{"font", STYLEPTR (&ctLabelFont)}}};
	BStyles::StyleSet focusStyles = {"labels", {{"background", STYLEPTR (&screenBg)},
					{"border", STYLEPTR (&focusborder)},
					{"textcolors", STYLEPTR (&txColors)},
					{"font", STYLEPTR (&lfLabelFont)}}};

	BStyles::Theme theme = BStyles::Theme
	({
		defaultStyles,
		{"B.Harvestr", 		{{"background", STYLEPTR (&BStyles::blackFill)},
					 {"border", STYLEPTR (&BStyles::noBorder)}}},
		{"main", 		{{"background", STYLEPTR (&widgetBg)},
					 {"border", STYLEPTR (&BStyles::noBorder)}}},
 		{"widget", 		{{"uses", STYLEPTR (&defaultStyles)}}},
		{"widget/focus",	{{"uses", STYLEPTR (&focusStyles)}}},
		{"label", 		{{"uses", STYLEPTR (&defaultStyles)},
					 {"font", STYLEPTR (&smLabelFont)},
				 	 {"textcolors", STYLEPTR (&ltBgColors)}}},
		{"tab", 		{{"background", STYLEPTR (&tabBg)},
					 {"border", STYLEPTR (&BStyles::noBorder)}}},
		{"activetab", 		{{"background", STYLEPTR (&activeTabBg)},
					 {"border", STYLEPTR (&BStyles::noBorder)}}},
		{"shape",	 	{{"background", STYLEPTR (&BStyles::noFill)},
 			 		 {"border", STYLEPTR (&BStyles::noBorder)},
 			 	 	 {"fgcolors", STYLEPTR (&fgColors)},
 					 {"symbolcolors", STYLEPTR (&fgColors)},
 					 {"font", STYLEPTR (&smLabelFont)},
 					 {"bgcolors", STYLEPTR (&bgColors)}}},
 		{"shape/focus", 	{{"background", STYLEPTR (&screenBg)},
 				 	 {"border", STYLEPTR (&screenBorder)},
				 	 {"font", STYLEPTR (&smLabelFont)},
				 	 {"textcolors", STYLEPTR (&txColors)}}},
		{"selection",	 	{{"background", STYLEPTR (&BStyles::noFill)},
 			 		 {"border", STYLEPTR (&BStyles::noBorder)},
 			 	 	 {"fgcolors", STYLEPTR (&selColors)}}},
 		{"pattern", 		{{"uses", STYLEPTR (&defaultStyles)},
 					 {"fgcolors", STYLEPTR (&fgColors)},
 					 {"bgcolors", STYLEPTR (&ptBgColors)}}},
		{"box", 		{{"background", STYLEPTR (&boxBg)},
					{"border", STYLEPTR (&boxBorder)}}},
		{"boxlabel",		{{"background", STYLEPTR (&boxBg)},
					 {"border", STYLEPTR (&boxlabelborder)},
					 {"textcolors", STYLEPTR (&BColors::whites)},
 					 {"font", STYLEPTR (&lfLabelFont)}}},
		{"button", 		{{"uses", STYLEPTR (&defaultStyles)}}},
		{"redbutton", 		{{"uses", STYLEPTR (&defaultStyles)},
					 {"bgcolors", STYLEPTR (&fgColors)}}},
		{"halobutton", 		{{"uses", STYLEPTR (&defaultStyles)},
					 {"fgcolors", STYLEPTR (&bgColors)}}},
		{"halobutton/focus",	{{"uses", STYLEPTR (&focusStyles)}}},
		{"dial", 		{{"uses", STYLEPTR (&defaultStyles)},
					 {"fgcolors", STYLEPTR (&fgColors)},
					 {"bgcolors", STYLEPTR (&bgColors)},
					 {"textcolors", STYLEPTR (&fgColors)},
					 {"font", STYLEPTR (&ctLabelFont)}}},
		{"dial/focus", 		{{"uses", STYLEPTR (&focusStyles)}}},
		{"ctlabel",	 	{{"uses", STYLEPTR (&labelStyles)}}},
		{"lflabel",	 	{{"uses", STYLEPTR (&labelStyles)},
					 {"font", STYLEPTR (&lfLabelFont)}}},
		{"menu",	 	{{"border", STYLEPTR (&menuBorder)},
					 {"background", STYLEPTR (&menuBg)}}},
		{"menu/item",	 	{{"uses", STYLEPTR (&defaultStyles)},
					 {"border", STYLEPTR (&labelborder)},
					 {"textcolors", STYLEPTR (&BColors::whites)},
					 {"font", STYLEPTR (&lfLabelFont)}}},
		{"menu/button",	 	{{"border", STYLEPTR (&menuBorder)},
					 {"background", STYLEPTR (&menuBg)},
					 {"bgcolors", STYLEPTR (&ltBgColors)}}},
		{"menu/listbox",	{{"border", STYLEPTR (&menuBorder)},
					 {"background", STYLEPTR (&menuBg)}}},
		{"menu/listbox/item",	{{"uses", STYLEPTR (&defaultStyles)},
					 {"border", STYLEPTR (&labelborder)},
					 {"textcolors", STYLEPTR (&BColors::whites)},
					 {"font", STYLEPTR (&lfLabelFont)}}},
		{"menu/listbox/button",	{{"border", STYLEPTR (&menuBorder)},
					 {"background", STYLEPTR (&menuBg)},
					 {"bgcolors", STYLEPTR (&ltBgColors)}}}
	});
};

#endif /* BHARVESTRGUI_HPP_ */
