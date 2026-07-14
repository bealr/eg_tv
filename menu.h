#ifndef MENU_H
#define MENU_H

#include "raylib.h"

#define MAX_FILES 256

typedef struct
{
    char name[256];
} MenuEntry;

typedef struct
{
    MenuEntry entries[MAX_FILES];

    int count;

    int selected;

    int scroll;

} Menu;

void MenuInit(Menu *menu);

void MenuUpdate(Menu *menu);

void MenuDraw(Menu *menu);

const char *MenuGetSelected(Menu *menu);

#endif