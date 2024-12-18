#include <dangerous_dave.h>
#include <screen_helper.h>
#include <libc.h>

// Constantes
#define FPS 30  // Frames por segundo
#define TICKS_PER_FRAME (1000 / FPS)

// {char, BG | FG}
const char WALL[2] = {' ', RED << 4 | RED};
const char EMPTY[2] = {' ', BLACK << 4 | BLACK};
const char TROPHY[2] = {'$', BLACK << 4 | YELLOW};
const char GEM[2] = {'*', BLACK << 4 | GREEN};
const char PLAYER[2] = {'&', BLACK << 4 | LIGHT_BLUE};

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
    "##                      *                               *                     ##",
    "##                  ##########                    ##########                  ##",
    "##                                                                            ##",
    "##                                                                            ##",
    "##            *                                                  *            ##",
    "##        ##########                                        ##########        ##",
    "##                                                                            ##",
    "##                                                                            ##",
    "##                                     *                                      ##",
    "################################################################################",
    "################################################################################"
};

struct Point {
    int x, y;
};

struct Player {
    struct Point p;
};

struct Player player;
int score = 0;

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

void update_thread_func(void *param)
{
    while (1) 
    {
        update_player();
        
        // Pequeña pausa para no saturar la CPU (No se si hace falta!)
        int start_time = gettime();
        while (gettime() - start_time < 20); // Pausa de 20ms
    }
}

void update_player()
{
    map[player.p.y][player.p.x] = ' '; // Limpiar la posición anterior del jugador

    semWait(sem_key); // Bloqueo para leer la tecla de forma segura
    char current_key = key;
    key = '\0'; // Limpiar la tecla
    semSignal(sem_key); // Desbloqueo
    
    // Movimiento del jugador
    if (current_key == 'w' && map[player.p.y - 1][player.p.x] != '#') {
        --player.p.y;
    } else if (current_key == 's' && map[player.p.y + 1][player.p.x] != '#')  {
        ++player.p.y;
    } else if (current_key == 'a' && map[player.p.y][player.p.x - 1] != '#')  {
        --player.p.x;
    } else if (current_key == 'd' && map[player.p.y][player.p.x + 1] != '#')  {
        ++player.p.x;
    }

    // Recolección de premios
    if (map[player.p.y][player.p.x] == '$') {
        score += 10;
    } else if (map[player.p.y][player.p.x] == '*') {
        score += 5;
    }

    map[player.p.y][player.p.x] = '&'; // Actualizar la posición del jugador
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
        render_score();

        // Mantener la velocidad de 30 FPS
        while (gettime() - start_frame_time < TICKS_PER_FRAME);
    }
}



void render_score()
{
    changeColor(GREEN, BLACK);
    char buff1[] = "score: ";
    gotoXY(0, 0);
    write(1, &buff1, sizeof(buff1));
    char buff2[3];
    itodec(score, buff2);
    gotoXY(7, 0);
    write(1, &buff2, sizeof(buff2));
}

void game_loop() 
{
    player.p.x = 40;
    player.p.y = 21;

    sem_key = semCreate(1); // Inicializamos semáforo de la variable 'key'

    // Creación de hilos
    threadCreateWithStack(keyboard_thread_func, 1, NULL);
    threadCreateWithStack(update_thread_func, 1, NULL);
    threadCreateWithStack(render_thread_func, 1, NULL);

    while (1) 
    {
        // Bucle principal vacío (todos los hilos corren de forma independiente)
        int start_frame_time = gettime();
        while (gettime() - start_frame_time < TICKS_PER_FRAME);
    }
}
