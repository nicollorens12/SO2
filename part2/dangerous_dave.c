#include <dangerous_dave.h>
#include <screen_helper.h>
#include <libc.h>

// Constantes
#define FPS 30  // Frames por segundo
#define TICKS_PER_FRAME (1000 / FPS)
#define GRAVITY 0.2f // Fuerza de la gravedad
#define JUMP_VELOCITY -2.05f // Velocidad inicial del salto

// {char, BG | FG}
const char WALL[2] = {' ', RED << 4 | RED};
const char EMPTY[2] = {' ', BLACK << 4 | BLACK};
const char TROPHY[2] = {'$', BLACK << 4 | YELLOW};
const char GEM[2] = {'*', BLACK << 4 | GREEN};
const char PLAYER[2] = {'&', BLACK << 4 | LIGHT_BLUE};
const char ENEMY[2] = {'+', MAGENTA << 4 | MAGENTA};

// Mapa del juego
char map[NUM_ROWS][NUM_COLUMNS] = {
    "################################################################################",
    "################################################################################",
    "##                                                                            ##",
    "##                                                                            ##",
    "##                                                                            ##",
    "##                                                                            ##",
    "##                                                                            ##",
    "##                                                                            ##",
    "##                                                                            ##",
    "##                                      $                                     ##",
    "##                                 ##########                                 ##",
    "##                                                                            ##",
    "##                                                                            ##",
    "##                      *  +                           * +                    ##",
    "##                  ##########                    ##########                  ##",
    "##                                                                            ##",
    "##                                                                            ##",
    "##            *                                                  *            ##",
    "##        ##########                                        ##########        ##",
    "##                                                                            ##",
    "##                                                                            ##",
    "##                                                                            ##",
    "################################################################################",
    "################################################################################"
};

struct Point {
    int x, y;
};

struct Player {
    struct Point p;
    float velocityY; // Velocidad vertical para el salto
    int isJumping;   // Indicador de si el jugador está saltando
};

struct Enemy {
    struct Point p;    // Posición (x, y) del enemigo
    int dx, dy;        // Dirección de movimiento: dx = (-1 o 1), dy = (-1, 0 o 1)
    char originalChar; // Contenido original de la celda del mapa
    int tickCounter;   // Contador de ticks para ralentizar el movimiento
};

struct GameStatus {
    int score;
    int lives;
};

struct Enemy enemies[2]; // Dos enemigos en las plataformas

struct Player player;

struct GameStatus gameStatus;

char key = 0; // Variable de tecla compartida
struct sem_t *sem_key; // Semáforo para proteger la variable 'key'

void keyboard_thread_func(void *param) 
{
    while (1) 
    {
        char c;
        getKey(&c, 1); // Lee la tecla sin bloqueo
        if (c == 'w' || c == 's' || c == 'a' || c == 'd') 
        {
            semWait(sem_key);
            key = c; // Actualizamos la tecla global de forma segura
            c = '\0'; // Limpiar la tecla
            semSignal(sem_key);
        }
    }
}

void update_enemies() {
    for (int i = 0; i < 2; i++) {
        struct Enemy *enemy = &enemies[i];

        // Reducir el contador de ticks
        if (enemy->tickCounter > 0) {
            enemy->tickCounter--;
            continue; // Salta el movimiento hasta que el contador llegue a 0
        }

        // Reiniciar el contador para el próximo movimiento
        enemy->tickCounter = 5; // Ajusta este valor para cambiar la velocidad

        // Restaurar el contenido original de la celda que el enemigo deja
        map[enemy->p.y][enemy->p.x] = enemy->originalChar;

        // Detectar si el enemigo debe cambiar de dirección
        if(enemy->dy == -1 && map[enemy->p.y][enemy->p.x + 1] == ' '){ // Estoy subiendo y puedo ir a la derecha
            enemy->dx = 1;
            enemy->dy = 0;
        } else if(enemy->dx == - 1 && map[enemy->p.y - 1][enemy->p.x] == ' '){ // Estoy por debajo y puedo subir
            enemy->dx = 0;  
            enemy->dy = -1;
        } else if(enemy->dy == 1 && map[enemy->p.y][enemy->p.x - 1] == ' ') { // Estoy bajando y puedo ir a la izquierda
            enemy->dx = -1;
            enemy->dy = 0;
        } else if (enemy->dx == 1 && map[enemy->p.y + 1][enemy->p.x] == ' ') { // Estoy por encima y puedo bajar
            enemy->dx = 0;
            enemy->dy = 1;
        }

        // Guardar el contenido original de la nueva celda
        enemy->p.x += enemy->dx;
        enemy->p.y += enemy->dy;
        if(map[enemy->p.y][enemy->p.x] != '&')
            enemy->originalChar = map[enemy->p.y][enemy->p.x];
        else
            enemy->originalChar = ' ';
        // Colocar el enemigo en la nueva posición
        map[enemy->p.y][enemy->p.x] = '+';
    }
}



