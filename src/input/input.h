#ifndef INPUT_H
#define INPUT_H

#include "vitajme.h"
#include <psp2/ctrl.h>
#include <psp2/touch.h>

/**
 * Initialize input subsystems (SceCtrl, SceTouch).
 */
void input_init(void);

/**
 * Called once per frame to read hardware inputs and translate them 
 * to Java ME key events. It fires callbacks into the javacall layer.
 */
void input_poll(void);

#endif // INPUT_H
