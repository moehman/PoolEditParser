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
#include "parser.h"
#include "xml.h"

// linked list node
struct node {
    char *head;
    struct node *tail;
};

typedef struct node node_t;

// start and end nodes of the linked list
node_t *list_start = NULL;
node_t *list_end = NULL;

// output file
FILE *fileOut;

int pool_size = 0;
int nro_total_objects = 0;
int nro_root_objects = 0;

int depth = 0;
int printTable = false;
int firstByte = true;

//
// function for adding a new string to the linked list
//
void addToList(char *bff)
{
    // allocate and init new node
    node_t *node = (node_t *) malloc(sizeof(node_t));
    if (node == NULL) {
        printf("out of memory!\n");
        exit(-1);
    }
    node->head = bff;
    node->tail = NULL;

    // add node to list
    if (list_start == NULL) {
        list_start = node;
        list_end = node;
    }
    else {
        list_end->tail = node;
        list_end = node;
    }
}

//
// function for printing the linked list
//
void printList()
{
    node_t *node = list_start;
    while (node != NULL) {
        fprintf(fileOut, "%s", node->head);
        node = node->tail;
    }
}

//
// callback function that prints ascii formatted numbers to file
//
void ascii_ready(char *data, int length)
{
    for (int i = 0; i < length; i++) {
        if (firstByte) {
            firstByte = false;
        }
        else {
            fprintf(fileOut, ", ");
            if (i == 0) {
                fprintf(fileOut, "\n  ");
            }
        }
        fprintf(fileOut, "%i", (unsigned char) data[i]);
    }
    pool_size += length;
    nro_total_objects++;
}

//
// callback function that prints raw bytes to file
//
void binary_ready(char *data, int length)
{
    for (int i = 0; i < length; i++) {
        fputc((unsigned char) data[i], fileOut);
    }
    pool_size += length;
    nro_total_objects++;
}

//
// callback function
//
void starts(void *userData, char *el, const char ** attr)
{
    (void) userData;
    (void) el;

    char tmp[256];
    char *bff;
    char *name;
    int id;
    int len;

    if (depth == 1) {
        name = getName(attr);
        id = getId(attr);

        // print string to tmp, allocate bff accordingly and copy
        // string
        sprintf(tmp, "#define %s %d\n", name, id);
        len = strlen(tmp) + 1;
        bff = (char *) malloc(len);
        memcpy(bff, tmp, len);

        addToList(bff);

        nro_root_objects++;
    }
    depth++;
}

//
// callback function
//
void ends(void *userData, char *el) {
    (void) userData;
    (void) el;
    depth--;
}

//
// function for printing usage information on stdout
//
void printUseage() {
    printf("Usage: pooleditparser xml-filename output-filename -d=[dimension] "
           "-sw=[softkey width] -sh=[softkey height] -c=[colors] [-table]\n");
}

//
// main program
//
int main(int argc, char *argv[])
{
    // input file handle
    FILE *fileIn;

    // setting defaults
    int dimension = 200;
    int skWidth = 60;
    int skHeight = 32;
    int colors = 256;

    // check arguments
    if (argc < 3) {
        printUseage();
        exit(-1);
    }

    // open files
    fileIn = fopen(argv[1], "r");
    if (fileIn == NULL) {
        printf("Can't open file: %s\n", argv[1]);
        exit(-2);
    }

    fileOut = fopen(argv[2], "w");
    if (fileOut == NULL) {
        printf("Can't open file: %s\n", argv[2]);
        exit(-3);
    }

    // evaluate other arguments
    for (int i = 3; i < argc; i++) {
        if (strncmp("-d=", argv[i], 3) == 0) {
            strtok(argv[i], "=");
            dimension = atoi(strtok(NULL, "="));
        }
        else if (strncmp("-sw=", argv[i], 4) == 0) {
            strtok(argv[i], "=");
            skWidth = atoi(strtok(NULL, "="));
        }
        else if (strncmp("-sh=", argv[i], 4) == 0) {
            strtok(argv[i], "=");
            skHeight = atoi(strtok(NULL, "="));
        }
        else if (strncmp("-c=", argv[i], 3) == 0) {
            strtok(argv[i], "=");
            colors = atoi(strtok(NULL, "="));
        }
        else if (strncmp("-table", argv[i], 6) == 0) {
            printTable = true;
        }
        else {
            printUseage();
            exit(-1);
        }
    }

    // sanity checks for arguments
    if (dimension < 200) {
        printf("Too small dimension (%d), using 200\n", dimension);
        dimension = 200;
    }
    if (skWidth < 60) {
        printf("Too small soft key width (%d), using 60\n", skWidth);
        skWidth = 60;
    }
    if (skHeight < 32) {
        printf("Too small soft key height (%d), using 32\n", skHeight);
        skHeight = 32;
    }
    if (colors != 2 && colors != 16 && colors != 256) {
        printf("Invalid number of colors (%d), using 256\n", colors);
        colors = 256;
    }

    // print settings
    printf(
           "***************************************************\n"
           "* Parsing: %s to %s\n"
           "* dimension: %i\n"
           "* softkey size: %ix%i\n"
           "* colors: %i\n",
           argv[1], argv[2], dimension, skWidth, skHeight, colors);

    if (printTable) {
        fprintf(fileOut, "unsigned char *pool = {\n  ");
        parse(fileIn, starts, ends, ascii_ready, dimension, skWidth, skHeight, colors);
        fprintf(fileOut, "\n};\n\n#define POOL_SIZE %d\n\n", pool_size);
        printList();
    }
    else {
        parse(fileIn, starts, ends, binary_ready, dimension, skWidth, skHeight, colors);
    }

    // close files
    fclose(fileIn);
    fclose(fileOut);

    pool_xform_t *xform = get_pool_xform();

    // print statistics
    printf("* dmMultiplier: %f\n"
           "* skMultiplier: %f\n"
           "* dmDeltaX: %d\n"
           "* dmDeltaY: %d\n"
           "* skDeltaX: %d\n"
           "* skDeltaY: %d\n"
           "* generated pool size: %d\n"
           "* total number of objects: %d\n"
           "* number of root level objects: %d\n"
           "***************************************************\n",
           xform->dm_mult, xform->sk_mult,
           xform->dm_dx, xform->dm_dy, xform->sk_dx, xform->sk_dy,
           pool_size, nro_total_objects, nro_root_objects);

    return 0;
}
