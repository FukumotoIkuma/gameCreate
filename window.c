#include "system.h"
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>

//変数
static TTF_Font* font_boss_health;
static TTF_Font* font_player_power;
static SDL_Color color_white;
static SDL_Color color_red;

/*描画関連の初期化*/ 
int InitWindow() 
{
    int ret = 0;
    //フォントの初期化

    font_boss_health = TTF_OpenFont("font.ttf", 18);
    if (font_boss_health == NULL) return PrintError(TTF_GetError());

    font_player_power = TTF_OpenFont("font.ttf", 24);
    if (font_player_power == NULL) return PrintError(TTF_GetError());

    color_white = (SDL_Color){ 255, 255, 255, 255 };
    color_red = (SDL_Color){ 255, 0, 0, 255 }; 


    //ウィンドウとレンダラーの初期化
    SDL_Window* window = SDL_CreateWindow("SDL Game", SDL_WINDOWPOS_CENTERED, 
    SDL_WINDOWPOS_CENTERED, WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_SHOWN);
    if (!window) {
        return PrintError(SDL_GetError());
    }
    Game.window = window;

    SDL_Renderer* renderer = SDL_CreateRenderer(Game.window, -1, SDL_RENDERER_ACCELERATED);
    if (!renderer) {
        SDL_DestroyWindow(Game.window);
        return PrintError(SDL_GetError());
    }
    Game.renderer = renderer;
    

    //テクスチャの設定
    //同じ処理繰り返してるが、型が違くてどうにもならん。どうしよう。
    for(int i=0;i<BALLTYPE_NUM;++i){//ボール
        SDL_Surface* tempSurface = IMG_Load(ballType[i].path);
        if (!tempSurface) 
            return PrintError(IMG_GetError());
        
        ballType[i].texture = SDL_CreateTextureFromSurface(Game.renderer, tempSurface);
        SDL_FreeSurface(tempSurface);

        if (ballType[i].texture == NULL) 
            return PrintError(SDL_GetError());
    }

    for (int i=0;i<CHARATYPE_NUM;++i){//キャラ
        SDL_Surface* tempSurface = IMG_Load(charaType[i].path);
        if (!tempSurface) 
            return PrintError(IMG_GetError());
        
        charaType[i].texture = SDL_CreateTextureFromSurface(Game.renderer, tempSurface);
        
        SDL_FreeSurface(tempSurface);

        if (charaType[i].texture ==NULL) 
            return PrintError(SDL_GetError());
        
    }
    /** ウインドウへの描画 **/
    renderWindow();

    /* image利用終了(テクスチャに転送後はゲーム中に使わないので) */
    IMG_Quit();

    return ret;
}

int CompSort(const void *a, const void *b) {
    const CharaInfo *ca = (const CharaInfo *)a;
    const CharaInfo *cb = (const CharaInfo *)b;

    if(ca->type == CT_BackGround ) return -1;
    if(cb->type == CT_BackGround) return 1;
    if (ca->type == CT_Ball && cb->type != CT_Ball) return -1;
    if (cb->type == CT_Ball && ca->type != CT_Ball) return 1;

    // 例：point.yを基準に昇順でソート
    return (int)(ca->point.y - cb->point.y);
}

int renderPlayerPower() {
    //テキストを生成
    char text[20];
    snprintf(text, sizeof(text), "power: %d", Game.player->power);
    
    //以下レンダリング
    SDL_Surface* textSurface = TTF_RenderText_Solid(font_player_power, text, color_white);
    
    if (!textSurface) return PrintError(TTF_GetError());
    
    // renderer is invalid
    SDL_Texture* textTexture = SDL_CreateTextureFromSurface(Game.renderer, textSurface);
    SDL_FreeSurface(textSurface);
    if (textTexture == NULL) return PrintError(SDL_GetError());


    SDL_Rect dstRect = { WINDOW_WIDTH - 200, 10, 190, 30 };

    SDL_RenderCopy(Game.renderer, textTexture, NULL, &dstRect);

    SDL_DestroyTexture(textTexture);
    return 1;
}
int renderBossHealth(CharaInfo* boss) {
    char healthText[50];
    snprintf(healthText, sizeof(healthText), "Boss Health: %d", boss->hp);


    SDL_Surface* textSurface = TTF_RenderText_Solid(font_boss_health, healthText, color_red);
    if (textSurface == NULL) {
        TTF_CloseFont(font_boss_health);
        return PrintError(TTF_GetError());
    }

    SDL_Texture* textTexture = SDL_CreateTextureFromSurface(Game.renderer, textSurface);
    if (textTexture == NULL) {
        SDL_FreeSurface(textSurface);
        TTF_CloseFont(font_boss_health);
        return PrintError(SDL_GetError());
    }

    SDL_Rect textRect = { 10, 10, textSurface->w, textSurface->h };
    SDL_RenderCopy(Game.renderer, textTexture, NULL, &textRect);

    SDL_FreeSurface(textSurface);
    SDL_DestroyTexture(textTexture);
    // TTF_CloseFont(font_boss_health);
    return 1;
}


