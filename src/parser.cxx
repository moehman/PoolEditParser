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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "xml.h"
#include "parser.h"
#include "../include/expat.h"
#include "parserdef.h"

// the maxium depth of xml-tree
#define MAX_STACK 256
void *objectStack[MAX_STACK];
int objectsInStack = 0;

// info about working sets and softkey dimensions
int vtDimension;
int vtSkWidth;
int vtSkHeight;

pool_xform_t xform;
/*
  int dm_dx, dm_dy;
  int sk_dx, sk_dy;

  float dm_mult, sk_mult;   // how much should all coordinates be multiplied
*/
float multiplier;           // current multiplier

// number of colors (2, 16 or 256) in the VT
int vtColors;

// pictureData that is in Base64
int dataReading = 0;   // tells if now reading data
char *dataText = NULL;
int dataLength =0;

void (*readyFunct)(char *data, int length);
void (*startFunct)(void *data, char *el, const char ** attr);
void (*endFunct)(void *data, char *el);

pool_xform_t *get_pool_xform()
{
    return &xform;
}

// returns the type of object
int getObjectType(void *object)
{
    return ((ObjectHeader *) object)->type;
}

// returns the object id of object
int getObjectId(void *object)
{
    return ((ObjectHeader *) object)->objectId;
}

// returns the number (function) of a command or -1 if no such command
// is supported
int getCommandFunction(const char *name)
{
    if (name == NULL)
        return -1;

    int i;
    for (i = 0; (commands[i] != NULL) && (strcmp(commands[i], name) != 0); i++)
        ;

    if (commands[i] == NULL)
        return -1;

    return commandFunction[i];
}

// returns the type of the given event, or -1 if no such event
int getEventId(const char *name)
{
    if (name == NULL)
        return -1;

    for (int i = 0; i < 26; i++)
        if (strcmp(events[i], name) == 0)
            return i + 1;

    return -1;
}

#define OBJECT_MACRO_INDEX(type, object) (sizeof(type) + ((type *) object)->objects * sizeof(ObjectReference) + ((type *) object)->macros * sizeof(MacroReference))
#define MACRO_INDEX(type, object) (sizeof(type) + ((type *) object)->macros * sizeof(MacroReference))

// this function returns the real size of a object given to it
// use this instead of sizeof()-function.
// objects must be initialized before calling this!
int getRealSize(void *object) {

    switch (getObjectType(object)) {
    case 0:
        {
            WorkingSet *workingSet = (WorkingSet *) object;
            return sizeof(WorkingSet) + workingSet->objects * sizeof(ObjectReference)
                + workingSet->macros * sizeof(MacroReference)
                + workingSet->languageCodes * sizeof(LanguageCode);
        }
    case 1:
        return OBJECT_MACRO_INDEX(DataMask, object);

    case 2:
        return OBJECT_MACRO_INDEX(AlarmMask, object);

    case 3:
        return OBJECT_MACRO_INDEX(Container, object);

    case 4:
        // special case!
        {
            SoftKeyMask *softKeyMask = (SoftKeyMask *) object;
            return sizeof(SoftKeyMask) + softKeyMask->objects * sizeof(unsigned short)
                + softKeyMask->macros * sizeof(MacroReference);
        }

    case 5:
        return OBJECT_MACRO_INDEX(Key, object);

    case 6:
        return OBJECT_MACRO_INDEX(Button, object);

    case 7:
        return MACRO_INDEX(InputBoolean, object);

    case 8:
        {
            InputString *inputString = (InputString *) object;
            return sizeof(InputString) + inputString->length * sizeof(unsigned char)
                + *inputStringMacros(inputString) * sizeof(MacroReference) + 2;
        }

    case 9:
        return MACRO_INDEX(InputNumber, object);

    case 10:
        {
            InputList *inputList = (InputList *) object;
            return sizeof(InputList) + inputList->numberOfListItems * sizeof(unsigned short)
                + inputList->macros * sizeof(MacroReference);
        }

    case 11:
        {
            OutputString *outputString = (OutputString *) object;
            return sizeof(OutputString) + outputString->length * sizeof(unsigned char)
                + *outputStringMacros(outputString) * sizeof(MacroReference) +1;
        }

    case 12:
        return MACRO_INDEX(OutputNumber, object);

    case 13:
        return MACRO_INDEX(Line, object);

    case 14:
        return MACRO_INDEX(Rectangle, object);

    case 15:
        return MACRO_INDEX(Ellipse, object);

    case 16:
        {
            Polygon *polygon = (Polygon *) object;
            return sizeof(Polygon) + polygon->macros * sizeof(MacroReference)
                + polygon->numberOfPoints * sizeof(Point);
        }
    case 17:
        return MACRO_INDEX(Meter, object);

    case 18:
        return MACRO_INDEX(LinearBarGraph, object);

    case 19:
        return MACRO_INDEX(ArchedBarGraph, object);

    case 20:
        {
            PictureGraphic *pictureGraphic = (PictureGraphic *) object;
            return sizeof(PictureGraphic) + pictureGraphic->macros * sizeof(MacroReference)
                + pictureGraphic->rawDataLength;
        }

    case 21:
        return sizeof(NumberVariable);

    case 22:
        {
            StringVariable *stringVariable = (StringVariable *) object;
            return sizeof(StringVariable) + stringVariable->length;
        }

    case 23:
        return MACRO_INDEX(FontAttributes, object);

    case 24:
        return MACRO_INDEX(LineAttributes, object);

    case 25:
        return MACRO_INDEX(FillAttributes, object);

    case 26:
        {
            InputAttributes *inputAttributes = (InputAttributes *) object;
            return sizeof(InputAttributes) + inputAttributes->length
                + *inputAttributesMacros(inputAttributes) * sizeof(PictureGraphic)+1;
        }

    case 27:
        return sizeof(ObjectPointer);

    case 28:
        {
            Macro *macro = (Macro *) object;
            return sizeof(Macro) + macro->numberOfBytes;
        }
    case 29:
        {
            AuxiliaryFunction *auxiliaryFunction = (AuxiliaryFunction *) object;
            return sizeof(AuxiliaryFunction) + auxiliaryFunction->objects * sizeof(ObjectReference);
        }
    case 30:
        {
            AuxiliaryInput *auxiliaryInput = (AuxiliaryInput *) object;
            return sizeof(AuxiliaryInput) + auxiliaryInput->objects * sizeof(ObjectReference);
        }

    }
    return -1;
}

#define OBJECT_INDEX(type, object) (sizeof(type) + ((type *) object)->objects * sizeof(ObjectReference))
#define OBJECTS_PLUS_PLUS(type, object) ( ((type *) object)->objects++ )

