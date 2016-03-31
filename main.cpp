// skeleton code for ECE 2036 thermostat lab
// code must be added by students
#include "mbed.h"
#include "TMP36.h"
#include "SDFileSystem.h"
#include "uLCD_4DGL.h"
#include "PinDetect.h"
#include "Speaker.h"
// must add your new class code to the project file Shiftbrite.h
#include "Shiftbrite.h"

// use class to setup temperature sensor pins
//TMP36 myTMP36(p15);  //Analog in

// use class to setup microSD card filesystem
SDFileSystem sd(p5, p6, p7, p8, "sd");

// use class to setup the  Color LCD


// use class to setup pushbuttons pins
PinDetect pb1(p23);
PinDetect pb2(p24);
PinDetect pb3(p25);

// use class to setup speaker pin
Speaker mySpeaker(p21); //PWM out
PwmOut speaker(p21);
// use class to setup Shiftbrite pins
Shiftbrite myShiftbrite(p9, p10, p11, p12, p13);// ei li di n/c ci

												// use class to setup Mbed's four on-board LEDs
DigitalOut myLED1(LED1);
DigitalOut myLED2(LED2);
DigitalOut myLED3(LED3);
DigitalOut myLED4(LED4);



//also setting any unused analog input pins to digital outputs reduces A/D noise a bit
//see http://mbed.org/users/chris/notebook/Getting-best-ADC-performance/
DigitalOut P16(p16);
DigitalOut P17(p17);
DigitalOut P18(p18);
DigitalOut P19(p19);
DigitalOut P20(p20);




// Global variables used in callbacks and main program
// C variables in interrupt routines should use volatile keyword
int volatile heat_setting = 78; // heat to temp
int volatile cool_setting = 68; // cool to temp


int volatile pb_status = 0;
// Callback routine is interrupt activated by a debounced pb1 hit
void pb1_hit_callback(void)
{
	pb_status = pb_status | 0x01;
}
// Callback routine is interrupt activated by a debounced pb2 hit
void pb2_hit_callback(void)
{
	pb_status = pb_status | 0x02;
}
// Callback routine is interrupt activated by a debounced pb3 hit
void pb3_hit_callback(void)
{
	pb_status = pb_status | 0x04;
}


//use the new class to set p15 to analog input to read and convert TMP36 sensor's voltage output
TMP36 myTMP36(p15);
//also setting unused analog input pins to digital outputs reduces A/D noise
//see http://mbed.org/users/chris/notebook/Getting-best-ADC-performance/

uLCD_4DGL uLCD(p28, p27, p29); // create a global lcd object

