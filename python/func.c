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
extern volatile uint16_t ms;
extern uint16_t	compare;

/* Game Variables */
extern uint8_t rdot, cdot;									/* Dot location */
extern uint8_t nextc, nextr;								/* Next step location */
extern uint16_t snakeSize;									/* Snake Size */
extern uint8_t snakeRow[64], snakeCol[64];					/* Store Snake */									
extern uint8_t direction;									/* direction of movement */
extern uint8_t store[8];	

/* Game Over */
extern const uint8_t letter[10][8] PROGMEM;	

/************************************************************************
 *	RETURN TIME
 */
uint16_t millis(void)
{
    return ms;
}

/************************************************************************
 *	RELOAD
 */
void Reload ( void ){
 
	for( uint8_t i = 0 ; i < snakeSize; i++ ) {					/* Clearing */
		snakeRow[i] = 0; 
		snakeCol[i] = 0;
	}
 
	snakeSize = 3;												/* Initialize Length */
	snakeRow[0] = 7;											/* Initialize Row */
	snakeRow[1] = 7;
	snakeRow[2] = 7;
 
	snakeCol[0] = 5;											/* Initialize Column */
	snakeCol[1] = 6;
	snakeCol[2] = 7;
 
	direction = RIGHT;											/* Start Direction */
	generateDot();												/* Plot Dot */

}

/************************************************************************
 *	NEXT POSSIBLE 
 */
uint8_t NextPossible( uint8_t d, uint8_t dnew ){
  
	if ( ( d == RIGHT ) && ( dnew == LEFT ) )					/* Stop opposite direction movement*/
		return 0; 
		
	else if ( ( d == UP ) && ( dnew == DOWN ) )
		return 0;   
		
	else if ( ( d == LEFT ) && ( dnew == RIGHT ) )
		return 0; 
		
	else if ( ( d == DOWN ) && ( dnew == UP ) )
		return 0;
		
	else
		return 1;
}

/************************************************************************
 *	FUND NEXT ROW
 */
uint8_t FindNextRow ( uint8_t d ){
  
	uint8_t r = 0;
	
	r = snakeRow[0];
	
	if( d == UP ) {												/* If going upwards */
		if( ( r-1 ) == -1 )										/* Loop to bottom */
			r = 7; 
		else
			r--;												/* OR go up if space */
	}
	
	else if( d == DOWN ){										/* if going down */          
		if( ( r+1 ) == 8 )										/* Loop to top */ 
			r = 0; 
		else
			r++;												/* OR go down if space */
	}

	return r;    
}

/************************************************************************
 *	FIND NEXT COLUMN
 */
uint8_t FindNextCol ( uint8_t d ){

	uint8_t c = 0;
	c = snakeCol[0];
  
	if( d == LEFT ){											/* If going LEFT */
		if( ( c+1 ) == 8 )												
			c = 0; 
		else
			c++; 
	}
	
	else if ( d == RIGHT ){										/* If going RIGHT */
		if( ( c-1 ) == -1 )
			c = 7; 
		else
			c--; 
	}
  
	return c;
}

/************************************************************************
 *	ADD TAIL
 */
void addTail( uint8_t nr, uint8_t nc ){
 
	snakeSize++;
 
	for( uint8_t i = snakeSize; i > 0; i-- ){					/* Shift and add */
		snakeRow[i] = snakeRow[i-1];
		snakeCol[i] = snakeCol[i-1];
	}
 
	snakeRow[0] = nr;											/* Add Tail */
	snakeCol[0] = nc;
}

/************************************************************************
 *	MOVE FORWARD
 */
void moveForward( uint8_t r, uint8_t c ){

	for( uint8_t i = snakeSize; i > 0; i-- ){					/* Shift */
		
		snakeRow[i] = snakeRow[i-1];
		snakeCol[i] = snakeCol[i-1];
  }
  
  snakeRow[0] = r;
  snakeCol[0] = c;
 
}

