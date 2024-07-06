#ifndef PLAYER_H
#define PLAYER_H

#include <SDL2/SDL.h>

#define WINDOW_WIDTH 800  // ウィンドウの幅を定義
#define WINDOW_HEIGHT 600 // ウィンドウの高さを定義

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
