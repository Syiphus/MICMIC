#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdio.h>
#include <ctype.h>
#define F_CPU 16000000UL
#include <util/delay.h>

/* In this first line we added a "function" that will compute the motor speed when we "feed" it with the percentage */
#define DC(valor) ((255*valor)/100)

/*This table will be used to display the numbers on the screen, we added all the values 0-9 so it is easier to add more if needed
  we also added the "-" for the negative sign*/
const unsigned char display[] = {0xc0, 0xf9, 0xa4, 0xb0, 0x99, 0x92, 0x82, 0xf8, 0x80, 0x90, 0xbf, 0xff};

/*This is our global variables, all volatile as all of them are altered in the middle of the code*/
volatile unsigned char switch_;
volatile unsigned char flag,negative=0;
volatile unsigned char screen1, screen2, screen3,screen0;
volatile unsigned char motor;
volatile unsigned char failsafe=0;
volatile unsigned char sentido;

/*this are the new variables that will be used with USART*/
typedef struct USARTRX
{
	char receiver_buffer;
	unsigned char status;
	unsigned char receive: 1;
	unsigned char error: 1;

} 	USARTRX_st;

volatile USARTRX_st rxUSART = {0,0,0,0};
char transmit_buffer[30];


/*This is the initialization function where all the starting commands will be performed (such as defining ports and interrupts)*/
void inic(void)
{
	/* Pin 7 and 8 will be used as outputs so we can select which display we intend to use*/
	DDRD = 0b11000000;			
	/*In this couple of lines we initialize PORTC as output and make sure that all the displays are turned off*/
	DDRC = 0b11111111;
	screen3 = display[11];
	screen2 = display[11];
	screen1 = display[11];

	/*PortB is the one used for the motor and is initialized in this 2 lines*/
	DDRB = 0b11100000;
	PORTB = 0b00100000;
	DDRE =  0b11000000;	
	
	/*External interrupts are initialized here which will be used to determine which of the switches were pressed*/
	EICRA = 0b10101010;
	EIMSK = 0b00001111;

	/*Here we initialize the Timer/Counter 0 with a prescaler of 1024 giving us a time of 5ms*/
	OCR0 = 77;
	TCCR0 = 0b00001111;    
	TIMSK |= 0b00000010;

	/*OCR2 is where we give the microcontroler the information of the speed we want it to operate in, we forced it to start turned off,
	this uses the Timer/Counter 2 so we also initialized it TC2 witch a prescaler of 64 mode 1 and phase correct*/
	OCR2 = DC(0);
	TCCR2 = 0b01100011;

	/*Here we will initialize and configure the USART*/
	UBRR0H = 0;
	UBRR0L = 207;
	UCSR0A = 0;
	UCSR0B = (1<<RXCIE0) | (1<<RXEN0) | (1<< TXEN0);
	UCSR0C = (1<<UCSZ01) | (1<<UCSZ00);

	sei();

}

/*Here we tell the program that there are functions on the bottom that will be needed*/
void mudar_rot(void);
void display_(void);
void send_message(char *buffer);
void lercons(void);
void computer(void);

/*This is the interrupt functions starting with the Timer/Counter 0 and followed by the switches*/
ISR(USART0_RX_vect)
{
	rxUSART.status = UCSR0A; //error flag
	
	if(rxUSART.status & ((1<<FE0) | (1<<DOR0)| (1>>UPE0)))
	{
		rxUSART.error = 1;
	}
	
	rxUSART.receiver_buffer = UDR0;
	rxUSART.receive = 1;
}

ISR(TIMER0_COMP_vect)
{
	display_();
	flag = 1;
}

ISR(INT0_vect) 
{
	switch_=1;
}

ISR(INT1_vect) 
{
	switch_=2;
}

ISR(INT2_vect)
{
	switch_=3;
}

ISR(INT3_vect)
{
	switch_=4;
}

void lercons(void)
{
	if(rxUSART.error == 1)
	{
		sprintf(transmit_buffer,"\r\nErro, tente de novo\r\n");
		rxUSART.error = 0;
	}

	while(rxUSART.receive!= 0)
	{
		sprintf(transmit_buffer,"\r\nFoi premida a tecla:\t %c\r\n",rxUSART.receiver_buffer);
		send_message(transmit_buffer);
		rxUSART.receive = 0;	
	}

	if(toupper(rxUSART.receiver_buffer) == 'C')
	{
		computer();
	}

	if(toupper(rxUSART.receiver_buffer) == 'L')
	{
		if(negative == 1)	sentido = '-';
		else sentido = '+';
		sprintf(transmit_buffer,"\r\nSentido: %c  Velocidade: %d\r\n",sentido,motor);
		send_message(transmit_buffer);
	}						
}

