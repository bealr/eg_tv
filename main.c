#include "raylib.h"
#include <stdio.h>
#include <string.h>
#include "menu.h"
#include "player.h"

#define WIDTH 960
#define HEIGHT 720

#define PIN_CODE "2580"
#define PIN_LENGTH 4

typedef enum
{
    STATE_BOOT,
    STATE_PIN,
    STATE_GRANTED,
    STATE_DENIED,
    STATE_MENU,
    STATE_VIDEO
} AppState;

static const char *bootLines[] =
{
    "VIDEO TERMINAL BIOS v1.42",
    "",
    "Checking Memory...............OK",
    "Checking Video.................OK",
    "Loading Archive...............OK",
    "Loading Security..............OK",
    "",
    "READY"
};

int main(void)
{
    InitWindow(WIDTH, HEIGHT, "tv player");
    Menu menu;
    MenuInit(&menu);
    SetTargetFPS(60);
    PlayerInit();

    AppState state = STATE_PIN;

    int bootIndex = 0;
    float bootTimer = 0.f;

    char pin[5] = {0};
    int pinLength = 0;

    float messageTimer = 0.f;

    while (!WindowShouldClose())
    {
        float dt = GetFrameTime();

        switch(state)
        {
            case STATE_BOOT:
            {
                bootTimer += dt;

                if(bootTimer > 0.45f)
                {
                    bootTimer = 0.f;
                    bootIndex++;

                    if(bootIndex > 8)
                        state = STATE_PIN;
                }
            }
            break;

            case STATE_PIN:
            {
                int key = GetKeyPressed();

                while(key > 0)
                {
                    if(pinLength < PIN_LENGTH)
                    {
                        if(key >= KEY_ZERO && key <= KEY_NINE)
                        {
                            pin[pinLength++] = '0' + key - KEY_ZERO;
                            pin[pinLength] = 0;
                        }

                        if(key >= KEY_KP_0 && key <= KEY_KP_9)
                        {
                            pin[pinLength++] = '0' + key - KEY_KP_0;
                            pin[pinLength] = 0;
                        }
                    }

                    key = GetKeyPressed();
                }

                if(IsKeyPressed(KEY_BACKSPACE) && pinLength > 0)
                {
                    pinLength--;
                    pin[pinLength] = 0;
                }

                if(IsKeyPressed(KEY_ENTER) && pinLength == PIN_LENGTH)
                {
                    messageTimer = 2.f;

                    if(strcmp(pin, PIN_CODE) == 0)
                        state = STATE_GRANTED;
                    else
                        state = STATE_DENIED;
                }
            }
            break;

            case STATE_GRANTED:
            {
                messageTimer -= dt;

                if(messageTimer <= 0.f)
                    state = STATE_MENU;
            }
            break;

            case STATE_DENIED:
            {
                messageTimer -= dt;

                if(messageTimer <= 0.f)
                {
                    memset(pin,0,sizeof(pin));
                    pinLength = 0;
                    state = STATE_PIN;
                }
            }
            break;

            case STATE_MENU:
                MenuUpdate(&menu);
                

                if(IsKeyPressed(KEY_ENTER))
                {
                    char path[512];

                    snprintf(
                        path,
                        sizeof(path),
                        "videos/%s",
                        MenuGetSelected(&menu));


                    PlayerPlay(path);
                    printf("THE FILE : %s\n", path);

                    state = STATE_VIDEO;
                }
                break;

            case STATE_VIDEO:
                PlayerUpdate();
                PlayerDraw();


                if(PlayerFinished())
                {
                    PlayerStop();
                    state = STATE_MENU;
                }


                if(IsKeyPressed(KEY_ESCAPE))
                {
                    PlayerStop();
                    state = STATE_MENU;
                }

            break;
        }

        BeginDrawing();

        ClearBackground(BLACK);

        // Léger scintillement
        int alpha = 20 + GetRandomValue(0,20);
        DrawRectangle(0,0,WIDTH,HEIGHT,(Color){0,255,0,alpha});

        // Scanlines
        for(int y=0;y<HEIGHT;y+=3)
            DrawLine(0,y,WIDTH,y,(Color){0,0,0,60});

        DrawRectangleLines(20,20,WIDTH-40,HEIGHT-40,GREEN);

        switch(state)
        {
            case STATE_BOOT:
            {
                DrawText("BOOTING...",50,40,30,GREEN);

                for(int i=0;i<bootIndex;i++)
                    DrawText(bootLines[i],70,100+i*40,24,GREEN);
            }
            break;

            case STATE_PIN:
            {
                DrawText("LECTEUR DE FICHIER",280,80,40,GREEN);

                DrawLine(120,140,840,140,GREEN);

                DrawText("ENTRER CODE D'ACCÈS",280,220,30,GREEN);

                char display[32] = "_ _ _ _";

                for(int i=0;i<pinLength;i++)
                    display[i*2]='*';

                if(((int)(GetTime()*2)%2)==0 && pinLength<PIN_LENGTH)
                    display[pinLength*2]='█';

                DrawText(display,380,330,42,GREEN);

                //DrawText("ENTER : Validate",300,520,20,DARKGREEN);
                //DrawText("BACKSPACE : Delete",300,550,20,DARKGREEN);
            }
            break;

            case STATE_GRANTED:
            {
                DrawText("ACCÈS AUTORISÉ",250,300,50,GREEN);
            }
            break;

            case STATE_DENIED:
            {
                DrawText("ACCÈS INTERDIT",250,300,50,RED);
            }
            break;

            case STATE_MENU:
            {
                MenuDraw(&menu);
            }
            break;
        }

        EndDrawing();
    }

    CloseWindow();

    return 0;
}