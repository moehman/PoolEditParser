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

// parsedef.h

#ifndef PARSERDEF
#define PARSERDEF

#pragma pack(1)

// this is the start of every object in pool
typedef struct
{
    unsigned short objectId;
    unsigned char  type;
} ObjectHeader;

// when a object has xy-coordinates inside of another object, this
// reference is used
typedef struct
{
    unsigned short objectId;
    unsigned short x;
    unsigned short y;
} ObjectReference;

// struct is used referring a macro
typedef struct
{
    unsigned char eventId;
    unsigned char macroId;
} MacroReference;

// language code
typedef struct
{
    unsigned char char1;
    unsigned char char2;
} LanguageCode;

// a polygon point
typedef struct
{
    unsigned short x;
    unsigned short y;
} Point;


/**************** ISOBUS objects ****************/

typedef struct
{
    unsigned short objectId;
    unsigned char  type;

    unsigned char  backgroundColor;
    unsigned char  selectable;
    unsigned short activeMask;
    unsigned char  objects;
    unsigned char  macros;
    unsigned char  languageCodes;
} WorkingSet;

typedef struct
{
    unsigned short objectId;
    unsigned char  type;

    unsigned char  backgroundColor;
    unsigned short softKeyMask;
    unsigned char  objects;
    unsigned char  macros;
} DataMask;

typedef struct
{
    unsigned short objectId;
    unsigned char  type;

    unsigned char  backgroundColor;
    unsigned short softKeyMask;
    unsigned char  priority;
    unsigned char  acousticSignal;
    unsigned char  objects;
    unsigned char  macros;
} AlarmMask;

typedef struct
{
    unsigned short objectId;
    unsigned char  type;

    unsigned short width;
    unsigned short height;
    unsigned char  hidden;
    unsigned char  objects;
    unsigned char  macros;
} Container;

typedef struct
{
    unsigned short objectId;
    unsigned char  type;

    unsigned char  backgroundColor;
    unsigned char  objects;
    unsigned char  macros;
    // objects here, only 2 bytes long (no x,y)
} SoftKeyMask;

typedef struct
{
    unsigned short objectId;
    unsigned char  type;

    unsigned char  backgroundColor;
    unsigned char  keyCode;
    unsigned char  objects;
    unsigned char  macros;
} Key;

typedef struct
{
    unsigned short objectId;
    unsigned char  type;

    unsigned short width;
    unsigned short height;
    unsigned char  backgroundColor;
    unsigned char  borderColor;
    unsigned char  keyCode;
    unsigned char  latchable;
    unsigned char  objects;
    unsigned char  macros;
} Button;

typedef struct
{
    unsigned short objectId;
    unsigned char  type;

    unsigned char  backgroundColor;
    unsigned short width;
    unsigned short foregroundColor;  // id!
    unsigned short variableReference;
    unsigned char  value;
    unsigned char  enabled;
    unsigned char  macros;
} InputBoolean;

typedef struct
{
    unsigned short objectId;
    unsigned char  type;

    unsigned short width;
    unsigned short height;
    unsigned char  backgroundColor;
    unsigned short fontAttributes;
    unsigned short inputAttributes;
    unsigned char  options;
    unsigned short variableReference;
    unsigned char  horizontalJustification;
    unsigned char  length;

    unsigned char value[]; // length of value is the 'length'

    // 2 bytes!
    //unsigned char  enabled;
    //unsigned char  macros;
} InputString;

// returns a pointer to the number of macros
unsigned char* inputStringMacros(InputString *ptr)
{
    return ptr->value + ptr->length * sizeof(unsigned char) + 1;
}

// returns a pointer to enabled-attribute
unsigned char* inputStringEnabled(InputString *ptr)
{
    return ptr->value + ptr->length * sizeof(unsigned char);
}

typedef struct
{
    unsigned short objectId;
    unsigned char  type;

    unsigned short width;
    unsigned short height;
    unsigned char  backgroundColor;
    unsigned short fontAttributes;
    unsigned char  options;
    unsigned short variableReference;
    unsigned int   value;
    unsigned int   minValue;
    unsigned int   maxValue;
    int   offset;
    float scale;
    unsigned char  numberOfDecimals;
    unsigned char  format;
    unsigned char  horizontalJustification;
    unsigned char  enabled;
    unsigned char  macros;
} InputNumber;

