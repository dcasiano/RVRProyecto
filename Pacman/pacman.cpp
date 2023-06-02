#include <iostream>
#include <ncurses.h>
#include <unistd.h>
#include <vector>
#include <arpa/inet.h>
#include <thread>

#include <sys/socket.h>
#include <sys/types.h>
#include <netdb.h>

#include <stdexcept>

#include <ostream>

#include "Entity.h"

using namespace std;

// Definicion de variables del juego

// Dimensiones del tablero
const int width = 20;
const int height = 21;
int currentFood;

// Los dos pacman
Entity* pacmanLocal;
Entity* pacmanRemote;

// Estado del juego
bool gameOver;

// Tablero ('.' -> comida '#' -> pared)
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

// Fantasmas
const int numGhosts=3;
vector<Entity*>ghosts;

// Tamaño mensajes
// pos jugador server y 3 fantasmas + 80 caracteres por cada nombre + variable isAlive
const size_t SERVER_MESSAGE_SIZE = 4*(2 * sizeof(int16_t) + 80 + sizeof(bool)); 
// pos jugador client + 80 caracteres del nombre + variable isAlive
const size_t CLIENT_MESSAGE_SIZE = 2 * sizeof(int16_t) + 80 + sizeof(bool); 


// Metodos

// Inicialización
void Setup()
{
    initscr();  // Inicializar ncurses
    clear();
    noecho();   // No mostrar las teclas pulsadas
    cbreak();   // Para enviar las pulsaciones directamente, sin pulsar Enter
    keypad(stdscr, TRUE);  // Habilitar teclas especiales
    nodelay(stdscr, TRUE); // No esperar a la entrada del usuario

    gameOver = false;
    pacmanLocal->dir=Entity::Direction::STOP;

    ghosts.push_back(new Entity("Ghost1",1,3));
    ghosts.push_back(new Entity("Ghost2",width-3,3));
    ghosts.push_back(new Entity("Ghost3",width-3,height-2));
    ghosts[0]->dir=Entity::Direction::DOWN;
    ghosts[1]->dir=Entity::Direction::DOWN;
    ghosts[2]->dir=Entity::Direction::LEFT;

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
    clear();
    endwin();  // Finalizar ncurses
}

// Dibujar el tablero, los Pacman y los fantasmmas
void Draw()
{
    clear();
    start_color();

    for (int i = 0; i < height; i++) {
        for (int j = 0; j < width; j++) {
            if (i == pacmanLocal->y && j == pacmanLocal->x){
                init_pair(1, COLOR_YELLOW, COLOR_BLACK);
                attron(COLOR_PAIR(1));
                if(pacmanLocal->isAlive)mvprintw(i, j, "C");  // Dibujar el Pacman
                attroff(COLOR_PAIR(1));
            }
            else if (i == pacmanRemote->y && j == pacmanRemote->x){
                init_pair(5, COLOR_MAGENTA, COLOR_BLACK);
                attron(COLOR_PAIR(5));
                if(pacmanRemote->isAlive)mvprintw(i, j, "C");  // Dibujar el Pacman
                attroff(COLOR_PAIR(5));
            }
            else
                mvprintw(i, j, "%c", board[i][j]);  // Dibujar elementos del tablero
        }
    }
    // Dibujar a los fantasmas
    init_pair(2, COLOR_RED, COLOR_BLACK);
    attron(COLOR_PAIR(2));
    mvprintw(ghosts[0]->y, ghosts[0]->x, "X");
    attroff(COLOR_PAIR(2));

    init_pair(3, COLOR_BLUE, COLOR_BLACK);
    attron(COLOR_PAIR(3));
    mvprintw(ghosts[1]->y, ghosts[1]->x, "X");
    attroff(COLOR_PAIR(3));

    init_pair(4, COLOR_GREEN, COLOR_BLACK);
    attron(COLOR_PAIR(4));
    mvprintw(ghosts[2]->y, ghosts[2]->x, "X");
    attroff(COLOR_PAIR(4));

    const char* name = pacmanLocal->name;
    mvprintw(height + 1, 0, "%s", name);
    mvprintw(height + 2, 0, "Use arrow buttons to move");
    refresh();
}

