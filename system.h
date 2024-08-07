#include <SDL2/SDL.h>

//キャラ設定用
#define MAX_LINEBUF 256
#define MAX_GAME_CHARA 256
#define BOSS_APPEAR_TIME 10000 // ボスが出現する時間（ミリ秒）
#define SNEAK_SPEED_MODIFIER 1.0/3
#define MAX_FIRE_BALL_COUNT 5//ボスのファイアーボールとして確保する数。多少dps上げても対応できる。

//ウィンドウ
#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 600

//マップ
#define MAP_LEFT 200
#define MAP_RIGHT 600


//キャラタイプ
#define CHARATYPE_NUM 4
typedef enum {
    CT_Player       = 0, // プレイヤー
    CT_BackGround   = 1, // 背景
    CT_Boss         = 2, // ボス
    CT_Ball         = 3, // オブジェクト

} CharaType;


//キャラのタイプ別情報
typedef struct {
    int w;       // 幅
    int h;       // 高さ
    int speed;   // 速度
    char* path;  // 画像ファイルのパス
    SDL_Texture* texture; // SDLテクスチャ
} CharaTypeInfo;

/*キャラのステータス*/
typedef enum {
    CS_Disable = 0, // 非表示
    CS_Normal  = 1, // 通常
    CS_Appeare = 2,
    CS_DisAppeare = 3,
} CharaStts;

//ボールタイプ
#define BALLTYPE_NUM 5
typedef enum{
    OS_PLUS10       = 0,
    OS_MINUS10      = 1,
    OS_TIME2        = 2,
    OS_ZERO         = 3,
    OS_FIREBALL
}BallType;

/*キャラクター情報*/
typedef struct CharaInfo_t {
    CharaType type;        // キャラクタータイプ
    CharaStts stts;        // 現在の状態
    CharaTypeInfo* entity; // タイプ別情報の実体
    SDL_FPoint dir;        // 現在の方向（大きさ最大1となる成分）
    SDL_FPoint point;      // 現在の座標（画像の中心、壁のみ左上）
    SDL_Point ani;         // 現在のアニメーションパターン（x:動作，y:向き）
    int power;              // パワー（プレイヤー、オブジェクト）
    int max_speed;          //最大速度(プレイヤー)
    int hp;                 // HP（boss）
    int action_frame_countter;//この値が任意の値で割り切れたタイミングで行動する。定期的にリセットしないとバグの元になる
    union {
        BallType bType;//オブジェクト用のタイプ設定
    };
} CharaInfo;

/* キー入力の状態 */
typedef struct {
    SDL_bool right;
    SDL_bool left;
    SDL_bool l_shift;
    SDL_bool r_shift;
} KeyStts;

/* ゲームの状態 */
typedef enum {
    GS_End     = 0, // 終了
    GS_Playing = 1, // 通常
    GS_Ready   = 2,  // 開始前
    GS_Clear   = 3
} GameStts;

/* 
ゲームの情報 
KeyStts input;
CharaInfo* player;       
SDL_Window* window;            
SDL_Renderer* render; 
*/
typedef struct {
    GameStts stts;
    KeyStts input;
    Uint32 startTime;
    float timeStep;                 // 時間の増分(1フレームの時間,s)
    CharaInfo* player;              // プレイヤー
    CharaInfo* boss;                //ボス
    SDL_Window* window;             // ウインドウ
    SDL_Renderer* renderer;           // レンダラー
} GameInfo;



//関数
//system.c
extern int InitSystem(const char* charaFileName,const char* objectFileName) ;
extern int PrintError(const char* message);
extern void Collision(CharaInfo* ci , CharaInfo* cj);
extern void setBalltype(CharaInfo* target,BallType bType);
extern void handleKeyInput(SDL_Event* key_event);
extern int getRandomBallPosition_X(CharaInfo* chara);
//window.c
extern int InitWindow();
extern void renderWindow();
extern void destroyWindow();
//変数
//system.c
extern SDL_Window* window;
extern SDL_Renderer* renderer;
extern CharaTypeInfo charaType[CHARATYPE_NUM];
extern CharaInfo* gameChara;
extern CharaInfo** fireBalls;
extern int NumGameChara;
extern GameInfo Game;
extern CharaTypeInfo ballType[BALLTYPE_NUM];


//window.c