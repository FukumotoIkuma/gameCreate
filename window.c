#include "system.h"
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>


/*描画関連の初期化*/ 
int InitWindow() 
{
    int ret = 0;
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
    TTF_Font* font = TTF_OpenFont("font.ttf", 24);
    if (font == NULL) return PrintError(TTF_GetError());

    SDL_Color color = { 255, 255, 255, 255 };
    
    //テキストを生成
    char text[20];
    snprintf(text, sizeof(text), "power: %d", Game.player->power);
    
    //以下レンダリング
    SDL_Surface* textSurface = TTF_RenderText_Solid(font, text, color);
    
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
int renderBossHealth(CharaInfo*boss) {
    SDL_Color textColor = { 255, 0, 0, 255 }; // 赤色のテキスト
    char healthText[50];
    snprintf(healthText, sizeof(healthText), "Boss Health: %d", boss->hp);

    TTF_Font* font = TTF_OpenFont("font.ttf", 18);
    if (font == NULL) return PrintError(TTF_GetError());

    SDL_Surface* textSurface = TTF_RenderText_Solid(font, healthText, textColor);
    if (textSurface == NULL) {
        TTF_CloseFont(font);
        return PrintError(SDL_GetError());
    }

    SDL_Texture* textTexture = SDL_CreateTextureFromSurface(Game.renderer, textSurface);
    if (textTexture == NULL) {
        SDL_FreeSurface(textSurface);
        TTF_CloseFont(font);
        return PrintError(SDL_GetError());
    }

    SDL_Rect textRect = { 10, 10, textSurface->w, textSurface->h };
    SDL_RenderCopy(Game.renderer, textTexture, NULL, &textRect);

    SDL_FreeSurface(textSurface);
    SDL_DestroyTexture(textTexture);
    TTF_CloseFont(font);
    return 1;
}


/*画面の更新*/
void renderWindow(){
    //エラー出るから放置
    //qsort(gameChara, MAX_GAME_CHARA, sizeof(CharaInfo), CompSort);
    
    for (int i=0;i<NumGameChara;++i){
        //非表示は処理しない
        if (gameChara[i].stts == CS_Disable) continue;

        //メインの描画
        if (gameChara[i].type == CT_Player)
            printf("positionx:%f\n",gameChara[i].point.x);    
        SDL_Rect dst = {gameChara[i].point.x,gameChara[i].point.y,gameChara[i].entity->w,gameChara[i].entity->h};
        SDL_RenderCopy(Game.renderer,gameChara[i].entity->texture,NULL,&dst);
       
        //各キャラの追加の描画
        switch (gameChara[i].type)
        {

            case CT_BackGround:
                // 2つ目の背景を描画してシームレスなスクロールを実現
                SDL_Rect dst2 = {gameChara[i].point.x,gameChara[i].point.y+gameChara[i].entity->h,\
                    gameChara[i].entity->w,gameChara[i].entity->h};
                SDL_RenderCopy(Game.renderer, gameChara[i].entity->texture, NULL, &dst2);
                break;
            case CT_Player:
                  

                //プレイヤー攻撃力
                renderPlayerPower();
                // プレイヤーの当たり判定用矩形を描画（赤色の矩形）
                //いったいこれはなんなんだ？当たり判定もよくわからんぞ
                SDL_Rect collisionRect = {
                    gameChara[i].point.x + gameChara[i].entity->w / 5,   // x 座標を調整
                    gameChara[i].point.y + gameChara[i].entity->h /8,   // y 座標を調整
                    gameChara[i].entity->w* 2 / 3,               // 幅を縮小
                    gameChara[i].entity->h / 2               // 高さを縮小
                };
                SDL_SetRenderDrawColor(Game.renderer, 43, 43, 43, 255);  // 黒色に設定
                SDL_RenderDrawRect(Game.renderer, &collisionRect);
                break;

            case CT_Boss:
                //ボス体力
                renderBossHealth(&gameChara[i]);
                break;
            case CT_Ball:

            default:
                break;
        }
        
    }
    SDL_RenderPresent(Game.renderer);

}
