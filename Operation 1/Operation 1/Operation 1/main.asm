;
; AssemblerApplication1.asm
;
; Created: 14/10/2020 10:31:32
; Author : DEE
;

//Sw1 turns on D4 and SW6 turns off D4
//--> (means will connect)
//Switches --> PORTA and LEDS --> PORTB
//DDRA can't use imideates so you need to load r16

//Initialization
init: 

	//Define PORTA as input

	ldi r16, 0			//Loads the value 0 to r16
	out DDRA, r16		//writes the DDRA with the value in r16
						//Defined all bits as input (could've just defined SW1 and SW6)

	//Define PORTC as output

	ldi r16, 0xff		//Loads the value 0b11111111 to r16 --> all pins as output 
						//or 0b00001000 or even 0x08 - only D4 as output
	out DDRC, r16		//writes the DDRC with the value in r16

	//Initialize the LEDS OFF

	out PORTC, r16		//Loads the value 1 to r16, we could also only initialize D4 as off
						//Turning the LED D4 OFF


cycle:


    SBIS PINA, 0
	ldi r16, 0b11100111

	SBIS PINA, 1
	ldi r16, 0b11000011

	SBIS PINA, 2
	ldi r16, 0b10000001

	SBIS PINA, 3
	ldi r16, 0b01111110

	SBIS PINA, 5
	ldi r16, 0xff

	out PORTC, r16

    rjmp cycle
