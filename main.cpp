#include <stdio.h>
#include <string>
#include <time.h>
#include <math.h>
#include <vector>
#include <algorithm>
#include <chrono>
#include <thread>

#ifdef _WIN32
#include <conio.h>
#include <windows.h>
#else
#include <poll.h>
#include <signal.h>
#include <termios.h>
#include <unistd.h>
#endif


/*
* ANSI Escape Sequences 参考 =>  https://gist.github.com/fnky/458719343aabd01cfb17a3a4f7296797
* Hexadecimal を使う
* conio.h キーボード（Windows）、termios キーボード（macOS / Linux）
*/

/*****************************
todo list
敵-普通
- [x] 敵の攻撃を実装する
- [x] 敵の弾を実装する
- [x] 敵の爆発を実装する
- [x] 敵の移動を実装する


敵ーボス
- [x] ボス戦を発生する
- [x] ボスの攻撃を実装する
- [x] ボスのHPを実装する
- [x] ボスを実装する
- [x] ボスの弱点を実装する
- [x] ボスの弱点を攻撃して倒す
- [x] ボスの弱点を攻撃して倒したら、敵のボスの爆発アニメーションを実装する
- [x] ボスを倒したら、エンディングを表示する
- [x] ボスがざこと召喚する
- [x] ざこの攻撃を実装する
- [x] ざこの爆発を実装する

Enviroment
- [x] 地形を実装する
- [x] Crosshairを実装する
- [x] spaceshipを実装する
- [x] UIを実装する
- [x] UIのFPSを実装する
- [x] 秒読み、打ち上げ
- [x] 秒読み、死亡
- [x] 秒読みを実装する
- [x] 秒読みのUIを実装する
- [x] 秒読みのアニメーションを実装する
- [x] 武器を実装する
- [x] 武器の弾を実装する
- [x] 敵のAttackするとき、画面反白を実装する
- [x] 敵撃っていたの判定

*****************************/

using namespace std;

const int CONSOLE_WIDTH = 120;
const int CONSOLE_HEIGHT = 30;
const bool EMPTY_CHAR_SW = false;
const int GUN_LENGTH = 5;
const int PLAYER_HP = 9;
const int PLAYER_AMMO = 7;
const int PLAYER_AMMO_MAX = 10;           // UIに収まる弾薬の上限
const double SHOT_COOLDOWN = 0.2;         // 連射間隔（秒）

// 準備フェーズ
const double PREP_PHASE_TIME = 60.0;      // 準備フェーズの時間（秒）
const int PREP_ENEMY_TOTAL = 40;          // 敵機の総数
const double PREP_SPAWN_INTERVAL = 1.5;   // 出現間隔（秒）
const double ENEMY_LIFE_MIN = 3.0;        // 生存時間の最小（秒）
const double ENEMY_LIFE_MAX = 7.0;        // 生存時間の最大（秒）
const int ENEMY_DAMAGE = 1;

// Bossフェーズ
const double BOSS_INTRO_TIME = 3.0;
const int BOSS_HP = 20;
const double BOSS_WEAKNESS_INTERVAL = 4.0;       // 弱点の切替周期（秒）
const double BOSS_WEAKNESS_INTERVAL_BONUS = 3.0; // 全機撃墜時はテンポが速くなる
const double BOSS_WEAKNESS_SHOW = 2.0;           // 弱点の表示時間（秒）
const double BOSS_ATTACK_INTERVAL = 7.0;
const int BOSS_DAMAGE = 3;
const double MINION_SUMMON_INTERVAL = 6.0;
const double MINION_LIFE = 8.0;
const int MINION_DAMAGE = 2;
const int MINION_MAX = 20;                // 撃墜数0のときの僕従数（撃墜2機ごとに1体減る）

// スコア
const int ENEMY_SCORE = 100;
const int MINION_SCORE = 150;
const int WEAKNESS_SCORE = 200;
const int BOSS_SCORE = 3000;
const int PERFECT_SCORE = 2000;
const int HP_BONUS_SCORE = 500;


const string author = "GC1B_10_ラ_ケツブン";

enum CharWord {
    SPACE = ' ',
    NEWLINE = '\n',
    NULL_CHAR = '\0',
    EMPTY_CHAR = '?',
};



/********* System , Config , Resource *************/
#pragma region "色"
// 明度
enum ConsoleColorLightness {
    BRIGHT = 0,
    BOLD = 1,
    DIMMED = 2,
};

// 色
enum ConsoleColor {
    BLACK = 30,
    RED = 31,
    GREEN = 32,
    YELLOW = 33,
    BLUE = 34,
    MAGENTA = 35,
    CYAN = 36,
    WHITE = 37,
    DEFAULT = 39,

};
#pragma endregion

#pragma region "Platform 関数「Windows / macOS / Linux」"
// OSごとに違う処理はここだけに書く。ゲームとエンジンはここの関数だけを使う

// 方向キーはOSごとに届き方が違うので、共通のコードに変換して返す
const int PLATFORM_KEY_UP = 1000;
const int PLATFORM_KEY_DOWN = 1001;
const int PLATFORM_KEY_LEFT = 1002;
const int PLATFORM_KEY_RIGHT = 1003;
const int PLATFORM_KEY_NONE = -1;
const int PLATFORM_KEY_ENTER = 13;
const int PLATFORM_KEY_ESCAPE = 27;

// 起動してからの時間（ミリ秒）
long long PlatformNowMs() {
    using namespace std::chrono;
    return duration_cast<milliseconds>(steady_clock::now().time_since_epoch()).count();
}

void PlatformSleepMs(int ms) {
    std::this_thread::sleep_for(std::chrono::milliseconds(ms));
}

#ifdef _WIN32

void PlatformSetup() {
    // UTF-8で出力し、ANSI Escape Sequencesを有効にする
    SetConsoleOutputCP(CP_UTF8);
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    DWORD mode = 0;
    if (GetConsoleMode(hOut, &mode)) {
        SetConsoleMode(hOut, mode | ENABLE_VIRTUAL_TERMINAL_PROCESSING);
    }
}

void PlatformRestore() {
}

bool PlatformKeyHit() {
    return _kbhit() != 0;
}

int PlatformReadKey() {
    int key = _getch();
    // 方向キーは2バイトで届く（0 または 224 の後にキーのコード）
    if (key == 0 || key == 224) {
        switch (_getch()) {
        case 72: return PLATFORM_KEY_UP;
        case 80: return PLATFORM_KEY_DOWN;
        case 75: return PLATFORM_KEY_LEFT;
        case 77: return PLATFORM_KEY_RIGHT;
        default: return PLATFORM_KEY_NONE;
        }
    }
    return key;
}

void PlatformSetTitle(const string& title) {
    SetConsoleTitleA(title.c_str());
}

void PlatformCursorVisible(bool flag) {
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_CURSOR_INFO info = { 100, flag };
    SetConsoleCursorInfo(hOut, &info);
}

#else

static termios originalTermios;
static bool termiosChanged = false;

void PlatformRestore() {
    if (termiosChanged) {
        tcsetattr(STDIN_FILENO, TCSANOW, &originalTermios);
        termiosChanged = false;
    }
    fputs("\x1b[0m\x1b[?25h", stdout); // 色とカーソルを元に戻す
    fflush(stdout);
}

// Ctrl+Cで終了しても、端末の設定を元に戻す
void PlatformSignalHandler(int signalNumber) {
    PlatformRestore();
    _exit(128 + signalNumber);
}

void PlatformSetup() {
    // 入力を1文字ずつ受け取り、押したキーを画面に表示しない
    if (tcgetattr(STDIN_FILENO, &originalTermios) == 0) {
        termios raw = originalTermios;
        raw.c_lflag &= ~(ICANON | ECHO);
        raw.c_cc[VMIN] = 0;
        raw.c_cc[VTIME] = 0;
        tcsetattr(STDIN_FILENO, TCSANOW, &raw);
        termiosChanged = true;
    }
    atexit(PlatformRestore);
    signal(SIGINT, PlatformSignalHandler);
    signal(SIGTERM, PlatformSignalHandler);
}

bool PlatformWaitInput(int timeoutMs) {
    pollfd fd = { STDIN_FILENO, POLLIN, 0 };
    return poll(&fd, 1, timeoutMs) > 0 && (fd.revents & POLLIN);
}

bool PlatformKeyHit() {
    return PlatformWaitInput(0);
}

int PlatformReadByte() {
    unsigned char c;
    return read(STDIN_FILENO, &c, 1) == 1 ? c : PLATFORM_KEY_NONE;
}

int PlatformReadKey() {
    int key = PlatformReadByte();
    if (key == '\n' || key == '\r') {
        return PLATFORM_KEY_ENTER;
    }
    // 方向キーは ESC [ A のように届く。ESCだけならEscキー
    if (key == PLATFORM_KEY_ESCAPE && PlatformWaitInput(10)) {
        int next = PlatformReadByte();
        if ((next == '[' || next == 'O') && PlatformWaitInput(10)) {
            switch (PlatformReadByte()) {
            case 'A': return PLATFORM_KEY_UP;
            case 'B': return PLATFORM_KEY_DOWN;
            case 'D': return PLATFORM_KEY_LEFT;
            case 'C': return PLATFORM_KEY_RIGHT;
            default: return PLATFORM_KEY_NONE;
            }
        }
        return PLATFORM_KEY_NONE;
    }
    return key;
}

void PlatformSetTitle(const string& title) {
    printf("\x1b]0;%s\x07", title.c_str());
}

void PlatformCursorVisible(bool flag) {
    fputs(flag ? "\x1b[?25h" : "\x1b[?25l", stdout);
}

#endif
#pragma endregion

#pragma region "Console システム関数"
void RunCmd(string cmd) {
    fputs(cmd.c_str(), stdout);
}

void OutPrint(string str) {
    printf("%s", str.c_str());
}

void OutPrintln(string str) {
    printf("%s\n", str.c_str());
}

void ConsoleCursorVisible(bool flag) {
    PlatformCursorVisible(flag); // カーソルを非表示にする
}

void ConsoleSetCursorPosition(int x, int y) {
    string cmd = "\x1b[" + to_string(y) + ';' + to_string(x) + 'H';
    RunCmd(cmd);
}

void ConsoleSetForegroundColor(ConsoleColorLightness lightness, ConsoleColor color) {
    string cmd = "\x1b[" + to_string(lightness) + ';' + to_string(color) + 'm';
    RunCmd(cmd);
}

void ConsoleSetForegroundColor(ConsoleColor color) {
    ConsoleSetForegroundColor(BOLD, color);
}

void ConsoleResetColor() {
    string cmd = "\x1b[39m";
    RunCmd(cmd);
}

void ConsoleClear() {
    // 画面全体を消して、カーソルを先頭に戻す
    RunCmd("\x1b[2J\x1b[H");
}

void ConsoleSetup() {
    PlatformSetup();
}

void ConsoleTitle(string title) {
    PlatformSetTitle(title);
}

#pragma endregion

