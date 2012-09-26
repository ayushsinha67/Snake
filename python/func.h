#ifndef FUNC_H_
#define FUNC_H_

/************************************************************************
 *	DEFINES
 */
/* Bit Manipulation */
#define CHK(x,b)	(x&b)
#define CLR(x,b)	(x&=~b)
#define SET(x,b)	(x|=b)
#define TOG(a,b)	(a^=b)
#define TOG(a,b)	(a^=b)

/* PORTS */
#define DDR_ROW			DDRA
#define DDR_COL			DDRC
#define DDR_SEL			DDRD
#define PORT_ROW		PORTA
#define PORT_COL		PORTC
#define PORT_SEL		PORTD
#define PIN_SEL			PIND

/* Selection */
#define SEL_UP			0x01
#define SEL_DOWN		0x02
#define SEL_RIGHT		0x04
#define SEL_LEFT		0x08
#define SEL_GAME_CHNG	0x10

/* Direction */
#define UP				3
#define DOWN			4
#define LEFT			1
#define RIGHT			2

/* Timer - in ms */
#define SNAKE_MOV_TIME	400		
#define SWEEP_TIME		1					
#define SCROLL_TIME		100
#define SEL_WAIT		200

/* Choose Game Mode 
	0 : Automatic move
	1 : Single shot Move
*/
#define SNAKE_MODE		1

/************************************************************************
 *	FUNCTION DEFINITIONS
 */
uint8_t		NextPossible		( uint8_t d, uint8_t dnew );
uint8_t		FindNextRow			( uint8_t d );
uint8_t		FindNextCol			( uint8_t d );
void		addTail				( uint8_t nr, uint8_t nc );
void		moveForward			( uint8_t r, uint8_t c );
uint8_t		CheckOverlap		( uint8_t r, uint8_t c);
void		Reload				( void );
uint8_t		foundDot			( uint8_t r, uint8_t c);
void		showSnake			( uint8_t rowNum );
void		showDot				( uint8_t r, uint8_t c, uint8_t rowNum );
void		generateDot			( void );
void		GameOver			( void );
void		ShowScroll			( void );
uint16_t	millis				( void );

#endif /* FUNC_H_ */