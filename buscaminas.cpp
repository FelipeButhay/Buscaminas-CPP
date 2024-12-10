#include "raylib.h"
#include <iostream>
#include <cstdlib>
#include <string>
#include <cstring>

#define BACKGROUND (Color){50,50,50,255}

std::string intToStr(int num){
    std::string str = std::to_string(num);
    return str;
}

char* strToChar(std::string& str){
    char* cstr = new char[str.length() + 1];
    std::strcpy(cstr, str.c_str());
    return cstr;
}

std::string cronometro(int time0){
    std::string cronometro = "00:00:00";

    int time_s = GetTime() - time0; 

    int hours   = time_s/3600;
    int minutes = (time_s/60)%60;
    int seconds = time_s%60;

    cronometro[0] = hours / 10 + 48;
    cronometro[1] = hours % 10 + 48;
    cronometro[3] = minutes / 10 + 48;
    cronometro[4] = minutes % 10 + 48;
    cronometro[6] = seconds / 10 + 48;
    cronometro[7] = seconds % 10 + 48;

    return cronometro;
}

class Game{
    private:
        int tilesx, tilesy, nbombs, bombsLeft, nflags;
        int sx, sy, sq, gap;
        int time0; // tiempo en que inicia la partida
        bool zeroInBoard;

        // 0 = menu, 1 = jugando, 2 = lose, 3 = win,
        unsigned int stage;
        Texture2D sprites[10];

        // tilesx = (gameSize + 1)*5, tilesy = ...*3
        unsigned int gameSize;
        // atom, nano, very small, small, medium, large, vey large, enormous, observable 
        
        // lo declaro para el tamaño mas grande del tablero y despues uso lo que necesito
        bool boardInitialized;
        int board[45][27];
        // 0 - 8 son numeros, 10 bomba, 11 bomba explotada 
        // (se asigna unicamente si el jugador explota una bomba con el fin de mostrarla en el gameover)

        int boardPlayer[45][27];
        // 0 - 8 son numeros, 9 bandera, 10 bomba, 11 bomba explotada, 12 bomba desactivada, -1 es desconocido

        std::string crono;

    void loopMenu(){
        if(IsMouseButtonPressed(0)) {
            int mx = GetMouseX(), my = GetMouseY();
            if ((mx > sy*0.05 && mx < sy*0.15) && (my > sy*0.5 && my < sy*0.6)){
                this->gameSize--;
            }
            if ((mx > sy*0.17 && mx < sy*0.27) && (my > sy*0.5 && my < sy*0.6)){
                this->gameSize++;
            }
            if ((mx > sx-sy*0.6 && mx < sx-sy*0.1) && (my > sy*0.4 && my < sy*0.9)){
                this->stage = 1;
                this->time0 = GetTime();

                this->tilesx = (gameSize%8 + 1)*5;
                this->tilesy = (gameSize%8 + 1)*3;
                this->nbombs = tilesx * tilesy / 5;
                this->bombsLeft = nbombs;
                this->nflags = 0;

                this->zeroInBoard = true;
                this->boardInitialized = false;

                this->sq = gap * 8 / tilesy;
            }
        }
    }

    void drawMenu(){
        ClearBackground(BACKGROUND);
        DrawText("Minesweeper", sy*0.05, sy*0.05, sy*0.2, WHITE);
        DrawText("Click the mine to play", sy*0.05, sy*0.25, sy*0.08, GRAY);

        Vector2 LogoPos;
        LogoPos.x = sx-sy*0.6;
        LogoPos.y = sy*0.4;
        DrawCircle(sx-sy*0.35, sy*0.65, sy*0.25, WHITE);
        DrawTextureEx(sprites[9], LogoPos, 0, sy*0.5/1000.0, WHITE);

        DrawRectangle(sy*0.05, sy*0.5, sy*0.1, sy*0.1, GRAY);
        DrawText("-", sy*0.08, sy*0.505, sy*0.1, WHITE);

        DrawRectangle(sy*0.17, sy*0.5, sy*0.1, sy*0.1, GRAY);
        DrawText("+", sy*0.195, sy*0.505, sy*0.1, WHITE);

        // MANTENER LA CANTINDA DE TAMAÑOS COMO POTENCIA DE DOS (2,4,8,16,32...) si no salen en cualquier orden
        std::string sizeName[8] = {"Nano", "Micro", "Small", "Medium", "Large", "Enormous", "Colossal", "Endless"};
        DrawText(strToChar(sizeName[gameSize%8]), sy*0.05, sy*0.7, sy*0.15, WHITE);
    }

