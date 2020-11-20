;********************OPERATION 3***********************************

.include <m128def.inc>


.def temp_int = r18
.def temp = r16				;Temporary Variable R16
.def luz = r19				;Variable that will determinate how many cars are in the lot
.cseg
							
.org 0						
	jmp main				;Forces the program to start on main

.org 0x1E
	jmp int_tc0				;Counter Funtion

.org 0x46

table:
    .db 0x90, 0x80, 0xF8, 0x82, 0x92, 0x99, 0xB0, 0xA4, 0xF9, 0xC0 ;Table with all the values for the 7 segment display	

;********************INITIALIZATION***********************************											  
init:

	ser temp
	out DDRA, temp				 ;Sets DDRA as output
	out DDRC, temp				 ;Sets DDRC as output
	ldi temp, 0xBE				 ;Sets D1 and D7 ON
	out portA, temp

	ldi temp, 0xFF
	out DDRD, temp 				 ;Sets DDRD as input (PD0 - PD5) and as output (PD6 - PD7)
	out portD, temp				 ;Selects the display placed on the right side

	;Timer
	ldi temp, 124				 ;Temporização de 1ms 128/(16*10^6)*(124+1)
	out OCR0, temp
	ldi temp, 0b00001101		 ;TC0 em modo CTC, prescaler 128, OC0 off
	out TCCR0, temp
	in temp, TIMSK				 ;Enable da interrupção do TC0 (CTC)
	ori temp, 0b00000010
	out TIMSK, temp

	sei							 ;Enable global das interrupções

	ldi ZL, low(table*2)		 ;Stores the low value of the table in Z
	ldi ZH, high(table*2)		 ;Stores the high value of the table in Z
	lpm r12, Z					 ; > R12 will be used to display the number of free slots in the lot
	lpm r11,Z					 ; > R11 will be used to set the "empty" limit 
	ldi r17, 9					 ;R17 will be used to determinate if the park if full
	add ZL,r17					 ;Increases the ZL value to reach the 0
	lpm r13,Z					 ; > R13 will be used to set the "full" limit
	sub ZL,r17					 ;Returns ZL to its original state

	out portC, r12				 ;Displays the value 9
	ret

;********************MAIN***********************************
main:							 ;Initialization of the stack pointer in the last position of the data memory
							 
	ldi temp, low(RAMEND)		 ;temp is asigned w/ the low value of RAMEND
	out spl, temp				 
	ldi temp, high(RAMEND)		 ;temp is asigned w/ the high value of RAMEND
	out sph, temp				 
	call init				     ;Calls the initialization function
ciclo:		
	cp r11, r12			         ;Checks if the park if empty
	breq teste_entrada			 ;If the park is empty pressing sw2 won't do anything
	cp r12,r13					 ;Checks if the park is full
	breq teste_saida			 ;If the parks if sull pressing sw1 won't do anything
	sbis portd,0				 ;Checks if sw1 (+1 car) is pressed
	rjmp entrada				 
	sbis portd,1				 ;Checks if sw2 (-1 car) is pressed
	rjmp saida
	rjmp ciclo					 ;Repeats the cicle
teste_entrada:					 ;This function will be used if the park if empty
	sbis portd,0				
	rjmp entrada
	rjmp ciclo
teste_saida:					 ;This function will be used if the park if full
	sbis portd,1
	rjmp saida
	rjmp ciclo
delay:							 ;This function prevents the entry of more than 1 car in 1 button press
	sbic portd,0
	ret
	rjmp delay
delay1:							 ;Same as the delay function
	sbic portd,1	
	ret
	rjmp delay1

entrada:
	brtc entrada			 ;Timer
	clt						 ;Clears the variable T
	sbic portD, 0			 ;Checks if sw1 is still pressed after 1ms
	rjmp ciclo			     ;If it was pressed less than 1 ms it will go back to the cycle 	
	inc ZL					 ;Increases the value of ZL
	inc luz				     ;Increases the value of luz
	lpm r12, Z				 ;Loads the value of Z in r12
	out portC, r12			 
	call LEDs				 ;Leds will determinate wich leds are activated
	sbis portd,0		     ;Checks if sw1 is still being activated
	call delay				 ;if it is it will stay in the delay loop till it stops being pressed
	rjmp ciclo

saida:
    brtc  saida				 ;Timer
	clt						 ;Clears the variable T
	sbic portD, 1			 ;Checks if sw2 is still pressed after 1ms
	rjmp ciclo				 ;If it was pressed less than 1 ms it will go back to the cycle 
	dec ZL					 ;Decreases the value of ZL
	dec luz					 ;Decreases the value of luz
	lpm r12, Z				 ;Loads the value of Z in r12
	out portC, r12
	call LEDs				 ;Leds will determinate wich leds are activated
	sbis portD,1			 ;Checks if sw2 is still being activated
	call delay1				 ;if it is it will stay in the delay loop till it stops being pressed
	rjmp ciclo

;********************LEDS***********************************

LEDs:						 ;This function will check wich leds need to be ativated
cpi luz,5					 ;Compares LUZ with the value 5
brlo green					 ;If it is lower than 5 (0<cars<5) it will turn on d1

cpi luz,8					 ;Compares LUZ with the value 9
brlo yellow					 ;If it is lower than 8 (5<=cars<9) it will turn on d2


rjmp red				     ;If there are 9 cars in the lot it will turn on d3

Green:
	ldi r25, $BE
	out portA, r25
	rjmp leave
Yellow:
	ldi r25, $BD
	out portA, r25
	rjmp leave
Red:
	ldi r25, $7B
	out portA, r25
leave:
	ret
;********************TIMER***********************************

int_tc0:
	IN temp_int, sreg			 ;Guarda o valor SREG
	out SREG, temp_int			 ;Report o valor SREG
	set							 ;Ativa a flag t do sreg
	reti
