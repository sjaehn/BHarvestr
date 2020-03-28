/* B.Harvestr
 * LV2 Plugin
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

#ifndef DEFINITIONS_H_
#define DEFINITIONS_H_

#define BHARVESTR_URI "https://www.jahnichen.de/plugins/lv2/BHarvestr"
#define BHARVESTR_GUI_URI "https://www.jahnichen.de/plugins/lv2/BHarvestr#gui"

#define MAXPATTERNSTEPS 1024
#define DISPLAYDATASIZE 2048
#define MAXSHAPENODES 16
#define MAXVOICES 16
#define MAXGRAINS 64
#define MAXSHAPES 9
#define MAXNODES 16
#define MAXUNDO 20
#define GRIDSIZE 2.0

#ifndef LIMIT
#define LIMIT(val, min, max) ((val) > (max) ? (max) : ((val) < (min) ? (min) : (val)))
#endif /* LIMIT */

#endif /* DEFINITIONS_H_ */