// Actualizar la posicion de los fantasmas
void UpdateGhosts()
{
    std::vector<Entity::Direction> possible_dirs;
    for(int i=0;i<numGhosts;i++){
        int ghost_new_x = ghosts[i]->x;
        int ghost_new_y = ghosts[i]->y;

        switch (ghosts[i]->dir) {
                case Entity::Direction::LEFT:
                    if (board[ghost_new_y][ghost_new_x - 1] != '#') {
                        possible_dirs.push_back(Entity::Direction::LEFT);
                    }
                    if (board[ghost_new_y - 1][ghost_new_x] != '#') {
                        possible_dirs.push_back(Entity::Direction::UP);
                    }
                    if (board[ghost_new_y + 1][ghost_new_x] != '#') {
                        possible_dirs.push_back(Entity::Direction::DOWN);
                    }
                    break;
                case Entity::Direction::RIGHT:
                    if (board[ghost_new_y][ghost_new_x + 1] != '#') {
                        possible_dirs.push_back(Entity::Direction::RIGHT);
                    }  
                    if (board[ghost_new_y - 1][ghost_new_x] != '#') {
                        possible_dirs.push_back(Entity::Direction::UP);
                    }
                    if (board[ghost_new_y + 1][ghost_new_x] != '#') {
                        possible_dirs.push_back(Entity::Direction::DOWN);
                    }
                    break;
                case Entity::Direction::UP:
                    if (board[ghost_new_y - 1][ghost_new_x] != '#') {
                        possible_dirs.push_back(Entity::Direction::UP);
                    }
                    if (board[ghost_new_y][ghost_new_x - 1] != '#') {
                        possible_dirs.push_back(Entity::Direction::LEFT);
                    }
                    if (board[ghost_new_y][ghost_new_x + 1] != '#') {
                        possible_dirs.push_back(Entity::Direction::RIGHT);
                    }
                    break;
                case Entity::Direction::DOWN:
                    if (board[ghost_new_y + 1][ghost_new_x] != '#') {
                        possible_dirs.push_back(Entity::Direction::DOWN);
                    }
                    if (board[ghost_new_y][ghost_new_x - 1] != '#') {
                        possible_dirs.push_back(Entity::Direction::LEFT);
                    }
                    if (board[ghost_new_y][ghost_new_x + 1] != '#') {
                        possible_dirs.push_back(Entity::Direction::RIGHT);
                    }
                    break;
                default:
                    break;
            }

        // Si hay direcciones disponibles, generar una direccion aleatoria entre las posibles
        // (podria ser seguir en linea recta)
        if (!possible_dirs.empty()) {
            int random_index = rand() % possible_dirs.size();
            Entity::Direction new_dir = possible_dirs[random_index];

            switch (new_dir) {
                case Entity::Direction::LEFT:
                    ghost_new_x--;
                    break;
                case Entity::Direction::RIGHT:
                    ghost_new_x++;
                    break;
                case Entity::Direction::UP:
                    ghost_new_y--;
                    break;
                case Entity::Direction::DOWN:
                    ghost_new_y++;
                    break;
                default:
                    break;
            }
            ghosts[i]->dir=new_dir;
        }
        // Si no las hay, se da media vuelta
        else{
            switch (ghosts[i]->dir) {
                case Entity::Direction::LEFT:
                    ghost_new_x++;
                    ghosts[i]->dir=Entity::Direction::RIGHT;
                    break;
                case Entity::Direction::RIGHT:
                    ghost_new_x--;
                    ghosts[i]->dir=Entity::Direction::LEFT;
                    break;
                case Entity::Direction::UP:
                    ghost_new_y++;
                    ghosts[i]->dir=Entity::Direction::DOWN;
                    break;
                case Entity::Direction::DOWN:
                    ghost_new_y--;
                    ghosts[i]->dir=Entity::Direction::UP;
                    break;
                default:
                    break;
            }
        }

        ghosts[i]->x = ghost_new_x;
        ghosts[i]->y = ghost_new_y;
        possible_dirs.clear();
    }
}

