/* B.Harvestr
 * LV2 Plugin
 *
 * Copyright (C) 2018 by Sven Jähnichen
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

#ifndef PRESETINFO_HPP_
#define PRESETINFO_HPP_

#define PRESETINFO_MAX_TXT_SIZE 1024


struct PresetInfo
{
        char name[PRESETINFO_MAX_TXT_SIZE];
        char type[PRESETINFO_MAX_TXT_SIZE];
        int date;
        char creator[PRESETINFO_MAX_TXT_SIZE];
        char uri[PRESETINFO_MAX_TXT_SIZE];
        char license[PRESETINFO_MAX_TXT_SIZE];
        char description[PRESETINFO_MAX_TXT_SIZE];
};

#endif /* PRESETINFO_HPP_ */
