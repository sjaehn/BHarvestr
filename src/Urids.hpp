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

#ifndef URIDS_HPP_
#define URIDS_HPP_

#include <lv2/lv2plug.in/ns/lv2core/lv2.h>
#include <lv2/lv2plug.in/ns/ext/atom/atom.h>
#include <lv2/lv2plug.in/ns/ext/atom/util.h>
#include <lv2/lv2plug.in/ns/ext/atom/forge.h>
#include <lv2/lv2plug.in/ns/ext/urid/urid.h>
#include <lv2/lv2plug.in/ns/ext/time/time.h>
#include <lv2/lv2plug.in/ns/ext/midi/midi.h>
#include <lv2/lv2plug.in/ns/ext/state/state.h>
#include "definitions.h"

struct BHarvestrURIs
{
	LV2_URID atom_Float;
	LV2_URID atom_Double;
	LV2_URID atom_Int;
	LV2_URID atom_Object;
	LV2_URID atom_Blank;
	LV2_URID atom_eventTransfer;
	LV2_URID atom_Vector;
	LV2_URID atom_String;
	LV2_URID atom_Path;
	LV2_URID midi_Event;
	LV2_URID bharvestr_uiOn;
	LV2_URID bharvestr_uiOff;
	LV2_URID bharvestr_samplePlay;
	LV2_URID bharvestr_sampleStop;
	LV2_URID bharvestr_selectionPlay;
	LV2_URID bharvestr_selectionStop;
	LV2_URID bharvestr_patternEvent;
	LV2_URID bharvestr_pattern;
	LV2_URID bharvestr_sampleFreeEvent;
	LV2_URID bharvestr_sampleSetEvent;
	LV2_URID bharvestr_sampleEvent;
	LV2_URID bharvestr_sampleSize;
	LV2_URID bharvestr_sampleData;
	LV2_URID bharvestr_samplePath;
	LV2_URID bharvestr_displayEvent;
	LV2_URID bharvestr_sampleDisplayData;
	LV2_URID bharvestr_selectionDisplayData;
	LV2_URID bharvestr_shapeEvent;
	LV2_URID bharvestr_shapeIndex;
	LV2_URID bharvestr_shapeData;
	LV2_URID bharvestr_presetInfoEvent;
	LV2_URID bharvestr_presetInfoName;
	LV2_URID bharvestr_presetInfoType;
	LV2_URID bharvestr_presetInfoDate;
	LV2_URID bharvestr_presetInfoCreator;
	LV2_URID bharvestr_presetInfoURI;
	LV2_URID bharvestr_presetInfoLicense;
	LV2_URID bharvestr_presetInfoDescription;
	LV2_URID bharvestr_statusEvent;
	LV2_URID bharvestr_statusTime;
	LV2_URID bharvestr_statusLfoPositions;
	LV2_URID bharvestr_statusSeqPositions;
	LV2_URID bharvestr_statusEnvPositions;
	LV2_URID bharvestr_statusGrainProperties;
	LV2_URID bharvestr_statusPatternPositions;
	LV2_URID bharvestr_keyboardEvent;
	LV2_URID bharvestr_keyOn;
	LV2_URID bharvestr_keyOff;
	LV2_URID bharvestr_messageEvent;
	LV2_URID bharvestr_message;
};

