#ifndef PLAYER_H
#define PLAYER_H

#include <stdbool.h>

bool PlayerInit(void);

bool PlayerPlay(const char *filename);

void PlayerUpdate(void);

void PlayerStop(void);

bool PlayerIsPlaying(void);

bool PlayerFinished(void);

void PlayerDraw(void);

void PlayerShutdown(void);

#endif