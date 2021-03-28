// Jake Jensen, Rpi3B 16x2 LCD kit

#include <wiringPi.h> 
#include <stdio.h>
#include <iostream>
#include <bitset>

#include <cstdio>
#include <memory>
#include <stdexcept>
#include <string>
#include <array>

// GPIO pins need to be exported before they can work. 
// gpio export <BCM pin>

// Character creator
// https://www.electronicsforu.com/wp-contents/uploads/2016/11/table-for-CGRAM-800x244.jpg

char Heart[8] = {0b00000,0b01010,0b11111,0b11111,0b01110,0b00100,0b00000,0b00000};
char Allone[8] = { 0b00000,0b00000,0b11111,0b11111,0b11111,0b11111,0b11111,0b11111 };
char b[8] = {0x10,0x10,0x16,0x19,0x11,0x11,0x1E};
char Blank = { 0x0 };

#define	RS 2
#define RW 3
#define EN 4
#define D0 17
#define D1 27
#define D2 22
#define D3 10
#define D4 9
#define D5 11 
#define D6 5
#define D7 6

// Pulses the enable pin on and off. Should be a 125us transaction
// But 5ms is safe for testing purposes.
void PulseEnable()
{
	digitalWrite(EN, HIGH);
	delay(5);
	digitalWrite(EN, LOW);
}

// Pulls RegisterSelect (rs) high, which allows data to be written
// To the data register, then ANDs the incoming char with a 
// Mask which allows us to select a single bit at a time. 
// Eg (0000(1)000)
void DataWriteEX(char value)
{
	digitalWrite(EN, LOW);
	digitalWrite(RS, HIGH);
	digitalWrite(RW, LOW);

	digitalWrite(D7, (value & 0x80) >> 7);
	digitalWrite(D6, (value & 0x40) >> 6);
	digitalWrite(D5, (value & 0x20) >> 5);
	digitalWrite(D4, (value & 0x10) >> 4);
	digitalWrite(D3, (value & 0x08) >> 3);
	digitalWrite(D2, (value & 0x04) >> 2);
	digitalWrite(D1, (value & 0x02) >> 1);
	digitalWrite(D0, value & 0x01);

	PulseEnable();
}

// Does the same as the DataWriteEX function, except
// Pulls the RS pin low, allowing data to be written
// To the command register.
void CommandWriteEX(char value)
{
	digitalWrite(EN, LOW);
	digitalWrite(RS, LOW);
	digitalWrite(RW, LOW);

	digitalWrite(D7, (value & 0x80) >> 7);
	digitalWrite(D6, (value & 0x40) >> 6);
	digitalWrite(D5, (value & 0x20) >> 5);
	digitalWrite(D4, (value & 0x10) >> 4);
	digitalWrite(D3, (value & 0x08) >> 3);
	digitalWrite(D2, (value & 0x04) >> 2);
	digitalWrite(D1, (value & 0x02) >> 1);
	digitalWrite(D0, value & 0x01);

	PulseEnable();
}

/*
The following functions are quick macros essentially to
Perform a command. Clearing the screen, moving the cursor
And etc. More of a cheat sheet.
*/

void ClearScreen() {
	CommandWriteEX(0x01);
}

void ReturnHome() {
	CommandWriteEX(0x02);
}

void CursorShiftLeftInc() {
	CommandWriteEX(0x04);
}

void CursorShiftRightInc() {
	CommandWriteEX(0x06);
}

void DisplayShiftLeft() {
	CommandWriteEX(0x07);
}

void DisplayShiftRight() {
	CommandWriteEX(0x05);
}

void DisplayOffCursorOff()
{
	CommandWriteEX(0x08);
}

void DisplayOffCursorOn() {
	CommandWriteEX(0x0A);
}

void DisplayOnCursorOff() {
	CommandWriteEX(0x0C);
}

void DisplayOnCursorBlink() {
	CommandWriteEX(0x0E);
	CommandWriteEX(0x0F);
}

void CursorPositionLeft() {
	CommandWriteEX(0x10);
}

void CursorPositionRight() {
	CommandWriteEX(0x14);
}

void DisplayEntireShiftLeft() {
	CommandWriteEX(0x18);
}

void DisplayEntireShiftRight() {
	CommandWriteEX(0x1C);
}

void CursorHomeTop() {
	CommandWriteEX(0x80);
}

void CursorHomeBottom() {
	CommandWriteEX(0xC0);
}

