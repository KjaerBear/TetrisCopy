#define SDL_MAIN_HANDLED
#include "SDL.h"
#include <vector>
#include <iostream>
//Add VC++ Drectories Include FIle
//C++ Include FIles to SDL
// Linker General Include x86 File
// Additional Include: (Input Section) SDL2.lib + SDL2main.lib

#define WIDTH 308
#define HEIGHT 600
#define TILE_SIZE 22;

using std::vector;

bool running;
bool canDrop = true;

SDL_Renderer* renderer;
SDL_Window* window;

int frameCount, timerFPS, lastFrame, fps;
bool left, right, up, down;

struct block {
    SDL_Color color;
    bool active;
};
struct shape {
    SDL_Color color;
    bool matrix[4][4];
    int x, y;
    int size;
};
vector < shape > pastBlocks;
block gameGrid[27][14];
void initGameGrid() {
    for (int i = 0; i < 27; i++) {
        for (int j = 0; j < 14; j++) {
            gameGrid[i][j] = { {150, 0, 0}, false };
        }
    }
}
shape blocks[7] = { {{255,165,0},       // in order: color, matrix, xpos, ypos, size
{{0,0,1,0} // L BLOCK
,{1,1,1,0}
,{0,0,0,0}
,{0,0,0,0}
},5,-3,3}
,{{255,0,0}, // Z BLOCK
{{1,1,0,0}
,{0,1,1,0}
,{0,0,0,0}
,{0,0,0,0}
},5,-2,3}
,{{224,255,255}, // I BLOCK
{{1,1,1,1}
,{0,0,0,0}
,{0,0,0,0}
,{0,0,0,0}
},5,-5,4}
,{{0,0,255}, // J BLOCK
{{1,0,0,0}
,{1,1,1,0}
,{0,0,0,0}
,{0,0,0,0}
},5,-3,3}
,{{255,255,0}, // O BLOCK
{{1,1,0,0}
,{1,1,0,0}
,{0,0,0,0}
,{0,0,0,0}
},5,-2,2}
,{{0,0,255}, // S BLOCK
{{0,1,1,0}
,{1,1,0,0}
,{0,0,0,0}
,{0,0,0,0}
},5,-2,3}
,{{128,0,128}, // T BLOCK
{{0,1,0,0}
,{1,1,1,0}
,{0,0,0,0}
,{0,0,0,0}
},5,-4,3} }, cur;
//Rotation
shape reverseCols(shape s) {
    shape tmp = s;
    for (int i = 0; i < s.size; i++) {
        for (int j = 0; j < s.size / 2; j++) {
            bool t = s.matrix[i][j];
            tmp.matrix[i][j] = s.matrix[i][s.size - j - 1];
            tmp.matrix[i][s.size - j - 1] = t;
        }
    }
    return tmp;
}
shape transpose(shape s) {
    shape tmp = s;
    for (int i = 0; i < s.size; i++) {
        for (int j = 0; j < s.size; j++) {
            tmp.matrix[i][j] = s.matrix[j][i];
        }
    }
    return tmp;
}

bool canRotate() { 
    //Checks the boundries of the shape that about to be rotated
    //Returns true if it can be rotated and false if it will be out of bounds
    shape tempCur = reverseCols(transpose(cur));
    for (int i = 0; i < tempCur.size; i++) {
        for (int j = 0; j < tempCur.size; j++) {
            if(tempCur.matrix[i][j]) {
                if (tempCur.x + i > 13 || tempCur.x + i < 0) {
                    return false;
                }
            }

        }
    }
    return true;
}
void rotate() {
    if (canRotate()) {
        cur = reverseCols(transpose(cur));
    }
}

SDL_Rect rect;

//Drawing the board
void drawCur(shape s) {
    for (int i = 0; i < s.size; i++) {
        for (int j = 0; j < s.size; j++) {
            if (s.matrix[i][j]) {
                rect.x = (s.x + i) * TILE_SIZE; rect.y = (s.y + j) * TILE_SIZE;
                SDL_SetRenderDrawColor(renderer, s.color.r, s.color.g, s.color.b, 255);
                SDL_RenderFillRect(renderer, &rect);
                SDL_SetRenderDrawColor(renderer, 219, 219, 219, 255);
                SDL_RenderDrawRect(renderer, &rect);
            }
        }
    }
}
void drawGrid() {
    for (int i = 0; i < 27; i++) {
        for (int j = 0; j < 14; j++) {
            if (gameGrid[i][j].active) {
                rect.x = (j) * TILE_SIZE; rect.y = (i) * TILE_SIZE;
                SDL_SetRenderDrawColor(renderer, gameGrid[i][j].color.r, gameGrid[i][j].color.g, gameGrid[i][j].color.b, 255);
                SDL_RenderFillRect(renderer, &rect);
                SDL_SetRenderDrawColor(renderer, 219, 219, 219, 255);
                SDL_RenderDrawRect(renderer, &rect);
            }
        }
    }
}
void copyToGamegrid() {
    for (int i = 0; i < 4;i++) {
        for (int j = 0; j < 4; j++) {          
            if (cur.matrix[i][j]) {               
                gameGrid[cur.y + j][cur.x + i].active = true;
                gameGrid[cur.y + j][cur.x + i].color = cur.color;
            }
        }
    }
}

