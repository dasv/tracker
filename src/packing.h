#ifndef _PACKING_H_
#define _PACKING_H_

#include <types.h>

extern const size_t package_size;
extern uint8_t packed[];  // without  closing character
extern uint8_t *p;

int is_little_endian();
void pack(void *x, size_t size, uint8_t *p, int le);
int16_t to_short(float x);
void pack_float(float x, uint8_t *p, int le);
void pack_short(float x, uint8_t* p, int le);

void  packer(marineData Data);

#endif _PACKING_H_