#pragma region "ASCII絵"
struct NumberAA {
    string sprites[10] = {
        "  ▄▄▄  " + string(1, NEWLINE) +
        " █   █ " + string(1, NEWLINE) +
        " █   █ " + string(1, NEWLINE) +
        " █   █ " + string(1, NEWLINE) +
        "  ▀▀▀  " + string(1, NEWLINE),

        "   █   " + string(1, NEWLINE) +
        "  ██   " + string(1, NEWLINE) +
        "   █   " + string(1, NEWLINE) +
        "   █   " + string(1, NEWLINE) +
        "  ▀▀▀  " + string(1, NEWLINE),

        " ▄▄▄▄  " + string(1, NEWLINE) +
        "     █ " + string(1, NEWLINE) +
        "  ▄▄▄  " + string(1, NEWLINE) +
        " █     " + string(1, NEWLINE) +
        " ▀▀▀▀▀ " + string(1, NEWLINE),

        "  ▄▄▄  " + string(1, NEWLINE) +
        "     █ " + string(1, NEWLINE) +
        "  ▄▄▄  " + string(1, NEWLINE) +
        "     █ " + string(1, NEWLINE) +
        "  ▀▀▀  " + string(1, NEWLINE),

        " █   █ " + string(1, NEWLINE) +
        " █   █ " + string(1, NEWLINE) +
        " █████ " + string(1, NEWLINE) +
        "     █ " + string(1, NEWLINE) +
        "     █ " + string(1, NEWLINE),

        " ▄▄▄▄▄ " + string(1, NEWLINE) +
        " █     " + string(1, NEWLINE) +
        " █▄▄▄  " + string(1, NEWLINE) +
        "     █ " + string(1, NEWLINE) +
        " █▄▄▄  " + string(1, NEWLINE),

        "  ▄▄▄  " + string(1, NEWLINE) +
        " █     " + string(1, NEWLINE) +
        " █▄▄▄  " + string(1, NEWLINE) +
        " █   █ " + string(1, NEWLINE) +
        "  ▀▀▀  " + string(1, NEWLINE),

        " ▄▄▄▄▄ " + string(1, NEWLINE) +
        "     █ " + string(1, NEWLINE) +
        "    █  " + string(1, NEWLINE) +
        "   █   " + string(1, NEWLINE) +
        "  █    " + string(1, NEWLINE),

        "  ▄▄▄  " + string(1, NEWLINE) +
        " █   █ " + string(1, NEWLINE) +
        "  ▄▄▄  " + string(1, NEWLINE) +
        " █   █ " + string(1, NEWLINE) +
        "  ▀▀▀  " + string(1, NEWLINE),

        "  ▄▄▄  " + string(1, NEWLINE) +
        " █   █ " + string(1, NEWLINE) +
        "  ▀▀▀█ " + string(1, NEWLINE) +
        "     █ " + string(1, NEWLINE) +
        "  ▀▀▀  " + string(1, NEWLINE),
    };
    const int height = 5;
    const int width = 7;
};


/*
        .---.
      ./[o o]\.
     |__\___/__|
      \__|_|__/
       '--'--'
*/
struct UFOBossSubAA {
    string sprites =
        "   .---.   " + string(1, NEWLINE) +
        " ./[o o]\\. " + string(1, NEWLINE) +
        "|__\\___/__|" + string(1, NEWLINE) +
        " \\__|_|__/ " + string(1, NEWLINE) +
        "  '--'--'  " + string(1, NEWLINE);
    const int height = 5;
    const int width = 11;
};

struct UFOBossAA {
    string sprites[4] = {
        "            ___             " + string(1, NEWLINE) +
        "          .'   '.           " + string(1, NEWLINE) +
        "     _.-'/  | |  \\'-._      " + string(1, NEWLINE) +
        " _.-'   |__/___\\__|   '-._  " + string(1, NEWLINE) +
        "|==[@]==[@]==[@]==[@]==[@]=|" + string(1, NEWLINE) +
        "'-._     \\_______/     _.-' " + string(1, NEWLINE) +
        "    '-._     _     _.-'     " + string(1, NEWLINE) +
        "        '----'----'         " + string(1, NEWLINE),

        "            ___             " + string(1, NEWLINE) +
        "          .'   '.           " + string(1, NEWLINE) +
        "     _.-'/  | |  \\'-._      " + string(1, NEWLINE) +
        " _.-'   |__/___\\__|   '-._  " + string(1, NEWLINE) +
        "|[@]==[@]==[@]==[@]==[@]==[|" + string(1, NEWLINE) +
        "'-._     \\_______/     _.-' " + string(1, NEWLINE) +
        "    '-._     _     _.-'     " + string(1, NEWLINE) +
        "        '----'----'         " + string(1, NEWLINE),

        "            ___             " + string(1, NEWLINE) +
        "          .'   '.           " + string(1, NEWLINE) +
        "     _.-'/  | |  \\'-._      " + string(1, NEWLINE) +
        " _.-'   |__/___\\__|   '-._  " + string(1, NEWLINE) +
        "|]==[@]==[@]==[@]==[@]==[@]|" + string(1, NEWLINE) +
        "'-._     \\_______/     _.-' " + string(1, NEWLINE) +
        "    '-._     _     _.-'     " + string(1, NEWLINE) +
        "        '----'----'         " + string(1, NEWLINE),

        "            ___             " + string(1, NEWLINE) +
        "          .'   '.           " + string(1, NEWLINE) +
        "     _.-'/  | |  \\'-._      " + string(1, NEWLINE) +
        " _.-'   |__/___\\__|   '-._  " + string(1, NEWLINE) +
        "|@]==[@]==[@]==[@]==[@]==[@|" + string(1, NEWLINE) +
        "'-._     \\_______/     _.-' " + string(1, NEWLINE) +
        "    '-._     _     _.-'     " + string(1, NEWLINE) +
        "        '----'----'         " + string(1, NEWLINE),
    };

    const int height = 8;
    const int width = 27;


    string deathSprites[4] = {
        "                           " + string(1, NEWLINE) +
        "          .' '.            " + string(1, NEWLINE) +
        "     _.-'/  |  \\'-._       " + string(1, NEWLINE) +
        " _.-' |__/___\\__|   '-.    " + string(1, NEWLINE) +
        "|==[]==[]==[]==[]==[]=|    " + string(1, NEWLINE) +
        "'-._   \\______/    _.-'    " + string(1, NEWLINE) +
        "    '-._    _    _.-'      " + string(1, NEWLINE) +
        "        '---'---'          " + string(1, NEWLINE) +
        "                           " + string(1, NEWLINE) +
        "                           " + string(1, NEWLINE),

        "                           " + string(1, NEWLINE) +
        "                           " + string(1, NEWLINE) +
        "            ___            " + string(1, NEWLINE) +
        "        .-'/ | \\'-._       " + string(1, NEWLINE) +
        "    _.-' |__\\__|  '-.      " + string(1, NEWLINE) +
        "   |=[]=[]==[ ==[]==[]=|   " + string(1, NEWLINE) +
        "   '-._   \\____/      .-'  " + string(1, NEWLINE) +
        "       '- '---'--'         " + string(1, NEWLINE) +
        "                           " + string(1, NEWLINE) +
        "                           " + string(1, NEWLINE),

        "                           " + string(1, NEWLINE) +
        "                           " + string(1, NEWLINE) +
        "                           " + string(1, NEWLINE) +
        "                           " + string(1, NEWLINE) +
        "           ___             " + string(1, NEWLINE) +
        "   .-' |_/_\\__  '-.        " + string(1, NEWLINE) +
        "  |= ]==[ =[]==[]=|        " + string(1, NEWLINE) +
        "     '-._    _  _.-'       " + string(1, NEWLINE) +
        "         '---''            " + string(1, NEWLINE) +
        "                           " + string(1, NEWLINE),

        "     _.-^^---....,,--      " + string(1, NEWLINE) +
        " _--                 --_   " + string(1, NEWLINE) +
        "<     B O O O O M !!!    >)" + string(1, NEWLINE) +
        " \\._     * * * *     _./   " + string(1, NEWLINE) +
        "    '''--. . , ; .--'''    " + string(1, NEWLINE) +
        "          | | | |          " + string(1, NEWLINE) +
        "      .-=|| | | ||=-.      " + string(1, NEWLINE) +
        "      '-=#$%&%$#=-'        " + string(1, NEWLINE) +
        "         | ;  :|           " + string(1, NEWLINE) +
        " _____.,-#%&$@%#&#~,._____ " + string(1, NEWLINE),
    };

    const int dHeight = 10;
    const int dWidth = 27;

    string weakness =
        "╔─┬─╗" + string(1, NEWLINE) +
        "├─╬─┤" + string(1, NEWLINE) +
        "╚─┴─╝" + string(1, NEWLINE);

    const int weaknessHeight = 3;
    const int weaknessWidth = 5;
};

struct UFOenemyAA {
    string rightSprites[4] = {
        "  _‗---‗_  " + string(1, NEWLINE) +
        " / [>--] \\ " + string(1, NEWLINE) +
        "|}__{}__{}|" + string(1, NEWLINE) +
        "   \\‗≡‗/   " + string(1, NEWLINE),

        "  _‗---‗_  " + string(1, NEWLINE) +
        " / [->-] \\ " + string(1, NEWLINE) +
        "|_{}__{}_{|" + string(1, NEWLINE) +
        "   \\‗≡‗/   " + string(1, NEWLINE),

        "  _‗---‗_  " + string(1, NEWLINE) +
        " / [-->] \\ " + string(1, NEWLINE) +
        "|}__{}__{}|" + string(1, NEWLINE) +
        "   \\‗≡‗/   " + string(1, NEWLINE),

        "  _‗---‗_  " + string(1, NEWLINE) +
        " / [<*>] \\ " + string(1, NEWLINE) +
        "|{}__{}__{|" + string(1, NEWLINE) +
        "   \\‗≡‗/   " + string(1, NEWLINE),
    };
    string leftSprites[4] = {

        "  _‗---‗_  " + string(1, NEWLINE) +
        " / [--<] \\ " + string(1, NEWLINE) +
        "|}__{}__{}|" + string(1, NEWLINE) +
        "   \\‗≡‗/   " + string(1, NEWLINE),

        "  _‗---‗_  " + string(1, NEWLINE) +
        " / [-<-] \\ " + string(1, NEWLINE) +
        "|_{}__{}_{|" + string(1, NEWLINE) +
        "   \\‗≡‗/   " + string(1, NEWLINE),

        "  _‗---‗_  " + string(1, NEWLINE) +
        " / [<--] \\ " + string(1, NEWLINE) +
        "|}__{}__{}|" + string(1, NEWLINE) +
        "   \\‗≡‗/   " + string(1, NEWLINE),

        "  _‗---‗_  " + string(1, NEWLINE) +
        " / [<*>] \\ " + string(1, NEWLINE) +
        "|_{}__{}_{|" + string(1, NEWLINE) +
        "   \\‗≡‗/   " + string(1, NEWLINE),
    };
    const int height = 4;
    const int width = 11;

