/* Do not edit this file. It was automatically generated. */

#ifndef HEADER_CheckItem
#define HEADER_CheckItem
/*
htop - CheckItem.h
(C) 2004-2011 Hisham H. Muhammad
Released under the GNU GPL, see the COPYING file
in the source distribution for its full text.
*/

#include "Object.h"

typedef struct CheckItem_ {
   Object super;
   char* text;
   bool value;
   bool* ref;
} CheckItem;


#ifdef DEBUG
extern char* CHECKITEM_CLASS;
#else
#define CHECKITEM_CLASS NULL
#endif

CheckItem* CheckItem_new(char* text, bool* ref, bool value);

void CheckItem_set(CheckItem* this, bool value);

bool CheckItem_get(CheckItem* this);

#endif