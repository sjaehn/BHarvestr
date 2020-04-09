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

#ifndef ENVELOPE_HPP_
#define ENVELOPE_HPP_

#include <cmath>
#include <limits>
#include <cstdio>
#define M_EXP_M3 0.049787068

class Envelope
{
protected:
        double attack_;
        double decay_;
        double sustain_;
        double release_;
        double releaseStartTime_;
        double releaseStartValue_;

public:
        Envelope () : Envelope (0.0, 0.0, 0.0, 0.0) {}

        Envelope (const double attack, const double decay, const double sustain, const double release) :
        attack_ (attack),
        decay_ (decay),
        sustain_ (sustain),
        release_ (release),
        releaseStartTime_ (std::numeric_limits<double>::max()),
        releaseStartValue_ (0.0)
        {}

        void releaseAt (const double time)
        {
                releaseStartTime_ = std::numeric_limits<double>::max();
                releaseStartValue_ = getValue (time);
                releaseStartTime_ = time;
        }

        double getValue (const double time) const
        {
                if (time < releaseStartTime_)
                {
                        if (time < 0.0) return 0.0;

                        if (time < attack_)
                        {
                                double t = time / attack_;
                                return (1.0 - exp (-3.0 * t) + t * M_EXP_M3);
                        }

                        if (time == attack_) return 1.0;

                        if (time < attack_ + decay_)
                        {
                                double t = (time - attack_) / decay_;
                                return 1.0 - (1.0 - sustain_) * (1.0 - exp (-3.0 * t) + t * M_EXP_M3);
                        }

                        return sustain_;
                }
                else
                {
                        if (time < releaseStartTime_ + release_)
                        {
                                double t = (time - releaseStartTime_) / release_;
                                return releaseStartValue_ - releaseStartValue_ * (1.0 - exp (-3.0 * t) + t * M_EXP_M3);
                        }

                        return 0.0;
                }
        }

        double getRelease() const {return release_;}

};

#endif /* ENVELOPE_HPP_ */