    string deathSprites =
        "      . . . . . .       " + string(1, NEWLINE) +
        "   . '  B O O M  ' .    " + string(1, NEWLINE) +
        " .  *  *  *  *  *  *  . " + string(1, NEWLINE) +
        ". *  *  *  *  *  *  *  ." + string(1, NEWLINE) +
        " .  *  *  *  *  *  *  . " + string(1, NEWLINE) +
        "   ' .  *  *  *  . '    " + string(1, NEWLINE) +
        "      ' ' ' ' ' '       " + string(1, NEWLINE);
    const int dHeight = 7;
    const int dHidth = 24;

};

struct SpaceshipByFlyAA {
    string sprites =
        "      A      " + string(1, NEWLINE) +
        "     /0\\     " + string(1, NEWLINE) +
        "    |===|    " + string(1, NEWLINE) +
        "   /_____\\   " + string(1, NEWLINE) +
        "  |       |  " + string(1, NEWLINE) +
        "  |  [#]  |  " + string(1, NEWLINE) +
        " /|  _ _  |\\ " + string(1, NEWLINE) +
        "/_|_|_|_|_|_\\" + string(1, NEWLINE) +
        "  /_\\/_\\/_\\  " + string(1, NEWLINE) +
        "  \\/\\/|\\/\\/  " + string(1, NEWLINE) +
        "  |||||||||  " + string(1, NEWLINE) +
        "  |||||||||  " + string(1, NEWLINE) +
        "  '-------'  " + string(1, NEWLINE);

    const int height = 13;
    const int width = 14;
};

struct SpaceshipAA {
    string sprites =
        "      A      " + string(1, NEWLINE) +
        "     / \\     " + string(1, NEWLINE) +
        "    |===|    " + string(1, NEWLINE) +
        "   /_____\\   " + string(1, NEWLINE) +
        "  |       |  " + string(1, NEWLINE) +
        "  |  [ ]  |  " + string(1, NEWLINE) +
        " /|  _ _  |\\ " + string(1, NEWLINE) +
        "/_|_|_|_|_|_\\" + string(1, NEWLINE) +
        "  |=======|  " + string(1, NEWLINE) +
        "  |   |   |  " + string(1, NEWLINE) +
        "  |   o   |  " + string(1, NEWLINE) +
        "  |   o   |  " + string(1, NEWLINE) +
        "  |   |   |  " + string(1, NEWLINE) +
        "  |=======|  " + string(1, NEWLINE) +
        " /|  ___  |\\ " + string(1, NEWLINE) +
        "/_|_|_|_|_|_\\" + string(1, NEWLINE) +
        "  /_\\/_\\/_\\  ";

    const int height = 17;
    const int width = 14;
};

struct CarAA {
    string sprites =
        "   ______   " + string(1, NEWLINE) +
        " _/[] []\\\\_ " + string(1, NEWLINE) +
        "|_  ____  _|" + string(1, NEWLINE) +
        "  O      O  " ;

    const int height = 4;
    const int width = 12;
};

struct AlienAA {
    string sprites =
        "     _______     " + string(1, NEWLINE) +
        "  .-'    0  `-.  " + string(1, NEWLINE) +
        " /     0   0   \\ " + string(1, NEWLINE) +
        "|    ‗‗‗‗‗‗‗‗‗  |" + string(1, NEWLINE) +
        "|   /         \\ |" + string(1, NEWLINE) +
        " \\‗/  \\___/   |/ " + string(1, NEWLINE) +
        "   \\≡_/   \\≡_/   " + string(1, NEWLINE);


    const int height = 7;
    const int width = 17;
};

struct CityAA {
    string sprites =
        "    __   __                     ___      _           " + string(1, NEWLINE) +
        "   |=-| |--|      /|           |- -|   _/+\\_         " + string(1, NEWLINE) +
        "   |==| |--|  _  |=|__         | --|_-/+=o=+\\-_     _" + string(1, NEWLINE) +
        " __|-=| |==|_|+| |=|-=|/\\_     |- -|- \\=====/ -|___| " + string(1, NEWLINE) +
        "|--|-=| |--| |=__|=|==|---|_   |---| = |___| --| =-| " + string(1, NEWLINE) +
        "|==|==|^|--| ||- |=|=-|- -| |__| --| - |=-=| - | =-| " + string(1, NEWLINE) +
        "|==|--|||==| ||- |=|==|-- | /\\ |-- | - |===|   |  -| " + string(1, NEWLINE) +
        "≡≡≡≡≡≡≡≡≡≡≡≡≡≡≡≡≡≡≡≡≡≡≡≡≡≡≡≡≡≡≡≡≡≡≡≡≡≡≡≡≡≡≡≡≡≡≡≡≡≡≡≡≡";


    const int height = 8;
    const int width = 53;
};



struct CrosshairAA {
    string sprites[2] = {
        "  │  " + string(1, NEWLINE) +
        " ┌│┐ " + string(1, NEWLINE) +
        "──O──" + string(1, NEWLINE) +
        " └│┘ " + string(1, NEWLINE) +
        "  │  " + string(1, NEWLINE),

        "     " + string(1, NEWLINE) +
        " ╔ ╗ " + string(1, NEWLINE) +
        "- ╬ -" + string(1, NEWLINE) +
        " ╚ ╝ " + string(1, NEWLINE) +
        "     " + string(1, NEWLINE),
    };

    const int height = 5;
    const int width = 5;

};


struct Story {
    string start[7] = {
        "人類が宇宙への一歩を踏み出すため、ロケット発射の準備が整った。",
        "それは文明の新たなマイルストーン――。",
        "しかし、カウントダウン直前、空に謎の光が走り、突如現れた異星艦が宣言する。",
        "「人類に宇宙はまだ早い！」",
        "混乱の中、君は立ち上がり、防衛システムを起動！",
        "この瞬間、人類の未来は――",
        "君の手に託された！",
    };

    string goodEnd[3] = {
        "敵の母艦が墜落し、空はふたたび静けさを取り戻した。",
        "ロケットは無事に発射され、人類は歓声をあげる。",
        "この瞬間、君は歴史を変えた――人類に宇宙への扉を開いたのだ！"
    };

    string goodEndCG[23] = {
        "                       /7                                               ",
        "              ┌‐┐     //                                                ",
        "      く＼    Ｌ_｣    〈/                                                  ",
        "        ＼>          '⌒＼                                               ",
        "                   /      ヽ                                            ",
        "            ／ﾞ7⌒^ ＜       .                                           ",
        "     ﾊ _,.イ   ﾊ  ヽ  ヽ      j                                          ",
        "    {. V厶/|/ j八j八   }     /      ＜ ゲームを遊んでくれて、ありがとう！  ",
        "     '  ∨ｲ ┃    ┃ jﾉﾐ ﾘ   ｛                                            ",
        "      V/人   r‐┐ 〃＾)      八                                           ",
        "￣￣￣￣￣￣￣￣￣￣￣￣￣￣￣￣￣￣￣￣￣￣￣￣￣￣￣￣￣￣￣￣￣￣￣       ",
        "             ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;                   ",
        "          ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;                    ",
        "                ;;;;;;;;;;;;;;;;;;             ∧＿∧                     ",
        "                                             （     ）  ,",
        "                                           /⌒｀'''＾ヽ                 ",
        "                                     /⌒ヾ/  / .   ／. |                ",
        "'-―'――――――――――――――ー'''‐'ー'''―‐'―'''' ＼, /  / . / ‐'| |'''ー'-''――'   ",
        "  ,,  ''''    ｀、  ｀´'､､,        '''＿ｿ  /  ｀   ﾉ, | |  '''    ､､,    ",
        "        ,,,      ''    ,,     ''' '' ξ＿ノ  丶ー'ｰｰ＜  ,ゝ__＞  '''''    ",
        "  ,,          ,,,,          '''  ,              ;;;;;;;;;;;            ",
        "                        ,,,,,,,                           ;        ;   ",
        ",,  '''''''            ,,,,              ,     ;;;;;                   ", };

    string badEnd[6] = {
        "ロケットは爆発し、炎の玉となって消えた。",
        "宇宙人の攻撃は容赦なく、人類の希望は打ち砕かれた。",
        "宇宙への夢は目の前で崩れ去る――しかし、これはほんの始まりに過ぎない。",
        "異星の艦隊は着々と地球へ迫っている。",
        "闇の時代が来る。だが、希望の火を再び灯す者が必要だ。",
    };

    string badEndCG[14] = {
        "                                    ∧＿∧",
        "                                  （ ｀･ω･)",
        "                                 ∩（ っ¶っ¶∩",
        "                                | ノ ￣￣￣  ヽ",
        "                                /   ●      ● |      ンゴォォォォォー！！",
        "                                |    ( _●_)  ミ",
        "                                彡､     ＼＼＼  ､｀＼",
        "        （   .                _'/  ＿     ＼＼＼ /´>  )",
        "  ）.:.:().:.）                (＿＿＿）ヽ/  ＼＼＼ (_／    ｽﾞｼｰﾝ",
        "  ノ. ..:;;.;.ノ                |  ::    /     ＼＼＼",
        "（   ,.‐''~ ﾜｰ                  |  ／＼: ＼     . ＼＼＼丿ヽﾉヾ/丶",
        "（..::;ノ   )ﾉ __.         ＿＿  | /   .)  )        ヽ＼＼_ﾉヽﾉヽﾉヽ ）",
        "   ）ﾉ_'|ﾛﾛ| ／   ＼ .___. |ﾛﾛ| ／∪ ＼/ヽ )   ヽ   ＼            ）ﾉ",
        "＿|田| _|ﾛﾛ|_|   ﾛﾛ|＿|田|.|ﾛﾛ|_|   ﾛﾛ|＿           （.:;;.;;丶 丿ｿ",
    };
};
// スプライトは全オブジェクトで共有する（敵ごとに文字列をコピーしない）
struct SpriteResource {
    UFOenemyAA enemy;
    UFOBossAA boss;
    UFOBossSubAA minion;
    NumberAA number;
    SpaceshipAA spaceship;
    SpaceshipByFlyAA flyingSpaceship;
};

const SpriteResource& Sprites() {
    static const SpriteResource resource{};
    return resource;
}
#pragma endregion

#pragma region "システムとプレイヤー設定"
struct SystemSetting {
    int targetFPS = 20;
    double frameDurationMS = 1000 / targetFPS;
    int consoleWidth = CONSOLE_WIDTH;
    int consoleHeight = CONSOLE_HEIGHT;
    int screenWidth = consoleWidth - 1;
    int screenHeight = consoleHeight;
    int spriteMaxWidth = screenWidth - 2;
    int spriteMaxHeight = screenHeight - 2;

    string screenBuffer[CONSOLE_WIDTH - 1][CONSOLE_HEIGHT];
    string screenGraphic;
    string frameOutput; // 1フレーム分の出力

    int groundHeight = screenHeight - 5;
};

struct SystemVariable {
    int frame = 0;
    int playerUiOffsetX = 19;
    bool confirmKey = false;   // Enterキーが押された
    bool quitKey = false;      // Escキーが押された
    bool screenInvert = false; // 画面反白
};

struct PlayerSetting {
    int crosshairSpeed = 2;
};

