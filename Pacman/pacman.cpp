#include <iostream>
#include <ncurses.h>
#include <unistd.h>
#include <vector>

#include "Player.h"
#include "Socket.h"

using namespace std;

// Dimensiones del tablero
const int width = 20;
const int height = 21;
int currentFood;

// Posición del Pacman
//int pacX, pacY;
Player pacman("Pacman",10,1);

// Dirección de movimiento del Pacman
//enum Direction { STOP = 0, LEFT, RIGHT, UP, DOWN };
//Direction dir;

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
const int numGhosts=3;
vector<int>ghostPos(2*numGhosts);

// Dirección de movimiento de los fantasmas
enum GhostDirection { G_LEFT, G_RIGHT, G_UP, G_DOWN };
vector<GhostDirection>ghostDir(numGhosts);




// Metodos

void StartServer(const char * s, const char * p){
    Socket socket(s,p);
    socket.bind();
}

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
    //dir = STOP;
    pacman.dir=Player::Direction::STOP;

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
    //pacX=10; pacY=1;

    // Inicializar la posición y dirección de los fantasmas
    ghostPos[0]=1;
    ghostPos[1]=1;
    ghostDir[0]=G_DOWN;

    ghostPos[2]=width-3;
    ghostPos[3]=1;
    ghostDir[1]=G_DOWN;

    ghostPos[4]=width-3;
    ghostPos[5]=height-2;
    ghostDir[2]=G_UP;

    currentFood = 0;

    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            if (board[y][x] == '.') {
                currentFood++;
            }
        }
    }
    
    srand(time(NULL));  // Inicializar la semilla aleatoria
}

// Liberar recursos al finalizar
void FreeResources()
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
            if (i == pacman.y && j == pacman.x){
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
    mvprintw(ghostPos[1], ghostPos[0], "G");
    attroff(COLOR_PAIR(2));

    init_pair(3, COLOR_BLUE, COLOR_BLACK);
    attron(COLOR_PAIR(3));
    mvprintw(ghostPos[3], ghostPos[2], "G");
    attroff(COLOR_PAIR(3));

    init_pair(4, COLOR_GREEN, COLOR_BLACK);
    attron(COLOR_PAIR(4));
    mvprintw(ghostPos[5], ghostPos[4], "G");
    attroff(COLOR_PAIR(4));


    mvprintw(height + 1, 0, "Use arrow buttons to move");
    mvprintw(height + 2, 0, "Press q to quit");
    refresh();
}

