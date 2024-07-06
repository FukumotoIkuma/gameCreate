// main.c

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "player.h"
#include "bg.h"
#include "object.h"
#include "boss.h" // 追加

#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 600
#define NUM_OBJECTS 2
#define BOSS_APPEAR_TIME 10000 // ボスが出現する時間（ミリ秒）

void updateCombatPowerText(SDL_Renderer* renderer, int combatPower, TTF_Font* font, SDL_Color color) {
    char text[20];
    snprintf(text, sizeof(text), "power: %d", combatPower);
    
    SDL_Surface* textSurface = TTF_RenderText_Solid(font, text, color);
    if (!textSurface) {
        printf("Failed to render text surface! SDL_ttf Error: %s\n", TTF_GetError());
        return;
    }
    
    SDL_Texture* textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);
    SDL_FreeSurface(textSurface);
    if (!textTexture) {
        printf("Failed to create text texture! SDL Error: %s\n", SDL_GetError());
        return;
    }

    SDL_Rect dstRect = { WINDOW_WIDTH - 200, 10, 190, 30 };
    SDL_RenderCopy(renderer, textTexture, NULL, &dstRect);
    SDL_DestroyTexture(textTexture);
}

void renderBossHealth(SDL_Renderer* renderer) {
    SDL_Color textColor = { 255, 0, 0, 255 }; // 赤色のテキスト
    char healthText[50];
    snprintf(healthText, sizeof(healthText), "Boss Health: %d", bossHealth);

    TTF_Font* font = TTF_OpenFont("font.ttf", 18);
    if (!font) {
        printf("Failed to load font! TTF_Error: %s\n", TTF_GetError());
        return;
    }

    SDL_Surface* textSurface = TTF_RenderText_Solid(font, healthText, textColor);
    if (!textSurface) {
        printf("Failed to render text surface! SDL_Error: %s\n", SDL_GetError());
        TTF_CloseFont(font);
        return;
    }

    SDL_Texture* textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);
    if (!textTexture) {
        printf("Failed to create text texture! SDL_Error: %s\n", SDL_GetError());
        SDL_FreeSurface(textSurface);
        TTF_CloseFont(font);
        return;
    }

    SDL_Rect textRect = { 10, 10, textSurface->w, textSurface->h };
    SDL_RenderCopy(renderer, textTexture, NULL, &textRect);

    SDL_FreeSurface(textSurface);
    SDL_DestroyTexture(textTexture);
    TTF_CloseFont(font);
}

// グローバル変数
Uint32 startTime;
int bossAppeared = 0;
Boss boss;

