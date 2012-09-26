/*
 * python.c
 *
 * GAMEROID 2012 
 *
 * Created: 24-09-2012 PM 10:13:02
 *  Author: Ayush Sinha
 */ 
#define F_CPU	16000000
#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <util/atomic.h>
#include <avr/pgmspace.h>
#include "func.h"

/************************************************************************
 *	GLOBAL VARIABLES
 */
/* Timer Variables */
volatile uint16_t ms = 0;
volatile uint16_t counter1 = 0;
volatile uint16_t counter2 = 0;
volatile uint16_t counter3 = 0;
volatile uint16_t counter4 = 0;

uint16_t compare;

/* Game Variables */
uint8_t rdot = 0, cdot = 0;								/* Dot location */
uint8_t nextc = 0, nextr = 0;							/* Next step location */
uint16_t snakeSize = 3;									/* Snake Size */
uint8_t snakeRow[64], snakeCol[64];						/* Store Snake */									
uint8_t direction = RIGHT;								/* direction of movement */
uint8_t store[8];						

/* GAME OVER */
const uint8_t letter[10][8] PROGMEM =	{{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
										{ 0x00, 0x3c, 0x20, 0x20, 0x2c, 0x24, 0x3c, 0x00}, 
										{ 0x00, 0x3c, 0x24, 0x24, 0x3c, 0x24, 0x24, 0x00},
										{ 0x00, 0x44, 0x6c, 0x54, 0x44, 0x44, 0x44, 0x00},
										{ 0x00, 0x3c, 0x20, 0x20, 0x38, 0x20, 0x3c, 0x00},
										{ 0x00, 0x3c, 0x24, 0x24, 0x24, 0x24, 0x3c, 0x00},
										{ 0x00, 0x22, 0x22, 0x22, 0x22, 0x14, 0x08, 0x00},
										{ 0x00, 0x3c, 0x20, 0x20, 0x38, 0x20, 0x3c, 0x00},
										{ 0x00, 0x3c, 0x24, 0x24, 0x3c, 0x28, 0x24, 0x00},
										{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
										};

 /************************************************************************
  *	MAIN
  */
void main()
{
	
#if ( SNAKE_MODE == 1 )	
	uint8_t pressed = 0;
#endif
	
	/* Initialize Ports */
	DDR_ROW = 0xFF;										/* Row */
	DDR_COL = 0xFF;										/* Column */
	DDR_SEL	= 0x00;										/* Buttons */
  
	PORT_ROW = 0xFF;
	PORT_COL = 0xFF;
	PORT_SEL = 0xFF;									/* Pull Up */
	
	/* Initialize Timer : CTC mode, PS = 64, TOP = OCR1A, 1ms, 16 Mhz*/  
	TCCR1B |= ( ( 1 << WGM12 ) | (1 << CS10) | (1 << CS11) );  
	TIMSK = ( 1 << OCIE1A ); 
	OCR1A = 249;               
  
	sei();												/* Enable interrupts */

	Reload();											/* Initialize Snake */
	generateDot();
	direction = RIGHT;
	
	/* LOOP */
	while (1){
    
#if ( SNAKE_MODE == 1 )		
		pressed = 0;
#endif	
														/* Up Select */
		if( ( ( PIN_SEL & SEL_UP ) == 0x00 ) && NextPossible( direction, UP ) && ( counter1 == 0 ) ){			
 
 #if ( SNAKE_MODE == 1 )      
			pressed = 1;
			counter1 = SEL_WAIT;
#endif
			direction = UP;	
		}
														/* Down Select */
		if( ( ( PIN_SEL & SEL_DOWN ) == 0x00 ) && NextPossible( direction, DOWN ) && ( counter2 == 0 ) ){		
		
#if ( SNAKE_MODE == 1 )      
			pressed = 1;
			counter2 = SEL_WAIT;
#endif
			direction = DOWN; 	
		}
														/* Left Select */
		if( ( ( PIN_SEL & SEL_LEFT ) == 0x00 ) && NextPossible( direction, LEFT ) && ( counter3 == 0 ) ){		
			
#if ( SNAKE_MODE == 1 )      
			pressed = 1;
			counter3 = SEL_WAIT;
#endif
			direction = LEFT; 		
		}	
														/* Right */
		if( ( ( PIN_SEL & SEL_RIGHT ) == 0x00 ) && NextPossible( direction, RIGHT ) && ( counter4 == 0 ) ){		
			
#if ( SNAKE_MODE == 1 )      
			pressed = 1;
			counter4 = SEL_WAIT;
#endif
			direction = RIGHT; 
	
		}
		
#if ( SNAKE_MODE == 0 )									/* Check Timer */
		ATOMIC_BLOCK( ATOMIC_RESTORESTATE ){
			compare = millis();							
		}	 
	
		if( compare >= SNAKE_MOV_TIME ) {
#endif

#if ( SNAKE_MODE == 1 )      
		if( pressed == 1 ) { 
#endif				
			      
			nextr = FindNextRow( direction );			/* Generate Next Row */
			nextc = FindNextCol( direction );			/* Generate Next Column */
       
			if( CheckOverlap( nextr, nextc ) ){			/* Check for snake overlap */
          
				GameOver();								/* Display Game Over */
				_delay_ms(500);  
				Reload();								/* Reload */
			}
       
			else if( foundDot( nextr, nextc ) ){		/* Check if Dot found */
        
				addTail( nextr, nextc );				/* Add trail to end */
				moveForward( nextr, nextc );			/* Move Forward */
				generateDot();							/* Create another random dot */
			}
       
			else { 
				moveForward( nextr, nextc );			/* OR just keep moving */
			}
       
#if ( SNAKE_MODE == 0 )
			ms = 0;										/* Reset Interrupt Counter */
#endif
		}
		
		for( uint8_t i = 0; i < 8; i++ ){				/* Display Sweep */
    
			PORT_ROW = 0x00;
			PORT_ROW |= ( 1 << i );
			showSnake( i );								/* latch snake to ports */
			showDot( rdot, cdot, i );					/* latch dot to ports */
			_delay_ms( SWEEP_TIME );		
		}
		
	}	
}

/************************************************************************
 *	TIMER INTERRUPT
 */
ISR(TIMER1_COMPA_vect)
{
	ms++;  
	
	if( counter1 > 0 )
		counter1--;
		
	if( counter2 > 0 )
		counter2--;
		
	if( counter3 > 0 )
		counter3--;
		
	if( counter4 > 0 )
		counter4--;
}

  
  
  
  
  
  
  
  
  
  
  
  


