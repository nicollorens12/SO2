#include <dangerous_dave.h>
#include <screen_helper.h>
#include <libc.h>

/* ------ */
/* PLAYER */
/* ------ */

struct point_t 
{
    uint x, y;
}

struct player_t
{
    struct point_t pos_x;
    int lifes;
}

struct player_t player;

struct player_t* create_player(int pos_x, int pos_y)
{
    player_t* p = &player;
    p->pos_y = pos_y;
    p->pos_x = pos_x;

    return p;
}

int move_player(struct player_t *player, int x, int y)
{
    if (x < 0 || y < 0 || x > NUM_ROWS || y > NUM_COLUMNS)
        return -1; // error posicion fuera de rango
    // obstaculo -->
    //if (x)

    // puntuar
}

/* ------ */
/* ------ */
/* ------ */

/* ------ */
/* PANTALLA */
/* ------ */
void screen_clear_position(struct point_t *p)
{

}

void screen_draw_position(struct point_t *p)
{
    gotoXY(p->x, p->y);

    //changeColor(fg, bg);

}

/* ------ */
/* ------ */
/* ------ */


// {char, BG | FG}
const char WALL[2] = {' ', RED << 4 | RED};
const char EMPTY[2] = {' ', BLACK << 4 | BLACK};
const char TROPHY[2] = {'$', BLACK << 4 | YELLOW};
const char GEM[2] = {'*', BLACK << 4 | GREEN};

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


/* ------ */
/* ------ */
/* ------ */

// run the game
void dangerous_dave()
{
    // vars
    struct player_t player = init_player();

    while (true)
    {

    }
}

/* ------ */
/* STATIC */
/* ------ */

void draw_map()
{
	char test_matrix[NUM_ROWS][NUM_COLUMNS][2];


	for (int i = 0; i < NUM_ROWS; ++i)
  	{
    	for (int j = 0; j < NUM_COLUMNS; ++j)
    	{
    	  	if (map[i][j] == '#')
    	  	{
    	  	  test_matrix[i][j][0] = WALL[0];
    	  	  test_matrix[i][j][1] = WALL[1];  
    	  	}
    	  	else if (map[i][j] == '$')
    	  	{
    	  	  test_matrix[i][j][0] = TROPHY[0];
    	  	  test_matrix[i][j][1] = TROPHY[1];  
    	  	}
    	  	else if (map[i][j] == '*')
    	  	{
    	  	  test_matrix[i][j][0] = GEM[0];
    	  	  test_matrix[i][j][1] = GEM[1];  
    	  	}
    	  	else if (map[i][j] == ' ')
    	  	{
    	  	  test_matrix[i][j][0] = EMPTY[0];
    	  	  test_matrix[i][j][1] = EMPTY[1];  
    	  	}
    	  	else
    	  	{
    	  	  test_matrix[i][j][0] = EMPTY[0];
    	  	  test_matrix[i][j][1] = EMPTY[1];   
    	  	}
	
	    	  
    	}
  	}


  	clrscr(&test_matrix[0][0][0]);
}

void draw_player()
{
	gotoXY(40,21);
  	//changeColor(YELLOW, RED);
  	changeColor(LIGHT_BLUE, BLACK);

  	char buff = '&';
  	write(1, &buff, sizeof(buff));
}



/* ------ */
/* ------ */
