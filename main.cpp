#include <stdio.h>
#include <string>
#include <time.h>
#include <conio.h>
#include <windows.h>
#include <math.h>
#include <vector>

#include <cwchar>
#include <clocale>
#include <codecvt>
#include <algorithm>


/*
* ANSI Escape Sequences 参考 =>  https://gist.github.com/fnky/458719343aabd01cfb17a3a4f7296797
* Hexadecimal を使う
* conio.h キーボード
*/

/*****************************
todo list
敵-普通
- [ ] 敵の攻撃を実装する
- [ ] 敵の弾を実装する
- [ ] 敵の爆発を実装する
- [ ] 敵の移動を実装する


敵ーボス
- [ ] ボス戦を発生する
- [ ] ボスの攻撃を実装する
- [ ] ボスのHPを実装する
- [ ] ボスを実装する
- [ ] ボスの弱点を実装する
- [ ] ボスの弱点を攻撃して倒す
- [ ] ボスの弱点を攻撃して倒したら、敵のボスの爆発アニメーションを実装する
- [ ] ボスを倒したら、エンディングを表示する
- [ ] ボスがざこと召喚する
- [ ] ざこの攻撃を実装する
- [ ] ざこの爆発を実装する

Enviroment
- [x] 地形を実装する
- [x] Crosshairを実装する
- [x] spaceshipを実装する
- [x] UIを実装する
- [x] UIのFPSを実装する
- [ ] 秒読み、打ち上げ
- [ ] 秒読み、死亡
- [ ] 秒読みを実装する
- [ ] 秒読みのUIを実装する
- [ ] 秒読みのアニメーションを実装する
- [x] 武器を実装する
- [ ] 武器の弾を実装する
- [ ] 敵のAttackするとき、画面反白を実装する
- [ ] 敵撃っていたの判定

*****************************/

using namespace std;

const int CONSOLE_WIDTH = 120;
const int CONSOLE_HEIGHT = 30;
const bool EMPTY_CHAR_SW = false;
const int GUN_LENGTH = 5;
const int PLAYER_HP = 9;
const int PLAYER_AMMO = 7;


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

#pragma region "Console システム関数"
void RunCmd(string cmd) {
    printf(cmd.c_str());
}

void OutPrint(string str) {
    printf("%s", str.c_str());
}

void OutPrintln(string str) {
    printf("%s\n", str.c_str());
}

void ConsoleCursorVisible(bool flag) {
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_CURSOR_INFO info = { 100, flag };// カーソルを非表示にする
    SetConsoleCursorInfo(hOut, &info);
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
    RunCmd("\x1b[H");
    for (int i = 0; i < CONSOLE_HEIGHT; ++i) {
        string clear = string(CONSOLE_WIDTH, ' ');
        printf(clear.c_str() + '\n');
    }
    // 最後にカーソルを先頭に戻す（ANSIが使える場合）
    RunCmd("\x1b[H");
}

void ConsoleTitle(string title) {
    SetConsoleTitleA(title.c_str());
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
        "それは文明の新たなマイルストーン――。"
        "しかし、カウントダウン直前、空に謎の光が走り、突如現れた異星艦が宣言する。"
        "「人類に宇宙はまだ早い！」"
        "混乱の中、君は立ち上がり、防衛システムを起動！"
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

    int groundHeight = screenHeight - 5;
};

struct SystemVariable {
    int frame = 0;
    int playerUiOffsetX = 19;

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
    int x;
    int y;
};

/****************************Gun****************************/
typedef enum GunType {
    LASER = 0,
    RAILGUN = 1,
} GunType;

static const string GUN_NAME[] = {
    "Laser Gun", "Railgun",
};

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
    UFOenemyAA sprite; // UFOのスプライト
    int hp; // 敵のHP
    int direction;
    int score; // 倒したときのスコア
    bool isAlive = true; // 生存状態
    int moveSpeed = 1; // 移動速度
};




struct EngineContext {
    Player player;
    vector<Enemy> enemys;
    //vector<Bullet> bullets;
    GameSetting setting;
    bool running = true;
    clock_t startTime;
    clock_t endTime;
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
    SetConsoleTitleA(title.c_str());  
    return ec;  
}

bool CmdGameProcessMessage(EngineContext& ec) {
    ec.setting.sysVal.frame++;
    return ec.running;
}

void CmdGameBeginFrame(EngineContext& ec) {
    ec.startTime = clock();
}

void CmdGameEndFrame(EngineContext& ec) {
    ec.endTime = clock();
    ec.fps = (int)(ec.setting.sysVal.frame / ((double)clock() / CLOCKS_PER_SEC));
    double frameTime = (double)(ec.endTime - ec.startTime) * 1000 / CLOCKS_PER_SEC;
    if (frameTime < ec.setting.sysSetting.frameDurationMS) {
        ec.gameDelay = ec.setting.sysSetting.frameDurationMS - frameTime;
        Sleep((int)ec.gameDelay);
    }
}