// function adds a reference to an object
// if role is none, then reference is a real include_object otherwise
// it is just an attribute
void addObjectReference(void **object, ObjectReference objectReference, int role)
{
    int startIndex = 0;
    int orginalSize = getRealSize(*object);
    int error = 0;
    int objectType = getObjectType(*object);

    if (role == ROLE_NONE) {
        switch (objectType) {
        case 0:
            startIndex = OBJECT_INDEX(WorkingSet, *object);
            OBJECTS_PLUS_PLUS(WorkingSet, *object);
            break;

        case 1:
            startIndex = OBJECT_INDEX(DataMask, *object);
            OBJECTS_PLUS_PLUS(DataMask, *object);
            objectReference.x += xform.dm_dx;
            objectReference.y += xform.dm_dy;
            break;

        case 2:
            startIndex = OBJECT_INDEX(AlarmMask, *object);
            OBJECTS_PLUS_PLUS(AlarmMask, *object);
            objectReference.x += xform.dm_dx;
            objectReference.y += xform.dm_dy;
            break;

        case 3:
            startIndex = OBJECT_INDEX(Container, *object);
            OBJECTS_PLUS_PLUS(Container, *object);
            break;

        case 4:
            {
                *object = realloc(*object, getRealSize(*object) + 2);
                SoftKeyMask *softKeyMask = (SoftKeyMask *) *object;
                unsigned char *ptr = ((unsigned char *) softKeyMask) + sizeof(SoftKeyMask)
                    + softKeyMask->objects * sizeof(unsigned short);
                softKeyMask->objects++;

                // move macros
                memmove(ptr, ptr + 2, softKeyMask->macros * sizeof(MacroReference));

                *((unsigned short *) ptr) = objectReference.objectId;
            }
            return;

        case 5:
            startIndex = OBJECT_INDEX(Key, *object);
            OBJECTS_PLUS_PLUS(Key, *object);
            objectReference.x += xform.sk_dx;
            objectReference.y += xform.sk_dy;
            break;

        case 6:
            startIndex = OBJECT_INDEX(Button, *object);
            OBJECTS_PLUS_PLUS(Button, *object);
            // the inside of a button does not scale like other objects
            // so a padding is added to it
            objectReference.x += (int) (4 * multiplier) - 4;
            objectReference.y += (int) (4 * multiplier) - 4;
            break;

        case 10:
            {
                *object = realloc(*object, getRealSize(*object) + 2);
                InputList *inputList = (InputList*) *object;
                unsigned char *ptr = ((unsigned char *) inputList) + sizeof(InputList)
                    + inputList->numberOfListItems * sizeof(unsigned short);
                inputList->numberOfListItems++;

                // move macros
                memmove(ptr, ptr + 2, inputList->macros * sizeof(MacroReference));

                *((unsigned short *) ptr) = objectReference.objectId;
            }
            return;

        case 29:
            startIndex = OBJECT_INDEX(AuxiliaryFunction, *object);
            OBJECTS_PLUS_PLUS(AuxiliaryFunction, *object);
            break;

        case 30:
            startIndex = OBJECT_INDEX(AuxiliaryInput, *object);
            OBJECTS_PLUS_PLUS(AuxiliaryInput, *object);
            break;

        default:
            error = 1;
            break;

        }

        if (error) {
            printf("ERROR. Object %i (Id=%i) can't have objects contained!\n",
                   ((ObjectHeader *) *object)->type, ((ObjectHeader *) *object)->objectId);
            return;
        }

        *object = realloc(*object, getRealSize(*object));

        // move events and language codes
        memmove( ( ((char *) *object) + startIndex + sizeof( ObjectReference )), (((char *) *object) + startIndex), orginalSize - startIndex);

        memmove( ( ((char *) *object) + startIndex), &objectReference, sizeof( ObjectReference ) );
    }
    if (role == ROLE_ACTIVE_MASK) {
        if (objectType == 0)
            ((WorkingSet *) *object)->activeMask = objectReference.objectId;
        else
            error = 1;
    }
    if (role == ROLE_FONT_ATTRIBUTES) {
        if (objectType == 8)
            ((InputString *) *object)->fontAttributes = objectReference.objectId;

        else if (objectType == 9)
            ((InputNumber *) *object)->fontAttributes = objectReference.objectId;

        else if (objectType == 11)
            ((OutputString *) *object)->fontAttributes = objectReference.objectId;

        else if (objectType == 12)
            ((OutputNumber *) *object)->fontAttributes = objectReference.objectId;

        else
            error = 1;
    }
    if (role == ROLE_SOFT_KEY_MASK) {
        if (objectType == 1)
            ((DataMask *) *object)->softKeyMask = objectReference.objectId;

        else if (objectType == 2)
            ((AlarmMask *) *object)->softKeyMask = objectReference.objectId;

        else
            error = 1;
    }
    if (role == ROLE_VARIABLE_REFERENCE) {
        if (objectType == 7)
            ((InputBoolean *) *object)->variableReference = objectReference.objectId;

        else if (objectType == 8)
            ((InputString *) *object)->variableReference = objectReference.objectId;

        else if (objectType == 9)
            ((InputNumber *) *object)->variableReference = objectReference.objectId;

        else if (objectType == 10)
            ((InputList *) *object)->variableReference = objectReference.objectId;

        else if (objectType == 11)
            ((OutputString *) *object)->variableReference = objectReference.objectId;

        else if (objectType == 12)
            ((OutputNumber *) *object)->variableReference = objectReference.objectId;

        else if (objectType == 17)
            ((Meter *) *object)->variableReference = objectReference.objectId;

        else if (objectType == 18)
            ((LinearBarGraph *) *object)->variableReference = objectReference.objectId;

        else if (objectType == 19)
            ((ArchedBarGraph *) *object)->variableReference = objectReference.objectId;

        else
            error = 1;
    }
    if (role == ROLE_TARGET_VARIABLE_REFERENCE) {
        if (objectType == 18)
            ((LinearBarGraph *) *object)->targetValueVariableReference = objectReference.objectId;
        else if (objectType == 19)
            ((ArchedBarGraph *) *object)->targetValueVariableReference = objectReference.objectId;
        else
            error = 1;
    }
    if (role == ROLE_FOREGROUND_COLOR) {
        if (objectType == 7)
            ((InputBoolean *) *object)->foregroundColor = objectReference.objectId;

        else
            error = 1;
    }
    if (role == ROLE_INPUT_ATTRIBUTES) {
        if (objectType == 8)
            ((InputString *) *object)->inputAttributes = objectReference.objectId;
        else
            error = 1;
    }
    if (role == ROLE_LINE_ATTRIBUTES) {
        if (objectType == 13)
            ((Line *) *object)->lineAttributes = objectReference.objectId;
        else if (objectType == 14)
            ((Rectangle *) *object)->lineAttributes = objectReference.objectId;
        else if (objectType == 15)
            ((Ellipse *) *object)->lineAttributes = objectReference.objectId;
        else if (objectType == 16)
            ((Polygon *) *object)->lineAttributes = objectReference.objectId;
        else
            error = 1;
    }
    if (role == ROLE_FILL_ATTRIBUTES) {
        if (objectType == 14)
            ((Rectangle *) *object)->fillAttributes = objectReference.objectId;
        else if (objectType == 15)
            ((Ellipse *) *object)->fillAttributes = objectReference.objectId;
        else if (objectType == 16)
            ((Polygon *) *object)->fillAttributes = objectReference.objectId;
        else
            error = 1;
    }
    if (role == ROLE_FILL_PATTERN) {
        if (objectType == 25)
            ((FillAttributes *) *object)->fillPattern = objectReference.objectId;
        else
            error = 1;
    }
    if (role == ROLE_OBJECT_POINTER_VALUE) {
        if (objectType == 27)
            ((ObjectPointer *) *object)->value = objectReference.objectId;
        else
            error = 1;
    }
    if (error)
        printf("ERROR. Object %i (Id=%i) can't have objects of role %i contained!\n",
               ((ObjectHeader *) *object)->type, ((ObjectHeader *) *object)->objectId, role);

}