/* TODO: FIX THE CURSOR CLEARING BUG */
// The main backbone function. Takes a string, the desired 
// Cursor position, the requested line (top or bottom), and
// Checks if we should clear the line of data first. 
// The string is split into chars piece by piece, and 
// The resulting char data is fed into the data senders above. 
// It also checks if the length of the string is greater than
// 16, and if so, send the rest to the second row. 
// Bug: Currently doesn't allow starting a message
// Further on than position 0.
void StringWrite(std::string Message, int CursorPosition = 0, int Line = 0, bool ClearLine = true) {
	if (ClearLine) {
		switch (Line)
		{
		case 0:
			CommandWriteEX(0x80);
			break;

		case 1:
			CommandWriteEX(0xC0);
			break;
		default:
			break;
		}
		CommandWriteEX(0x80);
		for (int i = 0; i < 15; i++) {
			DataWriteEX(0x20);
			CommandWriteEX(0x06);
		}
	}

	switch (Line)
	{
	case 0:
		CommandWriteEX(0x80);
		break;

	case 1:
		CommandWriteEX(0xC0);
		break;
	default:
		break;
	}

	for (int i = 0; i < CursorPosition; i++) {
		CommandWriteEX(0x06);
	}

	bool Dropped = false;
	for (std::size_t i = 0; i < Message.size(); ++i)
	{
		if (i > 15) {
			if (!Dropped) {
				CursorHomeBottom();
				Dropped = true;
			}
		}
		char XK = Message[i];
		DataWriteEX(XK);
	}
	
}

// Initializes the controller to 8 bit mode (8 wires)
// Instead of 4 bit mode (double nibble, 4 wires, half speed)
void Initialize8Bit()
{
	digitalWrite(EN, LOW);
	digitalWrite(RS, LOW);
	digitalWrite(RW, LOW);
	CommandWriteEX(0x30);
	PulseEnable();

	CommandWriteEX(0x30);
	PulseEnable();

	CommandWriteEX(0x30);
	PulseEnable();

	CommandWriteEX(0x38);
	PulseEnable();

	CommandWriteEX(0x08);
	PulseEnable();

	CommandWriteEX(0x01);
	PulseEnable();

	CommandWriteEX(0x06);
	PulseEnable();
}

// WiringPi related stuff. Simply tells the host which GPIO pins to
// Use, and in what mode.
void InitializePins(unsigned int RSPin, unsigned int RWPin, unsigned int ENPin, unsigned int D0Pin, unsigned int D1Pin, unsigned int D2Pin, unsigned int D3Pin, unsigned int D4Pin, unsigned int D5Pin, unsigned int D6Pin, unsigned int D7Pin) {

	pinMode(RSPin, OUTPUT);
	pinMode(RWPin, OUTPUT);
	pinMode(ENPin, OUTPUT);
	pinMode(D0Pin, OUTPUT);
	pinMode(D1Pin, OUTPUT);
	pinMode(D2Pin, OUTPUT);
	pinMode(D3Pin, OUTPUT);
	pinMode(D4Pin, OUTPUT);
	pinMode(D5Pin, OUTPUT);
	pinMode(D6Pin, OUTPUT);
	pinMode(D7Pin, OUTPUT);
}

// Taken from a guy on StackOverflow so long ago, I can't remember
// His name. Essentially gets the output of a command and returns
// The string of it.
std::string exec(const char* cmd) {
	std::array<char, 128> buffer;
	std::string result;
	std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(cmd, "r"), pclose);
	if (!pipe) {
		throw std::runtime_error("popen() failed!");
	}
	while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
		result += buffer.data();
	}
	return result;
}

int main(void)
{
	wiringPiSetupSys(); // Initialize WiringPi
	
	InitializePins(RS, RW, EN, D0, D1, D2, D3, D4, D5, D6, D7);

	std::cout << "Initializing display.\n";
	Initialize8Bit();
	// delay(500);

	std::cout << "Clearing display\n";
	CommandWriteEX(0x01);
	std::cout << "Display on, blink cursor\n";
	CommandWriteEX(0x0E);

	std::cout << "Forcing cursor to start.\n";
	CommandWriteEX(0x80);

	std::cout << "Writing new data.\n";
	StringWrite("Sapphire Labs");
	CommandWriteEX(0x0F);

	std::string Status = "";
	std::string Printer = "";

        // Pings Google DNS, and checks if the return packet was received.
	while (true) {
		Status = exec("ping 8.8.8.8 -c 1");
		if (Status.find("1 received") != std::string::npos) {
			Printer = "Network: Online ";
		}
		else {
			Printer = "Network: Offline";
		}
		StringWrite(Printer, 0, 1, false);
		delay(15000);
	}
	

}
