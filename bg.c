// bg.c

#include "bg.h"
#include "main.h"
#include <SDL2/SDL_image.h>
#include <stdio.h>

void initBackground(Background* bg, SDL_Renderer* renderer) {
    // 画像を読み込み
    SDL_Surface* tempSurface = IMG_Load("map1.png");
    if (!tempSurface) {
        printf("Failed to load background image: %s\n", IMG_GetError());
        return;
    }

    bg->texture = SDL_CreateTextureFromSurface(renderer, tempSurface);
    SDL_FreeSurface(tempSurface);

    if (!bg->texture) {
        printf("Failed to create background texture: %s\n", SDL_GetError());
        return;
    }

    bg->x = (WINDOW_WIDTH - BG_WIDTH) / 2; // ウィンドウ中央に配置
    bg->y = -WINDOW_HEIGHT; // スクロールの開始位置
    bg->width = BG_WIDTH;
    bg->height = BG_HEIGHT;

    bg->speed = 100; // スクロール速度を1に変更
}

void updateBackground(Background* bg, float deltaTime) {
    // 背景の位置を更新
    bg->y += bg->speed * deltaTime;

    // 背景がウィンドウの下端に達したら再スタート
    if (bg->y >= 0) {
        bg->y = -bg->height;
    }
}

void renderBackground(Background* bg, SDL_Renderer* renderer) {
    SDL_Rect dstRect = { bg->x, bg->y, bg->width, bg->height };
    SDL_RenderCopy(renderer, bg->texture, NULL, &dstRect);

    // 2つ目の背景を描画してシームレスなスクロールを実現
    SDL_Rect dstRect2 = { bg->x, bg->y + bg->height, bg->width, bg->height };
    SDL_RenderCopy(renderer, bg->texture, NULL, &dstRect2);
}
