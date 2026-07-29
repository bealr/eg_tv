#ifndef IR_H
#define IR_H

#include <pthread.h>

struct ir_t {
    int new_key;
    char key;
};

void *read_ir_th(void *param);
struct ir_t *read_ir_init();
char ir_get_key(struct ir_t *ir);


#endif