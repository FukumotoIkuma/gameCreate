#ifndef OBJECT_H
#define OBJECT_H

#include <SDL2/SDL.h>
#include "player.h" 

#define OBJECT_WIDTH 96
#define OBJECT_HEIGHT 96
#define OBJECT_SPEED 200

typedef enum {
    OBJECT_TYPE_PLUS_10,
    OBJECT_TYPE_MINUS_10,
    OBJECT_TYPE_UNKNOWN
} ObjectType;

typedef struct {
    SDL_Texture* texture;
    int x, y;
    int width, height;
    int speed;
    ObjectType type;
} Object;

void initObject(Object* object, SDL_Renderer* renderer, const char* imageFile);
void updateObject(Object* object, float deltaTime);
int checkCollision(Player* player, Object* object);
void renderObject(Object* object, SDL_Renderer* renderer);

#endif
