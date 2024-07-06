// player.c

#include "player.h"
#include <SDL2/SDL_image.h>
#include <stdio.h>
#include "main.h"

void initPlayer(Player* player, SDL_Renderer* renderer) {
    player->x = 100;
    player->y = WINDOW_HEIGHT - 150; // 画面下部に設定
    player->width = 100; // サイズを設定
    player->height = 150; // サイズを設定
    player->velocity_x = 0; // 初期速度は0
    player->max_speed = 300; // 最高速度を設定

    // 画像を読み込み
    SDL_Surface* tempSurface = IMG_Load("player.png");
    if (!tempSurface) {
        printf("Failed to load player image: %s\n", IMG_GetError());
        return;
    }

    player->texture = SDL_CreateTextureFromSurface(renderer, tempSurface);
    SDL_FreeSurface(tempSurface);

    if (!player->texture) {
        printf("Failed to create texture: %s\n", SDL_GetError());
    }
}

void handlePlayerEvent(Player* player, SDL_Event* event) {
    if (event->type == SDL_KEYDOWN) {
        switch (event->key.keysym.sym) {
            case SDLK_LEFT:
                if (event->key.keysym.mod & KMOD_SHIFT) {
                    player->velocity_x = -player->max_speed / 3; // 左方向の半分の速度で移動
                } else {
                    player->velocity_x = -player->max_speed; // 左方向の最高速度で移動
                }
                break;
            case SDLK_RIGHT:
                if (event->key.keysym.mod & KMOD_SHIFT) {
                    player->velocity_x = player->max_speed / 3; // 右方向の半分の速度で移動
                } else {
                    player->velocity_x = player->max_speed; // 右方向の最高速度で移動
                }
                break;
        }
    } else if (event->type == SDL_KEYUP) {
        switch (event->key.keysym.sym) {
            case SDLK_LEFT:
            case SDLK_RIGHT:
                player->velocity_x = 0; // 左右のキーが離されたら速度をゼロにする
                break;
        }
    }
}

void updatePlayer(Player* player, float deltaTime) {
    // プレイヤーの速度に応じて座標を更新する
    player->x += player->velocity_x * deltaTime;

    // マップの範囲を超えないように制限する
    if (player->x < 200) player->x = 200;
    if (player->x > 600 - player->width) player->x = 600 - player->width;
}

void renderPlayer(Player* player, SDL_Renderer* renderer) {
    // プレイヤーのテクスチャを描画
    SDL_Rect dstRect = { player->x, player->y, player->width, player->height };
    SDL_RenderCopy(renderer, player->texture, NULL, &dstRect);

    // プレイヤーの当たり判定用矩形を描画（赤色の矩形）
    SDL_Rect collisionRect = {
        player->x + player->width / 5,   // x 座標を調整
        player->y + player->height / 8,  // y 座標を調整
        player->width * 2 / 3,               // 幅を縮小
        player->height / 2               // 高さを縮小
    };
    SDL_SetRenderDrawColor(renderer, 43, 43, 43, 255);  // 黒色に設定
    SDL_RenderDrawRect(renderer, &collisionRect);
}
