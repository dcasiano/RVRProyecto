#include <iostream>
#include <ncurses.h>
#include <unistd.h>
using namespace std;

// Dimensiones del tablero
const int width = 21;
const int height = 21;

// Posición del Pacman
int x, y;

// Dirección de movimiento del Pacman
enum Direction { STOP = 0, LEFT, RIGHT, UP, DOWN };
Direction dir;

// Estado del juego
bool gameOver;

// Tablero con paredes
char board[height][width] = {
    {'#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#'},
    {'#', '.', '.', '.', '.', '.', '.', '.', '.', '.', '.', '.', '.', '.', '.', '.', '.', '.', '.', '#'},
    {'#', '.', '#', '#', '#', '.', '#', '#', '#', '#', '#', '#', '.', '#', '#', '#', '#', '.', '.', '#'},
    {'#', '.', '#', '.', '.', '.', '#', '.', '.', '.', '#', '.', '.', '.', '#', '.', '.', '.', '#', '#'},
    {'#', '.', '#', '.', '#', '#', '#', '.', '#', '#', '#', '.', '#', '#', '#', '.', '.', '.', '#', '#'},
    {'#', '.', '#', '.', '#', '.', '.', '.', '.', '.', '.', '#', '.', '.', '.', '.', '#', '.', '.', '#'},
    {'#', '.', '#', '.', '#', '.', '#', '#', '#', '.', '#', '#', '#', '#', '#', '#', '.', '#', '.', '#'},
    {'#', '.', '.', '.', '.', '.', '#', '.', '.', '.', '#', '.', '.', '.', '.', '.', '#', '.', '.', '#'},
    {'#', '#', '#', '#', '#', '#', '#', '.', '#', '.', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#'},
    {'#', '.', '.', '.', '.', '.', '.', '.', '.', '.', '.', '.', '.', '.', '.', '.', '.', '.', '.', '#'},
    {'#', '.', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '.', '#'},
    {'#', '.', '#', '.', '.', '.', '.', '.', '.', '.', '.', '.', '.', '.', '.', '.', '.', '#', '.', '#'},
    {'#', '.', '#', '.', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '.', '#', '.', '#'},
    {'#', '.', '#', '.', '.', '.', '.', '.', '.', '.', '.', '.', '.', '.', '.', '.', '.', '.', '#', '#'},
    {'#', '.', '#', '.', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '.', '#'},
    {'#', '.', '.', '.', '.', '.', '.', '.', '.', '.', '.', '.', '.', '.', '.', '.', '.', '#', '.', '#'},
    {'#', '#', '#', '#', '#', '#', '#', '.', '#', '.', '#', '#', '#', '#', '#', '#', '.', '#', '#', '#'},
    {'#', '.', '.', '.', '.', '.', '.', '.', '.', '.', '.', '.', '.', '.', '.', '.', '#', '.', '.', '#'},
    {'#', '.', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '.', '#', '.', '#'},
    {'#', '.', '.', '.', '.', '.', '.', '.', '.', '.', '.', '.', '.', '.', '.', '.', '.', '.', '.', '#'},
    {'#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#'}
};

// Posición de los fantasmas
int ghost1_x, ghost1_y;
int ghost2_x, ghost2_y;
int ghost3_x, ghost3_y;

// Dirección de movimiento de los fantasmas
enum GhostDirection { G_LEFT, G_RIGHT, G_UP, G_DOWN };
GhostDirection ghost1_dir, ghost2_dir, ghost3_dir;

// Inicialización
void Setup()
{
    initscr();  // Inicializar ncurses
    clear();
    noecho();   // No mostrar las teclas pulsadas
    cbreak();   // Deshabilitar el búfer de línea
    keypad(stdscr, TRUE);  // Habilitar teclas especiales
    nodelay(stdscr, TRUE); // No esperar a la entrada del usuario

    gameOver = false;
    dir = STOP;

    // Encontrar la posición inicial del Pacman en el tablero
    /*for (int i = 0; i < height; i++) {
        for (int j = 0; j < width; j++) {
            if (board[i][j] == 'C') {
                y = i;
                x = j;
                break;
            }
        }
    }*/
    x=10; y=1;

    // Inicializar la posición y dirección de los fantasmas
    ghost1_x = 1;
    ghost1_y = 1;
    ghost1_dir = G_DOWN;

    ghost2_x = width - 2;
    ghost2_y = 1;
    ghost2_dir = G_DOWN;

    ghost3_x = width - 2;
    ghost3_y = height - 2;
    ghost3_dir = G_UP;

    srand(time(NULL));  // Inicializar la semilla aleatoria
}

// Liberar recursos al finalizar
void Terminate()
{
    endwin();  // Finalizar ncurses
}

// Dibujar el tablero y el Pacman
void Draw()
{
    clear();
    start_color();

    for (int i = 0; i < height; i++) {
        for (int j = 0; j < width; j++) {
            if (i == y && j == x){
                init_pair(1, COLOR_YELLOW, COLOR_BLACK);
                attron(COLOR_PAIR(1));
                mvprintw(i, j, "C");  // Dibujar el Pacman
                //mvaddch(i, j, 'C');
                attroff(COLOR_PAIR(1));
            }
            else
                mvprintw(i, j, "%c", board[i][j]);  // Dibujar elementos del tablero
        }
    }
    // Dibujar a los fantasmas
    init_pair(2, COLOR_RED, COLOR_BLACK);
    attron(COLOR_PAIR(2));
    mvprintw(ghost1_y, ghost1_x, "G");
    attroff(COLOR_PAIR(2));

    init_pair(3, COLOR_BLUE, COLOR_BLACK);
    attron(COLOR_PAIR(3));
    mvprintw(ghost2_y, ghost2_x, "G");
    attroff(COLOR_PAIR(3));

    init_pair(4, COLOR_GREEN, COLOR_BLACK);
    attron(COLOR_PAIR(4));
    mvprintw(ghost3_y, ghost3_x, "G");
    attroff(COLOR_PAIR(4));


    mvprintw(height + 1, 0, "Use las flechas para moverse");
    refresh();
}

// Actualizar la posición de los fantasmas
void UpdateGhosts()
{
    // Fantasma 1
    int ghost1_new_x = ghost1_x;
    int ghost1_new_y = ghost1_y;

    // Generar dirección aleatoria para el Fantasma 1
    int ghost1_random = rand() % 4;
    switch (ghost1_random) {
        case 0:
            ghost1_new_x--;
            break;
        case 1:
            ghost1_new_x++;
            break;
        case 2:
            ghost1_new_y--;
            break;
        case 3:
            ghost1_new_y++;
            break;
    }

    // Comprobar si el Fantasma 1 puede moverse a la nueva posición
    if (board[ghost1_new_y][ghost1_new_x] != '#') {
        ghost1_x = ghost1_new_x;
        ghost1_y = ghost1_new_y;
    }

    // Fantasma 2
    int ghost2_new_x = ghost2_x;
    int ghost2_new_y = ghost2_y;

    // Generar dirección aleatoria para el Fantasma 2
    int ghost2_random = rand() % 4;
    switch (ghost2_random) {
        case 0:
            ghost2_new_x--;
            break;
        case 1:
            ghost2_new_x++;
            break;
        case 2:
            ghost2_new_y--;
            break;
        case 3:
            ghost2_new_y++;
            break;
    }

    // Comprobar si el Fantasma 2 puede moverse a la nueva posición
    if (board[ghost2_new_y][ghost2_new_x] != '#') {
        ghost2_x = ghost2_new_x;
        ghost2_y = ghost2_new_y;
    }

    // Fantasma 3
    int ghost3_new_x = ghost3_x;
    int ghost3_new_y = ghost3_y;

    // Generar dirección aleatoria para el Fantasma 3
    int ghost3_random = rand() % 4;
    switch (ghost3_random) {
        case 0:
            ghost3_new_x--;
            break;
        case 1:
            ghost3_new_x++;
            break;
        case 2:
            ghost3_new_y--;
            break;
        case 3:
            ghost3_new_y++;
            break;
    }

    // Comprobar si el Fantasma 3 puede moverse a la nueva posición
    if (board[ghost3_new_y][ghost3_new_x] != '#') {
        ghost3_x = ghost3_new_x;
        ghost3_y = ghost3_new_y;
    }
}

// Entrada del usuario
void Input()
{
    int key = getch();
    switch (key) {
        case KEY_LEFT:
            dir = LEFT;
            break;
        case KEY_RIGHT:
            dir = RIGHT;
            break;
        case KEY_UP:
            dir = UP;
            break;
        case KEY_DOWN:
            dir = DOWN;
            break;
        case 'q':
            gameOver = true;
            break;
    }
}

// Actualizar la posición del Pacman
void UpdatePacman()
{
    int nextX = x;
    int nextY = y;

    switch (dir) {
        case LEFT:
            nextX = x - 1;
            break;
        case RIGHT:
            nextX = x + 1;
            break;
        case UP:
            nextY = y - 1;
            break;
        case DOWN:
            nextY = y + 1;
            break;
    }

    // Verificar si la siguiente posición es un camino despejado
    if (board[nextY][nextX] != '#') {
        x = nextX;
        y = nextY;
    }

    // Verificar si se alcanzó la meta (punto de salida)
    /*if (board[y][x] == '.') {
        gameOver = true;
    }*/
}

// Verificar si se ha alcanzado una condición de finalización del juego
void CheckGameOver()
{
    // Verificar si el Pacman ha sido atrapado por un fantasma
    if (x == ghost1_x && y == ghost1_y) {
        gameOver = true;
    } else if (x == ghost2_x && y == ghost2_y) {
        gameOver = true;
    } else if (x == ghost3_x && y == ghost3_y) {
        gameOver = true;
    }
}


int main()
{
    Setup();

    while (!gameOver) {
        Draw();
        Input();
        UpdatePacman();
        UpdateGhosts();
        CheckGameOver();
        usleep(200000);  // Retardo de 100 ms
    }

    Terminate();

    return 0;
}
