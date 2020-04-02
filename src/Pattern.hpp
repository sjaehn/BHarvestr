/* B.Harvestr
 * Beat / envelope shaper LV2 plugin
 *
 * Copyright (C) 2019 by Sven Jähnichen
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

#ifndef PATTERN_HPP_
#define PATTERN_HPP_

#include "definitions.h"
#include <cmath>
#include <cstring>
#include <cstdio>
#include <vector>

enum PatternIndex
{
	RISING_PATTERN		= 0,
	FALLING_PATTERN		= 1,
	SINE_PATTERN		= 2,
	TRIANGLE_PATTERN	= 3,
	USER_PATTERN		= 4,
	NR_PATTERNS		= 5
};

class Pattern
{
public:
	Pattern () : Pattern (1, 1) {}

	Pattern (const int steps, const int rows) :
	 		steps_ (steps > 1 ? (steps <= MAXPATTERNSTEPS ? steps : MAXPATTERNSTEPS) : 1),
			rows_ (rows > 1 ? (rows <= MAXPATTERNSTEPS ? rows : MAXPATTERNSTEPS) : 1),
			patternIndex_ (RISING_PATTERN),
			values_ {0}
	{
		for (size_t i = 0; i < MAXPATTERNSTEPS; ++i) values_[i] = i % rows_;
	}

	virtual void setSteps (const int steps)
	{
		if ((steps >= 1) && (steps < MAXPATTERNSTEPS)) steps_ = steps;
	}

	int getSteps () const {return steps_;}

	virtual void setRows (const int rows)
	{
		if (rows >= 1)
		{
			rows_ = LIMIT (rows, 1, MAXPATTERNSTEPS);
			if (patternIndex_ != USER_PATTERN) setPattern (patternIndex_);
		}
	}

	int getRows () const {return rows_;}

	virtual void setPattern (const PatternIndex index)
	{
		patternIndex_ = index;

		switch (index)
		{
			case RISING_PATTERN:	for (int i = 0; i < MAXPATTERNSTEPS; ++i) values_[i] = i % rows_;
						break;

			case FALLING_PATTERN:	for (int i = 0; i < MAXPATTERNSTEPS; ++i) values_[i] = rows_ - (i % rows_) - 1;
						break;

			case SINE_PATTERN:	for (int i = 0; i < MAXPATTERNSTEPS; ++i)
						{
							int val = double (rows_) * (0.5 + 0.5 * sin (2.0 * double (i) / double (rows_)));
							values_[i] = (LIMIT (val, 0, rows_ - 1));
						}
						break;

			case TRIANGLE_PATTERN:	for (int i = 0; i < MAXPATTERNSTEPS; ++i)
						{
							int s = (i / rows_) % 2;
							int o = i % rows_;
							if (s == 0) values_[i] = o;
							else values_[i] = rows_ - o - 1;
						}
						break;

			default:		break;
		}
	}

	virtual void setPattern (const int* values, const int steps)
	{
		steps_ = LIMIT (steps, 0, MAXPATTERNSTEPS);
		memcpy (values_, values, steps_ * sizeof (int));
	}

	virtual void setPattern (const std::vector<int>& values)
	{
		patternIndex_ = USER_PATTERN;
		for (int i = 0; i < MAXPATTERNSTEPS; ++i)
		{
			if (i < int (values.size())) values_[i] = LIMIT (values[i], 0, rows_ - 1);
			else values_[i] = 0;
		}
	}

	virtual void setValue (const int step, const int value)
	{
		patternIndex_ = USER_PATTERN;
		if ((step >= 0) && (step < steps_)) values_[step] = LIMIT (value, 0, rows_ -1);
	}

	int* getPattern () {return values_;}

	int getValue (const int step) const {return LIMIT (values_[step], 0, rows_ - 1);}

protected:
	int steps_;
	int rows_;
	PatternIndex patternIndex_;
	int values_[MAXPATTERNSTEPS];
};

#endif /* PATTERN_HPP_ */