struct GameSetting {
    SystemSetting sysSetting;
    PlayerSetting playerSetting;
    SystemVariable sysVal;
};


#pragma endregion

#pragma region "CmdGameEngine"
struct Point {
    int x = 0;
    int y = 0;
};

/****************************Gun****************************/
typedef enum GunType {
    LASER = 0,
    RAILGUN = 1,
} GunType;

static const string GUN_NAME[] = {
    "Laser Gun", "Railgun",
};

// 命中時の弾薬補給（通常弾 +1、弾道兵器 +2）
static const int GUN_AMMO_REFILL[] = { 1, 2 };

// 当たり判定の広さ（Laserは照準の周り1マスも当たる、Railgunは中心のみ）
static const int GUN_HIT_MARGIN[] = { 1, 0 };

struct GunSetting {
    Point leftAnchor;
    Point middleAnchor;
    Point rightAnchor;

    Point gunTopOffset;
    int xGunOffset; // クロスヘアのXオフセット
    int yGunOffset; // クロスヘアのYオフセット
    double gunXStretch = 1.0f;
};

struct Gun {
    int crX, crY;
    GunType type; // 0: Laser, 1: Railgun
    CrosshairAA crosshairAA; // クロスヘアのスプライト
    GunSetting setting; // 銃の設定
};

struct Player {
    PlayerSetting* setting;
    Gun gun;
    int hp;
    int score;
    int hitCount = 0; // ヒット数
    bool isShooting = false; // 銃を撃っているかどうか
    int ammo; // 弾薬数
};

struct Enemy {
    Point position; // 敵の位置
    int hp; // 敵のHP
    int direction;
    int score; // 倒したときのスコア
    bool isAlive = true; // 生存状態
    int moveSpeed = 1; // 移動速度
    double moveInterval = 0.15; // 移動の間隔（秒）
    double moveTimer = 0;
    double lifeTimer = 0;  // 攻撃までの残り時間
    double deathTimer = 0; // 爆発アニメーションの残り時間
};

struct Minion {
    Point position;
    double lifeTimer = MINION_LIFE; // 攻撃までの残り時間
    double deathTimer = 0;
    bool isAlive = true;
};

struct Boss {
    Point position;
    int hp = BOSS_HP;
    int direction = 1;
    double moveTimer = 0;
    double attackTimer = BOSS_ATTACK_INTERVAL; // 攻撃までの残り時間
    double weaknessTimer = 0;                  // 弱点切替周期のタイマー
    bool weaknessVisible = false;
    int weaknessIndex = 0;                     // BOSS_WEAKNESS_OFFSETSの番号
    double summonTimer = MINION_SUMMON_INTERVAL;
    bool isAlive = true;
    double deathTimer = 0;
};

// ボス左上からの弱点の位置
static const Point BOSS_WEAKNESS_OFFSETS[] = {
    { 11, 1 }, { 1, 3 }, { 6, 3 }, { 16, 3 }, { 21, 3 }, { 11, 4 }, { 6, 5 }, { 16, 5 },
};
const int BOSS_WEAKNESS_OFFSET_COUNT = sizeof(BOSS_WEAKNESS_OFFSETS) / sizeof(Point);

// 敵の弾（ロケットへのビーム）
struct Beam {
    Point from;
    Point to;
    double timer;
};

// 一時的に表示する文字（HIT、MISSなど）
struct Popup {
    string text;
    Point position;
    ConsoleColor color;
    double timer;
};

enum GamePhase {
    PHASE_TITLE,      // タイトル・ストーリー
    PHASE_PREPARE,    // 準備フェーズ
    PHASE_BOSS_INTRO, // ボス登場
    PHASE_BOSS,       // Bossフェーズ
    PHASE_LAUNCH,     // 秒読み、打ち上げ
    PHASE_DEFEAT,     // 秒読み、死亡
    PHASE_GOOD_END,
    PHASE_BAD_END,
};

struct GameState {
    GamePhase phase = PHASE_TITLE;
    double phaseTime = 0;     // フェーズ開始からの経過時間
    bool screenDrawn = false; // 静止画面を描画済みかどうか
    long long lastTick = 0;

    // 準備フェーズ
    int spawnedCount = 0;
    int killCount = 0;
    double spawnTimer = 1.0;

    // Bossフェーズ
    Boss boss;
    vector<Minion> minions;
    int minionQuota = 0;      // これから召喚する僕従の数
    bool perfect = false;     // 全機撃墜ボーナス
    bool bossStarted = false;

    // 演出
    double shotCooldown = 0;
    double flashTimer = 0;
    vector<Beam> beams;
    vector<Popup> popups;
};

struct EngineContext {
    Player player;
    vector<Enemy> enemys;
    GameState game;
    GameSetting setting;
    bool running = true;
    long long launchTime; // 起動した時刻（FPSの計算用）
    long long startTime;
    long long endTime;
    int fps;
    double gameDelay;
};

EngineContext* CmdGameInitialize(string title, int width, int height) {  
    // Allocate EngineContext on the heap instead of the stack  
    EngineContext* ec = new EngineContext();  
    ec->setting.sysSetting.consoleWidth = width;  
    ec->setting.sysSetting.consoleHeight = height;  
    ec->setting.sysSetting.screenWidth = width - 1;  
    ec->setting.sysSetting.screenHeight = height;  
    ec->setting.sysSetting.spriteMaxWidth = ec->setting.sysSetting.screenWidth - 2;  
    ec->setting.sysSetting.spriteMaxHeight = ec->setting.sysSetting.screenHeight - 2;  
    ConsoleTitle(title);
    ConsoleSetup();
    srand((unsigned int)time(NULL));
    ec->launchTime = PlatformNowMs();
    ec->game.lastTick = ec->launchTime;
    return ec;
}

// 前のフレームからの経過時間（秒）
double CmdGameDeltaTime(EngineContext& ec) {
    long long now = PlatformNowMs();
    double dt = (double)(now - ec.game.lastTick) / 1000.0;
    ec.game.lastTick = now;
    return min(dt, 0.1); // 止まっていた場合に一気に進まないようにする
}

bool CmdGameProcessMessage(EngineContext& ec) {
    ec.setting.sysVal.frame++;
    return ec.running;
}

void CmdGameBeginFrame(EngineContext& ec) {
    ec.startTime = PlatformNowMs();
}

void CmdGameEndFrame(EngineContext& ec) {
    // clock()はmacOS / LinuxではCPU時間になるので、実時間で計る
    ec.endTime = PlatformNowMs();
    double elapsed = max(1LL, ec.endTime - ec.launchTime) / 1000.0;
    ec.fps = (int)(ec.setting.sysVal.frame / elapsed);
    double frameTime = (double)(ec.endTime - ec.startTime);
    if (frameTime < ec.setting.sysSetting.frameDurationMS) {
        ec.gameDelay = ec.setting.sysSetting.frameDurationMS - frameTime;
        PlatformSleepMs((int)ec.gameDelay);
    }
}

void CmdGameFinalize(EngineContext& ec) {
    ConsoleResetColor();
    ConsoleClear();
    ConsoleCursorVisible(true);
    PlatformRestore();
}
#pragma endregion

/**************************************************/

/********************* Game ***********************/
#pragma region "Object"




//****************************Player****************************//
struct PlayerGUI {    // プレイヤーのGUI設定
    Point hpPosi; // HP表示位置
    Point scorePosi; // スコア表示位置
    Point ammoPosi; // 弾薬表示位置
    Point gunNamePosi; // 銃の位置
};



struct HandlerContext {
    Player* player;
    GameSetting* setting; // ゲーム設定
};

//****************************Environment****************************//

struct Environment {
    SpaceshipAA spaceship;
    CityAA city;
    CarAA car;
    string border;
    string ground;
};



#pragma endregion

#pragma region 道具箱
string JoinCopyStr(int count, string str) {
    string result;
    result.reserve(count * 4);
    for (int i = 0; i < count; i++) {
        result += str;
    }
    return result;
}

int RandInt(int minValue, int maxValue) {
    if (maxValue <= minValue) {
        return minValue;
    }
    return minValue + rand() % (maxValue - minValue + 1);
}

double RandRange(double minValue, double maxValue) {
    return minValue + (maxValue - minValue) * rand() / RAND_MAX;
}


#pragma endregion

#pragma region "入力関数「イベント駆動を使う」"


enum AsciiCode {
    W_KEY = 119,      // wキー
    S_KEY = 115,      // sキー
    A_KEY = 97,      // aキー
    D_KEY = 100,      // dキー
    J_KEY = 106,      // jキー
    U_KEY = 117,      // uキー
    I_KEY = 105,      // iキー
    K_KEY = 107,      // kキー
    UP_ARROW = PLATFORM_KEY_UP,       // 上矢印キー
    DOWN_ARROW = PLATFORM_KEY_DOWN,   // 下矢印キー
    LEFT_ARROW = PLATFORM_KEY_LEFT,   // 左矢印キー
    RIGHT_ARROW = PLATFORM_KEY_RIGHT, // 右矢印キー
    SPACE_BAR = 32,     // スペースキー
    ESCAPE_KEY = PLATFORM_KEY_ESCAPE, // エスケープキー
    ENTER_KEY = PLATFORM_KEY_ENTER,   // Enterキー
};

enum GameEvent {
    UP = 0,
    DOWN = 1,
    LEFT = 2,
    RIGHT = 3,
    SHOT = 4,
    TAB = 5,
    SHOT_SPACE = 6,
    CONFIRM = 7,
    QUIT = 8,
};


const int MAX_EVENT = 15;

// キーのASCIIを保存する
int eventMapping[MAX_EVENT] = { 0 };

typedef void (*KeyBoardHandler)(HandlerContext* context);

KeyBoardHandler keyBoardHandlers[MAX_EVENT] = { NULL };

// イベントを登録
void RegisterKeyboardEventHandler(int eventId, int keyCode, KeyBoardHandler handler) {
    if (eventId >= 0 && eventId <= MAX_EVENT) {
        eventMapping[eventId] = keyCode;
        keyBoardHandlers[eventId] = handler;
    }
}
// イベントを実行
void RunEventHandler(char keyCode, HandlerContext& context) {
    for (int i = 0; i < MAX_EVENT; i++) {
        if (keyBoardHandlers[i] != NULL && keyCode == eventMapping[i]) {
            keyBoardHandlers[i](&context);
            break;
        }
    }
}

// イベントたち
void UpKeyEvent(HandlerContext* context);
void DownKeyEvent(HandlerContext* context);
void LeftKeyEvent(HandlerContext* context);
void RightKeyEvent(HandlerContext* context);
void ShootEvent(HandlerContext* context);
void SwitchGunEvent(HandlerContext* context);
void ConfirmEvent(HandlerContext* context);
void QuitEvent(HandlerContext* context);

void RegisterKeyboardEvent(HandlerContext context);

void RunKeyboardEvent(HandlerContext hc);

#pragma endregion

#pragma region "描画関数"

string SpritesEnvironmentBorder(EngineContext& ec);

string SpritesEnvironmentGround(EngineContext& ec);

void WriteToBuffer(int xPos, int yPos, string characters, EngineContext& ec);

