/* B.Harvestr
 * LV2 Plugin
 *
 * Copyright (C) 2018, 2019  Sven Jähnichen
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#ifndef HSLIDERRANGE_HPP_
#define HSLIDERRANGE_HPP_

#include "HSlider.hpp"
#include "VMarkerLine.hpp"
#include "BUtilities/to_string.hpp"

class HSliderRange : public HSlider
{
public:
	HSliderRange () : HSliderRange (0, 0, 0, 0, "", 0, 0, 0, 0) {}
	HSliderRange (const double x, const double y, const double width, const double height, const std::string& name,
		const double value, const double min, const double max, const double step, std::string format = "",
	 	std::function<double (double x)> func = [] (double x) {return x;},
		std::function<double (double x)> revfunc = [] (double x) {return x;}) :
			HSlider (x, y, width, height, name, value, min, max, step, format),
			startValue_ (min),
			endValue_ (max),
			transform_ (func),
			reverse_ (revfunc)
	{}

	HSliderRange (const HSliderRange& that) :
			HSlider (that),
			startValue_ (that.startValue_),
			endValue_ (that.endValue_),
			transform_ (that.transform_),
			reverse_ (that.reverse_)
	{}

	virtual Widget* clone () const override {return new HSliderRange (*this);}

	virtual void setValue (const double val) override
	{
		if (startValue_ <= endValue_) HSlider::setValue (LIMIT (val, startValue_, endValue_));
		else HSlider::setValue (LIMIT (val, endValue_, startValue_));
	}

	void setStartValue (const double val)
	{
		double valRounded = LIMIT (val, rangeMin, rangeMax);
		if ((rangeStep != 0.0) && (rangeMax >= rangeMin))
		{
			if (rangeStep > 0.0) valRounded = LIMIT (rangeMin + round ((val - rangeMin) / rangeStep) * rangeStep, rangeMin, rangeMax);
			else valRounded = LIMIT (rangeMax - round ((rangeMax - val) / rangeStep) * rangeStep, rangeMin, rangeMax);
		}

		if (valRounded != startValue_)
		{
			startValue_ = valRounded;
			setValue (getValue());
			postValueChanged();
			update();
		}
	}

	double getStartValue () const {return startValue_;}

	void setEndValue (const double val)
	{
		double valRounded = LIMIT (val, rangeMin, rangeMax);
		if ((rangeStep != 0.0) && (rangeMax >= rangeMin))
		{
			if (rangeStep > 0.0) valRounded = LIMIT (rangeMin + round ((val - rangeMin) / rangeStep) * rangeStep, rangeMin, rangeMax);
			else valRounded = LIMIT (rangeMax - round ((rangeMax - val) / rangeStep) * rangeStep, rangeMin, rangeMax);
		}

		if (valRounded != endValue_)
		{
			endValue_ = valRounded;
			setValue (getValue());
			postValueChanged();
			update();
		}
	}

	double getEndValue () const {return endValue_;}

	virtual void onButtonPressed (BEvents::PointerEvent* event) override {}

	virtual void onButtonReleased (BEvents::PointerEvent* event) override {}

	virtual void onPointerDragged (BEvents::PointerEvent* event) override
	{
		if (!event) return;

		BUtilities::Point pos = event->getOrigin();
		double x0 = getXOffset();
		double w = getEffectiveWidth();

		if ((w == 0) || (pos.x < x0) || (pos.x > x0 + w) || (getMin() == getMax())) return;

		double dist = getMax() - getMin();
		double startTransformed = transform_ ((startValue_ - getMin()) / dist);
		double endTransformed = transform_ ((endValue_ - getMin()) / dist);
		double x1 = x0 + startTransformed * w;
		double x2 = x0 + endTransformed * w;

		// Outside the range bar
		if
		(
			((int (pos.x) < int (x1)) && (int (pos.x) < int (x2))) ||
			((int (pos.x) > int (x1)) && (int (pos.x) > int (x2)))
		)
		{
			double nstart = LIMIT (startTransformed + event->getDelta ().x / w, 0.0, 1.0);
			double nend = LIMIT (endTransformed + event->getDelta ().x / w, 0.0, 1.0);
			setStartValue (getMin() + reverse_ (nstart) * dist);
			setEndValue (getMin() + reverse_ (nend) * dist);
		}

		// Inside the range bar
		else
		{
			double nend = LIMIT (endTransformed + event->getDelta ().x / w, 0.0, 1.0);
			setEndValue (getMin() + reverse_ (nend) * dist);
		}
	}

	virtual void onWheelScrolled (BEvents::WheelEvent* event) override
	{
		if (!event) return;

		BUtilities::Point pos = event->getPosition();
		double x0 = getXOffset();
		double w = getEffectiveWidth();

		if ((w == 0) || (pos.x < x0) || (pos.x > x0 + w) || (getMin() == getMax())) return;

		double dist = getMax() - getMin();
		double startTransformed = transform_ ((startValue_ - getMin()) / dist);
		double endTransformed = transform_ ((endValue_ - getMin()) / dist);
		double x1 = x0 + startTransformed * w;
		double x2 = x0 + endTransformed * w;

		// Outside the range bar
		if
		(
			((int (pos.x) < int (x1)) && (int (pos.x) < int (x2))) ||
			((int (pos.x) > int (x1)) && (int (pos.x) > int (x2)))
		)
		{
			double nstart = LIMIT (startTransformed + event->getDelta ().y / w, 0.0, 1.0);
			double nend = LIMIT (endTransformed + event->getDelta ().y / w, 0.0, 1.0);
			setStartValue (getMin() + reverse_ (nstart) * dist);
			setEndValue (getMin() + reverse_ (nend) * dist);
		}

		// Inside the range bar
		else
		{
			double nend = LIMIT (endTransformed + event->getDelta ().y / w, 0.0, 1.0);
			setEndValue (getMin() + reverse_ (nend) * dist);
		}
	}

protected:

	double startValue_;
	double endValue_;
	std::function<double(double)> transform_;
	std::function<double(double)> reverse_;

	virtual void draw (const BUtilities::RectArea& area) override
	{
		if ((!widgetSurface_) || (cairo_surface_status (widgetSurface_) != CAIRO_STATUS_SUCCESS)) return;

		// Draw super class widget elements first
		Widget::draw (area);

		// Draw scale only if it is not a null widget
		if ((scaleArea.getHeight() >= 1) && (scaleArea.getWidth() >= 1))
		{
			cairo_surface_clear (widgetSurface_);
			cairo_t* cr = cairo_create (widgetSurface_);

			if (cairo_status (cr) == CAIRO_STATUS_SUCCESS)
			{
				// Limit cairo-drawing area
				cairo_rectangle (cr, area.getX (), area.getY (), area.getWidth (), area.getHeight ());
				cairo_clip (cr);

				const double x0 = getXOffset ();
				const double y0 = getYOffset();
				const double h = scaleArea.getHeight();
				const double w = scaleArea.getWidth ();
				const double x1 = x0 + transform_ ((startValue_ - getMin()) / (getMax() - getMin())) * w;
				const double x2 = x0 + transform_ ((value - getMin()) / (getMax() - getMin())) * w;
				const double x3 = x0 + transform_ ((endValue_ - getMin()) / (getMax() - getMin())) * w;

				BColors::Color bgColor = *bgColors.getColor (BColors::OFF);
				BColors::Color mxColor = *bgColors.getColor (getState ()); mxColor.applyBrightness (BWIDGETS_DEFAULT_SHADOWED);
				BColors::Color slColor = *bgColors.getColor (getState ()); slColor.applyBrightness (BWIDGETS_DEFAULT_NORMALLIGHTED);
				BColors::Color fgColor = *fgColors.getColor (getState ()); fgColor.applyBrightness (BWIDGETS_DEFAULT_NORMALLIGHTED);
				BColors::Color txColor = *bgColors.getColor (getState ()); txColor.applyBrightness (BWIDGETS_DEFAULT_SHADOWED);
				BColors::Color frColor= *bgColors.getColor (getState ()); frColor.applyBrightness (BWIDGETS_DEFAULT_ILLUMINATED);

				// Slider bar
				// Background
				cairo_set_line_width (cr, 0.0);
				cairo_set_source_rgba (cr, CAIRO_RGBA (bgColor));
				cairo_rectangle (cr, x0, y0, w, h);
				cairo_fill (cr);

				// Slider (min)
				cairo_set_source_rgba (cr, CAIRO_RGBA (slColor));
				cairo_rectangle (cr, x0, y0, x1 - x0, h);
				cairo_fill (cr);

				// Slider (max)
				cairo_set_source_rgba (cr, CAIRO_RGBA (mxColor));
				cairo_rectangle (cr, x1, y0 + 0.4 * h, x3 - x1, 0.2 * h);
				cairo_fill (cr);
				cairo_set_line_width (cr, 2.0);
				cairo_move_to (cr, x3, y0 + 0.2 * h);
				cairo_rel_line_to (cr, 0, 0.6 * h);
				cairo_stroke (cr);

				// Slider (value)
				cairo_set_line_width (cr, 0.0);
				cairo_set_source_rgba (cr, CAIRO_RGBA (fgColor));
				cairo_rectangle (cr, x1, y0 + 0.4 * h, x2 - x1, 0.2 * h);
				cairo_fill (cr);

				// Frame
				cairo_set_line_width (cr, 1.0);
				cairo_rectangle (cr, x0, y0, w, h);
				cairo_set_source_rgba (cr, CAIRO_RGBA (frColor));
				cairo_stroke (cr);

				// Text
				cairo_text_extents_t ext;
				cairo_select_font_face (cr, "Sans", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_NORMAL);
				cairo_set_font_size (cr, 0.5 * h);
				const std::string valstr = BUtilities::to_string (startValue_, format_);
				cairo_text_extents (cr, valstr.c_str(), &ext);
				cairo_move_to (cr, x0 + w / 2 - ext.width / 2 - ext.x_bearing, y0 + h / 2 - ext.height / 2 - ext.y_bearing);
				cairo_set_source_rgba (cr, CAIRO_RGBA (frColor));
				cairo_show_text (cr, valstr.c_str ());
				cairo_move_to (cr, x0 + 1 + w / 2 - ext.width / 2 - ext.x_bearing, y0 + 1 + h / 2 - ext.height / 2 - ext.y_bearing);
				cairo_set_source_rgba (cr, CAIRO_RGBA (txColor));
				cairo_show_text (cr, valstr.c_str ());
			}
			cairo_destroy (cr);
		}
	}
};

#endif /* HSLIDERRANGE_HPP_ */
