/*#include <avr/io.h>
#define __SFR_OFFSET    0

.Global read_analog

.extern leituraL
.extern   leituraH



read_analog: 
     PUSH r16
     PUSH r17
     PUSH r18

     SBI    ADCSRA, 6
     IN        r16,    0
     cpi    r16,    0b01000000
     brne    read_analog

     IN        r17, ADCL
     IN        r18, ADCH

     STS   r21, r17
     STS    r20, r18

     POP r16
     POP r17
     POP r18
     ret*/
