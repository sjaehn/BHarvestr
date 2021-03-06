/* B.Harvestr
 * LV2 Plugin
 *
 * Copyright (C) 2018, 2019 by Sven Jähnichen
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

#ifndef BWIDGETS_LOADBUTTON_HPP_
#define BWIDGETS_LOADBUTTON_HPP_

#include "BWidgets/Button.hpp"

class LoadButton : public BWidgets::Button
{
public:
	LoadButton () : LoadButton (0.0, 0.0, BWIDGETS_DEFAULT_BUTTON_WIDTH, BWIDGETS_DEFAULT_BUTTON_HEIGHT, "loadbutton", 0.0) {}
	LoadButton (const double x, const double y, const double width, const double height, const std::string& name, double defaultValue = 0.0) :
		Button (x, y, width, height, name, defaultValue) {}

	/**
	 * Pattern cloning. Creates a new instance of the widget and copies all
	 * its properties.
	 */
	virtual Widget* clone () const override {return new LoadButton (*this);}

protected:
	virtual void draw (const BUtilities::RectArea& area) override
	{
		if ((!widgetSurface_) || (cairo_surface_status (widgetSurface_) != CAIRO_STATUS_SUCCESS)) return;

		if ((getWidth () >= 6) && (getHeight () >= 6))
		{

			Button::draw (area);

			cairo_t* cr = cairo_create (widgetSurface_);
			if (cairo_status (cr) == CAIRO_STATUS_SUCCESS)
			{
				// Limit cairo-drawing area
				cairo_rectangle (cr, area.getX (), area.getY (), area.getWidth (), area.getHeight ());
				cairo_clip (cr);

				double x0 = getXOffset ();
				double y0 = getYOffset ();
				double w = getEffectiveWidth ();
				double h = getEffectiveHeight ();
				double size = (w < h ? 0.8 * w : 0.8 * h);
				BColors::Color butColor = *bgColors.getColor (getState ()); butColor.applyBrightness (BWIDGETS_DEFAULT_NORMALLIGHTED);
				BColors::Color frColor= *bgColors.getColor (getState ());

				if (value) frColor.applyBrightness (2 * BWIDGETS_DEFAULT_ILLUMINATED);
				else frColor.applyBrightness (2 * BWIDGETS_DEFAULT_SHADOWED);

				// Symbol
				cairo_set_line_width (cr, BWIDGETS_DEFAULT_BUTTON_BORDER);
				cairo_move_to (cr, x0 + w/2, y0 + h/2 - 0.375 * size);
				cairo_line_to (cr, x0 + w/2 + 0.25 * size, y0 + h/2 - 0.125 * size);
				cairo_line_to (cr, x0 + w/2 + 0.15 * size, y0 + h/2 - 0.125 * size);
				cairo_line_to (cr, x0 + w/2 + 0.15 * size, y0 + h/2 + 0.25 * size);
				cairo_line_to (cr, x0 + w/2 - 0.15 * size, y0 + h/2 + 0.25 * size);
				cairo_line_to (cr, x0 + w/2 - 0.15 * size, y0 + h/2 - 0.125 * size);
				cairo_line_to (cr, x0 + w/2 - 0.25 * size, y0 + h/2 - 0.125 * size);
				cairo_close_path (cr);

				cairo_move_to (cr, x0 + w/2 + 0.25 * size, y0 + h/2 + 0.125 * size);
				cairo_line_to (cr, x0 + w/2 + 0.375 * size, y0 + h/2 + 0.125 * size);
				cairo_line_to (cr, x0 + w/2 + 0.375 * size, y0 + h/2 + 0.375 * size);
				cairo_line_to (cr, x0 + w/2 - 0.375 * size, y0 + h/2 + 0.375 * size);
				cairo_line_to (cr, x0 + w/2 - 0.375 * size, y0 + h/2 + 0.125 * size);
				cairo_line_to (cr, x0 + w/2 - 0.25 * size, y0 + h/2 + 0.125 * size);

				cairo_set_source_rgba (cr, CAIRO_RGBA (frColor));
				cairo_stroke (cr);

				cairo_destroy (cr);
			}
		}
	}
};

#endif /* BWIDGETS_LOADBUTTON_HPP_ */
