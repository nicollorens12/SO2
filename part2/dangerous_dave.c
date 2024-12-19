#include <dangerous_dave.h>
#include <screen_helper.h>
#include <libc.h>

// Constantes
#define FPS 30 
#define TICKS_PER_FRAME (1000 / FPS)
#define JUMP_VELOCITY -2.05f
#define NUM_ROWS 25
#define NUM_COLUMNS 80 

// {char, BG | FG}
const char WALL[2] = {' ', RED << 4 | RED};
const char EMPTY[2] = {' ', BLACK << 4 | BLACK};
const char TROPHY[2] = {'$', BLACK << 4 | YELLOW};
const char GEM[2] = {'*', BLACK << 4 | GREEN};
const char PLAYER[2] = {'&', BLACK << 4 | LIGHT_BLUE};
const char ENEMY[2] = {'+', BLACK << 4 | MAGENTA};
const char GRAVITY_BOOST[2] = {'^', BLACK << 4 | LIGHT_RED};
const char FINISH_DOOR[2] = {'=', LIGHT_CYAN << 4 | YELLOW};
enum game_state {PLAYING, WIN, GAMEOVER, INIT};

// Mapa del juego
char *gameMap;

char *renderMapAux;

char *mapAux;

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
    "##                                                                            ##",
    "##                                 ##########                                 ##",
    "##                                                                            ##",
    "##                                                                            ##",
    "##                         +                             +                    ##",
    "##                  ##########                    ##########                  ##",
    "##                                                                            ##",
    "##                                                                            ##",
    "##                                                                            ##",
    "##        ##########                                        ##########        ##",
    "##                                                                            ##",
    "##                                                                            ##",
    "##                                                                            ##",
    "################################################################################",
    "################################################################################"
};

char win_screen[NUM_ROWS][NUM_COLUMNS] = {
    "                                                                                ",
    "                                                                                ",
    "                                                                                ",
    "                                                                                ",
    "                          ##    ##    ## ####  ##     ##                        ",
    "                          ##   ###   ##   ##   ####   ##                        ",
    "                          ##  ####  ##    ##   ## ##  ##                        ",
    "                          ## ## ## ##     ##   ##  ## ##                        ",
    "                          ####  ####      ##   ##   ####                        ",
    "                          ##    ##       ####  ##     ##                        ",
    "                                                                                ",
    "                                                                                ",
    "                                                                                ",
    "                                                                                ",
    "                                                                                ",
    "                                                                                ",
    "                                                                                ",
    "                                                                                ",
    "                                                                                ",
    "                                                                                ",
    "                                                                                ",
    "                                                                                ",
    "                                                                                ",
    "                                                                                "
};

char gameover_screen[NUM_ROWS][NUM_COLUMNS] = {
    "                                                                                ",
    "                                                                                ",
    "                                                                                ",
    "                                                                                ",
    "                  #########  #########        ##     ## #########               ",
    "                  ##         ##     ##      ####   #### ##                      ",
    "                  ##  #####  ##     ##     ## ##  ## ## ##                      ",
    "                  ##  #  ##  #########    ##  ## ##  ## ######                  ",
    "                  ##     ##  ##     ##   ##   ####   ## ##                      ",
    "                  #########  ##     ##  ##    ##     ## #########               ",
    "                                                                                ",
    "                    #########  ##     ##  #########  #########                  ",
    "                    ##     ##  ##   ##    ##         ##     ##                  ",
    "                    ##     ##  ##  ##     ##         ##     ##                  ",
    "                    ##     ##  ## ##      ######     #########                  ",
    "                    ##     ##  ####       ##         ##   ##                    ",
    "                    #########  ##         #########  ##     ##                  ",
    "                                                                                ",
    "                                                                                ",
    "                                                                                ",
    "                                                                                ",
    "                                                                                ",
    "                                                                                ",
    "                                                                                "
};

char init_screen[NUM_ROWS][NUM_COLUMNS] = {
    "                                                                                ",
    "                                                                                ",
    "                                                                                ",
    "                                                                                ",
    "                    #########  #########    #####     ########                  ",
    "                         ###   ##         ##     ##  ##                         ",
    "                        ##     ##         ##     ##   #######                   ",
    "                       ##      ######     ##     ##         ##                  ",
    "                     ###       ##         ##     ##         ##                  ",
    "                    #########  #########    #####    ########                   ",
    "                                                                                ",
    "                    #######    #########  ##     ##  #########                  ",
    "                    ##    ##   ##     ##  ##   ##    ##                         ",
    "                    ##     ##  ##     ##  ##  ##     ##                         ",
    "                    ##     ##  #########  ## ##      ######                     ",
    "                    ##    ##   ##     ##  ####       ##                         ",
    "                    #######    ##     ##  ##         #########                  ",
    "                                                                                ",
    "                                                                                ",
    "                                                                                ",
    "                                                                                ",
    "                                                                                ",
    "                                                                                ",
    "                                                                                "
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
    int unlocked_platform;
    enum game_state state;
};

