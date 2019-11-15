/*
 * Copyright (C) 2007-2019 Automation technology laboratory,
 * Helsinki University of Technology
 *
 * Visit automation.tkk.fi for information about the automation
 * technology laboratory.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 3
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston,
 * MA 02111-1307, USA.
 */

#ifndef XML_PARSER_H
#define XML_PARSER_H

// Function parses a .xml file that is imported from PoolEdit program.
// - start() and end() functions are called when a new element is
//   started or ended.
// - ready() is called when parsing is done, and an array with
//   ISOBUS data is returned parameters vtDimension_, vtSkWidth_,
//   vtSkHeight_ and vtColors_ give info about VT

typedef struct pool_xform {
    float dm_mult;
    float sk_mult;
    int dm_dx;
    int dm_dy;
    int sk_dx;
    int sk_dy;
} pool_xform_t;

// the transform becomes available after the root object has been
// parsed
pool_xform_t *get_pool_xform();

void parse(FILE *file, void (*start_)(void *data, char *el, const char **attr),
    void (*end_) (void *data, char *el), void (*ready)(char *data, int length),
    int vtDimension_, int vtSkWidth_, int vtSkHeight_, int vtColors_);

#endif
