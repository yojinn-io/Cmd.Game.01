#include <stdio.h>
#include <string>

#define WIDTH  80
#define HEIGHT 25

using namespace std;

void FakeClearScreen() {
    for (int i = 0; i < HEIGHT; ++i) {
        for (int j = 0; j < WIDTH; ++j) {
            printf(" ");
        }
        printf("\n");
    }
    // 最後にカーソルを先頭に戻す（ANSIが使える場合）
    printf("\033[H");
}
void Wait5() {
    for (int i = 0; i < 5000000; i++) {
        printf("");
    }
}

void Test() {
    for (int x = 0; x < 5; x++) {
        // test image
        for (int i = 0; i < HEIGHT; ++i) {
            for (int j = 0; j < WIDTH; ++j) {
                if (x % 2 == 0)
                    printf("*");
                else
                    printf("&");
            }
            printf("\n");
        }
        printf("\033[H");
        getchar();

        FakeClearScreen();
    }
    FakeClearScreen();
}

struct UFOBoss {
    string sprites[8] = {
        "            ___            ",
        "          .'   '.          ",
        "     _.-'/   |   \\'-._     ",
        " _.-'   |__/___\\__|   '-._ ",
        "|==[@]==[@]==[@]==[@]==[@]|",
        "'-._     \\_______/     _.-'",
        "    '-._     _     _.-'    ",
        "        '----'----'        ",
    };
    const int height = 8;
    const int width = 27;
    int Weakness[28] = {};
};


struct Spaceship {
    string sprites[17] = {
        "         A         ",
        "        / \\        ",
        "       |===|       ",
        "      /_____\\      ",
        "     |       |     ",
        "     |  [ ]  |     ",
        "    /|  _ _  |\\    ",
        "   /_|_|_|_|_|_\\   ",
        "     |=======|     ",
        "     |   |   |     ",
        "     |   o   |     ",
        "     |   o   |     ",
        "     |   |   |     ",
        "     |=======|     ",
        "    /|  ___  |\\    ",
        "   /_|_|_|_|_|_\\   ",
        "     /_\\/_\\/_\\     ",
    };
    const int height = 17;
    const int width = 19;
};

int main() {
    Spaceship ss;
    int len = sizeof(ss.sprites) / sizeof(string);
    for (int i = 0; i < len; i++) {
        printf("%d\n", ss.sprites[i].size());
    }
    
   

}