void WriteLine(Point start, Point end, EngineContext& ec, string characters = "▓");

void WirteGunSprites(Player player, EngineContext& ec);

void WirteGUI(Player player, EngineContext& ec);

void DrawCrosshair(const Player* player, EngineContext& ec);

void DrawToScreen(EngineContext& ec);

void DrawToScreenWithColour(int xPos, int yPos, ConsoleColor colour, string characters, EngineContext& ec);

void DrawScreenUI(Environment& env, EngineContext* ec, Player* player);

void DrawCountdown(int countdown, EngineContext& ec);

void DrawGameObjects(EngineContext& ec);

void DrawTitleScreen(EngineContext& ec);

void DrawEndScreen(EngineContext& ec);

void DrawGame(Environment& env, EngineContext& ec);

void FlushScreen(EngineContext& ec);

#pragma endregion

#pragma region "Action関数"

void InitPlayer(EngineContext& ec) {
    ec.player.hp = PLAYER_HP;
    ec.player.score = 0;
    ec.player.ammo = PLAYER_AMMO; // 初期弾薬数
    ec.player.gun.setting = {
        {
            ec.setting.sysSetting.screenWidth / 2 - 1, // 左アンカーX位置
            ec.setting.sysSetting.screenHeight - 2     // 左アンカーY位置
        },
        {
            ec.setting.sysSetting.screenWidth / 2,     // 中央アンカーX位置
            ec.setting.sysSetting.screenHeight - 2     // 中央アンカーY位置
        },
        {
            ec.setting.sysSetting.screenWidth / 2 + 1, // 右アンカーX位置
            ec.setting.sysSetting.screenHeight - 2     // 右アンカーY位置
        }
    };
    ec.player.gun.crX = (int)ec.setting.sysSetting.screenWidth / 2;
    ec.player.gun.crY = (int)ec.setting.sysSetting.screenHeight / 3;
    ec.player.gun.type = LASER;

    ec.player.setting = &ec.setting.playerSetting; // PlayerSettingを設定
}

Environment InitEnvGUI(EngineContext& ec) {
    Environment env;
    env.border = SpritesEnvironmentBorder(ec);
    env.ground = SpritesEnvironmentGround(ec);
    return env;
}

vector<Enemy> InitEnemyV1(EngineContext ec) {
    vector<Enemy> enemys = {};
    return enemys;
}

void GunAction(Player* player, EngineContext& ec);

void ShootBulletAction(Player* player, EngineContext& ec);

void ChangePhase(EngineContext& ec, GamePhase phase) {
    ec.game.phase = phase;
    ec.game.phaseTime = 0;
    ec.game.screenDrawn = false;
}

void StartGame(EngineContext& ec) {
    long long lastTick = ec.game.lastTick;
    ec.game = GameState();
    ec.game.lastTick = lastTick;
    ec.enemys.clear();
    InitPlayer(ec);
    ChangePhase(ec, PHASE_PREPARE);
}

void AddPopup(EngineContext& ec, string text, int x, int y, ConsoleColor color) {
    ec.game.popups.push_back({ text, { x, y }, color, 0.7 });
}

// ロケットの中心（DrawToScreenWithColourの座標）
Point RocketCenter(EngineContext& ec) {
    const SpaceshipAA& spaceship = Sprites().spaceship;
    return { spaceship.width - spaceship.width / 2 + spaceship.width / 2, ec.setting.sysSetting.groundHeight - spaceship.height / 2 };
}

// 敵の弾をロケットへ撃つ
void FireBeam(EngineContext& ec, Point from) {
    ec.game.beams.push_back({ from, RocketCenter(ec), 0.25 });
}

void DamagePlayer(EngineContext& ec, int damage) {
    if (ec.game.phase == PHASE_DEFEAT) {
        return;
    }
    ec.player.hp = max(0, ec.player.hp - damage);
    ec.game.flashTimer = 0.3;
    int x = ec.setting.sysSetting.screenWidth - ec.setting.sysVal.playerUiOffsetX;
    AddPopup(ec, "-" + to_string(damage) + " HP", x, ec.setting.sysSetting.groundHeight - 1, RED);
    if (ec.player.hp <= 0) {
        ChangePhase(ec, PHASE_DEFEAT);
    }
}

// 照準の中心が矩形に入っているかどうか
bool IsHit(const Player* player, Point position, int width, int height) {
    int margin = GUN_HIT_MARGIN[player->gun.type];
    return player->gun.crX >= position.x - margin && player->gun.crX < position.x + width + margin &&
        player->gun.crY >= position.y - margin && player->gun.crY < position.y + height + margin;
}

/****************************準備フェーズ****************************/
void SpawnEnemy(EngineContext& ec) {
    Enemy enemy;
    int maxX = ec.setting.sysSetting.screenWidth - Sprites().enemy.width - 2;
    int maxY = ec.setting.sysSetting.groundHeight - Sprites().enemy.height - 9;
    enemy.position = { RandInt(2, maxX), RandInt(3, maxY) };
    enemy.direction = rand() % 2 == 0 ? 1 : -1;
    enemy.hp = 1;
    enemy.score = ENEMY_SCORE;
    enemy.moveInterval = RandRange(0.08, 0.2);
    enemy.lifeTimer = RandRange(ENEMY_LIFE_MIN, ENEMY_LIFE_MAX);
    ec.enemys.push_back(enemy);
}

void UpdateEnemies(EngineContext& ec, double dt) {
    int maxX = ec.setting.sysSetting.screenWidth - Sprites().enemy.width - 2;
    for (Enemy& enemy : ec.enemys) {
        if (!enemy.isAlive) {
            enemy.deathTimer -= dt;
            continue;
        }
        // 生存時間が終わったら攻撃して去る
        enemy.lifeTimer -= dt;
        if (enemy.lifeTimer <= 0) {
            enemy.isAlive = false;
            FireBeam(ec, { enemy.position.x + Sprites().enemy.width / 2, enemy.position.y + Sprites().enemy.height });
            DamagePlayer(ec, ENEMY_DAMAGE);
            continue;
        }
        // 左右に移動して、壁で折り返す
        enemy.moveTimer += dt;
        while (enemy.moveTimer >= enemy.moveInterval) {
            enemy.moveTimer -= enemy.moveInterval;
            enemy.position.x += enemy.direction * enemy.moveSpeed;
            if (enemy.position.x <= 2 || enemy.position.x >= maxX) {
                enemy.position.x = max(2, min(enemy.position.x, maxX));
                enemy.direction = -enemy.direction;
            }
        }
    }
    ec.enemys.erase(remove_if(ec.enemys.begin(), ec.enemys.end(), [](const Enemy& enemy) {
        return !enemy.isAlive && enemy.deathTimer <= 0;
    }), ec.enemys.end());
}

void InitBoss(EngineContext& ec);

void UpdatePreparePhase(EngineContext& ec, double dt) {
    GameState& game = ec.game;
    game.spawnTimer -= dt;
    if (game.spawnedCount < PREP_ENEMY_TOTAL && game.spawnTimer <= 0) {
        SpawnEnemy(ec);
        game.spawnedCount++;
        game.spawnTimer += PREP_SPAWN_INTERVAL;
    }
    UpdateEnemies(ec, dt);

    // 60秒が過ぎて、残った敵がいなくなったらBoss戦へ
    if (game.phase == PHASE_PREPARE && game.phaseTime >= PREP_PHASE_TIME &&
        game.spawnedCount >= PREP_ENEMY_TOTAL && ec.enemys.empty()) {
        InitBoss(ec);
        ChangePhase(ec, PHASE_BOSS_INTRO);
    }
}

/****************************Bossフェーズ****************************/
void InitBoss(EngineContext& ec) {
    GameState& game = ec.game;
    game.boss = Boss();
    game.bossStarted = true;
    game.boss.position = { (ec.setting.sysSetting.screenWidth - Sprites().boss.width) / 2, -Sprites().boss.height };
    game.perfect = game.killCount >= PREP_ENEMY_TOTAL;
    // 撃墜数と僕従の出現数：0→20、8→16、20→10、32→4、40→0
    game.minionQuota = game.perfect ? 0 : MINION_MAX - game.killCount / 2;
    game.minions.clear();
    if (game.perfect) {
        ec.player.score += PERFECT_SCORE;
    }
}

double BossWeaknessInterval(EngineContext& ec) {
    return ec.game.perfect ? BOSS_WEAKNESS_INTERVAL_BONUS : BOSS_WEAKNESS_INTERVAL;
}

void RelocateBossWeakness(Boss& boss) {
    int next = RandInt(0, BOSS_WEAKNESS_OFFSET_COUNT - 2);
    boss.weaknessIndex = next >= boss.weaknessIndex ? next + 1 : next; // 前と同じ場所にしない
}

Point BossWeaknessPosition(Boss& boss) {
    Point offset = BOSS_WEAKNESS_OFFSETS[boss.weaknessIndex];
    return { boss.position.x + offset.x, boss.position.y + offset.y };
}

void SummonMinion(EngineContext& ec) {
    Minion minion;
    Boss& boss = ec.game.boss;
    int maxX = ec.setting.sysSetting.screenWidth - Sprites().minion.width - 3;
    int minY = boss.position.y + Sprites().boss.height + 1;
    int maxY = ec.setting.sysSetting.groundHeight - Sprites().minion.height - 2;
    minion.position = { RandInt(2, maxX), RandInt(minY, maxY) };
    ec.game.minions.push_back(minion);
}

void UpdateBossIntro(EngineContext& ec, double dt) {
    Boss& boss = ec.game.boss;
    // 上から降りてくる
    boss.position.y = min(3, -Sprites().boss.height + (int)(ec.game.phaseTime * 5));
    if (ec.game.phaseTime >= BOSS_INTRO_TIME) {
        boss.position.y = 3;
        ChangePhase(ec, PHASE_BOSS);
    }
}

