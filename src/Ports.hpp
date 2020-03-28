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

#ifndef PORTS_HPP_
#define PORTS_HPP_

enum PortIndex {
	// Atom data ports
	CONTROL			= 0,	// Time, MIDI, GUI->DSP
	NOTIFY			= 1,	// DSP->GUI

	// Audio ports
	AUDIO_OUT_1		= 2,
	AUDIO_OUT_2		= 3,

	CONTROLLERS		= 4,
	// Global plugin definitions
	MAX_VOICES		= 0,
	MAX_GRAINS_PER_VOICE	= 1,

	// Sample properies
	SAMPLE_START		= 2,
	SAMPLE_END		= 3,
	SAMPLE_FREQ		= 4,

	// Motion pattern properties
	PATTERN_SIZE		= 5,
	PATTERN_TYPE		= 6,

	// Grain properites
	PROPERTIES		= 7,
	PROPERTY_VALUE_START	= 0,
	PROPERTY_VALUE_END	= 1,
	PROPERTY_MODULATORS	= 2,
	NR_PROPERTY_MODULATORS	= 4,
	PROPERTIES_SIZE		= PROPERTY_MODULATORS + NR_PROPERTY_MODULATORS,

	GRAIN_SIZE		= 0,
	GRAIN_RATE		= 1,
	GRAIN_PHASE		= 2,
	GRAIN_DRIVE		= 3,
	GRAIN_LEVEL		= 4,
	GRAIN_PAN		= 5,
	GRAIN_PITCH		= 6,
	NR_GRAIN_PROPERTIES	= 7,

	// Shapes
	GRAIN_SHAPE		= PROPERTIES + NR_GRAIN_PROPERTIES * PROPERTIES_SIZE,

	// LFOs
	LFOS			= GRAIN_SHAPE + 1,
	LFO_TYPE		= 0,
	LFO_FREQ		= 1,
	LFO_PHASE		= 2,
	LFO_AMP			= 3,
	LFO_SIZE		= 4,
	NR_LFOS			= 4,

	// Sequencers
	SEQS			= LFOS + LFO_SIZE * NR_LFOS,
	SEQ_CHS			= 0,
	SEQ_FREQ		= 1,
	SEQ_PHASE		= 2,
	SEQ_STEPS		= 3,
	NR_SEQ_STEPS		= 16,
	SEQ_SIZE		= SEQ_STEPS + NR_SEQ_STEPS,
	NR_SEQS			= 4,

	// Random engines
	RNDS			= SEQS + SEQ_SIZE * NR_SEQS,
	RND_MIN			= 0,
	RND_MAX			= 1,
	RND_SIZE		= 2,
	NR_RNDS			= 4,

	// Envelopes
	ENVS			= RNDS + RND_SIZE * NR_RNDS,
	ENV_ATTACK		= 0,
	ENV_DECAY		= 1,
	ENV_SUSTAIN		= 2,
	ENV_RELEASE		= 3,
	ENV_SIZE		= 4,
	NR_ENVS			= 4,

	// Synth
	SYNTH_ENV		= ENVS + ENV_SIZE * NR_ENVS,

	MAXCONTROLLERS		= SYNTH_ENV + 1
};

enum ShapeIndex
{
	SINE_SHAPE		= 0,
	HALF_SINE_SHAPE		= 1,
	TRIANGLE_SHAPE		= 2,
	TRAPEZ_SHAPE		= 3,
	USER_SHAPES		= 4,
	NR_USER_SHAPES		= 4,
};

enum PlaybackIndex
{
	PLAY_SAMPLE	= 0,
	PLAY_SELECTION	= 1
};

enum ModulatorIndex
{
	MODULATOR_NONE		= 0,
	MODULATOR_LFO1		= 1,
	MODULATOR_LFO2		= 2,
	MODULATOR_LFO3		= 3,
	MODULATOR_LFO4		= 4,
	MODULATOR_SEQ1		= 5,
	MODULATOR_SEQ2		= 6,
	MODULATOR_SEQ3		= 7,
	MODULATOR_SEQ4		= 8,
	MODULATOR_ENV1		= 9,
	MODULATOR_ENV2		= 10,
	MODULATOR_ENV3		= 11,
	MODULATOR_ENV4		= 12,
	MODULATOR_RANDOM1	= 13,
	MODULATOR_RANDOM2	= 14,
	MODULATOR_RANDOM3	= 15,
	MODULATOR_RANDOM4	= 16
};

enum PatchInfoIndex
{
	PATCH_NAME		= 0,
	PATCH_TYPE		= 1,
	PATCH_DATE		= 2,
	PATCH_CREATOR		= 3,
	PATCH_URI		= 4,
	PATCH_LICENSE		= 5,
	PATCH_DESCRIPTION	= 6,
	NR_PATCHES		= 7
};

#endif /* PORTS_HPP_ */
