#include "system.h"
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_image.h>
#include <time.h>

/*変数*/
static char gImgFilePath[CHARATYPE_NUM][MAX_LINEBUF];
static char ballImgFilePath[BALLTYPE_NUM][MAX_LINEBUF];
int NumGameChara;
CharaTypeInfo charaType[CHARATYPE_NUM];//キャラタイプごとの基本情報
CharaInfo* gameChara;
GameInfo Game;
CharaTypeInfo ballType[BALLTYPE_NUM];//ボールタイプごとの基本情報


/*関数*/
static void InitCharaInfo();
static BallType getRandomBall();
void setBalltype(CharaInfo* target,BallType bType);

/**/
int PrintError(const char* message) {
    fprintf(stderr, "Error: %s\n", message);
    return -1;
}

int InitSystem(const char* charaFileName,const char* objectFileName) 
{
    //初期化of初期化
    if (SDL_Init(SDL_INIT_VIDEO) < 0) return PrintError(SDL_GetError());

    if (!(IMG_Init(IMG_INIT_PNG) & IMG_INIT_PNG)) return PrintError(IMG_GetError());

    if (TTF_Init() == -1) return PrintError(TTF_GetError());

    //ステータスの更新
    Game.stts = GS_Ready; 
    int ret = 0;
    srand(time(NULL));// 乱数の初期化
    Game.startTime = SDL_GetTicks();
    Game.timeStep = 1.0f/60;

    //ボール情報の読み込み
    FILE* fp = fopen(objectFileName, "r");
    if (fp == NULL) {
        return PrintError("failed to ball data file.");
    }
    

    char linebuf_ball[MAX_LINEBUF];
    int typeno_ball = 0;

    while (fgets(linebuf_ball, MAX_LINEBUF, fp)) {
        // 先頭が#の行はコメントとして飛ばす
        if (linebuf_ball[0] == '#') continue;

        // ボールタイプ別情報読込
        if (typeno_ball < BALLTYPE_NUM) {
            //読み込み
            if (4 != sscanf(
                linebuf_ball, "%d%d%d%s", &(ballType[typeno_ball].w), &(ballType[typeno_ball].h),
                 &(ballType[typeno_ball].speed),ballImgFilePath[typeno_ball])) {
                ret = PrintError("failed to read the ball image data.");
                goto CLOSEFILE;
            }
            ballType[typeno_ball].path = ballImgFilePath[typeno_ball];
            printf("%s\n",ballType[typeno_ball].path);
            typeno_ball++;
        }
    }
    fclose(fp);

    //キャラ関係
    fp = fopen(charaFileName, "r");
    if (fp == NULL) {
        return PrintError("failed to open position data file.");
    }
    

    char linebuf_chara[MAX_LINEBUF];
    int typeno_chara = 0;
    while (fgets(linebuf_chara, MAX_LINEBUF, fp)) {
        // 先頭が#の行はコメントとして飛ばす
        if (linebuf_chara[0] == '#')
            continue;

        // キャラタイプ別情報読込
        if (typeno_chara < CHARATYPE_NUM) {
            //読み込み
            if (4 != sscanf(linebuf_chara, "%d%d%d%s", &(charaType[typeno_chara].w), &(charaType[typeno_chara].h), &(charaType[typeno_chara].speed),gImgFilePath[typeno_chara])) {
                ret = PrintError("failed to read the chara image data.");
                goto CLOSEFILE;
            }
            charaType[typeno_chara].path = gImgFilePath[typeno_chara];
            

            typeno_chara++;
        }
    }
    //キャラ情報確保 
    gameChara = (CharaInfo*)malloc(sizeof(CharaInfo) * MAX_GAME_CHARA);
    if (!gameChara) {
        ret = PrintError("failed to allocate the chara data.");
        goto CLOSEFILE;
    }
    // キャラ情報設定 
    InitCharaInfo();


CLOSEFILE:
    fclose(fp);
    return ret;
}