typedef struct
{
    unsigned short objectId;
    unsigned char  type;

    unsigned short width;
    unsigned short height;
    unsigned short variableReference;
    unsigned char  value;
    unsigned char  numberOfListItems;
    unsigned char  enabled;
    unsigned char  macros;
    // objects here, only 2 bytes long (no x,y)
} InputList;


typedef struct
{
    unsigned short objectId;
    unsigned char  type;

    unsigned short width;
    unsigned short height;
    unsigned char  backgroundColor;
    unsigned short fontAttributes;
    unsigned char  options;
    unsigned short variableReference;
    unsigned char  horizontalJustification;
    unsigned short  length;
    unsigned char value[];
    // the value (string) comes here! length of value is the 'length'

    // unsigned char  macros;
} OutputString;

// returns a pointer to the number of macros
unsigned char *outputStringMacros(OutputString *ptr)
{
    return ptr->value + ptr->length * sizeof(unsigned char);
}


typedef struct
{
    unsigned short objectId;
    unsigned char  type;

    unsigned short width;
    unsigned short height;
    unsigned char  backgroundColor;
    unsigned short fontAttributes;
    unsigned char  options;
    unsigned short variableReference;
    unsigned int   value;
    int   offset;
    float scale;
    unsigned char  numberOfDecimals;
    unsigned char  format;
    unsigned char  horizontalJustification;
    unsigned char  macros;
} OutputNumber;

typedef struct
{
    unsigned short objectId;
    unsigned char  type;

    unsigned short lineAttributes;
    unsigned short width;
    unsigned short height;
    unsigned char  lineDirection;
    unsigned char  macros;
} Line;

typedef struct
{
    unsigned short objectId;
    unsigned char  type;

    unsigned short lineAttributes;
    unsigned short width;
    unsigned short height;
    unsigned char  lineSupression;
    unsigned short fillAttributes;
    unsigned char  macros;
} Rectangle;

typedef struct
{
    unsigned short objectId;
    unsigned char  type;

    unsigned short lineAttributes;
    unsigned short width;
    unsigned short height;
    unsigned char  ellipseType;
    unsigned char  startAngle;
    unsigned char  endAngle;
    unsigned short fillAttributes;
    unsigned char  macros;
} Ellipse;

typedef struct
{
    unsigned short objectId;
    unsigned char  type;

    unsigned short width;
    unsigned short height;
    unsigned short lineAttributes;
    unsigned short fillAttributes;
    unsigned char  polygonType;
    unsigned char  numberOfPoints;
    unsigned char  macros;
    // points (x,y)
    // macros
} Polygon;

typedef struct
{
    unsigned short objectId;
    unsigned char  type;

    unsigned short width;
    unsigned char  needleColor;
    unsigned char  borderColor;
    unsigned char  arcAndTickColor;
    unsigned char  options;
    unsigned char  numberOfTicks;
    unsigned char  startAngle;
    unsigned char  endAngle;
    unsigned short minValue;
    unsigned short maxValue;
    unsigned short variableReference;
    unsigned short value;
    unsigned char  macros;
} Meter;

typedef struct
{
    unsigned short objectId;
    unsigned char  type;

    unsigned short width;
    unsigned short height;
    unsigned char  color;
    unsigned char  targetLineColor;
    unsigned char  options;
    unsigned char  numberOfTicks;
    unsigned short minValue;
    unsigned short maxValue;
    unsigned short variableReference;
    unsigned short value;
    unsigned short targetValueVariableReference;
    unsigned short targetValue;
    unsigned char  macros;
} LinearBarGraph;

typedef struct
{
    unsigned short objectId;
    unsigned char  type;

    unsigned short width;
    unsigned short height;
    unsigned char  color;
    unsigned char  targetLineColor;
    unsigned char  options;
    unsigned char  startAngle;
    unsigned char  endAngle;
    unsigned short barGraphWidth;
    unsigned short minValue;
    unsigned short maxValue;
    unsigned short variableReference;
    unsigned short value;
    unsigned short targetValueVariableReference;
    unsigned short targetValue;
    unsigned char  macros;
} ArchedBarGraph;

