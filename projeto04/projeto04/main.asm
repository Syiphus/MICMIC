;
; projeto04.asm
.include <m128def.inc>
.def temp = r16
 .def cnt_int = r17
 .def temp_int = r18
 .def limite = r19
 .def freq = r24
 ldi limite, 0x90
 .def save = r20
.cseg			;Inicio do segmento de codigo
.org 0			;Vetor de Reset
	jmp main 
.org 0x1E
	jmp int_tc0
.org 0x46

table:
	.db 0xF9, 0xA4, 0xB0, 0x99, 0x92, 0x82, 0xF8, 0x80, 0x90,0

;Inicialização
init: 
	ser temp
	out ddra, temp // coloca 1 em todos os bits de r16
	out porta,temp //configura os pinos da porta A como saídas
	out ddrc,temp // Colocar todos os pinos do PortC como saída para o display de 7 segmentos
	out portc,temp //leds do display apagados
	ldi temp, 0b11000000
	out DDRD, temp 				 ;Sets DDRD as input (PD0 - PD5) and as output (PD6 - PD7)
	ldi temp, 0b11111111
	out portD, temp				 ;Selects the display placed on the right side

	;timer
	ldi temp,155 ;temporização de 10ms 1024/(16*10^6)*(155)
	out OCR0,temp

	ldi cnt_int ,50 ; 2HZ 
	ldi freq,50
	ldi temp, 0b00001111 ;TC0 em modo CTC, prescaler 1024, OC0 off
	out TCCR0,temp
	in r16,TIMSK ;enable da interrupção do TC0 (CTC)
	ori r16,0b00000010
	out TIMSK,r16

	sei; enable global das interrupções
	ldi ZL,low(table*2) 
	ldi ZH,high(table*2)
	mov save,ZL
	lpm r12,z
	out portc,r12
	ret
;programa principal
main:
	ldi temp,low(RAMEND)
	out spl,temp
	ldi temp,high(RAMEND)
	out sph,temp

	call init
loop: 
	sbis pind,2
	call cinquenta
	sbis pind,3
	call dois
	sbis pind,4
	rjmp count
	rjmp loop
count:
	brtc count
    clt
	inc ZL
	lpm r12,Z
	SBRS r12,7
	call reset
	out portc,r12
	sbis pind,5
	rjmp loop
	sbis pind,2
	call cinquenta
	sbis pind,3
	call dois
	sbis pind,5
	rjmp loop
	rjmp count
cinquenta:
	ldi cnt_int ,2 ; 50HZ 
	ldi freq,2
	ret
dois: 
	ldi cnt_int ,50 ; 2HZ
	ldi freq, 50
	ret
reset:
	mov ZL,save
	lpm r12,Z
	ret
	
;TIMER
int_tc0:
	IN TEMP_INT,sreg ;guarda o valor SREG
	DEC cnt_int ;verufica se passou 500ms/20ms
	BRNE f_int
	mov cnt_int,freq
	out SREG,temp_int ;report o valor SREG
	set ;ativa a flag t do sreg
	reti
f_int:
	out SREG,temp_int
	reti
