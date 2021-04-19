#ifndef _DEMO_H
#define _DEMO_H

#define DEMO_TITLE                      ("Term Project")
#define DEMO_WIDTH                      (320)
#define DEMO_HEIGHT                     (240)
#define DEMO_SCALE                      (2)
#define DEMO_INPUT_UP                   (0)
#define DEMO_INPUT_DOWN                 (1)
#define DEMO_INPUT_LEFT                 (2)
#define DEMO_INPUT_RIGHT                (3)

void demo_init(void);
void demo_free(void);
void demo_tick(float delta_time, int* keys);
void demo_draw(void);

#endif