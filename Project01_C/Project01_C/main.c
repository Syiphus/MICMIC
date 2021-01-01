#include <avr/io.h>
#include <avr/interrupt.h>
#define F_CPU 16000000UL
#include <util/delay.h>


#define DC(valor) ((255*valor)/100)		// velocidade do motor

// criar tabela em memória de programa - const
const unsigned char display[] = {0xc0, 0xf9, 0xa4, 0xb0, 0x99, 0x92, 0x82, 0xf8, 0x80, 0x90, 0xbf, 0xff}; //0-9, "-", desligado

// variáveis globais
volatile unsigned char switch_;
volatile unsigned char flag;
volatile unsigned char screen1, screen2, screen3;
volatile unsigned char motor;


void inic(void)
{
	
	DDRD = 0b11000000;		// configura porta - switch_
	PORTD = 0b00111111;		// pinos 6,7 saídas, restantes entrada
	
	DDRC = 0b11111111;		// configura portc - displays
	PORTC = 0b11111111;		// todos os pinos saídas
	screen3 = display[11];
	screen2 = display[11];
	screen1 = display[11];
	
	DDRB = 0b11100000;    // portb motor
	PORTB = 0b00100000;
	
	//interrupção externa
	EICRA = 0b10101010;
	EIMSK = 0b00001111;
	
	OCR0 = 77;              // configura TC0 @16MHz
	TCCR0 = 0b00001111;     // 5ms, prescaler 1024, CTC
	TIMSK |= 0b00000010;    // interrupção TC0
	
	OCR2 = DC(0);
	TCCR2 = 0b01100011;		//  TC2 - prescaler 64, modo 1, phase correct
	
	sei();           //ativa flag I do SREG

}

void mudar_rot(void);
void display_(void);

// rotina interrupção TC0
ISR(TIMER0_COMP_vect)
{
	display_();
	flag = 1;
}

ISR(INT0_vect)  // switch_1
{
	switch_=1;
}

ISR(INT1_vect)  // switch_2
{
	switch_=2;
}

ISR(INT2_vect)	// switch_3
{
	switch_=3;
}

ISR(INT3_vect)	// switch_4
{
	switch_=4;
}



// rotina principal
int main(void)
{
	inic();
	
	while (1)
	{
		switch(switch_)
		{
			case 1:				// switch_1 - 25% da velocidade nominal
			screen3 = display[5];
			screen2 = display[2];
			motor = 25;
			break;
			
			case 2:				// switch_2 - 70% da velocidade nominal
			screen3 = display[0];
			screen2 = display[7];
			motor = 70;
			break;
			
			
			case 3:				// switch_3 - inverte o sentido de rotação
			if(flag==0) {break;}
			flag = 0;
			if(screen1 == display[11])
			{
				screen1 = display[10];
			}else
			{
				screen1 = display[11];
			}
			mudar_rot();
			break;

			case 4:			// switch_4 - para o motor
			screen3 = display[11];
			screen2 = display[11];
			screen1 = display[11];
			motor = 0;
			break;
		}
		switch_=0;
		OCR2 = DC(motor);
	}
}

void mudar_rot(void)
{
	if (PORTB & (1<<5))
	{
		PORTB |=(1<<5)|(1<<6);
		_delay_ms(500);
		PORTB &=~(1<<5);
	}else
	{
		PORTB |=(1<<5)|(1<<6);
		_delay_ms(500);
		PORTB &=~(1<<6);
	}
}


void display_(void)
{
	PORTD = 0b11000000;
	PORTC = screen3;
	_delay_ms(1);
	PORTD = 0b10000000;
	PORTC = screen2;
	_delay_ms(1);
	PORTD = 0b01000000;
	PORTC = screen1;
	_delay_ms(1);
}




