//-----------------------------------------------------------------------------------------------------------
// *** Obs�uga wy�wietlaczy alfanumerycznych zgodnych z HD44780 ***
//
// - Sterowanie: tryb 4-bitowy
// - Dowolne przypisanie ka�dego sygna�u steruj�cego do dowolnego pinu mikrokontrolera
// - Praca z pinem RW pod��czonym do GND lub do mikrokontrolera (sprawdzanie BusyFLAG - szybkie operacje LCD)
//
// Pliki 			: lcd44780.c , lcd44780.h
// Mikrokontrolery 	: Atmel AVR
// Kompilator 		: avr-gcc
// �r�d�o 			: http://www.atnel.pl
// Data 			: marzec 2010
// Autor 			: Miros�aw Karda�
//----------------------------------------------------------------------------------------------------------
// Rozmiar kodu z za��czonymi tylko funkcjami: lcd_init(), lcd_cls(), lcd_str() 				(RW<-->GND)
// dla procesor�w AVR: 240 bajt�w !!!
// Rozmiar kodu z za��czonymi tylko funkcjami: lcd_init(), lcd_cls(), lcd_str(), lcd_locate()	(RW<-->GND)
// dla procesor�w AVR: 254 bajty
// Rozmiar kodu z za��czonymi tylko funkcjami: lcd_init(), lcd_cls(), lcd_str() 				(RW<-->uC)
// dla procesor�w AVR: 326 bajt�w !!!
// Rozmiar kodu z za��czonymi tylko funkcjami: lcd_init(), lcd_cls(), lcd_str(), lcd_locate()	(RW<-->uC)
// dla procesor�w AVR: 340 bajt�w
//-----------------------------------------------------------------------------------------------------------
#ifndef LCD_H_
#define LCD_H_
//----------------------------------------------------------------------------------------
//
//		Parametry pracy sterownika
//
//----------------------------------------------------------------------------------------
// rozdzielczo�� wy�wietlacza LCD (wiersze/kolumny)
#define LCD_Y 2		// ilo�� wierszy wy�wietlacza LCD
#define LCD_X 16	// ilo�� kolumn wy�wietlacza LCD

// tu ustalamy za pomoc� zera lub jedynki czy sterujemy pinem RW
//	0 - pin RW pod��czony na sta�e do GND
//	1 - pin RW pod��czony do mikrokontrolera
#define USE_RW 0

//----------------------------------------------------------------------------------------
//
//		Ustawienia sprz�towe po��cze� sterownika z mikrokontrolerem
//
//----------------------------------------------------------------------------------------
// tu konfigurujemy port i piny do jakich pod��czymy linie D7..D4 LCD
#define LCD_D7PORT  C
#define LCD_D7 7
#define LCD_D6PORT  C
#define LCD_D6 6
#define LCD_D5PORT  C
#define LCD_D5 5
#define LCD_D4PORT  C
#define LCD_D4 4


// tu definiujemy piny procesora do kt�rych pod��czamy sygna�y RS,RW, E
#define LCD_RSPORT C
#define LCD_RS 0

//#define LCD_RWPORT F
//#define LCD_RW 2

#define LCD_EPORT C
#define LCD_E 1
//------------------------------------------------  koniec ustawie� sprz�towych ---------------


//----------------------------------------------------------------------------------------
//****************************************************************************************
//*																						 *
//*		U S T A W I E N I A   KOMPILACJI												 *
//*																						 *
//*		W��czamy kompilacj� komend u�ywanych lub wy��czamy nieu�ywanych					 *
//*		(dzi�ki temu regulujemy zaj�to�� pami�ci FLASH po kompilacji)					 *
//*																						 *
//*		1 - oznacza W��CZENIE do kompilacji												 *
//*		0 - oznacza wy��czenie z kompilacji (funkcja niedost�pna)						 *
//*																						 *
//****************************************************************************************
//----------------------------------------------------------------------------------------

#define USE_LCD_LOCATE	1			// ustawia kursor na wybranej pozycji Y,X (Y=0-3, X=0-n)

#define USE_LCD_CHAR 	1			// wysy�a pojedynczy znak jako argument funkcji

#define USE_LCD_STR_P 	1			// wysy�a string umieszczony w pami�ci FLASH
#define USE_LCD_STR_E 	1			// wysy�a string umieszczony w pami�ci FLASH

#define USE_LCD_INT 	1			// wy�wietla liczb� dziesietn� na LCD
#define USE_LCD_HEX 	1			// wy�wietla liczb� szesnastkow� na LCD

#define USE_LCD_DEFCHAR		0		// wysy�a zdefiniowany znak z pami�ci RAM
#define USE_LCD_DEFCHAR_P 	0		// wysy�a zdefiniowany znak z pami�ci FLASH
#define USE_LCD_DEFCHAR_E 	0		// wysy�a zdefiniowany znak z pami�ci EEPROM

