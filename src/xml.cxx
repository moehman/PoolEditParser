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

#include "xml.h"

// look xml.h for function definitions

int atoi2(char *str)
{
    // check for empty string
    if (str == NULL || str[0] == 0)
        return 0;

    return atoi(str);
}

/*
float atof2(char *str)
{
    //check for empty string
    if (str == NULL || str[0] == 0)
	return 0.0f;

    return atof(str);
}
*/

char *getAttribute(const char **attrs, const char *name)
{
    for (int i = 0; attrs[i]; i += 2) {
        if (strcmp(attrs[i], name) == 0)
            return (char *) attrs[i + 1]; // !!
    }
    return NULL;
}

////////// utf-8 to latin-1 conversion functions //////////

char *str_dup(const char *p)
{
    size_t len = strlen(p);
    char *rv = (char *) malloc(len + 1);
    if (rv == NULL) {
	fprintf(stderr, "malloc failed!\n");
	exit(1);
    }
    strcpy(rv, p);
    return rv;
}

size_t get_length(unsigned char c)
{
    if (c < 0x80) return 1;
    else if (!(c & 0x20)) return 2;
    else if (!(c & 0x10)) return 3;
    else if (!(c & 0x08)) return 4;
    else if (!(c & 0x04)) return 5;
    else return 6;
}

int utf8toLatin1Char(char **p)
{
    size_t len = get_length(**p);
    if (len == 1)
        return **p;

    int res = (**p & (0xFF >> (len + 1))) << ((len - 1) * 6);

    for (--len; len; --len) {
        ++(*p);
        res |= ((**p) - 0x80) << ((len - 1) * 6);
    }
    return res;
}

// in-place conversion
char *utf8toLatin1Str(char *rv)
{
    int i = 0;
    for (char *p = rv; *p; ++p, ++i) {
        int value = utf8toLatin1Char(&p);
        if (value > 0xFF) {
            fprintf(stderr, "utf8toLatin1Char() failed!\n");
            exit(1);
        }
        rv[i] = value;
    }
    rv[i] = '\0';
    return rv;
}

// user must free the returned pointer!
char *getAttributeLatin1(const char **attrs, const char *name)
{
    char *value = getAttribute(attrs, name);
    if (value == NULL)
	return NULL;
    char *rv = str_dup(value);
    return utf8toLatin1Str(rv);
}


/////////////////

char *getAttributeError(const char **attrs, const char *name)
{
    char *retval = getAttribute(attrs, name);
    if (retval == NULL) {
        printf("ERROR: can't find attribute: %s", name);
        exit(0);
    }
    return retval;
}

char *getName(const char **attrs)
{
    return getAttribute(attrs, "name");
}

int isName(const char **attrs, const char *name)
{
    char *value = getAttribute(attrs, "name");
    return strcmp(value, name) == 0;
}

// returns the value of id-attribute
int getId(const char **attrs)
{
    char attr[] = "id";
    char *value = getAttribute(attrs, attr);
    return atoi2(value);
}

// returns the value of pos_x-attribute
int getX(const char **attrs)
{
    char attr[] = "pos_x";
    char *value = getAttribute(attrs, attr);
    return atoi2(value);
}

// returns the value of pos_y-attribute
int getY(const char **attrs)
{
    char attr[] = "pos_y";
    char *value = getAttribute(attrs, attr);
    return atoi2(value);
}

// returns the role of object or include_object
int getRole(const char **attrs)
{
    char attr[] = "role";
    char *value = getAttribute(attrs, attr);

    if (value == NULL || strcmp(value, "") == 0)
        return ROLE_NONE;

    if (strcmp(value, "active_mask") == 0)
        return ROLE_ACTIVE_MASK;

    if (strcmp(value, "font_attributes") == 0)
        return ROLE_FONT_ATTRIBUTES;

    if (strcmp(value, "soft_key_mask") == 0)
        return ROLE_SOFT_KEY_MASK;

    if (strcmp(value, "variable_reference") == 0)
        return ROLE_VARIABLE_REFERENCE;

    if (strcmp(value, "foreground_colour") == 0)
        return ROLE_FOREGROUND_COLOR;

    if (strcmp(value, "input_attributes") == 0)
        return ROLE_INPUT_ATTRIBUTES;

    if (strcmp(value, "line_attributes") == 0)
        return ROLE_LINE_ATTRIBUTES;

    if (strcmp(value, "fill_attributes") == 0)
        return ROLE_FILL_ATTRIBUTES;

    if (strcmp(value, "target_value_variable_reference") == 0)
        return ROLE_TARGET_VARIABLE_REFERENCE;

    if (strcmp(value, "fill_pattern") == 0)
        return ROLE_FILL_PATTERN;

    if (strcmp(value, "value") == 0)
        return ROLE_OBJECT_POINTER_VALUE;

    printf("ERROR: unknown role: %s\n", value);
    return 0;
}