void UpdateBossPhase(EngineContext& ec, double dt) {
    GameState& game = ec.game;
    Boss& boss = game.boss;

    // 爆発アニメーションが終わったら打ち上げへ
    if (!boss.isAlive) {
        boss.deathTimer += dt;
        if (boss.deathTimer >= 2.0) {
            ChangePhase(ec, PHASE_LAUNCH);
        }
        return;
    }

    // 左右に移動する
    double moveInterval = game.perfect ? 0.15 : 0.25;
    int minX = 20;
    int maxX = ec.setting.sysSetting.screenWidth - Sprites().boss.width - 3;
    boss.moveTimer += dt;
    while (boss.moveTimer >= moveInterval) {
        boss.moveTimer -= moveInterval;
        boss.position.x += boss.direction;
        if (boss.position.x <= minX || boss.position.x >= maxX) {
            boss.position.x = max(minX, min(boss.position.x, maxX));
            boss.direction = -boss.direction;
        }
    }

    // 弱点：周期の最後の2秒間だけ表示する
    double interval = BossWeaknessInterval(ec);
    boss.weaknessTimer += dt;
    if (boss.weaknessTimer >= interval) {
        boss.weaknessTimer -= interval;
    }
    bool visible = boss.weaknessTimer >= interval - BOSS_WEAKNESS_SHOW;
    if (visible && !boss.weaknessVisible) {
        RelocateBossWeakness(boss);
    }
    boss.weaknessVisible = visible;

    // 本体攻撃
    boss.attackTimer -= dt;
    if (boss.attackTimer <= 0) {
        boss.attackTimer = BOSS_ATTACK_INTERVAL;
        FireBeam(ec, { boss.position.x + Sprites().boss.width / 2, boss.position.y + Sprites().boss.height });
        DamagePlayer(ec, BOSS_DAMAGE);
    }

    // 僕従の召喚
    if (game.minionQuota > 0) {
        boss.summonTimer -= dt;
        if (boss.summonTimer <= 0) {
            boss.summonTimer += MINION_SUMMON_INTERVAL;
            int count = min(RandInt(1, 2), game.minionQuota);
            for (int i = 0; i < count; i++) {
                SummonMinion(ec);
            }
            game.minionQuota -= count;
        }
    }

    // 僕従は8秒後に攻撃して消える
    for (Minion& minion : game.minions) {
        if (!minion.isAlive) {
            minion.deathTimer -= dt;
            continue;
        }
        minion.lifeTimer -= dt;
        if (minion.lifeTimer <= 0) {
            minion.isAlive = false;
            FireBeam(ec, { minion.position.x + Sprites().minion.width / 2, minion.position.y + Sprites().minion.height });
            DamagePlayer(ec, MINION_DAMAGE);
        }
    }
    game.minions.erase(remove_if(game.minions.begin(), game.minions.end(), [](const Minion& minion) {
        return !minion.isAlive && minion.deathTimer <= 0;
    }), game.minions.end());
}

void DefeatBoss(EngineContext& ec) {
    Boss& boss = ec.game.boss;
    boss.isAlive = false;
    boss.deathTimer = 0;
    boss.weaknessVisible = false;
    ec.game.minions.clear();
    ec.game.minionQuota = 0;
    ec.player.score += BOSS_SCORE + ec.player.hp * HP_BONUS_SCORE;
}

// 照準の先にいる目標に当たったかどうか
bool HitTarget(Player* player, EngineContext& ec) {
    GameState& game = ec.game;
    int refill = GUN_AMMO_REFILL[player->gun.type];
    string refillText = "+" + to_string(refill) + " AMMO";

    if (game.phase == PHASE_BOSS && game.boss.isAlive) {
        Boss& boss = game.boss;
        const UFOBossAA& sprite = Sprites().boss;
        // 弱点に命中すると、攻撃のクールタイムが即時リセットされる
        if (boss.weaknessVisible && IsHit(player, BossWeaknessPosition(boss), sprite.weaknessWidth, sprite.weaknessHeight)) {
            Point position = BossWeaknessPosition(boss);
            boss.hp--;
            boss.attackTimer = BOSS_ATTACK_INTERVAL;
            player->score += WEAKNESS_SCORE;
            AddPopup(ec, "CRITICAL " + refillText, position.x, position.y + sprite.weaknessHeight, YELLOW);
            if (boss.hp <= 0) {
                DefeatBoss(ec);
            } else {
                RelocateBossWeakness(boss);
            }
            return true;
        }
        for (Minion& minion : game.minions) {
            if (minion.isAlive && IsHit(player, minion.position, Sprites().minion.width, Sprites().minion.height)) {
                minion.isAlive = false;
                minion.deathTimer = 0.5;
                player->score += MINION_SCORE;
                AddPopup(ec, refillText, minion.position.x, minion.position.y - 1, GREEN);
                return true;
            }
        }
        if (IsHit(player, boss.position, sprite.width, sprite.height)) {
            AddPopup(ec, "BLOCKED", player->gun.crX + 3, player->gun.crY, WHITE);
            return false;
        }
    }

    for (Enemy& enemy : ec.enemys) {
        if (enemy.isAlive && IsHit(player, enemy.position, Sprites().enemy.width, Sprites().enemy.height)) {
            enemy.isAlive = false;
            enemy.deathTimer = 0.5;
            game.killCount++;
            player->score += enemy.score;
            AddPopup(ec, refillText, enemy.position.x, enemy.position.y - 1, GREEN);
            return true;
        }
    }

    AddPopup(ec, "MISS", player->gun.crX + 3, player->gun.crY, RED);
    return false;
}

void UpdateEffects(EngineContext& ec, double dt) {
    GameState& game = ec.game;
    game.shotCooldown = max(0.0, game.shotCooldown - dt);
    game.flashTimer = max(0.0, game.flashTimer - dt);
    ec.setting.sysVal.screenInvert = game.flashTimer > 0;
    for (Beam& beam : game.beams) {
        beam.timer -= dt;
    }
    game.beams.erase(remove_if(game.beams.begin(), game.beams.end(), [](const Beam& beam) {
        return beam.timer <= 0;
    }), game.beams.end());
    for (Popup& popup : game.popups) {
        popup.timer -= dt;
    }
    game.popups.erase(remove_if(game.popups.begin(), game.popups.end(), [](const Popup& popup) {
        return popup.timer <= 0;
    }), game.popups.end());
}

void UpdateGame(EngineContext& ec, double dt) {
    GameState& game = ec.game;
    SystemVariable& sysVal = ec.setting.sysVal;
    game.phaseTime += dt;

    switch (game.phase) {
    case PHASE_TITLE:
        if (sysVal.confirmKey) {
            StartGame(ec);
        }
        break;
    case PHASE_PREPARE:
        UpdatePreparePhase(ec, dt);
        break;
    case PHASE_BOSS_INTRO:
        UpdateBossIntro(ec, dt);
        break;
    case PHASE_BOSS:
        UpdateBossPhase(ec, dt);
        break;
    case PHASE_LAUNCH:
        // 3秒の秒読みの後、ロケットが飛び去る
        if (game.phaseTime >= 7.0) {
            ChangePhase(ec, PHASE_GOOD_END);
        }
        break;
    case PHASE_DEFEAT:
        if (game.phaseTime >= 2.5) {
            ChangePhase(ec, PHASE_BAD_END);
        }
        break;
    case PHASE_GOOD_END:
    case PHASE_BAD_END:
        if (sysVal.confirmKey) {
            ChangePhase(ec, PHASE_TITLE);
        }
        break;
    }

    // 射撃は準備フェーズとBossフェーズのみ
    GunAction(&ec.player, ec);
    if (game.phase == PHASE_PREPARE || game.phase == PHASE_BOSS) {
        ShootBulletAction(&ec.player, ec);
    }
    ec.player.isShooting = false;

    UpdateEffects(ec, dt);
}



#pragma endregion
/**************************************************/




int main() {
    EngineContext* ec = CmdGameInitialize(author, CONSOLE_WIDTH, CONSOLE_HEIGHT);
    ConsoleCursorVisible(false);

    InitPlayer(*ec);
    HandlerContext context;
    context.player = &ec->player;                       
    context.setting = &ec->setting;

    Environment env = InitEnvGUI(*ec);

    RegisterKeyboardEvent(context);

    while (CmdGameProcessMessage(*ec)) {
        CmdGameBeginFrame(*ec);
        double dt = CmdGameDeltaTime(*ec);

        // Keyboardイベントを実行
        RunKeyboardEvent(context);
        if (ec->setting.sysVal.quitKey) {
            ec->running = false;
        }

        // Actionを実行する
        UpdateGame(*ec, dt);
        ec->setting.sysVal.confirmKey = false;

        // 描画する
        DrawGame(env, *ec);

        string title = author + " FPS:" + to_string(ec->fps);
        ConsoleTitle(title);

        CmdGameEndFrame(*ec);
    }
    CmdGameFinalize(*ec);
    delete ec;
    return 0;
}

void DrawGame(Environment& env, EngineContext& ec) {
    GamePhase phase = ec.game.phase;
    // 静止画面は1回だけ描画する
    if (phase == PHASE_TITLE || phase == PHASE_GOOD_END || phase == PHASE_BAD_END) {
        if (!ec.game.screenDrawn) {
            ec.game.screenDrawn = true;
            if (phase == PHASE_TITLE) {
                DrawTitleScreen(ec);
            } else {
                DrawEndScreen(ec);
            }
        }
        return;
    }

    // UIを描画する
    DrawScreenUI(env, &ec, &ec.player);
    DrawGameObjects(ec);
    if (phase != PHASE_LAUNCH && phase != PHASE_DEFEAT) {
        DrawCrosshair(&ec.player, ec);
    }
    // 1フレーム分の出力をまとめて書き出す
    FlushScreen(ec);
}

void DrawScreenUI(Environment& env, EngineContext* ec, Player* player) {
    WriteToBuffer(0, 0, env.border, *ec);
    WriteToBuffer(1, ec->setting.sysSetting.groundHeight, env.ground, *ec);
    // 打ち上げ後と爆発中はロケットを地上に描かない
    GamePhase phase = ec->game.phase;
    bool rocketOnGround = !(phase == PHASE_LAUNCH && ec->game.phaseTime >= 3.0) && phase != PHASE_DEFEAT;
    if (rocketOnGround) {
        WriteToBuffer(env.spaceship.width - env.spaceship.width / 2, ec->setting.sysSetting.groundHeight - env.spaceship.height, env.spaceship.sprites, *ec);
    }
    // 敵の弾（バッファの座標は描画座標より1小さい）
    for (const Beam& beam : ec->game.beams) {
        WriteLine({ beam.from.x - 1, beam.from.y - 1 }, { beam.to.x - 1, beam.to.y - 1 }, *ec, "░");
    }
    WriteToBuffer(ec->setting.sysSetting.screenWidth - (env.city.width + 1), ec->setting.sysSetting.groundHeight - env.city.height, env.city.sprites, *ec);
    WriteToBuffer(ec->setting.sysSetting.screenWidth - (env.city.width + env.car.width + 4), ec->setting.sysSetting.groundHeight - env.car.height, env.car.sprites, *ec);

    WirteGunSprites(*player, *ec);
    WirteGUI(*player, *ec);

    WriteToBuffer(ec->setting.sysSetting.screenWidth - ec->setting.sysVal.playerUiOffsetX + 8, ec->setting.sysSetting.groundHeight, GUN_NAME[player->gun.type], *ec);
    WriteToBuffer(ec->setting.sysSetting.screenWidth - ec->setting.sysVal.playerUiOffsetX + 8, ec->setting.sysSetting.groundHeight + 1, JoinCopyStr(player->hp, "❚"), *ec);
    WriteToBuffer(ec->setting.sysSetting.screenWidth - ec->setting.sysVal.playerUiOffsetX + 8, ec->setting.sysSetting.groundHeight + 2, JoinCopyStr(player->ammo, "❚"), *ec);
    WriteToBuffer(ec->setting.sysSetting.screenWidth - ec->setting.sysVal.playerUiOffsetX + 8, ec->setting.sysSetting.groundHeight + 3, to_string(player->score), *ec);
    DrawToScreen(*ec);
}

