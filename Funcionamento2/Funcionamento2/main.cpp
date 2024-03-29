#include <avr/io.h>
#include <math.h>
#include <avr/interrupt.h>
#define F_CPU 16000000UL
#include <util/delay.h>

/*register*/volatile  unsigned char lerH; /*asm("r20");*/
volatile unsigned char lerL;

/* In this first line we added a "function" that will compute the motor speed when we "feed" it with the percentage */
#define DC(valor) ((255*valor)/100)

/*This table will be used to display the numbers on the screen, we added all the values 0-9 so it is easier to add more if needed
  we also added the "-" for the negative sign*/
const unsigned char display[] = {0xc0, 0xf9, 0xa4, 0xb0, 0x99, 0x92, 0x82, 0xf8, 0x80, 0x90, 0xbf, 0xff,0b01110000};

/*This is our global variables, all volatile as all of them are altered in the middle of the code*/
volatile unsigned char switch_;
//volatile unsigned char lerL,lerH;
volatile unsigned char flag,negative=0;
volatile unsigned char screen1, screen2, screen3,screen0;
volatile unsigned char motor;
volatile unsigned char analog=0,flag1;
unsigned int AD,temp;

/*This is the initialization function where all the starting commands will be performed (such as defining ports and interrupts)*/
void inic(void)
{
	/* Pin 7 and 8 will be used as outputs so we can select which display we intend to use*/
	DDRD = 0b11000000;			
	/*In this couple of lines we initialize PORTC as output and make sure that all the displays are turned off*/
	DDRC = 0b11111111;
	DDRF = 0b00000000;
	screen3 = display[11];
	screen2 = display[11];
	screen1 = display[11];
	/*PortB is the one used for the motor and is initialized in this 2 lines*/
	DDRB = 0b11100000;
	PORTB = 0b00100000;
	
	/*External interrupts are initialized here which will be used to determine which of the switches were pressed*/
	EICRA = 0b10101010;
	EICRB = 0b00001010;
	EIMSK = 0b00001111;
	/*Here we initialize the Timer/Counter 0 with a prescaler of 1024 giving us a time of 5ms*/
	OCR0 = 77;
	TCCR0 = 0b00001111;    
	TIMSK |= 0b00000010;    
	/*OCR2 is where we give the microcontroler the information of the speed we want it to operate in, we forced it to start turned off,
	this uses the Timer/Counter 2 so we also initialized it TC2 witch a prescaler of 64 mode 1 and phase correct*/
	OCR2 = DC(0);
	TCCR2 = 0b01100011;
	//AD - Only if testing C function
	ADMUX = 0b00000000; //ADC0 channel for the potenciometer
	ADCSRA = 0b10000111;
		
	
	sei();

}
/*Here we tell the program that there are functions on the bottom that will be needed*/
void mudar_rot(void);
void display_(void);
int digital(void);
int analogico(void);
int convert(unsigned int n);
extern "C" int ler_AD(void);

/*This is the interrupt functions starting with the Timer/Counter 0 and followed by the switches*/
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

int digital(void){
    while (1)
	{
		screen0= 0b10100001;
		if((PIND | 0b11011111) == 0b11011111){
		analogico();
		}
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
			}else
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
/*void ler_AD(void){
	ADCSRA = ADCSRA | 0b01000000;
	
	while((ADCSRA & (1<<ADSC)) != 0);
	
	lerL = ADCL;
	lerH = ADCH;
}*/
int analogico(void){
    while(1){
        screen0 = 0b10001000;
		if((PIND | 0b11101111) == 0b11101111){
		digital();
		}
		ler_AD();
		_delay_ms(10);
		if(negative == 1) flag1 = 1;
		AD = (lerH << 8) + lerL;
		//AD = convert(AD);
		temp = AD;
		if(AD == 0 || AD == 1023){
			screen3 = display[9];
			screen2 = display[9];
			motor=100;
		}
		else{
			if(AD<=481){
				motor = ((481-temp)*0.207);
				if(flag1 == 0)
				{
					//mudar_rot();
				}
				flag1 = 1;
			}
			else{
				if(AD<543)
				{
					motor = 0;
				}
				else{
					motor = ((temp-542)*0.185);
					if(flag1 == 1)
					{
						//mudar_rot();
					}
					flag1 = 0;
				}
			}
			screen3=display[motor%10];
			screen2=display[motor/10];
		}
		OCR2 = DC(motor);
	}
}
/*This is our main function where there is a case/switch that will operate acording to the switches that are pressed*/
int main(void)
{
	inic();
	digital();
}
/*In this function the direction of the motor will be checked and changed */
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
	if(motor!=0){
	PORTC = screen1;
	_delay_ms(1);}
	PORTD = 0b00000000;
	PORTC = screen0;
}
int convert(unsigned int n)
{
	int dec = 0, i = 0, rem;
	while (n != 0) {
		rem = n % 10;
		n /= 10;
		dec += rem * pow(2, i);
		++i;
	}
	return dec;
}