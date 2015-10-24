#define LATCH 10
#define CLK   11
#define DATA  12

//This is the hex value of each number stored in an array by index num
byte digitOne[10]= {0xDE, 0x06, 0xEC, 0xAE, 0x36, 0xBA, 0xFA, 0x0E, 0xFE, 0xBE};
//byte digitOne[10]= {0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80, 0x00, 0x00};


void setup(){
 
  pinMode(LATCH, OUTPUT);
  pinMode(CLK,   OUTPUT);
  pinMode(DATA,  OUTPUT);
 
}

void loop(){
  doDigit();
}

void doDigit(){
  for(int i=0; i<10; i++){
      digitalWrite(LATCH, LOW);
      digitalWrite(CLK, LOW);
      shiftOut(DATA, CLK, MSBFIRST, digitOne[i]); // digitOne
      digitalWrite(LATCH, HIGH);
      delay(700);
  }
}
