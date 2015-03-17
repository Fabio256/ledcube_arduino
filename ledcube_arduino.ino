// Led cube software, Arduino side - by cybervinc
//
// I'm using :
// - an ATMEGA328 with the Duemilanove bootloader
// - 8 74HC164 shift registers to drive the 64 columns
// - 8 NPN transistors to drive the lines


// Cube constants, using bytes to save program space (no need for ints here)
const byte clock=19; // Clock pin of all the shift registers
const byte data_pin[8]={2,3,4,14,15,16,17,18}; // Data pins of the 8 shift registers
const byte floors[8]={6,11,9,7,5,12,10,8}; // Pins of the transistors driving the floors

// Wiring of the shift registers, [x][y] and then "shift register id" and "bit number"
const byte reg_wiring[8][8][2]={
{{3,5},{3,4},{4,7},{5,1},{0,6},{1,0},{2,3},{2,2}},
{{3,3},{3,2},{4,6},{5,0},{0,7},{1,1},{2,5},{2,4}},
{{3,1},{3,0},{3,7},{3,6},{2,1},{2,0},{2,7},{2,6}},
{{4,3},{4,2},{4,1},{4,0},{1,7},{1,6},{1,5},{1,4}},
{{4,5},{4,4},{5,3},{5,2},{0,5},{0,4},{1,3},{1,2}},
{{5,7},{5,6},{5,5},{5,4},{0,3},{0,2},{0,1},{0,0}},
{{6,1},{6,2},{6,3},{6,0},{7,7},{7,4},{7,5},{7,6}},
{{6,4},{6,5},{6,6},{6,7},{7,0},{7,1},{7,2},{7,3}}};

const int scanDelay1 = 50, scanDelay2=0;  // Delays for the floors scanning ...

// Cube variables
byte leds_state[8][8]={0}; // Buffer to store all the leds state, working by columns
byte data_buffer[8]={0};  // Buffer to store the bytes in order to send them to the shift registers
byte dataInBuffer[64]={0};  // Buffer used to receive the frames through the serial port
int q = 0;

void setup() {
  // Setting of all used pins to OUTPUT mode
  pinMode(clock, OUTPUT);
  for(int i=0;i<=7;i++) { pinMode(data_pin[i] , OUTPUT); }; 
  for(int i=0;i<=7;i++) { pinMode(floors[i] , OUTPUT); };
  // Initialisation of the serial port
  Serial.begin(115200);
  Serial.setTimeout(0);
}

void loop() {
  // If there is something in the serial buffer, read it and store it in the dataInBuffer array
  while(Serial.available()) {
    dataInBuffer[q] = Serial.read();
    q++;
    if(q >= 64) { 
      writeDataBuffers();
      while (Serial.read() > 0) {}
      q = 0 ;
    }
  }
  // Else, refresh the display
  refreshLeds();
}

// Write the dataInBuffer in the led_state array
void writeDataBuffers() { 
  int a=0;
  for(int i=0;i<=7;i++) {
    for(int j=0;j<=7;j++) {
      leds_state[i][j] = dataInBuffer[a];
      a++;   
    }
  }
}
// Resfresh the display
void refreshLeds() {
  for(int i=0;i<=7;i++) { // Scan all floors
    // For each floor, prepare the bytes to send to the shift registers
    for(int j=0; j<=7; j++) {
      for(int k=0; k<=7; k++) {
        // Write the bit corresponding to the right led in the right buffer
        bitWrite(data_buffer[ reg_wiring[j][k][0] ], reg_wiring[j][k][1], bitRead(leds_state[j][k],i));
      } 
      // Turn on the current floor, wait for some time and then turn it off 
      // This would normally be done with the "shiftOut8();", it was initially a mistake, but it works well like that :3
      
      //delayMicroseconds(scanDelay2);  // An unused delay that can be used to dim the light intensity
    }
      digitalWrite(floors[i], 1);
      delayMicroseconds(scanDelay1);    
      digitalWrite(floors[i], 0);
      // Send the data_buffer values to the shift registers
      shiftOut8();
  }
}
// A modification of the original Arduino shiftOut() function to send data simultaneously on 8 pins
void shiftOut8() {
  uint8_t i;
  for (i = 0; i < 8; i++)  {
    digitalWrite(data_pin[0], !!(data_buffer[0] & (1 << i)));
    digitalWrite(data_pin[1], !!(data_buffer[1] & (1 << i)));
    digitalWrite(data_pin[2], !!(data_buffer[2] & (1 << i)));
    digitalWrite(data_pin[3], !!(data_buffer[3] & (1 << i)));
    digitalWrite(data_pin[4], !!(data_buffer[4] & (1 << i)));
    digitalWrite(data_pin[5], !!(data_buffer[5] & (1 << i)));
    digitalWrite(data_pin[6], !!(data_buffer[6] & (1 << i)));
    digitalWrite(data_pin[7], !!(data_buffer[7] & (1 << i)));
    
    digitalWrite(clock, HIGH);
    digitalWrite(clock, LOW);            
  }
}
// Set a led on or off using its coordinates, only used when doing standalone software, not used with the serial link
void setLed(int x, int y, int z, boolean state) 
{
  bitWrite(leds_state[x][y], z, state);
}
