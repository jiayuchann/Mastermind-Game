#include <msp430.h>
#include <stdlib.h>
#include <string.h>
#include <libemb/serial/serial.h>
#include <libemb/conio/conio.h>

#include "dtc.h"
#include "input.h"

//
// REQUIRED WIRING:
 /**************************************
 * P1.0 <- SEGF      -    SEGD -> P2.6 *
 * P1.1    UART RXD  -    SEGE -> P2.7 *
 * P1.2    UART TXD  -                 *
 * P1.3    Button    -                 *
 * P1.4 <- Trimpot   -    DIG1 -> P1.7 *
 * P1.5 <- SEGG      -    DIG4 -> P1.6 *
 * -.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.- *
 * P2.0 <- SEGA      -    BLUE    P2.5 *
 * P2.1    RED       -    SEGC -> P2.4 *
 * P2.2 <- SEGB      -   GREEN    P2.3 *
 **************************************/

unsigned int pot_value;
char answer_array[5] = {0};	//the code
char dialIn_array[4] = {0};	//player's guess

//bit patterns for displaying digits
char number_patterns1[] = {BIT5, BIT5 | BIT0, BIT0, BIT0, 0};	
char number_patterns2[] = {0, BIT0 | BIT6 | BIT7, BIT4, BIT7, BIT0 | BIT6 | BIT7};
int number_index = 0;

char dialInColor;		//to store selected colors 
int dialIn_index = 0;		//index for dialIn_array

int changeDigit = 0;		//for displaying two different digits (under timer ISR)

int codeSet = 0;		//to prevent new code being set while a game is running

//results
int blackPeg = 0;
int whitePeg = 0;
int round = 0;
int win = 0;
int winReset = 0;

void checkAnswer(){
	blackPeg = 0;
	whitePeg = 0;

	cio_printf("\n\r%i ", round);

	for (int i = 0; i < 4; i++){
		cio_printf("%c", dialIn_array[i]);
	}
	cio_printf(": ");	
	
	//arrays to check off to prevent too many white pegs
	int checkDialIn[4] = {1,1,1,1};
	int checkAnswer[4] = {1,1,1,1};

	for (int i = 0; i < 4; i++){
		if (dialIn_array[i] == answer_array[i]){
			blackPeg++;
			checkAnswer[i] = 0;
			checkDialIn[i] = 0;
		}
	}

	for (int i = 0; i < 4; i++){
		for (int j = 0; j < 4; j++){
			if (answer_array[i] == dialIn_array[j] && checkDialIn[i] && checkAnswer[j] && i != j) {
				whitePeg++;
				checkDialIn[i] = 0;
				checkAnswer[j] = 0;
			}
		}
	}

	cio_printf("%i %i",blackPeg,whitePeg);
	
	round++;
	if (blackPeg == 4){
		cio_printf(" WON!!!!\n\r");
		win = 1;
		memset(answer_array, 0, 5);	//clear array
		codeSet = 0;
		round = 0;
	}

	if (round == 10 && win == 0){
		cio_printf(" LOST :P\n\r");
		memset(answer_array, 0, 5);	//clear array
		codeSet = 0;
		round = 0;
		win = 0;
	}
	
	win = winReset;
}

int main(void) {
	// WIZARD WORDS
	WDTCTL  = WDTPW | WDTHOLD;
	BCSCTL1 = CALBC1_1MHZ;
	DCOCTL  = CALDCO_1MHZ;

	// INITIALIZATION
	P2SEL  &= ~(BIT6 | BIT7);	//bic
	P2DIR = 0b11111111;

	P1DIR = 0b11100001;
	P1OUT = BIT7|BIT6;			//enable digits

	//TIMER
	TA0CTL 	 = TASSEL_2 | ID_0 | MC_1;
	TA0CCR0  = 1024;
	TA0CCTL0 = CCIE;

	//BUTTON INTERRUPTS
	P1IE |= BIT3;
	P1IES |= BIT3;
	
	// INITIALIZE SERIAL
	serial_init(9600);

	// INITIALIZE DTC
	initialize_dtc(INCH_4, &pot_value);

	// ENABLE GLOBAL INTERRUPTS
	__eint();

	for(;;) {
		// LOOP FOREVER
		if (codeSet == 0){
			prompt_for_line(answer_array);
			__delay_cycles(250000);

			cio_printf("\n\r? %s  P L", answer_array);

			//prevent code setting
			codeSet = 1;
		}
	}

	// NEVER GET HERE
	return 0;
}


#pragma vector=PORT1_VECTOR
__interrupt void button_isr(void)
{
	// BUTTON INTERRUPT
	if (codeSet == 1){
		dialIn_array[dialIn_index] = dialInColor;

		if (dialIn_index == 3){
			dialIn_index = 0;	//reset index
			checkAnswer();
		}else {
			dialIn_index++;
		}
	}
	// DEBOUNCE ROUTINE
	while (!(BIT3 & P1IN));
	__delay_cycles(32000);
	P1IFG &= ~BIT3;
}

#pragma vector=TIMER0_A0_VECTOR
__interrupt void timer0_isr(void)
{
	// TIMER 0 ISR
	// 1. turn off digits
	P1OUT &= ~(BIT7|BIT6);	//bic
	P1OUT &= ~(BIT0|BIT5);	//bic
	P2OUT &= ~(BIT0|BIT2|BIT4|BIT6|BIT7);	//bic

	// 2. configure segments
	if (changeDigit == 0){
		number_index = blackPeg;
	}else {
		number_index = whitePeg;
	}

	P1OUT |= number_patterns1[number_index];
	P2OUT |= number_patterns2[number_index];

	// 3. turn on next digit 
	if (changeDigit == 0){
		P1OUT |= BIT7;
	}else {
		P1OUT |= BIT6;
	}

	changeDigit++;
	changeDigit %= 2;


	//POT VALUE 
	//CYAN
	if (pot_value < 128){
		P2OUT &= ~(BIT1|BIT3|BIT5);
		P2OUT |= BIT3|BIT5;
		dialInColor = 'C';
	}
	//MAGENTA
	if (pot_value >= 128 && pot_value < 256){
		P2OUT &= ~(BIT1|BIT3|BIT5);
		P2OUT |= BIT1|BIT5;
		dialInColor = 'M';
	}
	//YELLOW
	if (pot_value >= 256 && pot_value < 384){
		P2OUT &= ~(BIT1|BIT3|BIT5);
		P2OUT |= BIT1|BIT3;
		dialInColor = 'Y';
	}
	//BLACK
	if (pot_value >= 384 && pot_value < 512){
		P2OUT &= ~(BIT1|BIT3|BIT5);
		dialInColor = 'K';
	}
	//RED
	if (pot_value >= 512 && pot_value < 640){
		P2OUT &= ~(BIT1|BIT3|BIT5);
		P2OUT |= BIT1;
		dialInColor = 'R';
	}
	//GREEN
	if (pot_value >= 640 && pot_value < 768){
		P2OUT &= ~(BIT1|BIT3|BIT5);
		P2OUT |= BIT3;
		dialInColor = 'G';
	}
	//BLUE
	if (pot_value >= 768 && pot_value < 896){
		P2OUT &= ~(BIT1|BIT3|BIT5);
		P2OUT |= BIT5;
		dialInColor = 'B';
	}
	//WHITE
	if (pot_value >= 896){
		P2OUT &= ~(BIT1|BIT3|BIT5);
		P2OUT |= BIT1|BIT3|BIT5;
		dialInColor = 'W';
	}
}

#pragma vector=TIMER1_A0_VECTOR
__interrupt void timer1_isr(void)
{
	// TIMER 1 ISR
}

