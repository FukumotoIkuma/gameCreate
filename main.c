#include "system.h"

// 変数
static SDL_Event event;

void bossFireBall(){
    printf("boss fire ball\n");
    
}

/*
キャラ情報の更新
移動や状態に依存した処理など
*/
void updateChara() {
    for (int i = 0; i < NumGameChara; ++i) {
        switch (gameChara[i].type) {
            case CT_Player: {
                int dir = (Game.input.right - Game.input.left); // intにキャストしないと元々boolだからバグる
                float diff = dir * gameChara[i].max_speed * Game.timeStep;
                // printf("dir:%d\tspeed:%d\tdiff:%f\n",dir,gameChara[i].max_speed,diff);
                if (Game.input.l_shift || Game.input.r_shift) {
                    diff *= SNEAK_SPEED_MODIFIER;
                }
                gameChara[i].point.x += diff;

                // マップの範囲を超えないように制限する
                if (gameChara[i].point.x < MAP_LEFT) {
                    gameChara[i].point.x = MAP_LEFT;
                }
                if (MAP_RIGHT < gameChara[i].point.x + gameChara[i].entity->w) {
                    gameChara[i].point.x = MAP_RIGHT - gameChara[i].entity->w;
                }
                break;
            }
            case CT_Ball: {
                gameChara[i].point.y += gameChara[i].entity->speed * Game.timeStep;

                // 画面下部を超えたら上に再出現
                if (gameChara[i].point.y >= WINDOW_HEIGHT) {
                    gameChara[i].point.x = getRandomBallPosition_X(&gameChara[i]);
                    gameChara[i].point.y = -gameChara[i].entity->h; // 画面上部から再出現する
                }
                break;
            }
            case CT_Boss: {
                
                Uint32 currentTime;
                switch (gameChara[i].stts) {
                    case CS_Disable:
                        currentTime = SDL_GetTicks();
                        if (currentTime - Game.startTime >= BOSS_APPEAR_TIME) {
                            gameChara[i].stts = CS_Appeare;
                        }
                        break;
                    case CS_Appeare:
                        // ボスの移動
                        gameChara[i].point.y += gameChara[i].entity->speed * Game.timeStep;
                        if (gameChara[i].point.y >= 0) {
                            gameChara[i].point.y = 0;
                            gameChara[i].stts = CS_Normal;
                        }
                        break;
                    case CS_Normal:
                        if (gameChara[i].action_frame_countter %120 ==0){
                            //このリセットタイミングは複数攻撃の周期の最小公倍数にすれば安全にリセットできる。
                            gameChara[i].action_frame_countter = 0;
                            bossFireBall();
                        }
                        gameChara[i].action_frame_countter ++;
                        if (gameChara[i].hp <= 0) {
                            gameChara[i].stts = CS_DisAppeare;
                            Game.stts = GS_Clear;
                        }
                        break;
                    case CS_DisAppeare:
                        gameChara[i].point.y -= gameChara[i].entity->speed * Game.timeStep;
                        if (gameChara[i].point.y + gameChara[i].entity->h < 0) {
                            gameChara[i].stts = CS_Disable;
                            gameChara[i].hp = 100000;
                        }
                        break;
                    default:
                        break;
                }
                break;
            }
            case CT_BackGround: {
                gameChara[i].point.y += gameChara[i].entity->speed * Game.timeStep;
                // 背景がウィンドウの下端に達したら再スタート
                if (gameChara[i].point.y >= 0) {
                    gameChara[i].point.y = -gameChara[i].entity->h;
                }
                break;
            }
            default:
                break;
        }
    }
}

void inputHandler() {
    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_QUIT) {
            Game.stts = GS_End;
        } else if (event.type == SDL_MOUSEBUTTONDOWN) { // マウス
            int mouseX, mouseY;
            SDL_GetMouseState(&mouseX, &mouseY);

            for (int i = 0; i < NumGameChara; ++i) {
                // クリックが関係していれば処理開始
                if (!(gameChara[i].point.x <= mouseX && mouseX <= gameChara[i].point.x + gameChara[i].entity->w &&
                      gameChara[i].point.y <= mouseY && mouseY <= gameChara[i].point.y + gameChara[i].entity->h)) {
                    continue;
                }
                switch (gameChara[i].type) {
                    case CT_Ball:
                        switch (gameChara[i].bType) {
                            case OS_PLUS10:
                                setBalltype(&gameChara[i], OS_MINUS10);
                                break;
                            case OS_MINUS10:
                                setBalltype(&gameChara[i], OS_PLUS10);
                                break;
                            case OS_TIME2:
                                setBalltype(&gameChara[i], OS_ZERO);
                                break;
                            case OS_ZERO:
                                setBalltype(&gameChara[i], OS_TIME2);
                                break;
                            default:
                                break;
                        }
                        break;
                    case CT_Boss: // 体力を減らす
                        gameChara[i].hp -= Game.player->power;
                        if (gameChara[i].hp < 0) {
                            gameChara[i].hp = 0;
                        }
                        break;
                    default:
                        break;
                }
            }
        } else if (event.type == SDL_KEYDOWN || event.type == SDL_KEYUP) { // キーボード
            handleKeyInput(&event);
        }
    }
}

int main(int argc, char* argv[]) {
    // 初期化
    if (InitSystem("./data/chara.data", "./data/ball.data") < 0) {
        return PrintError("failed to init system");
    }

    if (InitWindow() < 0) {
        return PrintError("failed to init window");
    }

    // メインループ
    Game.stts = GS_Playing;
    while (Game.stts) {
        // 入力の処理
        inputHandler();

        // 情報の更新
        updateChara();

        // 当たり判定
        for (int i = 0; i < NumGameChara; i++) {
            for (int j = i + 1; j < NumGameChara; j++) {
                Collision(&(gameChara[i]), &(gameChara[j]));
            }
        }

        // 描画
        renderWindow();
        SDL_Delay(1000 / 60); // 60 FPS

        //クリアの場合
        if (Game.stts == GS_Clear) {
        SDL_Delay(3000);
        break;
        // ここでゲームクリア時の描画やメッセージ表示を行う

    }
    }

    destroyWindow();

    return 0;
}
