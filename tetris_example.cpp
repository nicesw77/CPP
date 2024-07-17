#include <SDL2/SDL.h>
#include <stdbool.h>
#include <time.h>

#define SCREEN_WIDTH 300
#define SCREEN_HEIGHT 600
#define BLOCK_SIZE 30

typedef struct {
    int x, y;
    int shape[4][4];
    SDL_Color color;
} Tetrimino;

Tetrimino shapes[] = {
    {0, 0, {{1, 1, 1, 1}}, {255, 0, 0, 255}},
    {0, 0, {{1, 1}, {1, 1}}, {0, 255, 0, 255}},
    {0, 0, {{1, 1, 0}, {0, 1, 1}}, {0, 0, 255, 255}},
    {0, 0, {{0, 1, 1}, {1, 1, 0}}, {255, 255, 0, 255}},
    {0, 0, {{1, 1, 1}, {0, 1, 0}}, {255, 165, 0, 255}},
    {0, 0, {{1, 1, 1}, {1, 0, 0}}, {128, 0, 128, 255}},
    {0, 0, {{1, 1, 1}, {0, 0, 1}}, {0, 255, 255, 255}},
};

SDL_Color grid[20][10];

void rotate(Tetrimino *tetrimino) {
    int temp[4][4] = {0};
    for (int y = 0; y < 4; y++) {
        for (int x = 0; x < 4; x++) {
            temp[x][3 - y] = tetrimino->shape[y][x];
        }
    }
    for (int y = 0; y < 4; y++) {
        for (int x = 0; x < 4; x++) {
            tetrimino->shape[y][x] = temp[y][x];
        }
    }
}

bool check_collision(Tetrimino *tetrimino, int offsetX, int offsetY) {
    for (int y = 0; y < 4; y++) {
        for (int x = 0; x < 4; x++) {
            if (tetrimino->shape[y][x]) {
                int newX = tetrimino->x + x + offsetX;
                int newY = tetrimino->y + y + offsetY;
                if (newX < 0 || newX >= 10 || newY >= 20 || (newY >= 0 && grid[newY][newX].r != 0)) {
                    return true;
                }
            }
        }
    }
    return false;
}

void lock_tetrimino(Tetrimino *tetrimino) {
    for (int y = 0; y < 4; y++) {
        for (int x = 0; x < 4; x++) {
            if (tetrimino->shape[y][x]) {
                grid[tetrimino->y + y][tetrimino->x + x] = tetrimino->color;
            }
        }
    }
}

void clear_lines() {
    for (int y = 19; y >= 0; y--) {
        bool full = true;
        for (int x = 0; x < 10; x++) {
            if (grid[y][x].r == 0) {
                full = false;
                break;
            }
        }
        if (full) {
            for (int row = y; row > 0; row--) {
                for (int col = 0; col < 10; col++) {
                    grid[row][col] = grid[row - 1][col];
                }
            }
            for (int col = 0; col < 10; col++) {
                grid[0][col] = (SDL_Color){0, 0, 0, 255};
            }
            y++;
        }
    }
}

int main(int argc, char* args[]) {
    SDL_Init(SDL_INIT_VIDEO);

    SDL_Window* window = SDL_CreateWindow("Tetris", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

    srand(time(NULL));
    Tetrimino current = shapes[rand() % 7];
    current.x = 3;

    bool quit = false;
    SDL_Event e;
    Uint32 lastTick = SDL_GetTicks();
    Uint32 speed = 500;

    memset(grid, 0, sizeof(grid));

    while (!quit) {
        while (SDL_PollEvent(&e) != 0) {
            if (e.type == SDL_QUIT) {
                quit = true;
            } else if (e.type == SDL_KEYDOWN) {
                switch (e.key.keysym.sym) {
                    case SDLK_LEFT:
                        if (!check_collision(&current, -1, 0)) current.x--;
                        break;
                    case SDLK_RIGHT:
                        if (!check_collision(&current, 1, 0)) current.x++;
                        break;
                    case SDLK_DOWN:
                        if (!check_collision(&current, 0, 1)) current.y++;
                        break;
                    case SDLK_UP:
                        rotate(&current);
                        if (check_collision(&current, 0, 0)) {
                            rotate(&current);
                            rotate(&current);
                            rotate(&current);
                        }
                        break;
                }
            }
        }

        if (SDL_GetTicks() - lastTick > speed) {
            if (!check_collision(&current, 0, 1)) {
                current.y++;
            } else {
                lock_tetrimino(&current);
                clear_lines();
                current = shapes[rand() % 7];
                current.x = 3;
                current.y = 0;
                if (check_collision(&current, 0, 0)) {
                    quit = true; // Game Over
                }
            }
            lastTick = SDL_GetTicks();
        }

        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);

        for (int y = 0; y < 20; y++) {
            for (int x = 0; x < 10; x++) {
                if (grid[y][x].r != 0) {
                    SDL_SetRenderDrawColor(renderer, grid[y][x].r, grid[y][x].g, grid[y][x].b, 255);
                    SDL_Rect rect = { x * BLOCK_SIZE, y * BLOCK_SIZE, BLOCK_SIZE, BLOCK_SIZE };
                    SDL_RenderFillRect(renderer, &rect);
                }
            }
        }

        SDL_SetRenderDrawColor(renderer, current.color.r, current.color.g, current.color.b, 255);
        for (int y = 0; y < 4; y++) {
            for (int x = 0; x < 4; x++) {
                if (current.shape[y][x]) {
                    SDL_Rect rect = { (current.x + x) * BLOCK_SIZE, (current.y + y) * BLOCK_SIZE, BLOCK_SIZE, BLOCK_SIZE };
                    SDL_RenderFillRect(renderer, &rect);
                }
            }
        }

        SDL_RenderPresent(renderer);
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}