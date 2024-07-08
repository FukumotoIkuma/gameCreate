#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "main.h"

#include "system.h"

/*キー入力をゲームに反映
key_eventはdown,upのみ受け付ける*/
void handleKeyInput(SDL_Event* key_event){
    if (key_event->type != SDL_KEYDOWN && key_event->type != SDL_KEYUP)

        return;

    //downならTrue,upならfalse
    SDL_bool is_key_down  = key_event->type == SDL_KEYDOWN;
    
    switch (key_event->key.keysym.sym) {
            case SDLK_LEFT:
                Game.input.left = is_key_down;
                break;
            case SDLK_RIGHT:
                Game.input.right = is_key_down;
                break;
            case SDLK_LSHIFT:
                Game.input.l_shift = is_key_down;
            case SDLK_RSHIFT:
                Game.input.r_shift = is_key_down;
        }

    
}

/*
キャラ情報の更新
移動や状態に依存した処理など
*/
void updateChara(){
    for (int i=0;i<NumGameChara;++i){
        switch (gameChara[i].type)
        {
        case CT_Player:
            int dir = (Game.input.right-Game.input.left);//intにキャストしないと元々boolだからバグる
            float diff = dir*gameChara[i].max_speed * Game.timeStep;
            printf("dir:%d\tspeed:%d\tdiff:%f\n",dir,gameChara[i].max_speed,diff);
            if (Game.input.l_shift || Game.input.r_shift)
                diff *= 1/3;
            gameChara[i].point.x += diff;

            // マップの範囲を超えないように制限する
            if (gameChara[i].point.x<200) 
                gameChara[i].point.x = 200;
            if (gameChara[i].point.x>600-gameChara[i].entity->w) 
                gameChara[i].point.x = 600-gameChara[i].entity->w;//マジックナンバー
            
            

            break;
        case CT_Ball:
            gameChara[i].point.y += gameChara[i].entity->speed*Game.timeStep;

            //画面下部を超えたら上に再出現
            if (gameChara[i].point.y>=WINDOW_HEIGHT){
                gameChara[i].point.x = rand() % (531 - 190) + 190;//マジックナンバー
                gameChara[i].point.y = -gameChara[i].entity->h; // 画面上部から再出現する  
            }
            break;
        case CT_Boss:
            //一定時間経過でボスが出現
            if (gameChara[i].stts == CS_Disable){
                Uint32 currentTime = SDL_GetTicks();
                if (currentTime - Game.startTime >= BOSS_APPEAR_TIME) 
                        gameChara[i].stts = CS_Normal;
                else
                    continue;//これ以上処理は必要ない
                }
            //ボスの移動
            gameChara[i].point.y += gameChara[i].entity->speed*Game.timeStep;
            break;
        case CT_BackGround:
            gameChara[i].point.y += gameChara[i].entity->speed * Game.timeStep;
            // 背景がウィンドウの下端に達したら再スタート
            if (gameChara[i].point.y >= 0)
                gameChara[i].point.y = -gameChara[i].entity->h;
            break;

        default:
            break;
        }
    }

}

/*他でチェックしきれないゲームステータスの変化を追跡する*/
void updateGameInfo(){
    return;
}




int main(int argc, char* argv[]) {
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

    //処理開始
    
    //初期化
    
    if (0>InitSystem("./data/chara.data", "./data/ball.data")) return PrintError("failed to init system");
        
    
    
    if (0>InitWindow()) return PrintError("failed to init window");

    //その他
    
    SDL_Event event;
    
    
    //メインループ
    Game.stts = GS_Playing;
    while (Game.stts) {
        //入力の処理
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                Game.stts = GS_End;

            } else if (event.type == SDL_MOUSEBUTTONDOWN) {
                int mouseX, mouseY;
                SDL_GetMouseState(&mouseX, &mouseY);

                for (int i = 0; i < NumGameChara; ++i) {
                    //クリックが関係していれば処理開始
                    if (!(gameChara[i].point.x<=mouseX && mouseX<=gameChara[i].point.x &&
                        gameChara[i].point.y<=mouseY && mouseY<=gameChara[i].point.y))
                        goto NEXTLOOP;
                    switch (gameChara[i].oType)
                    {
                    case OS_PLUS10:
                        gameChara[i].oType = OS_MINUS10;
                        break;
                    case OS_MINUS10:
                        gameChara[i].type = OS_PLUS10;
                        break;
                    
                    default:
                        break;
                    }
                }
            } else if (event.type == SDL_KEYDOWN|| event.type == SDL_KEYUP) {
                handleKeyInput(&event);
            }
        NEXTLOOP:
        }
        //情報の更新と描画
        updateChara();

        //当たり判定
        for (int i = 0; i < NumGameChara; i++) {
            for (int j = i + 1; j < NumGameChara; j++)
                Collision(&(gameChara[i]), &(gameChara[j]));
        }

        
        //ゲームの状態更新
        renderWindow();
        SDL_Delay(1000 / 60); // 60 FPS
    }

    SDL_DestroyRenderer(Game.renderer);
    SDL_DestroyWindow(Game.window);
    TTF_Quit();
    IMG_Quit();
    SDL_Quit();

    return 0;
}
