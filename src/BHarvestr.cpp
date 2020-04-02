/* B.Harvestr
 * LV2 Plugin
 *
 * Copyright (C) 2018 by Sven Jähnichen
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

#include "BHarvestr.hpp"
#include <ctime>

inline double framesToSeconds (const uint64_t frame, const double rate) {return (double (frame)) / rate;}
inline double framesToMilliseconds (const uint64_t frame, const double rate) {return 1000.0 * (double (frame)) / rate;}
inline uint64_t secondsToFrames (const double s, const double rate) {return rate * s;}
inline uint64_t millisecondsToFrames (const double ms, const double rate) {return rate * ms / 1000.0;}
inline double noteToFrequency (const double note) {return pow (2.0, (note - 69.0) / 12.0) * 440.0;}
inline float modulatef (const float min, const float max, const float mod) {return (min == max ? min : min + mod * (max - min));}
inline float fracf (const float value) {return value - floor (value);}

BHarvestr::BHarvestr (double samplerate, const LV2_Feature* const* features) :
	map (NULL), unmap (NULL), workerSchedule (NULL),
	controlPort (nullptr), notifyPort (nullptr),
	audioOutput1 (nullptr), audioOutput2 (nullptr),
	notifyForge (), notifyFrame (),
	new_controllers {nullptr}, controllers {0},
	pattern (), sample (nullptr), voices (),
	lfo {Lfo()},
	seq {Sequencer<NR_SEQ_STEPS>()},
	env {Envelope {0.2, 0.2, 0.8, 0.4}},
	shape {Shape<MAXNODES>()},
	presetInfo {"", "", 0, "", "", "", ""},
	rate (samplerate),
	frame (0),
	sampleFrame (0xFFFFFFFFFFFFFFFF), sampleSelectionFrame (0xFFFFFFFFFFFFFFFF),
	ui_on (false),
	notify {false, false, false, false, false, false, {false}, false, false},
	message ("")

{
	//Scan host features for URID map
	LV2_URID_Map* m = NULL;
	LV2_URID_Unmap* u = NULL;
	for (int i = 0; features[i]; ++i)
	{
		if (strcmp (features[i]->URI, LV2_URID__map) == 0)
		{
			m = (LV2_URID_Map*) features[i]->data;
		}

		else if (strcmp (features[i]->URI, LV2_URID__unmap) == 0)
		{
			u = (LV2_URID_Unmap*) features[i]->data;
		}

		else if (!strcmp(features[i]->URI, LV2_WORKER__schedule))
		{
                        workerSchedule = (LV2_Worker_Schedule*)features[i]->data;
		}
	}

	if (!m) throw std::invalid_argument ("BHarvestr.lv2: Host does not support urid:map.");
	if (!workerSchedule) throw std::invalid_argument ("BHarvestr.lv2: Host does not support work:schedule.");

	//Map URIS
	map = m;
	unmap = u;
	getURIs (m, &uris);

	// Initialize notify
	lv2_atom_forge_init (&notifyForge, map);

	// Init presetInfo
	memset (&presetInfo, 0, sizeof (presetInfo));

	// Init shapes
	for (int i = 0; i < USER_SHAPES + NR_USER_SHAPES; ++i) shape[i].setDefaultShape ();
	shape[SINE_SHAPE].insertNode ({CORNER_NODE, {0.001, 0.0}, {0.0, 0.0}, {0.1821, 0.0}});
	shape[SINE_SHAPE].insertNode ({CORNER_NODE, {0.5, 1.0}, {-0.1821, 0.0}, {0.1821, 0.0}});
	shape[SINE_SHAPE].insertNode ({CORNER_NODE, {0.999, 0.0}, {-0.1821, 0.0}, {0.0, 0.0}});
	shape[HALF_SINE_SHAPE].insertNode ({CORNER_NODE, {0.001, 0.0}, {0.0, 0.0}, {0.1821, 0.5}});
	shape[HALF_SINE_SHAPE].insertNode ({CORNER_NODE, {0.5, 1.0}, {-0.1821, 0.0}, {0.1821, 0.0}});
	shape[HALF_SINE_SHAPE].insertNode ({CORNER_NODE, {0.999, 0.0}, {-0.1821, 0.5}, {0.0, 0.0}});
	shape[TRIANGLE_SHAPE].insertNode ({POINT_NODE, {0.5, 1.0}, {0.0, 0.0}, {0.0, 0.0}});
	shape[TRAPEZ_SHAPE].insertNode ({POINT_NODE, {0.25, 1.0}, {0.0, 0.0}, {0.0, 0.0}});
	shape[TRAPEZ_SHAPE].insertNode ({POINT_NODE, {0.75, 1.0}, {0.0, 0.0}, {0.0, 0.0}});

	// Init random engine
	srand (time (0));

	ui_on = false;
}

void BHarvestr::connect_port (uint32_t port, void *data)
{
	switch (port)
	{
		case CONTROL:
			controlPort = (LV2_Atom_Sequence*) data;
			break;
		case NOTIFY:
			notifyPort = (LV2_Atom_Sequence*) data;
			break;
		case AUDIO_OUT_1:
			audioOutput1 = (float*) data;
			break;
		case AUDIO_OUT_2:
			audioOutput2 = (float*) data;
			break;
		default:
			// Connect controllers
			if ((port >= CONTROLLERS) && (port < CONTROLLERS + MAXCONTROLLERS)) new_controllers[port - CONTROLLERS] = (float*) data;
	}
}

void BHarvestr::run (uint32_t n_samples)
{
	uint32_t last_t = 0;

	if ((!controlPort) || (!notifyPort) || (!audioOutput1) || (!audioOutput2)) return;

	// Init notify port
	uint32_t space = notifyPort->atom.size;
	lv2_atom_forge_set_buffer(&notifyForge, (uint8_t*) notifyPort, space);
	lv2_atom_forge_sequence_head(&notifyForge, &notifyFrame, 0);

	// Validate controllers
	for (int i = 0; i < MAXCONTROLLERS; ++i)
	{
		if (new_controllers[i])
		{
			float val = controllerLimits[i].validate (*(new_controllers[i]));
			if (val != *(new_controllers[i]))
			{
				fprintf (stderr, "BHarvestr.lv2: Value out of range in run (): Controller#%i\n", i);
				*(new_controllers[i]) = val;
				// TODO update GUI controller
			}

			if (controllers[i] != val)
			{
				controllers[i] = val;

				if (i == SAMPLE_START)
				{
					if (controllers[SAMPLE_END] < val) controllers[SAMPLE_END] = val;
					notify.selectionDisplay = true;
				}

				else if (i == SAMPLE_END)
				{
					if (controllers[SAMPLE_START] > val) controllers[SAMPLE_START] = val;
					notify.selectionDisplay = true;
				}

				else if (i == PATTERN_TYPE) pattern.setPattern (PatternIndex (int (val)));

				else if (i == PATTERN_SIZE) pattern.setSteps (pattern.getRows() * val);

				else if ((i >= LFOS) && (i < LFOS + LFO_SIZE * NR_LFOS))
				{
					int nr = (i - LFOS) / LFO_SIZE;
					int param = (i - LFOS) % LFO_SIZE;

					switch (param)
					{
						case LFO_TYPE:	lfo[nr].setType (LfoIndex (int (val)));
								break;

						case LFO_FREQ:	lfo[nr].setFrequency (val, framesToSeconds (frame, rate));
								break;

						case LFO_PHASE:	lfo[nr].setPhase (val);
								break;

						default:	break;
					}
				}

				else if ((i >= SEQS) && (i < SEQS + SEQ_SIZE * NR_SEQS))
				{
					int nr = (i - SEQS) / SEQ_SIZE;
					int param = (i - SEQS) % SEQ_SIZE;

					switch (param)
					{
						case SEQ_CHS:	seq[nr].setSize (val);
								break;

						case SEQ_FREQ:	seq[nr].setFrequency (val, framesToSeconds (frame, rate));
								break;

						case SEQ_PHASE:	seq[nr].setPhase (val);
								break;

						default:	seq[nr].setStep (param - SEQ_STEPS, val);
					}
				}

				else if ((i >= ENVS) && (i < ENVS + ENV_SIZE * NR_ENVS))
				{
					int nr = (i - ENVS) / ENV_SIZE;
					int param = (i - ENVS) % ENV_SIZE;

					switch (param)
					{
						case ENV_ATTACK:	env[nr].attack = val;
									break;

						case ENV_DECAY:		env[nr].decay = val;
									break;

						case ENV_SUSTAIN:	env[nr].sustain = val;
									break;

						case ENV_RELEASE:	env[nr].release = val;
									break;

						default:		break;
					}
				}

				// Recalculate pattern.rows_
				if
				(
					(i == SAMPLE_START) ||
					(i == SAMPLE_END) ||
					(i == PROPERTIES + GRAIN_SIZE * PROPERTIES_SIZE + PROPERTY_VALUE_START) ||
					(i == PROPERTIES + GRAIN_SIZE * PROPERTIES_SIZE + PROPERTY_VALUE_END)
				)
				{
					if (!sample) pattern.setRows (1);
					else
					{
						float grainSize =
						(
							controllers[PROPERTIES + GRAIN_SIZE * PROPERTIES_SIZE + PROPERTY_VALUE_START] +
							controllers[PROPERTIES + GRAIN_SIZE * PROPERTIES_SIZE + PROPERTY_VALUE_END]
						) / 2.0f;

						if (grainSize != 0.0f)
						{
							float selectionSize = (controllers[SAMPLE_END] - controllers[SAMPLE_START]) *
									      framesToSeconds(sample->info.frames, rate) * 1000.0f;
							int grainsPerSelection = ceil (selectionSize / grainSize);
							pattern.setRows (grainsPerSelection);
						}
						else pattern.setRows (1);
					}
				}
			}
		}
	}

	// Read CONTROL port (notifications from GUI and host)
	LV2_ATOM_SEQUENCE_FOREACH (controlPort, ev)
	{
		if ((ev->body.type == uris.atom_Object) || (ev->body.type == uris.atom_Blank))
		{
			const LV2_Atom_Object* obj = (const LV2_Atom_Object*)&ev->body;

			// GUI on
			if (obj->body.otype == uris.bharvestr_uiOn)
			{
				ui_on = true;
				notify.pattern = true;
				notify.sampleDisplay = true;
				notify.selectionDisplay = true;
				notify.samplePath = true;
				for (int i = 0; i < USER_SHAPES + NR_USER_SHAPES; ++i) notify.shape[i] = true;
				notify.presetInfo = true;
			}

			// GUI off
			else if (obj->body.otype == uris.bharvestr_uiOff) ui_on = false;

			// Sample/Selection playback started/stopped
			else if (obj->body.otype == uris.bharvestr_samplePlay) sampleFrame = 0;
			else if (obj->body.otype == uris.bharvestr_sampleStop) sampleFrame = 0xFFFFFFFFFFFFFFFF;
			else if (obj->body.otype == uris.bharvestr_selectionPlay) sampleSelectionFrame = 0;
			else if (obj->body.otype == uris.bharvestr_selectionStop) sampleSelectionFrame = 0xFFFFFFFFFFFFFFFF;

			// Pattern changed notifications
			else if (obj->body.otype == uris.bharvestr_patternEvent)
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
					pattern.setRows (rows);
				}

				// Pattern notification
				if (oPat && (oPat->type == uris.atom_Vector))
				{
					const LV2_Atom_Vector* vec = (const LV2_Atom_Vector*) oPat;
					if (vec->body.child_type == uris.atom_Int)
					{
						const uint32_t size = (uint32_t) ((oPat->size - sizeof(LV2_Atom_Vector_Body)) / sizeof (int));
						int* new_pattern = (int*) (&vec->body + 1);
						pattern.setPattern (new_pattern, size);
						controllers[PATTERN_TYPE] = USER_PATTERN;
						notify.pattern = true;
					}
				}

				if (oPs && (oPs->type == uris.atom_Int))
				{
					const int steps = ((const LV2_Atom_Int*)oPs)->body;
					pattern.setSteps (steps);
				}
			}

			// Sample path notification -> forward to worker
			else if (obj->body.otype == uris.bharvestr_sampleEvent)
			{
				LV2_Atom* oPath = NULL;
				lv2_atom_object_get (obj, uris.bharvestr_samplePath, &oPath, NULL);

				if (oPath && (oPath->type == uris.atom_Path))
				{
					workerSchedule->schedule_work (workerSchedule->handle, lv2_atom_total_size(&ev->body), &ev->body);
				}
			}

			// Shape changed notifications
			else if (obj->body.otype == uris.bharvestr_shapeEvent)
			{
				LV2_Atom *oId = NULL, *oData = NULL;
				lv2_atom_object_get (obj,
						     uris.bharvestr_shapeIndex, &oId,
						     uris.bharvestr_shapeData, &oData,
						     NULL);

				if (oId && (oId->type == uris.atom_Int) && oData && (oData->type == uris.atom_Vector))
				{
					const int index = ((const LV2_Atom_Int*)oId)->body;
					if ((index >= USER_SHAPES) && (index < USER_SHAPES + NR_USER_SHAPES))
					{
						const LV2_Atom_Vector* vec = (const LV2_Atom_Vector*) oData;
						if (vec->body.child_type == uris.atom_Float)
						{
							shape[index].clearShape ();
							const uint32_t vecSize = (uint32_t) ((oData->size - sizeof(LV2_Atom_Vector_Body)) / (7 * sizeof (float)));
							float* data = (float*) (&vec->body + 1);
							for (unsigned int i = 0; (i < vecSize) && (i < MAXNODES); ++i)
							{
								Node node;
								node.nodeType = NodeType (int (data[i * 7]));
								node.point.x = data[i * 7 + 1];
								node.point.y = data[i * 7 + 2];
								node.handle1.x = data[i * 7 + 3];
								node.handle1.y = data[i * 7 + 4];
								node.handle2.x = data[i * 7 + 5];
								node.handle2.y = data[i * 7 + 6];
								shape[index].appendRawNode (node);
							}
							shape[index].validateShape();
						}
					}
				}
			}

			// Preset info notification
			else if (obj->body.otype ==uris.bharvestr_presetInfoEvent)
			{
				const LV2_Atom *oName = NULL, *oType = NULL, *oDate = NULL, *oCreator = NULL, *oUri = NULL, *oLicense = NULL, *oDescription = NULL ;
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

				if (oName && (oName->type == uris.atom_String)) strncpy (presetInfo.name, (const char*)LV2_ATOM_BODY_CONST (oName), PRESETINFO_MAX_TXT_SIZE - 1);
				if (oType && (oType->type == uris.atom_String)) strncpy (presetInfo.type, (const char*)LV2_ATOM_BODY_CONST (oType), PRESETINFO_MAX_TXT_SIZE - 1);
				if (oDate && (oDate->type == uris.atom_Int)) presetInfo.date = ((const LV2_Atom_Int*)oDate)->body;
				if (oCreator && (oCreator->type == uris.atom_String)) strncpy (presetInfo.creator, (const char*)LV2_ATOM_BODY_CONST (oCreator), PRESETINFO_MAX_TXT_SIZE - 1);
				if (oUri && (oUri->type == uris.atom_String)) strncpy (presetInfo.uri, (const char*)LV2_ATOM_BODY_CONST (oUri), PRESETINFO_MAX_TXT_SIZE - 1);
				if (oLicense && (oLicense->type == uris.atom_String)) strncpy (presetInfo.license, (const char*)LV2_ATOM_BODY_CONST (oLicense), PRESETINFO_MAX_TXT_SIZE - 1);
				if (oDescription && (oDescription->type == uris.atom_String)) strncpy (presetInfo.description, (const char*)LV2_ATOM_BODY_CONST (oDescription), PRESETINFO_MAX_TXT_SIZE - 1);
			}

			// Keyboard
			else if (obj->body.otype ==uris.bharvestr_keyboardEvent)
			{
				LV2_Atom *okOn = NULL, *okOff = NULL;
				lv2_atom_object_get (obj,
						     uris.bharvestr_keyOn, &okOn,
						     uris.bharvestr_keyOff, &okOff,
						     NULL);

				if (okOn && (okOn->type == uris.atom_Int))
				{
					const int note = ((const LV2_Atom_Int*)okOn)->body;
					noteOn (note, 64, frame + ev->time.frames);
				}

				if (okOff && (okOff->type == uris.atom_Int))
				{
					const int note = ((const LV2_Atom_Int*)okOff)->body;
					noteOff (note, frame + ev->time.frames);
				}
			}
		}

		// Read incoming MIDI_IN events
		else if (ev->body.type == uris.midi_Event)
		{
			const uint8_t* const msg = (const uint8_t*)(ev + 1);
			uint8_t typ = lv2_midi_message_type(msg);
			//uint8_t chn = msg[0] & 0x0F;
			uint8_t note = msg[1];

			switch (typ)
			{

				case LV2_MIDI_MSG_NOTE_ON:
				{
					if (msg[2] != 0)
					{
						noteOn (note, msg[2], frame + ev->time.frames);
						break;
					}
					// Otherwise continue with note off
				}

				case LV2_MIDI_MSG_NOTE_OFF:
				noteOff (note, frame + ev->time.frames);
				break;

				case LV2_MIDI_MSG_CONTROLLER:
				{
					switch (note)
					{

						// LV2_MIDI_CTL_SUSTAIN: Forward to all outputs
						case LV2_MIDI_CTL_SUSTAIN:

						// LV2_MIDI_CTL_ALL_SOUNDS_OFF: Stop all outputs
						case LV2_MIDI_CTL_ALL_SOUNDS_OFF:

						// LV2_MIDI_CTL_ALL_NOTES_OFF: Stop all outputs and delete all keys
						// As B.SEQuencer doesn't interpret LV2_MIDI_CTL_SUSTAIN itself, the
						// result is the same as in LV2_MIDI_CTL_ALL_SOUNDS_OFF
						case LV2_MIDI_CTL_ALL_NOTES_OFF:

						// All other MIDI signals
						default: break;
					}
				}

				default: break;
			}
		}

		else fprintf (stderr, "BHarvestr.lv2: Ignored event in Control port (otype = %i, %s)\n", ev->body.type,
					  (unmap ? unmap->unmap (unmap->handle, ev->body.type) : NULL));

		// Update for this iteration
		uint32_t next_t = (ev->time.frames < n_samples ? ev->time.frames : n_samples);
		play (last_t, next_t);
		last_t = next_t;
	}

	// Update for the remainder of the cycle
	if (last_t < n_samples) play (last_t, n_samples);

	// Update position in case of no new barBeat submitted on next call
	frame += n_samples;

	if (ui_on)
	{
		notifyStatusToGui ();
		if (notify.sampleStop) notifySampleStopToGui();
		if (notify.selectionStop) notifySelectionStopToGui();
		if (notify.pattern) notifyPatternToGui();
		if (notify.sampleDisplay || notify.selectionDisplay) notifyDisplayToGui();
		if (notify.samplePath) notifySamplePathToGui();
		for (int i = 0; i < USER_SHAPES + NR_USER_SHAPES; ++i)
		{
			if (notify.shape[i]) notifyShapeToGui (ShapeIndex (i));
		}
		if (notify.presetInfo) notifyPresetInfoToGui();
		if (notify.message) notifyMessageToGui();
	}
	lv2_atom_forge_pop(&notifyForge, &notifyFrame);
}

LV2_State_Status BHarvestr::state_save (LV2_State_Store_Function store, LV2_State_Handle handle, uint32_t flags,
			const LV2_Feature* const* features)
{
	LV2_State_Map_Path* map_path = NULL;
	for (int i = 0; features[i]; ++i)
	{
		if (!strcmp(features[i]->URI, LV2_STATE__mapPath))
		{
			map_path = (LV2_State_Map_Path*)features[i]->data;
			break;
		}
	}

	if (!map_path)
	{
		fprintf (stderr, "BHarvestr.lv2: Feature map_path not available! Can't save plugin status!\n" );
		return LV2_STATE_ERR_NO_FEATURE;
	}

	// Save sample path
	if (sample && sample->path)
	{
		char* abstrPath = map_path->abstract_path(map_path->handle, sample->path);
		store(handle, uris.bharvestr_samplePath, abstrPath, strlen (sample->path) + 1, uris.atom_Path, LV2_STATE_IS_POD | LV2_STATE_IS_PORTABLE);
		free (abstrPath);
	}

	// Save preset data
	if (presetInfo.name[0] != '\0') store (handle, uris.bharvestr_presetInfoName, presetInfo.name, strlen (presetInfo.name) + 1, uris.atom_String, LV2_STATE_IS_POD);
	if (presetInfo.type[0] != '\0') store (handle, uris.bharvestr_presetInfoType, presetInfo.type, strlen (presetInfo.type) + 1, uris.atom_String, LV2_STATE_IS_POD);
	if (presetInfo.date != 0) store (handle, uris.bharvestr_presetInfoDate, &presetInfo.date, sizeof(presetInfo.date), uris.atom_Int, LV2_STATE_IS_POD);
	if (presetInfo.creator[0] != '\0') store (handle, uris.bharvestr_presetInfoCreator, presetInfo.creator, strlen (presetInfo.creator) + 1, uris.atom_String, LV2_STATE_IS_POD);
	if (presetInfo.uri[0] != '\0') store (handle, uris.bharvestr_presetInfoURI,  presetInfo.uri, strlen (presetInfo.uri) + 1, uris.atom_String, LV2_STATE_IS_POD | LV2_STATE_IS_POD);
	if (presetInfo.license[0] != '\0') store (handle, uris.bharvestr_presetInfoLicense, presetInfo.license, strlen (presetInfo.license) + 1, uris.atom_String, LV2_STATE_IS_POD);
	if (presetInfo.description[0] != '\0') store (handle, uris.bharvestr_presetInfoDescription, presetInfo.description, strlen (presetInfo.description) + 1, uris.atom_String, LV2_STATE_IS_POD);


	// Store pattern
	if (controllers[PATTERN_TYPE] == USER_PATTERN)
	{
		char patternDataString[0x4010] = "\nPattern data:\n";
		const int patternSteps = pattern.getSteps();
		const int patternRows = pattern.getRows();

		for (int s = 0; s < MAXPATTERNSTEPS; ++s)
		{
			char valueString[16];
			int val = pattern.getValue (s);
			snprintf (valueString, 14, "val:%d;", val);
			if (s % 8 == 7) strcat (valueString, "\n");
			else strcat (valueString, " ");
			strcat (patternDataString, valueString);
		}
		store (handle, uris.bharvestr_patternRows, &patternRows, sizeof(patternRows), uris.atom_Int, LV2_STATE_IS_POD);
		store (handle, uris.bharvestr_patternSteps, &patternSteps, sizeof(patternSteps), uris.atom_Int, LV2_STATE_IS_POD);
		store (handle, uris.bharvestr_pattern, patternDataString, strlen (patternDataString) + 1, uris.atom_String, LV2_STATE_IS_POD);
	}
	return LV2_STATE_SUCCESS;
}

LV2_State_Status BHarvestr::state_restore (LV2_State_Retrieve_Function retrieve, LV2_State_Handle handle, uint32_t flags,
			const LV2_Feature* const* features)
{
	size_t   size;
	uint32_t type;
	uint32_t valflags;

	// Retireve sample path
	const void* pathData = retrieve (handle, uris.bharvestr_samplePath, &size, &type, &valflags);
        if (pathData)
	{
		const char* path = (const char*)pathData;
		Sample* s = loadSample (path);
		if (s)
		{
			if (sample) delete sample;
			installSample (s);
		}
        }

	// Retrieve preset data
	const void* presetNameData = retrieve (handle, uris.bharvestr_presetInfoName, &size, &type, &valflags);
        if (presetNameData)
	{
		const char* str = (const char*)presetNameData;
		strncpy (presetInfo.name, str, LIMIT (strlen (str) + 1, 0, PRESETINFO_MAX_TXT_SIZE - 1));
		notify.presetInfo = true;
        }

	const void* presetTypeData = retrieve (handle, uris.bharvestr_presetInfoType, &size, &type, &valflags);
        if (presetTypeData)
	{
		const char* str = (const char*)presetTypeData;
		strncpy (presetInfo.type, str, LIMIT (strlen (str) + 1, 0, PRESETINFO_MAX_TXT_SIZE - 1));
		notify.presetInfo = true;
        }

	const void* presetDateData = retrieve (handle, uris.bharvestr_presetInfoDate, &size, &type, &valflags);
        if (presetDateData)
	{
		presetInfo.date = *(int*)presetDateData;
		notify.presetInfo = true;
        }

	const void* presetCreatorData = retrieve (handle, uris.bharvestr_presetInfoCreator, &size, &type, &valflags);
        if (presetNameData)
	{
		const char* str = (const char*)presetCreatorData;
		strncpy (presetInfo.creator, str, LIMIT (strlen (str) + 1, 0, PRESETINFO_MAX_TXT_SIZE - 1));
		notify.presetInfo = true;
        }

	const void* presetUriData = retrieve (handle, uris.bharvestr_presetInfoURI, &size, &type, &valflags);
        if (presetUriData)
	{
		const char* str = (const char*)presetUriData;
		strncpy (presetInfo.uri, str, LIMIT (strlen (str) + 1, 0, PRESETINFO_MAX_TXT_SIZE - 1));
		notify.presetInfo = true;
        }

	const void* presetLicenseData = retrieve (handle, uris.bharvestr_presetInfoLicense, &size, &type, &valflags);
        if (presetLicenseData)
	{
		const char* str = (const char*)presetLicenseData;
		strncpy (presetInfo.license, str, LIMIT (strlen (str) + 1, 0, PRESETINFO_MAX_TXT_SIZE - 1));
		notify.presetInfo = true;
        }

	const void* presetDescriptionData = retrieve (handle, uris.bharvestr_presetInfoDescription, &size, &type, &valflags);
        if (presetDescriptionData)
	{
		const char* str = (const char*)presetDescriptionData;
		strncpy (presetInfo.description, str, LIMIT (strlen (str) + 1, 0, PRESETINFO_MAX_TXT_SIZE - 1));
		notify.presetInfo = true;
        }

	// Retrieve pattern data
	const void* patternRowsData = retrieve (handle, uris.bharvestr_patternRows, &size, &type, &valflags);
        if (patternRowsData)
	{
		const int rows = *(int*)patternRowsData;
		pattern.setRows (rows);
        }

	const void* patternStepsData = retrieve (handle, uris.bharvestr_patternSteps, &size, &type, &valflags);
        if (patternStepsData)
	{
		const int steps = *(int*)patternStepsData;
		pattern.setSteps (steps);
        }

	const void* patternData = retrieve (handle, uris.bharvestr_pattern, &size, &type, &valflags);
        if (patternData)
	{
		std::string str = (char*)patternData;

		// Parse retrieved data
		int step = 1;
		std::vector<int> data = {};
		while (!str.empty())
		{
			// Look for next "val:"
			size_t strPos = str.find ("val:");
			size_t nextPos = 0;
			if (strPos == std::string::npos) break;	// No "val:" found => end
			if (strPos + 4 > str.length()) break;	// Nothing more after "val:" => end
			str.erase (0, strPos + 4);
			if (step > MAXPATTERNSTEPS)
			{
				fprintf (stderr, "BHarvestr.lv2: Max. pattern size exceeded. Pattern data truncated at step %i.\n", step);
				break;
			}
			int val;
			try {val = std::stof (str, &nextPos);}
			catch  (const std::exception& e)
			{
				fprintf (stderr, "BHarvestr.lv2: Restore pattern incomplete. Can't parse step %i from \"%s...\"", step, str.substr (0, 63).c_str());
				break;
			}

			if (nextPos > 0) str.erase (0, nextPos);
			if ((val < 0) || (val >= MAXPATTERNSTEPS))
			{
				fprintf (stderr, "BHarvestr.lv2: Restore pattern incomplete. Invalid matrix data loaded for step %i.\n", step);
				break;
			}
			data.push_back (val);
			++step;
		}

		// Set pattern
		pattern.setPattern (USER_PATTERN);
		pattern.setPattern (data);
		controllers[PATTERN_TYPE] = USER_PATTERN;
		notify.pattern = true;
        }

	return LV2_STATE_SUCCESS;
}

LV2_Worker_Status BHarvestr::work (LV2_Worker_Respond_Function respond, LV2_Worker_Respond_Handle handle, uint32_t size, const void* data)
{
	const LV2_Atom* atom = (const LV2_Atom*)data;

	// Free old sample
        if (atom->type == uris.bharvestr_sampleFreeEvent)
	{
		const LV2_Atom_Ptr* workerMessage = (LV2_Atom_Ptr*) atom;
		if (workerMessage && workerMessage->data) delete (Sample*)workerMessage->data;
        }

	// Load sample
	else
	{
                const LV2_Atom_Object* obj = (const LV2_Atom_Object*)data;


		// Sample path event
		if (obj->body.otype == uris.bharvestr_sampleEvent)
		{
			const LV2_Atom* path = NULL;
			lv2_atom_object_get(obj, uris.bharvestr_samplePath, &path, 0);

			if (path && (path->type == uris.atom_Path))
			{
				Sample* s = loadSample ((const char*)LV2_ATOM_BODY_CONST(path));

				if (s)
				{
					LV2_Atom_Ptr responseMessage = LV2_Atom_Ptr {{sizeof (Sample*), uris.bharvestr_sampleSetEvent}, s};
					respond (handle, sizeof(responseMessage), &responseMessage);
				}
			}

			else return LV2_WORKER_ERR_UNKNOWN;
		}
        }

        return LV2_WORKER_SUCCESS;
}

LV2_Worker_Status BHarvestr::work_response (uint32_t size, const void* data)
{
	const LV2_Atom* atom = (const LV2_Atom*)data;

	// Set sample
	if (atom->type == uris.bharvestr_sampleSetEvent)
	{
		// Schedule free old sample
		LV2_Atom_Ptr workerMessage = LV2_Atom_Ptr {{sizeof (Sample*), uris.bharvestr_sampleFreeEvent}, sample};
		workerSchedule->schedule_work (workerSchedule->handle, sizeof (workerMessage), &workerMessage);

		// Install new sample from data
		const LV2_Atom_Ptr* atom_ptr = (const LV2_Atom_Ptr*) atom;
		installSample ((Sample*) atom_ptr->data);
	}

	else fprintf(stderr, "B.Harvestr.lv2: Worker response unknown.\n");
	return LV2_WORKER_SUCCESS;
}

Sample* BHarvestr::loadSample (const char* path)
{
	Sample* s = nullptr;
	try {s = new Sample (path);}
	catch (std::bad_alloc &ba)
	{
		fprintf (stderr, "BHarvestr.lv2: Can't allocate enough memory to open sample file.\n");
		strcpy (message, "BHarvestr.lv2: Can't allocate enough memory to open sample file.");
		notify.message = true;
	}
	catch (std::invalid_argument &ia)
	{
		fprintf (stderr, "%s\n", ia.what());
		strcpy (message, ia.what());
		notify.message = true;
	}
	return s;
}

void BHarvestr::installSample (Sample* s)
{
	sample = s;
	sampleFrame = 0xFFFFFFFFFFFFFFFF;
	sampleSelectionFrame = 0xFFFFFFFFFFFFFFFF;
	notify.sampleDisplay = true;
	notify.selectionDisplay = true;
	notify.sampleStop = true;
	notify.selectionStop = true;

	// Recalculate pattern.rows_
	if (sample)
		{
		float grainSize =
		(
			controllers[PROPERTIES + GRAIN_SIZE * PROPERTIES_SIZE + PROPERTY_VALUE_START] +
			controllers[PROPERTIES + GRAIN_SIZE * PROPERTIES_SIZE + PROPERTY_VALUE_END]
		) / 2.0f;

		if (grainSize != 0.0f)
		{
			float selectionSize = (controllers[SAMPLE_END] - controllers[SAMPLE_START]) *
					      framesToSeconds(sample->info.frames, rate) * 1000.0f;
			int grainsPerSelection = ceil (selectionSize / grainSize);
			pattern.setRows (grainsPerSelection);
		}
		else pattern.setRows (1);
	}
	else pattern.setRows (1);
}

void BHarvestr::noteOn (const uint8_t note, const uint8_t velocity, const uint64_t frame)
{
	bool newNote = true;

	// Scan if this is an additional midi message
	// (e.g., double note on, velocity changed)
	for (Voice** it = voices.begin(); it < voices.end(); ++it)
	{
		if (((**it).note == note) && (frame < (**it).endFrame))
		{
			(**it).velocity = velocity;
			newNote = false;
			break;
		}
	}

	// New voice
	if (newNote && (voices.size < controllers[MAX_VOICES]))
	{
		Voice voice = Voice (note, velocity, frame, 0xFFFFFFFF00000000);
		voice.grains.push_back (Grain {frame, 0xFFFFFFFF00000000, 0ul, 0l, 0.0, 0.0, 0.0});
		voices.push_back (voice);
	}
}
void BHarvestr::noteOff (const uint8_t note, const uint64_t frame)
{
	for (Voice** it = voices.begin(); it < voices.end(); ++it)
	{
		if (((**it).note == note) && (frame < (**it).endFrame))
		{
			(**it).endFrame = frame;
			const int envNr = controllers[SYNTH_ENV];
			const double pos = framesToSeconds (frame - (**it).startFrame, rate);
			(**it).endValue = env[envNr].getValue (true, pos);
		}
	}
}

void BHarvestr::play (const int start, const int end)
{
	if (end < start) return;
	if ((!sample) || (!sample->data))
	{
		memset (&audioOutput1[start], 0, (end - start) * sizeof (float));
		memset (&audioOutput2[start], 0, (end - start) * sizeof (float));
		return;
	}

	int envNr = controllers[SYNTH_ENV];
	uint64_t vReleaseFrames = secondsToFrames (env[envNr].release, rate);

	for (int i = start; i < end; ++i)
	{
		uint64_t iframe = frame + i;
		float isample1 = 0.0f;
		float isample2 = 0.0f;

		// Sample playback?
		if (sampleFrame != 0xFFFFFFFFFFFFFFFF)
		{
			if (sampleFrame >= uint64_t (sample->info.frames))
			{
				sampleFrame = 0xFFFFFFFFFFFFFFFF;
				notify.sampleStop = true;
			}
			else
			{
				isample1 += sample->data[sampleFrame * sample->info.channels];
				isample2 += sample->data[sampleFrame * sample->info.channels];
				++sampleFrame;
			}
		}

		// Selection playback?
		if (sampleSelectionFrame != 0xFFFFFFFFFFFFFFFF)
		{
			uint64_t start = controllers[SAMPLE_START] * sample->info.frames;
			uint64_t size = (controllers[SAMPLE_END] - controllers[SAMPLE_START]) * sample->info.frames;
			if ((sampleSelectionFrame >= size) || (start + sampleSelectionFrame >= uint64_t (sample->info.frames)))
			{
				sampleSelectionFrame = 0xFFFFFFFFFFFFFFFF;
				notify.selectionStop = true;
			}
			else
			{
				isample1 += sample->data[(start + sampleSelectionFrame) * sample->info.channels];
				isample2 += sample->data[(start + sampleSelectionFrame) * sample->info.channels];
				++sampleSelectionFrame;
			}
		}

		for (size_t v = 0; v < voices.size; ++v)
		{
			Voice& voice = voices[v];
			float vsample1 = 0.0f;
			float vsample2 = 0.0f;

			if ((iframe >= voice.startFrame) && (iframe <= voice.endFrame + vReleaseFrames))
			{
				// Remove outtimed grains
				bool done;
				do
				{
					done = true;
					for (Grain** git = voice.grains.begin (); git < voice.grains.end(); ++git)
					{
						if ((**git).endFrame < iframe)
						{
							voice.grains.erase (git);
							done = false;
							break;
						}
					}
				} while (!done);


				// Seed new grains
				if ((voice.grains.size > 0) && (voice.grains.back().startFrame <= iframe))
				{
					Grain& grain = voice.grains.back();

					// Calculate pattern position first
					uint64_t grainStartFrame;
					uint64_t selectionFrames = (controllers[SAMPLE_END] - controllers[SAMPLE_START]) * sample->info.frames;
					uint64_t patternFrames = double (selectionFrames) * controllers[PATTERN_SIZE];
					double grainMs = controllers[PROPERTIES + GRAIN_SIZE * PROPERTIES_SIZE + PROPERTY_VALUE_START];
					int64_t grainFrames = millisecondsToFrames (grainMs, rate);

					if (iframe - voice.patternStartFrame > patternFrames) voice.patternStartFrame = iframe;
					if (grainFrames <= 0) grainStartFrame = controllers[SAMPLE_START] * sample->info.frames;
					else
					{
						int patternStep = double (iframe - voice.patternStartFrame) / double (grainFrames);
						patternStep = LIMIT (patternStep, 0, MAXPATTERNSTEPS - 1);
						int grainStep = pattern.getValue (patternStep);
						grainStartFrame = controllers[SAMPLE_START] * sample->info.frames + grainStep * grainFrames;
					}


					// Set modulated grain properties
					float graintune = modulateGrainProperty (&voice, GRAIN_TUNE, iframe);
					float grainfine = modulateGrainProperty (&voice, GRAIN_FINE, iframe);
					grain.speed = noteToFrequency(float (voice.note) + graintune + 0.01 * grainfine) / controllers[SAMPLE_FREQ];

					float grainsize = modulateGrainProperty (&voice, GRAIN_SIZE, iframe);
					grain.endFrame = grain.startFrame + millisecondsToFrames (grainsize, rate) * grain.speed;

					float phase = modulateGrainProperty (&voice, GRAIN_PHASE, iframe);
					grain.sampleStartFrame = grainStartFrame + grain.driveFrames - grainsize * phase;

					grain.level = modulateGrainProperty (&voice, GRAIN_LEVEL, iframe);
					grain.pan = modulateGrainProperty (&voice, GRAIN_PAN, iframe);


					// Reserve next grain
					float grainrate = modulateGrainProperty (&voice, GRAIN_RATE, iframe);
					double diffframes = double (grain.endFrame - grain.startFrame) / grainrate;
					uint64_t nextframe = iframe + diffframes;

					float graindrive = modulateGrainProperty (&voice, GRAIN_DRIVE, iframe);
					int64_t nextdriveframes = int64_t (grain.driveFrames + graindrive * diffframes) % grainFrames ;

					voice.grains.push_back(Grain {nextframe , 0xFFFFFFFF00000000, 0ul, nextdriveframes, 0.0, 0.0, 0.0});

					// Kick out first grain if grains full
					while (voice.grains.size > controllers[MAX_GRAINS_PER_VOICE]) voice.grains.pop_front();
				}


				// Render
				for (int g = 0; g < int (voice.grains.size); ++g)
				{
					Grain& grain = voice.grains[g];

					if ((iframe >= grain.startFrame) && (iframe <= grain.endFrame))
					{
						double pos = grain.sampleStartFrame + double (iframe - grain.startFrame) * grain.speed;
						uint64_t p0 = pos;
						double pfrac = pos - p0;

						if (int64_t (p0 + 1) < sample->info.frames)
						{
							double relpos =
							(
								grain.endFrame > grain.startFrame ?
								double (iframe - grain.startFrame) / double (grain.endFrame - grain.startFrame) :
								0
							);
							// Get raw sample
							float s0 = sample->data[p0 * sample->info.channels];
							float s1 = sample->data[(p0 + 1) * sample->info.channels];
							float psample = s0 + (s1 - s0) * pfrac;
							// Apply shape
							psample *= shape[int (controllers[GRAIN_SHAPE])].getMapValue (relpos);
							// Set level
							psample *= grain.level;
							// Set panning
							float psample1 = psample * (grain.pan < 0 ? grain.pan + 1 : 1);
							float psample2 = psample * (grain.pan < 0 ? 1: 1 - grain.pan);

							// Add to vsample
							vsample1 += psample1;
							vsample2 += psample2;
						}
					}
				}

				// Apply envelope and add to isample
				float f =
				(
					iframe <= voice.endFrame ?
					env[envNr].getValue (true, framesToSeconds (iframe - voice.startFrame, rate)) :
					env[envNr].getValue (false, framesToSeconds (iframe - voice.endFrame, rate), voice.endValue)
				);
				isample1 += vsample1 * f;
				isample2 += vsample2 * f;
			}
		}

		// Write to audio out
		audioOutput1[i] = isample1;
		audioOutput2[i] = isample2;
	}

	// Cleanup release-ended voices
	bool done;
	do
	{
		done = true;
		for (Voice** vit = voices.begin (); vit < voices.end(); ++vit)
		{
			if ((**vit).endFrame + vReleaseFrames < frame + end)
			{
				voices.erase (vit);
				done = false;
				break;
			}
		}
	} while (!done);
}

double BHarvestr::modulateGrainProperty (const Voice* voiceptr, const int property, const uint64_t frame) const
{
	float start = controllers[PROPERTIES + property * PROPERTIES_SIZE + PROPERTY_VALUE_START];
	float end = controllers[PROPERTIES + property * PROPERTIES_SIZE + PROPERTY_VALUE_END];
	float factor = (controllers[PROPERTIES + property * PROPERTIES_SIZE + PROPERTY_MODULATORS] == 0.0f ? 0.0f : 1.0f);

	for (int i = 0; i < NR_PROPERTY_MODULATORS; ++i)
	{
		int mod = controllers[PROPERTIES + property * PROPERTIES_SIZE + PROPERTY_MODULATORS + i];
		if (mod == 0) break;

		// LFO
		if (mod < MODULATOR_SEQ1)
		{
			const int nr = mod - MODULATOR_LFO1;
			factor *= controllers[LFOS + nr * LFO_SIZE + LFO_AMP] * lfo[nr].getValue (framesToSeconds (frame, rate));
		}

		// Sequencer
		else if (mod < MODULATOR_ENV1)
		{
			const int nr = mod - MODULATOR_SEQ1;
			factor *= seq[nr].getValue (framesToSeconds (frame, rate));
		}

		// Envelope
		else if (mod < MODULATOR_RANDOM1)
		{
			if (voiceptr)
			{
				const int nr = mod - MODULATOR_ENV1;
				if (frame < voiceptr->startFrame) factor = 0.0f;
				if (frame < voiceptr->endFrame) factor *= env[nr].getValue (true, framesToSeconds (frame - voiceptr->startFrame, rate));
				else factor *= env[nr].getValue (false, framesToSeconds (frame, rate), voiceptr->endValue);
			}
		}

		// Random
		else
		{
			const int nr = mod - MODULATOR_RANDOM1;
			const double rnd = double (rand()) / double (RAND_MAX);
			factor *= controllers[RNDS + nr * RND_SIZE + RND_MIN] + rnd * (controllers[RNDS + nr * RND_SIZE + RND_MAX] - controllers[RNDS + nr * RND_SIZE + RND_MIN]);
		}
	}

	return modulatef (start, end, factor);
}

void BHarvestr::notifyStatusToGui ()
{
	LV2_Atom_Forge_Frame forgeframe;
	lv2_atom_forge_frame_time(&notifyForge, 0);
	lv2_atom_forge_object(&notifyForge, &forgeframe, 0, uris.bharvestr_statusEvent);

	// Time
	lv2_atom_forge_key(&notifyForge, uris.bharvestr_statusTime);
	lv2_atom_forge_double(&notifyForge, double (frame) / rate);

	// LFO positions
	float lfoPositions[NR_LFOS];
	for (int i = 0; i < NR_LFOS; ++i) lfoPositions[i] = fracf (lfo[i].getPosition(double (frame) / rate) + lfo[i].getPhase());
	lv2_atom_forge_key(&notifyForge, uris.bharvestr_statusLfoPositions);
	lv2_atom_forge_vector(&notifyForge, sizeof(float), uris.atom_Float, NR_LFOS, (void*) lfoPositions);

	// Seq positions
	float seqPositions[NR_SEQS];
	for (int i = 0; i < NR_SEQS; ++i) seqPositions[i] = fracf (seq[i].getPosition(double (frame) / rate) + seq[i].getPhase());
	lv2_atom_forge_key(&notifyForge, uris.bharvestr_statusSeqPositions);
	lv2_atom_forge_vector(&notifyForge, sizeof(float), uris.atom_Float, NR_LFOS, (void*) seqPositions);

	// Env positions
	float envPositions[2 * MAXVOICES];
	int count = 0;
	for (unsigned int v = 0; v < voices.size; ++v)
	{
		if (frame >= voices[v].startFrame)
		{
			if (frame < voices[v].endFrame)
			{
				envPositions[count] = 0.0f;
				envPositions[count + 1] = framesToSeconds (frame - voices[v].startFrame, rate);
			}
			else
			{
				envPositions[count] = 1.0f;
				envPositions[count + 1] = framesToSeconds (frame - voices[v].endFrame, rate);
			}

			count += 2;
		}
	}
	lv2_atom_forge_key(&notifyForge, uris.bharvestr_statusEnvPositions);
	lv2_atom_forge_vector(&notifyForge, sizeof(float), uris.atom_Float, count, (void*) envPositions);

	// Grain properties
	float grainProperties[NR_GRAIN_PROPERTIES];
	for (int i = 0; i < NR_GRAIN_PROPERTIES; ++i)
	{
		grainProperties[i] = modulateGrainProperty (nullptr, i, frame);
	}

	lv2_atom_forge_key(&notifyForge, uris.bharvestr_statusGrainProperties);
	lv2_atom_forge_vector(&notifyForge, sizeof(float), uris.atom_Float, NR_GRAIN_PROPERTIES, (void*) grainProperties);

	// Pattern positions
	int patternPos[MAXVOICES] = {0};
	double grainMs = controllers[PROPERTIES + GRAIN_SIZE * PROPERTIES_SIZE + PROPERTY_VALUE_START];
	int64_t grainFrames = millisecondsToFrames (grainMs, rate);
	for (unsigned int v = 0; v < voices.size; ++v)
	{
		Voice& voice = voices[v];
		int patternStep = double (frame - voice.patternStartFrame) / double (grainFrames);
		patternPos[v] = LIMIT (patternStep, 0, MAXPATTERNSTEPS - 1);
	}

	lv2_atom_forge_key(&notifyForge, uris.bharvestr_statusPatternPositions);
	lv2_atom_forge_vector(&notifyForge, sizeof(int), uris.atom_Int, voices.size, (void*) patternPos);

	lv2_atom_forge_pop(&notifyForge, &forgeframe);
}

void BHarvestr::notifySampleStopToGui ()
{
	LV2_Atom_Forge_Frame forgeframe;
	lv2_atom_forge_frame_time(&notifyForge, 0);
	lv2_atom_forge_object(&notifyForge, &forgeframe, 0, uris.bharvestr_sampleStop);
	lv2_atom_forge_pop(&notifyForge, &forgeframe);

	notify.sampleStop = false;
}

void BHarvestr::notifySelectionStopToGui ()
{
	LV2_Atom_Forge_Frame forgeframe;
	lv2_atom_forge_frame_time(&notifyForge, 0);
	lv2_atom_forge_object(&notifyForge, &forgeframe, 0, uris.bharvestr_selectionStop);
	lv2_atom_forge_pop(&notifyForge, &forgeframe);

	notify.selectionStop = false;
}

void BHarvestr::notifyPatternToGui ()
{
	LV2_Atom_Forge_Frame frame;
	lv2_atom_forge_frame_time(&notifyForge, 0);
	lv2_atom_forge_object(&notifyForge, &frame, 0, uris.bharvestr_patternEvent);
	lv2_atom_forge_key(&notifyForge, uris.bharvestr_patternRows);
	lv2_atom_forge_int(&notifyForge, pattern.getRows());
	lv2_atom_forge_key(&notifyForge, uris.bharvestr_patternSteps);
	lv2_atom_forge_int(&notifyForge, pattern.getSteps());
	lv2_atom_forge_key(&notifyForge, uris.bharvestr_pattern);
	lv2_atom_forge_vector(&notifyForge, sizeof(int), uris.atom_Int, MAXPATTERNSTEPS, (void*) pattern.getPattern());
	lv2_atom_forge_pop(&notifyForge, &frame);
	notify.pattern = false;
}

void BHarvestr::notifySamplePathToGui ()
{
	if (sample && sample->path)
	{
		LV2_Atom_Forge_Frame frame;
		lv2_atom_forge_frame_time(&notifyForge, 0);
		lv2_atom_forge_object(&notifyForge, &frame, 0, uris.bharvestr_sampleEvent);
		lv2_atom_forge_key(&notifyForge, uris.bharvestr_samplePath);
		lv2_atom_forge_path (&notifyForge, sample->path, strlen (sample->path) + 1);

		lv2_atom_forge_pop(&notifyForge, &frame);
	}

	notify.samplePath = false;
}

void BHarvestr::notifyDisplayToGui ()
{
	LV2_Atom_Forge_Frame frame;
	lv2_atom_forge_frame_time(&notifyForge, 0);
	lv2_atom_forge_object(&notifyForge, &frame, 0, uris.bharvestr_displayEvent);

	if (notify.sampleDisplay)
	{
		float sampleSize = 0.0f;
		float sampleDisplay[DISPLAYDATASIZE] {0.0f};
		if (sample && sample->info.frames)
		{
			sampleSize = double (sample->info.frames) / rate;
			for (int i = 0; i < DISPLAYDATASIZE; ++i)
			{
				sampleDisplay[i] = sample->data[size_t (i * sample->info.frames / DISPLAYDATASIZE) * sample->info.channels];
			}
		}

		else memset (sampleDisplay, 0, DISPLAYDATASIZE * sizeof (float));

		lv2_atom_forge_key(&notifyForge, uris.bharvestr_sampleSize);
		lv2_atom_forge_float(&notifyForge, sampleSize);
		lv2_atom_forge_key(&notifyForge, uris.bharvestr_sampleDisplayData);
		lv2_atom_forge_vector(&notifyForge, sizeof(float), uris.atom_Float, DISPLAYDATASIZE, (void*) sampleDisplay);
		notify.sampleDisplay = false;
	}

	if (notify.selectionDisplay)
	{
		float selectionDisplay[DISPLAYDATASIZE] {0.0f};
		if (sample && sample->info.frames)
		{
			size_t s = controllers[SAMPLE_START] * sample->info.frames;
			size_t e = controllers[SAMPLE_END] * sample->info.frames;

			for (int i = 0; i < DISPLAYDATASIZE; ++i)
			{
				selectionDisplay[i] = sample->data[(s + size_t (i * (e - s) / DISPLAYDATASIZE)) * sample->info.channels];
			}
		}

		else memset (selectionDisplay, 0, DISPLAYDATASIZE * sizeof (float));

		lv2_atom_forge_key(&notifyForge, uris.bharvestr_selectionDisplayData);
		lv2_atom_forge_vector(&notifyForge, sizeof(float), uris.atom_Float, DISPLAYDATASIZE, (void*) selectionDisplay);
		notify.selectionDisplay = false;
	}

	lv2_atom_forge_pop(&notifyForge, &frame);
}

void BHarvestr::notifyShapeToGui (ShapeIndex index)
{
	unsigned int shapeNr = index;
	size_t size = shape[shapeNr].size ();

	// Load shapeBuffer
	float shapeBuffer[MAXNODES * 7];
	for (unsigned int i = 0; i < size; ++i)
	{
		Node node = shape[shapeNr].getRawNode (i);
		shapeBuffer[i * 7] = (float)node.nodeType;
		shapeBuffer[i * 7 + 1] = (float)node.point.x;
		shapeBuffer[i * 7 + 2] = (float)node.point.y;
		shapeBuffer[i * 7 + 3] = (float)node.handle1.x;
		shapeBuffer[i * 7 + 4] = (float)node.handle1.y;
		shapeBuffer[i * 7 + 5] = (float)node.handle2.x;
		shapeBuffer[i * 7 + 6] = (float)node.handle2.y;
	}

	// Notify shapeBuffer
	LV2_Atom_Forge_Frame frame;
	lv2_atom_forge_frame_time(&notifyForge, 0);
	lv2_atom_forge_object(&notifyForge, &frame, 0, uris.bharvestr_shapeEvent);
	lv2_atom_forge_key(&notifyForge, uris.bharvestr_shapeIndex);
	lv2_atom_forge_int(&notifyForge, shapeNr);
	lv2_atom_forge_key(&notifyForge, uris.bharvestr_shapeData);
	lv2_atom_forge_vector(&notifyForge, sizeof(float), uris.atom_Float, (uint32_t) (7 * size), &shapeBuffer);
	lv2_atom_forge_pop(&notifyForge, &frame);

	notify.shape[shapeNr] = false;
}

void BHarvestr::notifyPresetInfoToGui()
{
	LV2_Atom_Forge_Frame frame;
	lv2_atom_forge_frame_time(&notifyForge, 0);
	lv2_atom_forge_object(&notifyForge, &frame, 0, uris.bharvestr_presetInfoEvent);
	lv2_atom_forge_key(&notifyForge, uris.bharvestr_presetInfoName);
	lv2_atom_forge_string(&notifyForge, presetInfo.name, strlen (presetInfo.name) + 1);
	lv2_atom_forge_key(&notifyForge, uris.bharvestr_presetInfoType);
	lv2_atom_forge_string(&notifyForge, presetInfo.type, strlen (presetInfo.type) + 1);
	lv2_atom_forge_key(&notifyForge, uris.bharvestr_presetInfoDate);
	lv2_atom_forge_int(&notifyForge, presetInfo.date);
	lv2_atom_forge_key(&notifyForge, uris.bharvestr_presetInfoCreator);
	lv2_atom_forge_string(&notifyForge, presetInfo.creator, strlen (presetInfo.creator) + 1);
	lv2_atom_forge_key(&notifyForge, uris.bharvestr_presetInfoURI);
	lv2_atom_forge_string(&notifyForge, presetInfo.uri, strlen (presetInfo.uri) + 1);
	lv2_atom_forge_key(&notifyForge, uris.bharvestr_presetInfoLicense);
	lv2_atom_forge_string(&notifyForge, presetInfo.license, strlen (presetInfo.license) + 1);
	lv2_atom_forge_key(&notifyForge, uris.bharvestr_presetInfoDescription);
	lv2_atom_forge_string(&notifyForge, presetInfo.description, strlen (presetInfo.description) + 1);
	lv2_atom_forge_pop(&notifyForge, &frame);

	notify.presetInfo = false;
}

void BHarvestr::notifyMessageToGui()
{
	LV2_Atom_Forge_Frame frame;
	lv2_atom_forge_frame_time(&notifyForge, 0);
	lv2_atom_forge_object(&notifyForge, &frame, 0, uris.bharvestr_messageEvent);
	lv2_atom_forge_key(&notifyForge, uris.bharvestr_message);
	lv2_atom_forge_string(&notifyForge, message, strlen (message) + 1);
	lv2_atom_forge_pop(&notifyForge, &frame);

	notify.message = false;
}

/*
 *
 *
 ******************************************************************************
 *  LV2 specific declarations
 */

