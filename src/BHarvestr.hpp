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

#ifndef BHARVESTR_HPP_
#define BHARVESTR_HPP_

#define CONTROLLER_CHANGED(con) ((new_controllers[con]) ? (controllers[con] != *(new_controllers[con])) : false)

#include <cmath>
#include <climits>
#include <cstdlib>
#include <cstdio>
#include <string>
#include <vector>
#include <array>
#include <lv2/lv2plug.in/ns/lv2core/lv2.h>
#include <lv2/lv2plug.in/ns/ext/atom/atom.h>
#include <lv2/lv2plug.in/ns/ext/atom/util.h>
#include <lv2/lv2plug.in/ns/ext/atom/forge.h>
#include <lv2/lv2plug.in/ns/ext/urid/urid.h>
#include <lv2/lv2plug.in/ns/ext/midi/midi.h>
#include <lv2/lv2plug.in/ns/ext/state/state.h>
#include "lv2/lv2plug.in/ns/ext/worker/worker.h"
#include "LV2_Atom_Ptr.h"
#include "definitions.h"
#include "Ports.hpp"
#include "Urids.hpp"
#include "Limit.hpp"
#include "LFO.hpp"
#include "Sequencer.hpp"
#include "Envelope.hpp"
#include "Shape.hpp"
#include "Sample.hpp"
#include "Voice.hpp"
#include "Pattern.hpp"
#include "StaticArrayList.hpp"
#include "PresetInfo.hpp"

class BHarvestr
{
public:
	BHarvestr (double samplerate, const LV2_Feature* const* features);
	void connect_port(uint32_t port, void *data);
	void run(uint32_t n_samples);
	LV2_State_Status state_save(LV2_State_Store_Function store, LV2_State_Handle handle, uint32_t flags, const LV2_Feature* const* features);
	LV2_State_Status state_restore(LV2_State_Retrieve_Function retrieve, LV2_State_Handle handle, uint32_t flags, const LV2_Feature* const* features);
	LV2_Worker_Status work (LV2_Worker_Respond_Function respond, LV2_Worker_Respond_Handle handle, uint32_t size, const void* data);
	LV2_Worker_Status work_response (uint32_t size, const void* data);

private:
	Sample* loadSample (const char* path);
	void installSample (Sample* s);
	void noteOn (const uint8_t note, const uint8_t velocity, const uint64_t frame);
	void noteOff (const uint8_t note, const uint64_t frame);
	void play (const int start, const int end);
	double modulateGrainProperty (const Voice* voiceptr, const int property, const uint64_t frame) const;
	void notifyStatusToGui();
	void notifySampleStopToGui();
	void notifySelectionStopToGui();
	void notifyPatternToGui ();
	void notifySamplePathToGui ();
	void notifyDisplayToGui ();
	void notifyShapeToGui (ShapeIndex index);
	void notifyPresetInfoToGui ();
	void notifyMessageToGui();

	// URIs
	BHarvestrURIs uris;
	LV2_URID_Map* map;
	LV2_URID_Unmap* unmap;

	LV2_Worker_Schedule* workerSchedule;

	// Control ports: MIDI, DSP <-> GUI communication
	const LV2_Atom_Sequence* controlPort;
	LV2_Atom_Sequence* notifyPort;

	// Audio ports
	float* audioOutput1;
	float* audioOutput2;

	LV2_Atom_Forge notifyForge;
	LV2_Atom_Forge_Frame notifyFrame;

