#include <iostream>
#include <ncurses.h>
#include <unistd.h>
#include <vector>
#include <arpa/inet.h>
#include <thread>
//#include <atomic>

#include <sys/socket.h>
#include <sys/types.h>
#include <netdb.h>

#include <stdexcept>

#include <ostream>

#include "Entity.h"
//#include "Socket.h"

using namespace std;

// Dimensiones del tablero
const int width = 20;
const int height = 21;
int currentFood;

// Posición del Pacman
//int pacX, pacY;
Entity* pacmanLocal;
Entity* pacmanRemote;

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
//vector<int>ghostPos(2*numGhosts);
vector<Entity*>ghosts;

// Dirección de movimiento de los fantasmas
/*enum GhostDirection { G_LEFT, G_RIGHT, G_UP, G_DOWN };
vector<GhostDirection>ghostDir(numGhosts);*/

// pos jugador server y 3 fantasmas + 80 caracteres por cada nombre + 1 bool clientAlive + 1 bool gameOver
const size_t SERVER_MESSAGE_SIZE = 4*(2 * sizeof(int16_t) + 80) + 2*sizeof(bool); 
// pos jugador client + 80 caracteres por cada nombre
const size_t CLIENT_MESSAGE_SIZE = 2 * sizeof(int16_t) + 80; 


// Metodos

/*void StartServer(const char * s, const char * p){
    Socket socket(s,p);
    socket.bind();
}

void StartClient(const char * s, const char * p){
    Socket socket(s,p);
    socket.bind();
}*/

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
    pacmanLocal->dir=Entity::Direction::STOP;

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
    /*ghostPos[0]=1;
    ghostPos[1]=1;
    ghostDir[0]=G_DOWN;

    ghostPos[2]=width-3;
    ghostPos[3]=1;
    ghostDir[1]=G_DOWN;

    ghostPos[4]=width-3;
    ghostPos[5]=height-2;
    ghostDir[2]=G_UP;*/

    ghosts.push_back(new Entity("Ghost1",1,1));
    ghosts.push_back(new Entity("Ghost2",width-3,1));
    ghosts.push_back(new Entity("Ghost3",width-3,height-2));
    ghosts[0]->dir=Entity::Direction::DOWN;
    ghosts[1]->dir=Entity::Direction::DOWN;
    ghosts[2]->dir=Entity::Direction::UP;

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
            if (pacmanLocal->isAlive && i == pacmanLocal->y && j == pacmanLocal->x){
                init_pair(1, COLOR_YELLOW, COLOR_BLACK);
                attron(COLOR_PAIR(1));
                mvprintw(i, j, "C");  // Dibujar el Pacman
                //mvaddch(i, j, 'C');
                attroff(COLOR_PAIR(1));
            }
            else if (pacmanRemote->isAlive && i == pacmanRemote->y && j == pacmanRemote->x){
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


    mvprintw(height + 1, 0, "Use arrow buttons to move");
    mvprintw(height + 2, 0, "Press q to quit");
    refresh();
}

