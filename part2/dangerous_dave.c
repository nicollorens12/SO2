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
    "##                                                                            ##",
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

char key;
struct sem_t *sem_frame;

void keyboard_thread_func(void* param) 
{
    struct sem_t *sem_input = (struct sem_t *)param;
    semWait(sem_input); // Espera a que el ciclo principal lo desbloquee
    while (1) 
    {
        char c;
        getKey(&c, 0);
        
        // Solo actualizamos la tecla si es válida
        if (c == 'w' || c == 's' || c == 'a' || c == 'd') 
        {
            key = c;
        }
        
        semSignal(sem_frame); // Desbloquea el ciclo principal
        semWait(sem_input);
    }
}

void update_thread_func(void *param)
{
    struct sem_t *sem_update = (struct sem_t *)param;
    semWait(sem_update); // Espera a que el ciclo principal lo desbloquee
    while (1)
    {
        update_player();
        semSignal(sem_frame); // Desbloquea el renderizador
        semWait(sem_update); // Espera a que el render termine
    }
}

void update_player()
{
    map[player.p.y][player.p.x] = ' ';

    if (key == 'w' && map[player.p.y - 1][player.p.x] == ' ') {
        --player.p.y;
    } else if (key == 's' && map[player.p.y + 1][player.p.x] == ' ') {
        ++player.p.y;
    } else if (key == 'a' && map[player.p.y][player.p.x - 1] == ' ') {
        --player.p.x;
    } else if (key == 'd' && map[player.p.y][player.p.x + 1] == ' ') {
        ++player.p.x;
    }

    if (map[player.p.y][player.p.x] == '$') {
        score += 10;
    } else if (map[player.p.y][player.p.x] == '*') {
        score += 5;
    }

    map[player.p.y][player.p.x] = '&';
}

void render_thread_func(void *param) 
{
    struct sem_t *sem_render = (struct sem_t *)param;
    semWait(sem_render); // Espera a que el ciclo principal lo desbloquee
    while (1) 
    {
        render_map();
        render_score();
        semSignal(sem_frame); // Desbloquea el ciclo principal para continuar
        semWait(sem_render); // Espera a que la actualización termine
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
            } else {
                render_map[i][j][0] = EMPTY[0];
                render_map[i][j][1] = EMPTY[1];
            }
        }
    }
    //clrscr(&render_map[0][0][0]);
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

    struct sem_t *sem_input = semCreate(0);
    struct sem_t *sem_update = semCreate(0);
    struct sem_t *sem_render = semCreate(0);
    sem_frame = semCreate(0);



    threadCreateWithStack(keyboard_thread_func, 1, sem_input);
    threadCreateWithStack(update_thread_func, 1, sem_update);
    threadCreateWithStack(render_thread_func, 1, sem_render);

    int start_frame_time;
    while (1) 
    {
        start_frame_time = gettime();
        
        semSignal(sem_input); // Desbloquea la entrada
        semWait(sem_frame); // Espera a que el render termine
        semSignal(sem_update); // Desbloquea la actualización
        semWait(sem_frame); // Espera a que la actualización termine
        semSignal(sem_render); // Desbloquea el renderizador
        semWait(sem_frame); // Espera a que el render termine
        
        while (gettime() - start_frame_time < TICKS_PER_FRAME);
    }
}
