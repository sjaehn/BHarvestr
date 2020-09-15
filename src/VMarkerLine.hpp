/* B.Choppr
 * Step Sequencer Effect Plugin
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

#ifndef VMARKERLINE_HPP_
#define VMARKERLINE_HPP_

#include "BWidgets/RangeWidget.hpp"
#include "BWidgets/Focusable.hpp"
#include "BWidgets/Label.hpp"
#include "BUtilities/to_string.hpp"

class VMarkerLine : public BWidgets::RangeWidget, public BWidgets::Focusable
{
protected:
        BColors::ColorSet bgColors;
        BWidgets::Label focusLabel;

public:
        VMarkerLine () :
        VMarkerLine (0.0, 0.0, BWIDGETS_DEFAULT_WIDTH, BWIDGETS_DEFAULT_HEIGHT, "marker",
                BWIDGETS_DEFAULT_VALUE, BWIDGETS_DEFAULT_RANGE_MIN, BWIDGETS_DEFAULT_RANGE_MAX, BWIDGETS_DEFAULT_RANGE_STEP) {}

        VMarkerLine (const double x, const double y, const double width, const double height, const std::string& name,
                const double value, const double min, const double max, const double step) :
        RangeWidget (x, y, width, height, name, value, min, max, step),
        Focusable (std::chrono::milliseconds (BWIDGETS_DEFAULT_FOCUS_IN_MS),
                   std::chrono::milliseconds (BWIDGETS_DEFAULT_FOCUS_OUT_MS)),
        bgColors (BWIDGETS_DEFAULT_BGCOLORS),
        focusLabel (0 ,0, 40, 20, name_ + BWIDGETS_DEFAULT_FOCUS_NAME, "")
        {
                std::string valstr = BUtilities::to_string (getValue());
        	focusLabel.setText (valstr);
        	focusLabel.setStacking (BWidgets::STACKING_OVERSIZE);
        	focusLabel.resize ();
        	focusLabel.hide ();
        	add (focusLabel);
        }

        VMarkerLine (const VMarkerLine& that) :
        RangeWidget (that), Focusable (that), bgColors (that.bgColors),
        focusLabel (that.focusLabel)
        {
                focusLabel.hide();
        	add (focusLabel);
        }

        VMarkerLine& operator= (const VMarkerLine& that)
        {
                release (&focusLabel);

                bgColors = that.bgColors;
                focusLabel = that.focusLabel;
        	focusLabel.hide();
        	RangeWidget::operator= (that);
                Focusable::operator= (that);

                add (focusLabel);

        	return *this;
        }

        virtual Widget* clone () const override {return new VMarkerLine (*this);}

        virtual void setValue (const double val) override
        {
        	RangeWidget::setValue (val);
        	std::string valstr = BUtilities::to_string (value) + " s";
        	focusLabel.setText(valstr);
        	focusLabel.resize ();
        }

        virtual void update () override
        {
        	RangeWidget::update ();
        	focusLabel.resize ();
        }

        virtual void onFocusIn (BEvents::FocusEvent* event) override
        {
        	if (event && event->getWidget())
        	{
        		BUtilities::Point pos = event->getPosition();
        		focusLabel.moveTo (pos.x - 0.5 * focusLabel.getWidth(), pos.y - focusLabel.getHeight());
        		focusLabel.show();
        	}
        	Widget::onFocusIn (event);
        }
        virtual void onFocusOut (BEvents::FocusEvent* event) override
        {
        	if (event && event->getWidget()) focusLabel.hide();
        	Widget::onFocusOut (event);
        }

        virtual void applyTheme (BStyles::Theme& theme) override {applyTheme (theme, name_);}
	virtual void applyTheme (BStyles::Theme& theme, const std::string& name) override
        {
                Widget::applyTheme (theme, name);
                focusLabel.applyTheme (theme, name + BWIDGETS_DEFAULT_FOCUS_NAME);

        	void* bgPtr = theme.getStyle(name, BWIDGETS_KEYWORD_BGCOLORS);
        	if (bgPtr)
        	{
        		bgColors = *((BColors::ColorSet*) bgPtr);
        		update ();
        	}
        }

        virtual void draw (const BUtilities::RectArea& area) override
        {
                if ((!widgetSurface_) || (cairo_surface_status (widgetSurface_) != CAIRO_STATUS_SUCCESS)) return;

        	// Draw super class widget elements first
        	Widget::draw (area);

                double x0 = getXOffset ();
                double y0 = getYOffset ();
                double heff = getEffectiveHeight ();
        	double weff = getEffectiveWidth ();

        	// Draw knobs
        	// only if minimum requirements satisfied
        	if ((heff >= 1) && (weff >= 1))
        	{
        		cairo_surface_clear (widgetSurface_);
        		cairo_t* cr = cairo_create (widgetSurface_);

        		if (cairo_status (cr) == CAIRO_STATUS_SUCCESS)
        		{
        			// Limit cairo-drawing area
        			cairo_rectangle (cr, area.getX (), area.getY (), area.getWidth (), area.getHeight ());
        			cairo_clip (cr);

        			// Colors uses within this method
                                BColors::Color cLo = *bgColors.getColor (getState ()); cLo.applyBrightness (BWIDGETS_DEFAULT_NORMALLIGHTED);
        			BColors::Color cMid = *bgColors.getColor (getState ()); cMid.applyBrightness ((BWIDGETS_DEFAULT_ILLUMINATED + BWIDGETS_DEFAULT_NORMALLIGHTED) / 2);
                                BColors::Color cHi = *bgColors.getColor (getState ()); cHi.applyBrightness (BWIDGETS_DEFAULT_ILLUMINATED);

                                // Top marker
                                cairo_set_line_width (cr, 0.0);
                                cairo_set_source_rgba (cr, CAIRO_RGBA (cMid));
                                cairo_move_to (cr, x0 + 0.5 * weff, y0 + 1.5 * weff);
                                cairo_line_to (cr, x0, y0 + 1.5 * 0.75 * weff);
                                cairo_line_to (cr, x0, y0);
                                cairo_line_to (cr, x0 + weff, y0);
                                cairo_line_to (cr, x0 + weff, y0 + 1.5 * 0.75 * weff);
                                cairo_close_path (cr);
                                cairo_fill (cr);

                                cairo_set_line_width (cr, 0.05 * weff);
                                cairo_set_source_rgba (cr, CAIRO_RGBA (cLo));
                                for (int i = 0; i < 3; ++i)
                                {
                                        cairo_move_to (cr, x0 + 0.25 * weff, y0 + 1.5 * (0.3 + (i * 0.1)) * weff);
                                        cairo_rel_line_to (cr, 0.5 * weff, 0);
                                }
                                cairo_stroke (cr);

                                cairo_set_line_width (cr, 0.1 * weff);
                                cairo_set_source_rgba (cr, CAIRO_RGBA (cHi));
                                cairo_move_to (cr, x0 + weff, y0);
                                cairo_line_to (cr, x0 + weff, y0 + 1.5 * 0.75 * weff);
                                cairo_line_to (cr, x0 + 0.5 * weff, y0 + 1.5 * weff);
                                cairo_stroke (cr);

                                // Marker line
                                cairo_set_line_width (cr, 0.2 * weff);
                                cairo_set_source_rgba (cr, CAIRO_RGBA (cHi));
                                cairo_move_to (cr, x0 + 0.5 * weff, y0 + 1.5 * weff);
                                cairo_rel_line_to (cr, 0, heff - 3 * weff);
                                cairo_stroke (cr);

                                // Bottom marker
        			cairo_set_line_width (cr, 0.0);
                                cairo_set_source_rgba (cr, CAIRO_RGBA (cMid));
                                cairo_move_to (cr, x0 + 0.5 * weff, y0 + heff - 1.5 * weff);
                                cairo_line_to (cr, x0, y0 + heff - 1.5 * 0.75 * weff);
                                cairo_line_to (cr, x0, y0 + heff);
                                cairo_line_to (cr, x0 + weff, y0 + heff);
                                cairo_line_to (cr, x0 + weff, y0 + heff - 1.5 * 0.75 * weff);
                                cairo_close_path (cr);
                                cairo_fill (cr);

                                cairo_set_line_width (cr, 0.05 * weff);
                                cairo_set_source_rgba (cr, CAIRO_RGBA (cLo));
                                for (int i = 0; i < 3; ++i)
                                {
                                        cairo_move_to (cr, x0 + 0.25 * weff, y0 + heff - 1.5 * (0.3 + (i * 0.1)) * weff);
                                        cairo_rel_line_to (cr, 0.5 * weff, 0);
                                }
                                cairo_stroke (cr);

                                cairo_set_line_width (cr, 0.1 * weff);
                                cairo_set_source_rgba (cr, CAIRO_RGBA (cHi));
                                cairo_move_to (cr, x0, y0 + heff);
                                cairo_line_to (cr, x0 + weff, y0 + heff);
                                cairo_line_to (cr, x0 + weff, y0 + heff - 1.5 * 0.75 * weff);
                                cairo_stroke (cr);
        		}

        		cairo_destroy (cr);
        	}
        }
};

#endif /* VMARKERLINE_HPP_ */