void getURIs (LV2_URID_Map* m, BHarvestrURIs* uris)
{
	uris->atom_Float = m->map(m->handle, LV2_ATOM__Float);
	uris->atom_Double = m->map(m->handle, LV2_ATOM__Double);
	uris->atom_Int = m->map(m->handle, LV2_ATOM__Int);
	uris->atom_Object = m->map(m->handle, LV2_ATOM__Object);
	uris->atom_Blank = m->map(m->handle, LV2_ATOM__Blank);
	uris->atom_eventTransfer = m->map(m->handle, LV2_ATOM__eventTransfer);
	uris->atom_Vector = m->map(m->handle, LV2_ATOM__Vector);
	uris->atom_String = m->map (m->handle, LV2_ATOM__String);
	uris->atom_Path = m->map(m->handle, LV2_ATOM__Path);
	uris->midi_Event = m->map(m->handle, LV2_MIDI__MidiEvent);
	uris->bharvestr_uiOn = m->map(m->handle, BHARVESTR_URI "#uiOn");
	uris->bharvestr_uiOff = m->map(m->handle, BHARVESTR_URI "#uiOff");
	uris->bharvestr_samplePlay = m->map(m->handle, BHARVESTR_URI "#samplePlay");
	uris->bharvestr_sampleStop = m->map(m->handle, BHARVESTR_URI "#sampleStop");
	uris->bharvestr_selectionPlay = m->map(m->handle, BHARVESTR_URI "#selectionPlay");
	uris->bharvestr_selectionStop = m->map(m->handle, BHARVESTR_URI "#selectionStop");
	uris->bharvestr_patternEvent = m->map(m->handle, BHARVESTR_URI "#patternEvent");
	uris->bharvestr_pattern = m->map(m->handle, BHARVESTR_URI "#pattern");
	uris->bharvestr_sampleFreeEvent = m->map(m->handle, BHARVESTR_URI "#sampleFreeEvent");
	uris->bharvestr_sampleSetEvent = m->map(m->handle, BHARVESTR_URI "#sampleSetEvent");
	uris->bharvestr_sampleEvent = m->map(m->handle, BHARVESTR_URI "#sampleEvent");
	uris->bharvestr_sampleSize = m->map(m->handle, BHARVESTR_URI "#sampleSize");
	uris->bharvestr_sampleData = m->map(m->handle, BHARVESTR_URI "#sampleData");
	uris->bharvestr_samplePath = m->map(m->handle, BHARVESTR_URI "#samplePath");
	uris->bharvestr_displayEvent = m->map(m->handle, BHARVESTR_URI "#displayEvent");
	uris->bharvestr_sampleDisplayData = m->map(m->handle, BHARVESTR_URI "#sampleDisplayData");
	uris->bharvestr_selectionDisplayData = m->map(m->handle, BHARVESTR_URI "#selectionDisplayData");
	uris->bharvestr_shapeEvent = m->map(m->handle, BHARVESTR_URI "#shapeEvent");
	uris->bharvestr_shapeIndex = m->map(m->handle, BHARVESTR_URI "#shapeIndex");
	uris->bharvestr_shapeData = m->map(m->handle, BHARVESTR_URI "#shapeData");
	uris->bharvestr_presetInfoEvent = m->map(m->handle, BHARVESTR_URI "#presetInfoEvent");
	uris->bharvestr_presetInfoName = m->map(m->handle, BHARVESTR_URI "#presetInfoName");
	uris->bharvestr_presetInfoType = m->map(m->handle, BHARVESTR_URI "#presetInfoType");
	uris->bharvestr_presetInfoDate = m->map(m->handle, BHARVESTR_URI "#presetInfoDate");
	uris->bharvestr_presetInfoCreator = m->map(m->handle, BHARVESTR_URI "#presetInfoCreator");
	uris->bharvestr_presetInfoURI = m->map(m->handle, BHARVESTR_URI "#presetInfoURI");
	uris->bharvestr_presetInfoLicense = m->map(m->handle, BHARVESTR_URI "#presetInfoLicense");
	uris->bharvestr_presetInfoDescription = m->map(m->handle, BHARVESTR_URI "#presetInfoDescription");
	uris->bharvestr_statusEvent = m->map(m->handle, BHARVESTR_URI "#statusEvent");
	uris->bharvestr_statusTime = m->map(m->handle, BHARVESTR_URI "#statusTime");
	uris->bharvestr_statusLfoPositions = m->map(m->handle, BHARVESTR_URI "#statusLfoPositions");
	uris->bharvestr_statusSeqPositions = m->map(m->handle, BHARVESTR_URI "#statusSeqPositions");
	uris->bharvestr_statusEnvPositions = m->map(m->handle, BHARVESTR_URI "#statusEnvPositions");
	uris->bharvestr_statusGrainProperties = m->map(m->handle, BHARVESTR_URI "#statusGrainProperties");
	uris->bharvestr_statusPatternPositions = m->map(m->handle, BHARVESTR_URI "#statusPatternPositions");
	uris->bharvestr_keyboardEvent = m->map(m->handle, BHARVESTR_URI "#keyboardEvent");
	uris->bharvestr_keyOn = m->map(m->handle, BHARVESTR_URI "#keyOn");
	uris->bharvestr_keyOff = m->map(m->handle, BHARVESTR_URI "#keyOff");
	uris->bharvestr_messageEvent = m->map(m->handle, BHARVESTR_URI "#messageEvent");
	uris->bharvestr_message = m->map(m->handle, BHARVESTR_URI "#message");
}

#endif /* URIDS_HPP_ */
