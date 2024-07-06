#ifndef BOSS_H
#define BOSS_H

#include <SDL2/SDL.h>

typedef struct {
    SDL_Texture* texture;
    int x, y;
    int width, height;
    int power;
} Boss;

extern int bossHealth; // 外部で定義されたbossHealthの宣言

void initBoss(Boss* boss, SDL_Renderer* renderer, const char* imageFile, int power);
void updateBoss(Boss* boss, float deltaTime);
void renderBoss(Boss* boss, SDL_Renderer* renderer);

#endif // BOSS_H
