#include "../include/menu.h"
#include <stdbool.h>
#include "../include/oled.h"

void menu_move_right(void){
    if (main_menu_index < MAIN_MENU_SIZE){
        main_menu_index++;
    };
}

void menu_move_left(void){
    if (main_menu_index > 0){
        main_menu_index--;
    };
}

void menu_move_down(void){
    if (sub_menu_index < MAIN_MENU_SIZE){
        sub_menu_index++;
    };
}
void menu_move_up(void){
    if (sub_menu_index > 0){
        sub_menu_index--;
    };
}

void menu_goto_mainpage(){
    
    if (main_menu_index < MAIN_MENU_SIZE || main_menu_index >= 0){
        selected_main_menu = main_menu_index; 
    }
}
void menu_goto_subpage(){
    if (sub_menu_index < MAIN_MENU_SIZE || sub_menu_index >= 0){
        selected_main_menu = sub_menu_index; 
    }
}
void update_menu_visuals(void){
    for (uint8_t i = 0; i < MAIN_MENU_SIZE; i++){
        oled_box(i*(MAIN_MENU_WIDTH/2), i*(MAIN_MENU_HEIGHT/2), MAIN_MENU_WIDTH, MAIN_MENU_HEIGHT, 0);
        
        if (selected_main_menu == i){
            // draw allready selected box
            oled_box(i*(MAIN_MENU_WIDTH/2), i*(MAIN_MENU_HEIGHT/2), MAIN_MENU_WIDTH, MAIN_MENU_HEIGHT, 1);
        }
        if (main_menu_index == i) {
            oled_box(i*(MAIN_MENU_WIDTH/2), i*(MAIN_MENU_HEIGHT/2), MAIN_MENU_WIDTH, MAIN_MENU_HEIGHT, 2);
        }
    }
}
