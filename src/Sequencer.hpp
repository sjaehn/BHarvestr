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

#ifndef SEQUENCER_HPP_
#define SEQUENCER_HPP_

#include <cstddef>
#include <cmath>
#include <array>

template<size_t sz>
class Sequencer
{
protected:
        size_t size_;
        double frequency_;
        double phase_;
        double starttime_;
        std::array<double, sz> steps_;

public:

        Sequencer () : Sequencer (1.0, 1.0, 0.0, {}, 0.0) {}

        Sequencer (const size_t size, const double frequency, const double phase, std::array<double, sz> steps = {}, double starttime = 0.0) :
                size_ (size),
                frequency_ (frequency),
                phase_ (phase),
                starttime_ (starttime),
                steps_ (steps)
        {}

        void setSize (const size_t size) {size_ = size;}

        size_t getSize () const {return size_;}

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

        void setStep (const size_t nr, const double val) {steps_[nr] = val;}

        double getStep (const size_t nr) {return steps_[nr];}

        double getPosition (const double time) const
        {
                double position = (time - starttime_) * frequency_ - phase_;
                return position - floor (position);
        }

        double getValue (const double time) const
        {
                double fracTime = getPosition (time);
                size_t nr = fracTime * size_;
                return steps_[nr];
        }
};

#endif /* SEQUENCER_HPP_ */