// adds a point to a polygon-object
void addPoint(void **object, Point point)
{
    if (getObjectType(*object) != 16) {
        printf("ERROR. Object %i can't have points contained!\n",
               ((ObjectHeader *) *object)->type);
        return;
    }

    Polygon *polygon = (Polygon *) *object;
    int orginalSize = getRealSize(*object);
    int startIndex =  sizeof(Polygon) + polygon->numberOfPoints * sizeof(Point);
    polygon->numberOfPoints++;

    *object = realloc(*object, getRealSize(*object));

    // move events
    // FIXME, might not work!
    memmove(( ((char *) *object) + startIndex + sizeof(Point)), (((char *) *object) + startIndex), orginalSize - startIndex);

    // copy point
    memmove(( ((char *) *object) + startIndex), &point, sizeof(Point) );
}

// converts one character from base64 to number
// A-Z = 0-25, a-z = 26-51, 0-9 = 52-16, + = 62 and / = 63
int base64toIndex(const char base64) {
    if (base64 >= 'A' && base64 <= 'Z')
        return base64 - 'A';
    else if (base64 >= 'a' && base64 <= 'z')
        return base64 - 'a' + 26;
    else if (base64 >= '0' && base64 <= '9')
        return base64 - '0' + 52;
    else if (base64 == '+')
        return 62;
    else if (base64 == '/')
        return 63;

    printf("ERROR: wrong character %c in Base64", base64);
    return -1;
}

// calculates the real data length when base64 lenght is given
int getDataLength(int base64Length) {
    int length = base64Length / 4 * 3;
    if (base64Length % 4 == 2)
        length += 1;
    if (base64Length % 4 == 3)
        length += 2;
    return length;
}

// converts a string (or an array of characters), from base64 to
// binary data
unsigned char *convertFromBase64(const char *base64, int dataLength) {

    unsigned char *datas = (unsigned char *) malloc(sizeof(char) * dataLength);

    for (int i = 0; i < dataLength; i++) {
        int indexStart = base64toIndex(base64[i * 4 / 3]);
        int indexEnd = base64toIndex(base64[i * 4 / 3 + 1]);
        if ((i % 3) == 0)
            datas[i] = (indexStart << 2) + (indexEnd >> 4);
        if ((i % 3) == 1)
            datas[i] = ((indexStart & 15) << 4) + (indexEnd >> 2);
        if ((i % 3) == 2)
            datas[i] = ((indexStart & 3) << 6) + indexEnd;
    }
    return datas;
}

// Adds image data to image object
// Image will have 2,16 or 256 colors according to VT's color depth
void addPictureData(void **object)
{
    if (getObjectType(*object) != 20) {
        printf("ERROR. Object %i can't have pictureData!\n",
               ((ObjectHeader *) *object)->type);
        return;
    }
    PictureGraphic *picture = (PictureGraphic *) *object;
    int size = picture->actualWidth * picture->actualHeight;
    if (size != getDataLength(dataLength)) {
        printf("ERROR. Data length miss match (size = %i, size2 = %i)\n",
               size, getDataLength(dataLength));
        return;
    }
    unsigned char *data = convertFromBase64(dataText, size);
    unsigned char *reducedData = data;

    // reduce colors and rearrenge data
    if (vtColors == 16) {
        int dataWidth = (picture->actualWidth + 1) / 2;
        reducedData = (unsigned char *) calloc(dataWidth * picture->actualHeight, sizeof(char));

        for (int y = 0; y < picture->actualHeight; y++)
            for (int x = 0; x < picture->actualWidth; x += 2)
                for (int i = 0; (i < 2) && ((x + i) < picture->actualWidth); i++)
                    reducedData[y * dataWidth + x / 2] += (reduceColor(data[y * picture->actualWidth + x + i], vtColors) << (1 - i) * 4);

        size = dataWidth * picture->actualHeight;
        free(data);
        data = reducedData;
    }
    else if (vtColors == 2) {
        int dataWidth = (picture->actualWidth + 7) / 8;
        reducedData = (unsigned char *) calloc(dataWidth * picture->actualHeight, sizeof(char));

        for (int y = 0; y < picture->actualHeight; y++)
            for (int x = 0; x < picture->actualWidth; x += 8)
                for (int i = 0; (i < 8) && ((x + i) < picture->actualWidth); i++)
                    reducedData[y * dataWidth + x / 8] += (reduceColor(data[y * picture->actualWidth + x + i], vtColors) << (7 - i));

        size = dataWidth * picture->actualHeight;
        free(data);
        data = reducedData;
    }

//  int originalSize = getRealSize(*object);
//  int startIndex = sizeof(PictureGraphic) + picture->rawDataLength;
    picture->rawDataLength = size;
    *object = realloc(*object, getRealSize(*object));

    // move events ????
    // memcpy( ( ((char *) *object) + size + sizeof( PictureGraphic )), (((char *) *object) + sizeof( PictureGraphic )), orginalSize - sizeof( PictureGraphic ));

    // copy data
    memmove(( ((char *) *object) + sizeof(PictureGraphic)), data, size);

    free(data);
}

#define INIT_OBJECT(oType, object) oType *object = (oType *) calloc(sizeof(oType), 1); object->objectId = id; object->type = type

