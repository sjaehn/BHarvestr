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

#ifndef HSLIDER_HPP_
#define HSLIDER_HPP_

#include "BWidgets/HScale.hpp"
#include "BUtilities/to_string.hpp"

class HSlider : public BWidgets::HScale
{
public:
	HSlider () : HSlider (0, 0, 0, 0, "", 0, 0, 0, 0) {}
	HSlider (const double x, const double y, const double width, const double height, const std::string& name,
		 const double value, const double min, const double max, const double step, std::string format = "",
 	 	 std::function<double (double x)> func = [] (double x) {return x;},
 		 std::function<double (double x)> revfunc = [] (double x) {return x;}) :
			HScale (x, y, width, height, name, value, min, max, step),
			format_ (format),
			transform_ (func),
			reverse_ (revfunc)
	{}

	virtual Widget* clone () const override {return new HSlider (*this);}

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
		double valueTransformed = transform_ ((getValue() - getMin()) / dist);
		double nval = LIMIT (valueTransformed + event->getDelta ().x / w, 0.0, 1.0);
		setValue (getMin() + reverse_ (nval) * dist);
	}

	virtual void onWheelScrolled (BEvents::WheelEvent* event) override
	{
		if (!event) return;

		BUtilities::Point pos = event->getPosition();
		double x0 = getXOffset();
		double w = getEffectiveWidth();

		if ((w == 0) || (pos.x < x0) || (pos.x > x0 + w) || (getMin() == getMax())) return;

		double dist = getMax() - getMin();
		double valueTransformed = transform_ ((getValue() - getMin()) / dist);
		double nval = LIMIT (valueTransformed + event->getDelta ().y / w, 0.0, 1.0);
		setValue (getMin() + reverse_ (nval) * dist);
	}

protected:
	std::string format_;
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
				const double y0 = getYOffset ();
				const double h = scaleArea.getHeight ();
				const double w = scaleArea.getWidth ();
				const double x1 = x0 + transform_ ((value - getMin()) / (getMax() - getMin())) * w;;

				BColors::Color bgColor = *bgColors.getColor (BColors::OFF);
				BColors::Color slColor = *bgColors.getColor (getState ()); slColor.applyBrightness (BWIDGETS_DEFAULT_NORMALLIGHTED);
				BColors::Color txColor = *bgColors.getColor (getState ()); txColor.applyBrightness (BWIDGETS_DEFAULT_SHADOWED);
				BColors::Color frColor= *bgColors.getColor (getState ()); frColor.applyBrightness (BWIDGETS_DEFAULT_ILLUMINATED);

				// Slider bar
				cairo_set_line_width (cr, 0.0);
				cairo_set_source_rgba (cr, CAIRO_RGBA (bgColor));
				cairo_rectangle (cr, x0, y0, w, h);
				cairo_fill (cr);

				cairo_set_source_rgba (cr, CAIRO_RGBA (slColor));
				cairo_rectangle (cr, x0, y0, x1 - x0, h);
				cairo_fill (cr);

				cairo_set_line_width (cr, 1.0);
				cairo_rectangle (cr, x0, y0, w, h);
				cairo_set_source_rgba (cr, CAIRO_RGBA (frColor));
				cairo_stroke (cr);

				// Text
				cairo_text_extents_t ext;
				cairo_select_font_face (cr, "Sans", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_NORMAL);
				cairo_set_font_size (cr, 0.5 * h);
				const std::string valstr = BUtilities::to_string (getValue(), format_);
				cairo_text_extents (cr, valstr.c_str(), &ext);
				cairo_move_to (cr, w / 2 - ext.width / 2 - ext.x_bearing, h / 2 - ext.height / 2 - ext.y_bearing);
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

#endif /* HSLIDER_HPP_ */
