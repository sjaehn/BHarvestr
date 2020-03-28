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

#ifndef SAMPLE_HPP_
#define SAMPLE_HPP_

#include "sndfile.h"
#include <cstdlib>
#include <cstring>
#include <string>
#include <stdexcept>


struct Sample
{
        SF_INFO  info;      // Info about sample from sndfile
        float*   data;      // Sample data in float
        char*    path;      // Path of file

        Sample () : info {0, 0, 0, 0, 0, 0}, data (nullptr), path (nullptr) {}

        Sample (const char* samplepath) : info {0, 0, 0, 0, 0, 0}, data (nullptr), path (nullptr)
        {
        	if (!samplepath) return;

        	SNDFILE* const sndfile = sf_open (samplepath, SFM_READ, &info);

                if (!sndfile || !info.frames) throw std::invalid_argument("Can't open " + std::string(samplepath) + ".");

                // Read & render data
                data = (float*) malloc (sizeof(float) * info.frames * info.channels);
                if (!data)
        	{
        		sf_close (sndfile);
        		throw std::bad_alloc();
        	}

                sf_seek (sndfile, 0, SEEK_SET);
                sf_read_float (sndfile, data, info.frames * info.channels);
                sf_close (sndfile);

        	int len = strlen (samplepath);
                path = (char*) malloc (len + 1);
                if (path) memcpy(path, samplepath, len + 1);
        }

        ~Sample()
        {
        	if (data) free (data);
        	if (path) free (path);
        }

        float get (const sf_count_t frame, const int channel, const int rate)
        {
        	if (!data) return 0.0f;

        	// Direct access if same frame rate
        	if (info.samplerate == rate)
        	{
        		if (frame >= info.frames) return 0.0f;
        		else return data[frame * info.channels + channel];
        	}

        	// Linear rendering (TODO) if frame rates differ
        	double f = (frame * info.samplerate) / rate;
        	sf_count_t f1 = f;

        	if (f1 + 1 >= info.frames) return 0.0f;
        	if (f1 == f) return data[f1 * info.channels + channel];

        	float data1 = data[f1 * info.channels + channel];
        	float data2 = data[(f1 + 1) * info.channels + channel];
        	return data1 + (f - double (f1)) * (data2 - data1);
        }
};

#endif /* SAMPLE_HPP_ */
