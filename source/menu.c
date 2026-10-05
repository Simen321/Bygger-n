#include "../include/menu.h"
#include <stdbool.h>
#include "../include/oled.h"

typedef enum {
    MAIN_MENU,
    SUB_MENU
}MenuState;

MenuState menu_state = MAIN_MENU;
uint8_t main_menu_index = 0;
uint8_t sub_menu_index = 0;

uint8_t selected_main_menu = 0;
uint8_t selected_sub_menu = 0;
#define MAIN_MENU_SIZE 3
#define MAIN_MENU_WIDTH 40
#define MAIN_MENU_HEIGHT 20

#define SUB_MENU_SIZE 4
#define SUB_MENU_WIDTH 60
#define SUB_MENU_HEIGHT 10





void menu_move_right(void){
    if (menu_state == MAIN_MENU){
        if (main_menu_index < MAIN_MENU_SIZE -1){
        main_menu_index++;
        }    
    };
}

void menu_move_left(void){
    if (menu_state == MAIN_MENU){
            if (main_menu_index > 0){
        main_menu_index--;
        }
    };
}

void menu_move_down(void){
     if (menu_state == SUB_MENU){
         if (sub_menu_index < SUB_MENU_SIZE -1){
        sub_menu_index++;
         }
    };
}
void menu_move_up(void){
    if (sub_menu_index > 0){
        sub_menu_index--;
    } else if (sub_menu_index == 0) {
        menu_state = MAIN_MENU;
    }
}

/*
void menu_goto_mainpage(void)
{
    selected_main_menu = main_menu_index;
}

void menu_goto_subpage(void)
{
    selected_sub_menu = sub_menu_index;
}*/

void menu_click(void){
    if (menu_state == MAIN_MENU){
        selected_main_menu = main_menu_index;

        sub_menu_index = 0;
        menu_state = SUB_MENU;
    }
    else if (menu_state == SUB_MENU){
        selected_sub_menu = sub_menu_index;
    }
}

void update_menu_visuals(void){
    oled_clear();
    uint8_t offset = MAIN_MENU_WIDTH/2;
    for (uint8_t i = 0; i < MAIN_MENU_SIZE; i++){
        uint8_t j = i + 1;
        oled_box(j*(MAIN_MENU_WIDTH)-offset, (MAIN_MENU_HEIGHT/2), MAIN_MENU_WIDTH, MAIN_MENU_HEIGHT, 0);
        
        if (selected_main_menu == i){
            // draw allready selected box
            oled_box(j*(MAIN_MENU_WIDTH)-offset, (MAIN_MENU_HEIGHT/2), MAIN_MENU_WIDTH, MAIN_MENU_HEIGHT, 1);
        }
        if (main_menu_index == i) {
            oled_box(j*(MAIN_MENU_WIDTH)-offset, (MAIN_MENU_HEIGHT/2), MAIN_MENU_WIDTH, MAIN_MENU_HEIGHT, 2);
        }
    }
    update_submenu_visuals();
}
uint8_t menu_get_main_index() {
    return main_menu_index;
}


void update_submenu_visuals(void){
    uint8_t offset = SUB_MENU_HEIGHT/2;
    
    for (uint8_t i = 0; i < SUB_MENU_SIZE; i++){
        uint8_t j = i + 1;
        oled_box((SUB_MENU_WIDTH/2), (j*SUB_MENU_HEIGHT)-offset+MAIN_MENU_HEIGHT, SUB_MENU_WIDTH, SUB_MENU_HEIGHT, 0);
        
        if (selected_sub_menu == i){
            // draw allready selected box
            oled_box((SUB_MENU_WIDTH/2), j*(SUB_MENU_HEIGHT)-offset+MAIN_MENU_HEIGHT, SUB_MENU_WIDTH, SUB_MENU_HEIGHT, 1);
        }
        if (sub_menu_index == i) {
            oled_box((SUB_MENU_WIDTH/2), j*(SUB_MENU_HEIGHT)-offset+MAIN_MENU_HEIGHT, SUB_MENU_WIDTH, SUB_MENU_HEIGHT, 2);
        }
    }
}

uint8_t submenu_get_index(void)
{
    return sub_menu_index;
}
