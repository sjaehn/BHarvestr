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

#ifndef SEQUENCERWIDGET_HPP_
#define SEQUENCERWIDGET_HPP_

#include "definitions.h"
#include <array>
#include <cmath>
#include "BUtilities/to_string.hpp"
#include "BWidgets/Widget.hpp"
#include "VSlider.hpp"

template<size_t sz>
class SequencerWidget : public BWidgets::Widget
{
public:
	std::array<VSlider, sz> sliders;

	SequencerWidget () : SequencerWidget (0, 0, 0, 0, "shape") {}

	SequencerWidget (const double x, const double y, const double width, const double height, const std::string& name) :
			Widget (x, y, width, height, name),
			nr_steps (1),
			scaleVisible (true),
			scaleAnchorYPos (0), scaleAnchorValue (0), scaleRatio (1),
			minorXSteps (1), majorXSteps (1),
			xmin (0.0), xmax (1.0),
			prefix (""), unit (""),
			fgColors (BColors::reds), bgColors (BColors::darks),
			lbfont (BWIDGETS_DEFAULT_FONT)
	{
		sliders.fill (VSlider(0, 0, 0, 0, name, 1.0, 0.0, 1.0, 0.0));
		for (VSlider& s : sliders) add (s);
	}

	SequencerWidget (const SequencerWidget& that) :
			Widget (that),
			sliders (that.sliders),
			nr_steps (that.nr_steps),
			scaleVisible (that.scaleVisible),
			scaleAnchorYPos (that.scaleAnchorYPos),
			scaleAnchorValue (that.scaleAnchorValue),
			scaleRatio (that.scaleRatio),
			minorXSteps (that.minorXSteps),
			majorXSteps (that.majorXSteps),
			xmin (that.xmin),
			xmax (that.xmax),
			prefix (that.prefix),
			unit (that.unit),
			fgColors (that.fgColors),
			bgColors (that.bgColors),
			lbfont (that.lbfont)
	{
		for (VSlider& s : sliders) add (s);
	}

	SequencerWidget& operator= (const SequencerWidget& that)
	{
		nr_steps = that.nr_steps;
		scaleVisible = that.scaleVisible;
		scaleAnchorYPos = that.scaleAnchorYPos;
		scaleAnchorValue = that.scaleAnchorValue;
		scaleRatio = that.scaleRatio;
		minorXSteps = that.minorXSteps;
		majorXSteps = that.majorXSteps;
		xmin = that.xmin;
		xmax = that.xmax;
		prefix = that.prefix;
		unit = that.unit;
		fgColors = that.fgColors;
		bgColors = that.bgColors;
		lbfont = that.lbfont;

		for (unsigned int i = 0; i < sliders.size(); ++i)
		{
			release (&sliders[i]);
			sliders[i] = that.sliders[i];
			add (sliders[i]);
		}
		Widget::operator= (that);
		return *this;
	}

	virtual BWidgets::Widget* clone () const override {return new SequencerWidget (*this);}

	void setSize (const size_t size)
	{
		if (nr_steps != size)
		{
			nr_steps = size;
			update();
		}
	}

	size_t getSize () const {return nr_steps;}

	void setScaleVisible (const bool visible)
	{
		if (visible != scaleVisible)
		{
			scaleVisible = visible;
			update ();
		}
	}

	bool isScaleVisible () const {return scaleVisible;}

	void setScaleParameters (const double anchorYPos, const double anchorValue, const double ratio)
	{
		if ((scaleAnchorYPos != anchorYPos) || (scaleAnchorValue != anchorValue) || (scaleRatio != ratio))
		{
			scaleAnchorYPos = anchorYPos;
			scaleAnchorValue = anchorValue;
			scaleRatio = ratio;
			update ();
		}
	}

	void setMinorXSteps (const double stepSize)
	{
		if (minorXSteps != stepSize)
		{
			minorXSteps = stepSize;
			update ();
		}
	}

	void setMajorXSteps (const double stepSize)
	{
		if (majorXSteps != stepSize)
		{
			majorXSteps = stepSize;
			update ();
		}
	}

	void setXRange (const double min, const double max)
	{
		if (((min != xmin) || (max != xmax)) && (min <= max))
		{
			xmin = min;
			xmax = max;
			update();
		}
	}

	void setPrefix (const std::string& text)
	{
		if (prefix != text)
		{
			prefix = text;
			update ();
		}
	}

	void setUnit (const std::string& text)
	{
		if (unit != text)
		{
			unit = text;
			update ();
		}
	}

	virtual void update () override
	{
		Widget::update();

		double x0 = getXOffset();
		double y0 = getYOffset();
		double w = getEffectiveWidth();
		double stepw = w / nr_steps;
		double h = getEffectiveHeight();


		for (unsigned int i = 0; i < sliders.size(); ++i)
		{
			if (i < nr_steps)
			{
				sliders[i].moveTo (x0 + (i + 0.05) * stepw, y0);
				sliders[i].resize (stepw * 0.9, h);
				sliders[i].show();
			}
			else sliders[i].hide();
		}
	}

	virtual void onPointerDragged (BEvents::PointerEvent* event) override
	{
		scaleAnchorYPos += (-event->getDelta().y) / getEffectiveHeight ();
		update ();
	}
	virtual void onWheelScrolled (BEvents::WheelEvent* event) override
	{
		const double ygrid = pow (10, floor (log10 (scaleRatio)));
		scaleRatio += 0.1 * ygrid * event->getDelta().y;
		if (scaleRatio < 0.01) scaleRatio = 0.01;
		update ();
	}

