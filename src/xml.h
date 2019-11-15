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

#ifndef XML_HELP_H
#define XML_HELP_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define ROLE_NONE 0
#define ROLE_ACTIVE_MASK 1
#define ROLE_FONT_ATTRIBUTES 2
#define ROLE_SOFT_KEY_MASK 3
#define ROLE_VARIABLE_REFERENCE 4
#define ROLE_FOREGROUND_COLOR 5
#define ROLE_INPUT_ATTRIBUTES 6
#define ROLE_LINE_ATTRIBUTES 7
#define ROLE_FILL_ATTRIBUTES 8
#define ROLE_TARGET_VARIABLE_REFERENCE 9
#define ROLE_FILL_PATTERN 10
#define ROLE_OBJECT_POINTER_VALUE 11

// returns the value of given attribute, or null if attribute doesn't
// exist
char *getAttribute(const char **attrs, const char *name);

// returns the value of given attribute, or ends program if attribute
// doesn't exist
char *getAttributeError(const char **attrs, const char *name);

char *getName(const char **attrs);
int isName(const char **attrs, const char *name);

// returns the value of id-attribute
int getId(const char **attrs);

// returns the value of x-attribute
int getX(const char **attrs);

// returns the value of y-attribute
int getY(const char **attrs);

// returns the role of object or include_object
int getRole(const char **attrs);

// retruns the nearest color. colors is the maxium number of colors
// (2,16 or 256)
int reduceColor(int color, int colors);

// returns the color
int getBackgroundColor(const char **attrs, int colors);
int getBorderColor(const char **attrs, int colors);
int getFontColor(const char **attrs, int colors);
int getLineColor(const char **attrs, int colors);
int getNeedleColor(const char **attrs, int colors);
int getArcAndTickColor(const char **attrs, int colors);
int getColorColor(const char **attrs, int colors);
int getTargetLineColor(const char **attrs, int colors);
int getTransparencyColor(const char **attrs);
int getFillColor(const char **attrs, int colors);

// returns the value of selectable-attribute (0 or 1)
int isSelectable(const char **attrs);
int isHidden(const char **attrs);
int isLatchable(const char **attrs);
int isEnabled(const char **attrs);

// returns the priority (0,1 or 2)
int getPriority(const char **attrs);

// returns the acoustic signal (0,1,2 or 3)
int getAcousticSignal(const char **attrs);

// returns the horizontal justification (0,1 or 2)
int getHorizontalJustification(const char **attrs);

// returns the line direction (0, 1)
int getLineDirection(const char **attrs);

// returns the line supression
int getLineSuppression(const char **attrs);

int getWidth(const char **attrs);
int getHeight(const char **attrs);
int getActualWidth(const char **attrs);
int getActualHeight(const char **attrs);
int getKeyCode(const char **attrs);
unsigned int getValue(const char **attrs);
unsigned int getTargetValue(const char **attrs);
unsigned int getMinValue(const char **attrs);
unsigned int getMaxValue(const char **attrs);
int getOffset(const char **attrs);
float getScale(const char **attrs);
int getNumberOfDecimals(const char **attrs);
int getNumberOfTicks(const char **attrs);
int getLength(const char **attrs);
int getStartAngle(const char **attrs);
int getEndAngle(const char **attrs);
int getLineWidth(const char **attrs);
int getBarGraphWidth(const char **attrs);
int getInputID(const char **attrs);

int getLineArt(const char **attrs);

int getFontType(const char **attrs);
int getEllipseType(const char **attrs);
int getPolygonType(const char **attrs);
int getFunctionType(const char **attrs);

// returns the value-attribute as string, that is given length given
// string must be freed!
char *getValueString(const char **attrs, int length);
char *getValidatioinString(const char **attrs, int length);

int getInputStringOptions(const char **attrs);
int getInputNumberOptions(const char **attrs);
int getMeterOptions(const char **attrs);
int getLinearBarGraphOptions(const char **attrs);
int getArchedBarGraphOptions(const char **attrs);
int getPictureGraphicOptions(const char **attrs);
int getNumberFormat(const char **attrs);
int getFontStyle(const char **attrs);
int getFillType(const char **attrs);
int getFontSize(const char **attrs);
int getValidationType(const char **attrs);

// get dimensions
int getDimension(const char **attrs);
int getSkWidth(const char **attrs);
int getSkHeight(const char **attrs);

// for commands
int getObjectId(const char **attrs);
int getHideShow(const char **attrs);
int getEnableDisable(const char **attrs);
int getRepetitions(const char **attrs);
int getFrequency(const char **attrs);
int getOnTime(const char **attrs);
int getOffTime(const char **attrs);
int getParentId(const char **attrs);
int getChildId(const char **attrs);
int getVolume(const char **attrs);
int getDx(const char **attrs);
int getDy(const char **attrs);
int getMaskType(const char **attrs);
int getAID(const char **attrs);
int getFillPatternID(const char **attrs);
int getPosX(const char **attrs);
int getPosY(const char **attrs);
int getListIndex(const char **attrs);

// for getting multipliers
float getMultiplier(const char **attrs, float old, float mask, float designator);

// for getting block font / col / row
int getBlockFontWidth(const char **attrs, int fontMultiplier);
int getBlockFontHeight(const char **attrs, int fontMultiplier);
int getBlockCol(const char **attrs);
int getBlockRow(const char **attrs);

#endif
