#include "system.h"



//変数
static SDL_Event event;

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
            // printf("dir:%d\tspeed:%d\tdiff:%f\n",dir,gameChara[i].max_speed,diff);
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

void inputHandler(){
    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_QUIT) {
            Game.stts = GS_End;

        } else if (event.type == SDL_MOUSEBUTTONDOWN) {//マウス
            int mouseX, mouseY;
            SDL_GetMouseState(&mouseX, &mouseY);

            for (int i = 0; i < NumGameChara; ++i) {
                //クリックが関係していれば処理開始
                if (!(gameChara[i].point.x<=mouseX && mouseX<=gameChara[i].point.x+gameChara[i].entity->w&&
                    gameChara[i].point.y<=mouseY && mouseY<=gameChara[i].point.y+gameChara[i].entity->h)){
                    continue;
                    }
                switch (gameChara[i].bType)
                {
                case OS_PLUS10:
                    setBalltype(&gameChara[i],OS_MINUS10);
                    break;
                case OS_MINUS10:
                    setBalltype(&gameChara[i],OS_PLUS10);
                    break;
                
                default:
                    break;
                }
            }
        } else if (event.type == SDL_KEYDOWN|| event.type == SDL_KEYUP) {//キーボード
            handleKeyInput(&event);
        }
    }

}
int main(int argc, char* argv[]) {
    
    //初期化
    
    if (0>InitSystem("./data/chara.data", "./data/ball.data")) return PrintError("failed to init system");
        
    if (0>InitWindow()) return PrintError("failed to init window");
    
    
    
    //メインループ
    Game.stts = GS_Playing;
    while (Game.stts) {
        //入力の処理
        inputHandler();
        
        //情報の更新
        updateChara();

        //当たり判定
        for (int i = 0; i < NumGameChara; i++) {
            for (int j = i + 1; j < NumGameChara; j++)
                Collision(&(gameChara[i]), &(gameChara[j]));
        }

        
        //描画
        renderWindow();
        SDL_Delay(1000 / 60); // 60 FPS
    }
    
    destroyWindow();

    return 0;
}