	virtual void applyTheme (BStyles::Theme& theme) override {applyTheme (theme, name_);}
	virtual void applyTheme (BStyles::Theme& theme, const std::string& name) override
	{
		Widget::applyTheme (theme, name);
		for (VSlider& s : sliders) s.applyTheme (theme, name);

		// Foreground colors (curve)
		const void* fgPtr = theme.getStyle(name, BWIDGETS_KEYWORD_FGCOLORS);
		if (fgPtr) fgColors = *((BColors::ColorSet*) fgPtr);

		// Background colors (grid)
		const void* bgPtr = theme.getStyle(name, BWIDGETS_KEYWORD_BGCOLORS);
		if (bgPtr) bgColors = *((BColors::ColorSet*) bgPtr);

		// Font
		const void* fontPtr = theme.getStyle(name, BWIDGETS_KEYWORD_FONT);
		if (fontPtr) lbfont = *((BStyles::Font*) fontPtr);

		if (fgPtr || bgPtr || fontPtr) update ();

	}

protected:
	size_t nr_steps;
	bool scaleVisible;
	double scaleAnchorYPos;
	double scaleAnchorValue;
	double scaleRatio;
	double minorXSteps;
	double majorXSteps;
	double xmin;
	double xmax;
	std::string prefix;
	std::string unit;
	BColors::ColorSet fgColors;
	BColors::ColorSet bgColors;
	BStyles::Font lbfont;

	virtual void draw (const BUtilities::RectArea& area) override
	{
		if ((!widgetSurface_) || (cairo_surface_status (widgetSurface_) != CAIRO_STATUS_SUCCESS)) return;

		Widget::draw (area);

		double x0 = getXOffset ();
		double y0 = getYOffset ();
		double w = getEffectiveWidth ();
		double h = getEffectiveHeight ();
		double ymin = scaleAnchorValue - scaleRatio * scaleAnchorYPos;
		double ymax = ymin + scaleRatio;
		//BColors::Color lineColor = *fgColors.getColor (BColors::NORMAL);
		BColors::Color gridColor = *bgColors.getColor (BColors::NORMAL);

		if ((xmax == xmin) || (ymin == ymax)) return;

		cairo_t* cr = cairo_create (widgetSurface_);

		if (cairo_status (cr) == CAIRO_STATUS_SUCCESS)
		{
			// Limit cairo-drawing area
			cairo_rectangle (cr, area.getX(), area.getY(), area.getWidth(), area.getHeight());
			cairo_clip (cr);

			if (scaleVisible)
			{
				double ygrid = pow (10, floor (log10 (scaleRatio / 1.5)));
				int ldYgrid = log10 (ygrid);
				std::string nrformat = "%" + ((ygrid < 1) ? ("1." + std::to_string (-ldYgrid)) : (std::to_string (ldYgrid + 1) + ".0")) + "f";
				cairo_text_extents_t ext;
				cairo_select_font_face (cr, lbfont.getFontFamily ().c_str (), lbfont.getFontSlant (), lbfont.getFontWeight ());
				cairo_set_font_size (cr, lbfont.getFontSize ());

				// Draw Y steps
				for (double yp = ceil (ymin / ygrid) * ygrid; yp <= ymax; yp += ygrid)
				{
					cairo_move_to (cr, x0, y0 + h - h * (yp - ymin) / (ymax - ymin));
					cairo_line_to (cr, x0 + 0.02 * w, y0 + h - h * (yp - ymin) / (ymax - ymin));

					std::string label = prefix + BUtilities::to_string (yp, nrformat) + ((unit != "") ? (" " + unit) : "");
					cairo_text_extents (cr, label.c_str(), &ext);
					cairo_move_to (cr, x0 + 0.025 * w - ext.x_bearing, y0 + h - h * (yp - ymin) / (ymax - ymin) - ext.height / 2 - ext.y_bearing);
					cairo_set_source_rgba (cr, CAIRO_RGBA (gridColor));
					cairo_show_text (cr, label.c_str ());

					cairo_move_to (cr, x0 + 0.03 * w + ext.width, y0 + h - h * (yp - ymin) / (ymax - ymin));
					cairo_line_to (cr, x0 + w, y0 + h - h * (yp - ymin) / (ymax - ymin));
				}

				cairo_set_source_rgba (cr, CAIRO_RGBA (gridColor));
				cairo_set_line_width (cr, 1);
				cairo_stroke (cr);

				// Draw X steps
				for (double x = xmin; x < xmax; x += minorXSteps)
				{
					cairo_move_to (cr, (x - xmin) / (xmax - xmin) * w, 0);
					cairo_line_to (cr, (x - xmin) / (xmax - xmin) * w, h);
				}
				cairo_set_source_rgba (cr, CAIRO_RGBA (gridColor));
				cairo_set_line_width (cr, 1.0);
				cairo_stroke (cr);

				for (double x = xmin; x < xmax; x += majorXSteps)
				{
					cairo_move_to (cr, (x - xmin) / (xmax - xmin) * w, 0);
					cairo_line_to (cr, (x - xmin) / (xmax - xmin) * w, h);
				}
				cairo_set_source_rgba (cr, CAIRO_RGBA (gridColor));
				cairo_set_line_width (cr, 2.0);
				cairo_stroke (cr);
			}

			cairo_destroy (cr);
		}
	}
};

#endif /* SEQUENCERWIDGET_HPP_ */
