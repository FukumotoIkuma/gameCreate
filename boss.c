// boss.c
#include "boss.h"
#include <SDL2/SDL_image.h>
#include <stdio.h>

int bossHealth = 10000; // ボスの初期体力

void initBoss(Boss* boss, SDL_Renderer* renderer, const char* imageFile, int power) {
    SDL_Surface* surface = IMG_Load(imageFile);
    if (!surface) {
        printf("Failed to load boss image: %s\n", IMG_GetError());
        return;
    }
    boss->texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_FreeSurface(surface);
    if (!boss->texture) {
        printf("Failed to create boss texture: %s\n", SDL_GetError());
        return;
    }
    boss->width = surface->w;
    boss->height = surface->h;
    boss->x = (800 - boss->width) / 2; // 中央に配置
    boss->y = -boss->height; // 画面外から降りてくるように配置
    boss->power = power;
}

void updateBoss(Boss* boss, float deltaTime) {
    boss->y += 50 * deltaTime; // 50 ピクセル/秒の速度で降りてくる
}

void renderBoss(Boss* boss, SDL_Renderer* renderer) {
    SDL_Rect dstRect = { boss->x, boss->y, boss->width, boss->height };
    SDL_RenderCopy(renderer, boss->texture, NULL, &dstRect);
}