/*画面の更新*/
void renderWindow(){
    //画面のクリア
    SDL_RenderClear(Game.renderer);
    //エラー出るから放置
    //qsort(gameChara, MAX_GAME_CHARA, sizeof(CharaInfo), CompSort);
    
    for (int i=0;i<NumGameChara;++i){
        //非表示は処理しない
        if (gameChara[i].stts == CS_Disable) continue;

        //メインの描画   
        SDL_Rect dst = {gameChara[i].point.x,gameChara[i].point.y,gameChara[i].entity->w,gameChara[i].entity->h};
        SDL_RenderCopy(Game.renderer,gameChara[i].entity->texture,NULL,&dst);
       
        //各キャラの追加の描画
        switch (gameChara[i].type)
        {
            case CT_BackGround: {
                // 2つ目の背景を描画してシームレスなスクロールを実現
                SDL_Rect dst2 = {gameChara[i].point.x,gameChara[i].point.y+gameChara[i].entity->h,\
                    gameChara[i].entity->w,gameChara[i].entity->h};
                SDL_RenderCopy(Game.renderer, gameChara[i].entity->texture, NULL, &dst2);
                break;
            }
            case CT_Player: {
                //プレイヤー攻撃力
                renderPlayerPower();
                // プレイヤーの当たり判定用矩形を描画（黒色の矩形）
                SDL_Rect collisionRect = {
                    gameChara[i].point.x + gameChara[i].entity->w / 5,   // x 座標を調整
                    gameChara[i].point.y + gameChara[i].entity->h /8,   // y 座標を調整
                    gameChara[i].entity->w* 2 / 3,               // 幅を縮小
                    gameChara[i].entity->h / 2               // 高さを縮小
                };
                SDL_SetRenderDrawColor(Game.renderer, 43, 43, 43, 255);  // 黒色に設定
                SDL_RenderDrawRect(Game.renderer, &collisionRect);
                break;
            }
            case CT_Boss: {
                //ボス体力
                renderBossHealth(&gameChara[i]);
                break;
            }
            case CT_Ball: {
                break;
            }
            default:
                break;
        }
    }

     // ゲームクリア時のテキスト表示
    if (Game.stts == GS_Clear) {
        SDL_Surface* clearSurface = TTF_RenderText_Solid(font_player_power, "GAME CLEAR", color_white);
        if (clearSurface != NULL) {
            SDL_Texture* clearTexture = SDL_CreateTextureFromSurface(Game.renderer, clearSurface);
            if (clearTexture != NULL) {
                SDL_Rect clearRect = { WINDOW_WIDTH / 2 - clearSurface->w / 2, WINDOW_HEIGHT / 2 - clearSurface->h / 2, clearSurface->w, clearSurface->h };
                SDL_RenderCopy(Game.renderer, clearTexture, NULL, &clearRect);
                SDL_DestroyTexture(clearTexture);
            }
            SDL_FreeSurface(clearSurface);
        }
    }
    SDL_RenderPresent(Game.renderer);
}

void destroyWindow(){
    TTF_CloseFont(font_boss_health);
    TTF_CloseFont(font_player_power);
    SDL_DestroyRenderer(Game.renderer);
    SDL_DestroyWindow(Game.window);
    TTF_Quit();
    IMG_Quit();
    SDL_Quit();
}