int getColor(const char **attrs, const char *name)
{
    static const char *colors[] =
	{"black", "white", "green", "teal",
	 "maroon", "purple", "olive", "silver",
	 "grey", "blue", "lime", "cyan",
	 "red", "magenta", "yellow", "navy"};

    char *value = getAttribute(attrs, name);
    for (int i = 0; i < 16; i++)
        if (strcmp(colors[i], value) == 0)
            return i;

    return atoi2(value);
}

int reduceColor(int color, int colors)
{
    // these tables tell what is the nearest color
    unsigned char Colors256to16[] =
	{0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15,
	 0, 0, 15, 15, 9, 9, 0, 0, 15, 15, 9, 9, 2, 2, 3, 3,
	 3, 9, 2, 2, 3, 3, 3, 3,  2, 2, 3, 3, 3, 11, 10, 10,
	 10, 3, 11, 11, 0, 0, 15, 15, 9, 9, 0, 0, 15, 15, 9, 9,
	 2, 2, 3, 3, 3, 9, 2, 2, 3, 3, 3, 3, 2, 2, 3, 3,
	 3, 11, 10, 10, 10, 3, 11, 11, 4, 4, 5, 5, 5, 9, 4, 4,
	 5, 5, 5, 9, 6, 6, 8, 8, 8, 8, 6, 6, 8, 8, 8, 8,
	 6, 6, 8, 8, 7, 7, 10, 10, 8, 8, 7, 11, 4, 4, 5, 5,
	 5, 5, 4, 4, 5, 5, 5, 5, 6, 6, 8, 8, 8, 8, 6, 6,
	 8, 8, 8, 8, 6, 6, 8, 8, 7, 7, 6, 6, 8, 8, 7, 1,
	 4, 4, 5, 5, 5, 13, 4, 4, 5, 5, 5, 13, 6, 6, 8, 8,
	 7, 7, 6, 6, 8, 8, 7, 7, 6, 6, 7, 7, 7, 7, 14, 14,
	 7, 7, 7, 1, 12, 12, 12, 5, 13, 13, 12, 12, 12, 5, 13, 13,
	 12, 12, 8, 8, 7, 13, 6, 6, 8, 8, 7, 1, 14, 14, 7, 7,
	 7, 1, 14, 14, 14, 1, 1, 1};

    unsigned char Colors256to2[] =
	{0, 1, 0, 0, 0, 0, 0, 1, 1, 0, 0, 1, 0, 1, 1, 0,
	 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 1, 0, 0,
	 0, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 1, 0, 0, 0, 1,
	 1, 1, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0,
	 0, 0, 0, 1, 0, 0, 0, 0, 1, 1, 0, 0, 0, 1, 1, 1,
	 0, 0, 1, 1, 1, 1, 0, 1, 1, 1, 1, 1, 0, 0, 0, 0,
	 0, 1, 0, 0, 0, 0, 1, 1, 0, 0, 0, 1, 1, 1, 0, 0,
	 1, 1, 1, 1, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
	 0, 0, 0, 0, 1, 1, 0, 0, 0, 1, 1, 1, 0, 0, 1, 1,
	 1, 1, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
	 1, 1, 1, 1, 0, 0, 0, 1, 1, 1, 0, 0, 1, 1, 1, 1,
	 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
	 1, 1, 1, 1, 1, 1, 1, 1};

    if (colors == 256)
        return color;
    else if (colors == 16)
        return Colors256to16[color];
    else
        return Colors256to2[color];
}