void RegisterKeyboardEvent(HandlerContext hc) {
    RegisterKeyboardEventHandler(UP, W_KEY, UpKeyEvent);
    RegisterKeyboardEventHandler(DOWN, S_KEY, DownKeyEvent);
    RegisterKeyboardEventHandler(LEFT, A_KEY, LeftKeyEvent);
    RegisterKeyboardEventHandler(RIGHT, D_KEY, RightKeyEvent);
    // space ascii code
    RegisterKeyboardEventHandler(SHOT, J_KEY, ShootEvent);
    RegisterKeyboardEventHandler(TAB, K_KEY, SwitchGunEvent);
    RegisterKeyboardEventHandler(SHOT_SPACE, SPACE_BAR, ShootEvent);
    RegisterKeyboardEventHandler(CONFIRM, ENTER_KEY, ConfirmEvent);
    RegisterKeyboardEventHandler(QUIT, ESCAPE_KEY, QuitEvent);
}

void RunKeyboardEvent(HandlerContext hc) {
    // 溜まったキーを全部処理する（1フレーム1キーだと入力が遅れる）
    while (PlatformKeyHit()) {
        int key = PlatformReadKey();
        // 方向キーはWASDと同じイベントにする
        switch (key) {
        case UP_ARROW:    key = W_KEY; break;
        case DOWN_ARROW:  key = S_KEY; break;
        case LEFT_ARROW:  key = A_KEY; break;
        case RIGHT_ARROW: key = D_KEY; break;
        case PLATFORM_KEY_NONE: continue;
        default: break;
        }
        RunEventHandler((char)tolower(key), hc);
    }
}

// 命中した場合のみ弾薬を消費せず、弾薬を回復する。外れたら1発消費する
void ShootBulletAction(Player* player, EngineContext& ec) {
    if (!player->isShooting || ec.game.shotCooldown > 0) {
        return;
    }
    if (player->ammo <= 0) {
        AddPopup(ec, "NO AMMO", player->gun.crX + 3, player->gun.crY, RED);
        return;
    }
    ec.game.shotCooldown = SHOT_COOLDOWN;
    if (HitTarget(player, ec)) {
        player->hitCount++;
        player->ammo = min(player->ammo + GUN_AMMO_REFILL[player->gun.type], PLAYER_AMMO_MAX);
    } else {
        player->ammo--;
    }
}

void GunAction(Player* player, EngineContext& ec) {
    double theta = atan2(player->gun.setting.middleAnchor.y - player->gun.crY, player->gun.setting.middleAnchor.x - player->gun.crX);
    player->gun.setting.xGunOffset = -(int)floor(cos(theta) * GUN_LENGTH); // 初期オフセット
    player->gun.setting.yGunOffset = -(int)floor(sin(theta) * GUN_LENGTH); // 初期オフセット
    player->gun.setting.gunTopOffset = {
        (int)(player->gun.setting.xGunOffset * player->gun.setting.gunXStretch),
        player->gun.setting.yGunOffset
    };
}

string SpritesEnvironmentBorder(EngineContext& ec) {
    // Allocate strings on the heap instead of the stack  
    string* topBorder = new string("╔" + JoinCopyStr(ec.setting.sysSetting.spriteMaxWidth, "═") + "╗" + string(1, NEWLINE));  
    string* middleBorder = new string("║" + JoinCopyStr(ec.setting.sysSetting.spriteMaxWidth, " ") + "║" + string(1, NEWLINE));  
    string* bottomBorder = new string("╚" + JoinCopyStr(ec.setting.sysSetting.spriteMaxWidth, "═") + "╝");  

    // Combine the strings into a single result  
    string allBorder = *topBorder + JoinCopyStr(ec.setting.sysSetting.spriteMaxHeight, *middleBorder) + *bottomBorder;  

    // Free the allocated memory  
    delete topBorder;  
    delete middleBorder;  
    delete bottomBorder;  

    return allBorder;  
}

string SpritesEnvironmentGround(EngineContext& ec) {
    string* topGround = new string(JoinCopyStr(ec.setting.sysSetting.spriteMaxWidth - ec.setting.sysVal.playerUiOffsetX, "═") + string(1, NEWLINE));
    string* middleGround = new string(JoinCopyStr(ec.setting.sysSetting.spriteMaxWidth - ec.setting.sysVal.playerUiOffsetX, "▂") + string(1, NEWLINE));
    string* bottomGround = new string(JoinCopyStr(ec.setting.sysSetting.spriteMaxWidth - ec.setting.sysVal.playerUiOffsetX, "▇"));
    string allGround = *topGround + *middleGround + *middleGround  + *bottomGround;

    delete topGround;
    delete middleGround;
    delete bottomGround;

    return allGround;
}


void WirteGunSprites(Player player, EngineContext& ec) {
    for (int i = player.gun.setting.leftAnchor.x; i <= player.gun.setting.rightAnchor.x; i++) {
        Point gunBottomOffset = { i, player.gun.setting.middleAnchor.y };
        Point newGunOffset = {
            gunBottomOffset.x + player.gun.setting.gunTopOffset.x,
            gunBottomOffset.y + player.gun.setting.gunTopOffset.y
        };
        WriteLine(gunBottomOffset, newGunOffset, ec);
    }
}



void WirteGUI(Player player, EngineContext& ec) {
    int x = ec.setting.sysSetting.screenWidth - ec.setting.sysVal.playerUiOffsetX - 1;
    int y = ec.setting.sysSetting.groundHeight - 1;

    string* topFrame =    new string("╔" + JoinCopyStr(ec.setting.sysVal.playerUiOffsetX - 1, "═") + "╣" + string(1, NEWLINE));
    string* gunFrame =    new string("║   Gun:           " + string(1, NEWLINE)); // 仮のHP
    string* hpFrame =     new string("║    HP:           " + string(1, NEWLINE)); // 仮のHP
    string* ammoFrame =   new string("║  Ammo:           " + string(1, NEWLINE));
    string* scoreFrame =  new string("║ Score:           " + string(1, NEWLINE));
    string* bottomFrame = new string("╩" + JoinCopyStr(ec.setting.sysVal.playerUiOffsetX - 1, "═") + "╝");

    string allFrames = *topFrame + *gunFrame + *hpFrame + *ammoFrame + *scoreFrame + *bottomFrame;
    WriteToBuffer(x, y, allFrames, ec);
}

// ロケットの秒読み（大きい数字で表示する）
void DrawCountdown(int countdown, EngineContext& ec) {
    const NumberAA& number = Sprites().number;
    countdown = max(0, min(countdown, 99));
    ConsoleColor color = countdown <= 10 ? RED : WHITE;
    DrawToScreenWithColour(3, 2, color, number.sprites[countdown / 10], ec);
    DrawToScreenWithColour(3 + number.width, 2, color, number.sprites[countdown % 10], ec);
}

void DrawGameObjects(EngineContext& ec) {
    const SpriteResource& sprites = Sprites();
    GameState& game = ec.game;
    int frame = ec.setting.sysVal.frame;
    int animeframe = frame % 20 / 5;
    bool blink = frame / 3 % 2 == 0;
    int centerX = ec.setting.sysSetting.screenWidth / 2;

    // 準備フェーズの秒読み
    if (game.phase == PHASE_PREPARE) {
        DrawCountdown((int)ceil(max(0.0, PREP_PHASE_TIME - game.phaseTime)), ec);
        DrawToScreenWithColour(3, 7, WHITE, "KILL " + to_string(game.killCount) + "/" + to_string(PREP_ENEMY_TOTAL), ec);
    }

    // 敵機
    for (const Enemy& enemy : ec.enemys) {
        if (enemy.isAlive) {
            // 攻撃の1秒前は充填中のスプライトで点滅する
            bool charging = enemy.lifeTimer < 1.0;
            const string* animation = enemy.direction > 0 ? sprites.enemy.rightSprites : sprites.enemy.leftSprites;
            ConsoleColor color = charging && blink ? RED : GREEN;
            DrawToScreenWithColour(enemy.position.x, enemy.position.y, color, animation[charging ? 3 : animeframe % 3], ec);
        } else if (enemy.deathTimer > 0) {
            DrawToScreenWithColour(enemy.position.x - 6, enemy.position.y - 1, YELLOW, sprites.enemy.deathSprites, ec);
        }
    }

    // ボス
    bool bossVisible = game.bossStarted && game.phase != PHASE_LAUNCH;
    if (bossVisible) {
        const Boss& boss = game.boss;
        if (boss.isAlive) {
            bool charging = game.phase == PHASE_BOSS && boss.attackTimer < 1.5;
            DrawToScreenWithColour(boss.position.x, boss.position.y, charging && blink ? RED : MAGENTA, sprites.boss.sprites[animeframe], ec);
            if (boss.weaknessVisible) {
                Point position = BossWeaknessPosition(game.boss);
                DrawToScreenWithColour(position.x, position.y, blink ? YELLOW : CYAN, sprites.boss.weakness, ec);
            }
        } else {
            int deathFrame = min(3, (int)(boss.deathTimer / 0.4));
            DrawToScreenWithColour(boss.position.x, boss.position.y, deathFrame == 3 ? YELLOW : RED, sprites.boss.deathSprites[deathFrame], ec);
        }

        // ボスのHP
        if (game.phase == PHASE_BOSS) {
            int bar = (boss.hp + 1) / 2;
            DrawToScreenWithColour(3, 2, WHITE, "BOSS HP " + to_string(boss.hp) + "/" + to_string(BOSS_HP), ec);
            DrawToScreenWithColour(3, 3, RED, string(bar, '#'), ec);
            DrawToScreenWithColour(3 + bar, 3, WHITE, string(BOSS_HP / 2 - bar, '.'), ec);
            if (game.minionQuota > 0 || !game.minions.empty()) {
                DrawToScreenWithColour(3, 4, YELLOW, "MINION " + to_string(game.minionQuota + (int)game.minions.size()), ec);
            }
        }
    }

    // 僕従
    for (const Minion& minion : game.minions) {
        if (minion.isAlive) {
            bool charging = minion.lifeTimer < 2.0;
            DrawToScreenWithColour(minion.position.x, minion.position.y, charging && blink ? RED : YELLOW, sprites.minion.sprites, ec);
            DrawToScreenWithColour(minion.position.x + 5, minion.position.y + sprites.minion.height, RED, to_string((int)ceil(minion.lifeTimer)), ec);
        } else if (minion.deathTimer > 0) {
            DrawToScreenWithColour(minion.position.x - 6, minion.position.y - 1, YELLOW, sprites.enemy.deathSprites, ec);
        }
    }

    // ボス登場
    if (game.phase == PHASE_BOSS_INTRO) {
        if (blink) {
            DrawToScreenWithColour(centerX - 7, 14, RED, "!! WARNING !!", ec);
        }
        DrawToScreenWithColour(centerX - 13, 16, WHITE, "MOTHERSHIP IS APPROACHING", ec);
        if (game.perfect) {
            DrawToScreenWithColour(centerX - 14, 18, CYAN, "PERFECT! 40/40 - NO MINIONS", ec);
        } else {
            DrawToScreenWithColour(centerX - 8, 18, YELLOW, "MINIONS: " + to_string(game.minionQuota), ec);
        }
    }

    // 秒読み、打ち上げ
    if (game.phase == PHASE_LAUNCH) {
        Point rocket = RocketCenter(ec);
        if (game.phaseTime < 3.0) {
            int count = 3 - (int)game.phaseTime;
            DrawToScreenWithColour(centerX - sprites.number.width / 2, 8, YELLOW, sprites.number.sprites[count], ec);
        } else {
            const SpaceshipByFlyAA& ship = sprites.flyingSpaceship;
            int groundY = ec.setting.sysSetting.groundHeight - ship.height + 1;
            int y = groundY - (int)((game.phaseTime - 3.0) * 8);
            DrawToScreenWithColour(rocket.x - ship.width / 2, y, WHITE, ship.sprites, ec);
            DrawToScreenWithColour(centerX - 5, 10, CYAN, "LIFT OFF!", ec);
        }
    }

    // 秒読み、死亡（ロケットの爆発）
    if (game.phase == PHASE_DEFEAT) {
        Point rocket = RocketCenter(ec);
        DrawToScreenWithColour(rocket.x - 12, rocket.y - 3, blink ? RED : YELLOW, sprites.enemy.deathSprites, ec);
        DrawToScreenWithColour(centerX - 10, 10, RED, "THE ROCKET IS DOWN...", ec);
    }

    for (const Popup& popup : game.popups) {
        DrawToScreenWithColour(popup.position.x, popup.position.y, popup.color, popup.text, ec);
    }
}