    void generateBoard(int exceptX, int exceptY){
        srand(time(0));
        int nbombsLocal = nbombs;

        while (nbombsLocal > 0){
            int bombPosX = std::rand() % tilesx;
            int bombPosY = std::rand() % tilesy;
            if (board[bombPosX][bombPosY] == 10  
                || (bombPosX == exceptX   && bombPosY == exceptY  )
                || (bombPosX == exceptX+1 && bombPosY == exceptY-1)
                || (bombPosX == exceptX+1 && bombPosY == exceptY  )
                || (bombPosX == exceptX+1 && bombPosY == exceptY+1)
                || (bombPosX == exceptX   && bombPosY == exceptY+1)
                || (bombPosX == exceptX-1 && bombPosY == exceptY+1)
                || (bombPosX == exceptX-1 && bombPosY == exceptY  )
                || (bombPosX == exceptX-1 && bombPosY == exceptY-1)
                || (bombPosX == exceptX   && bombPosY == exceptY-1)){ // MEJOR PONGAMOS QUE EXCEPT XY SEA 0
                continue;
            } else {
                this->board[bombPosX][bombPosY] = 10;
                nbombsLocal--;
            }
        }

        int i, j;
        for(i = 0; i<tilesx; i++){
            for(j = 0; j<tilesy; j++){
                if(board[i][j] == 10) continue;
                if(i < tilesx              ) {if(board[i+1][j  ] == 10) this->board[i][j]++;}
                if(i < tilesx && j < tilesy) {if(board[i+1][j+1] == 10) this->board[i][j]++;}
                if(              j < tilesy) {if(board[i  ][j+1] == 10) this->board[i][j]++;}
                if(i > 0      && j < tilesy) {if(board[i-1][j+1] == 10) this->board[i][j]++;}
                if(i > 0                   ) {if(board[i-1][j  ] == 10) this->board[i][j]++;}
                if(i > 0      && j > 0     ) {if(board[i-1][j-1] == 10) this->board[i][j]++;}
                if(              j > 0     ) {if(board[i  ][j-1] == 10) this->board[i][j]++;}
                if(i < tilesx && j > 0     ) {if(board[i+1][j-1] == 10) this->board[i][j]++;}
            }
        }
    }

    void loopGame(){
        int mx = GetMouseX(), my = GetMouseY();
        int sqPx, sqPy;
        crono = cronometro(time0);

        if (zeroInBoard && boardInitialized){
            this->zeroInBoard = cascadeReveal();
        }

        if (bombsLeft == 0 && nbombs - nflags == 0){
            this->stage = 3;

            for (int i = 0; i<tilesx; i++){
                for (int j = 0; j<tilesy; j++){
                    if(boardPlayer[i][j] == -1){
                        this->boardPlayer[i][j] = board[i][j]; 
                    }
                    if(boardPlayer[i][j] == 9 && board[i][j] == 10){
                        this->boardPlayer[i][j] = 12;
                    }
                }
            }
        }

        if (IsMouseButtonPressed(0) && !boardInitialized){
            if ((mx > gap/2 && mx < (gap/2 + sq*tilesx)) && (my > gap && my < (gap + sq*tilesy))) {
                sqPx = (mx - gap/2) / sq;
                sqPy = (my - gap) / sq;
                generateBoard(sqPx, sqPy);
                
                this->boardPlayer[sqPx][sqPy] = board[sqPx][sqPy];
                this->boardInitialized = true;
            }
        }

        if (IsMouseButtonPressed(0) && boardInitialized) {
            if ((mx > gap/2 && mx < (gap/2 + sq*tilesx)) && (my > gap && my < (gap + sq*tilesy))) {
                sqPx = (mx - gap/2) / sq;
                sqPy = (my - gap) / sq;

                if (boardPlayer[sqPx][sqPy] == -1) {
                    if (board[sqPx][sqPy] == 10) { // es una bomba
                        this->stage = 2;
                        this->board[sqPx][sqPy] = 11;

                    for (int i = 0; i<tilesx; i++){
                        for (int j = 0; j<tilesy; j++){
                            if(boardPlayer[i][j] == -1){
                                this->boardPlayer[i][j] = board[i][j];
                            }
                            if(boardPlayer[i][j] == 9){
                                if(board[i][j] == 10){
                                    this->boardPlayer[i][j] = 12;
                                } else {
                                    this->boardPlayer[i][j] = board[i][j];
                                }
                            }
                        }
                    }

                    } else { // all numbers
                        this->boardPlayer[sqPx][sqPy] = board[sqPx][sqPy];
                        if (board[sqPx][sqPy] == 0) this->zeroInBoard = true;
                    }
                }
            }
        }
        if (IsMouseButtonPressed(1) && boardInitialized) {
            if ((mx > gap/2 && mx < (gap/2 + sq*tilesx)) && (my > gap && my < (gap + sq*tilesy))) {
                sqPx = (mx - gap/2) / sq;
                sqPy = (my - gap) / sq;

                if (boardPlayer[sqPx][sqPy] == -1) {
                    this->nflags++;
                    this->boardPlayer[sqPx][sqPy] = 9;

                    if (board[sqPx][sqPy] == 10){
                        this->bombsLeft--;
                    }
                } else if (boardPlayer[sqPx][sqPy] == 9) {
                    this->nflags--;
                    this->boardPlayer[sqPx][sqPy] = -1;

                    if (board[sqPx][sqPy] == 10){
                        this->bombsLeft++;
                    }
                }
            }
        }
    }