/************************************************************************
 *	GENERATE DOT
 */
void generateDot ( void ){
 
	uint8_t found = 1;											/* Assume found */
	
	do{
 
		rdot = (uint8_t) ( rand() % 8 );						/* Generate random points */
		cdot = (uint8_t) ( rand() % 8 );
	
		for( uint8_t i = 0; i < snakeSize; i++ ){
  
			if( ( rdot == snakeRow[i] ) && ( cdot == snakeCol[i] ) )
				found = 0;										/* Retry if point on snake body */   
		} 
	} while( !found );
}

/************************************************************************
 *	DOT FOUND
 */
uint8_t foundDot( uint8_t r, uint8_t c ){
  
	if( ( r == rdot ) && ( c == cdot ) )						/* Check if points match */
		return 1; 
	else 
		return 0;
}

/************************************************************************
 *	CHECK OVERLAP OF SNAKE
 */
uint8_t CheckOverlap( uint8_t r, uint8_t c ){
  
	for( uint8_t i = 0; i < snakeSize; i++ ){
  
		if( ( snakeRow[i] == r ) && ( snakeCol[i] == c ) )
			return 1; 											/* Overlap Found */
	}
  
	return 0;
}

/************************************************************************
 *	SHOW SNAKE
 */
void showSnake( uint8_t rowNum ){
  
	PORT_COL = 0x00;											/* Clear Column */
	
	for( uint8_t i = 0 ; i < snakeSize; i++ ){				
	
		if( snakeRow[i] == rowNum ) {							/* Check the columns high in the row */
       
			PORT_COL |= ( 1 << snakeCol[i] );
		}
	}  
}

/************************************************************************
 *	SHOW DOT
 */
void showDot( uint8_t r, uint8_t c, uint8_t rowNum ){
   
	if( r == rowNum )    
		PORT_COL |= ( 1 << c );									/* Shift bit to column */
}

/************************************************************************
 *	GAME OVER
 */
void GameOver ( void ){

  PORT_ROW = 0x00;
  PORT_COL = 0x00;
  uint8_t i = 0, c, r, k;
  uint8_t n = 1;  
  
  while( n < 9 ){
	  
	while( i <= 7 ){
      
		ATOMIC_BLOCK( ATOMIC_RESTORESTATE ){
			compare = millis();									/* Check Timer */
		}	  
      
		if( compare >= SCROLL_TIME ){ 
      
			for( k = 0 ; k < 8; k++ ) {							/* Clearing */
				store[k] = 0x00; 
			}		
        
			for( c = 7; c > 0; c-- ){							/* Back of First Letter */
				for( r = 0; r < 8; r++ ){
																/* Writing */
					if( CHK( pgm_read_byte( &letter[n-1][r] ) , ( 1 << c ) ) )	
						SET( store[r], 1 << ( c + i + 1) ); 
				}
			}
        
			for( c = 0; c <= i; c++ ){							/* Front of 2nd Letter */
				for( r = 0; r < 8; r++ ){
																/* Writing */
					if( CHK( pgm_read_byte( &letter[n][r] ) , 1 << ( 7-c ) ) )	
						SET( store[r], 1 << ( i-c ) ); 
				}        
			} 
       
			i++;												/* Scroll */
			ms = 0;												/* Reset Timer */
		}
       
		ShowScroll();											/* Display */
	}
    
	n++; i=0;													/* Increment, Initialize */
	}
}  

/************************************************************************
 *	SHOW SCROLL
 */
void ShowScroll ( void ){

	for( uint8_t i = 0; i < 8; i++ ){							/* Display Sweep */
    
		PORT_ROW = 0x00;
		PORT_ROW |= ( 1<<i );
		
		_delay_ms( SWEEP_TIME );
      
		PORT_COL = 0x00;
		PORT_COL = store[i];
		
	}  
}