// Actualizar la posición de los fantasmas
void UpdateGhosts()
{
    // Fantasma 1
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

        // Si hay direcciones disponibles, generar una dirección aleatoria entre las posibles
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

// Actualizar la posición del Pacman
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

    // Verificar si la siguiente posición es un camino despejado
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


// Verificar si se ha alcanzado una condición de finalización del juego
void CheckGameOver()
{
    // Verificar si el Pacman ha sido atrapado por un fantasma
    for(int i=0;i<numGhosts;i++){
        if (pacmanLocal->x == ghosts[i]->x && pacmanLocal->y == ghosts[i]->y){
            pacmanLocal->isAlive=false;
        }
        if (pacmanRemote->x == ghosts[i]->x && pacmanRemote->y == ghosts[i]->y){
            pacmanRemote->isAlive=false;
        }
    }
    if(!pacmanLocal->isAlive&&!pacmanRemote->isAlive) gameOver=true;
    // Verificar si no queda comida
    if(currentFood<=0)gameOver=true;
}

/*void PrepareClientData(char* &buffer){
    pacmanLocal->to_bin();
    memcpy(buffer,pacmanLocal->data(),REMOTE_MESSAGE_SIZE);
}
void PrepareServerData(char* &buffer){
    pacmanLocal->to_bin();
    memcpy(buffer,pacmanLocal->data(),REMOTE_MESSAGE_SIZE);
}*/

void SendDataToClient(int sd){
    char buffer[SERVER_MESSAGE_SIZE];
    pacmanLocal->to_bin();
    // se utiliza CLIENT_MESSAGE_SIZE porque es lo que ocupa los datos
    // de una entidad, de manera que se pueda separar los datos de cada una
    memcpy(buffer,pacmanLocal->data(),CLIENT_MESSAGE_SIZE);
    for(int i=0;i<numGhosts;i++){
        ghosts[i]->to_bin();
        memcpy(buffer+(i+1)*CLIENT_MESSAGE_SIZE,ghosts[i]->data(),CLIENT_MESSAGE_SIZE);
    }
    char aux[2*sizeof(bool)+1];
    sprintf(aux, "%d%d", pacmanRemote->isAlive, gameOver);
    memcpy(buffer +4*CLIENT_MESSAGE_SIZE,aux,2*sizeof(bool));
    send(sd,buffer,SERVER_MESSAGE_SIZE,0);
}
void SendDataToServer(int sd){
    char buffer[CLIENT_MESSAGE_SIZE];
    pacmanLocal->to_bin();
    memcpy(buffer,pacmanLocal->data(),CLIENT_MESSAGE_SIZE);
    send(sd,buffer,CLIENT_MESSAGE_SIZE,0);
}
// Procesa los datos en el servidor recibidos por el cliente
void ProcessClientData(char* buffer, ssize_t bytes){
    if(bytes<=0)return;
    pacmanRemote->from_bin(buffer);
}
// Procesa los datos en el cliente recibidos por el servidor
void ProcessServerData(char* buffer, ssize_t bytes){
    if(bytes<=0)return;
    pacmanRemote->from_bin(buffer);
    for(int i=0;i<numGhosts;i++){
        ghosts[i]->from_bin(buffer+(i+1)*CLIENT_MESSAGE_SIZE);
    }
    int intAux1, intAux2;
    sscanf(buffer+4*CLIENT_MESSAGE_SIZE, "%d%d", &intAux1, &intAux2);
    pacmanLocal->isAlive=static_cast<bool>(intAux1);
    //gameOver=static_cast<bool>(intAux2);
    //cout<<gameOver<<"\n";
}

void ServerGameLogic(){
    Draw();
    Input();
    UpdatePacman();
    UpdateGhosts();
    CheckGameOver();
    usleep(200000);  // Retardo de 200 ms
}

void ClientGameLogic(){
    Draw();
    Input();
    UpdatePacman();
    //UpdateGhosts();
    //CheckGameOver();
    usleep(200000);  // Retardo de 200 ms
}

void ReceiveMessagesFromClient(int sd){
    while(true){
        char buffer[CLIENT_MESSAGE_SIZE];
        ssize_t bytes =  recv(sd,buffer,CLIENT_MESSAGE_SIZE,0);
        if(bytes==0)break;
        ProcessClientData(buffer, bytes);
    }
    close(sd);
}

void ReceiveMessagesFromServer(int sd){
    while(true){
        char buffer[SERVER_MESSAGE_SIZE];
        ssize_t bytes = recv(sd,buffer,SERVER_MESSAGE_SIZE,0);
        if(bytes==0)break;
        ProcessServerData(buffer,bytes);
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
        std::thread receiveThread(ReceiveMessagesFromClient, client_sd);

        // Inicializar variables para servidor
        pacmanLocal=new Entity("PacmanServer",10,1);
        pacmanRemote=new Entity("PacmanClient",13,1);
        Setup();

        while(!gameOver){
            /*char bufferClient[CLIENT_MESSAGE_SIZE];
            ssize_t bytes =  recv(client_sd,bufferClient,CLIENT_MESSAGE_SIZE,0);
            ProcessClientData(bufferClient, bytes);*/
            // ejecutar juego para server
            ServerGameLogic();
            // mandar datos a cliente
            SendDataToClient(client_sd);
            /*char bufferServer[LOCAL_MESSAGE_SIZE];
            pacmanLocal->to_bin();
            memcpy(bufferServer,pacmanLocal->data(),LOCAL_MESSAGE_SIZE);
            send(client_sd,bufferServer,LOCAL_MESSAGE_SIZE,0);*/
        }
        receiveThread.join();
        close(sd);
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
        std::thread receiveThread(ReceiveMessagesFromServer, sd);

        // Inicializar variables para cliente
        pacmanLocal=new Entity("PacmanClient",13,1);
        pacmanRemote=new Entity("PacmanServer",10,1);
        Setup();

        while (!gameOver) {
            ClientGameLogic();
            // mandar datos al server
            SendDataToServer(sd);
            /*char bufferClient[LOCAL_MESSAGE_SIZE];
            pacmanLocal->to_bin();
            memcpy(bufferClient,pacmanLocal->data(),LOCAL_MESSAGE_SIZE);
            send(sd,bufferClient,LOCAL_MESSAGE_SIZE,0);*/
        }
        cout<<"terminado";
        receiveThread.join();
        close(sd);
    }

    

    FreeResources();

    return 0;
}