void CmdGameFinalize(EngineContext& ec) {
    
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

// 全角文字列の変換
string ConvertStr(string str) {
    wstring_convert<codecvt_utf8_utf16<wchar_t>> converterTo16;
    wstring_convert<codecvt_utf8<wchar_t> > converterTo8;
    wstring tmpStrTo16 = converterTo16.from_bytes(str);
    string rStr = "";
    for (int i = 0; i < tmpStrTo16.length(); i++) {
        rStr += converterTo8.to_bytes(tmpStrTo16[i]);
    }
    return rStr;
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
    UP_ARROW = 72,      // 上矢印キー
    DOWN_ARROW = 80,    // 下矢印キー
    LEFT_ARROW = 75,    // 左矢印キー
    RIGHT_ARROW = 77,   // 右矢印キー
    SPACE_BAR = 32,     // スペースキー
    ESCAPE_KEY = 27,    // エスケープキー
    ENTER_KEY = 13,     // Enterキー
};

enum GameEvent {
    UP = 0,
    DOWN = 1,
    LEFT = 2,
    RIGHT = 3,
    SHOT = 4,
    TAB = 5
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
        if (keyCode == eventMapping[i]) {
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

void RegisterKeyboardEvent(HandlerContext context);

void RunKeyboardEvent(HandlerContext hc);

#pragma endregion

#pragma region "描画関数"

string SpritesEnvironmentBorder(EngineContext ec);

string SpritesEnvironmentGround(EngineContext ec);

void WriteToBuffer(int xPos, int yPos, string characters, EngineContext& ec);

void WriteLine(Point start, Point end, EngineContext& ec);

void WirteGunSprites(Player player, EngineContext& ec);

void WirteGUI(Player player, EngineContext& ec);

void DrawCrosshair(const Player* player, EngineContext ec);

void DrawToScreen(EngineContext& ec);

void DrawToScreenWithColour(int xPos, int yPos, ConsoleColor colour, string characters, EngineContext ec);

void DrawScreenUI(Environment& env, EngineContext* ec, Player* player);

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

Environment InitEnvGUI(EngineContext ec) {
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
    
    int countdown = 10;

    while (CmdGameProcessMessage(*ec)) {
        CmdGameBeginFrame(*ec);

        // Keyboardイベントを実行
        RunKeyboardEvent(context);


        // Actionを実行

        int animeframe = ec->setting.sysVal.frame % 20 / 5;

        if (countdown == 0) {
            countdown = 10;
        }
        if (ec->setting.sysVal.frame % ec->setting.sysSetting.targetFPS == 0) {
            countdown--;
        }

        //
        //DrawToScreenWithColour(10, 20, RED, to_string(ec.gameSetting.val.frame), ec);
        
        // UIを描画する
        DrawScreenUI(env, ec, context.player);
            
        // Actionを実行する
        {
            GunAction(context.player, *ec);
            ShootBulletAction(context.player, *ec);
        }

        
        {   
            ///////////////////// Test
            UFOenemyAA ue;
            UFOBossAA ub;
            UFOBossSubAA ubs;
            
            DrawToScreenWithColour(64, 8, RED, ub.sprites[animeframe], *ec);
            DrawToScreenWithColour(50, 6, GREEN, ue.leftSprites[animeframe], *ec);
            DrawToScreenWithColour(50, 12, YELLOW, ubs.sprites, *ec);
            ////////////////////////////////


            // User Interfaceを描画する
            // int x = ec->setting.sysSetting.screenWidth - ec->setting.sysVal.playerUiOffsetX + 9;
            // int y = ec->setting.sysSetting.groundHeight + 2;
            // DrawToScreenWithColour(x, y++, RED, "❚❚❚❚❚❚❚", *ec);
            // DrawToScreenWithColour(x, y + 2, MAGENTA, "❚❚❚❚❚❚❚", *ec);
        }
        
        // Draw Crosshair
        DrawCrosshair(context.player, *ec);
        
        string title = author + " FPS:" + to_string(ec->fps);
        ConsoleTitle(title);

        CmdGameEndFrame(*ec);
    }
    CmdGameFinalize(*ec);
    return 0;
}

void DrawScreenUI(Environment& env, EngineContext* ec, Player* player) {
    WriteToBuffer(0, 0, env.border, *ec);
    WriteToBuffer(1, ec->setting.sysSetting.groundHeight, env.ground, *ec);
    WriteToBuffer(env.spaceship.width - env.spaceship.width / 2, ec->setting.sysSetting.groundHeight - env.spaceship.height, env.spaceship.sprites, *ec);
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
}

void RunKeyboardEvent(HandlerContext hc) {
    // キーが押されたかどうかチェック
    if (_kbhit()) {
        RunEventHandler(_getch(), hc);
    }
}

void ShootBulletAction(Player* player, EngineContext& ec) {
    if (player->isShooting && player->ammo > 0) {
        player->ammo--;
    }
    player->isShooting = false; // 銃を撃った後は、撃っていない状態に戻す
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

string SpritesEnvironmentBorder(EngineContext ec) {
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

string SpritesEnvironmentGround(EngineContext ec) {
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

// ロケットの秒読み
void DrawCountdown(int countdown, EngineContext ec) {
    DrawToScreenWithColour(2, 2, RED, to_string(countdown), ec);
}

void DrawCrosshair(const Player* player, EngineContext ec) {
    int x = player->gun.crX - player->gun.crosshairAA.height / 2;
    int y = player->gun.crY - player->gun.crosshairAA.width / 2;
    DrawToScreenWithColour(x, y, player->isShooting ? CYAN : BLUE, player->gun.crosshairAA.sprites[player->gun.type], ec);
}

void WriteLine(Point start, Point end, EngineContext& ec) {
    int x = start.x;
    int y = start.y;
    int dx = abs(start.x - end.x);
    int dy = -abs(start.y - end.y);
    int sx = start.x < end.x ? 1 : -1;
    int sy = start.y < end.y ? 1 : -1;
    int error = dx + dy;
    while (true) {
        WriteToBuffer(x, y, "▓", ec); // ░▒▓█
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
    for (int y = 0; y < ec.setting.sysSetting.screenHeight; y++) {
        for (int x = 0; x < ec.setting.sysSetting.screenWidth; x++) {
            if (ec.setting.sysSetting.screenBuffer[x][y] == string(1, NULL_CHAR)) {
                ec.setting.sysSetting.screenGraphic += (' ');
            } else {
                ec.setting.sysSetting.screenGraphic += ec.setting.sysSetting.screenBuffer[x][y];
            }
        }
        if (y < ec.setting.sysSetting.screenHeight - 1) {
            ec.setting.sysSetting.screenGraphic += string(1, NEWLINE);
        }
    }
    ConsoleSetCursorPosition(0, 0);
    OutPrint(ec.setting.sysSetting.screenGraphic);

    
    for (int i = 0; i < CONSOLE_WIDTH - 1; i++) {
        ec.setting.sysSetting.screenBuffer[i]->clear();
    }
    
    ec.setting.sysSetting.screenGraphic.clear(); // Clear the screen graphic buffer
}

void WriteToBuffer(int xPos, int yPos, string characters, EngineContext& ec) {
    int x = xPos;
    int y = yPos;
    wstring_convert<codecvt_utf8_utf16<wchar_t>> converterTo16;
    wstring_convert<codecvt_utf8<wchar_t> > converterTo8;
    wstring wide = converterTo16.from_bytes(characters);
    for (int i = 0; i < wide.length(); i++) {
        string charStr = converterTo8.to_bytes(wide[i]);
        if (charStr == string(1, NEWLINE)) {
            y++;
            x = xPos;
        } else if (x < 0 || y < 0 || x >= ec.setting.sysSetting.screenWidth || y >= ec.setting.sysSetting.screenHeight) {
            x++;
        } else {
            ec.setting.sysSetting.screenBuffer[x][y] = charStr;
            x++;
        }
    }
}

void DrawToScreenWithColour(int xPos, int yPos, ConsoleColor colour, string characters, EngineContext ec) {
    int x = xPos;
    int y = yPos;
    ConsoleSetForegroundColor(colour);
    wstring_convert<codecvt_utf8_utf16<wchar_t>> converterTo16;
    wstring_convert<codecvt_utf8<wchar_t> > converterTo8;
    wstring wide = converterTo16.from_bytes(characters);

    for (int i = 0; i < wide.length(); i++) {
        string charStr = converterTo8.to_bytes(wide[i]);
        
        if (charStr == string(1, NEWLINE)) {
            y++;
            x = xPos;
            continue;
        }

        if (charStr.find_first_not_of(' ') == string::npos) {
            x++;
            continue;
        }

        if (x >= 1 && x < ec.setting.sysSetting.screenWidth - 1 &&
            y >= 1 && y < ec.setting.sysSetting.screenHeight - 1) {
            if (EMPTY_CHAR_SW && charStr == string(1, EMPTY_CHAR)) {
                ConsoleSetCursorPosition(x, y);
                OutPrint(" ");
            } else {
                ConsoleSetCursorPosition(x, y);
                OutPrint(charStr);
            }
        }
        x++;
    }
    ConsoleSetForegroundColor(WHITE);
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