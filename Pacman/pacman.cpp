#include <iostream>
#include <ncurses.h>
#include <unistd.h>
#include <vector>
#include <arpa/inet.h>
#include <thread>
//#include <atomic>


#include "Player.h"
#include "Socket.h"

using namespace std;

// Dimensiones del tablero
const int width = 20;
const int height = 21;
int currentFood;

// Posición del Pacman
//int pacX, pacY;
Player* pacmanServer;
Player* pacmanClient;

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

// TODO cambiar a SERVER_MESSAGE_SIZE = 8 * sizeof(int16_t) + 80; para incluir fantasmas
const size_t SERVER_MESSAGE_SIZE = 2 * sizeof(int16_t) + 80; // pos jugador server y 3 fantasmas + 80 caracteres de nombre
const size_t CLIENT_MESSAGE_SIZE = 2 * sizeof(int16_t) + 80; // pos jugador client + 80 caracteres de nombre


// Metodos

void StartServer(const char * s, const char * p){
    Socket socket(s,p);
    socket.bind();
}

void StartClient(const char * s, const char * p){
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
    pacmanServer->dir=Player::Direction::STOP;

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
            if (i == pacmanServer->y && j == pacmanServer->x){
                init_pair(1, COLOR_YELLOW, COLOR_BLACK);
                attron(COLOR_PAIR(1));
                mvprintw(i, j, "C");  // Dibujar el Pacman
                //mvaddch(i, j, 'C');
                attroff(COLOR_PAIR(1));
            }
            else if (i == pacmanClient->y && j == pacmanClient->x){
                init_pair(5, COLOR_MAGENTA, COLOR_BLACK);
                attron(COLOR_PAIR(5));
                mvprintw(i, j, "C");  // Dibujar el Pacman
                attroff(COLOR_PAIR(5));
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
void Input(bool isServer)
{
    int key = getch();
    switch (key) {
        case KEY_LEFT:
            if(isServer)pacmanServer->dir = Player::Direction::LEFT;
            else pacmanClient->dir = Player::Direction::LEFT;
            break;
        case KEY_RIGHT:
            if(isServer)pacmanServer->dir = Player::Direction::RIGHT;
            else pacmanClient->dir = Player::Direction::RIGHT;
            break;
        case KEY_UP:
            if(isServer)pacmanServer->dir = Player::Direction::UP;
            else pacmanClient->dir = Player::Direction::UP;
            break;
        case KEY_DOWN:
            if(isServer)pacmanServer->dir = Player::Direction::DOWN;
            else pacmanClient->dir = Player::Direction::DOWN;
            break;
        case 'q':
            gameOver = true;
            break;
        default:
            break;
    }
}

// Actualizar la posición del Pacman
void UpdatePacmanServer()
{
    int nextX = pacmanServer->x;
    int nextY = pacmanServer->y;

    switch (pacmanServer->dir) {
        case Player::Direction::LEFT:
            nextX = pacmanServer->x - 1;
            break;
        case Player::Direction::RIGHT:
            nextX = pacmanServer->x + 1;
            break;
        case Player::Direction::UP:
            nextY = pacmanServer->y - 1;
            break;
        case Player::Direction::DOWN:
            nextY = pacmanServer->y + 1;
            break;
        default:
            break;
    }

    // Verificar si la siguiente posición es un camino despejado
    if (board[nextY][nextX] != '#') {
        pacmanServer->x = nextX;
        pacmanServer->y = nextY;
    }

    // Actualizar la comida
    if(board[pacmanServer->y][pacmanServer->x]=='.'){
        currentFood--;
        board[pacmanServer->y][pacmanServer->x]=' ';
    }
    if(board[pacmanClient->y][pacmanClient->x]=='.'){
        currentFood--;
        board[pacmanClient->y][pacmanClient->x]=' ';
    }

}
void UpdatePacmanClient()
{
    int nextX = pacmanClient->x;
    int nextY = pacmanClient->y;

    switch (pacmanClient->dir) {
        case Player::Direction::LEFT:
            nextX = pacmanClient->x - 1;
            break;
        case Player::Direction::RIGHT:
            nextX = pacmanClient->x + 1;
            break;
        case Player::Direction::UP:
            nextY = pacmanClient->y - 1;
            break;
        case Player::Direction::DOWN:
            nextY = pacmanClient->y + 1;
            break;
        default:
            break;
    }

    // Verificar si la siguiente posición es un camino despejado
    if (board[nextY][nextX] != '#') {
        pacmanClient->x = nextX;
        pacmanClient->y = nextY;
    }

    // Actualizar la comida
    if(board[pacmanServer->y][pacmanServer->x]=='.'){
        currentFood--;
        board[pacmanServer->y][pacmanServer->x]=' ';
    }
    if(board[pacmanClient->y][pacmanClient->x]=='.'){
        currentFood--;
        board[pacmanClient->y][pacmanClient->x]=' ';
    }

}

// Verificar si se ha alcanzado una condición de finalización del juego
void CheckGameOver()
{
    // Verificar si el Pacman ha sido atrapado por un fantasma
    for(int i=0;i<numGhosts;i++){
        if (pacmanServer->x == ghostPos[2*i] && pacmanServer->y == ghostPos[2*i+1]){
            //gameOver=true;
            break;
        }
    }
    // Verificar si no queda comida
    if(currentFood<=0)gameOver=true;
}

void PrepareClientData(char* &buffer){
    pacmanClient->to_bin();
    memcpy(buffer,pacmanClient->data(),CLIENT_MESSAGE_SIZE);
}
void PrepareServerData(char* &buffer){
    pacmanServer->to_bin();
    memcpy(buffer,pacmanServer->data(),CLIENT_MESSAGE_SIZE);
}
// Procesa los datos en el servidor recibidos por el cliente
void ProcessClientData(char* buffer, ssize_t bytes){
    if(bytes<=0)return;
    pacmanClient->from_bin(buffer);
}
// Procesa los datos en el cliente recibidos por el servidor
void ProcessServerData(char* buffer, ssize_t bytes){
    if(bytes<=0)return;
    pacmanServer->from_bin(buffer);
}

void ServerGameLogic(){
    Draw();
    Input(true);
    UpdatePacmanServer();
    UpdateGhosts();
    CheckGameOver();
    usleep(200000);  // Retardo de 200 ms
}

void ClientGameLogic(){
    Draw();
    Input(false);
    UpdatePacmanClient();
    //UpdateGhosts();
    //CheckGameOver();
    usleep(200000);  // Retardo de 200 ms
}

void ServerReceiveMessages(int client_sd){
    while(true){
        char bufferClient[CLIENT_MESSAGE_SIZE];
        ssize_t bytes =  recv(client_sd,bufferClient,CLIENT_MESSAGE_SIZE,0);
        ProcessClientData(bufferClient, bytes);
    }
    
}

void ClientReceiveMessages(int sd){
    while(true){
        char bufferServer[SERVER_MESSAGE_SIZE];
        ssize_t bytes = recv(sd,bufferServer,SERVER_MESSAGE_SIZE,0);
        ProcessServerData(bufferServer,bytes);
    }
}

int main(int argc, char** argv)
{
    
    
    if (argc >= 4 && strcmp(argv[1], "s") == 0) { // si es el server
        //StartServer(argv[2],argv[3]);

        struct addrinfo hints;
        struct addrinfo *result;

        memset(&hints,0,sizeof(struct addrinfo)); // inicializa a 0 hints

        hints.ai_flags=AI_PASSIVE;
        hints.ai_family=AF_INET; // ipv4
        hints.ai_socktype=SOCK_STREAM;

        int rc = getaddrinfo(argv[2],argv[3],&hints,&result);

        if(rc!=0){
            std::cerr<<"[addrinfo]: "<<gai_strerror(rc)<<"\n";
            return -1;
        }

        int sd=socket(result->ai_family,result->ai_socktype,result->ai_protocol);

        rc=bind(sd,result->ai_addr,result->ai_addrlen);
        listen(sd,5);

        char host[NI_MAXHOST];
        char serv[NI_MAXSERV];
        struct sockaddr_storage client;
        socklen_t client_len=sizeof(struct sockaddr_storage);
        int client_sd = accept(sd,(struct sockaddr*)&client,&client_len);
        getnameinfo((struct sockaddr *) &client,client_len,host,NI_MAXHOST,serv,NI_MAXSERV,NI_NUMERICHOST|NI_NUMERICSERV);
        std::cout << "Conexión desde "<<host<<" "<<serv<<"\n";
        
        // Crear un hilo para recibir mensajes
        std::thread receiveThread(ServerReceiveMessages, client_sd);
        pacmanServer=new Player("PacmanServer",10,1);
        pacmanServer->isServer=true;
        pacmanClient=new Player("PacmanClient",13,1);
        Setup();
        while(true){
            /*char bufferClient[CLIENT_MESSAGE_SIZE];
            ssize_t bytes =  recv(client_sd,bufferClient,CLIENT_MESSAGE_SIZE,0);
            ProcessClientData(bufferClient, bytes);*/
            // ejecutar juego para server
            ServerGameLogic();
            // mandar datos a cliente
            char bufferServer[SERVER_MESSAGE_SIZE];
            pacmanServer->to_bin();
            memcpy(bufferServer,pacmanServer->data(),CLIENT_MESSAGE_SIZE);
            send(client_sd,bufferServer,SERVER_MESSAGE_SIZE,0);
        }
        receiveThread.join();
    }
    else if(argc >= 4 && strcmp(argv[1], "c") == 0){ //si es el client
        //StartClient(argv[2],argv[3]);

        const std::string ip = argv[2];
        const int port = std::stoi(argv[3]);

        // Crear el socket
        int sd = socket(AF_INET, SOCK_STREAM, 0);
        if (sd < 0) {
            std::cerr<<"No se pudo crear el socket\n";
        }

        // Especificar la dirección del servidor
        sockaddr_in serv_addr;
        memset(&serv_addr, 0, sizeof(serv_addr));
        serv_addr.sin_family = AF_INET;
        serv_addr.sin_addr.s_addr = inet_addr(ip.c_str());
        serv_addr.sin_port = htons(port);

        // Conectar al servidor
        if (connect(sd, (sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
            std::cerr<<"No se pudo conectar al servidor\n";
        }

        std::cout << "Conectado al servidor." << std::endl;

        
        // Crear un hilo para recibir mensajes
        std::thread receiveThread(ClientReceiveMessages, sd);
        pacmanClient=new Player("PacmanClient",13,1);
        pacmanServer=new Player("PacmanServer",10,1);
        Setup();
        while (true) {
            ClientGameLogic();
            // mandar datos al server
            char bufferClient[CLIENT_MESSAGE_SIZE];
            pacmanClient->to_bin();
            memcpy(bufferClient,pacmanClient->data(),CLIENT_MESSAGE_SIZE);
            send(sd,bufferClient,SERVER_MESSAGE_SIZE,0);
        }
        receiveThread.join();
    }

    //Setup();

    while (!gameOver) {
        Draw();
        Input(true);
        UpdatePacmanServer();
        UpdateGhosts();
        CheckGameOver();
        usleep(200000);  // Retardo de 200 ms
    }

    FreeResources();

    return 0;
}
