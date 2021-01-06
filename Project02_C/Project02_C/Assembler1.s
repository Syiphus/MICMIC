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
 
in r16,ADCL
in r17,ADCH

ret