	// Controllers
	float* new_controllers [MAXCONTROLLERS];
	float controllers [MAXCONTROLLERS];
	const Limit controllerLimits [MAXCONTROLLERS] =
	{
		{1, 16, 1},		// MAX_VOICES
		{1, 63, 1},		// MAX_GRAINS_PER_VOICE
		{0, 1, 0},		// SAMPLE_START
		{0, 1, 0},		// SAMPLE_END
		{10, 20000, 0},		// SAMPLE_FREQ
		{0.01, 8.0, 0},		// PATTERN_SIZE
		{0, 4, 1},		// PATTERN_TYPE
		{1.0, 1000.0, 0},	// GRAIN_SIZE + PROPERTY_VALUE_START
		{1.0, 1000.0, 0},	// GRAIN_SIZE + PROPERTY_VALUE_END
		{0, 16, 1},		// GRAIN_SIZE + PROPERTY_MODULATORS
		{0, 16, 1},		// GRAIN_SIZE + PROPERTY_MODULATORS + 1
		{0, 16, 1},		// GRAIN_SIZE + PROPERTY_MODULATORS + 2
		{0, 16, 1},		// GRAIN_SIZE + PROPERTY_MODULATORS + 3
		{0.1, 100.0, 0},	// GRAIN_RATE + PROPERTY_VALUE_START
		{0.1, 100.0, 0},	// GRAIN_RATE + PROPERTY_VALUE_END
		{0, 16, 1},		// GRAIN_RATE + PROPERTY_MODULATORS
		{0, 16, 1},		// GRAIN_RATE + PROPERTY_MODULATORS + 1
		{0, 16, 1},		// GRAIN_RATE + PROPERTY_MODULATORS + 2
		{0, 16, 1},		// GRAIN_RATE + PROPERTY_MODULATORS + 3
		{-1.0, 1.0, 0},		// GRAIN_PHASE + PROPERTY_VALUE_START
		{-1.0, 1.0, 0},		// GRAIN_PHASE + PROPERTY_VALUE_END
		{0, 16, 1},		// GRAIN_PHASE + PROPERTY_MODULATORS
		{0, 16, 1},		// GRAIN_PHASE + PROPERTY_MODULATORS + 1
		{0, 16, 1},		// GRAIN_PHASE + PROPERTY_MODULATORS + 2
		{0, 16, 1},		// GRAIN_PHASE + PROPERTY_MODULATORS + 3
		{-1.0, 1.0, 0},		// GRAIN_DRIVE + PROPERTY_VALUE_START
		{-1.0, 1.0, 0},		// GRAIN_DRIVE + PROPERTY_VALUE_END
		{0, 16, 1},		// GRAIN_DRIVE + PROPERTY_MODULATORS
		{0, 16, 1},		// GRAIN_DRIVE + PROPERTY_MODULATORS + 1
		{0, 16, 1},		// GRAIN_DRIVE + PROPERTY_MODULATORS + 2
		{0, 16, 1},		// GRAIN_DRIVE + PROPERTY_MODULATORS + 3
		{0.0, 2.0, 0},		// GRAIN_LEVEL + PROPERTY_VALUE_START
		{0.0, 2.0, 0},		// GRAIN_LEVEL + PROPERTY_VALUE_END
		{0, 16, 1},		// GRAIN_LEVEL + PROPERTY_MODULATORS
		{0, 16, 1},		// GRAIN_LEVEL + PROPERTY_MODULATORS + 1
		{0, 16, 1},		// GRAIN_LEVEL + PROPERTY_MODULATORS + 2
		{0, 16, 1},		// GRAIN_LEVEL + PROPERTY_MODULATORS + 3
		{-1.0, 1.0, 0},		// GRAIN_PAN + PROPERTY_VALUE_START
		{-1.0, 1.0, 0},		// GRAIN_PAN + PROPERTY_VALUE_END
		{0, 16, 1},		// GRAIN_PAN + PROPERTY_MODULATORS
		{0, 16, 1},		// GRAIN_PAN + PROPERTY_MODULATORS + 1
		{0, 16, 1},		// GRAIN_PAN + PROPERTY_MODULATORS + 2
		{0, 16, 1},		// GRAIN_PAN + PROPERTY_MODULATORS + 3
		{-12.0, 12.0, 0},	// GRAIN_PITCH + PROPERTY_VALUE_START
		{-12.0, 12.0, 0},	// GRAIN_PITCH + PROPERTY_VALUE_END
		{0, 16, 1},		// GRAIN_PITCH + PROPERTY_MODULATORS
		{0, 16, 1},		// GRAIN_PITCH + PROPERTY_MODULATORS + 1
		{0, 16, 1},		// GRAIN_PITCH + PROPERTY_MODULATORS + 2
		{0, 16, 1},		// GRAIN_PITCH + PROPERTY_MODULATORS + 3
		{0, 7, 1},		// GRAIN_SHAPE
		{0, 13, 1},		// LFO_TYPE
		{0.1, 10.0, 0},		// LFO_FREQ
		{-1.0, 1.0, 0},		// LFO_PHASE
		{0.0, 1.0, 0},		// LFO_AMP
		{0, 13, 1},		// LFO_TYPE
		{0.1, 10.0, 0},		// LFO_FREQ
		{-1.0, 1.0, 0},		// LFO_PHASE
		{0.0, 1.0, 0},		// LFO_AMP
		{0, 13, 1},		// LFO_TYPE
		{0.1, 10.0, 0},		// LFO_FREQ
		{-1.0, 1.0, 0},		// LFO_PHASE
		{0.0, 1.0, 0},		// LFO_AMP
		{0, 13, 1},		// LFO_TYPE
		{0.1, 10.0, 0},		// LFO_FREQ
		{-1.0, 1.0, 0},		// LFO_PHASE
		{0.0, 1.0, 0},		// LFO_AMP
		{1, 16, 1},		// SEQ_SIZE
		{0.1, 10.0, 0},		// SEQ_FREQ
		{-1.0, 1.0, 0},		// SEQ_PHASE
		{0.0, 1.0, 0},		// SEQ_STEP
		{0.0, 1.0, 0},		// SEQ_STEP + 1
		{0.0, 1.0, 0},		// SEQ_STEP + 2
		{0.0, 1.0, 0},		// SEQ_STEP + 3
		{0.0, 1.0, 0},		// SEQ_STEP + 4
		{0.0, 1.0, 0},		// SEQ_STEP + 5
		{0.0, 1.0, 0},		// SEQ_STEP + 6
		{0.0, 1.0, 0},		// SEQ_STEP + 7
		{0.0, 1.0, 0},		// SEQ_STEP + 8
		{0.0, 1.0, 0},		// SEQ_STEP + 9
		{0.0, 1.0, 0},		// SEQ_STEP + 10
		{0.0, 1.0, 0},		// SEQ_STEP + 11
		{0.0, 1.0, 0},		// SEQ_STEP + 12
		{0.0, 1.0, 0},		// SEQ_STEP + 13
		{0.0, 1.0, 0},		// SEQ_STEP + 14
		{0.0, 1.0, 0},		// SEQ_STEP + 15
		{1, 16, 1},		// SEQ_SIZE
		{0.1, 10.0, 0},		// SEQ_FREQ
		{-1.0, 1.0, 0},		// SEQ_PHASE
		{0.0, 1.0, 0},		// SEQ_STEP
		{0.0, 1.0, 0},		// SEQ_STEP + 1
		{0.0, 1.0, 0},		// SEQ_STEP + 2
		{0.0, 1.0, 0},		// SEQ_STEP + 3
		{0.0, 1.0, 0},		// SEQ_STEP + 4
		{0.0, 1.0, 0},		// SEQ_STEP + 5
		{0.0, 1.0, 0},		// SEQ_STEP + 6
		{0.0, 1.0, 0},		// SEQ_STEP + 7
		{0.0, 1.0, 0},		// SEQ_STEP + 8
		{0.0, 1.0, 0},		// SEQ_STEP + 9
		{0.0, 1.0, 0},		// SEQ_STEP + 10
		{0.0, 1.0, 0},		// SEQ_STEP + 11
		{0.0, 1.0, 0},		// SEQ_STEP + 12
		{0.0, 1.0, 0},		// SEQ_STEP + 13
		{0.0, 1.0, 0},		// SEQ_STEP + 14
		{0.0, 1.0, 0},		// SEQ_STEP + 15
		{1, 16, 1},		// SEQ_SIZE
		{0.1, 10.0, 0},		// SEQ_FREQ
		{-1.0, 1.0, 0},		// SEQ_PHASE
		{0.0, 1.0, 0},		// SEQ_STEP
		{0.0, 1.0, 0},		// SEQ_STEP + 1
		{0.0, 1.0, 0},		// SEQ_STEP + 2
		{0.0, 1.0, 0},		// SEQ_STEP + 3
		{0.0, 1.0, 0},		// SEQ_STEP + 4
		{0.0, 1.0, 0},		// SEQ_STEP + 5
		{0.0, 1.0, 0},		// SEQ_STEP + 6
		{0.0, 1.0, 0},		// SEQ_STEP + 7
		{0.0, 1.0, 0},		// SEQ_STEP + 8
		{0.0, 1.0, 0},		// SEQ_STEP + 9
		{0.0, 1.0, 0},		// SEQ_STEP + 10
		{0.0, 1.0, 0},		// SEQ_STEP + 11
		{0.0, 1.0, 0},		// SEQ_STEP + 12
		{0.0, 1.0, 0},		// SEQ_STEP + 13
		{0.0, 1.0, 0},		// SEQ_STEP + 14
		{0.0, 1.0, 0},		// SEQ_STEP + 15
		{1, 16, 1},		// SEQ_SIZE
		{0.1, 10.0, 0},		// SEQ_FREQ
		{-1.0, 1.0, 0},		// SEQ_PHASE
		{0.0, 1.0, 0},		// SEQ_STEP
		{0.0, 1.0, 0},		// SEQ_STEP + 1
		{0.0, 1.0, 0},		// SEQ_STEP + 2
		{0.0, 1.0, 0},		// SEQ_STEP + 3
		{0.0, 1.0, 0},		// SEQ_STEP + 4
		{0.0, 1.0, 0},		// SEQ_STEP + 5
		{0.0, 1.0, 0},		// SEQ_STEP + 6
		{0.0, 1.0, 0},		// SEQ_STEP + 7
		{0.0, 1.0, 0},		// SEQ_STEP + 8
		{0.0, 1.0, 0},		// SEQ_STEP + 9
		{0.0, 1.0, 0},		// SEQ_STEP + 10
		{0.0, 1.0, 0},		// SEQ_STEP + 11
		{0.0, 1.0, 0},		// SEQ_STEP + 12
		{0.0, 1.0, 0},		// SEQ_STEP + 13
		{0.0, 1.0, 0},		// SEQ_STEP + 14
		{0.0, 1.0, 0},		// SEQ_STEP + 15
		{0.0, 1.0, 0},		// RND_MIN
		{0.0, 1.0, 0},		// RND_MAX
		{0.0, 1.0, 0},		// RND_MIN
		{0.0, 1.0, 0},		// RND_MAX
		{0.0, 1.0, 0},		// RND_MIN
		{0.0, 1.0, 0},		// RND_MAX
		{0.0, 1.0, 0},		// RND_MIN
		{0.0, 1.0, 0},		// RND_MAX
		{0.0, 4.0, 0},		// ENV_ATTACK
		{0.0, 4.0, 0},		// ENV_DECAY
		{0.0, 1.0, 0},		// ENV_SUSTAIN
		{0.0, 4.0, 0},		// ENV_RELEASE
		{0.0, 4.0, 0},		// ENV_ATTACK
		{0.0, 4.0, 0},		// ENV_DECAY
		{0.0, 1.0, 0},		// ENV_SUSTAIN
		{0.0, 4.0, 0},		// ENV_RELEASE
		{0.0, 4.0, 0},		// ENV_ATTACK
		{0.0, 4.0, 0},		// ENV_DECAY
		{0.0, 1.0, 0},		// ENV_SUSTAIN
		{0.0, 4.0, 0},		// ENV_RELEASE
		{0.0, 4.0, 0},		// ENV_ATTACK
		{0.0, 4.0, 0},		// ENV_DECAY
		{0.0, 1.0, 0},		// ENV_SUSTAIN
		{0.0, 4.0, 0},		// ENV_RELEASE
		{0, 3, 1}		// SYNTH_ENV
	};

	// Pattern
	Pattern pattern;

	// Sample
	Sample* sample;

	// Voices
	StaticArrayList<Voice, MAXVOICES> voices;

	Lfo lfo[NR_LFOS];
	Sequencer<NR_SEQ_STEPS> seq[NR_SEQS];
	Envelope env[NR_ENVS];
	Shape<MAXNODES> shape[MAXSHAPES];
	PresetInfo presetInfo;

	// Host communicated data
	double rate;

	// Position data
	uint64_t frame;
	uint64_t sampleFrame;
	uint64_t sampleSelectionFrame;

	// Internals
	struct Notify
	{
		bool sampleStop;
		bool selectionStop;
		bool pattern;
		bool sampleDisplay;
		bool selectionDisplay;
		bool samplePath;
		bool shape [USER_SHAPES + NR_USER_SHAPES];
		bool presetInfo;
		bool message;
	};

	bool ui_on;
	Notify notify;
	char message[256];
};

#endif /* BHARVESTR_HPP_ */