    void backToMenu(){
        int mx = GetMouseX(), my = GetMouseY();

        if (IsMouseButtonPressed(0)) {
            if ((mx > gap + sq*tilesx && mx < (gap + sq*tilesx + sx-gap*1.5-tilesx*sq)) && (my > sy*0.6 && my < (sy*0.7))) {
                this->stage = 0;
                this->gameSize = 0;

                for (int i = 0; i<45; i++){
                    for (int j = 0; j<27; j++){
                        this->board[i][j] = 0;
                        this->boardPlayer[i][j] = -1;
                    }
                }
            }
        }
    }

    bool cascadeReveal(){
        bool zeroFound = false;

        for (int i = 0; i<tilesx; i++){
            for (int j = 0; j<tilesy; j++){
                if(boardPlayer[i][j] != 0) continue;
                
                zeroFound = true;

                if(i < tilesx              ) this->boardPlayer[i+1][j  ] = board[i+1][j  ];
                if(i < tilesx && j < tilesy) this->boardPlayer[i+1][j+1] = board[i+1][j+1];
                if(              j < tilesy) this->boardPlayer[i  ][j+1] = board[i  ][j+1];
                if(i > 0      && j < tilesy) this->boardPlayer[i-1][j+1] = board[i-1][j+1];
                if(i > 0                   ) this->boardPlayer[i-1][j  ] = board[i-1][j  ];
                if(i > 0      && j > 0     ) this->boardPlayer[i-1][j-1] = board[i-1][j-1];
                if(              j > 0     ) this->boardPlayer[i  ][j-1] = board[i  ][j-1];
                if(i < tilesx && j > 0     ) this->boardPlayer[i+1][j-1] = board[i+1][j-1];
            }
        }

        return zeroFound;
    }