//Testing Collision
bool testLeft() {
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            if (cur.matrix[i][j]) {
                if (cur.x + i < 1) {
                    return false;
                }
                if (gameGrid[cur.y + j][cur.x + i - 1].active == true) {
                    return false;
                }
            }
        }
    }


    return true;
}
bool testRight() {
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            if (cur.matrix[i][j]) {
                if (cur.x + i > 12) {
                    return false;
                }
                if (gameGrid[cur.y + j + 1][cur.x + i + 1].active == true) {
                    return false;
                }

            }
        }
    }


    return true;
}
bool checkDownBorder() {
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            if (cur.matrix[i][j]) {
                if (cur.y + j > 25) {
                    return false;
                }
                if (gameGrid[cur.y + j + 1][cur.x + i].active == true) {
                    return false;
                }
            }
        }
    }



    return true;
}

//Testing Game Over
void checkGameOver() {
    for (int i = 6; i < 9;i++) {
        if (gameGrid[0][i].active || gameGrid[1][i].active) {
            running = 0;
        }
    }
}

void printGameGrid() {
    std::cout << "\n";
    for (int i = 0; i < 27; i++){
        for (int j = 0; j < 14; j++){
            std::cout << gameGrid[i][j].active;
        }
        std::cout << "\n";
    }
}

void setNewCur(shape s) {

    pastBlocks.push_back(cur);
    cur = blocks[rand() % 7];
}

void dropTime() {
    if (frameCount % 200 == 0) {
        cur.y++;
    }
}

void checkRows() {
    bool rowFull;

    for (int i = 0; i < 27; i++) {
        rowFull = true;
        for (int j = 0; j < 14; j++) {
            if (!gameGrid[i][j].active) { // Checks if there is a full row
                rowFull = false;
            }
        }
        if (rowFull) { // If there is a full row
            for (int j = 0; j < 14; j++) { // set the all full row to an empty row
                gameGrid[i][j].active = false;
            }
            for (int k = i; k > 0; k--) {               // Starting at the row that was full
                for (int j = 0; j < 14; j++) {
                    gameGrid[k][j] = gameGrid[k-1][j]; //Set the row we are on to the row above it 
                }
            }
        }
    }
}

void update() {

    if (left && testLeft()) cur.x--;
    if (right && testRight()) cur.x++;
    if (down && checkDownBorder()) cur.y++;
    if (up) rotate();

    checkRows();
    checkGameOver();
}

void input() {
    up = down = left = right = 0;
    SDL_Event e;
    while (SDL_PollEvent(&e)) {
        if (e.type == SDL_QUIT) running = false;
        switch (e.type) {
        case SDL_KEYDOWN: //DOWN
            switch (e.key.keysym.sym) {
            case SDLK_LEFT:
                left = 1;
                break;
            case SDLK_RIGHT:
                right = 1;
                break;
            case SDLK_UP:
                up = 1;
                break;
            case SDLK_DOWN:
                down = 1;
                break;
            case SDLK_ESCAPE:
                running = false;
                break;
            }
        }
    }
}

void render() {
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);

    frameCount++;
    int timerFPS = SDL_GetTicks() - lastFrame;
    if (timerFPS < (1000 / 60)) {
        SDL_Delay((1000 / 60) - timerFPS);
    }

    drawCur(cur);
    drawGrid();
    SDL_RenderPresent(renderer);
}

int main() {
    srand(time(NULL));
    cur = blocks[rand() % 7];
    rect.w = rect.h = TILE_SIZE;
    running = 1;
    static int lastTime = 0;
    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "0");
    if (SDL_Init(SDL_INIT_EVERYTHING) < 0) std::cout << "Failed at SDL_Init()" << std::endl;
    if (SDL_CreateWindowAndRenderer(WIDTH, HEIGHT, 0, &window, &renderer) < 0) std::cout << "Failed at SDL_CreateWindowAndRenderer()" << std::endl;
    SDL_SetWindowTitle(window, "Tetris");

    while (running) { // Game Loop
        lastFrame = SDL_GetTicks();
        if (lastFrame >= (lastTime + 1000)) {
            lastTime = lastFrame;
            fps = frameCount;
            frameCount = 0;
        }
        
        if (checkDownBorder()) {
            dropTime();
        }
        else { 
            printGameGrid();
            copyToGamegrid();
            setNewCur(cur);
        }
        update();
        input();
        render();
    }
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

}