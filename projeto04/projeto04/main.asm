;*************************************************************
;--------------------Projeto04--------------------------------
;************************************************************
.include <m128def.inc>
.def temp = r16 
.def cnt_int = r17
.def temp_int = r18
.def freq = r24
.def save = r20
.cseg			;Inicio do segmento de codigo
.org 0			;Vetor de Reset
	jmp main 
.org 0x1E
	jmp int_tc0
.org 0x46

table:
	.db 0xF9, 0xA4, 0xB0, 0x99, 0x92, 0x82, 0xF8, 0x80, 0x90,0

;***********************Inicialização**************************
init: 
	ser temp				
	out ddrc,temp				 ;Sets DDRD as output
	ldi temp, 0b11000000
	out DDRD, temp 				 ;Sets DDRD as input (PD0 - PD5) and as output (PD6 - PD7)
	ldi temp, 0b11111111
	out portD, temp				 ;Selects the display placed on the right side and sets the value 1 in all switches

	;timer
	ldi temp,155				 ;temporização de 10ms 1024/(16*10^6)*(155)
	out OCR0,temp

	ldi cnt_int ,50				 ;2HZ 
	ldi freq,50					 ; Variable that will serve as a cnt_int backup
	ldi temp, 0b00001111		 ;TC0 em modo CTC, prescaler 1024, OC0 off
	out TCCR0,temp
	in r16,TIMSK				 ;enable da interrupção do TC0 (CTC)
	ori r16,0b00000010
	out TIMSK,r16

	sei							 ; enable global das interrupções
	ldi ZL,low(table*2) 
	ldi ZH,high(table*2)
	mov save,ZL
	lpm r12,z
	out portc,r12
	ret
;*********************Programa_Principal**************************************
main:
	ldi temp,low(RAMEND)		 ;Inicialização da stack
	out spl,temp
	ldi temp,high(RAMEND)
	out sph,temp

	call init
loop: 
	sbis pind,2					 ;Checks if SW3 is pressed
	call cinquenta				 ;if it is the frequency will change to 50Hz
	sbis pind,3					 ;Checks if SW4 is pressed
	call dois					 ;if it is the frequency will change to 2Hz
	sbis pind,4					 ;Checks if SW4 is being pressed
	rjmp count					 ;if it is it will start the  "game"
	rjmp loop					 ;repeats the loop
count:
	brtc count					 ;Counter
    clt						     ;Clears T
	inc ZL					     ;Increments the value of ZL
	lpm r12,Z					 ;Loads the value of ZL into Z
	SBRS r12,7					 ;Checks if most significant bit of r12 is set 
	call reset					 ;If it is reset will be called
	out portc,r12			     ;Outputs the value of r12 into the display
//	sbis pind,5				     --Parte comentada por duvida se frequencia poderia
//	rjmp loop					 --ser alterada a meio do codigo ou não
//	sbis pind,2
//	call cinquenta
//	sbis pind,3
//	call dois
	sbis pind,5					 ;Checks is SW6 is being pressed
	rjmp loop					 ;If it is the "game" will stop
	rjmp count					 ;If it is not the "game" will continua
cinquenta:						 ;Changes the value of cnt_int to change the frequency of the program
	ldi cnt_int ,2 ; 50HZ		 ;making it count more/less times 
	ldi freq,2
	ret
dois:							 ;Same as the segment "cinquenta"
	ldi cnt_int ,50 ; 2HZ
	ldi freq, 50
	ret
reset:						     ;Once the table has reached its end 
	mov ZL,save					 ;ZL will begin from the start of the table again
	lpm r12,Z					 ;we do this by copying the value stored on "save"
	ret							 ;returns 

;*************************Timer***********************************
int_tc0:
	IN TEMP_INT,sreg			 ;guarda o valor SREG
	DEC cnt_int				     ;verifica se passou 500ms/20ms
	BRNE f_int
	mov cnt_int,freq
	out SREG,temp_int			 ;report o valor SREG
	set							 ;ativa a flag t do sreg
	reti
f_int:
	out SREG,temp_int
	reti
