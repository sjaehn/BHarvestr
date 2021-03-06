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

#ifndef VOICE_HPP_
#define VOICE_HPP_

#include <cstdint>
#include "StaticArrayList.hpp"
#include "Grain.hpp"
#include "Envelope.hpp"
#include "definitions.h"
#include "Ports.hpp"

struct Voice
{
        uint8_t note;
        uint8_t velocity;
        uint64_t startFrame;
        uint64_t patternStartFrame;
        uint64_t endFrame;
        uint64_t releaseFrames;


        Envelope envelope[NR_ENVS];
        StaticArrayList<Grain, MAXGRAINS> grains;


        Voice () : Voice (0, 0, 0, 0, 0,  nullptr) {}

        Voice (const uint8_t note, const uint8_t velocity, const uint64_t startFrame, const uint64_t endFrame, const uint64_t releaseFrames, Envelope* env) :
        note (note), velocity (velocity), startFrame (startFrame),
        patternStartFrame (startFrame), endFrame (endFrame),
        releaseFrames (releaseFrames),
        grains ()
        {
                if (env)
                {
                        for (int i = 0; i < NR_ENVS; ++i) envelope[i] = env[i];
                }
        }

};

#endif /* VOICE_HPP_ */