static LV2_Handle instantiate (const LV2_Descriptor* descriptor, double samplerate, const char* bundle_path, const LV2_Feature* const* features)
{
	// New instance
	BHarvestr* instance;
	try {instance = new BHarvestr(samplerate, features);}
	catch (std::exception& exc)
	{
		fprintf (stderr, "BHarvestr.lv2: Plugin instantiation failed. %s\n", exc.what ());
		return NULL;
	}

	return (LV2_Handle)instance;
}

static void connect_port (LV2_Handle instance, uint32_t port, void *data)
{
	BHarvestr* inst = (BHarvestr*) instance;
	inst->connect_port (port, data);
}

static void run (LV2_Handle instance, uint32_t n_samples)
{
	BHarvestr* inst = (BHarvestr*) instance;
	if (inst) inst->run (n_samples);
}

static LV2_State_Status state_save(LV2_Handle instance, LV2_State_Store_Function store, LV2_State_Handle handle, uint32_t flags,
           const LV2_Feature* const* features)
{
	BHarvestr* inst = (BHarvestr*)instance;
	if (!inst) return LV2_STATE_SUCCESS;

	return inst->state_save (store, handle, flags, features);
}

static LV2_State_Status state_restore(LV2_Handle instance, LV2_State_Retrieve_Function retrieve, LV2_State_Handle handle, uint32_t flags,
           const LV2_Feature* const* features)
{
	BHarvestr* inst = (BHarvestr*)instance;
	if (!inst) return LV2_STATE_SUCCESS;

	return inst->state_restore (retrieve, handle, flags, features);
}