int main(int argc, char* argv[]) {
    srand(time(NULL));

    //エラーのハンドリング

    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        printf("Failed to initialize SDL! SDL_Error: %s\n", SDL_GetError());
        return 1;
    }

    if (!(IMG_Init(IMG_INIT_PNG) & IMG_INIT_PNG)) {
        printf("Failed to initialize SDL_image! IMG_Error: %s\n", IMG_GetError());
        return 1;
    }

    if (TTF_Init() == -1) {
        printf("Failed to initialize SDL_ttf! TTF_Error: %s\n", TTF_GetError());
        return 1;
    }

    SDL_Window* window = SDL_CreateWindow("SDL Game", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_SHOWN);
    if (!window) {
        printf("Failed to create window! SDL_Error: %s\n", SDL_GetError());
        return 1;
    }

    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (!renderer) {
        printf("Failed to create renderer! SDL_Error: %s\n", SDL_GetError());
        SDL_DestroyWindow(window);
        return 1;
    }

    //処理開始
    
    //初期化

    //プレイヤー
    Player player;
    initPlayer(&player, renderer);

    //背景
    Background bg;
    initBackground(&bg, renderer);

    //オブジェクト
    Object objects[NUM_OBJECTS];
    const char* objectImages[] = {"+10.png", "-10.png"}; // オブジェクトの種類を定義
    for (int i = 0; i < NUM_OBJECTS; ++i) {
        const char* imageFile = objectImages[rand() % 2]; // ランダムにオブジェクトの種類を選択
        initObject(&objects[i], renderer, imageFile);
    }

    //ボス
    initBoss(&boss, renderer, "boss.png", 100);

    //戦闘力
    TTF_Font* font = TTF_OpenFont("font.ttf", 24);
    if (!font) {
        printf("Failed to load font! TTF_Error: %s\n", TTF_GetError());
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        TTF_Quit();
        IMG_Quit();
        SDL_Quit();
        return 1;
    }

    SDL_Color textColor = { 255, 255, 255, 255 };

    int combatPower = 0;

    //その他
    int quit = 0;
    SDL_Event event;

    startTime = SDL_GetTicks();

    
    
    //メインループ
    while (!quit) {
        //入力の処理
        while (SDL_PollEvent(&event)) {

            if (event.type == SDL_QUIT) {
                quit = 1;

            } else if (event.type == SDL_MOUSEBUTTONDOWN) {
                int mouseX, mouseY;
                SDL_GetMouseState(&mouseX, &mouseY);

                for (int i = 0; i < NUM_OBJECTS; ++i) {
                    if (mouseX >= objects[i].x && mouseX <= objects[i].x + objects[i].width &&
                        mouseY >= objects[i].y && mouseY <= objects[i].y + objects[i].height) {
                        
                        const char* newImageFile;
                        switch (objects[i].type) {
                            case OBJECT_TYPE_MINUS_10:
                                newImageFile = "+10.png";
                                objects[i].type = OBJECT_TYPE_PLUS_10;
                                break;
                            case OBJECT_TYPE_PLUS_10:
                                newImageFile = "-10.png";
                                objects[i].type = OBJECT_TYPE_MINUS_10;
                                break;
                            default:
                                break;
                        }
                            
                        SDL_Surface* newSurface = IMG_Load(newImageFile);
                        if (!newSurface) {
                            printf("Failed to load object image: %s\n", IMG_GetError());
                            break;
                        }
                        SDL_Texture* newTexture = SDL_CreateTextureFromSurface(renderer, newSurface);
                        SDL_FreeSurface(newSurface);
                        if (!newTexture) {
                            printf("Failed to create object texture: %s\n", SDL_GetError());
                            break;
                        }

                        SDL_DestroyTexture(objects[i].texture);
                        objects[i].texture = newTexture;
                    }
                }
            } else if (event.type == SDL_KEYDOWN) {
                handlePlayerEvent(&player, &event);
            }
        }

        //情報の更新と描画

        //描画.先に記述するほど後ろ側に配置されるの
        SDL_RenderClear(renderer);

        //背景
        updateBackground(&bg, 1.0f / 60);
        renderBackground(&bg, renderer);

        //プレイヤー
        updatePlayer(&player, 1.0f / 60);
        renderPlayer(&player, renderer);


        //オブジェクト
        for (int i = 0; i < NUM_OBJECTS; ++i) {
            updateObject(&objects[i], 1.0f / 60);
            if (checkCollision(&player, &objects[i])) {
                combatPower += getObjectEffect(&objects[i]);
                objects[i].y = WINDOW_HEIGHT + 1; // 画面外に移動して再配置
                const char* imageFile = objectImages[rand() % 2]; // 新しいオブジェクトの種類を選択
                initObject(&objects[i], renderer, imageFile);
            }
        }

        for (int i = 0; i < NUM_OBJECTS; ++i) {
            renderObject(&objects[i], renderer);
        }

        //戦闘力
        updateCombatPowerText(renderer, combatPower, font, textColor);

        //ボス
        Uint32 currentTime = SDL_GetTicks();
        if (!bossAppeared && currentTime - startTime >= BOSS_APPEAR_TIME) {
            bossAppeared = 1;
        }
        if (bossAppeared) {
            updateBoss(&boss, 1.0f / 60);
            renderBoss(&boss, renderer);
            renderBossHealth(renderer); // ボスの体力を表示する

            //ゲームの動作がこんなこと起きないようになってるが？？？
            if (boss.y >= player.y - boss.height) { // ボスがプレイヤーに到達した場合
                if (combatPower < boss.power) {
                    printf("Game Over! Player power: %d, Boss power: %d\n", combatPower, boss.power);
                    quit = 1;
                } else {
                    printf("Game Clear! Player power: %d, Boss power: %d\n", combatPower, boss.power);
                    quit = 1;
                }
            }
        }



        SDL_RenderPresent(renderer);
        SDL_Delay(1000 / 60); // 60 FPS
    }

    TTF_CloseFont(font);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    TTF_Quit();
    IMG_Quit();
    SDL_Quit();

    return 0;
}
