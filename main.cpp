#include <avr/io.h>
#include <avr/interrupt.h>
#define F_CPU 16000000UL
#include <util/delay.h>
#include <stdio.h>

#define DC(valor) ((255*valor)/100)		// velocidade do motor

// criar tabela em mem?ria de programa - const
const unsigned char display[] = {0xc0, 0xf9, 0xa4, 0xb0, 0x99, 0x92, 0x82, 0xf8, 0x80, 0x90, 0b10111111, 0xFF, 0b00010001, 0b00000011, 0b11000110}; //0-9, "-", desligado, A, D, C

// vari?veis globais
volatile unsigned char sw,pc=0;
volatile unsigned char flagtimer;
volatile unsigned char display_dir, display_esq, display_mid, display_aleixo;
volatile unsigned char motor, negativo;
volatile unsigned char val[4] = {11, 11, 11, 11};



void config(void);
void send_message(char *buffer);
void mudar_rot(void);
void disupdate(void);

void inic(void)
{
	
	DDRD = 0b11000000;		// configura porta - sw
	DDRC = 0b11111111;		// configura portc - displays
	
	
	
	
	DDRB = 0b11100000;    // port motor
	PORTB = 0b00100000;
	
	//interrup??o externa
	EICRA = 0b10101010;
	EIMSK = 0b00001111;
	
	OCR0 = 77;              // configura TC0 @16MHz
	TCCR0 = 0b00001111;     // 5ms, prescaler 1024, CTC
	TIMSK |= 0b00000010;    // interrup??o TC0
	
	OCR2 = DC(0);
	TCCR2 = 0b01100011;		//  TC2 - prescaler 64, modo 1, phase correct
	
	sei();           //ativa flag I do SREG

	negativo = 1;
}



// rotina interrup??o TC0
ISR(TIMER0_COMP_vect)
{
	disupdate();
	flagtimer = 1;
}

volatile char rx_usart;
volatile char dado_rx;
char tx_buffer[50];

ISR(USART0_RX_vect)
{
	dado_rx = UDR0;
	rx_usart = 1;
}


int main()
{
	inic();
	config();

	while (1)
	{
		if(rx_usart == 1)
		{
			rx_usart = 0;
			sprintf(tx_buffer, "tecla:%c\n", dado_rx);
			send_message(tx_buffer);
		}
		while(pc == 0)
		{
			switch (dado_rx)
			{
				case 'c':
				val[3] = 14;
				break;
				
				case 'b':
				val[1] = 2;
				val[2] = 5;
				val[3] = 14;
				motor = 25;
				OCR2 = DC(motor);
				break;
				
				case 'm':
				val[1] = 7;
				val[2] = 0;
				val[3] = 14;
				motor = 70;
				OCR2 = DC(motor);
				break;
				
				case 'i':
				val[3] = 14;
				if(val[0] == 11)
				{
					val[0] = 10;
					negativo = 0;
				}
				else
				{
					val[0] = 11;
					negativo =1;
				}
				dado_rx = 0;
				
				mudar_rot();
				break;
				
				case 'l':
				if(negativo==0)
				{
					sprintf(tx_buffer, "Sentido: negativo\r\n");
					send_message(tx_buffer);
					sprintf(tx_buffer, "Velocidade do motor: %d\r\n", motor);
					send_message(tx_buffer);
				}
				else
				{
					sprintf(tx_buffer, "Sentido: positivo\r\n");
					send_message(tx_buffer);
					sprintf(tx_buffer, "Velocidade do motor: %d\r\n", motor);
					send_message(tx_buffer);
				}
				dado_rx = 0;
				break;
				case 'p':
				val[3] = 14;
				val[1] = 11;
				val[2] = 11;
				val[0] = 11;
				motor = 0;
				OCR2 = DC(motor);
				if(negativo == 0)
				{
					mudar_rot();
				}
				break;
				if((PIND | 0b11101111) == 0b11101111)
				{
					val[0] = 0;
					pc = 1;
				}
				if((PIND | 0b11011111) == 0b11011111)
				{
					pc = 2;
				}
				
			}
		}
		while(pc == 1)
		{
			switch(sw)
			{
				case 1:				// sw1 - 25% da velocidade nominal
				sw=0;
				val[2] = 2;
				val[3] = 5;
				motor = 25;
				OCR2 = DC(motor);
				break;
				
				case 2:			// sw2 - 70% da velocidade nominal
				sw=0;
				val[2] = 7;
				val[3] = 0;
				motor = 70;
				OCR2 = DC(motor);
				break;
				
				
				case 3:			// sw3 - inverte o sentido de rota??o
				sw=0;
				if(flagtimer == 1)
				{
					
					if(val[1] == 11)
					{
						val[1] = 10;
						negativo = 0;
					}
					else
					{
						val[1] = 11;
						negativo =1;
					}
					
					mudar_rot();
					flagtimer = 0;
				}
				
				
				break;
				
				case 4:			// sw4 - para o motor
				sw=0;
				val[2] = 11;
				val[3] = 11;
				val[1] = 11;
				motor = 0;
				OCR2 = DC(motor);
				if(negativo == 0)
				{
					mudar_rot();
				}
				break;
			}
		}
		
	}
}







// inverte sentido da rota??o
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
		PORTB &= ~(1<<6);
	}
}


void disupdate(void)
{
	PORTD += 0x40;
	PORTC = display[val[PORTD>>6]];
}

//configuration

void config(void)
{
	UBRR0H = 0;
	UBRR0L = 207;
	UCSR0A = 0;
	UCSR0B = (1<<RXCIE0) | (1<<RXEN0) | (1<< TXEN0);
	UCSR0C = (1<<UCSZ01) | (1<<UCSZ00);
	
	sei(); //habilita interrupçoes
}

void send_message(char *buffer)
{
	unsigned char i=0;
	while(buffer[i]!='\0'){
		while((UCSR0A & 1<<UDRE0) == 0);
		UDR0 = buffer[i];
		i++;
	}
	return;
}