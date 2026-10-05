#ifndef MENU_H
#define MENU_H
#include <stdint.h>

uint8_t main_menu_index = 0;
uint8_t sub_menu_index = 0;

uint8_t selected_main_menu = 0;
uint8_t selected_sub_menu = 0;
#define MAIN_MENU_SIZE 3
#define MAIN_MENU_WIDTH 50
#define MAIN_MENU_HEIGHT 20


void menu_move_right(void);
void menu_move_left(void);

void menu_move_down(void);
void menu_move_down(void);

void menu_click(void);
// menu_click


void menu_goto_mainpage();
void menu_goto_subpage();

void update_menu_visuals(void);

#endif