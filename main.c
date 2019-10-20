/*
 * ATMega328P
 *
 * Created: 7/2/2016 9:34:05 PM
 * Author : Elliott Shugerman
 */
#ifndef F_CPU
#define F_CPU 16000000UL // 16 MHz
#endif

#include <avr/interrupt.h>
#include <avr/io.h>
#include <avr/sleep.h>
#include <util/delay.h>

#define set_input(portdir, pin) portdir &= ~(_BV(pin))
#define set_output(portdir, pin) portdir |= (_BV(pin))

// Globals
volatile uint8_t state = 0;
volatile uint8_t sleep_flag = 0;
volatile uint8_t debounce_flag = 0;
volatile uint8_t count_debounce;
volatile uint16_t count_5s;
volatile uint8_t count_1m;

// timer timer
void timer0_init() {
  // set up timer with prescaler = 256
  TCCR0B |= (1 << CS02);

  // initialize counter
  TCNT0 = 0;

  // enable overflow interrupt
  TIMSK0 |= (1 << TOIE0);

  // initialize overflow counter variable
  count_5s = 0; // incremented every .016s
  count_1m = 0;
}

// debounce timer
void timer2_init() {
  // set up timer with prescaler = 64
  TCCR2B |= (1 << CS22);

  // initialize counter
  TCNT2 = 0;

  // enable overflow interrupt
  TIMSK2 |= (1 << TOIE2);
}

void delay_us(int us) {
  for (int i = 0; i < us; i++) {
    _delay_us(1);
  }
}

void beep_state() {

  for (int i = 800; i > 200; i -= 15) {
    PORTC = 0x01;
    delay_us(200);
    PORTC = 0x00;
    delay_us(i);
  }
}

void beep_5s() {
  for (int i = 100; i < 200; i += 5) {
    PORTC = 0x01;
    delay_us(i);
    PORTC = 0x00;
    delay_us(500);
  }
}

void beep_1m() {
  beep_5s();
  _delay_ms(300);
  beep_5s();
  _delay_ms(200);
  beep_5s();
}

void beep_on() {
  _delay_ms(150);

  for (int i = 0; i < 100; i++) {
    PORTC = 0x01;
    delay_us(20);
    PORTC = 0x00;
    delay_us(980);
  }

  _delay_ms(100);

  for (int i = 0; i < 200; i++) {
    PORTC = 0x01;
    delay_us(67);
    PORTC = 0x00;
    delay_us(600);
  }
}

void beep_off() {
  _delay_ms(300);

  for (int i = 0; i < 100; i++) {
    PORTC = 0x01;
    delay_us(67);
    PORTC = 0x00;
    delay_us(600);
  }

  _delay_ms(100);

  for (int i = 0; i < 200; i++) {
    PORTC = 0x01;
    delay_us(50);
    PORTC = 0x00;
    delay_us(950);
  }
}

void go_to_sleep() {
  set_sleep_mode(SLEEP_MODE_PWR_DOWN);
  sleep_enable();
  sleep_bod_disable();
  sleep_cpu();
}

void inc_state() {
  uint8_t new_state = state + 1;
  if (new_state == 4) {
    new_state = 0;
  }

  switch (new_state) {
  case 0:
    beep_state();
    PORTB = 0x00;   // turn off motor
    TCCR0B = 0x00;  // turn off timer
    sleep_flag = 1; // power off
    beep_off();     // power down sound
    break;

  case 1:
    beep_state();
    PORTB = 0x07;
    timer0_init();
    beep_on();
    break;

  case 2:
    beep_state();
    PORTB = 0x0f;
    timer0_init();
    break;

  case 3:
    beep_state();
    PORTB = 0x1f;
    timer0_init();
    break;
  }
  state = new_state;
}

// button input interrupt service routine
ISR(INT0_vect) {
  if (!debounce_flag) {
    inc_state();
    debounce_flag = 1;
    timer2_init();
  }
}

// 5s and 1m timers
ISR(TIMER0_OVF_vect) {
  // keep track of number of overflows
  count_5s++;

  // overflows occur at 244.1 Hz
  if (count_5s >= 1221) {
    count_5s = 0; // reset overflow counter
    count_1m++;

    if (count_1m >= 12) {
      beep_1m();
      count_1m = 0;
    } else {
      beep_5s();
    }
    // timer is reset when it overflows
  }
}

// debounce timer
ISR(TIMER2_OVF_vect) {
  count_debounce++;

  if (count_debounce >= 16) {
    TCCR2B = 0;
    count_debounce = 0;
    debounce_flag = 0;
  }
}

void setup(void) {

  DDRB = 0x01f; // set output motor

  set_output(DDRC, 0); // speaker

  set_input(DDRD, 2); // 328 INT0

  // set_input(DDRB, 7);			//43U INT0

  sei();

  EIMSK |= (1 << INT0);                    // enable int0 interrupt
  EICRA &= ~((1 << ISC00) | (1 << ISC01)); // on low level
  PORTD |= (1 << 2);                       // enable internal pull up	for button
}

int main(void) {
  setup();

  while (1) {

    if (sleep_flag == 1) {
      go_to_sleep();
      sleep_disable();
      sleep_flag = 0;
    }
  }
}
