#include "object.h"
#include <SDL2/SDL_image.h>
#include <stdio.h>
#include <stdlib.h>
#include "player.h"

void initObject(Object* object, SDL_Renderer* renderer, const char* imageFile) {
    SDL_Surface* tempSurface = IMG_Load(imageFile);
    if (!tempSurface) {
        printf("Failed to load object image: %s\n", IMG_GetError());
        return;
    }

    object->texture = SDL_CreateTextureFromSurface(renderer, tempSurface);
    SDL_FreeSurface(tempSurface);

    if (!object->texture) {
        printf("Failed to create object texture: %s\n", SDL_GetError());
        return;
    }

    object->width = 96; // オブジェクトの幅
    object->height = 96; // オブジェクトの高さ

    object->x = rand() % (531 - 190) + 190;
    object->y = -object->height; // 画面上部から出現する

    object->speed = 200; // 移動速度を設定

    // 画面外に出た場合は必ず +10.png のオブジェクトとして再配置する
    if (object->y >= WINDOW_HEIGHT) {
        imageFile = "+10.png";
    }

    if (strcmp(imageFile, "+10.png") == 0) {
        object->type = OBJECT_TYPE_PLUS_10;
    } else if (strcmp(imageFile, "-10.png") == 0) {
        object->type = OBJECT_TYPE_MINUS_10;
    } else {
        object->type = OBJECT_TYPE_UNKNOWN;
    }
}

void updateObject(Object* object, float deltaTime) {
    object->y += object->speed * deltaTime;

    if (object->y >= WINDOW_HEIGHT) {
        object->x = rand() % (531 - 190) + 190;
        object->y = -object->height; // 画面上部から再出現する
    }
}

int checkCollision(Player* player, Object* object) {
    SDL_Rect playerRect = { player->x + player->width / 5, player->y + player->height / 8, player->width * 2 / 3, player->height / 2 };
    SDL_Rect objectRect = { object->x, object->y, object->width, object->height };

    if (SDL_HasIntersection(&playerRect, &objectRect)) {
        return 1; // 当たっている場合は1を返す
    }
    return 0; // 当たっていない場合は0を返す
}

void renderObject(Object* object, SDL_Renderer* renderer) {
    SDL_Rect dstRect = { object->x, object->y, object->width, object->height };
    SDL_RenderCopy(renderer, object->texture, NULL, &dstRect);
}

int getObjectEffect(Object* object) {
    switch (object->type) {
        case OBJECT_TYPE_PLUS_10:
            return 10;
        case OBJECT_TYPE_MINUS_10:
            return -10;
        default:
            return 0;
    }
}
