#include <avr/io.h>
#define __SFR_OFFSET    0

.Global ler_AD

.extern lerL
.extern    lerH



ler_AD: 
	  ldi r16, hi8(RAMEND)
out SPH, r16
ldi r16, lo8(RAMEND)
out SPL, r16
     PUSH r16
     PUSH r17
     PUSH r18

     SBI    ADCSRA, 6
     IN        r16,    0
     cpi    r16,    0b01000000
     brne    ler_AD

     IN        r17, ADCL
     IN        r18, ADCH

     STS    lerL, r17
     STS    lerH, r18

     POP r16
     POP r17
     POP r18
     ret