static LV2_Worker_Status work (LV2_Handle instance, LV2_Worker_Respond_Function respond, LV2_Worker_Respond_Handle handle,
	uint32_t size, const void* data)
{
	BHarvestr* inst = (BHarvestr*)instance;
	if (!inst) return LV2_WORKER_SUCCESS;

	return inst->work (respond, handle, size, data);
}

static LV2_Worker_Status work_response (LV2_Handle instance, uint32_t size,  const void* data)
{
	BHarvestr* inst = (BHarvestr*)instance;
	if (!inst) return LV2_WORKER_SUCCESS;

	return inst->work_response (size, data);
}

static void cleanup (LV2_Handle instance)
{
	BHarvestr* inst = (BHarvestr*) instance;
	if (inst) delete inst;
}


static const void* extension_data(const char* uri)
{
  static const LV2_State_Interface  state  = {state_save, state_restore};
  static const LV2_Worker_Interface worker = {work, work_response, NULL};
  if (!strcmp(uri, LV2_STATE__interface)) return &state;
  if (!strcmp(uri, LV2_WORKER__interface)) return &worker;
  return NULL;
}


static const LV2_Descriptor descriptor =
{
		BHARVESTR_URI,
		instantiate,
		connect_port,
		NULL,	// activate
		run,
		NULL,	// deactivate
		cleanup,
		extension_data
};

// LV2 Symbol Export
LV2_SYMBOL_EXPORT const LV2_Descriptor* lv2_descriptor (uint32_t index)
{
	switch (index)
	{
	case 0: return &descriptor;
	default: return NULL;
	}
}

/* End of LV2 specific declarations
 *
 * *****************************************************************************
 *
 *
 */