// creates a new object of given type, using the xml-attributes
void *createObject(int type, const char **attr){
    int id = getId(attr);

    multiplier = getMultiplier(attr, multiplier, xform.dm_mult, xform.sk_mult);

    switch (type) {
    case 0: // WorkingSet
        {
            INIT_OBJECT(WorkingSet, workingset);
            workingset->backgroundColor = getBackgroundColor(attr, vtColors);
            workingset->selectable = isSelectable(attr);
            return workingset;
        }
    case 1: // DataMask
        {
            INIT_OBJECT(DataMask, dataMask);
            dataMask->backgroundColor = getBackgroundColor(attr, vtColors);
            dataMask->softKeyMask = 65535;
            return dataMask;
        }
    case 2: // AlarmMask
        {
            INIT_OBJECT(AlarmMask, alarmMask);
            alarmMask->backgroundColor = getBackgroundColor(attr, vtColors);
            alarmMask->softKeyMask = 65535;
            alarmMask->priority = getPriority(attr);
            alarmMask->acousticSignal = getAcousticSignal(attr);
            return alarmMask;
        }
    case 3: // Container
        {
            INIT_OBJECT(Container, container);
            container->width = (int) (multiplier * getWidth(attr));
            container->height = (int) (multiplier * getHeight(attr));
            container->hidden = isHidden(attr);
            return container;
        }
    case 4: // SoftKeyMask
        {
            INIT_OBJECT(SoftKeyMask, softKeyMask);
            softKeyMask->backgroundColor = getBackgroundColor(attr, vtColors);
            return softKeyMask;
        }
    case 5: // Key
        {
            INIT_OBJECT(Key, key);
            key->backgroundColor = getBackgroundColor(attr, vtColors);
            key->keyCode = getKeyCode(attr);
            return key;
        }
    case 6: // Button
        {
            INIT_OBJECT(Button, button);
            button->width = (int) (multiplier * getWidth(attr));
            button->height = (int) (multiplier * getHeight(attr));
            button->backgroundColor = getBackgroundColor(attr, vtColors);
            button->borderColor = getBorderColor(attr, vtColors);
            button->keyCode = getKeyCode(attr);
            button->latchable = isLatchable(attr);
            return button;
        }
    case 7: // InputBoolean
        {
            INIT_OBJECT(InputBoolean, inputBoolean);
            inputBoolean->backgroundColor = getBackgroundColor(attr, vtColors);
            inputBoolean->width = (int) (multiplier * getWidth(attr));
            inputBoolean->foregroundColor = 65535;
            inputBoolean->variableReference = 65535;
            inputBoolean->value = getValue(attr);
            inputBoolean->enabled = isEnabled(attr);
            return inputBoolean;
        }
    case 8: // InputString
        {
            // length of string + 2 must be counted
            InputString *inputString = (InputString *) calloc(sizeof(InputString) + getLength(attr) + 2, 1);
            inputString->objectId = id;
            inputString->type = type;

            inputString->width = (int) (multiplier * getWidth(attr));
            inputString->height = (int) (multiplier * getHeight(attr));
            inputString->backgroundColor = getBackgroundColor(attr, vtColors);
            inputString->fontAttributes = 65535;
            inputString->inputAttributes = 65535;
            inputString->options = getInputStringOptions(attr);
            inputString->variableReference = 65535;
            inputString->horizontalJustification = getHorizontalJustification(attr);
            inputString->length = getLength(attr);

            // copy string
            char *string = getValueString(attr, getLength(attr));
            memcpy( inputString->value, string, getLength(attr) );
            free( string );

            *inputStringEnabled(inputString) = isEnabled(attr);
            return inputString;
        }
    case 9:  // InputNumber
        {
            INIT_OBJECT(InputNumber, inputNumber);
            inputNumber->width = (int) (multiplier * getWidth(attr));
            inputNumber->height = (int) (multiplier * getHeight(attr));
            inputNumber->backgroundColor = getBackgroundColor(attr, vtColors);
            inputNumber->fontAttributes = 65535;
            inputNumber->options = getInputNumberOptions(attr);
            inputNumber->variableReference = 65535;
            inputNumber->value = getValue(attr);
            inputNumber->minValue = getMinValue(attr);
            inputNumber->maxValue = getMaxValue(attr);
            inputNumber->offset = getOffset(attr);
            inputNumber->scale = getScale(attr);
            inputNumber->numberOfDecimals = getNumberOfDecimals(attr);
            inputNumber->format = getNumberFormat(attr);
            inputNumber->horizontalJustification = getHorizontalJustification(attr);
            inputNumber->enabled = isEnabled(attr);
            return inputNumber;
        }
    case 10: // InputList
        {
            INIT_OBJECT(InputList, inputList);
            inputList->width = (int) (multiplier * getWidth(attr));
            inputList->height = (int) (multiplier * getHeight(attr));
            inputList->variableReference = 65535;
            inputList->value = getValue(attr);
            inputList->enabled = isEnabled(attr);
            return inputList;
        }
    case 11: // OutputString
        {
            OutputString *outputString = (OutputString *) calloc(sizeof(OutputString) + getLength(attr) + 1, 1);
            outputString->objectId = id;
            outputString->type = type;

            outputString->width = (int) (multiplier * getWidth(attr));
            outputString->height = (int) (multiplier * getHeight(attr));
            outputString->backgroundColor = getBackgroundColor(attr, vtColors);
            outputString->fontAttributes = 65535;
            outputString->options = getInputStringOptions(attr);
            outputString->variableReference = 65535;
            outputString->horizontalJustification = getHorizontalJustification(attr);
            outputString->length = getLength(attr);

            // copy string
            char *string = getValueString(attr, getLength(attr));
            memcpy(outputString->value, string, getLength(attr));
            free(string);

            return outputString;
        }
    case 12: // OutputNumber
        {
            INIT_OBJECT(OutputNumber, outputNumber);
            outputNumber->width = (int) (multiplier * getWidth(attr));
            outputNumber->height = (int) (multiplier * getHeight(attr));
            outputNumber->backgroundColor = getBackgroundColor(attr, vtColors);
            outputNumber->fontAttributes = 65535;
            outputNumber->options = getInputNumberOptions(attr);
            outputNumber->variableReference = 65535;
            outputNumber->value = getValue(attr);
            outputNumber->offset = getOffset(attr);
            outputNumber->scale = getScale(attr);
            outputNumber->numberOfDecimals = getNumberOfDecimals(attr);
            outputNumber->format = getNumberFormat(attr);
            outputNumber->horizontalJustification = getHorizontalJustification(attr);
            return outputNumber;
        }
    case 13: // Line
        {
            INIT_OBJECT(Line, line);
            line->lineAttributes = 65535;
            line->width = (int) (multiplier * getWidth(attr));
            line->height = (int) (multiplier * getHeight(attr));
            line->lineDirection = getLineDirection(attr);
            return line;
        }
    case 14: // Rectangle
        {
            INIT_OBJECT(Rectangle, rectangle);
            rectangle->lineAttributes = 65535;
            rectangle->width = (int) (multiplier * getWidth(attr));
            rectangle->height = (int) (multiplier * getHeight(attr));
            rectangle->lineSupression = getLineSuppression(attr);
            rectangle->fillAttributes = 65535;
            return rectangle;
        }
    case 15: // Ellipse
        {
            INIT_OBJECT(Ellipse, ellipse);
            ellipse->lineAttributes = 65535;
            ellipse->width = (int) (multiplier * getWidth(attr));
            ellipse->height = (int) (multiplier * getHeight(attr));
            ellipse->ellipseType = getEllipseType(attr);
            ellipse->startAngle = getStartAngle(attr);
            ellipse->endAngle = getEndAngle(attr);
            ellipse->fillAttributes = 65535;
            return ellipse;
        }
    case 16: // Polygon
        {
            INIT_OBJECT(Polygon, polygon);
            polygon->width = (int) (multiplier * getWidth(attr));
            polygon->height = (int) (multiplier * getHeight(attr));
            polygon->lineAttributes = 65535;
            polygon->fillAttributes = 65535;
            polygon->polygonType = getPolygonType(attr);
            return polygon;
        }
    case 17: // Meter
        {
            INIT_OBJECT(Meter, meter);
            meter->width = (int) (multiplier * getWidth(attr));
            meter->needleColor = getNeedleColor(attr, vtColors);
            meter->borderColor = getBorderColor(attr, vtColors);
            meter->arcAndTickColor = getArcAndTickColor(attr, vtColors);
            meter->options = getMeterOptions(attr);
            meter->numberOfTicks = getNumberOfTicks(attr);
            meter->startAngle = getStartAngle(attr);
            meter->endAngle = getEndAngle(attr);
            meter->minValue = getMinValue(attr);
            meter->maxValue = getMaxValue(attr);
            meter->variableReference = 65535;
            meter->value = getValue(attr);
            return meter;
        }
    case 18: // LinearBarGraph
        {
            INIT_OBJECT(LinearBarGraph, linearBarGraph);
            linearBarGraph->width = (int) (multiplier * getWidth(attr));
            linearBarGraph->height = (int) (multiplier * getHeight(attr));
            linearBarGraph->color = getColorColor(attr, vtColors);
            linearBarGraph->targetLineColor = getTargetLineColor(attr, vtColors);
            linearBarGraph->options = getLinearBarGraphOptions(attr);
            linearBarGraph->numberOfTicks = getNumberOfTicks(attr);
            linearBarGraph->minValue = getMinValue(attr);
            linearBarGraph->maxValue = getMaxValue(attr);
            linearBarGraph->variableReference = 65535;
            linearBarGraph->value = getValue(attr);
            linearBarGraph->targetValueVariableReference = 65535;
            linearBarGraph->targetValue = getTargetValue(attr);
            return linearBarGraph;
        }
    case 19: // ArchedBarGraph
        {
            INIT_OBJECT(ArchedBarGraph, archedBarGraph);
            archedBarGraph->width = (int) (multiplier * getWidth(attr));
            archedBarGraph->height = (int) (multiplier * getHeight(attr));
            archedBarGraph->color = getColorColor(attr, vtColors);
            archedBarGraph->targetLineColor = getTargetLineColor(attr, vtColors);
            archedBarGraph->options = getArchedBarGraphOptions(attr);
            archedBarGraph->startAngle = getStartAngle(attr);
            archedBarGraph->endAngle = getEndAngle(attr);
            archedBarGraph->barGraphWidth = getBarGraphWidth(attr);
            archedBarGraph->minValue = getMinValue(attr);
            archedBarGraph->maxValue = getMaxValue(attr);
            archedBarGraph->variableReference = 65535;
            archedBarGraph->value = getValue(attr);
            archedBarGraph->targetValueVariableReference = 65535;
            archedBarGraph->targetValue = getTargetValue(attr);
            return archedBarGraph;
        }
    case 20: // PictureGraphic
        {
            INIT_OBJECT(PictureGraphic, pictureGraphic);
            pictureGraphic->width = (int) (multiplier * getWidth(attr));
            //pictureGraphic->actualWidth = getActualWidth(attr);
            //pictureGraphic->actualHeight = getActualHeight(attr);
            pictureGraphic->format = 0;     // = 2 colors
            if (vtColors == 16)
                pictureGraphic->format = 1; // = 16 colors
            else if (vtColors == 256)
                pictureGraphic->format = 2; // = 256 colors

            pictureGraphic->options = getPictureGraphicOptions(attr);
            pictureGraphic->transparencyColor = getTransparencyColor(attr);

            return pictureGraphic;
        }
    case 21: // NumberVariable
        {
            INIT_OBJECT(NumberVariable, numberVariable);
            numberVariable->value = getValue(attr);
            return numberVariable;
        }
    case 22: // StringVariable
        {
            StringVariable *stringVariable = (StringVariable *) calloc(sizeof(StringVariable) + getLength(attr), 1);
            stringVariable->objectId = id;
            stringVariable->type = type;
            stringVariable->length = getLength(attr);

            // copy string
            char *string = getValueString(attr, getLength(attr));
            memcpy(( ((char *) stringVariable) + 5), string, getLength(attr));
            free(string);
            return stringVariable;
        }
    case 23: // FontAttributes
        {
            INIT_OBJECT(FontAttributes, fontAttributes);
            fontAttributes->fontColor = getFontColor(attr, vtColors);
            fontAttributes->fontSize = getFontSize(attr) + ((int) (multiplier) - 1) * 3;
            fontAttributes->fontType = getFontType(attr);
            fontAttributes->fontStyle = getFontStyle(attr);

            return fontAttributes;
        }
    case 24: // LineAttributes
        {
            INIT_OBJECT(LineAttributes, lineAttributes);
            lineAttributes->lineColor = getLineColor(attr, vtColors);
            lineAttributes->lineWidth = (int) (multiplier * getLineWidth(attr));  // ???
            lineAttributes->lineArt = getLineArt(attr);
            return lineAttributes;
        }
    case 25: // FillAttributes
        {
            INIT_OBJECT(FillAttributes, fillAttributes);
            fillAttributes->fillType = getFillType(attr);
            fillAttributes->fillColor = getFillColor(attr, vtColors);
            return fillAttributes;
        }
    case 26: // InputAttributes
        {
            InputAttributes *inputAttributes = (InputAttributes *) calloc(sizeof(InputAttributes) + getLength(attr) + 1, 1);
            inputAttributes->objectId = id;
            inputAttributes->type = type;

            inputAttributes->validationType = getValidationType(attr);
            inputAttributes->length = getLength(attr);

            // copy string
            char *string = getValidatioinString(attr, getLength(attr));
            memcpy(inputAttributes->validationString, string, getLength(attr));
            free(string);
            return inputAttributes;
        }
    case 27: // ObjectPointer
        {
            INIT_OBJECT(ObjectPointer, objectPointer);
            objectPointer->value = 65535;
            return objectPointer;
        }
    case 28: // Macro
        {
            INIT_OBJECT(Macro, macro);
            return macro;
        }
    case 29:
        {
            INIT_OBJECT(AuxiliaryFunction, auxiliaryFunction);
            auxiliaryFunction->backgroundColor = getBackgroundColor(attr, vtColors);
            auxiliaryFunction->functionType = getFunctionType(attr);
            return auxiliaryFunction;
        }
    case 30:
        {
            INIT_OBJECT(AuxiliaryInput, auxiliaryInput);
            auxiliaryInput->backgroundColor = getBackgroundColor(attr, vtColors);
            auxiliaryInput->functionType = getFunctionType(attr);
            auxiliaryInput->inputId = getInputID(attr);
            return auxiliaryInput;
        }
    default:
        printf("ERROR. Object %i not implemented yet!\n", type);
        return NULL;
    }
}