    void drawGame(){
        ClearBackground(BACKGROUND);

        int i, j;
        std::string numberOfBombs = intToStr(nbombs - nflags);
        DrawText(strToChar(crono), gap + sq*tilesx, gap, sy*0.075, WHITE);
        DrawText("Bombs", gap + sq*tilesx, gap*2.5, sy*0.1, WHITE);
        DrawText("Left:", gap + sq*tilesx, gap*3.4, sy*0.1, WHITE);
        DrawText(strToChar(numberOfBombs), gap + sq*tilesx, gap*4.4, sy*0.12, WHITE);

        if (stage == 2 || stage ==3){
            DrawRectangle(gap + sq*tilesx, sy*0.6, sx-gap*1.5-tilesx*sq, sy*0.1, GRAY);
            DrawText("Menu", gap + sq*tilesx+ sy*0.02, sy*0.605, sy*0.1, WHITE);
        }

        if (stage == 2){
            DrawText("BOOM!", gap + sq*tilesx, sy-2*gap, sy*0.1, RED);
        } else if (stage == 3){
            DrawText("WIN!", gap + sq*tilesx, sy-2*gap, sy*0.1, GREEN);
        }

        for (i = 0; i<tilesx; i++){
            for (j = 0; j<tilesy; j++){
                switch (boardPlayer[i][j]){ // 0 - 8 son numeros, 9 bandera, 10 bomba, 11 bomba explotada, -1 es desconocido
                    case  0: DrawRectangle(gap/2.0 + i*sq, gap + j*sq, sq, sq, BACKGROUND); break;
                    case -1: DrawRectangle(gap/2.0 + i*sq, gap + j*sq, sq, sq, ORANGE); break;
                    // case  1: case  2: case  3: case  4: case  5: case  6: case  7: case  8:
                    //     DrawTextureEx(sprites[board[i][j]], (Vector2){(float)gap + i*sq, (float)gap + j*sq}, 0, sq/1000, WHITE); break;
                    case  9: DrawTextureEx(sprites[ 0], (Vector2){(float)gap/2.0f + i*sq, (float)gap + j*sq}, 0, sq/1000.0, WHITE); break;
                    case 10: 
                        DrawRectangle(gap/2.0 + i*sq, gap + j*sq, sq, sq, ORANGE); 
                        DrawTextureEx(sprites[ 9], (Vector2){(float)gap/2.0f + i*sq, (float)gap + j*sq}, 0, sq/1000.0, WHITE); break;
                    case 12: 
                        DrawRectangle(gap/2.0 + i*sq, gap + j*sq, sq, sq, GREEN); 
                        DrawTextureEx(sprites[ 9], (Vector2){(float)gap/2.0f + i*sq, (float)gap + j*sq}, 0, sq/1000.0, WHITE); break;
                    case 11: 
                        DrawRectangle(gap/2.0 + i*sq, gap + j*sq, sq, sq, RED); 
                        DrawTextureEx(sprites[ 9], (Vector2){(float)gap/2.0f + i*sq, (float)gap + j*sq}, 0, sq/1000.0, WHITE); break;
                    default: DrawTextureEx(sprites[boardPlayer[i][j]], (Vector2){(float)gap/2.0f + i*sq, (float)gap + j*sq}, 0, sq/1000.0, WHITE); break;
                }
            } 
        }
    }

    public:
    void loop(){
        switch (stage){
            case 0: loopMenu(); break;
            case 1: loopGame(); break;
            case 2: backToMenu(); break; // que solo te deje volver al menu
            case 3: backToMenu(); break; // same
        }
    }

    void draw(){
        switch (stage){
            case 0: drawMenu(); break;
            case 1: drawGame(); break;
            case 2: drawGame(); break;
            case 3: drawGame(); break;
        }
    }

    Game(){
        // int monitor = GetCurrentMonitor();

        // this->sx = GetMonitorWidth(monitor);
        // this->sy = GetMonitorHeight(monitor);

        this->sx = 1920;
        this->sy = 1080;

        this->gap = sy/10;

        InitWindow(sx, sy, "BuscaMinas");
        SetWindowPosition(0,0);
        SetTargetFPS(60);

        this->sprites[0]  = LoadTexture("sprites/minesweeper/flag.png");
        this->sprites[1]  = LoadTexture("sprites/minesweeper/one.png");
        this->sprites[2]  = LoadTexture("sprites/minesweeper/two.png");
        this->sprites[3]  = LoadTexture("sprites/minesweeper/three.png");
        this->sprites[4]  = LoadTexture("sprites/minesweeper/four.png");
        this->sprites[5]  = LoadTexture("sprites/minesweeper/five.png");
        this->sprites[6]  = LoadTexture("sprites/minesweeper/six.png");
        this->sprites[7]  = LoadTexture("sprites/minesweeper/seven.png");
        this->sprites[8]  = LoadTexture("sprites/minesweeper/eight.png");
        this->sprites[9]  = LoadTexture("sprites/minesweeper/mine.png");

        this->stage = 0;
        this->gameSize = 0;

        for (int i = 0; i<45; i++){
            for (int j = 0; j<27; j++){
                this->board[i][j] = 0;
                this->boardPlayer[i][j] = -1;
            }
        }
    }

    void unload(){
        for (int i = 0; i<10; i++){
            UnloadTexture(sprites[i]);
        }
    }
};

int main(){
    Game game;

    while(!WindowShouldClose()){
        game.loop();
        BeginDrawing();
            game.draw();
        EndDrawing();
    }

    game.unload();
	return 0;
}