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
char *winScreen;
char *gameoverScreen;
char *initScreen;

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

Slab *player_slab;

// Slab para GameStatus
Slab *gameStatus_slab;

struct Player *player = NULL;
struct GameStatus *gameStatus = NULL;


char key = 0; // Variable de tecla compartida
int text_tearing = 0;
float GRAVITY = 0.2f;

char get_map_position(char *matrix, int row, int col) {
    return matrix[row * NUM_COLUMNS + col];
}

void set_boosts(int reset){
    if(reset){
        gameMap[17 * NUM_COLUMNS + 15] = '*';
        gameMap[17 * NUM_COLUMNS + 65] = '*';
    }
    gameMap[9 * NUM_COLUMNS + 40] = '$';
    gameMap[13 * NUM_COLUMNS + 25] = '^';
    gameMap[13 * NUM_COLUMNS + 55] = '^';

}

void init_game(){
    player->p.x = 40;
    player->p.y = 20;
    player->velocityY = 0;
    player->isJumping = 0;

    gameStatus->score = 0;
    gameStatus->lives = 1;
    gameStatus->unlocked_platform = 0;
    lock_finish_slab();

    GRAVITY = 0.2f;
    set_boosts(1);
}

void reset_game(){
    set_boosts(0);
    GRAVITY = 0.2f;
    gameStatus->unlocked_platform = 0;
    gameStatus->score = 0;
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
        else if(gameStatus->state == GAMEOVER || gameStatus->state == WIN || gameStatus->state == INIT){
            if(c == 'r'){
                gameStatus->state = PLAYING;
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

        gameMap[enemy->p.y * NUM_COLUMNS + enemy->p.x] = enemy->originalChar;
        // Detectar si el enemigo debe cambiar de dirección
        if(enemy->dy == -1 && gameMap[enemy->p.y * NUM_COLUMNS + enemy->p.x + 1] != '#' ){ // Estoy subiendo y puedo ir a la derecha
            enemy->dx = 1;
            enemy->dy = 0;
        } else if(enemy->dx == - 1 && gameMap[(enemy->p.y - 1) * NUM_COLUMNS + enemy->p.x] != '#'){ // Estoy por debajo y puedo subir
            enemy->dx = 0;
            enemy->dy = -1;
        } else if(enemy->dy == 1 && gameMap[enemy->p.y * NUM_COLUMNS + enemy->p.x - 1] != '#' ) { // Estoy bajando y puedo ir a la izquierda
            enemy->dx = -1;
            enemy->dy = 0;
        } else if (enemy->dx == 1 && gameMap[(enemy->p.y + 1) * NUM_COLUMNS + enemy->p.x] != '#' ) { // Estoy por encima y puedo bajar
            enemy->dx = 0;
            enemy->dy = 1;
        }

        // Guardar el contenido original de la nueva celda
        enemy->p.x += enemy->dx;
        enemy->p.y += enemy->dy;
        if(gameMap[enemy->p.y * NUM_COLUMNS + enemy->p.x] != '&')
            enemy->originalChar = gameMap[enemy->p.y * NUM_COLUMNS + enemy->p.x];
        else
            enemy->originalChar = ' ';
        // Colocar el enemigo en la nueva posición
        gameMap[enemy->p.y * NUM_COLUMNS + enemy->p.x] = '+';
    }
}

void unlock_finish_slab(){
    gameMap[8 * NUM_COLUMNS + 68] = '#';
    gameMap[8 * NUM_COLUMNS + 69] = '#';
    gameMap[8 * NUM_COLUMNS + 70] = '#';
    gameMap[8 * NUM_COLUMNS + 71] = '#';
    gameMap[8 * NUM_COLUMNS + 72] = '#';
    gameMap[8 * NUM_COLUMNS + 73] = '#';
    gameMap[8 * NUM_COLUMNS + 74] = '#';
    gameMap[8 * NUM_COLUMNS + 75] = '#';
    gameMap[8 * NUM_COLUMNS + 76] = '#';

    //Exit door
    gameMap[7 * NUM_COLUMNS + 76] = '=';
}
void lock_finish_slab(){
    gameMap[8 * NUM_COLUMNS + 68] = ' ';
    gameMap[8 * NUM_COLUMNS + 69] = ' ';
    gameMap[8 * NUM_COLUMNS + 70] = ' ';
    gameMap[8 * NUM_COLUMNS + 71] = ' ';
    gameMap[8 * NUM_COLUMNS + 72] = ' ';
    gameMap[8 * NUM_COLUMNS + 73] = ' ';
    gameMap[8 * NUM_COLUMNS + 74] = ' ';
    gameMap[8 * NUM_COLUMNS + 75] = ' ';
    gameMap[8 * NUM_COLUMNS + 76] = ' ';
    gameMap[7 * NUM_COLUMNS + 76] = ' ';
}



void update_player(struct sem_t *sem_key) {
    gameMap[player->p.y * NUM_COLUMNS + player->p.x] = ' '; 

    semWait(sem_key);
    char current_key = key;
    key = '\0'; // Limpiar la tecla
    semSignal(sem_key); 
    
    // Movimiento del jugador en eje X
    if (current_key == 'a' && gameMap[player->p.y * NUM_COLUMNS +  player->p.x - 1] != '#')  {
        --player->p.x;
    } 
    else if (current_key == 'd' && gameMap[player->p.y * NUM_COLUMNS + player->p.x + 1] != '#')  {
        ++player->p.x;
    } 
    else if (current_key == 'w' && player->isJumping == 0) { 
        // Salto
        player->velocityY = JUMP_VELOCITY; 
        player->isJumping = 1;
    }

    // Aplicar gravedad y movimiento vertical
    player->velocityY += GRAVITY; // Aumentar la velocidad con la gravedad
    int new_y = player->p.y + (int)player->velocityY;

    if (new_y > player->p.y) { 
        // Movimiento hacia abajo (caída)
        for (int y = player->p.y; y <= new_y; ++y) {
            if (gameMap[y*NUM_COLUMNS + player->p.x] == '#') {
                player->velocityY = 0;
                player->p.y = y - 1; // Posiciona al jugador sobre la plataforma
                player->isJumping = 0; // El jugador aterriza
                break;
            }
            player->p.y = y;
        }
    } 
    else if (new_y < player->p.y) { 
        // Movimiento hacia arriba (salto)
        for (int y = player->p.y; y >= new_y; --y) {
            if (gameMap[y*NUM_COLUMNS + player->p.x] == '#') {
                player->velocityY = 0;
                break;
            }
            player->p.y = y;
        }
    }

    // Verificar colisión con enemigos
    for (int i = 0; i < 2; i++) {
        if (player->p.x == enemies[i].p.x && player->p.y == enemies[i].p.y) {
            gameStatus->lives--; // Reducir vidas
            if(gameStatus->lives > 0) reset_game();
            else gameStatus->state = GAMEOVER;
            break;
        }
    }

    if (gameMap[player->p.y*NUM_COLUMNS + player->p.x] == '$') {
        gameStatus->score += 10;
        gameStatus->unlocked_platform = 1;
    } 
    else if (gameMap[player->p.y*NUM_COLUMNS + player->p.x] == '*') {
        gameStatus->score += 5;
    }
    else if(gameMap[player->p.y*NUM_COLUMNS + player->p.x] == '^'){
        GRAVITY = 0.1f;
    }
    else if(gameMap[player->p.y*NUM_COLUMNS + player->p.x] == '='){
        gameStatus->state = WIN;
        //init_game();
    }

    gameMap[player->p.y * NUM_COLUMNS + player->p.x] = '&'; 
}

void update_map() 
{
    if(gameStatus->unlocked_platform == 1){
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
        if(gameStatus->state == PLAYING){
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
            if (get_map_position(gameMap, i, j) == '#') {
                render_map[i][j][0] = WALL[0];
                render_map[i][j][1] = WALL[1];  
            } else if (get_map_position(gameMap, i, j) == '$') {
                render_map[i][j][0] = TROPHY[0];
                render_map[i][j][1] = TROPHY[1];  
            } else if (get_map_position(gameMap, i, j) == '*') {
                render_map[i][j][0] = GEM[0];
                render_map[i][j][1] = GEM[1];  
            } else if (get_map_position(gameMap, i, j) == '&') {
                render_map[i][j][0] = PLAYER[0];
                render_map[i][j][1] = PLAYER[1];
            } else if (get_map_position(gameMap, i, j) == '+') {
                render_map[i][j][0] = ENEMY[0];
                render_map[i][j][1] = ENEMY[1];
            } else if(get_map_position(gameMap, i, j) == '^'){
                render_map[i][j][0] = GRAVITY_BOOST[0];
                render_map[i][j][1] = GRAVITY_BOOST[1];
            } else if(get_map_position(gameMap, i, j) == '='){
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
            render_map[i][j][0] = get_map_position(winScreen, i, j);
            render_map[i][j][1] = BLACK << 4 | WHITE;
        }
    }

    // Manejar el fin del juego
    clrscr(&render_map[0][0][0]);
}



void render_gameover_screen()
{
    char render_map[NUM_ROWS][NUM_COLUMNS][2];
    for (int i = 0; i < NUM_ROWS; ++i) 
    {
        for (int j = 0; j < NUM_COLUMNS; ++j) 
        {
            render_map[i][j][0] = get_map_position(gameoverScreen, i, j);
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
            render_map[i][j][0] = get_map_position(initScreen, i, j);
            render_map[i][j][1] = BLACK << 4 | WHITE;
        }
    }

    // Manejar el fin del juego
    clrscr(&render_map[0][0][0]);
}

void render_score_text()
{
    gotoXY(36, 14);
    changeColor(YELLOW, BLACK);

    char message[] = "Score: ";
    write(1, &message, sizeof(message));

    char buff[3] = "   ";
    itodec(gameStatus->score, buff);
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

        if(gameStatus->state == PLAYING)
        {
           render_map();
           render_game_status();
        }
        else if(gameStatus->state == WIN)
        {
            render_win_screen();
            render_score_text();
            render_restart_text();
        }
        else if(gameStatus->state == GAMEOVER)
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
    // Quizas hacer un condicional para actualizar solo cuando ha cambiado score o lives
    changeColor(GREEN, RED);
    char buff1[] = "score: ";
    gotoXY(0, 0);
    write(1, &buff1, sizeof(buff1));
    char buff2[3] = "   ";
    itodec(gameStatus->score, buff2);
    gotoXY(7, 0);
    write(1, &buff2, sizeof(buff2));

    char buff3[] = "lives: ";
    gotoXY(0, 1);
    write(1, &buff3, sizeof(buff3));
    char buff4[3];
    itodec(gameStatus->lives, buff4);
    gotoXY(7, 1);
    write(1, &buff4, sizeof(buff4));
}

void generate_game_map(){
    for (int i = 0; i < NUM_ROWS; i++) {
        for (int j = 0; j < NUM_COLUMNS; j++) {
            if( j == 0 || j == 1 || j == NUM_COLUMNS - 1 || j == NUM_COLUMNS - 2|| 
                i == 0 || i == 1 || i == NUM_ROWS - 1    || i == NUM_ROWS - 2   || i == NUM_ROWS - 3  ) { //Paredes
                gameMap[i * NUM_COLUMNS + j] = '#';
            }
            else if(i == 18 && ((j > 10 && j < 19) || ( j > 60 && j < 69)) ){ // Plataformas de abajo
                gameMap[i * NUM_COLUMNS + j] = '#';
            }
            else if(i == 14 && ((j > 21 && j < 30) || (j > 49 && j < 58 )) ){ //Plataformas medias
                gameMap[i * NUM_COLUMNS + j] = '#';
            }
            else if(i == 10 && (j > 35 && j < 45)){ //Plataforma superior
                gameMap[i * NUM_COLUMNS + j] = '#';
            }
            else if(i == 13 && (j == 27 || j == 56)){ // Enemigos
                gameMap[i * NUM_COLUMNS + j] = '+';
            }
            else gameMap[i * NUM_COLUMNS + j] = ' '; 
        }
    }
}

void generate_init_screen() {
    for (int i = 0; i < NUM_ROWS; i++) {
        for (int j = 0; j < NUM_COLUMNS; j++) {
            // Bordes de la pantalla
            
            // Letra Z
            if (i >= 4 && i <= 8 && j >= 4 && j <= 10) {
                if (i == 4 || i == 8 || i + j == 14) initScreen[i * NUM_COLUMNS + j] = '#';
                else initScreen[i * NUM_COLUMNS + j] = ' ';
            } 
            // Letra e
            else if (i >= 4 && i <= 8 && j >= 12 && j <= 18) {
                if (i == 4 || i == 8 || i == 6 || j == 12) initScreen[i * NUM_COLUMNS + j] = '#';
                else initScreen[i * NUM_COLUMNS + j] = ' ';
            } 
            // Letra o
            else if (i >= 4 && i <= 8 && j >= 20 && j <= 26) {
                if (i == 4 || i == 8 || j == 20 || j == 26) initScreen[i * NUM_COLUMNS + j] = '#';
                else initScreen[i * NUM_COLUMNS + j] = ' ';
            } 
            // Letra S
            else if (i >= 4 && i <= 8 && j >= 28 && j <= 34) {
                if (i == 4 || i == 8 || i == 6 || (j == 28 && i < 6) || (j == 34 && i > 6)) initScreen[i * NUM_COLUMNS + j] = '#';
                else initScreen[i * NUM_COLUMNS + j] = ' ';
            } 
            // Espacio entre "ZeoS" y "DAVE"
            else if (j >= 36 && j <= 38) {
                initScreen[i * NUM_COLUMNS + j] = ' ';
            } 
            // Letra D
            else if (i >= 4 && i <= 8 && j >= 40 && j <= 46) {
                if (j == 40 || i == 4 || i == 8 || (j == 46 && i > 4 && i < 8)) initScreen[i * NUM_COLUMNS + j] = '#';
                else initScreen[i * NUM_COLUMNS + j] = ' ';
            } 
            // Letra A
            else if (i >= 4 && i <= 8 && j >= 48 && j <= 54) {
                if (i == 4 || i == 6 || j == 48 || j == 54) initScreen[i * NUM_COLUMNS + j] = '#';
                else initScreen[i * NUM_COLUMNS + j] = ' ';
            } 
            // Letra V
            else if (i >= 4 && i <= 8 && j >= 56 && j <= 62) {
                if ((j == 56 + (i - 4)) || (j == 62 - (i - 4))) initScreen[i * NUM_COLUMNS + j] = '#';
                else initScreen[i * NUM_COLUMNS + j] = ' ';
            } 
            // Letra E
            else if (i >= 4 && i <= 8 && j >= 64 && j <= 70) {
                if (i == 4 || i == 8 || i == 6 || j == 64) initScreen[i * NUM_COLUMNS + j] = '#';
                else initScreen[i * NUM_COLUMNS + j] = ' ';
            } 
            // Espacio vacío
            else {
                initScreen[i * NUM_COLUMNS + j] = ' ';
            }
        }
    }
}
void generate_win_screen() {
    // Calculamos la posición inicial para centrar horizontalmente
    int j_start = (NUM_COLUMNS - 25) / 2; 

    for (int i = 0; i < NUM_ROWS; i++) {
        for (int j = 0; j < NUM_COLUMNS; j++) {
            
            // Letra W
            if (i >= 4 && i <= 8 && j >= j_start && j < j_start + 7) {
                // Barras laterales
                if (j == j_start || j == j_start + 6) 
                    winScreen[i * NUM_COLUMNS + j] = '#';
                // Diagonales
                //else if (i == 8 && (j == j_start + 2 || j == j_start + 4)) 
                //    winScreen[i * NUM_COLUMNS + j] = '#';
                else if (i == 7 && (j == j_start + 1 || j == j_start + 5)) 
                    winScreen[i * NUM_COLUMNS + j] = '#';
                else if (i == 6 && (j == j_start + 2 || j == j_start + 4)) 
                    winScreen[i * NUM_COLUMNS + j] = '#';
                else 
                    winScreen[i * NUM_COLUMNS + j] = ' ';
            } 
            
            // Letra I
            else if (i >= 4 && i <= 8 && j >= j_start + 9 && j < j_start + 16) {
                if (i == 4 || i == 8 || j == j_start + 12) 
                    winScreen[i * NUM_COLUMNS + j] = '#';
                else 
                    winScreen[i * NUM_COLUMNS + j] = ' ';
            } 
            
            // Letra N
            else if (i >= 4 && i <= 8 && j >= j_start + 18 && j < j_start + 25) {
                // Barras laterales
                if (j == j_start + 18 || j == j_start + 24) 
                    winScreen[i * NUM_COLUMNS + j] = '#';
                // Diagonal de la N (Corregido)
                else if (j == j_start + 18 + (i - 4)) 
                    winScreen[i * NUM_COLUMNS + j] = '#';
                else 
                    winScreen[i * NUM_COLUMNS + j] = ' ';
            } 
            
            // Resto de la pantalla
            else {
                winScreen[i * NUM_COLUMNS + j] = ' ';
            }
        }
    }
}



/**
 * @brief Genera la pantalla de GAME OVER
 */
void generate_gameover_screen() {
    for (int i = 0; i < NUM_ROWS; i++) {
        for (int j = 0; j < NUM_COLUMNS; j++) {
            
            // Letra G
            if (i >= 4 && i <= 8 && j >= 4 && j <= 10) {
                if (i == 4 || i == 8 || j == 4 || (j == 10 && i >= 6) || (i == 6 && j >= 8)) gameoverScreen[i * NUM_COLUMNS + j] = '#';
                else gameoverScreen[i * NUM_COLUMNS + j] = ' ';
            } 
            // Letra A
            else if (i >= 4 && i <= 8 && j >= 12 && j <= 18) {
                if (i == 4 || i == 6 || j == 12 || j == 18) gameoverScreen[i * NUM_COLUMNS + j] = '#';
                else gameoverScreen[i * NUM_COLUMNS + j] = ' ';
            } 
            // Letra M
            else if (i >= 4 && i <= 8 && j >= 20 && j <= 26) {
                if (j == 20 || j == 26 || 
                    (i == 4 && (j == 21 || j == 25)) || 
                    (i == 5 && (j == 22 || j == 24)) || 
                    (i == 6 && j == 23)) 
                {
                    gameoverScreen[i * NUM_COLUMNS + j] = '#';
                } 
                else {
                    gameoverScreen[i * NUM_COLUMNS + j] = ' ';
                }
            }

            // Letra E
            else if (i >= 4 && i <= 8 && j >= 28 && j <= 34) {
                if (i == 4 || i == 8 || i == 6 || j == 28) gameoverScreen[i * NUM_COLUMNS + j] = '#';
                else gameoverScreen[i * NUM_COLUMNS + j] = ' ';
            } 
            // Letra O
            else if (i >= 4 && i <= 8 && j >= 40 && j <= 46) {
                if (i == 4 || i == 8 || j == 40 || j == 46) gameoverScreen[i * NUM_COLUMNS + j] = '#';
                else gameoverScreen[i * NUM_COLUMNS + j] = ' ';
            } 
            // Letra V
            else if (i >= 4 && i <= 8 && j >= 48 && j <= 54) {
                if ((j == 48 + (i - 4)) || (j == 54 - (i - 4))) gameoverScreen[i * NUM_COLUMNS + j] = '#';
                else gameoverScreen[i * NUM_COLUMNS + j] = ' ';
            } 
            // Letra E
            else if (i >= 4 && i <= 8 && j >= 56 && j <= 62) {
                if (i == 4 || i == 8 || i == 6 || j == 56) gameoverScreen[i * NUM_COLUMNS + j] = '#';
                else gameoverScreen[i * NUM_COLUMNS + j] = ' ';
            } 
            // Letra R
            else if (i >= 4 && i <= 8 && j >= 64 && j <= 70) {
                if (j == 64 || // Barra vertical izquierda
                    i == 4 || // Parte superior de la R
                    i == 6 || // Barra horizontal intermedia
                    (j == 70 && i <= 6) || // Parte derecha superior de la R
                    (i >= 6 && j == 64 + (i - 6))) // Diagonal de la R
                {
                    gameoverScreen[i * NUM_COLUMNS + j] = '#';
                } 
                else {
                    gameoverScreen[i * NUM_COLUMNS + j] = ' ';
                }
            }

            else {
                gameoverScreen[i * NUM_COLUMNS + j] = ' ';
            }
        }
    }
}

struct Player *alloc_player() {
    return (struct Player *)slab_alloc(player_slab);
}

void free_player(struct Player *p) {
    slab_free(player_slab, p);
}

struct GameStatus *alloc_game_status() {
    return (struct GameStatus *)slab_alloc(gameStatus_slab);
}

void free_game_status(struct GameStatus *gs) {
    slab_free(gameStatus_slab, gs);
}


void init_slabs() {
    player_slab = slab_create(sizeof(struct Player));

    gameStatus_slab = slab_create(sizeof(struct GameStatus));
}


void game_loop() 
{
    init_slabs();
    player = alloc_player();
    gameStatus = alloc_game_status();

    gameStatus->state = INIT;

    gameMap = memRegGet(4); // 4 paginas es algo mes dels 80x25x8 bytes que ocupa la matriu
    if(gameMap == NULL){
        return;
    }
    generate_game_map();

    initScreen = memRegGet(4);
    if(initScreen == NULL){
        return;
    }
    generate_init_screen();

    winScreen = memRegGet(4);
    if(winScreen == NULL){
        return;
    }
    generate_win_screen();

    gameoverScreen = memRegGet(4);
    if(gameoverScreen == NULL){
        return;
    }
    generate_gameover_screen();

    init_slabs();

    // Asignar player y gameStatus desde los slabs
    
    
    //*get_map_position(17, 15) = '*';


    // Inicializar enemigos
    int enemy_index = 0;
    for (int i = 0; i < NUM_ROWS; i++) {
        for (int j = 0; j < NUM_COLUMNS; j++) {
            if (gameMap[i*NUM_COLUMNS + j] == '+' && enemy_index < 2) { 
                enemies[enemy_index].p.x = j;
                enemies[enemy_index].p.y = i;
                enemies[enemy_index].dx = 1;  // Empieza moviéndose a la derecha
                enemies[enemy_index].dy = 0;  
                enemies[enemy_index].originalChar = ' '; // Asume que la celda inicial es un espacio vacío
                
                enemies[enemy_index].tickCounter = 5; // AJUSTAR EL VALOR PARA CAMBIAR LA VELOCIDAD DE LOS ENEMIGOS

                gameMap[i * NUM_COLUMNS + j] = ' '; 
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