int getCommandSize(const void *object)
{
    int type = *((unsigned char *) object);
    switch (type) {
    case 179:
        return sizeof(ChangeStringValue) + ((ChangeStringValue *) object)->length;
    case 180:
        return sizeof(ChangeChildPosition);
    default:
	return 8;
    }
}

void *createCommand(int command, const char **attr)
{
    multiplier = getMultiplier(attr, multiplier, xform.dm_mult, xform.sk_mult);

    // this works for all other commands, except change string value
    void *object = calloc(8, 1);

    switch (command) {
    case 160:
        {
            HideShowObject *hideShowObject = (HideShowObject *) object;
            hideShowObject->VTFunction = 160;
            hideShowObject->objectId = getObjectId(attr);
            hideShowObject->show = getHideShow(attr);
            hideShowObject->padding = 0xFFFFFFFF;
            return hideShowObject;
        }
    case 161:
        {
            EnableDisableObject *enableDisableObject = (EnableDisableObject *) object;
            enableDisableObject->VTFunction = 161;
            enableDisableObject->objectId = getObjectId(attr);
            enableDisableObject->enable = getEnableDisable(attr);
            enableDisableObject->padding = 0xFFFFFFFF;
            return enableDisableObject;
        }
    case 162:
        {
            SelectInputObject *selectInputObject  = (SelectInputObject *) object;
            selectInputObject->VTFunction = 162;
            selectInputObject->objectId = getObjectId(attr);
            selectInputObject->padding1 = 0xFFFFFFFF;
            selectInputObject->padding2 = 0xFF;
            return selectInputObject;
        }
    case 163:
        {
            ControlAudioDevice *controlAudioDevice = (ControlAudioDevice *) object;
            controlAudioDevice->VTFunction = 163;
            controlAudioDevice->repetitions = getRepetitions(attr);
            controlAudioDevice->frequency = getFrequency(attr);
            controlAudioDevice->onTime = getOnTime(attr);
            controlAudioDevice->offTime = getOffTime(attr);
            return controlAudioDevice;
        }
    case 164:
        {
            SetAudioVolume *setAudioVolume = (SetAudioVolume *) object;
            setAudioVolume->VTFunction = 164;
            setAudioVolume->volume = getVolume(attr);
            setAudioVolume->padding1 = 0xFFFFFFFF;
            setAudioVolume->padding2 = 0xFFFF;
            return  setAudioVolume;
        }
    case 165:
        {
            ChangeChildLocation *changeChildLocation = (ChangeChildLocation *) object;
            changeChildLocation->VTFunction = 165;
            changeChildLocation->parentId = getParentId(attr);
            changeChildLocation->childId = getChildId(attr);
            changeChildLocation->dx = ((int) multiplier * getDx(attr)) +127; // this can be dangerous!
            changeChildLocation->dy = ((int) multiplier * getDy(attr)) +127;
            changeChildLocation->padding = 0xFF;
            return changeChildLocation;
        }
    case 166:
        {
            ChangeSize *changeSize = (ChangeSize *) object;
            changeSize->VTFunction = 166;
            changeSize->objectId = getObjectId(attr);
            changeSize->width = ((int) multiplier * getWidth(attr));
            changeSize->height = ((int) multiplier * getHeight(attr));
            changeSize->padding = 0xFF;
            return changeSize;
        }
    case 167:
        {
            ChangeBackgroundColor *changeBackgroundColor = (ChangeBackgroundColor *) object;
            changeBackgroundColor->VTFunction = 167;
            changeBackgroundColor->objectId = getObjectId(attr);
            changeBackgroundColor->backgroundColor = getBackgroundColor(attr, vtColors);
            changeBackgroundColor->padding = 0xFFFFFFFF;
            return changeBackgroundColor;
        }
    case 168:
        {
            ChangeNumericValue *changeNumericValue = (ChangeNumericValue *) object;
            changeNumericValue->VTFunction = 168;
            changeNumericValue->objectId = getObjectId(attr);
            changeNumericValue->padding = 0xFF;
            changeNumericValue->value = getValue(attr);
            return changeNumericValue;
        }
    case 179:
        {
            int length = getLength(attr);
            object = realloc(object, sizeof(ChangeStringValue) + length);
            ChangeStringValue *changeStringValue = (ChangeStringValue *) object;
            changeStringValue->VTFunction = 179;
            changeStringValue->objectId = getObjectId(attr);
            changeStringValue->length = length;

            char *string = getValueString(attr, length);
            memmove(changeStringValue->string, string, length);
            free(string);
            return changeStringValue;
        }
    case 169:
        {
            ChangeEndPoint *changeEndPoint = (ChangeEndPoint *) object;
            changeEndPoint->VTFunction = 169;
            changeEndPoint->objectId = getObjectId(attr);
            changeEndPoint->width = ((int) multiplier * getWidth(attr));
            changeEndPoint->height = ((int) multiplier * getHeight(attr));
            changeEndPoint->lineDirection = getLineDirection(attr);
            return changeEndPoint;
        }
    case 170:
        {
            ChangeFontAttributes *changeFontAttributes = (ChangeFontAttributes *) object;
            changeFontAttributes->VTFunction = 170;
            changeFontAttributes->objectId = getObjectId(attr);
            changeFontAttributes->fontColor = getFontColor(attr, vtColors);
            changeFontAttributes->fontSize = getFontSize(attr) + ((int) multiplier - 1) * 3;
            changeFontAttributes->fontType = getFontType(attr);
            changeFontAttributes->fontStyle = getFontStyle(attr);
            changeFontAttributes->padding = 0xFF;
            return changeFontAttributes;
        }
    case 171:
        {
            ChangeLineAttributes *changeLineAttributes = (ChangeLineAttributes *) object;
            changeLineAttributes->VTFunction = 171;
            changeLineAttributes->objectId = getObjectId(attr);
            changeLineAttributes->lineColor = getLineColor(attr, vtColors);
            changeLineAttributes->lineWidth = (int) multiplier * getLineWidth(attr);
            changeLineAttributes->lineArt = getLineArt(attr);
            changeLineAttributes->padding = 0xFF;
            return changeLineAttributes;
        }
    case 172:
        {
            ChangeFillAttributes *changeFillAttributes = (ChangeFillAttributes *) object;
            changeFillAttributes->VTFunction = 172;
            changeFillAttributes->objectId = getObjectId(attr);
            changeFillAttributes->fillType = getFillType(attr);
            changeFillAttributes->fillColor = getFillColor(attr, vtColors);
            changeFillAttributes->fillPattern = getFillPatternID(attr);
            changeFillAttributes->padding = 0xFF;
            return changeFillAttributes;
        }
    case 173:
        {
            ChangeActiveMask *changeActiveMask = (ChangeActiveMask *) object;
            changeActiveMask->VTFunction = 173;
            changeActiveMask->parentId = getParentId(attr);
            changeActiveMask->childId = getChildId(attr);
            changeActiveMask->padding1 = 0xFFFF;
            changeActiveMask->padding2 = 0xFF;
            return changeActiveMask;
        }
    case 174:
        {
            ChangeSoftKeyMask *changeSoftKeyMask = (ChangeSoftKeyMask *) object;
            changeSoftKeyMask->VTFunction = 174;
            changeSoftKeyMask->maskType = getMaskType(attr);
            changeSoftKeyMask->parentId = getParentId(attr);
            changeSoftKeyMask->childId = getChildId(attr);
            changeSoftKeyMask->padding1 = 0xFFFF;
            return changeSoftKeyMask;
        }
    case 175:
        {
            ChangeAttribute *changeAttribute = (ChangeAttribute *) object;
            changeAttribute->VTFunction = 175;
            changeAttribute->objectId = getObjectId(attr);
            changeAttribute->AID = getAID(attr);
            changeAttribute->value = getValue(attr);
            return changeAttribute;
        }
    case 176:
        {
            ChangePriority *changePriority = (ChangePriority *) object;
            changePriority->VTFunction = 176;
            changePriority->objectId = getObjectId(attr);
            changePriority->priority = getPriority(attr);
            changePriority->padding = 0xFFFFFFFF;
            return changePriority;
        }
    case 177:
        {
            ChangeListItem *changeListItem = (ChangeListItem *) object;
            changeListItem->VTFunction = 177;
            changeListItem->parentId = getParentId(attr);
            changeListItem->listIndex = getListIndex(attr);
            changeListItem->childId = getChildId(attr);
            changeListItem->padding = 0xFFFF;
            return changeListItem;
        }
    case 180:
        {
            object = realloc(object, sizeof(ChangeChildPosition));
            ChangeChildPosition *changeChildPosition = (ChangeChildPosition *) object;
            changeChildPosition->VTFunction = 180;
            changeChildPosition->parentId = getParentId(attr);
            changeChildPosition->childId = getChildId(attr);
            changeChildPosition->x = ((int) multiplier * getPosX(attr));
            changeChildPosition->y = ((int) multiplier * getPosY(attr));
            return changeChildPosition;
        }
    default:
        printf("ERROR: command %i, not implemented!\n", command);
    }
    return NULL;
}

