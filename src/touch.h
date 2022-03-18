//
// Created by jqt3o on 3/17/2022.
//

#ifndef BLINDS_CONTROLLER_TOUCH_H
#define BLINDS_CONTROLLER_TOUCH_H

//initializes the touch sub system
void touch_init(int [] touch_pins);

//each loop (Should be called often) will update the internal filtering
void touch_loop();

//Get the currently determined status;
uint16_t touch_status();

//void touch_register(void * callback);
//void touch_unregister(void * callback);

#endif //BLINDS_CONTROLLER_TOUCH_H
