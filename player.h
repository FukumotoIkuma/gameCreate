#ifndef PLAYER_H
#define PLAYER_H

#include <SDL2/SDL.h>

#define PLAYER_WIDTH 100
#define PLAYER_HEIGHT 150
#define PLAYER_MAX_SPEED 300

typedef struct {
    int x, y;
    int width, height;
    int velocity_x; // X方向の速度
    int max_speed;  // 最高速度
    SDL_Texture* texture;
} Player;

void initPlayer(Player* player, SDL_Renderer* renderer);
void handlePlayerEvent(Player* player, SDL_Event* event);
void updatePlayer(Player* player, float deltaTime);
void renderPlayer(Player* player, SDL_Renderer* renderer);

#endif // PLAYER_H
