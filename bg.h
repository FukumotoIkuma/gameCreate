#ifndef BG_H
#define BG_H

#include <SDL2/SDL.h>

typedef struct {
    SDL_Texture* texture;
    int x, y;
    int width, height;
    int speed;
} Background;

void initBackground(Background* bg, SDL_Renderer* renderer);
void updateBackground(Background* bg, float deltaTime);
void renderBackground(Background* bg, SDL_Renderer* renderer);

#endif