// Actualizar la posición de los fantasmas
void UpdateGhosts()
{
    // Fantasma 1
    std::vector<GhostDirection> possible_dirs;
    for(int i=0;i<numGhosts;i++){
        int ghost1_new_x = ghostPos[2*i];
        int ghost1_new_y = ghostPos[2*i+1];

        switch (ghostDir[i]) {
                case G_LEFT:
                    if (board[ghost1_new_y][ghost1_new_x - 1] != '#') {
                        possible_dirs.push_back(G_LEFT);
                    }
                    if (board[ghost1_new_y - 1][ghost1_new_x] != '#') {
                        possible_dirs.push_back(G_UP);
                    }
                    if (board[ghost1_new_y + 1][ghost1_new_x] != '#') {
                        possible_dirs.push_back(G_DOWN);
                    }
                    break;
                case G_RIGHT:
                    if (board[ghost1_new_y][ghost1_new_x + 1] != '#') {
                        possible_dirs.push_back(G_RIGHT);
                    }  
                    if (board[ghost1_new_y - 1][ghost1_new_x] != '#') {
                        possible_dirs.push_back(G_UP);
                    }
                    if (board[ghost1_new_y + 1][ghost1_new_x] != '#') {
                        possible_dirs.push_back(G_DOWN);
                    }
                    break;
                case G_UP:
                    if (board[ghost1_new_y - 1][ghost1_new_x] != '#') {
                        possible_dirs.push_back(G_UP);
                    }
                    if (board[ghost1_new_y][ghost1_new_x - 1] != '#') {
                        possible_dirs.push_back(G_LEFT);
                    }
                    if (board[ghost1_new_y][ghost1_new_x + 1] != '#') {
                        possible_dirs.push_back(G_RIGHT);
                    }
                    break;
                case G_DOWN:
                    if (board[ghost1_new_y + 1][ghost1_new_x] != '#') {
                        possible_dirs.push_back(G_DOWN);
                    }
                    if (board[ghost1_new_y][ghost1_new_x - 1] != '#') {
                        possible_dirs.push_back(G_LEFT);
                    }
                    if (board[ghost1_new_y][ghost1_new_x + 1] != '#') {
                        possible_dirs.push_back(G_RIGHT);
                    }
                    break;
                default:
                    break;
            }

        // Si hay direcciones disponibles, generar una dirección aleatoria entre las posibles
        if (!possible_dirs.empty()) {
            int random_index = rand() % possible_dirs.size();
            GhostDirection new_dir = possible_dirs[random_index];

            switch (new_dir) {
                case G_LEFT:
                    ghost1_new_x--;
                    break;
                case G_RIGHT:
                    ghost1_new_x++;
                    break;
                case G_UP:
                    ghost1_new_y--;
                    break;
                case G_DOWN:
                    ghost1_new_y++;
                    break;
                default:
                    break;
            }
            ghostDir[i]=new_dir;
        }
        else{
            switch (ghostDir[i]) {
                case G_LEFT:
                    ghost1_new_x++;
                    ghostDir[i]=G_RIGHT;
                    break;
                case G_RIGHT:
                    ghost1_new_x--;
                    ghostDir[i]=G_LEFT;
                    break;
                case G_UP:
                    ghost1_new_y++;
                    ghostDir[i]=G_DOWN;
                    break;
                case G_DOWN:
                    ghost1_new_y--;
                    ghostDir[i]=G_UP;
                    break;
                default:
                    break;
            }
        }

        ghostPos[2*i] = ghost1_new_x;
        ghostPos[2*i+1] = ghost1_new_y;
        possible_dirs.clear();
    }
}

// Entrada del usuario
void Input()
{
    int key = getch();
    switch (key) {
        case KEY_LEFT:
            pacman.dir = Player::Direction::LEFT;
            break;
        case KEY_RIGHT:
            pacman.dir = Player::Direction::RIGHT;
            break;
        case KEY_UP:
            pacman.dir = Player::Direction::UP;
            break;
        case KEY_DOWN:
            pacman.dir = Player::Direction::DOWN;
            break;
        case 'q':
            gameOver = true;
            break;
        default:
            break;
    }
}

// Actualizar la posición del Pacman
void UpdatePacman()
{
    int nextX = pacman.x;
    int nextY = pacman.y;

    switch (pacman.dir) {
        case Player::Direction::LEFT:
            nextX = pacman.x - 1;
            break;
        case Player::Direction::RIGHT:
            nextX = pacman.x + 1;
            break;
        case Player::Direction::UP:
            nextY = pacman.y - 1;
            break;
        case Player::Direction::DOWN:
            nextY = pacman.y + 1;
            break;
        default:
            break;
    }

    // Verificar si la siguiente posición es un camino despejado
    if (board[nextY][nextX] != '#') {
        pacman.x = nextX;
        pacman.y = nextY;
    }

    // Actualizar la comida
    if(board[pacman.y][pacman.x]=='.'){
        currentFood--;
        board[pacman.y][pacman.x]=' ';
    }

}

// Verificar si se ha alcanzado una condición de finalización del juego
void CheckGameOver()
{
    // Verificar si el Pacman ha sido atrapado por un fantasma
    for(int i=0;i<numGhosts;i++){
        if (pacman.x == ghostPos[2*i] && pacman.y == ghostPos[2*i+1]){
            //gameOver=true;
            break;
        }
    }
    // Verificar si no queda comida
    if(currentFood<=0)gameOver=true;
}


int main(int argc, char** argv)
{
    if (argc >= 4 && strcmp(argv[1], "s") == 0) {
        StartServer(argv[2],argv[3]);
    }

    //Setup();

    while (!gameOver) {
        Draw();
        Input();
        UpdatePacman();
        UpdateGhosts();
        CheckGameOver();
        usleep(200000);  // Retardo de 200 ms
    }

    FreeResources();

    return 0;
}