// 日本語の文章はそのまま出力する（全角文字は2マス使うため、バッファを通さない）
void PrintLines(int x, int y, const string* lines, int count) {
    for (int i = 0; i < count; i++) {
        ConsoleSetCursorPosition(x, y + i);
        OutPrint(lines[i]);
    }
}

void DrawTitleScreen(EngineContext& ec) {
    Story story;
    ConsoleClear();
    ConsoleSetForegroundColor(CYAN);
    PrintLines(6, 3, story.start, 7);

    static const string howToPlay[] = {
        "【操作方法】",
        "  W/A/S/D・方向キー : 照準の移動",
        "  J / SPACE          : 発射",
        "  K                  : 武器の切替（Laser：当たり判定が広い ／ Railgun：命中時 弾薬+2）",
        "",
        "【ルール】",
        "  命中すれば弾薬は減らず回復する（Laser +1 ／ Railgun +2）。外すと1発消費する。",
        "  準備フェーズ（60秒）：敵機40機が1.5秒ごとに出現し、3〜7秒後に攻撃してくる（-1HP）。",
        "  Bossフェーズ：弱点を撃つとBossの攻撃（-3HP）が遅れる。僕従は8秒後に攻撃してくる（-2HP）。",
        "  全機撃墜でBoss戦に僕従が出現しない。",
    };
    ConsoleSetForegroundColor(WHITE);
    PrintLines(6, 12, howToPlay, sizeof(howToPlay) / sizeof(string));

    ConsoleSetForegroundColor(YELLOW);
    ConsoleSetCursorPosition(6, 25);
    OutPrint("ENTER : スタート    ESC : 終了");
    ConsoleSetForegroundColor(WHITE);
    fflush(stdout);
}

void DrawEndScreen(EngineContext& ec) {
    Story story;
    bool good = ec.game.phase == PHASE_GOOD_END;
    string result = "SCORE: " + to_string(ec.player.score) +
        "    撃墜: " + to_string(ec.game.killCount) + "/" + to_string(PREP_ENEMY_TOTAL) +
        "    命中: " + to_string(ec.player.hitCount) +
        "    残りHP: " + to_string(ec.player.hp);

    ConsoleClear();
    if (good) {
        ConsoleSetForegroundColor(CYAN);
        PrintLines(4, 1, story.goodEnd, 3);
        ConsoleSetForegroundColor(YELLOW);
        PrintLines(4, 5, &result, 1);
        ConsoleSetForegroundColor(WHITE);
        PrintLines(4, 6, story.goodEndCG, 23);
    } else {
        ConsoleSetForegroundColor(RED);
        PrintLines(4, 1, story.badEnd, 5);
        ConsoleSetForegroundColor(YELLOW);
        PrintLines(4, 7, &result, 1);
        ConsoleSetForegroundColor(WHITE);
        PrintLines(4, 9, story.badEndCG, 14);
    }
    ConsoleSetForegroundColor(YELLOW);
    ConsoleSetCursorPosition(4, ec.setting.sysSetting.screenHeight);
    OutPrint("ENTER : タイトルへ    ESC : 終了");
    ConsoleSetForegroundColor(WHITE);
    fflush(stdout);
}

void DrawCrosshair(const Player* player, EngineContext& ec) {
    int x = player->gun.crX - player->gun.crosshairAA.height / 2;
    int y = player->gun.crY - player->gun.crosshairAA.width / 2;
    DrawToScreenWithColour(x, y, ec.game.shotCooldown > 0 ? CYAN : BLUE, player->gun.crosshairAA.sprites[player->gun.type], ec);
}

void WriteLine(Point start, Point end, EngineContext& ec, string characters) {
    int x = start.x;
    int y = start.y;
    int dx = abs(start.x - end.x);
    int dy = -abs(start.y - end.y);
    int sx = start.x < end.x ? 1 : -1;
    int sy = start.y < end.y ? 1 : -1;
    int error = dx + dy;
    while (true) {
        WriteToBuffer(x, y, characters, ec); // ░▒▓█
        if (x == end.x && y == end.y) {
            return;
        }
        int error2 = error * 2;
        if (error2 >= dy) {
            if (x == end.x) {
                break;
            }
            error += dy;
            x += sx;
        }
        if (error2 <= dx) {
            if (y == end.y) {
                break;
            }
            error += dx;
            y += sy;
        }
    }
}

void DrawToScreen(EngineContext& ec) {
    SystemSetting& sys = ec.setting.sysSetting;
    string& graphic = sys.screenGraphic;
    graphic.clear();
    graphic.reserve(sys.screenWidth * sys.screenHeight * 3);
    graphic += "\x1b[H";
    // 敵の攻撃を受けたとき、画面を反白にする
    if (ec.setting.sysVal.screenInvert) {
        graphic += "\x1b[7m";
    }
    for (int y = 0; y < sys.screenHeight; y++) {
        for (int x = 0; x < sys.screenWidth; x++) {
            const string& cell = sys.screenBuffer[x][y];
            if (cell.empty() || cell[0] == NULL_CHAR) {
                graphic += ' ';
            } else {
                graphic += cell;
            }
        }
        if (y < sys.screenHeight - 1) {
            graphic += NEWLINE;
        }
    }
    if (ec.setting.sysVal.screenInvert) {
        graphic += "\x1b[27m";
    }
    // 画面はすぐに出力せず、フレームの最後にまとめて出力する
    sys.frameOutput = graphic;
}

// UTF-8の1文字のバイト数
int Utf8CharLength(unsigned char lead) {
    if (lead < 0x80) return 1;
    if ((lead & 0xE0) == 0xC0) return 2;
    if ((lead & 0xF0) == 0xE0) return 3;
    if ((lead & 0xF8) == 0xF0) return 4;
    return 1;
}

void WriteToBuffer(int xPos, int yPos, string characters, EngineContext& ec) {
    int x = xPos;
    int y = yPos;
    // 1文字ずつ（UTF-8のまま）バッファに書き込む
    for (size_t i = 0; i < characters.length();) {
        int length = Utf8CharLength((unsigned char)characters[i]);
        if (characters[i] == NEWLINE) {
            y++;
            x = xPos;
        } else {
            if (x >= 0 && y >= 0 && x < ec.setting.sysSetting.screenWidth && y < ec.setting.sysSetting.screenHeight) {
                ec.setting.sysSetting.screenBuffer[x][y].assign(characters, i, length);
            }
            x++;
        }
        i += length;
    }
}

void DrawToScreenWithColour(int xPos, int yPos, ConsoleColor colour, string characters, EngineContext& ec) {
    int x = xPos;
    int y = yPos;
    string& out = ec.setting.sysSetting.frameOutput;
    out += "\x1b[" + to_string(BOLD) + ';' + to_string(colour) + 'm';

    // 前に書いた文字のすぐ右なら、カーソル移動を省略する
    int cursorX = -1;
    int cursorY = -1;
    for (size_t i = 0; i < characters.length();) {
        int length = Utf8CharLength((unsigned char)characters[i]);
        char lead = characters[i];
        i += length;

        if (lead == NEWLINE) {
            y++;
            x = xPos;
            continue;
        }
        if (lead == SPACE) {
            x++;
            continue;
        }

        // 外枠の内側だけに描画する
        if (x >= 2 && x < ec.setting.sysSetting.screenWidth &&
            y >= 2 && y < ec.setting.sysSetting.screenHeight) {
            if (x != cursorX || y != cursorY) {
                out += "\x1b[" + to_string(y) + ';' + to_string(x) + 'H';
            }
            if (EMPTY_CHAR_SW && lead == EMPTY_CHAR) {
                out += ' ';
            } else {
                out.append(characters, i - length, length);
            }
            cursorX = x + 1;
            cursorY = y;
        }
        x++;
    }
    out += "\x1b[" + to_string(BOLD) + ';' + to_string(WHITE) + 'm';
}

// 1フレーム分の出力を1回で書き出す（ちらつきと出力回数を減らす）
void FlushScreen(EngineContext& ec) {
    string& out = ec.setting.sysSetting.frameOutput;
    fwrite(out.data(), 1, out.size(), stdout);
    fflush(stdout);
    out.clear();
}

void UpKeyEvent(HandlerContext* context) {
    context->player->gun.crY -= context->player->setting->crosshairSpeed;
    context->player->gun.crY = max(context->player->gun.crY, 2); // クロスヘアが画面外に出ないようにする
}
void DownKeyEvent(HandlerContext* context) {
    context->player->gun.crY += context->player->setting->crosshairSpeed;
    context->player->gun.crY = min(context->player->gun.crY, (context->setting->sysSetting.screenHeight - context->player->gun.crosshairAA.height)); // クロスヘアが画面外に出ないようにする
}
void LeftKeyEvent(HandlerContext* context) {
    context->player->gun.crX -= context->player->setting->crosshairSpeed;
    context->player->gun.crX = max(context->player->gun.crX, 2);
}
void RightKeyEvent(HandlerContext* context) {
    context->player->gun.crX += context->player->setting->crosshairSpeed;
    context->player->gun.crX = min(context->player->gun.crX, (context->setting->sysSetting.screenWidth - context->player->gun.crosshairAA.width + 3));
}
void ShootEvent(HandlerContext* context) {
    context->player->isShooting = true;
}

void SwitchGunEvent(HandlerContext* context) {
    context->player->gun.type = (context->player->gun.type == LASER) ? RAILGUN : LASER; // 銃の切り替え
}

void ConfirmEvent(HandlerContext* context) {
    context->setting->sysVal.confirmKey = true;
}

void QuitEvent(HandlerContext* context) {
    context->setting->sysVal.quitKey = true;
}