// adds a command to a macro
void addCommand(void **object, void *command)
{
    int objectType = getObjectType(*object);
    if (objectType != 28) {
        printf("ERROR: object of type %i , can't have commands!\n", objectType);
        return;
    }

    *object = realloc(*object, getRealSize(*object) + getCommandSize(command));
    Macro *macro = (Macro *) *object;
    memmove(((char *) macro) + getRealSize( macro ), command, getCommandSize(command));
    macro->numberOfBytes += getCommandSize(command);
}

// adds a macro to a object
#define MACROS_PLUS_PLUS(type, object) ( ((type *) object)->macros++ )
void addEventReference(void **object, MacroReference macroRef)
{
    int objectType = getObjectType(*object);
    if (objectType < 0 || objectType == 21 || objectType == 22 || objectType > 26) {
        printf("ERROR: object of type %i , can't have macros!\n", objectType);
        return;
    }
    int orginalSize = getRealSize(*object);

    switch (objectType) {
    case 0:
        MACROS_PLUS_PLUS(WorkingSet, *object);
        break;
    case 1:
        MACROS_PLUS_PLUS(DataMask, *object);
        break;
    case 2:
        MACROS_PLUS_PLUS(AlarmMask, *object);
        break;
    case 3:
        MACROS_PLUS_PLUS(Container, *object);
        break;
    case 4:
        MACROS_PLUS_PLUS(SoftKeyMask, *object);
        break;
    case 5:
        MACROS_PLUS_PLUS(Key, *object);
        break;
    case 6:
        MACROS_PLUS_PLUS(Button, *object);
        break;
    case 7:
        MACROS_PLUS_PLUS(InputBoolean, *object);
        break;
    case 8:
        (*inputStringMacros((InputString *) *object))++;
        break;
    case 9:
        MACROS_PLUS_PLUS(InputNumber, *object);
        break;
    case 10:
        MACROS_PLUS_PLUS(InputList, *object);
        break;
    case 11:
        (*outputStringMacros((OutputString *) *object))++;
        break;
    case 12:
        MACROS_PLUS_PLUS(OutputNumber, *object);
        break;
    case 13:
        MACROS_PLUS_PLUS(Line, *object);
        break;
    case 14:
        MACROS_PLUS_PLUS(Rectangle, *object);
        break;
    case 15:
        MACROS_PLUS_PLUS(Ellipse, *object);
        break;
    case 16:
        MACROS_PLUS_PLUS(Polygon, *object);
        break;
    case 17:
        MACROS_PLUS_PLUS(Meter, *object);
        break;
    case 18:
        MACROS_PLUS_PLUS(LinearBarGraph, *object);
        break;
    case 19:
        MACROS_PLUS_PLUS(ArchedBarGraph, *object);
        break;
    case 20:
        MACROS_PLUS_PLUS(PictureGraphic, *object);
        break;

    case 23:
        MACROS_PLUS_PLUS(FontAttributes, *object);
        break;
    case 24:
        MACROS_PLUS_PLUS(LineAttributes, *object);
        break;
    case 25:
        MACROS_PLUS_PLUS(FillAttributes, *object);
        break;
    case 26:
        (*inputAttributesMacros((InputAttributes *) *object))++;
        break;
    default:
        printf("ERROR: object of type %i , can't have macros??\n", objectType);
        return;
    }
    *object = realloc(*object, getRealSize(*object));

    // copy reference
    memmove(((char *) *object) + orginalSize, &macroRef, sizeof(MacroReference));
}

