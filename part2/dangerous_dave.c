#include <dangerous_dave.h>
#include <screen_helper.h>
#include <libc.h>

// {char, BG | FG}
const char WALL[2] = {' ', RED << 4 | RED};
const char EMPTY[2] = {' ', BLACK << 4 | BLACK};
const char TROPHY[2] = {'$', BLACK << 4 | YELLOW};
const char GEM[2] = {'*', BLACK << 4 | GREEN};
const char PLAYER[2] = {'&', BLACK << 4 | LIGHT_BLUE};

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

void render_map()
{
	char render_map[NUM_ROWS][NUM_COLUMNS][2];


	for (int i = 0; i < NUM_ROWS; ++i)
  	{
    	for (int j = 0; j < NUM_COLUMNS; ++j)
    	{
    	  	if (map[i][j] == '#')
    	  	{
    	  	  render_map[i][j][0] = WALL[0];
    	  	  render_map[i][j][1] = WALL[1];  
    	  	}
    	  	else if (map[i][j] == '$')
    	  	{
    	  	  render_map[i][j][0] = TROPHY[0];
    	  	  render_map[i][j][1] = TROPHY[1];  
    	  	}
    	  	else if (map[i][j] == '*')
    	  	{
    	  	  render_map[i][j][0] = GEM[0];
    	  	  render_map[i][j][1] = GEM[1];  
    	  	}
    	  	else if (map[i][j] == ' ')
    	  	{
    	  	  render_map[i][j][0] = EMPTY[0];
    	  	  render_map[i][j][1] = EMPTY[1];  
    	  	}
            else if (map[i][j] == '&')
            {
                render_map[i][j][0] = PLAYER[0];
                render_map[i][j][1] = PLAYER[1];
            }
    	  	else
    	  	{
    	  	  render_map[i][j][0] = EMPTY[0];
    	  	  render_map[i][j][1] = EMPTY[1];   
    	  	}
	
	    	  
    	}
  	}


  	clrscr(&render_map[0][0][0]);
}



void draw_player()
{
	gotoXY(40,21);
  	//changeColor(YELLOW, RED);
  	changeColor(LIGHT_BLUE, BLACK);

  	char buff = '&';
  	write(1, &buff, sizeof(buff));
}


// ------------------------
// ------------------------
// ------------------------


// void move(struct Point *from, struct Point *to, char c, int fg, int bg) 
// {
//     char buff = ' ';
//     changeColor(BLACK, BLACK);
//     gotoXY(from->x, from->y);
//     write(1, &buff, sizeof(buff));

//     changeColor(fg, bg);
//     gotoXY(to->x, to->y);
//     write(1, &c, sizeof(c));
// }


// ------------------------
// ------------------------
// ------------------------

struct Point
{
    int x, y;
};

struct Player
{
    struct Point p;
    int score;
};

struct Player player;

void game_loop()
{
    // Esto hay que encapsularlo en una funcion de init
    player.p.x = 40;
    player.p.y = 21;
    player.score = 0;

    //Time variables to track elapsed time between updates
    int current_time, elapsed_time, last_time = gettime();

    while (1)
    {
        update();
        render();
    }
}

void update()
{
    update_player();
    //update_enemies();
}

int direction = 1;
void update_player()
{
    map[player.p.y][player.p.x] = ' ';

    if (direction)
    {
        ++player.p.x;

        if (map[player.p.y][player.p.x] != ' ')
        {
            --player.p.x;
            direction = 0;
        }

    }
    else
    {
        --player.p.x;

        if (map[player.p.y][player.p.x] != ' ')
        {
            ++player.p.x;
            direction = 1;
        }
    }


    map[player.p.y][player.p.x] = '&';
}

void render()
{
    render_map();
}