void InitCharaInfo(){
    //初期キャラ設定
    CharaType ini_chara[] = {CT_BackGround,CT_Player,CT_Ball,CT_Ball,CT_Boss};
    int ini_chara_length = 5;
    

    for(int i = 0;i<ini_chara_length;++i){
        //共通設定
        NumGameChara ++;
        gameChara[i].type = ini_chara[i];
        gameChara[i].stts = CS_Normal;
        gameChara[i].entity = &(charaType[ini_chara[i]]);
        gameChara[i].dir = (SDL_FPoint) { 0.0, 0.0 };
        gameChara[i].ani = (SDL_Point) { 0, 0 };


        //キャラごとの設定
        switch (ini_chara[i])
        {
        case CT_Player:
            gameChara[i].point.x = 300;
            gameChara[i].point.y = WINDOW_HEIGHT - gameChara[i].entity->h;
            gameChara[i].max_speed = 300;
            gameChara[i].power = 0;
            Game.player = &(gameChara[i]);
            gameChara[i].hp = 300;
            break;
        case CT_Boss:
            gameChara[i].stts = CS_Disable;
            gameChara[i].point.x = (WINDOW_WIDTH - gameChara[i].entity->w) / 2;
            gameChara[i].point.y = -(gameChara[i].entity->h);
            gameChara[i].power = 100;
            gameChara[i].hp = 1000000;
            break;
        case CT_Ball:
            gameChara[i].point.x = getRandomBallPosition_X(&gameChara[i]);
            gameChara[i].point.y = -(gameChara[i].entity->h);

            //ランダムにオブジェクトタイプを設定
            setBalltype(&gameChara[i],getRandomBall());
            break;
        case CT_BackGround:
            gameChara[i].point.x = (WINDOW_WIDTH - gameChara[i].entity->w)/2;
            gameChara[i].point.y = -(gameChara[i].entity->h);
            break;

        
        default:
            break;
        }

    }

}
/*オブジェクトとの衝突処理。
*オブジェクトによるパワーの変化.
オブジェクトの再配置

args
---
player:CharaInfo
    プレイヤー
object:CharaInfo
    オブジェクト
*/
void collisionBall(CharaInfo* player , CharaInfo* ball){

    float y_distanve = player->point.y - (ball->point.y+ball->entity->h);
    if (y_distanve>0) return;
    if (ball->point.x+ball->entity->w<player->point.x || 
    player->point.x+player->entity->w<ball->point.x) return;
    
    //プレイヤーパワーの変更
    switch (ball->bType)
    {
    case OS_PLUS10:
        player->power += 10;
        break;
    case OS_MINUS10:
        player->power -= 10;
        break;
    case OS_TIME2:
        player->power *= 2;
        break;
    case OS_ZERO:
        player->power = 0;
        break;
    default:
        break;
    }
    //オブジェクトの再配置
    ball->point.y = WINDOW_HEIGHT+1;//画面外に映すことで、再配置される
    setBalltype(ball,getRandomBall());
}

/*ボスとの衝突*/
void collisionBoss(CharaInfo* player ,CharaInfo* boss){
    if (boss->point.y >= player->point.y - boss->entity->h){
        printf("Game Over! Player power: %d, Boss power: %d\n", player->power, boss->power);
        Game.stts = GS_End;
    }/*else{
        printf("Game Clear! Player power: %d, Boss power: %d\n", player->power, boss->power);
    }*/
   //意味がわからない。崩壊してる。
    

}


/* 当たり判定
対象キャラ同士が重なったか調べ，状態更新などをする
 
args
----
ci: 対象キャラ
cj: 対象キャラ
 */
void Collision(CharaInfo* ci , CharaInfo* cj){
    //非表示は処理しない
    if (ci->stts == CS_Disable || cj->stts == CS_Disable) return;

    //プレイヤ＝以外に衝突判定はないので処理しない
    if (!(ci->type == CT_Player || cj->type == CT_Player))return;

    //ciがプレイヤーになるようにする
    if (cj->type == CT_Player) {
        CharaInfo* temp = ci;
        ci = cj;
        cj = temp;
    }

    switch (cj->type)
    {
        case CT_Ball:
            collisionBall(ci,cj);
            break;
        case CT_Boss:
            collisionBoss(ci,cj);
        default:
            break;
    }

}


void setBalltype(CharaInfo* target,BallType bType){
    target->bType = bType;
    target->entity = &ballType[bType];
}

/*ランダムなオブジェクトを取得する関数

return
---
BallType
*/
BallType getRandomBall() {
    int randomPercent = rand() % 1000;
    if(randomPercent < 50)
        return OS_TIME2;
    else if(randomPercent < 500)
        return OS_PLUS10;
    else if(randomPercent < 950)
        return OS_MINUS10;
    else return OS_ZERO;
}
int getRandomBallPosition_X(CharaInfo* chara){
    int over_map = 10;
    int over_left = MAP_LEFT-over_map;
    int over_right = MAP_RIGHT+over_map;
    int res = rand() % ((over_right-chara->entity->w)-over_left) + over_left;
    return res;
}

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
/* end of system.c */

