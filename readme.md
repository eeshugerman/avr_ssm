This code runs on the Atmel ATtiny43U microcontroller. The MCU controls a piezo buzzer via PORTB[0] and a vibration motor via PORTA[0:4] (multiple (5) pins are used to distribute current draw and provide variable current output). The MCU takes input from a momentary tact button via PORTB[7]. On each button press, a beep is emitted and the state is incremented to the next of four possible states (OFF, LOW, MEDIUM, HIGH, OFF, ...) . When OFF, the MCU enters sleep mode. When LOW, MEDIUM, or HIGH, the corresponding current is sent to the vibration motor and a single beep is emitted every 5 seconds, and three beeps (in fast succession) are emitted every minute.