int getReducedColor(const char **attrs, const char *name, int colors) {
    return reduceColor(getColor(attrs, name), colors);
}

int getBackgroundColor(const char **attrs, int colors) {
    return getReducedColor(attrs, "background_colour", colors);
}

int getBorderColor(const char **attrs, int colors) {
    return getReducedColor(attrs, "border_colour", colors);
}

int getFontColor(const char **attrs, int colors) {
    return getReducedColor(attrs, "font_colour", colors);
}

int getLineColor(const char **attrs, int colors) {
    return getReducedColor(attrs, "line_colour", colors);
}

int getNeedleColor(const char **attrs, int colors) {
    return getReducedColor(attrs, "needle_colour", colors);
}

int getArcAndTickColor(const char **attrs, int colors) {
    return getReducedColor(attrs, "arc_and_tick_colour", colors);
}

int getColorColor(const char **attrs, int colors) {
    return getReducedColor(attrs, "colour", colors);
}

int getTargetLineColor(const char **attrs, int colors) {
    return getReducedColor(attrs, "target_line_colour", colors);
}

//FIXME what should this return???
int getTransparencyColor(const char **attrs) {
    return getColor(attrs, "transparency_colour");
}

int getFillColor(const char **attrs, int colors) {
    return getReducedColor(attrs, "fill_colour", colors);
}

int getBoolean(const char **attrs, const char *name) {
    static const char *trues[] = {"yes", "true", "on", "show", "enable", "1"};
    char *value = getAttribute(attrs, name);
    for (int i = 0; i < 6; i++)
        if (strcmp(trues[i], value) == 0)
            return 1;

    return 0;
}

// returns the value of selectable-attribute (0 or 1)
int isSelectable(const char **attrs) {
    return getBoolean(attrs, "selectable");
}

int isHidden(const char **attrs) {
    return getBoolean(attrs, "hidden");
}

int isLatchable(const char **attrs) {
    return getBoolean(attrs, "latchable");
}

int isEnabled(const char **attrs) {
    return getBoolean(attrs, "enabled");
}

// returns the priority (0,1 or 2)
int getPriority(const char **attrs) {
    static const char *priorities[] = {"high", "medium", "low"};
    char *value = getAttribute(attrs, "priority");
    for (int i = 0; i < 3; i++)
        if (strcmp(priorities[i], value) == 0)
            return i;

    return atoi2(value);
}

// returns the acoustic signal (0,1,2 or 3)
int getAcousticSignal(const char **attrs) {
    static const char *priorities[] = {"high", "medium", "low", "none"};
    char *value = getAttribute(attrs, "acoustic_signal");
    for (int i = 0; i < 4; i++)
        if (strcmp(priorities[i], value) == 0)
            return i;

    return atoi2(value);
}

// returns the horizontal justification (0,1 or 2)
int getHorizontalJustification(const char **attrs) {
    static const char *priorities[] = {"left", "middle", "right"};
    char *value = getAttribute(attrs, "horizontal_justification");
    for (int i = 0; i < 3; i++)
        if (strcmp(priorities[i], value) == 0)
            return i;

    return atoi2(value);
}

int getEllipseType(const char **attrs) {
    static const char *types[] = {"closed", "open", "closedsegment", "closedsection"};
    char *value = getAttribute(attrs, "ellipse_type");
    for (int i=0; i<4; i++)
        if (strcmp(types[i], value) == 0)
            return i;

    return atoi2(value);
}

int getPolygonType(const char **attrs)
{
    static const char *types[] = {"convex", "nonconvex", "complex", "open"};
    char *value = getAttribute(attrs, "polygon_type");
    for (int i = 0; i < 4; i++)
        if (strcmp(types[i], value) == 0)
            return i;

    return atoi2(value);
}

int getFillType(const char **attrs) {
    static const char *types[] = {"nofill", "linecolour", "fillcolour", "pattern"};
    char *value = getAttribute(attrs, "fill_type");
    for (int i = 0; i < 4; i++)
        if (strcmp(types[i], value) == 0)
            return i;

    return atoi2(value);
}

