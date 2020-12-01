;************Initialization************

init: 
	;Define portA as input
	clr r16				;Loads the value 0 to r16 --> all pins as input 
	out DDRA, r16		;writes the DDRA with the value in r16
	
	;Define PORTC as output
	ser r16				;Loads the value 1 to r16 --> all pins as output 
	out DDRC, r16		;writes the DDRC with the value in r16

	;Initialize the LEDS OFF
	out portC, r16		;Loads the value 1 to r16 -> Turning all LED's off
						
	;STACK POINTER
    ldi r16, 0xff       ;Start stack address at 0x10FF
    out spl, r16	    ;16 bits length divided into 2 registers 8 bits length
    ldi r16, 0x10
    out sph, r16 	    ;SPH (stack high) e SPL (stack low)


	ldi r22, 0
	ldi r23, 0
	ldi r29, 0b11111111

;************LOOP************

loop:
    SBIS pinA, 0	  ;Checks if SW1 is pressed
	rjmp SW1		  ;if SW1 is pressed it will jump to sw1
    rjmp loop	   	  ;Repeats the loop

SW1:
	SBRS r29, 0
	call delay
	cpi r23, 1		  ;Verifies if sw6 was pressed during the delay
	breq SW6_2		  ;If sw6 was pressed during delay it will invert the cycle
	ldi r16, 0xFE	  ;loads the value to turn D1 on r16
	out portC, r16	  ;Turns LED1 on

SW1_1:
	call delay		  ;Calls the delay function
	ldi r29, 0

SW1_2:
	clr r22			  ;clears the value on the registers
	clr r23
	sec
	rol r16			  ;Logic shift left
	out portC, r16	  ;Outputs in portC the value of r16
	sbic portC, 7	  ;Checks if led 8 is on
	jmp SW1_1		  ;If led 8 isn't on it will jump to sw1_1

	ldi r22, 1		  ;Loads the value 1 on r22 so that in the delay sw6 will be tested
    rjmp SW1		  ;restarts the cycle

SW6: 
	call delay
	cpi r23, 2        ;Verifies if sw1 was pressed during the delay
	breq sw1_2		  ;If sw6 was pressed during delay it will invert the cycle
	ldi r16, 0x7F	  ;loads the value to turn D8 on r16
	out portC, r16	  ;Turns LED8 on

SW6_1:
	call delay		  ;Calls the delay function

sw6_2:
	clr r22			  ;clears the value on the registers
	clr r23
	sec
	ror r16			  ;Logic shift right
	out portC, r16	  ;Outputs in portC the value of r16
	sbic portC, 0	  ;Checks if led 1 is on
	rjmp SW6_1		  ;If led 1 isn't on it will jump to sw6_1

	ldi r22, 2		  ;Loads the value 2 on r22 so that in the delay sw6 will be tested
    rjmp SW6		  ;restarts the cycle

;************DELAY************

delay:
	push r18
	push r19
	push r20
	ldi	r20, 82
cycle0:
	ldi	r19, 255
cycle1:
	ldi r18, 254

cycle2:
	dec r18
	brne cycle2		
	cpi r22, 1		;Tests if D8 is ON
	breq testSW6	;If it is on it will jump to testSW6
	cpi r22, 2		;Tests if D1 is ON
	breq testSW1	;If it is on it will jump to testSW1
cycle3:
	dec r19
	brne cycle1

	dec r20
	brne cycle0

	pop r20
	pop r19
	pop r18

	ret

;************TESTS************
testSW1:
	sbis pinA, 0	;Tests if sw1 is being pressed
	ldi r23, 2		;If it is being pressed it will store 2 in r23
	rjmp cycle3		;Returns to the delay function

testSW6:
	sbis pinA, 5    ;Tests if sw5 is being pressed
	ldi r23, 1	    ;If it is being pressed it will store 1 in r23
	rjmp cycle3		;Returns to the delay function