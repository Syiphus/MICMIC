;.include <m128def.inc>
#define __SFR_OFFSET 0
#include<avr/interrupt.h>

.global read_analog
.byte	4		; allocate 4 bytes
.extern leituraL
.extern leituraH

read_analog:
sbis ADCSRA,ADIF
rjmp read_analog
 
in r11,ADCL
in r12,ADCH
mov leituraL,r11 
mov leituraH,r12

ret