void update_player() {
    map[player.p.y][player.p.x] = ' '; 

    semWait(sem_key);
    char current_key = key;
    key = '\0'; // Limpiar la tecla
    semSignal(sem_key); 
    
    // Movimiento del jugador en eje X
    if (current_key == 'a' && map[player.p.y][player.p.x - 1] != '#')  {
        --player.p.x;
    } 
    else if (current_key == 'd' && map[player.p.y][player.p.x + 1] != '#')  {
        ++player.p.x;
    } 
    else if (current_key == 'w' && player.isJumping == 0) { 
        // Salto
        player.velocityY = JUMP_VELOCITY; 
        player.isJumping = 1;
    }

    // Aplicar gravedad y movimiento vertical
    player.velocityY += GRAVITY; // Aumentar la velocidad con la gravedad
    int new_y = player.p.y + (int)player.velocityY;

    if (new_y > player.p.y) { 
        // Movimiento hacia abajo (caída)
        for (int y = player.p.y; y <= new_y; ++y) {
            if (map[y][player.p.x] == '#') {
                player.velocityY = 0;
                player.p.y = y - 1; // Posiciona al jugador sobre la plataforma
                player.isJumping = 0; // El jugador aterriza
                break;
            }
            player.p.y = y;
        }
    } 
    else if (new_y < player.p.y) { 
        // Movimiento hacia arriba (salto)
        for (int y = player.p.y; y >= new_y; --y) {
            if (map[y][player.p.x] == '#') {
                player.velocityY = 0;
                break;
            }
            player.p.y = y;
        }
    }

    // Verificar colisión con enemigos
    for (int i = 0; i < 2; i++) {
        if (player.p.x == enemies[i].p.x && player.p.y == enemies[i].p.y) {
            gameStatus.lives--; // Reducir vidas
            if (gameStatus.lives > 0) {
                map[player.p.y][player.p.x] = ' ';
                player.p.x = 40;
                player.p.y = 21;
                player.velocityY = 0;
                player.isJumping = 0;
            } else {
                // Manejar el fin del juego
                clrscr(NULL);
                char message[] = "Game Over!";
                gotoXY(35, 12);
                write(1, &message, sizeof(message));
                while (1); // Detener el juego
            }
            break;
        }
    }

    if (map[player.p.y][player.p.x] == '$') {
        gameStatus.score += 10;
    } else if (map[player.p.y][player.p.x] == '*') {
        gameStatus.score += 5;
    }

    map[player.p.y][player.p.x] = '&'; 
}



void update_thread_func(void *param)
{
    while (1) 
    {
        update_player();
        update_enemies();
        // Pequeña pausa para no saturar la CPU
        int start_time = gettime();
        while (gettime() - start_time < 20); // Pausa de 20ms (REVISAR)
    }
}


void render_map() 
{
    char render_map[NUM_ROWS][NUM_COLUMNS][2];
    for (int i = 0; i < NUM_ROWS; ++i) 
    {
        for (int j = 0; j < NUM_COLUMNS; ++j) 
        {
            if (map[i][j] == '#') {
                render_map[i][j][0] = WALL[0];
                render_map[i][j][1] = WALL[1];  
            } else if (map[i][j] == '$') {
                render_map[i][j][0] = TROPHY[0];
                render_map[i][j][1] = TROPHY[1];  
            } else if (map[i][j] == '*') {
                render_map[i][j][0] = GEM[0];
                render_map[i][j][1] = GEM[1];  
            } else if (map[i][j] == '&') {
                render_map[i][j][0] = PLAYER[0];
                render_map[i][j][1] = PLAYER[1];
            } else if (map[i][j] == '+') {
                render_map[i][j][0] = ENEMY[0];
                render_map[i][j][1] = ENEMY[1];
            } else {
                render_map[i][j][0] = EMPTY[0];
                render_map[i][j][1] = EMPTY[1];
            }
        }
    }
    clrscr(&render_map[0][0][0]);
}


void render_thread_func(void *param) 
{
    while (1) 
    {
        int start_frame_time = gettime();
        
        render_map();
        render_game_status();

        while (gettime() - start_frame_time < TICKS_PER_FRAME);
    }
}

void render_game_status()
{
    //Quizas hacer un condicional para actualizar solo cuando ha cambiado score o lives
    changeColor(GREEN, RED);
    char buff1[] = "score: ";
    gotoXY(0, 0);
    write(1, &buff1, sizeof(buff1));
    char buff2[3];
    itodec(gameStatus.score, buff2);
    gotoXY(7, 0);
    write(1, &buff2, sizeof(buff2));

    char buff3[] = "lives: ";
    gotoXY(0, 1);
    write(1, &buff3, sizeof(buff3));
    char buff4[3];
    itodec(gameStatus.lives, buff4);
    gotoXY(7, 1);
    write(1, &buff4, sizeof(buff4));
}

void game_loop() 
{
    player.p.x = 40;
    player.p.y = 21;
    player.velocityY = 0;
    player.isJumping = 0;

    gameStatus.score = 0;
    gameStatus.lives = 1;

    // Inicializar enemigos
    int enemy_index = 0;
    for (int i = 0; i < NUM_ROWS; i++) {
        for (int j = 0; j < NUM_COLUMNS; j++) {
            if (map[i][j] == '+' && enemy_index < 2) { 
                enemies[enemy_index].p.x = j;
                enemies[enemy_index].p.y = i;
                enemies[enemy_index].dx = 1;  // Empieza moviéndose a la derecha
                enemies[enemy_index].dy = 0;  
                enemies[enemy_index].originalChar = ' '; // Asume que la celda inicial es un espacio vacío
                
                enemies[enemy_index].tickCounter = 5; // AJUSTAR EL VALOR PARA CAMBIAR LA VELOCIDAD DE LOS ENEMIGOS

                map[i][j] = ' '; 
                enemy_index++;
            }
        }
    }

    sem_key = semCreate(1); 

    threadCreateWithStack(keyboard_thread_func, 1, NULL);
    threadCreateWithStack(update_thread_func, 1, NULL);
    threadCreateWithStack(render_thread_func, 1, NULL);

    while (1);
}