#define USE_LCD_CURSOR_ON 		1	// obs�uga w��czania/wy��czania kursora
#define USE_LCD_CURSOR_BLINK 	1	// obs�uga w��czania/wy��czania migania kursora
#define USE_LCD_CURSOR_HOME 	1	// ustawia kursor na pozycji pocz�tkowej

//------------------------------------------------  koniec ustawie� kompilacji ---------------






// definicje adres�w w DDRAM dla r�nych wy�wietlaczy
// inne s� w wy�wietlaczach 2wierszowych i w 4wierszowych
#if ( (LCD_Y == 4) && (LCD_X == 16) )
#define LCD_LINE1 0x00		// adres 1 znaku 1 wiersza
#define LCD_LINE2 0x14		// adres 1 znaku 2 wiersza
#define LCD_LINE3 0x28  	// adres 1 znaku 3 wiersza
#define LCD_LINE4 0x54  	// adres 1 znaku 4 wiersza
#else
#define LCD_LINE1 0x00		// adres 1 znaku 1 wiersza
#define LCD_LINE2 0x40		// adres 1 znaku 2 wiersza
#define LCD_LINE3 0x14  	// adres 1 znaku 3 wiersza
#define LCD_LINE4 0x54  	// adres 1 znaku 4 wiersza
#endif


// Makra upraszczaj�ce dost�p do port�w
// *** PORT
#define PORT(x) SPORT(x)
#define SPORT(x) (PORT##x)
// *** PIN
#define PIN(x) SPIN(x)
#define SPIN(x) (PIN##x)
// *** DDR
#define DDR(x) SDDR(x)
#define SDDR(x) (DDR##x)


// Komendy steruj�ce
#define LCDC_CLS					0x01
#define LCDC_HOME					0x02
#define LCDC_ENTRY					0x04
	#define LCDC_ENTRYR					0x02
	#define LCDC_ENTRYL					0
	#define LCDC_MOVE					0x01
#define LCDC_ONOFF					0x08
	#define LCDC_DISPLAYON				0x04
	#define LCDC_CURSORON				0x02
	#define LCDC_CURSOROFF				0
	#define LCDC_BLINKON				0x01
#define LCDC_SHIFT					0x10
	#define LCDC_SHIFTDISP				0x08
	#define LCDC_SHIFTR					0x04
	#define LCDC_SHIFTL					0
#define LCDC_FUNC					0x20
	#define LCDC_FUNC8B					0x10
	#define LCDC_FUNC4B					0
	#define LCDC_FUNC2L					0x08
	#define LCDC_FUNC1L					0
	#define LCDC_FUNC5x10				0x04
	#define LCDC_FUNC5x7				0
#define LCDC_SET_CGRAM				0x40
#define LCDC_SET_DDRAM				0x80





// deklaracje funkcji na potrzeby innych modu��w
void lcd_init(void);								// W��CZONA na sta�e do kompilacji
void lcd_cls(void);									// W��CZONA na sta�e do kompilacji
void lcd_str(char * str);							// W��CZONA na sta�e do kompilacji

void lcd_locate(uint8_t y, uint8_t x);				// domy�lnie W��CZONA z kompilacji w pliku lcd.c

void lcd_char(char c);								// domy�lnie wy��czona z kompilacji w pliku lcd.c
void lcd_str_P(char * str);							// domy�lnie wy��czona z kompilacji w pliku lcd.c
void lcd_str_E(char * str);							// domy�lnie wy��czona z kompilacji w pliku lcd.c
void lcd_int(int val);								// domy�lnie wy��czona z kompilacji w pliku lcd.c
void lcd_hex(int val);								// domy�lnie wy��czona z kompilacji w pliku lcd.c
void lcd_defchar(uint8_t nr, uint8_t *def_znak);	// domy�lnie wy��czona z kompilacji w pliku lcd.c
void lcd_defchar_P(uint8_t nr, uint8_t *def_znak);	// domy�lnie wy��czona z kompilacji w pliku lcd.c
void lcd_defchar_E(uint8_t nr, uint8_t *def_znak);	// domy�lnie wy��czona z kompilacji w pliku lcd.c

void lcd_home(void);								// domy�lnie wy��czona z kompilacji w pliku lcd.c
void lcd_cursor_on(void);							// domy�lnie wy��czona z kompilacji w pliku lcd.c
void lcd_cursor_off(void);							// domy�lnie wy��czona z kompilacji w pliku lcd.c
void lcd_blink_on(void);							// domy�lnie wy��czona z kompilacji w pliku lcd.c
void lcd_blink_off(void);							// domy�lnie wy��czona z kompilacji w pliku lcd.c

#endif /* LCD_H_ */