// when object is read, main program is informed and memory released
void objectReady(void *object) {
    int size = getRealSize(object);
    readyFunct((char *) object, size);
    free(object);
}

float min(float a, float b) {
    if (a < b)
        return a;
    else
        return b;
}

// expat-parser calls this function when new xml-element is found
void start(void *data, const char *el, const char **attr) {

    // check if element is an ISOBUS object - if so, get the type
    int type = -1;
    for (int i = 0; i <= 30; i++)
        if (strcmp(xmlNames[i], el) == 0)
            type = i;

    int command = getCommandFunction(el);
    int eventId = getEventId(getAttribute(attr, "role"));

    // create new object (if it is a real object)
    if (type >= 0) {
        objectStack[ objectsInStack ] = createObject(type, attr);
    }

    // if object is a macro or a reference to macro (checked from
    // "role" attribute)
    if ((objectsInStack > 0) && (eventId > 0)) {
        //printf("add macro\n");
        MacroReference macro;
        macro.eventId = eventId;
        macro.macroId = getId(attr);
        addEventReference(&objectStack[objectsInStack - 1], macro);
    }

    // add object to its parent (unless object is in objectpool)
    else if ((objectsInStack > 0) && (type >= 0 || strcmp( el, "include_object") == 0)) {

        multiplier = getMultiplier(attr, multiplier, xform.dm_mult, xform.sk_mult);

        ObjectReference objectReference;
        objectReference.objectId = getId(attr);

        // calculate using block font and multipliers

        objectReference.x = (int) (multiplier * getX(attr)) + getBlockCol(attr) * getBlockFontWidth(attr, (int) multiplier);
        objectReference.y = (int) (multiplier * getY(attr)) + getBlockRow(attr) * getBlockFontHeight(attr, (int) multiplier);
        addObjectReference(&objectStack[ objectsInStack - 1], objectReference, getRole(attr));
    }

    // if elment is the root element "objectpool", calculate deltas
    else if (strcmp(el, "objectpool") == 0) {

        // calculate scales and deltas
        xform.dm_mult = ((float) vtDimension) / ((float) getDimension(attr));
        xform.sk_mult = min(((float) vtSkWidth) / ((float) getSkWidth(attr)),
			    ((float) vtSkHeight) / ((float) getSkHeight(attr)));

        multiplier = min(xform.dm_mult, xform.sk_mult);

        xform.dm_dx = (int) (vtDimension - xform.dm_mult * getDimension(attr)) / 2;
        if (xform.dm_dx < 0) {
            xform.dm_dx = 0;
        }
        xform.dm_dy = xform.dm_dx;

        xform.sk_dx = (int) (vtSkWidth - xform.sk_mult * getSkWidth(attr)) / 2;
        if (xform.sk_dx < 0) {
            xform.sk_dx = 0;
        }

        xform.sk_dy = (int) (vtSkHeight - xform.sk_mult * getSkHeight(attr)) / 2;
        if (xform.sk_dy < 0) {
            xform.sk_dy = 0;
        }

        //printf("dm_mult: %f sk_mult: %f dm_dx: %i dm_dy: %i sk_dx: %i sk_dy: %i\n",
	//       dm_mult, sk_mult, dm_dx, dm_dy, sk_dx, sk_dy);

    }

    // if element is point, add it to its parent (should be a polygon)
    else if (strcmp(el, "point") == 0) {
        Point point;
        point.x = (int) (multiplier * getX(attr));
        point.y = (int) (multiplier * getY(attr));
        addPoint(&objectStack[ objectsInStack - 1], point);
    }

    // if element is image_data start reading data
    else if (strcmp(el, "image_data") == 0) {
        PictureGraphic *pictureGraphic = (PictureGraphic *) objectStack[ objectsInStack-1];
        pictureGraphic->actualWidth = getActualWidth(attr);
        pictureGraphic->actualHeight = getActualHeight(attr);
        dataReading = 1;
    }
    else if (strcmp(el, "language") == 0) {
        // FIXME not implemented yet
    }

    // if elment is a command, add it to a macro
    else if ((command >= 0) && (objectsInStack > 0)) {
        //printf("command: %s\n", el);
        void *comm = createCommand(command, attr);
        addCommand(&objectStack[objectsInStack - 1], comm);
        free(comm);
    }
    else if (objectsInStack == 0) {
    }
    else {
        printf("ERROR: element: %s\n", el);
    }

    if (type >= 0) {
        objectsInStack++;
    }

    // call startFunct() in the main program
    startFunct(data, (char *) el, attr);
}

