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

#ifndef PATTERNWIDGET_HPP_
#define PATTERNWIDGET_HPP_

#include "definitions.h"
#include <vector>
#include <cmath>
#include "BUtilities/to_string.hpp"
#include "BWidgets/Widget.hpp"
#include "Pattern.hpp"

class PatternWidget : public BWidgets::Widget, public Pattern
{
public:
	PatternWidget () : PatternWidget (0, 0, 0, 0, "pattern") {}

	PatternWidget (const double x, const double y, const double width, const double height, const std::string& name) :
			Widget (x, y, width, height, name),
			Pattern (),
			fgColors_ (BColors::reds),
			bgColors_ (BColors::darks),
			selected_ {}
	{
		setDraggable (true);
	}

	PatternWidget& operator= (const PatternWidget& that)
	{
		fgColors_ = that.fgColors_;
		bgColors_ = that.bgColors_;
		selected_ = that.selected_;
		Pattern::operator= (that);
		Widget::operator= (that);
		return *this;
	}

	virtual void setSteps (const int steps) override
	{
		Pattern::setSteps (steps);
		update();
	}

	virtual void setRows (const int rows) override
	{
		Pattern::setRows (rows);
		update();
	}

	virtual void setPattern (const PatternIndex index) override
	{
		Pattern::setPattern (index);
		update();
	}

	virtual void setValues (const int* values, const int steps) override
	{
		Pattern::setValues (values, steps);
		update();
	}

	virtual void setValues (const std::vector<int>& values) override
	{
		Pattern::setValues (values);
		update();
	}

	virtual void setValue (const int step, const int value)
	{
		Pattern::setValue (step, value);
		update();
	}

	virtual void setSelection (const std::vector<int>& steps)
	{
		selected_ = steps;
		update();
	}

	virtual void onButtonPressed (BEvents::PointerEvent* event) override
	{
		if (!event) return;

		const double s = getStepAt (event->getPosition().y);
		const double r = getRowAt (event->getPosition().x);
		setValue (s, r);

		cbfunction_[BEvents::EventType::BUTTON_PRESS_EVENT] (event);
	}

	virtual void onPointerDragged (BEvents::PointerEvent* event) override
	{
		onButtonPressed (event);
	}

	virtual void onWheelScrolled (BEvents::WheelEvent* event) override
	{
		if (!event) return;

		const double s = getStepAt (event->getPosition().y);
		const double r = LIMIT (getValue (s) + event->getDelta().y, 0, rows_ - 1);
		setValue (s, r);

		cbfunction_[BEvents::EventType::WHEEL_SCROLL_EVENT] (event);
	}

	virtual void applyTheme (BStyles::Theme& theme) override {applyTheme (theme, name_);}
	virtual void applyTheme (BStyles::Theme& theme, const std::string& name) override
	{
		Widget::applyTheme (theme, name);

		// Foreground colors (curve)
		const void* fgPtr = theme.getStyle(name, BWIDGETS_KEYWORD_FGCOLORS);
		if (fgPtr) fgColors_ = *((BColors::ColorSet*) fgPtr);

		// Background colors (grid)
		const void* bgPtr = theme.getStyle(name, BWIDGETS_KEYWORD_BGCOLORS);
		if (bgPtr) bgColors_ = *((BColors::ColorSet*) bgPtr);

		if (fgPtr || bgPtr) update ();

	}

protected:
	BColors::ColorSet fgColors_;
	BColors::ColorSet bgColors_;
	std::vector<int> selected_;

	int getStepAt (const double ypos)
	{
		double y0 = getYOffset ();
		double h = getEffectiveHeight ();

		if (h == 0.0) return 0;

		int s = int (double (steps_) * (h - (ypos - y0)) / h);
		return (LIMIT (s, 0, steps_ - 1));
	}

	int getRowAt (const double xpos)
	{
		double x0 = getXOffset ();
		double w = getEffectiveWidth ();

		if (w == 0.0) return 0;

		int r = int (double (rows_) * (xpos - x0) / w);
		return (LIMIT (r, 0, rows_ - 1));
	}

	virtual void draw (const BUtilities::RectArea& area) override
	{
		if ((!widgetSurface_) || (cairo_surface_status (widgetSurface_) != CAIRO_STATUS_SUCCESS)) return;

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

			BColors::Color btColor = *fgColors_.getColor (getState());
			BColors::Color bgColor = *bgColors_.getColor (getState());
			BColors::Color btiColor = btColor; btiColor.applyBrightness (0.05);
			BColors::Color btdColor = btColor; btdColor.applyBrightness (-0.33);
			BColors::Color bsiColor = btColor; bsiColor.applyBrightness (0.5);
			BColors::Color bsdColor = btColor; bsdColor.applyBrightness (0.33);
			BColors::Color bgiColor = bgColor; bgiColor.applyBrightness (0.05);
			BColors::Color bgdColor = bgColor; bgdColor.applyBrightness (-0.33);

			if (w && h)
			{
				const double padw = w / rows_;
				const double padh = h / steps_;

				if ((padw < 4.0) || (padh < 4.0))
				{
					cairo_set_source_rgba (cr, CAIRO_RGBA (bgiColor));
					cairo_set_line_width (cr, 0.0);
					cairo_rectangle (cr, x0, y0, w, h);
					cairo_fill (cr);

					for (int s = 0; s < steps_; ++s)
					{
						cairo_set_source_rgba (cr, CAIRO_RGBA (btiColor));
						for (int sel : selected_)
						{
							if (s == sel)
							{
								cairo_set_source_rgba (cr, CAIRO_RGBA (bsiColor));
								break;
							}
						}
						cairo_rectangle (cr, x0 + getValue (s) * padw, y0 + h - (s + 1) * padh, padw, padh);
						cairo_fill (cr);
					}

				}


				else
				{
					const double padrd = ((padw < 20) || (padh < 20) ?  (padw < padh ? padw : padh) / 4 : 5);
					const double pr = (padw < padh ? padh : padw);
					for (int s = 0; s < steps_; ++s)
					{
						for (int r = 0; r < rows_; ++r)
						{
							const double x = x0 + r * padw;
							const double y = y0 + h - (s + 1) * padh;

							// Draw background
							cairo_set_source_rgba (cr, CAIRO_RGBA (bgColor));
							cairo_set_line_width (cr, 0.0);
							cairo_rectangle (cr, x, y, padw, padh);
							cairo_fill (cr);


							BColors::Color illuminated =  bgiColor;
							BColors::Color darkened = bgdColor;
							if (r == getValue (s))
							{
								illuminated = btiColor;
								darkened = btdColor;

								for (int sel : selected_)
								{
									if (s == sel)
									{
										illuminated = bsiColor;
										darkened = bsdColor;
										break;
									}
								}
							}

							cairo_pattern_t* pat = cairo_pattern_create_radial
							(
								x + padw / 2, y + padh / 2, 0.125 * pr,
								x + padw / 2, y + padh / 2, 0.5 * pr);

							cairo_pattern_add_color_stop_rgba (pat, 0.0, CAIRO_RGBA (illuminated));
							cairo_pattern_add_color_stop_rgba (pat, 1.0, CAIRO_RGBA (darkened));

							cairo_rectangle_rounded (cr, x, y, padw, padh, padrd);
							cairo_set_source (cr, pat);
							cairo_fill (cr);
							cairo_pattern_destroy (pat);

						}
					}
				}
			}

			cairo_destroy (cr);
		}
	}
};

#endif /* PATTERNWIDGET_HPP_ */
