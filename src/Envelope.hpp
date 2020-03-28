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
#define M_EXP_M3 0.049787068

struct Envelope
{
        double attack;
        double decay;
        double sustain;
        double release;

        double getValue (const bool noteOn, const double time, double startValue = NAN) const
        {
                if (noteOn)
                {
                        if (time < 0.0) return 0.0;

                        if (time < attack)
                        {
                                double t = time / attack;
                                return (1.0 - exp (-3.0 * t) + t * M_EXP_M3);
                        }

                        if (time == attack) return 1.0;

                        if (time < attack + decay)
                        {
                                double t = (time - attack) / decay;
                                return 1.0 - (1.0 - sustain) * (1.0 - exp (-3.0 * t) + t * M_EXP_M3);
                        }

                        return sustain;
                }
                else
                {
                        if (std::isnan (startValue)) startValue = sustain;

                        if (time <= 0.0) return startValue;

                        if (time < release)
                        {
                                double t = time / release;
                                return startValue - startValue * (1.0 - exp (-3.0 * t) + t * M_EXP_M3);
                        }

                        return 0.0;
                }
        }

};

#endif /* ENVELOPE_HPP_ */
