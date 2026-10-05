#ifndef MENU_H
#define MENU_H
#include <stdint.h>





void menu_move_right(void);
void menu_move_left(void);

void menu_move_up(void);
void menu_move_down(void);

void menu_click(void);
// menu_click



void menu_goto_mainpage();
void menu_goto_subpage();

void update_menu_visuals(void);
uint8_t menu_get_main_index();

void update_submenu_visuals(void);
uint8_t submenu_get_main_index();
#endif