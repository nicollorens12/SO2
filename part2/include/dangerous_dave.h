/*
 * dangerous_dave.h - Joc dangerous_dave versio ZeOS
 */

#ifndef __DANGEROUS_DAVE_H__
#define __DANGEROUS_DAVE_H__

void game_loop();

void update();
void update_player();
void render();
void render_map();

// -----------------

void render_map();

void draw_player();

#endif  /* __DANGEROUS_DAVE_H__ */