int getFunctionType(const char **attrs) {
    static const char *types[] = {"boolean", "analog"};
    char *value = getAttribute(attrs, "function_type");
    for (int i = 0; i < 2; i++)
        if (strcmp(types[i], value) == 0 )
            return i;

    return atoi2(value);
}

int getLineDirection(const char **attrs)
{
    static const char *bltr[] = {"bottomlefttotopright", "1"};
    char *value = getAttribute(attrs, "line_direction");
    for (int i = 0; i < 2; i++)
        if (strcmp(bltr[i], value) == 0)
            return 1;

    return 0;
}

int getWidth(const char **attrs) {
    return atoi2(getAttribute(attrs, "width") );
}

int getHeight(const char **attrs) {
    return atoi2(getAttribute(attrs, "height") );
}

int getActualWidth(const char **attrs) {
    return atoi2(getAttribute(attrs, "image_width") );
}

int getActualHeight(const char **attrs) {
    return atoi2(getAttribute(attrs, "image_height") );
}

int getKeyCode(const char **attrs) {
    return atoi2(getAttribute(attrs, "key_code") );
}

unsigned int getValue(const char **attrs) {
    // FIXME range
    return atoi2(getAttribute(attrs, "value") );
}

unsigned int getTargetValue(const char **attrs) {
    return atoi2(getAttribute(attrs, "target_value") );
}

unsigned int getMinValue(const char **attrs) {
    // FIXME range
    return atoi2(getAttribute(attrs, "min_value") );
}

unsigned int getMaxValue(const char **attrs) {
    // FIXME range
    return atoi2(getAttribute(attrs, "max_value") );
}

int getOffset(const char **attrs) {
    return atoi2(getAttribute(attrs, "offset") );
}

float getScale(const char **attrs) {
    char *str = getAttribute(attrs, "scale");

    // check for empty string, default to 1!
    if (str == NULL || str[0] == 0)
        return 1.0f;

    return atof(str);
}

int getNumberOfDecimals(const char **attrs) {
    return atoi2(getAttribute(attrs, "number_of_decimals") );
}

int getNumberOfTicks(const char **attrs) {
    return atoi2(getAttribute(attrs, "number_of_ticks") );
}

int getLength(const char **attrs) {
    return atoi2(getAttribute(attrs, "length") );
}

int getStartAngle(const char **attrs) {
    return atoi2(getAttribute(attrs, "start_angle") );
}

int getEndAngle(const char **attrs) {
    return atoi2(getAttribute(attrs, "end_angle") );
}

int getLineWidth(const char **attrs) {
    return atoi2(getAttribute(attrs, "line_width") );
}

int getBarGraphWidth(const char **attrs) {
    return atoi2(getAttribute(attrs, "bar_graph_width") );
}

int getInputID(const char **attrs) {
    return atoi2(getAttribute(attrs, "input_id") );
}

int getLineArt(const char **attrs) {
    int art = 0;
    char *value = getAttribute(attrs, "line_art");
    for (int i = 0; value[i]; i++) {
        art *= 2;
        if (value[i] == '1')
            art++;
    }
    return art;
}

// only one supported
int getFontType(const char **attrs) {
    if (strstr(getAttribute(attrs, "font_type"), "latin1") != NULL)
        return 0;

    return 0;
}

int getValidationType(const char **attrs) {
    if (strstr(getAttribute(attrs, "validation_type"), "invalidcharacters") != NULL)
        return 1;

    return 0;
}

// returns the value-attribute as string, that is given length.
// given string must be freed!
char *getValueString(const char **attrs, int length)
{
    char *string = (char *) malloc(sizeof(char) * length);
    char *value = getAttributeLatin1(attrs, "value");
    int valueLength = strlen(value);

    if (length <= valueLength) {
        memcpy(string, value, length);
    }
    else {
        memcpy(string, value, valueLength);
        memset(string + valueLength, ' ', length - valueLength);
    }
    free(value);
    return string;
}

char *getValidatioinString(const char **attrs, int length)
{
    char *string = (char *) malloc(sizeof(char) * length);
    char *value = getAttributeLatin1(attrs, "validation_string");
    int valueLength = strlen(value);

    if (length <= valueLength) {
        memcpy(string, value, length);
    }
    else {
        memcpy(string, value, valueLength);
        memset(string + valueLength, ' ', length - valueLength);
    }
    free(value);
    return string;
}

