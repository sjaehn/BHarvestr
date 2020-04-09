/* B.Harvestr
 * LV2 Plugin
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

#ifndef MINIMAXIMIZEBUTTON_HPP_
#define MINIMAXIMIZEBUTTON_HPP_

#include "BWidgets/ToggleButton.hpp"

class MiniMaximizeButton : public BWidgets::ToggleButton
{
public:
	MiniMaximizeButton () : MiniMaximizeButton (0.0, 0.0, BWIDGETS_DEFAULT_BUTTON_WIDTH, BWIDGETS_DEFAULT_BUTTON_HEIGHT, "button", 0.0) {}

	MiniMaximizeButton (const double x, const double y, const double width, const double height, const std::string& name, double defaultValue = 0) :
			ToggleButton (x, y, width, height, name, defaultValue),
			minPosition_ (x, y),
			maxPosition_ (x, y),
			parentMinArea_(),
			parentMaxArea_() {}

	void setParentMinimizeArea (const BUtilities::RectArea& area)
	{
		parentMinArea_ = area;
		if (!value) onValueChanged (nullptr);
	}

	void setParentMaximizeArea (const BUtilities::RectArea& area)
	{
		parentMaxArea_ = area;
		if (value) onValueChanged (nullptr);
	}

	virtual void moveTo (const double x, const double y) override {moveTo (BUtilities::Point (x, y));}

	virtual void moveTo (const BUtilities::Point& position) override
	{
		minPosition_ = position;
		maxPosition_ = position;
		Widget::moveTo (position);
	}

	virtual void moveTo (const BUtilities::Point& minPosition, const BUtilities::Point& maxPosition)
	{
		minPosition_ = minPosition;
		maxPosition_ = maxPosition;
		Widget::moveTo (value ? maxPosition : minPosition);
	}

	virtual void onValueChanged (BEvents::ValueChangedEvent* event) override
	{
		if (parent_)
		{
			BUtilities::RectArea area = (value ? parentMaxArea_ : parentMinArea_);
			parent_->moveTo (area.getPosition());
			parent_->resize (area.getExtends());
			parent_->raiseToTop();
		}

		Widget::moveTo (value ? maxPosition_ : minPosition_);

		if (event) ToggleButton::onValueChanged (event);
	}

protected:
	BUtilities::Point minPosition_;
	BUtilities::Point maxPosition_;
	BUtilities::RectArea parentMinArea_;
	BUtilities::RectArea parentMaxArea_;

	virtual void draw (const BUtilities::RectArea& area) override
	{
		if ((!widgetSurface_) || (cairo_surface_status (widgetSurface_) != CAIRO_STATUS_SUCCESS)) return;

		if ((area.getWidth() >= 6) && (area.getHeight() >= 6))
		{
			// Draw super class widget elements first
			Widget::draw (area);

			cairo_t* cr = cairo_create (widgetSurface_);
			if (cairo_status (cr) == CAIRO_STATUS_SUCCESS)
			{
				// Limit cairo-drawing area
				cairo_rectangle (cr, area.getX(), area.getY(), area.getWidth(), area.getHeight());
				cairo_clip (cr);

				double x0 = getXOffset ();
				double y0 = getYOffset ();
				double w = getEffectiveWidth ();
				double h = getEffectiveHeight ();
				BColors::Color butColor = *bgColors.getColor (value == 1 ? BColors::ACTIVE : BColors::NORMAL);

				if (value)
				{
					cairo_move_to (cr, x0, y0 + 0.75 * h);
					cairo_line_to (cr, x0 + 0.5 * w, y0 + 0.25 * h);
					cairo_line_to (cr, x0 + w, y0 + 0.75 * h);
				}
				else
				{
					cairo_move_to (cr, x0, y0 + 0.25 * h);
					cairo_line_to (cr, x0 + 0.5 * w, y0 + 0.75 * h);
					cairo_line_to (cr, x0 + w, y0 + 0.25 * h);
				}

				cairo_set_line_width (cr, 2);
				cairo_set_source_rgba (cr, CAIRO_RGBA (butColor));
				cairo_stroke (cr);

				cairo_destroy (cr);
			}
		}
	}
};

#endif /* MINIMAXIMIZEBUTTON_HPP_ */
