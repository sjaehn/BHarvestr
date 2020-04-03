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

#ifndef DIAL_HPP_
#define DIAL_HPP_

#include "BWidgets/RangeWidget.hpp"
#include "BUtilities/to_string.hpp"

class Dial : public BWidgets::RangeWidget
{
public:
	Dial () : Dial (0, 0, 0, 0, "", 0, 0, 0, 0) {}
	Dial (const double x, const double y, const double width, const double height, const std::string& name,
		 const double value, const double min, const double max, const double step, std::string format = "",
		 std::string unit = "",
 	 	 std::function<double (double x)> func = [] (double x) {return x;},
 		 std::function<double (double x)> revfunc = [] (double x) {return x;}) :
			RangeWidget (x, y, width, height, name, value, min, max, step),
			format_ (format),
			unit_ (unit),
			transform_ (func),
			reverse_ (revfunc),
			fgColors_ (BWIDGETS_DEFAULT_FGCOLORS),
			bgColors_ (BWIDGETS_DEFAULT_BGCOLORS)
	{
		setDraggable (true);
	}

	virtual Widget* clone () const override {return new Dial (*this);}

	virtual void applyTheme (BStyles::Theme& theme) override {applyTheme (theme, name_);}

	virtual void applyTheme (BStyles::Theme& theme, const std::string& name) override
	{
		Widget::applyTheme (theme, name);

		// Foreground colors (scale)
		void* fgPtr = theme.getStyle(name, BWIDGETS_KEYWORD_FGCOLORS);
		if (fgPtr) fgColors_ = *((BColors::ColorSet*) fgPtr);

		// Background colors (scale background, knob)
		void* bgPtr = theme.getStyle(name, BWIDGETS_KEYWORD_BGCOLORS);
		if (bgPtr) bgColors_ = *((BColors::ColorSet*) bgPtr);

		if (fgPtr || bgPtr) update ();

	}

	virtual void onButtonPressed (BEvents::PointerEvent* event) override {}

	virtual void onButtonReleased (BEvents::PointerEvent* event) override {}

	virtual void onPointerDragged (BEvents::PointerEvent* event) override
	{
		if (!event) return;

		double w = getEffectiveWidth();
		double h = getEffectiveHeight();
		double d = (w < h ? w : h);

		if ((d == 0) || (getMin() == getMax())) return;

		double dist = getMax() - getMin();
		double valueTransformed = transform_ ((getValue() - getMin()) / dist);
		double nval = LIMIT (valueTransformed - event->getDelta ().y / 2.0 / w, 0.0, 1.0);
		setValue (getMin() + reverse_ (nval) * dist);
	}

	virtual void onWheelScrolled (BEvents::WheelEvent* event) override
	{
		if (!event) return;

		double w = getEffectiveWidth();
		double h = getEffectiveHeight();
		double d = (w < h ? w : h);

		if ((d == 0) || (getMin() == getMax())) return;

		double dist = getMax() - getMin();
		double valueTransformed = transform_ ((getValue() - getMin()) / dist);
		double nval = LIMIT (valueTransformed + event->getDelta ().y / 2.0 / w, 0.0, 1.0);
		setValue (getMin() + reverse_ (nval) * dist);
	}

protected:
	std::string format_;
	std::string unit_;
	std::function<double(double)> transform_;
	std::function<double(double)> reverse_;
	BColors::ColorSet fgColors_;
	BColors::ColorSet bgColors_;

	virtual void draw (const BUtilities::RectArea& area) override
	{
		if ((!widgetSurface_) || (cairo_surface_status (widgetSurface_) != CAIRO_STATUS_SUCCESS)) return;

		// Draw super class widget elements first
		Widget::draw (area);

		const double x0 = getXOffset ();
		const double y0 = getYOffset ();
		const double h = getEffectiveHeight ();
		const double w = getEffectiveWidth ();
		const double d = (w < h ? w : h);
		const double xc = x0 + 0.5 * w;
		const double yc = y0 + 0.5 * h;
		const double alpha = transform_ ((value - getMin()) / (getMax() - getMin()));
		const double x1 = xc + 0.5 * d * cos ((0.5 + 2 * alpha) * M_PI);
		const double y1 = yc + 0.5 * d * sin ((0.5 + 2 * alpha) * M_PI);

		// Draw scale only if it is not a null widget
		if (d > 0)
		{
			cairo_surface_clear (widgetSurface_);
			cairo_t* cr = cairo_create (widgetSurface_);

			if (cairo_status (cr) == CAIRO_STATUS_SUCCESS)
			{
				// Limit cairo-drawing area
				cairo_rectangle (cr, area.getX (), area.getY (), area.getWidth (), area.getHeight ());
				cairo_clip (cr);

				BColors::Color slColor = *fgColors_.getColor (getState ()); slColor.applyBrightness (BWIDGETS_DEFAULT_NORMALLIGHTED);
				BColors::Color txColor = *fgColors_.getColor (getState ()); txColor.applyBrightness (BWIDGETS_DEFAULT_NORMALLIGHTED);

				// Circle
				cairo_set_line_width (cr, 1.0);
				cairo_pattern_t* pat = cairo_pattern_create_linear (x1, y1, xc, yc);
				cairo_pattern_add_color_stop_rgba (pat, 0, slColor.getRed (), slColor.getGreen (), slColor.getBlue (), slColor.getAlpha ());
				cairo_pattern_add_color_stop_rgba (pat, 1, slColor.getRed (), slColor.getGreen (), slColor.getBlue (), 0.1 * slColor.getAlpha ());
				cairo_set_source (cr, pat);
				cairo_arc (cr,xc, yc, 0.49 * d, M_PI * (0.54 + 2.0 * alpha) , M_PI * (2.5 + 2.00 * alpha));
				cairo_stroke (cr);
				cairo_pattern_destroy (pat);

				// Arc
				cairo_set_line_width (cr, 0.0);
				cairo_set_source_rgba (cr, CAIRO_RGBA (slColor));
				cairo_arc (cr,xc, yc, 0.42 * d, M_PI * 0.5, M_PI * (0.5 + alpha * 2.0));
				cairo_arc_negative (cr, xc, yc , 0.32 * d, M_PI * (0.5 + alpha * 2.0), M_PI * 0.5);
				cairo_close_path (cr);
				cairo_fill (cr);

				// Text
				double txtlines = (unit_ == "" ? 1.0 : 2.0);
				cairo_text_extents_t ext;
				cairo_select_font_face (cr, "Sans", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_NORMAL);
				cairo_set_font_size (cr, 0.18 * d);

				const std::string valstr = BUtilities::to_string (getValue(), format_);
				cairo_text_extents (cr, valstr.c_str(), &ext);
				cairo_move_to (cr, w / 2 - ext.width / 2 - ext.x_bearing, h / 2 - txtlines * ext.height / 2 - ext.y_bearing);
				cairo_set_source_rgba (cr, CAIRO_RGBA (txColor));
				cairo_show_text (cr, valstr.c_str ());

				if (txtlines > 1.0)
				{
					cairo_text_extents (cr, unit_.c_str(), &ext);
					cairo_move_to (cr, w / 2 - ext.width / 2 - ext.x_bearing, h / 2 + ext.height / 2 - ext.y_bearing);
					cairo_set_source_rgba (cr, CAIRO_RGBA (txColor));
					cairo_show_text (cr, unit_.c_str ());
				}

			}
			cairo_destroy (cr);
		}
	}
};

#endif /* DIAL_HPP_ */