struct Enemy enemies[2]; // Dos enemigos en las plataformas

struct Player player;

struct GameStatus gameStatus;

char key = 0; // Variable de tecla compartida
int text_tearing = 0;
float GRAVITY = 0.2f;

void set_boosts(int reset){
    if(reset){
        map[17][15] = '*';
        map[17][65] = '*';  
    }
    map[9][40]  = '$';
    map[13][25] = '^';
    map[13][55] = '^';

}

void init_game(){
    player.p.x = 40;
    player.p.y = 21;
    player.velocityY = 0;
    player.isJumping = 0;

    gameStatus.score = 0;
    gameStatus.lives = 1;
    gameStatus.unlocked_platform = 0;
    lock_finish_slab();

    GRAVITY = 0.2f;
    set_boosts(1);
}

void reset_game(){
    set_boosts(0);
    GRAVITY = 0.2f;
    gameStatus.unlocked_platform = 0;
    gameStatus.score = 0;
}

void keyboard_thread_func(void *param) 
{
    struct sem_t *sem_key = (struct sem_t *)param;
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
        else if(gameStatus.state == GAMEOVER || gameStatus.state == WIN || gameStatus.state == INIT){
            if(c == 'r'){
                gameStatus.state = PLAYING;
                init_game();
            }
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
        if(enemy->dy == -1 && map[enemy->p.y][enemy->p.x + 1] != '#' ){ // Estoy subiendo y puedo ir a la derecha
            enemy->dx = 1;
            enemy->dy = 0;
        } else if(enemy->dx == - 1 && map[enemy->p.y - 1][enemy->p.x] != '#'){ // Estoy por debajo y puedo subir
            enemy->dx = 0;
            enemy->dy = -1;
        } else if(enemy->dy == 1 && map[enemy->p.y][enemy->p.x - 1] != '#' ) { // Estoy bajando y puedo ir a la izquierda
            enemy->dx = -1;
            enemy->dy = 0;
        } else if (enemy->dx == 1 && map[enemy->p.y + 1][enemy->p.x] != '#' ) { // Estoy por encima y puedo bajar
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

void unlock_finish_slab(){
    map[8][68] = '#';
    map[8][69] = '#';
    map[8][70] = '#';
    map[8][71] = '#';
    map[8][72] = '#';
    map[8][73] = '#';
    map[8][74] = '#';
    map[8][75] = '#';
    map[8][76] = '#';

    //Exit door
    map[7][76] = '=';
}
void lock_finish_slab(){
    map[8][68] = ' ';
    map[8][69] = ' ';
    map[8][70] = ' ';
    map[8][71] = ' ';
    map[8][72] = ' ';
    map[8][73] = ' ';
    map[8][74] = ' ';
    map[8][75] = ' ';
    map[8][76] = ' ';
    map[7][76] = ' ';
}



void update_player(struct sem_t *sem_key) {
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
            if(gameStatus.lives > 0) reset_game();
            else gameStatus.state = GAMEOVER;
            break;
        }
    }

    if (map[player.p.y][player.p.x] == '$') {
        gameStatus.score += 10;
        gameStatus.unlocked_platform = 1;
    } 
    else if (map[player.p.y][player.p.x] == '*') {
        gameStatus.score += 5;
    }
    else if(map[player.p.y][player.p.x] == '^'){
        GRAVITY = 0.1f;
    }
    else if(map[player.p.y][player.p.x] == '='){
        gameStatus.state = WIN;
        //init_game();
    }

    map[player.p.y][player.p.x] = '&'; 
}

void update_map() 
{
    if(gameStatus.unlocked_platform == 1){
        unlock_finish_slab();
    } else {
        lock_finish_slab();
    }
}

void update_thread_func(void *param)
{
    struct sem_t *sem_key = (struct sem_t *)param;
    while (1) 
    {
        if(gameStatus.state == PLAYING){
            update_player(sem_key);
            update_enemies();
            update_map();
        }
        
        

        
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
            } else if(map[i][j] == '^'){
                render_map[i][j][0] = GRAVITY_BOOST[0];
                render_map[i][j][1] = GRAVITY_BOOST[1];
            } else if(map[i][j] == '='){
                render_map[i][j][0] = FINISH_DOOR[0];
                render_map[i][j][1] = FINISH_DOOR[1];
            } else {
                render_map[i][j][0] = EMPTY[0];
                render_map[i][j][1] = EMPTY[1];
            }
        }
    }
    clrscr(&render_map[0][0][0]);
}