typedef struct
{
    unsigned short objectId;
    unsigned char  type;

    unsigned short width;
    unsigned short actualWidth;
    unsigned short actualHeight;
    unsigned char  format;
    unsigned char  options;
    unsigned char  transparencyColor;
    unsigned int   rawDataLength;
    unsigned char  macros;
    // raw-data
    // macros
} PictureGraphic;

typedef struct
{
    unsigned short objectId;
    unsigned char  type;

    unsigned int value;
} NumberVariable;

typedef struct
{
    unsigned short objectId;
    unsigned char  type;

    unsigned short length;
    // data
} StringVariable;

typedef struct
{
    unsigned short objectId;
    unsigned char  type;

    unsigned char  fontColor;
    unsigned char  fontSize;
    unsigned char  fontType;
    unsigned char  fontStyle;
    unsigned char  macros;
} FontAttributes;

typedef struct
{
    unsigned short objectId;
    unsigned char  type;

    unsigned char  lineColor;
    unsigned char  lineWidth;
    unsigned short lineArt;
    unsigned char  macros;
} LineAttributes;

typedef struct
{
    unsigned short objectId;
    unsigned char  type;

    unsigned char  fillType;
    unsigned char  fillColor;
    unsigned short fillPattern;
    unsigned char  macros;
} FillAttributes;

typedef struct
{
    unsigned short objectId;
    unsigned char  type;

    unsigned char  validationType;
    unsigned char  length;
    unsigned char  validationString[];

    // unsigned char  macros;
} InputAttributes;

// returns a pointer to the number of macros
unsigned char *inputAttributesMacros(InputAttributes *ptr)
{
    return ptr->validationString + ptr->length * sizeof(unsigned char);
}

typedef struct
{
    unsigned short objectId;
    unsigned char  type;

    unsigned short value;
} ObjectPointer;


typedef struct
{
    unsigned short objectId;
    unsigned char  type;

    unsigned short numberOfBytes;
    unsigned char  commands[];
} Macro;

// auxiliary control

typedef struct
{
    unsigned short objectId;
    unsigned char  type;

    unsigned char  backgroundColor;
    unsigned char  functionType;

    unsigned char  objects;
} AuxiliaryFunction;

typedef struct
{
    unsigned short objectId;
    unsigned char  type;

    unsigned char  backgroundColor;
    unsigned char  functionType;
    unsigned char  inputId;

    unsigned char  objects;
} AuxiliaryInput;

/*********** COMMANDS ***************/

typedef struct
{
    unsigned char  VTFunction;

    unsigned short objectId;
    unsigned char  show;
    unsigned int   padding;
} HideShowObject;

typedef struct
{
    unsigned char  VTFunction;

    unsigned short objectId;
    unsigned char  enable;
    unsigned int   padding;
} EnableDisableObject;

typedef struct
{
    unsigned char  VTFunction;

    unsigned short objectId;
    unsigned int   padding1;
    unsigned char  padding2;
} SelectInputObject;

typedef struct
{
    unsigned char  VTFunction;

    unsigned char  repetitions;
    unsigned short frequency;
    unsigned short onTime;
    unsigned short offTime;
} ControlAudioDevice;

typedef struct
{
    unsigned char  VTFunction;

    unsigned char  volume;
    unsigned int   padding1;
    unsigned short padding2;
} SetAudioVolume;

typedef struct
{
    unsigned char  VTFunction;

    unsigned short parentId;
    unsigned short childId;
    unsigned char  dx;
    unsigned char  dy;
    unsigned char  padding;
} ChangeChildLocation;

typedef struct
{
    unsigned char  VTFunction;

    unsigned short objectId;
    unsigned short width;
    unsigned short height;
    unsigned char  padding;
} ChangeSize;

typedef struct
{
    unsigned char  VTFunction;

    unsigned short objectId;
    unsigned char  backgroundColor;
    unsigned int   padding;
} ChangeBackgroundColor;

typedef struct
{
    unsigned char  VTFunction;

    unsigned short objectId;
    unsigned char  padding;
    unsigned int   value;
} ChangeNumericValue;

typedef struct
{
    unsigned char  VTFunction;

    unsigned short objectId;
    unsigned short length;
    unsigned char  string[];
} ChangeStringValue;

