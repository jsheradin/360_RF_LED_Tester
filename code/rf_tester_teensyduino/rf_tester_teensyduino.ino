//Teensy 2.0, 3.3V, 8MHz
//Push power button on RF board to cycle colors
//Reference: https://wiki.tkkrlab.nl/tkkrlab.nl/wiki/XBOX_360_RF_Module.html

#define pin_sw 7 //D2, PWRSW_N_R
#define pin_data 6 //D1, ARGON_DATA
#define pin_clk 5 //D0, ARGON_CLK

void rfTx(int data){
  Serial.print("Sending: ");
  Serial.print(data, HEX);
  Serial.print(" = ");
  
  //Bitbang the data with a preceding two 0s
  digitalWrite(pin_data, LOW);
  while(digitalRead(pin_clk) == HIGH);
  digitalWrite(pin_data, LOW);
  while(digitalRead(pin_clk) == LOW);
  while(digitalRead(pin_clk) == HIGH);
  digitalWrite(pin_data, LOW);
  while(digitalRead(pin_clk) == LOW);

  for(int i=8; i--; i>0){
    while(digitalRead(pin_clk) == HIGH);
    Serial.print(bitRead(data, i));
    digitalWrite(pin_data, bitRead(data, i));
    while(digitalRead(pin_clk) == LOW);
  }
  digitalWrite(pin_data, HIGH);
  Serial.println();
  delay(100);
}

void setup(){
  pinMode(pin_sw, INPUT_PULLUP);
  pinMode(pin_data, OUTPUT);
  digitalWrite(pin_data, HIGH);
  pinMode(pin_clk, INPUT_PULLUP);

  Serial.begin(9600);

  //Start up into full green
  delay(500); //Boards seem to be unresponsive immediately after start
  rfTx(0x80); //Clear whatever's going on if hotplugged
  rfTx(0x84); //Init LED controller
  rfTx(0xAF); //All green
}

void loop(){
  static int stage = 1;

  //Requires release before registering another press
  static bool pressed = 0;
  if(digitalRead(pin_sw) == HIGH){
    pressed = 0;
  }
  
  if(digitalRead(pin_sw) == LOW && pressed == 0){
    //Debounce
    delay(100);
    if(digitalRead(pin_sw) == HIGH){
      return;
    }
    pressed = 1;
    
    //Serial.println("Button pressed");
    Serial.print("LED state: ");
    Serial.println(stage);

    switch(stage){
      case 0:
        rfTx(0x80); //Clear whatever's going on if hotplugged
        rfTx(0x84); //Init LED controller
        rfTx(0xAF); //All green
        break;
      case 1:
        rfTx(0x80);
        rfTx(0x84);
        rfTx(0xBF); //All red
        break;
      case 2:
        rfTx(0x80);
        rfTx(0x84);
        rfTx(0xEF); //All amber
        break;
    }

    //Toggle through modes
    stage++;
    stage %= 3;
  }

  //Manually control board if you send bytes over serial
  if(Serial.available() > 0){
    rfTx(Serial.read());
  }
}