void computer(void)
{
	while((PIND | 0b11101111) != 0b11101111)
	{
		screen0= 0b11000110;
		_delay_ms(30);

		if(rxUSART.receive == 1)
		{
			sprintf(transmit_buffer,"\r\nFoi premida a tecla:\t %c\r\n",rxUSART.receiver_buffer);
			send_message(transmit_buffer);
			rxUSART.receive = 0;
		}

		switch(toupper(rxUSART.receiver_buffer))
		{
			case 'B':	
				screen3 = display[5];
				screen2 = display[2];
				if(negative == 1)
				{
					screen1 = display[10];
				}
				motor = 25;
				break;
			
			/*Switch 2 will make the motor rotate at 25% of its nominal speed, and save the value "25" in variables
			 so it is displayed in the 7 segment display*/
			case 'M':	
				screen3 = display[0];
				screen2 = display[7];
				if(negative == 1)
				{
					screen1 = display[10];
				}
				motor = 70;
				break;
			
			/*Switch 3 will check what value is being currently displayed ( "-" or " " ) and call the function that will make the motor
			switch the direction of its rotation*/
			case 'I':	
				if(flag==0) break;
				flag = 0;
				if(screen1 == display[11] && negative == 0)
				{
					screen1 = display[10];
					negative = 1;
				}
				else
				{
					screen1 = display[11];
					negative = 0;
				}
				mudar_rot();
				rxUSART.receiver_buffer = 0;
				break;

			/*Switch 4 will make the motor stop and turn of the display*/
			case 'P':	
				screen3 = display[11];
				screen2 = display[11];
				screen1 = display[11];
				motor = 0;
				break;

			case 'L':
				if(negative == 1)	sentido = '-';
				else sentido = '+';
				sprintf(transmit_buffer,"\r\nSentido: %c  Velocidade: %d\r\n",sentido,motor);
				send_message(transmit_buffer);
				rxUSART.receiver_buffer = 0;
		}
		OCR2 = DC(motor);
		rxUSART.receive = 0;
	}
}

/*This is our main function where there is a case/switch that will operate acording to the switches that are pressed*/
int main(void)
{
	inic();
	
	while (1)
	{
		_delay_ms(30);
		if(rxUSART.receive == 1)
		{	
			lercons();
		}
		screen0= 0b10100001;

		switch(switch_)
		{
			/*Switch 1 will make the motor rotate at 25% of its nominal speed, and save the value "25" in variables
			 so it is displayed in the 7 segment display*/
			case 1:	
				screen3 = display[5];
				screen2 = display[2];
				if(negative == 1)
				{
					screen1 = display[10];
				}
				motor = 25;
				break;
			
			/*Switch 2 will make the motor rotate at 25% of its nominal speed, and save the value "25" in variables
			 so it is displayed in the 7 segment display*/
			case 2:	
				screen3 = display[0];
				screen2 = display[7];
				if(negative == 1)
				{
					screen1 = display[10];
				}
				motor = 70;
				break;
			
			/*Switch 3 will check what value is being currently displayed ( "-" or " " ) and call the function that will make the motor
			switch the direction of its rotation*/
			case 3:	
				if(flag==0) break;
				flag = 0;
				if(screen1 == display[11] && negative == 0)
				{
					screen1 = display[10];
					negative = 1;
				}
				else
				{
					screen1 = display[11];
					negative = 0;
				}
				mudar_rot();
				break;

			/*Switch 4 will make the motor stop and turn of the display*/
			case 4:	

				screen3 = display[11];
				screen2 = display[11];
				screen1 = display[11];
				motor = 0;
				break;
		}

		/*Resets the value of the switch*/
		switch_=0;
		/*Changes the motor speed*/
		OCR2 = DC(motor);
	}
}

/*In this function the direction of the motor will be checked and changed */
void mudar_rot(void)
{
	if (PORTB & (1<<5))
	{
		PORTB |=(1<<5)|(1<<6);
		_delay_ms(500);
		PORTB &=~(1<<5);
	}
	else
	{
		PORTB |=(1<<5)|(1<<6);
		_delay_ms(500);
		PORTB &=~(1<<6);
	}
}

/*In this function we update the display with the values of the variables given from the above functions*/
void display_(void)
{
	PORTD = 0b11000000;
	PORTC = screen3;
	_delay_ms(1);

	PORTD = 0b10000000;
	PORTC = screen2;
	_delay_ms(1);

	PORTD = 0b01000000;
	/*This "if" condition is used to make sure the "-" sign wont appear when the motor is off*/
	if(motor!=0)
	{
		PORTC = screen1;
		_delay_ms(1);
	}

	PORTD = 0b00000000;
	PORTC = screen0;
}

void send_message(char *buffer)
{
	unsigned char i=0;
	while(buffer[i]!='\0')
	{
		while((UCSR0A & 1<<UDRE0) == 0);
		UDR0 = buffer[i];
		i++;
	}
	return;
}