typedef struct
{
    unsigned char  VTFunction;

    unsigned short objectId;
    unsigned short width;
    unsigned short height;
    unsigned char  lineDirection;
} ChangeEndPoint;

typedef struct
{
    unsigned char  VTFunction;

    unsigned short objectId;
    unsigned char  fontColor;
    unsigned char  fontSize;
    unsigned char  fontType;
    unsigned char  fontStyle;
    unsigned char  padding;
} ChangeFontAttributes;

typedef struct
{
    unsigned char  VTFunction;

    unsigned short objectId;
    unsigned char  lineColor;
    unsigned char  lineWidth;
    unsigned short lineArt;
    unsigned char  padding;
} ChangeLineAttributes;

typedef struct
{
    unsigned char  VTFunction;

    unsigned short objectId;
    unsigned char  fillType;
    unsigned char  fillColor;
    unsigned short fillPattern;
    unsigned char  padding;
} ChangeFillAttributes;


typedef struct
{
    unsigned char  VTFunction;

    unsigned short parentId;
    unsigned short childId;
    unsigned short padding1;
    unsigned char  padding2;
} ChangeActiveMask;

typedef struct
{
    unsigned char  VTFunction;

    unsigned char  maskType;
    unsigned short parentId;
    unsigned short childId;
    unsigned short padding1;
} ChangeSoftKeyMask;

typedef struct
{
    unsigned char  VTFunction;

    unsigned short objectId;
    unsigned char  AID;
    unsigned int   value;
} ChangeAttribute;

typedef struct
{
    unsigned char  VTFunction;

    unsigned short objectId;
    unsigned char  priority;
    unsigned int   padding;
} ChangePriority;

typedef struct
{
    unsigned char  VTFunction;

    unsigned short parentId;
    unsigned short childId;
    unsigned short x;
    unsigned short y;
} ChangeChildPosition;

typedef struct
{
    unsigned char  VTFunction;

    unsigned short parentId;
    unsigned char  listIndex;
    unsigned short childId;
    unsigned short padding;
} ChangeListItem;

#pragma pack()

// an array of all XML elements that have a corresponding ISOBUS
// object

// names are ordered by object type
const char *xmlNames[] =
    {"workingset", "datamask", "alarmmask", "container", "softkeymask", "key",
     "button",  "inputboolean",  "inputstring",  "inputnumber",
     "inputlist", "outputstring", "outputnumber",  "line", "rectangle",
     "ellipse", "polygon", "meter",  "linearbargraph", "archedbargraph",
     "picturegraphic", "numbervariable", "stringvariable", "fontattributes",
     "lineattributes",  "fillattributes", "inputattributes",
     "objectpointer", "macro", "auxiliaryfunction", "auxiliaryinput"};


// commands-array has xml-names of all supported commands
const char *commands[] =
    {"command_hide_show_object", "command_enable_disable_object", "command_select_input_object",
     "command_control_audio_device", "command_set_audio_volume", "command_change_child_location",
     "command_change_size", "command_change_background_colour", "command_change_numeric_value",
     "command_change_string_value", "command_change_end_point", "command_change_font_attributes",
     "command_change_line_attributes", "command_change_fill_attributes", "command_change_active_mask",
     "command_change_soft_key_mask", "command_change_attribute", "command_change_priority",
     "command_change_child_position", "command_change_list_item",
     NULL};

// commandFunction-array has the function numbers of the commands in
// the commands-array
int commandFunction[] =
    {160, 161, 162,
     163, 164, 165,
     166, 167, 168,
     179, 169, 170,
     171, 172, 173,
     174, 175, 176,
     180, 177};

// XML names of all events, ordered by event number
const char *events[] =
    {"on_activate", "on_deactivate", "on_show", "on_hide", "on_enable", "on_disable",
     "on_change_active_mask", "on_change_soft_key_mask", "on_change_attribute",
     "on_change_background_colour", "on_change_font_attributes", "on_change_line_attributes",
     "on_change_fill_attributes", "on_change_child_location", "on_change_size", "on_change_value",
     "on_change_priority", "on_change_end_point", "on_input_field_selection",
     "on_input_field_deselection", "on_esc", "on_entry_of_value", "on_entry_of_new_value",
     "on_key_press", "on_key_release" , "on_change_child_position"};

#endif /* #ifndef PARSERDEF */
