#include "menu.h"
#include "files.h"
#include <string.h>

#define LINE_HEIGHT 34
#define MAX_VISIBLE 12

void MenuInit(Menu *menu)
{
    memset(menu, 0, sizeof(Menu));

    LoadVideos(menu);
}

void MenuUpdate(Menu *menu)
{
    if(IsKeyPressed(KEY_DOWN))
    {
        if(menu->selected < menu->count-1)
            menu->selected++;
    }

    if(IsKeyPressed(KEY_UP))
    {
        if(menu->selected>0)
            menu->selected--;
    }

    if(menu->selected < menu->scroll)
        menu->scroll = menu->selected;

    if(menu->selected >= menu->scroll + MAX_VISIBLE)
        menu->scroll = menu->selected - MAX_VISIBLE + 1;
}

void MenuDraw(Menu *menu)
{

    if(menu->count == 0)
    {
        DrawText(
            "NO VIDEO FOUND",
            250,
            250,
            30,
            RED);

        DrawText(
            "Place files in ./videos",
            220,
            290,
            20,
            DARKGREEN);

        return;
    }

    DrawText("LECTURE DES FICHIERS",260,70,40,GREEN);

    DrawLine(100,120,860,120,GREEN);

    DrawRectangleLines(120,150,720,430,GREEN);

    int start=menu->scroll;

    int end=start+MAX_VISIBLE;

    if(end>menu->count)
        end=menu->count;

    int y=170;

    for(int i=start;i<end;i++)
    {
        Color color=DARKGREEN;

        if(i==menu->selected)
        {
            DrawRectangle(130,y-4,700,30,(Color){0,80,0,120});

            color=GREEN;

            if(((int)(GetTime()*2)%2)==0)
                DrawText(">",145,y,24,GREEN);
        }

        DrawText(menu->entries[i].name,180,y,24,color);

        y+=LINE_HEIGHT;
    }

    DrawText("HAUT/BAS : Naviguer",150,620,20,DARKGREEN);

    DrawText("OK : Lire vidéo",430,620,20,DARKGREEN);

    //DrawText("ESC : Quit",650,620,20,DARKGREEN);
}

const char *MenuGetSelected(Menu *menu)
{
    return menu->entries[menu->selected].name;
}