int getOptions(const char **attrs, const char **names, int bits, const char *name)
{
    char *options = getAttribute(attrs, name);
    int retVal = 0;
    int base = 1;
    for (int i = 0; i < bits; i++) {
        if (strstr(options, names[i]) != NULL)
            retVal += base;
        base *= 2;
    }
    return retVal;
}

int getInputStringOptions(const char **attrs)
{
    char *options = getAttribute(attrs, "options");
    int retVal = 0;
    if (strstr(options, "transparent") != NULL)
        retVal += 1;
    if (strstr(options, "autowrap") != NULL)
        retVal += 2;

    return retVal;
}

int getInputNumberOptions(const char **attrs)
{
    char *options = getAttribute(attrs, "options");
    int retVal = 0;
    if (strstr(options, "transparent") != NULL)
        retVal += 1;
    if (strstr(options, "leadingzeros") != NULL)
        retVal += 2;
    if (strstr(options, "blankzero") != NULL)
        retVal += 4;

    return retVal;
}

int getMeterOptions(const char **attrs)
{
    char *options = getAttribute(attrs, "options");
    int retVal = 0;
    if (strstr(options, "arc") != NULL)
        retVal += 1;
    if (strstr(options, "border") != NULL)
        retVal += 2;
    if (strstr(options, "ticks") != NULL)
        retVal += 4;
    if (strstr(options, "clockwise") != NULL)
        retVal += 8;

    return retVal;
}

int getLinearBarGraphOptions(const char **attrs)
{
    static const char *names[] = {"border", "targetline", "ticks", "nofill", "horizontal" , "growpositive"};
    return getOptions(attrs, names, 6, "options");
}

int getArchedBarGraphOptions(const char **attrs)
{
    static const char *names[] = {"border", "targetline", "NOT_USED", "nofill", "clockwise"};
    return getOptions(attrs, names, 5, "options");
}

int getPictureGraphicOptions(const char **attrs)
{
    static const char *names[] = {"transparent", "flashing"};  // No rle!
    return getOptions(attrs, names, 2, "options");
}

int getFontStyle(const char **attrs)
{
    const char *style = getAttribute(attrs, "font_style");
    int retVal = 0;
    if (strstr(style, "bold") != NULL)
        retVal += 1;
    if (strstr(style, "crossed") != NULL)
        retVal += 2;
    if (strstr(style, "underlined") != NULL)
        retVal += 4;
    if (strstr(style, "italic") != NULL)
        retVal += 8;
    if (strstr(style, "inverted") != NULL)
        retVal += 16;
    if (strstr(style, "flashinginverted") != NULL)
        retVal += 32;
    if (strstr(style, "flashinghidden") != NULL)
        retVal += 64;

    return retVal;
}

int getLineSuppression(const char **attrs)
{
    char *suppression = getAttribute(attrs, "line_suppression");
    int retVal = 0;
    if (strstr(suppression, "top") != NULL)
        retVal += 1;
    if (strstr(suppression, "right") != NULL)
        retVal += 2;
    if (strstr(suppression, "bottom") != NULL)
        retVal += 4;
    if (strstr(suppression, "left") != NULL)
        retVal += 8;

    return retVal + atoi2(suppression);
}

int getNumberFormat(const char **attrs)
{
    if (strcmp(getAttribute(attrs, "format"), "exponential") == 0)
        return 1;

    return 0;
}

int getFontSize2(const char **attrs, const char *name)
{
    static const char *fonts[] = {"6x8", "8x8", "8x12",
				  "12x16", "16x16", "16x24",
				  "24x32", "32x32", "32x48",
				  "48x64", "64x64", "64x96",
				  "96x128", "128x128", "128x192"};

    char *value = getAttribute(attrs, name);
    for (int i = 0; i < 15; i++)
        if (strcmp(fonts[i], value) == 0 )
            return i;

    return atoi2(value);
}

int getFontSize(const char **attrs){
    return getFontSize2(attrs, "font_size");
}

