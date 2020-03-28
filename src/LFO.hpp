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

#ifndef LFO_HPP_
#define LFO_HPP_

#include <cmath>

enum LfoIndex
{
        SINE_LFO                = 0,
        TRIANGLE_LFO            = 1,
        TRIANGLE_2_BITS_LFO     = 2,
        TRIANGLE_3_BITS_LFO     = 3,
        TRIANGLE_4_BITS_LFO     = 4,
        SAWTOOTH_LFO            = 5,
        SAWTOOTH_2_BITS_LFO     = 6,
        SAWTOOTH_3_BITS_LFO     = 7,
        SAWTOOTH_4_BITS_LFO     = 8,
        REVERSE_SAW_LFO         = 9,
        REVERSE_SAW_2_BITS_LFO  = 10,
        REVERSE_SAW_3_BITS_LFO  = 11,
        REVERSE_SAW_4_BITS_LFO  = 12,
        SQUARE_LFO              = 13

};

class Lfo
{
protected:
        LfoIndex type_;
        double frequency_;
        double phase_;
        double starttime_;

public:

        Lfo () : Lfo (SINE_LFO, 1.0, 0.0, 0.0) {}

        Lfo (const LfoIndex type, const double frequency, const double phase, double starttime = 0.0) :
                type_ (type), frequency_ (frequency), phase_ (phase), starttime_ (starttime)
        {}

        void setType (const LfoIndex type) {type_ = type;}

        LfoIndex getType () const {return type_;}

        void setFrequency (const double frequency) {setFrequency (frequency, starttime_);}

        void setFrequency (const double frequency, const double time)
        {
                double fracpos = getPosition (time);
                starttime_ = time - (fracpos + phase_) / frequency;

                frequency_ = frequency;
        }

        double getFrequency () const {return frequency_;}

        void setPhase (const double phase) {phase_ = phase;}

        double getPhase () const {return phase_;}

        double getPosition (const double time) const
        {
                double position = (time - starttime_) * frequency_ - phase_;
                return position - floor (position);
        }

        double getValue (const double time) const
        {
                double fracTime = getPosition (time);

                switch (type_)
                {
                        case SINE_LFO:                  return 0.5 + 0.5 * sin (fracTime * 2.0 * M_PI);
                        case TRIANGLE_LFO:              return (fracTime < 0.5 ? 2.0 * fracTime : 2.0 * (1.0 - fracTime));
                        case TRIANGLE_2_BITS_LFO:       return floor (fracTime < 0.5 ? 8.0 * fracTime : 8.0 * (1.0 - fracTime)) / 4.0;
                        case TRIANGLE_3_BITS_LFO:       return floor (fracTime < 0.5 ? 16.0 * fracTime : 16.0 * (1.0 - fracTime)) / 8.0;
                        case TRIANGLE_4_BITS_LFO:       return floor (fracTime < 0.5 ? 32.0 * fracTime : 32.0 * (1.0 - fracTime)) / 16.0;
                        case SAWTOOTH_LFO:              return fracTime;
                        case SAWTOOTH_2_BITS_LFO:       return floor (fracTime * 4.0) / 4.0;
                        case SAWTOOTH_3_BITS_LFO:       return floor (fracTime * 8.0) / 8.0;
                        case SAWTOOTH_4_BITS_LFO:       return floor (fracTime * 16.0) / 16.0;
                        case REVERSE_SAW_LFO:           return 1.0 - fracTime;
                        case REVERSE_SAW_2_BITS_LFO:    return floor ((1.0 - fracTime) * 4.0) / 4.0;
                        case REVERSE_SAW_3_BITS_LFO:    return floor ((1.0 - fracTime) * 8.0) / 8.0;
                        case REVERSE_SAW_4_BITS_LFO:    return floor ((1.0 - fracTime) * 16.0) / 16.0;
                        case SQUARE_LFO:                return (fracTime < 0.5 ? 0.0 : 1.0);
                        default:                        return 0.0;
                }
        }
};

#endif /* LFO_HPP_ */
