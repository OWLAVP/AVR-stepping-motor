#define F_CPU 1000000UL
#include <avr/io.h>
#include <util/delay.h> 
#include <avr/interrupt.h>
#include <avr/eeprom.h>
#include <avr/wdt.h>
#define setBit(sfr, bit)     (_SFR_BYTE(sfr) |= (1 << bit))
#define clearBit(sfr, bit)   (_SFR_BYTE(sfr) &= ~(1 << bit))
#define toggleBit(sfr, bit) (_SFR_BYTE(sfr) ^= (1 << bit))
#define getBit(sfr, bit)	(_SFR_BYTE(sfr) & (1<<(bit)))
#define	leftPin PD2            
#define rightPin PD3           
#define	stepPin PD5            //Define Step pin
#define dirPin PD6            //Define Direction pin
#define fastPin PB1            
#define slowPin PB2
#define ledPin PB4           
#define AddrEeprom 0           
uint16_t microsec;
ISR (TIMER1_COMPA_vect){
	PORTB ^= _BV(ledPin);
}
void timer1_init(){
	TCCR1B |= (1 << WGM12); //enable CTC mode
	
	OCR1A = 488; //about 2 Hz with 1024 prescale
	
	TCCR1B |= (1 << CS10) | (1 << CS12); // Set up timer with 1024 prescale
	
	TIMSK1 |= (1 << OCIE1A); //enable timer1 CTC interrupt
	sei();
}

void my_delay_us(uint16_t us)
{
	while (0 < us)
	{
		_delay_us(1);
		--us;
	}
}

void FastSlow(){
					if (bit_is_clear(PINB, fastPin) && !bit_is_clear(PINB, slowPin))
					{
						setBit(PORTB, ledPin);
						microsec = microsec - 200;
						eeprom_update_word((uint16_t *) AddrEeprom, microsec);
						_delay_ms(200);
						if (microsec <= 750)
						{
							microsec = 750;
						}
					}
					if (bit_is_clear(PINB, slowPin) && !bit_is_clear(PINB, fastPin))
					{
						setBit(PORTB, ledPin);
						microsec = microsec + 200;
						eeprom_update_word((uint16_t *) AddrEeprom, microsec);
						_delay_ms(200);
						if (microsec >= 2500)
						{
							microsec = 2500;
						}
					}	
}

int main(void)
{
	DDRD |= (1<<dirPin)|(1<<stepPin);     // Configure PORT as output
	DDRD &= ~_BV(leftPin);
	DDRD &= ~_BV(rightPin);
	DDRB &= ~_BV(fastPin);//вход
	DDRB &= ~_BV(slowPin);
	DDRB |= (1<<ledPin); //out
	PORTD |= (1 << leftPin) | (1 << rightPin); //установить "1" на линии  порта B
	PORTB |= (1 << fastPin) | (1 << slowPin); //установить "1" на линии  порта B
	setBit(PORTB, ledPin);
	//eeprom_write_word((uint16_t *) AddrEeprom, 1000);
    microsec = eeprom_read_word((uint16_t*)AddrEeprom); 
	wdt_enable (WDTO_500MS);
	timer1_init();
	for (;;)
	{
		while(!bit_is_clear(PIND, leftPin)){
			wdt_enable (WDTO_500MS);
			PORTD |= (1<<dirPin);                //Make high to rotate motor in clockwise direction
			PORTD |=(1<<stepPin);
			my_delay_us(microsec);
			//_delay_us(1500);
			PORTD &=~(1<<stepPin);
			my_delay_us(microsec);
			//_delay_us(1500);
			FastSlow();

		}
		clearBit(TIMSK1, OCIE1A);
		setBit(PORTB, ledPin);
		wdt_disable();
		_delay_ms(10000);
		setBit(TIMSK1, OCIE1A);
		while(!bit_is_clear(PIND, rightPin)){
			wdt_enable (WDTO_500MS);
			PORTD &= ~(1<<dirPin);              //Make low to rotate motor in anti-clockwise direction
			PORTD |=(1<<stepPin);
			//_delay_us(1500);
			my_delay_us(microsec);
			PORTD &=~(1<<stepPin);
			my_delay_us(microsec);
			//_delay_us(1500);
			FastSlow();				
		}
		clearBit(TIMSK1, OCIE1A);
		setBit(PORTB, ledPin);
		wdt_disable();
		_delay_ms(10000);
		setBit(TIMSK1, OCIE1A);
	}
}