// Entrada del usuario
void Input()
{
    if(!pacmanLocal->isAlive)return;
    int key = getch();
    switch (key) {
        case KEY_LEFT:
            pacmanLocal->dir = Entity::Direction::LEFT;
            break;
        case KEY_RIGHT:
            pacmanLocal->dir = Entity::Direction::RIGHT;
            break;
        case KEY_UP:
            pacmanLocal->dir = Entity::Direction::UP;
            break;
        case KEY_DOWN:
            pacmanLocal->dir = Entity::Direction::DOWN;
            break;
        case 'q':
            gameOver = true;
            break;
        default:
            break;
    }
}

// Actualizar la posicion del pacman
void UpdatePacman()
{
    int nextX = pacmanLocal->x;
    int nextY = pacmanLocal->y;

    switch (pacmanLocal->dir) {
        case Entity::Direction::LEFT:
            nextX = pacmanLocal->x - 1;
            break;
        case Entity::Direction::RIGHT:
            nextX = pacmanLocal->x + 1;
            break;
        case Entity::Direction::UP:
            nextY = pacmanLocal->y - 1;
            break;
        case Entity::Direction::DOWN:
            nextY = pacmanLocal->y + 1;
            break;
        default:
            break;
    }

    // Verificar si la siguiente posicion es un camino despejado
    if (board[nextY][nextX] != '#') {
        pacmanLocal->x = nextX;
        pacmanLocal->y = nextY;
    }

    // Actualizar la comida
    if(board[pacmanLocal->y][pacmanLocal->x]=='.'){
        currentFood--;
        board[pacmanLocal->y][pacmanLocal->x]=' ';
    }
    if(board[pacmanRemote->y][pacmanRemote->x]=='.'){
        currentFood--;
        board[pacmanRemote->y][pacmanRemote->x]=' ';
    }

}


// Verificar si se ha producido una condicion de gameover
void CheckGameOver()
{
    // Verificar si el Pacman ha sido atrapado por un fantasma
    for(int i=0;i<numGhosts;i++){
        if (pacmanLocal->x == ghosts[i]->x && pacmanLocal->y == ghosts[i]->y){
            pacmanLocal->isAlive=false;
            pacmanLocal->dir=Entity::Direction::STOP;
        }
    }
    // Si estan los dos pacman muertos se acaba la partida
    if(!pacmanLocal->isAlive&&!pacmanRemote->isAlive) gameOver=true;
    // Si no queda comida se acaba la partida
    if(currentFood<=0)gameOver=true;
}


// Metodos para enviar y recibir mensajes para el juego online

// Envia al cliente los datos del pacman del servidor y de los fantasmas
void SendDataToClient(int sd){
    char buffer[SERVER_MESSAGE_SIZE];
    pacmanLocal->to_bin();

    // se utiliza CLIENT_MESSAGE_SIZE porque es lo que ocupan los datos
    // de una entidad, de manera que se pueda separar los datos de cada una
    memcpy(buffer,pacmanLocal->data(),CLIENT_MESSAGE_SIZE);
    for(int i=0;i<numGhosts;i++){
        ghosts[i]->to_bin();
        memcpy(buffer+(i+1)*CLIENT_MESSAGE_SIZE,ghosts[i]->data(),CLIENT_MESSAGE_SIZE);
    }
    send(sd,buffer,SERVER_MESSAGE_SIZE,0);
}
// Envia al server los datos del pacman del cliente
void SendDataToServer(int sd){
    char buffer[CLIENT_MESSAGE_SIZE];
    pacmanLocal->to_bin();
    memcpy(buffer,pacmanLocal->data(),CLIENT_MESSAGE_SIZE);
    send(sd,buffer,CLIENT_MESSAGE_SIZE,0);
}
// El servidor procesa los datos recibidos del cliente
void ProcessClientData(char* buffer, ssize_t bytes){
    if(bytes<=0)return;
    pacmanRemote->from_bin(buffer);
}
// El cliente procesa los datos recibidos del servidor
void ProcessServerData(char* buffer, ssize_t bytes){
    if(bytes<=0)return;
    pacmanRemote->from_bin(buffer);
    for(int i=0;i<numGhosts;i++){
        ghosts[i]->from_bin(buffer+(i+1)*CLIENT_MESSAGE_SIZE);
    }
}

void ServerGameLogic(){
    Draw();
    Input();
    UpdatePacman();
    UpdateGhosts();
    CheckGameOver();
    usleep(200000);  // Delay de 200 ms
}

