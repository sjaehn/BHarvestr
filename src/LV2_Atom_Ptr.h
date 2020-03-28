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

#ifndef LV2_ATOM_PTR_H_
#define LV2_ATOM_PTR_H_

#include <lv2/lv2plug.in/ns/ext/atom/atom.h>

#ifdef __cplusplus
extern "C" {
#endif

struct LV2_Atom_Ptr
{
        LV2_Atom atom;
        void* data;
};

#ifdef __cplusplus
}  /* extern "C" */
#endif

#endif /* LV2_ATOM_PTR_H_ */