void render_win_screen()
{
    char render_map[NUM_ROWS][NUM_COLUMNS][2];
    for (int i = 0; i < NUM_ROWS; ++i) 
    {
        for (int j = 0; j < NUM_COLUMNS; ++j) 
        {
            render_map[i][j][0] = win_screen[i][j];
            render_map[i][j][1] = BLACK << 4 | WHITE;
        }
    }

    // Manejar el fin del juego
    clrscr(&render_map[0][0][0]);
}

char get_map_position(char *matrix, int row, int col) {
    return matrix[row * NUM_COLUMNS + col];
}

short int get_render_map_position(char *matrix, int row, int col) {
    return matrix[row * NUM_COLUMNS + col * 2];
}

void render_gameover_screen()
{
    char render_map[NUM_ROWS][NUM_COLUMNS][2];
    for (int i = 0; i < NUM_ROWS; ++i) 
    {
        for (int j = 0; j < NUM_COLUMNS; ++j) 
        {
            render_map[i][j][0] = get_map_position(mapAux, i, j);
            render_map[i][j][1] = BLACK << 4 | WHITE;
        }
    }

    // Manejar el fin del juego
    clrscr(&render_map[0][0][0]);
}

void render_init_screen()
{
    char render_map[NUM_ROWS][NUM_COLUMNS][2];
    for (int i = 0; i < NUM_ROWS; ++i) 
    {
        for (int j = 0; j < NUM_COLUMNS; ++j) 
        {
             render_map[i][j][0] = init_screen[i][j];
             render_map[i][j][1] = BLACK << 4 | WHITE;
            //*get_render_map_position(renderMapAux, i, j) = RED << 12 | YELLOW | '#';
            //renderMapAux[i * NUM_COLUMNS * 2 + j * 2] = RED << 12 | YELLOW << 8 | '#';
        }
    }

    // Manejar el fin del juego
    clrscr(&render_map[0][0][0]);
    //clrscr(renderMapAux);
}

void render_score_text()
{
    gotoXY(36, 14);
    changeColor(YELLOW, BLACK);

    char message[] = "Score: ";
    write(1, &message, sizeof(message));

    char buff[3] = "   ";
    itodec(gameStatus.score, buff);
    write(1, &buff, sizeof(buff));
}


void render_restart_text()
{
    if (text_tearing < 10)
    {
        char message[] = "Press R to start new game";
        gotoXY(28, 19);
        changeColor(GREEN, BLACK);
        write(1, &message, sizeof(message));
    }

    ++text_tearing;
    if (text_tearing > 14)
        text_tearing = 0;
}

void render_thread_func(void *param) 
{
    while (1) 
    {
        int start_frame_time = gettime();
        //if(1) render_init_screen();
        
        if(gameStatus.state == PLAYING)
        {
           render_map();
           render_game_status();
        }
        else if(gameStatus.state == WIN)
        {
            render_win_screen();
            render_score_text();
            render_restart_text();
        }
        else if(gameStatus.state == GAMEOVER)
        {
            render_gameover_screen();
            render_restart_text();           
        }
        else{
            // Pantalla de inicio
            render_init_screen();
            render_restart_text();
        }

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
    gameStatus.state = INIT;


    mapAux = memRegGet(10);
    if(mapAux == NULL){
        return;
    }

    renderMapAux = memRegGet(20); // 8
    if (renderMapAux == NULL)
        return;

    for (int i = 0; i < NUM_ROWS; i++) {
        for (int j = 0; j < NUM_COLUMNS; j++) {
            mapAux[i * NUM_COLUMNS + j] = '.'; // Rellenamos con puntos ('.')
        }
    }
    //*get_map_position(17, 15) = '*';


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

    struct sem_t *sem_key = semCreate(1); 

    threadCreateWithStack(keyboard_thread_func, 1, sem_key);
    threadCreateWithStack(update_thread_func, 1, sem_key);
    threadCreateWithStack(render_thread_func, 1, NULL);

    while (1);
}