void ClientGameLogic(){
    Draw();
    Input();
    UpdatePacman();
    CheckGameOver();
    usleep(200000);  // Delay de 200 ms
}

// Metodo que se ejecuta en un hilo diferente al programa
// Dedicado a recibir mensajes del cliente
void ReceiveMessagesFromClient(int sd){
    while(!gameOver){
        char buffer[CLIENT_MESSAGE_SIZE];
        ssize_t bytes =  recv(sd,buffer,CLIENT_MESSAGE_SIZE,0);
        if(bytes<=0)break;
        ProcessClientData(buffer, bytes);
    }
    close(sd);
}

// Metodo que se ejecuta en un hilo diferente al programa
// Dedicado a recibir mensajes del server
void ReceiveMessagesFromServer(int sd){
    while(!gameOver){
        char buffer[SERVER_MESSAGE_SIZE];
        ssize_t bytes = recv(sd,buffer,SERVER_MESSAGE_SIZE,0);
        if(bytes<=0)break;
        ProcessServerData(buffer,bytes);
    }
}

int main(int argc, char** argv)
{
    if (argc >= 4 && strcmp(argv[1], "s") == 0) { // Si es el server

        struct addrinfo hints;
        struct addrinfo *result;

        memset(&hints,0,sizeof(struct addrinfo));

        // Se establecen algunos criterios de busqueda
        hints.ai_flags=AI_PASSIVE;
        hints.ai_family=AF_INET; // ipv4
        hints.ai_socktype=SOCK_STREAM;

        int rc = getaddrinfo(argv[2],argv[3],&hints,&result); // Devuelve la direccion de red

        if(rc!=0){
            std::cerr<<"[addrinfo]: "<<gai_strerror(rc)<<"\n";
            return -1;
        }

        // Se crea el socket del tipo especificado
        int sd=socket(result->ai_family,result->ai_socktype,result->ai_protocol);

        // Se asocia la direccion al socket
        rc=bind(sd,result->ai_addr,result->ai_addrlen);

        // Se pone el socket en estado listen
        listen(sd,5);

        char host[NI_MAXHOST];
        char serv[NI_MAXSERV];
        struct sockaddr_storage client;
        socklen_t client_len=sizeof(struct sockaddr_storage);

        // Se acepta la conexion. Devuelve un socket nuevo referente a la conexion establecida
        // Esta llamada es bloqueante
        int client_sd = accept(sd,(struct sockaddr*)&client,&client_len);
        getnameinfo((struct sockaddr *) &client,client_len,host,NI_MAXHOST,serv,NI_MAXSERV,NI_NUMERICHOST|NI_NUMERICSERV);
        std::cout << "Conexión desde "<<host<<" "<<serv<<"\n";
        
        // Crear un hilo para recibir mensajes
        std::thread receiveThread(ReceiveMessagesFromClient, client_sd);

        // Inicializar variables para servidor
        pacmanLocal=new Entity("PacmanServer",10,1);
        pacmanRemote=new Entity("PacmanClient",12,1);
        Setup();

        while(!gameOver){
            // Ejecutar la logica del juego del servidor
            ServerGameLogic();
            // Mandar datos a cliente
            SendDataToClient(client_sd);
        }
        receiveThread.join();
        close(sd);
    }
    else if(argc >= 4 && strcmp(argv[1], "c") == 0){ // Si es el client

        const std::string ip = argv[2];
        const int port = std::stoi(argv[3]);

        // Crear el socket
        int sd = socket(AF_INET, SOCK_STREAM, 0);
        if (sd < 0) {
            std::cerr<<"No se pudo crear el socket\n";
        }

        // Especificar la direccion del servidor
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
        std::thread receiveThread(ReceiveMessagesFromServer, sd);

        // Inicializar variables para cliente
        pacmanLocal=new Entity("PacmanClient",12,1);
        pacmanRemote=new Entity("PacmanServer",10,1);
        Setup();

        while (!gameOver) {
            // Ejecutar la logica del cliente
            ClientGameLogic();
            // Mandar datos al server
            SendDataToServer(sd);
        }
        receiveThread.join();
        close(sd);
    }

    

    FreeResources();

    return 0;
}