// expat-parser calls this function when xml-element is 'closed'
void end(void *data, const char *el) {

    // if element was image_data, all data is readed and can be
    // parsed, and aded to image
    if (strcmp(el, "image_data") == 0) {
        addPictureData(&objectStack[objectsInStack - 1]);

        dataReading = 0;
        free(dataText);
        dataText = NULL;
        dataLength = 0;
    }
    int type = -1;
    for (int i = 0; i <= 30; i++)
        if (strcmp(xmlNames[i], el) == 0)
            type = i;

    // object is ready (if it is a real object)
    if (type >= 0) {
        objectsInStack--;
        objectReady(objectStack[objectsInStack]);
    }

    // call endFunct() in the main program
    endFunct(data, (char *) el);
}

// expat-parser calls this function when data is read inside xml-element
// if current element is image_data, then data is saved to an array
void characterDataHandler(void *userData, const XML_Char *s, int len)
{
    (void) userData;
    if (dataReading) {
        dataText = (char *) realloc(dataText, len + dataLength);
        memcpy(dataText + dataLength, s, len);
        dataLength += len;
    }
}

// Fuction parses a .xml file that is imported from PoolEdit program.
// - start() and end() functions are called when a new element is
//   started or ended.
// - ready() is called when parsing is done, and an array with
//   ISOBUS data is returned parameters vtDimension_, vtSkWidth_,
//   vtSkHeight_ and vtColors_ give info about VT
void parse(FILE *file, void (*start_)(void *data, char *el, const char **attr),
           void (*end_) (void *data, char *el), void (*ready)(char *data, int length),
           int vtDimension_, int vtSkWidth_, int vtSkHeight_, int vtColors_)
{
    readyFunct = ready;
    startFunct = start_;
    endFunct = end_;
    vtDimension = vtDimension_;
    vtSkWidth = vtSkWidth_;
    vtSkHeight = vtSkHeight_;
    vtColors = vtColors_;

    objectsInStack = 0;

    XML_Parser p = XML_ParserCreate(NULL);
    XML_SetElementHandler(p, start, end);
    XML_SetCharacterDataHandler(p, characterDataHandler);

    if (file == NULL) {
        fprintf(stderr, "No such file!\n");
        return;
    }

    char Buff[256];
    while (!feof(file)) {
        int len = fread(Buff, 1, 256, file);

        if (ferror(file)) {
            fprintf(stderr, "Read error\n");
            exit(-1);
        }

        if (!XML_Parse(p, Buff, len, feof(file))) {
            fprintf(stderr, "Parse error at line %ld:\n%s\n",
                    XML_GetCurrentLineNumber(p),
                    XML_ErrorString(XML_GetErrorCode(p)));
            exit(-1);
        }
    }
    XML_ParserFree(p);
}
