// Rotary Encoder Inputs
#define CLK0 1
#define DT0 0
#define CLK1 4
#define DT1 3
#define CLK2 7
#define DT2 6
#define CLK3 10
#define DT3 9

int counter = 0;
int currentStateCLK;
int lastStateCLK;
unsigned long lastButtonPress = 0;

int CLKA[4] = {CLK0,CLK1,CLK2,CLK3};
int DTA[4] =  {DT0,DT1,DT2,DT3};

int lastCLKstate[4] = {0,0,0,0};
int state[4] = {0,0,0,0};

int lastState[4] = {0,0,0,0};

int valState[4] = {0,0,0,0};

int jumpVal = 5;

int lastBtnState[3] = {0,0,0};

bool testNote = false;
bool lastTestNote = true;

int phoMin = 0;
int phoMax = 1023;

void setup() {
  
  // Set encoder pins as inputs
  pinMode(CLK0,INPUT);
  pinMode(DT0,INPUT);
  pinMode(CLK1,INPUT);
  pinMode(DT1,INPUT);
  pinMode(CLK2,INPUT);
  pinMode(DT2,INPUT);
  pinMode(CLK3,INPUT);
  pinMode(DT3,INPUT);

  pinMode(13, INPUT);
  pinMode(14, INPUT);
  pinMode(15, INPUT);

  // Setup Serial Monitor
  Serial.begin(9600);

  // Read the initial state of CLK
  //lastStateCLK = digitalRead(CLK);
}

void loop() {
  checkKnob(0);
  checkKnob(1);
  checkKnob(2);
  checkKnob(3);

  bool comparison = compareState();
  bool isEmpty = emptyState();
  if(comparison && !isEmpty){
    for(int i = 0; i < 4; i++)
    {
      Serial.print(String(valState[i]) + ", ");
      sendChange(i);
    }
    Serial.println("");
  }
  copyState();
  
  btn1Action();
  btn2Action();
  btn3Action();

  phoAction();

  // Put in a slight delay to help debounce the reading
  delay(1);
}

void setValState(int num, int val){
  val = val * jumpVal;
  if(valState[num] + val > 127){
    valState[num] = 127;
  }
  else if(valState[num] + val < 0){
    valState[num] = 0;
  }
  else{
    valState[num] += val; 
  }
}

void sendChange(int num){
  int cc = num + 1;
  if (state[num] != lastState[num]){
    usbMIDI.sendControlChange(cc, valState[num], 1);
  }
}

void checkKnob(int num){
  
  int currentDir =0;

  int lastStateCLK = lastCLKstate[num];

  int CLK = CLKA[num];
  int DT = DTA[num];

  int currentStateCLK = digitalRead(CLK);

  // If last and current state of CLK are different, then pulse occurred
  // React to only 1 state change to avoid double count
  if (currentStateCLK != lastStateCLK  && currentStateCLK == 1){

    // If the DT state is different than the CLK state then
    // the encoder is rotating CCW so decrement
    if (digitalRead(DT) != currentStateCLK) {
      //counter --;
      currentDir =1;
    } else {
      // Encoder is rotating CW so increment
      //counter ++;
      currentDir =-1;
    }
  }
  else{
    currentDir = 0;
  }
  // Return direction and current state
  state[num] = currentDir;
  lastCLKstate[num] = currentStateCLK;
  setValState(num, currentDir);
}

bool compareState(){
  for(int i=0;i<4;i++){
    if(lastState[i] != state[i]){
      return true;
    }
  }
  return false;
}

bool emptyState(){
  for(int i=0;i<4;i++){
    if(state[i] != 0){
      return false;
    }
  }
  return true;
}

void copyState(){
  for(int i=0;i<4;i++){
    lastState[i] = state[i];
  }
}

int analogToMidi(int aRead, int minV = 0, int maxV = 1023){
  int reading = analogRead(aRead);
  if(reading < minV){
    reading = minV;
  }
  if(reading > maxV){
    reading = maxV;
  }
  reading = reading - minV;
  int range = maxV - minV;
  float factor = 127.00 / float(range);
  //Serial.println(factor);
  //Serial.println(reading * factor);
  return reading *  factor;
}


void btn1Action(){
  int btn = digitalRead(13);
  if(btn==1 && lastBtnState[0] != btn){
    testNote = !testNote;
    if(testNote == true){
      usbMIDI.sendNoteOn(60, 127, 1);
    }else{
      usbMIDI.sendNoteOff(60, 0, 1);
    }
    delay(5);//debounce hack
  }
  lastBtnState[0] = btn;
}

void btn2Action(){
  int btn = digitalRead(14);
  
  if(btn==1 && lastBtnState[1] != btn){
    phoMin = analogRead(16);
    Serial.print("Set Pho Min: ");
    Serial.println( phoMin);
    delay(5);//debounce hack
  }
  lastBtnState[1] = btn;
}
void btn3Action(){
  int btn = digitalRead(15);
  if(btn==1 && lastBtnState[2] != btn){
    phoMax = analogRead(16);
    Serial.print("Set Pho Max: ");
    Serial.println( phoMax);
    Serial.print("Range: ");
    Serial.println(phoMax - phoMin);
    Serial.print("Factor: ");
    Serial.println(127.00 / (float(phoMax) - float(phoMin)));
    int reading = analogToMidi(16, phoMin, phoMax);
    Serial.print("CC Conversion Reading: ");
    Serial.println(reading);
    delay(5);//debounce hack
  }
  lastBtnState[2] = btn;
}

void phoAction(){
  int mPho = analogToMidi(16, phoMin, phoMax);
  usbMIDI.sendControlChange(5, mPho, 1);
}