int main()
{
	//float Current_temp=0.0;

	// Use internal pullups for the three pushbuttons
	pb1.mode(PullUp);
	pb2.mode(PullUp);
	pb3.mode(PullUp);
	// Delay for initial pullup to take effect
	wait(.01);
	// Setup Interrupt callback functions for a pb hit
	pb1.attach_deasserted(&pb1_hit_callback);
	pb2.attach_deasserted(&pb2_hit_callback);
	pb3.attach_deasserted(&pb3_hit_callback);
	// Start sampling pb inputs using interrupts
	pb1.setSampleFrequency();
	pb2.setSampleFrequency();
	pb3.setSampleFrequency();
	// pushbuttons now setup and running


	// State machine code below will need changes and additions
	float tempC = 0, tempF = 0;
	float setpoint = 75;
	enum Statetype { Heat_off = 0, Heat_on, Cool_off, Cool_on, off };
	Statetype state = Heat_off;
	enum ModeType { Mode_off = 0, Heat, Cool };
	ModeType mode = Mode_off;
	//int mode = 0;
	// int mode = 0; // heat or cool mode

	// NEST INITIALIZATION
	uLCD.background_color(LGREY);
	uLCD.textbackground_color(WHITE);
	uLCD.cls();
	uLCD.filled_circle(63, 63, 50, WHITE);
	uLCD.locate(7, 5);
	uLCD.color(BLUE);
	uLCD.text_width(1);
	uLCD.text_height(1);
	uLCD.printf("NEST");
	uLCD.locate(4, 6);
	uLCD.printf("INITIALIZING");
	wait(7);

	while (1) {
		tempC = myTMP36.read();
		tempF = (9.0*tempC) / 5.0 + 32.0;

		//CHECK FOR SETTING CHANGE
		switch (pb_status) {
		case 0x01:
			myLED2 = 1;
			myLED3 = 0;
			myLED4 = 0;
			speaker.period(1.0 / 150.0); // 500hz period
			speaker = 0.25; //25% duty cycle - mid range volume
			wait(.02);
			speaker = 0.0; // turn off audio
			setpoint = setpoint + float(1);
			pb_status = 0;
			break;

		case 0x02:
			myLED3 = 1;
			myLED2 = 0;
			myLED4 = 0;
			speaker.period(1.0 / 100.0); // 500hz period
			speaker = 0.25; //25% duty cycle - mid range volume
			wait(.02);
			speaker = 0.0; // turn off audio
			setpoint = setpoint - float(1);
			pb_status = 0;
			break;

		case 0x04:

			switch (mode)
			{
			case Mode_off:
				mode = Heat;
				break;
			case Heat:
				mode = Cool;
				break;
			case Cool:
				mode = Mode_off;
				break;
			default:
				mode = Mode_off;
				break;
			}

			if (mode == 0)
				mySpeaker.PlayNote(800.0, 0.1, 0.8);
			else if (mode == 1)
				mySpeaker.PlayNote(1000.0, 0.1, 0.8);
			else
				mySpeaker.PlayNote(1200.0, 0.1, 0.8);

			pb_status = 0;
			break;

			//no pb or multiple pbs hit
		default:
			pb_status = 0;
			break;
		} //end pushbutton switch



		  // STATE SELECTION
		  //MODE CASES
		  //OFF
		switch (mode) {
		case Mode_off:
			state = off;
			//uLCD.locate(0,0);
			//uLCD.printf("------------------\n|Temp=%5.2F F Off|\n|                |\n|                |\n-----------------\nOFF state   ", tempF);
			uLCD.background_color(LGREY);
			uLCD.textbackground_color(WHITE);
			uLCD.cls();
			uLCD.filled_circle(63, 63, 50, WHITE);
			uLCD.locate(5, 5);
			uLCD.color(BLUE);
			uLCD.text_width(1);
			uLCD.text_height(1);
			uLCD.printf("NEST OFF");
			break;

			//HEATING
		case Heat:
			if (tempF < setpoint) {
				state = Heat_on;
				//uLCD.locate(0,0);
				//uLCD.printf("------------------\n|Temp=%5.2F F    |\n|                |\n|Heating to %1.0F   |\n-----------------\nHeat_ON state ", tempF,setpoint);
				uLCD.background_color(LGREY);
				uLCD.textbackground_color(RED);
				uLCD.cls();
				uLCD.filled_circle(63, 63, 50, RED);
				uLCD.locate(3, 5);
				uLCD.color(WHITE);
				uLCD.text_width(1);
				uLCD.text_height(1);
				uLCD.printf("NEST HEATING");
				uLCD.locate(3, 6);
				uLCD.printf("Temp: %5.2F", tempF);
				uLCD.text_width(3);
				uLCD.text_height(3);
				uLCD.locate(2, 3);
				uLCD.printf("%1.0F", setpoint);
				break;
			}
			else {
				state = Heat_off;
				//uLCD.locate(0,0);
				//uLCD.printf("------------------\n|Temp=%5.2F F    |\n|                |\n|Holding Abv. %1.0F |\n-----------------\nHeat_OFF state", tempF,setpoint);
				uLCD.background_color(LGREY);
				uLCD.textbackground_color(RED);
				uLCD.cls();
				uLCD.filled_circle(63, 63, 50, RED);
				uLCD.locate(3, 5);
				uLCD.color(WHITE);
				uLCD.text_width(1);
				uLCD.text_height(1);
				uLCD.printf("NEST HOLDING");
				uLCD.locate(3, 6);
				uLCD.printf("Temp: %5.2F", tempF);
				uLCD.text_width(3);
				uLCD.text_height(3);
				uLCD.locate(2, 3);
				uLCD.printf("%1.0F", setpoint);
				break;
			}

			//COOLING
		case Cool:
			if (tempF < setpoint) {
				state = Cool_off;
				//uLCD.locate(0,0);
				//uLCD.printf("------------------\n|Temp=%5.2F F    |\n|                |\n|Holding Blw. %1.0F |\n-----------------\nCool_OFF state", tempF,setpoint);
				uLCD.background_color(LGREY);
				uLCD.textbackground_color(BLUE);
				uLCD.cls();
				uLCD.filled_circle(63, 63, 50, BLUE);
				uLCD.locate(3, 5);
				uLCD.color(WHITE);
				uLCD.text_width(1);
				uLCD.text_height(1);
				uLCD.printf("NEST HOLDING");
				uLCD.locate(3, 6);
				uLCD.printf("Temp: %5.2F", tempF);
				uLCD.text_width(3);
				uLCD.text_height(3);
				uLCD.locate(2, 3);
				uLCD.printf("%1.0F", setpoint);
				break;
			}
			else {
				state = Cool_on;
				//uLCD.locate(0,0);
				//uLCD.printf("------------------\n|Temp=%5.2F F    |\n|                |\n|Cooling to %1.0F   |\n-----------------\nCool_ON state", tempF,setpoint);
				uLCD.background_color(LGREY);
				uLCD.textbackground_color(BLUE);
				uLCD.cls();
				uLCD.filled_circle(63, 63, 50, BLUE);
				uLCD.locate(3, 5);
				uLCD.color(WHITE);
				uLCD.text_width(1);
				uLCD.text_height(1);
				uLCD.printf("NEST COOLING");
				uLCD.locate(3, 6);
				uLCD.printf("Temp: %5.2F", tempF);
				uLCD.text_width(3);
				uLCD.text_height(3);
				uLCD.locate(2, 3);
				uLCD.printf("%1.0F", setpoint);

				break;
			}
		}
		wait(0.5);
		// heartbeat LED - common debug tool
		// blinks as long as code is running and not locked up
		myLED1 = !myLED1;
	}        //while
}    //main