int getBlockFontWidth(const char **attrs, int fontMultiplier) {
    int widths[] = {6,  8,   8,
                    12, 16,  16,
                    24, 32,  32,
                    48, 64,  64,
                    96, 128, 128};

    if (getAttribute(attrs, "block_font_size") == NULL)
        return 0;

    int font = getFontSize2(attrs, "block_font_size") * fontMultiplier;
    if (font < 0 || font > 14)
        return 0;
    return widths[font];
}

int getBlockFontHeight(const char **attrs, int fontMultiplier) {
    int heights[] = {8,   8,   12,
                     16,  16,  24,
                     32,  32,  48,
                     64,  64,  96,
                     128, 128, 192};

    if (getAttribute(attrs, "block_font_size") == NULL)
        return 0;

    int font = getFontSize2(attrs, "block_font_size") * fontMultiplier;
    if (font < 0 || font > 14)
        return 0;
    return heights[font];
}

int getBlockCol(const char **attrs)
{
    return atoi2(getAttribute(attrs, "block_col"));
}

int getBlockRow(const char **attrs)
{
    return atoi2(getAttribute(attrs, "block_row"));
}

int getDimension(const char **attrs)
{
    return atoi2(getAttribute(attrs, "dimension"));
}

int getSkWidth(const char **attrs)
{
    return atoi2(getAttribute(attrs, "sk_width"));
}

int getSkHeight(const char **attrs)
{
    return atoi2(getAttribute(attrs, "sk_height"));
}

// for commands
int getObjectId(const char **attrs)
{
    return atoi2(getAttribute(attrs, "object_id"));
}

int getHideShow(const char **attrs)
{
    if (strcmp(getAttribute(attrs, "hide_show"), "show") == 0)
        return 1;

    return 0;
}

int getEnableDisable(const char **attrs)
{
    if (strcmp(getAttribute(attrs, "enable_disable"), "enable") == 0)
        return 1;

    return 0;
}

int getRepetitions(const char **attrs)
{
    return atoi2(getAttribute(attrs, "number_of_repetitions"));
}

int getFrequency(const char **attrs)
{
    return atoi2(getAttribute(attrs, "frequency"));
}

int getOnTime(const char **attrs)
{
    return atoi2(getAttribute(attrs, "on_time"));
}

int getOffTime(const char **attrs)
{
    return atoi2(getAttribute(attrs, "off_time"));
}

int getParentId(const char **attrs)
{
    return atoi2(getAttribute(attrs, "parent_id"));
}

int getChildId(const char **attrs)
{
    return atoi2(getAttribute(attrs, "child_id"));
}

int getVolume(const char **attrs)
{
    return atoi2(getAttribute(attrs, "volume"));
}

int getDx(const char **attrs)
{
    return atoi2(getAttribute(attrs, "d_pos_x"));
}

int getDy(const char **attrs)
{
    return atoi2(getAttribute(attrs, "d_pos_y"));
}

int getMaskType(const char **attrs)
{
    char *value = getAttribute(attrs, "mask_type");
    if (strcmp(value, "alarmmask") == 0 ||
        strcmp(value, "2") == 0)
        return 2;

    return 1;
}

int getAID(const char **attrs)
{
    return atoi2(getAttribute(attrs, "attribute_id"));
}

int getFillPatternID(const char **attrs)
{
    return atoi2(getAttribute(attrs, "fill_pattern"));
}

int getPosX(const char **attrs)
{
    return atoi2(getAttribute(attrs, "c_pos_x"));
}

int getPosY(const char **attrs)
{
    return atoi2(getAttribute(attrs, "c_pos_y"));
}

int getListIndex(const char **attrs)
{
    return atoi2(getAttribute(attrs, "list_index"));
}

float getMultiplier(const char **attrs, float old, float mask, float designator)
{
    float multip = old;
    char *value = getAttribute(attrs, "use");

    if (value != NULL) {
        if (strcmp(value, "mask") == 0)
            multip = mask;
        else if (strcmp(value, "designator") == 0)
            multip = designator;
        else if (strcmp(value, "both") == 0)  // if 'both' use smaller
            multip = (mask < designator) ? mask : designator;
    }
    return multip;
}
