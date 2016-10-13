/*
 * main.c
 *
 *  Created on: 2012-01-13
 *      Author: Piotr
 */
#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <stdlib.h>
#include <stdio.h>
#include "lcd44780.h"
#include <avr/eeprom.h>
#include <avr/pgmspace.h>
#define MAX_PROGRAM 120                    //maxymalna liczba programow do zapisania
#define KEY_RIGHT (!(PIND&_BV(4)))      //klawisze
#define KEY_LEFT (!(PIND&_BV(6)))
#define KEY_OK (!(PIND&_BV(5)))
#define KEY_START ((PINB&_BV(1)))
#define KEY_RESET_ZERO (!(PIND&_BV(0))) //klawisz reset pozycji

#define RELAY4_ON PORTA|=1<<1

#define MOTOR_FAST PORTA|=1<<2                  //ustawienia silnika
#define MOTOR_SLOW PORTA&=~(1<<2)
#define MOTOR_START PORTA|=1<<3
#define MOTOR_STOP PORTA&=~(1<<3)

#define MOTOR_RIGHT PORTA|=1<<4
#define MOTOR_LEFT PORTA&=~(1<<4)

#define ZERO_SENSOR (PIND&_BV(2)) //CZujnik pozycji 0

// przyciski lewo pd6 ok pd5 prawo pd4
//czujnik kata T0
//czujnik 0 INT0
//przekazniki od PA1-PA4
#define TRUE 1
#define FALSE 0
#define RIGHT 82
#define LEFT 76
#define START 0
#define WORK 1
#define STOP 2
//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
volatile uint16_t set_angle =0;
volatile uint16_t set_angle_dog =0;
volatile uint16_t set_angle_odg =0;
volatile uint16_t total_angle=0;
volatile uint16_t actual_angle = 0;
uint8_t end =0;
uint8_t work=0,stop = 0;
volatile uint8_t zero = FALSE;
volatile uint8_t state =3 ;        //0-poczatek ,1-kreci,2-koniec
uint8_t display =0;
uint16_t temp = 0;
char buffer[20];
uint8_t program =0;
volatile uint8_t calibrate =0;
uint16_t temp_angle=0,temp_angle_dog=0,temp_angle_odg=0,temp_dir=' ';
uint16_t program_table[MAX_PROGRAM][4] = {
            {0,0,0,LEFT},
            {0,1,0,RIGHT},
            {0,0,0,LEFT},
            {0,0,0,LEFT},
            {0,0,0,RIGHT},

};
EEMEM uint16_t program_table_eeprom[MAX_PROGRAM][4] = {
            {0,0,0,LEFT},
            {0,0,0,RIGHT},
            {0,0,0,LEFT},
            {0,0,0,LEFT},
            {0,0,0,RIGHT},
            {0,0,0,LEFT},
            {0,0,0,RIGHT},
            {0,0,0,LEFT},
            {0,0,0,LEFT},
            {0,0,0,RIGHT},
            {0,0,0,LEFT},


};
void edit_program();
void find_zero();
int main(void)
{
	DDRA = 255;
	PORTA = 0;
	DDRD = 0;
	PORTD = (1<<4)|(1<<5)|(1<<6)|(1<<0);
	MCUCR = 1<<ISC11|1<<ISC01;
	GICR = 1<<INT1|1<<INT0;
	sei();
	lcd_init();
	lcd_cls();
	lcd_home();
	lcd_str(" Skrecarka v2.1");
	lcd_locate(1,0);
	lcd_str("  ");
	lcd_str(__DATE__);
	do{
	display =0;
	eeprom_read_block(program_table,program_table_eeprom,sizeof(program_table));
	_delay_ms(2200);
	lcd_cls();
	lcd_home();
	lcd_str("Wybierz program");
	_delay_ms(1000);
	do{
		if(KEY_LEFT){
			_delay_ms(100);
			if(KEY_LEFT){
			program--;
			if (program == 255)program = 0;
		}
		}
		if(KEY_RIGHT){
			_delay_ms(100);
			if(KEY_RIGHT){
			program++;
			if (program == MAX_PROGRAM)program = 119;
		}
		}

			if(KEY_OK){
				_delay_ms(100);
				if(KEY_OK){
				break;
			}
		}
			lcd_cls();
			lcd_home();
			lcd_str("Prg:");
			lcd_str(itoa(program,buffer,10));
			lcd_locate(1,0);
			lcd_str("G:");
			lcd_str(itoa(program_table[program][0],buffer,10));
			lcd_char(0b11011111);
			lcd_str("D:");
			lcd_str(itoa(program_table[program][1],buffer,10));
			lcd_str("O:");
			lcd_str(itoa(program_table[program][2],buffer,10));
			lcd_char(0b11011111);
			lcd_str(" ");
			lcd_char(program_table[program][3]);
			_delay_ms(30);

}while(1);
actual_angle =0;
while(1){
    display =1;
    if(KEY_OK){    //edycja programu
              _delay_ms(500);
                 if(KEY_OK)edit_program();
    }
    if(KEY_RIGHT&&KEY_LEFT){    //edycja programu
       _delay_ms(100);
       if(KEY_RIGHT&&KEY_LEFT)
         {
           break;
    }
}
if(KEY_LEFT){
    _delay_ms(30);
        if(KEY_LEFT){
            MOTOR_LEFT;
            _delay_ms(50);
            MOTOR_START;
            do{

            }while(KEY_LEFT);
            MOTOR_STOP;
        }
}
if(KEY_RIGHT){
    _delay_ms(30);
        if(KEY_RIGHT){
            MOTOR_RIGHT;
            _delay_ms(50);
            MOTOR_START;
            do{

            }while(KEY_RIGHT);
            MOTOR_STOP;
        }
}
if(KEY_RESET_ZERO){
    _delay_ms(10);
    if(KEY_RESET_ZERO){
        actual_angle = 0;
        _delay_ms(1000);
        if(KEY_RESET_ZERO){
            find_zero();
        }
    }

}
if(KEY_START){
    display =2;
            _delay_ms(200);
            if((KEY_START) && (end == 0)){
                state= START;
        }
            if((KEY_START) && (end == 1)){
                           find_zero();
                   }
    }

    if(state == START){
        actual_angle =0;
        MOTOR_STOP;
        display =1;
        state = WORK;
        _delay_ms(20);
        MOTOR_FAST;
        //start programu w lewo
        if(program_table[program][3] == 'L'){
            actual_angle =0;
            MOTOR_LEFT;
            _delay_ms(500);
            MOTOR_START;
            do{
//&&&&&&&&&&&&&&&&&&&&&&&&&GIECIE I DOGINANIE&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&
            }while(actual_angle != program_table[program][0]+program_table[program][1]);
            MOTOR_STOP;
            _delay_ms(250);
            MOTOR_RIGHT;
            _delay_ms(250);
 //*************************ODGINANIE****************************************************
            actual_angle =0;
            MOTOR_START;
            _delay_ms(250);
            do{
            }while(actual_angle != program_table[program][2]);
            cli();
            MOTOR_STOP;
        }
        if(program_table[program][3] == 'R'){
            actual_angle =0;
                    MOTOR_RIGHT;
                    _delay_ms(500);
                    MOTOR_START;
                    do{

                    }while(actual_angle != program_table[program][0]+program_table[program][1]);

                    MOTOR_STOP;
                    _delay_ms(250);
                    MOTOR_RIGHT;
                    _delay_ms(250);
                    MOTOR_START;
                    actual_angle =0;
                    _delay_ms(250);
                    do{
                    }while(actual_angle != program_table[program][2]);
                    cli();
                    MOTOR_STOP;
                }
        work =0;
        end = 1;
        state=STOP;
sei();
    }
    lcd_cls();
    lcd_home();
    lcd_str("G:");
    lcd_str(itoa(program_table[program][0],buffer,10));
    lcd_char(0b11011111);
    lcd_str(" D:");
    lcd_str(itoa(program_table[program][1],buffer,10));
    lcd_char(0b11011111);
    lcd_str(" O:");
        lcd_str(itoa(program_table[program][2],buffer,10));
        lcd_char(0b11011111);
    lcd_locate(1,0);
    lcd_str("Ka:");
    lcd_str(itoa(actual_angle,buffer,10));
    lcd_char(0b11011111);
    lcd_locate(1,15);
    lcd_char(program_table[program][3]);

    _delay_ms(20);

	}

	}while(1);
}
void find_zero(void)
{
  display= 4;
  calibrate =1;

  zero = FALSE;
    lcd_cls();
     lcd_home();
      lcd_str("Powrot do poz. 0");
      if(!ZERO_SENSOR){
          MOTOR_LEFT;
          _delay_ms(300);
          MOTOR_START;
          do{

          }while(!ZERO_SENSOR);
          MOTOR_STOP;
          _delay_ms(200);
      }
      MOTOR_FAST;

        MOTOR_RIGHT;
        MOTOR_START;
  while(zero == FALSE){
      do{
      }while(zero != TRUE);
	MOTOR_STOP;
	end = 0;
	actual_angle =0;
	work =0;
  }
  work =0;
  calibrate = 0;
}
void edit_program(void){
  display =3;
                        lcd_cls();
                       lcd_home();
                       lcd_str("     Edycja");
                       _delay_ms(1000);
                       temp_angle =program_table[program][0];
                       temp_angle_dog=program_table[program][1];
                       temp_angle_odg = program_table[program][2];


                       do{
                       lcd_cls();
                       lcd_home();
                       lcd_str("Kat giecia:");
                       lcd_locate(1,0);
                       if(KEY_RIGHT){
                         _delay_ms(80);
                          if(KEY_RIGHT){
                           temp_angle+=2;
                     }
               }
                       if(KEY_LEFT){
                          _delay_ms(80);
                            if(KEY_LEFT){
                                if(temp_angle !=0)
                                  {
                            temp_angle-=2;
                                  }

                     }
               }
                       lcd_str(itoa(temp_angle,buffer,10));
                       lcd_char(0b11011111);
                       _delay_ms(30);
                       }while(!KEY_OK);
                       _delay_ms(1000);


                       do{
                                              lcd_cls();
                                              lcd_home();
                                              lcd_str("Kat dogiecia:");
                                              lcd_locate(1,0);
                                              if(KEY_RIGHT){
                                                _delay_ms(80);
                                                 if(KEY_RIGHT){
                                                  temp_angle_dog+=2;
                                            }
                                      }
                                              if(KEY_LEFT){
                                                 _delay_ms(80);
                                                   if(KEY_LEFT){
                                                       if(temp_angle_dog !=0){
                                                       temp_angle_dog-=2;
                                                       }
                                            }
                                      }
                                              lcd_str(itoa(temp_angle_dog,buffer,10));
                                              lcd_char(0b11011111);
                                              _delay_ms(30);
                                              }while(!KEY_OK);
                       _delay_ms(1000);
                       do{
                                                                     lcd_cls();
                                                                     lcd_home();
                                                                     lcd_str("Kat odgiecia:");
                                                                     lcd_locate(1,0);
                                                                     if(KEY_RIGHT){
                                                                       _delay_ms(80);
                                                                        if(KEY_RIGHT){
                                                                         temp_angle_odg+=2;

                                                                   }
                                                             }
                                                                     if(KEY_LEFT){
                                                                        _delay_ms(80);
                                                                          if(KEY_LEFT){
                                                                              if(temp_angle_odg !=0){
                                                                                  temp_angle_odg-=2;
                                                                              }
                                                                   }
                                                             }
                                                                     lcd_str(itoa(temp_angle_odg,buffer,10));
                                                                     lcd_char(0b11011111);
                                                                     _delay_ms(30);
                                                                     }while(!KEY_OK);
                       _delay_ms(1000);
                                              do{
                                                                     lcd_cls();
                                                                     lcd_home();
                                                                     lcd_str("Kierunek:");
                                                                     lcd_locate(1,0);
                                                                     if(KEY_RIGHT){
                                                                       _delay_ms(80);
                                                                        if(KEY_RIGHT){
                                                                         temp_dir=RIGHT;
                                                                   }
                                                             }
                                                                     if(KEY_LEFT){
                                                                        _delay_ms(80);
                                                                          if(KEY_LEFT){
                                                                              temp_dir=LEFT;
                                                                   }
                                                             }

                                                                     lcd_char(temp_dir);
                                                                     _delay_ms(30);
                                                                     }while(!KEY_OK);
                                              lcd_cls();
                                              lcd_home();
                                             lcd_str("Zapisano!");
                                             program_table[program][0] = temp_angle;
                                             program_table[program][1] = temp_angle_dog;
                                             program_table[program][2] = temp_angle_odg;
                                             program_table[program][3] = temp_dir;
                                             eeprom_update_block(program_table,program_table_eeprom,sizeof(program_table));
                                             _delay_ms(400);
                                             lcd_cls();
                                             lcd_home();


  }
ISR(INT1_vect){
actual_angle+=2;
if(display == 1){
    if(temp != actual_angle){
    lcd_cls();
    lcd_locate(1,0);
    lcd_str("Ka:");
    lcd_str(itoa(actual_angle,buffer,10));
    lcd_char(0b11011111);
    temp = actual_angle;
    }
}
}
ISR(INT0_vect){
  if(calibrate == 1){
if (!ZERO_SENSOR){
    MCUCR = 1<<ISC11|1<<ISC01|1<<ISC00;                //rosnace
    MOTOR_SLOW;

}
if (ZERO_SENSOR){
    MCUCR = 1<<ISC11|1<<ISC01;                    //opadajace
    MOTOR_FAST;
    zero = TRUE;
}
  }

}
