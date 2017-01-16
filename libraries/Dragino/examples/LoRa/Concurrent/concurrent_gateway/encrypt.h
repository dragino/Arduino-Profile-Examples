#ifndef encrypt_h
#define  encrypt_h


#if defined(ARDUINO) && (ARDUINO >= 100)
#include <Arduino.h>
#else
#include <WProgram.h>
#endif


class ENCRYPT
{
public:

   unsigned char key[16]={0};
   void btea(uint8_t *v, int n, uint8_t const key[16]) ;
};
#endif
