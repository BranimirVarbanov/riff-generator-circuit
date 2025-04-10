#include <string.h> //This library is needed for operations with the Tab Generation

//PART I: DECLARATION OF THE PINS AND VARIABLES
//Here are the Pins from the way I connected them into my Arduino
#define pMeter A0
#define progPin 13
#define melPin 12
#define rhPin 11
#define rePin 10 
#define tabPin 9
#define piezo 2
#define reLED 6
#define rhLED 7

//I need these variables for the digitalRead() functions
bool progPinState=0;
bool melPinState=0;
bool rhPinState=0;
bool rePinState=0;
bool tabPinState=0;

//This is the musical scale which will be played
float scale[] = {164.81, 174.61, 196.00, 220.00, 246.94, 261.63, 293.66}; //it's important to be a scale in only one octave
short scaleTab[] = {0,1,3,5,7,8,10}; //I use that array as making the Tab Generation easier

//These are variables for the purpose of my note generation
short count=0; //this variable is only for the purpose of Tremolo-Progression Riff generation; for indexes
short rhCount=0;
short rnd; //for random() functions
short rnd2; 
short rnd3;
short note; //for memorization of which note to play when generated


//There is array for the Tremolo-Progression Generation
short progGen[] = {0,0,0,0}; //that's the random sequence which will the determine the Tremolo-Progression

//These are the arrays which will represent the riffs with the note values
short progRiff[32]; //I will store Scale Position notes in these arrays which will be transformed into Tab
short melRiff[32];

//These are arrays which will organize the tab
short onString[32]; //on which string
short riffTab[32]; //which tab on the screen
short rhythmCalc[6];
short rhythm[32];

//These are the strings which I'll be using for the Tab Generation
String firstString = "D|-"; //they are based on the tuning and the scale
String secondString = "G|-";
String thirdString = "C|-";
String fourthString = "F|-";
String fifthString = "A|-";
String sixthString = "d|-";

//These are variables for the Potenciometer and tempo measuring
short pMeterVal;
short tempo;

//These are variables for measuring if I have pressed the buttons
//Buttons make quick bouncing which makes the Switching function hard to detect
//The point is to make the buttons acting like toggle switches, so I need to count the bouncings
long reN=0; //it's long-type because it may be pressed for longer periods
long rhN=0;
short progN=0; //these are int-type because in the end I reset them to 0
short melN=0;
short tabN=0;

//These are booleans to check if I have pressed the button; they are based on the bouncing
bool reset=0; //This is for the Reset Function
bool progState=0;
bool melState=0;
bool rhState=0;
bool tabState=0;

//These are for the purpose of which Tab to be represented
bool progLast=0;
bool rhLast=0;
bool melLast=0;

//PART II: SETUP
void setup() {
  Serial.begin(9600);
  //Random Generating
  rnd=random(1,6); //between which empty pins
  switch(rnd){ //I switch the empty pins because it gives more random sequences 
    case 1:{ 
      randomSeed(analogRead(A1)); //when the empty pin stays the same it generates same sequences as before
      break;
    }
    case 2:{
      randomSeed(analogRead(A2));
      break;
    }
    case 3:{
      randomSeed(analogRead(A3));
      break;
    }
    case 4:{
      randomSeed(analogRead(A4));
      break;
    }
    case 5:{
      randomSeed(analogRead(A5));
      break;
    }
  }
  //Setting up the pinMode()
  pinMode(progPin,INPUT);
  pinMode(melPin,INPUT);
  pinMode(rhPin,INPUT);
  pinMode(rePin,INPUT);
  pinMode(tabPin,INPUT);
  pinMode(reLED,OUTPUT);
  pinMode(rhLED,OUTPUT);

  //LEDs are turned off by default
  digitalWrite(reLED,0);
  digitalWrite(rhLED,0);
  }

  //PART III: FUNCTIONS
  //This is a function which returns me a note in a form of Scale Position
  short NoteGeneration(short riff[], short index, bool repeatedNotes){ //(which array, which index, is repeating allowed)
    if(repeatedNotes==1) //1=repeating is allowed; 0=repeating isn't allowed
      rnd2=random(-4,5); //includes 0
    else{
      rnd=random(0,2); //random if it will be + or -
      rnd2=random(1,5); //not including 0
      if(rnd==0) rnd2*=(-1);
    }
    riff[index]=riff[index-1]+rnd2;
    if(riff[index]<0){ //Scale Position can't be less than 0, that's why I prevent that possibility from the Generation
      if(repeatedNotes==1){ //this includes 0
        rnd=random(0,5);
        riff[index]=riff[index-1]+rnd; //changing the from - to +
      }
      else{
        rnd=random(1,5); //this doesn't include 0
        riff[index]=riff[index-1]+rnd;
      }
    }
    if(riff[index]>17){ //I don't want Scale Position higher than 17 because it's so high to measure
      if(repeatedNotes==1){ 
        rnd=random(0,5);
        riff[index]=riff[index-1]-rnd; //changing from + to -
      }
      else{
        rnd=random(1,5);
        riff[index]=riff[index-1]-rnd;
      }
    }
    if(index>1&&riff[index-1]==riff[index-2]+4){ //by musical theory after +dominant the next step should be -
        rnd=random(1,5);
        while(rnd==2)
          rnd=random(1,5);
        riff[index]=riff[index-1]-rnd;
    }
    if(index>1&&riff[index-1]+4==riff[index-2]){ //again but for -dominant changing to +
      rnd=random(1,5);
      while(rnd==2)
        rnd=random(1,5);
      riff[index]=riff[index-1]+rnd;
    }
    if(index>2&&riff[index-1]-riff[index-3]==5){ //this is for the situation where it leaps to 6th and then it needs to go down
        if(riff[index-1]-riff[index-2]==2)
          rnd=random(1,4);
        else rnd=random(1,5);
        riff[index]=riff[index-1]-rnd;
    }
    if(index>2&&riff[index-3]-riff[index-1]==5){ //same but in the opposite direction
        if(riff[index-2]-riff[index-1]==2)
          rnd=random(1,4);
        else rnd=random(1,5);
        riff[index]=riff[index-1]+rnd;
    }
    if(index>1&&(riff[index-1]==riff[index-2]+2)){ //melodies are more interesting when from 3rds they don't leap to 7ths and 5ths (arpeggios); (0,2)
      if(repeatedNotes==1){ //0 is included
        if(riff[index-1]>2){ 
          rnd=random(-3,3); //when it's like (3,5,X) - X can be from 2 to 8 (without 7)
          if(rnd==2) riff[index]=riff[index-1]+3; //when it's +2 I turn it to +3
          else riff[index]=riff[index-1]+rnd; //others are correct
        }
        else{ //that "else" is equal to riff[index-1]==2
          rnd=random(-2,3); //I can't go from to 2 to -1, so I reduce the range
          if(rnd==2) riff[index]=riff[index-1]+3; //again turning +2 to +3
          else riff[index]=riff[index-1]+rnd;
        }
      }
      else{ //0 isn't included
        rnd=random(0,2); //random deciding whether + or -
        if(rnd==0){ //0 is +
          rnd2=random(1,3); 
          if(rnd2==2) riff[index]=riff[index-1]+3; //again turning +2 to +3
          else riff[index]=riff[index-1]+1; //the only other option is +1
        }
        else{ //this is for -
          if(riff[index-1]==2){
            rnd2=random(-2,0); //from -2 to -1; can't go lower than 0
            riff[index]=riff[index-1]+rnd2;
          }
          else{ //not the lowest option
            rnd2=random(-3,0); //so -3 is available; -4 is not because it will form arpeggio from the opposite side
            riff[index]=riff[index-1]+rnd2;
          }
        }
      }
    }
    if(index>1&&(riff[index-1]+2==riff[index-2])){ //same stuff with 3rds but from higher to lower; (2,0)
      if(repeatedNotes==1){ //0 is included
        if(riff[index-1]==0){ //0 is the lowest option; it's all possible but +4
          rnd2=random(0,4);
          riff[index]=riff[index-1]+rnd2;
        }
        else{ 
          if(riff[index-1]>0&&riff[index-1]<3){ //when riff[index-1] >0 and <3, it can go -1, but can't go -3; (4,2,1)
            rnd2=random(-1,4);
            riff[index]=riff[index-1]+rnd2;
          }
          else{ //that means riff[index-1] >=3, so it can go -3
            rnd2=random(-3,4);
            while(rnd2==-2) //when it's -2 it forms arpeggio so it must be other value; (6,4,2)
              rnd2=random(-3,4);
            riff[index]=riff[index-1]+rnd2;
          }
        }
      }
      else{ //0 is not included
        if(riff[index-1]==0) rnd=0; //when it's 0, it can't go -, so it's always +
        else rnd=random(0,2); //when it's >0, it decides on random whether + or -
        if(rnd==0){ //it's +
          rnd2=random(1,4);
          riff[index]=riff[index-1]+rnd2;
        }
        else{ //it's -
          if(riff[index-1]>0&&riff[index-1]<3) //when >0 and <3, it can only go -1, as above
            riff[index]=riff[index-1]-1;
          else{ //when >3, it can be -1 or -3
            rnd2=random(0,2); //decides which (-1 or -3)
            if(rnd==0) riff[index]=riff[index-1]-1;
            else riff[index]=riff[index-1]-3;
          }
        }
      }
    }
    if(index>1&&(riff[index-1]==riff[index-2]+3)){ //this is for the case where from 4ths it leaps to 7ths or 8th which is also forbidden by theory; (0,3)
      if(repeatedNotes==1){ //0 is included
        if(riff[index-1]==3){ 
          rnd2=random(-3,3); //when it's 3 it can't go to -1 Scale Position
          riff[index]=riff[index-1]+rnd2;
        }
        else{ //but when it's higher than 3, it can go -4
          rnd2=random(-4,3);
          riff[index]=riff[index-1]+rnd2;
        }
      }
      else{ //0 isn't included
        if(riff[index-1]==3){ //when it's 3 possibilities are limited
          rnd=random(0,2); //decides whether + or -
          rnd2=random(-3,0); //from -3 to -1
          rnd3=random(1,3); //from 1 to 2, because if it's 3 it will form 7th; (0,3,6)
          if(rnd==0) riff[index]=riff[index-1]+rnd2; //it's -
          else riff[index]=riff[index-1]+rnd3; //it's +
        }
        else{ //when higher than 3
          rnd=random(0,2); //same stuff
          rnd2=random(-4,0); //-4 is already an option
          rnd3=random(1,3);
          if(rnd==0) riff[index]=riff[index-1]+rnd2;
          else riff[index]=riff[index-1]+rnd3;
        }
      }
    }
    if(index>1&&(riff[index-1]+3==riff[index-2])){ //same principle for 4ths but from + to -; (3,0)
      if(repeatedNotes==1){ //0 is included
        if(riff[index-1]==0){ //when it's 0 it can't go negative
          rnd2=random(0,5);
          riff[index]=riff[index-1]+rnd2;
        }
        else{ //when it's 1, it can go -1 but can't go -2
          if(riff[index-1]==1){
            rnd2=random(-1,5);
            riff[index]=riff[index-1]+rnd2;
          }
          else{ //when it's >2 it can go from -2 to 5
            rnd2=random(-2,5);
            riff[index]=riff[index-1]+rnd2;
          }
        }
      }
      else{ //0 isn't included
        if(riff[index-1]==0){ //can't be negative, so only +
          rnd2=random(1,5);
          riff[index]=riff[index-1]+rnd2;
        }
        else{
          if(riff[index-1]==1){
            rnd=random(0,2); //whether + or -
            rnd2=random(1,5);
            if(rnd==0) riff[index]=riff[index-1]-1; //the only possible -
            else riff[index]=riff[index-1]+rnd2;
          }
          else{ //riff[index]>1, so it can go -2
            rnd=random(0,2); //whether + or -
            rnd2=random(1,5);
            rnd3=random(0,2); //whether -1 or -2
            if(rnd2==0){ //it's -
              if(rnd3==0) riff[index]=riff[index-1]-1;
              else riff[index]=riff[index-1]-2;
            }
            else riff[index]=riff[index-1]+rnd2; //it's +
          }
        }
      }
    }
    return riff[index];
  }

  //Function to play the music from the piezo
  void PlayMusic(short source[],short rhythm[],short size,float tempo){ //(from which array, what rhythm, what is the array size, in what tempo)
    rhCount=0; //pointer which needs to work in parallel
    for(int i=rhCount;i<size;i++){ 
      tone(piezo,scale[source[i]%(sizeof(scaleTab)/2)]*((source[i]/(sizeof(scaleTab)/2))+1),tempo*rhythm[i]);
      delay((tempo*rhythm[i])*2); //intuitively based on the piezo sounding
      rhCount+=rhythm[i];
      i+=rhythm[i]-1;
    }
  }

  //Function which helps to the Tab Generation
  void FillEmptyTab(short whichString,short n){
    switch(whichString){
    case 1:{
      for(int i=0;i<n;i++){
        secondString+="-";
        thirdString+="-";
        fourthString+="-";
        fifthString+="-";
        sixthString+="-";
      }
      break;
    }
    case 2:{
      for(int i=0;i<n;i++){
        firstString+="-";
        thirdString+="-";
        fourthString+="-";
        fifthString+="-";
        sixthString+="-";
      }
      break;
    }
    case 3:{
      for(int i=0;i<n;i++){
        firstString+="-";
        secondString+="-";
        fourthString+="-";
        fifthString+="-";
        sixthString+="-";
      }
      break;
    }
    case 4:{
      for(int i=0;i<n;i++){
        firstString+="-";
        secondString+="-";
        thirdString+="-";
        fifthString+="-";
        sixthString+="-";
      }
      break;
    }
    case 5:{
      for(int i=0;i<n;i++){
        firstString+="-";
        secondString+="-";
        thirdString+="-";
        fourthString+="-";
        sixthString+="-";
      }
      break;
    }
    case 6:{
      for(int i=0;i<n;i++){
        firstString+="-";
        secondString+="-";
        thirdString+="-";
        fourthString+="-";
        fifthString+="-";
      }
      break;
    }
  } 
  }

  //Function which sets on what octave the tonic will be
  void SetTonicOctave(short riff[]){ 
    rnd=random(1,4);
    if(rnd==1) riff[0]=0;
    else riff[0]=(sizeof(scaleTab)/2);
  }

  //Tab Generation function
  void MakeTab(short source[],short rhythm[],short size,bool sentenceForm){ //(which array, what rhythm, how many notes, is it A B A C)
    for(int i=0;i<32;i++) //filling all string to start from 1 (firstString)
      onString[i]=1;
    for(int i=0;i<size;i++){ //This operation transforms the scale notes on tab and then distribute them into the exact strings
      riffTab[i]=scaleTab[source[i]%(sizeof(scaleTab)/2)]+((source[i]/(sizeof(scaleTab)/2))*12);
      onString[i]=onString[i]+(riffTab[i]/5);
      riffTab[i]=riffTab[i]%5;
      if(onString[i]>1){
        riffTab[i]+=5;
        onString[i]--;
      }
    }
    //Making the 8 notes from the second measure same Tab as in the first measure
    for(int i=0;i<size-1;i++){
      if(sentenceForm==1){
        if(i==(size/2)-1){
          for(int i=0;i<=(size/4)-1;i++){
            riffTab[i+(size/2)]=riffTab[i];
            onString[i+(size/2)]=onString[i];
          }
          i+=4;
        }
      }
      //Below I transform the Tab in a way which will be comfortable for my hands
      //It changes the tab only in progressive way (changing next, next and the next note...)
      if(onString[i]==onString[i]&&((riffTab[i]>0&&riffTab[i+1]-riffTab[i]>3)||(riffTab[i]==0&&riffTab[i+1]-riffTab[i]>5))){
        riffTab[i+1]-=5;
        onString[i+1]++;
      }
      if(onString[i]==onString[i+1]&&onString[i]>1&&riffTab[i]-riffTab[i+1]>=3){
        riffTab[i+1]+=5;
        onString[i+1]--;
      }
      if(onString[i]+1==onString[i+1]&&riffTab[i]-riffTab[i+1]>=3){
        riffTab[i+1]+=5;
        onString[i+1]--;
      }
      if(onString[i]==onString[i+1]+1&&riffTab[i+1]-riffTab[i]>=3){
        riffTab[i+1]-=5;
        onString[i+1]++;
        if(riffTab[i+1]<0){
          riffTab[i+1]+=5;
          onString[i+1]--;
        }
      }
      if(onString[i]+2==onString[i+1]){
        riffTab[i+1]+=5;
        onString[i+1]--;
        if(onString[i]+1==onString[i+1]&&riffTab[i]-riffTab[i+1]>3){
          riffTab[i+1]+=5;
          onString[i+1]--;
        }
      }
      if(onString[i]==onString[i+1]+2){
        riffTab[i+1]-=5;
        onString[i+1]++;
        if(riffTab[i+1]<0){
          riffTab[i+1]+=5;
          onString[i+1]--;
        }
      }
    }
    for(int i=0;i<size;i++){ //starting the Tab Generation
      if(i==size/2){ //adding -||- to the strings when the first measure is over
        firstString+="-||-";
        secondString+="-||-";
        thirdString+="-||-";
        fourthString+="-||-";
        fifthString+="-||-";
        sixthString+="-||-";
      }
      switch(onString[i]){ //transforming the arrays into Tab
        case 1:{
          if(rhythm[i]==1){
            firstString+=((String)riffTab[i]+"-");
            if(riffTab[i]>=10) FillEmptyTab(1,3);
            else FillEmptyTab(1,2);
          }
          else if(rhythm[i]==2){
            firstString+=((String)riffTab[i]+"--");
            if(riffTab[i]>=10) FillEmptyTab(1,4);
            else FillEmptyTab(1,3);
            i++;
          }
          else{
            firstString+=((String)riffTab[i]+"----");
            if(riffTab[i]>=10) FillEmptyTab(1,6);
            else FillEmptyTab(1,5);
            i+=3;
          }
          break;
        }
        case 2:{
          if(rhythm[i]==1){
            secondString+=((String)riffTab[i]+"-");
            if(riffTab[i]>=10) FillEmptyTab(2,3);
            else FillEmptyTab(2,2);
          }
          else if(rhythm[i]==2){
            secondString+=((String)riffTab[i]+"--");
            if(riffTab[i]>=10) FillEmptyTab(2,4);
            else FillEmptyTab(2,3);
            i++;
          }
          else{
            secondString+=((String)riffTab[i]+"----");
            if(riffTab[i]>=10) FillEmptyTab(2,6);
            else FillEmptyTab(2,5);
            i+=3;
          }
          break;
        }
        case 3:{
          if(rhythm[i]==1){
            thirdString+=((String)riffTab[i]+"-");
            if(riffTab[i]>=10) FillEmptyTab(3,3);
            else FillEmptyTab(3,2);
          }
          else if(rhythm[i]==2){
            thirdString+=((String)riffTab[i]+"--");
            if(riffTab[i]>=10) FillEmptyTab(3,4);
            else FillEmptyTab(3,3);
            i++;
          }
          else{
            thirdString+=((String)riffTab[i]+"----");
            if(riffTab[i]>=10) FillEmptyTab(3,6);
            else FillEmptyTab(3,5);
            i+=3;
          }
          break;
        }
        case 4:{
          if(rhythm[i]==1){
            fourthString+=((String)riffTab[i]+"-");
            if(riffTab[i]>=10) FillEmptyTab(4,3);
            else FillEmptyTab(4,2);
          }
          else if(rhythm[i]==2){
            fourthString+=((String)riffTab[i]+"--");
            if(riffTab[i]>=10) FillEmptyTab(4,4);
            else FillEmptyTab(4,3);
            i++;
          }
          else{
            fourthString+=((String)riffTab[i]+"----");
            if(riffTab[i]>=10) FillEmptyTab(4,6);
            else FillEmptyTab(4,5);
            i+=3;
          }
          break;
        }
        case 5:{
          if(rhythm[i]==1){
            fifthString+=(((String)(riffTab[i]+1))+"-");
            if(riffTab[i]+1>=10) FillEmptyTab(5,3);
            else FillEmptyTab(5,2);
          }
          else if(rhythm[i]==2){
            fifthString+=(((String)(riffTab[i]+1))+"--");
            if(riffTab[i]+1>=10) FillEmptyTab(5,4);
            else FillEmptyTab(5,3);
            i++;
          }
          else{
            fifthString+=(((String)(riffTab[i]+1))+"----");
            if(riffTab[i]+1>=10) FillEmptyTab(5,6);
            else FillEmptyTab(5,5);
            i+=3;
          }
          break;
        }
        case 6:{
          if(rhythm[i]==1){
            sixthString+=(((String)(riffTab[i]+1))+"-");
            if(riffTab[i]+1>=10) FillEmptyTab(6,3);
            else FillEmptyTab(6,2);
          }
          else if(rhythm[i]==2){
            sixthString+=(((String)(riffTab[i]+1))+"--");
            if(riffTab[i]+1>=10) FillEmptyTab(6,4);
            else FillEmptyTab(6,3);
            i++;
          }
          else{
            sixthString+=(((String)(riffTab[i]+1))+"----");
            if(riffTab[i]+1>=10) FillEmptyTab(6,6);
            else FillEmptyTab(6,5);
            i+=3;
          }
          break;
        }
        case 7:{
          if(rhythm[i]==1){
            sixthString+=(((String)(riffTab[i]+6))+"-");
            if(riffTab[i]+6>=10) FillEmptyTab(6,3);
            else FillEmptyTab(6,2);
          }
          else if(rhythm[i]==2){
            sixthString+=(((String)(riffTab[i]+6))+"--");
            if(riffTab[i]+6>=10) FillEmptyTab(6,4);
            else FillEmptyTab(6,3);
            i++;
          }
          else{
            sixthString+=(((String)(riffTab[i]+6))+"----");
            if(riffTab[i]+6>=10) FillEmptyTab(6,6);
            else FillEmptyTab(6,5);
            i+=3;
          }
          break;
        }
      }    
    }
    firstString+="|"; //adding these as final symbols
    secondString+="|";
    thirdString+="|";
    fourthString+="|";
    fifthString+="|";
    sixthString+="|";
      Serial.println("\n\n\n\n\n\n\n\n\n\n"); //for better view
      Serial.println(sixthString);
      Serial.println(fifthString);
      Serial.println(fourthString);
      Serial.println(thirdString);
      Serial.println(secondString);
      Serial.println(firstString);
      delay(1200000);
    }

  //PART IV: LOOP
  void loop() {

  //Calculating the Potenciometer values and tempo
  pMeterVal=1025-(analogRead(pMeter)+1); //I make it opposite because my Potenciometer works in the opposite direction
  tempo=60+((pMeterVal/64)*5); //setting the tempo by intuition
  float actualTempo = (200-tempo)*0.6; //opposite because of the Potenciometer; 0.8 coefficient is by intuition

  //Checking if any button is pressed
  progPinState=digitalRead(progPin);
  melPinState=digitalRead(melPin);
  rhPinState=digitalRead(rhPin);
  rePinState=digitalRead(rePin);
  tabPinState=digitalRead(tabPin);

  //The functions below are made to turn buttons into toggle switches
  //This one is for the Reset Function
  if(rePinState==HIGH) //counting the bouncing
    reN++;
  if(reN%4000>=2001) { //the 5000 value is measured by intuition; it's for every button
    digitalWrite(reLED,HIGH);
    reset=1; //setting the Reset 
  }
  else{
    digitalWrite(reLED,LOW);
    reset=0;
  }

  //This is for the Rhythm Generation
  if(rhPinState==HIGH)
    rhN++;
  if(rhN%4000>=2001){
    digitalWrite(rhLED,HIGH);
    rhState=1;
  }
  else{
    digitalWrite(rhLED,LOW);
    rhState=0;
  }

  //This is one is for the Tremolo-Progression
  if(progPinState==HIGH)
    progN++;
  if(progN>2000) //checks if its (>5000) because later I reset the "N"-values to 0
    progState=1;
  else progState=0;

  //Same for the Melody Generation
  if(melPinState==HIGH)
    melN++;
  if(melN>2000){
    melState=1;
  }
  else melState=0;


  //Same for the Tab Generation
  if(tabPinState==HIGH)
    tabN++;
  if(tabN>2000)
    tabState=1;
  else tabState=0;

  //Function which generates the Tremolo-Progression
  if(progState==1){
    progLast=1; //Setting that the last played is "prog", so the Tab knows to show it
    melLast=0; //Setting the others to 0
    rhLast=0;
    if(reset==0){ //if the Reset is OFF - new melody will be generated and played
      count=0; //resetting the "count" in the beggining
      for(int i=0;i<32;i++)
        progRiff[i]=-1;
      for(int i=0;i<32;i++)
        rhythm[i]=2;
      for(int i=0;i<4;i++) //generating from 1 to 3; which type of Tremolo (1+3,2+2,4)
        progGen[i]=random(1,4);
      SetTonicOctave(progRiff);
      for(int i=0;i<4;i++){
        if(progGen[i]==1){ //Generates (1+3)-riffing
          if(count==0){
            count++;
            note=NoteGeneration(progRiff,count,0);
            for(int i=count;i<=count+2;i++){
              progRiff[i]=note;
            }
            count+=3;
          }
          else{
            progRiff[count]=NoteGeneration(progRiff,count,0);
            count++;
            note=NoteGeneration(progRiff,count,0);
            for(int i=count;i<=count+2;i++)
              progRiff[i]=note;
            count+=3;
          }      
        }
        if(progGen[i]==2){ //Generates (2+2)-riffing
          if(count==0){
            progRiff[1]=progRiff[0];
            count+=2;
            note=NoteGeneration(progRiff,count,0);
            for(int i=count;i<=count+1;i++)
              progRiff[i]=note;
            count+=2;
          }
          else{
            note=NoteGeneration(progRiff,count,0);
            for(int i=count;i<=count+1;i++)
              progRiff[i]=note;
            count+=2;
            note=NoteGeneration(progRiff,count,0);
            for(int i=count;i<=count+1;i++)
              progRiff[i]=note;
            count+=2;
          }
        }
        if(progGen[i]==3){ //Generates 4-note riffing
          if(count==0){
            for(int i=1;i<=3;i++)
              progRiff[i]=progRiff[0];
            count+=4;
          }
          else{
            note=NoteGeneration(progRiff,count,0);
            for(int i=count;i<=count+3;i++)
              progRiff[i]=note;
            count+=4;
        }
      }
      }
      for(int i=15;i>=0;i--){ //transforming the 16-note riff into 32-note; doubling the notes
        progRiff[2*i+1]=progRiff[i];
        progRiff[2*i]=progRiff[i];
      }
      if(rhState==1){ //the Rhythm Generation is activated
        for(int i=0;i<4;i++){
          if(progGen[i]==1){
            for(int j=i*8;j<(i+1)*8;j+=2){ //these calculations in the "for"-cycles are based on easier playing for the hands
              rnd=random(1,4);
              if(rnd==1&&j!=i*8&&j!=(i*8)+6){
                rhythm[j]=1;
                rhythm[j+1]=1;
                j+=2;
              }
            }
          }
          else if(progGen[i]==2){
            for(int j=i*8;j<(i+1)*8;j+=2){
              rnd=random(1,4);
              if(rnd==1&&j!=(i*8)+2&&j!=(i*8)+6){
                rhythm[j]=1;
                rhythm[j+1]=1;
                j+=2;
              }
            }
          }
          else{
            for(int j=i*8;j<(i+1)*8;j+=2){
              rnd=random(1,4);
              if(rnd==1&&j!=(i*8)+6){
                rhythm[j]=1;
                rhythm[j+1]=1;
                j+=2;
              }
            }
          }
        }
      }

      PlayMusic(progRiff,rhythm,32,actualTempo); //Playing the music
      PlayMusic(progRiff,rhythm,32,actualTempo); //it's twice with the intentions to feel it as it's in a real song
      progN=0; //resetting the "N"-values; tabN is not reset because the Tab Generation is ending the program
      melN=0; //also the reN is not reset, because it's not correlated to which riff I play, it may stay forever
  }
    else if(reset==1){ //if the Reset is ON - the last generated melody will be played
      PlayMusic(progRiff,rhythm,32,actualTempo);
      PlayMusic(progRiff,rhythm,32,actualTempo);
      progN=0;
      melN=0;
  }
  }

  //Generating the Melody
  if(melState==1){
    progLast=0;
    melLast=1;
    rhLast=0;
    if(reset==0){ 
      for(int i=0;i<32;i++) //resetting the melRiff
        melRiff[i]=-1;
      //There are several kind of riffs, so I will randomize what type of riff to be generated
      rnd=random(1,4);
      rnd=1;
      if(rnd==1){
        if(rhState==1){ //rhythm generation for the first kind is long to explain, so to be understood check the UPDATE ADDED.txt
          for(int i=0;i<=1;i++){
            rhythmCalc[i]=random(2,7);
            if(rhythmCalc[i]<=3) rhythmCalc[i]=2;
            else if(rhythmCalc[i]>3&&rhythmCalc[i]<=5) rhythmCalc[i]=3;
            else rhythmCalc[i]=4;
          }
          if(rhythmCalc[0]==4){
            rhythmCalc[1]=2;
            rhythmCalc[2]=2;
          }
          else if(rhythmCalc[1]==4){
            rhythmCalc[0]=2;
            rhythmCalc[2]=2;
          }
          else rhythmCalc[2]=8-(rhythmCalc[0]+rhythmCalc[1]); 
    
          rhythmCalc[3]=rhythmCalc[0];
          if(rhythmCalc[3]==4){
            rhythmCalc[4]=2;
            rhythmCalc[5]=2;
          }
          else if(rhythmCalc[3]==3) rhythmCalc[4]=random(2,4);
          else{
            rhythmCalc[4]=random(2,7);
            if(rhythmCalc[4]<=3) rhythmCalc[4]=2;
            else if(rhythmCalc[4]>3&&rhythmCalc[4]<=5) rhythmCalc[4]=3;
            else rhythmCalc[4]=4;
          }
          rhythmCalc[5]=8-(rhythmCalc[3]+rhythmCalc[4]);

          rhCount=0;
          for(int i=0;i<6;i++){
            if(rhythmCalc[i]==2){
            rnd=random(1,4);
            if(i==1){
              if(rhythmCalc[0]==3) rnd=2;
            }
            if(rnd==1){
              for(int i=rhCount;i<rhCount+4;i++){
                if(i<rhCount+2) rhythm[i]=1;
                else rhythm[i]=2;
              }
            }
            else if(rnd==2){
              for(int i=rhCount;i<rhCount+4;i++)
                rhythm[i]=2;
            }
            else{
              for(int i=rhCount;i<rhCount+4;i++)
                rhythm[i]=4;
            }
            rhCount+=4;
          }
          else if(rhythmCalc[i]==3){
            rnd=random(1,6);
            if(i==1){
              if(rhythmCalc[0]==2){
              //2,3,5
              rnd2=random(2,5);
              if(rnd2=4) rnd=5;
              else rnd=rnd2;
            }
            else{
              //1,4,5
              rnd2=random(3,6);
              if(rnd2==3) rnd=1;
              else rnd=rnd2;
            }
          }
          if(rnd==1){
            for(int i=rhCount;i<rhCount+6;i++){
              if(i<rhCount+2) rhythm[i]=2;
              else if(i>=rhCount+2&&i<rhCount+4) rhythm[i]=1;
              else rhythm[i]=2;
            }
          }
            else if(rnd==2){
              for(int i=rhCount;i<rhCount+6;i++){
                if(i<rhCount+2) rhythm[i]=1;
                else rhythm[i]=2;
              }
            }
            else if(rnd==3){
              for(int i=rhCount;i<rhCount+6;i++){
                if(i<rhCount+4) rhythm[i]=4;
                else rhythm[i]=2;
              }
            }
            else if(rnd==4){
              for(int i=rhCount;i<rhCount+6;i++){
                if(i<rhCount+2) rhythm[i]=2;
                else rhythm[i]=4;
              }
            }
            else{
              for(int i=rhCount;i<rhCount+6;i++)
                rhythm[i]=2;
            }
            rhCount+=6;
          }
          else{
            for(int i=0;i<=1;i++){
              rnd=random(1,4);
              if(rnd==1){
                for(int i=rhCount;i<rhCount+4;i++){
                  if(i<rhCount+2) rhythm[i]=1;
                  else rhythm[i]=2;
                }
              }
              else if(rnd==2){
                for(int i=rhCount;i<rhCount+4;i++)
                  rhythm[i]=2;
              }
              else{
                for(int i=rhCount;i<rhCount+4;i++)
                  rhythm[i]=4;
              }
              rhCount+=4;
              }
            }
          }
          for(int i=16;i<24;i++)
            rhythm[i]=rhythm[i-16];
    
          if((rhythm[24]==1&&rhythm[26]!=2)||(rhythm[24]==4&&rhythm[26]!=4)){
            rhythm[24]=2;
            rhythm[25]=2;
          }
        }
        else{ //if Rhythm Generation is not generated - the rhythm is straight
          for(int i=0;i<32;i++)
            rhythm[i]=2;
        }
        SetTonicOctave(melRiff);
        rhCount=0; //for parallel counting
        note=0; //for memorizing which to generate
        //Function below adds the generated notes to the rhythm[] array
        for(int i=0;i<16;i++){ //generating only in the first measure
          if(rhythm[i]==1||rhythm[i]==2){ 
            if(i==0) note=melRiff[0]; //if index is 0, the first note should be generated
            else note=NoteGeneration(melRiff,i,0);
            melRiff[rhCount]=note;
            melRiff[rhCount+1]=note;
            rhCount+=2; //parallel indexes
            i++;
          }
          else{
            if(i==0) note=melRiff[0];
            else note=NoteGeneration(melRiff,i,0);
            for(int i=rhCount;i<rhCount+4;i++){
              melRiff[i]=note;
            }
          rhCount+=4;
          i+=3;
          }
        }
        for(int i=16;i<24;i++) //makes first 8 notes of the measures same
          melRiff[i]=melRiff[i-16];
        rhCount=24; //setting the count for the purpose of parallelism
        for(int i=24;i<32;i++){ //generating the next 8 notes 
          if(rhythm[i]==1||rhythm[i]==2){
            if(i==0) note=melRiff[0];
            else note=NoteGeneration(melRiff,i,0);
            melRiff[rhCount]=note;
            melRiff[rhCount+1]=note;
            rhCount+=2;
            i++;
          }
          else{
            if(i==0) note=melRiff[0];
            else note=NoteGeneration(melRiff,i,0);
            for(int i=rhCount;i<rhCount+4;i++){
              melRiff[i]=note;
            }
            rhCount+=4;
            i+=3;
          }
        }
      }
      else if(rnd==2){ //2 is the kind of riffs which include the tonic as pedal note; popular in metal; again Sentence Form
        for(int i=0;i<32;i++)
          rhythm[i]=2;
        for(int i=1;i<16;i++){
          rnd3=random(1,6);
          if(rnd3<=2) melRiff[i]=0;
        }
        melRiff[0]=0; //I want the first notes of the measures 0
        melRiff[8]=0;
        melRiff[7]=-1; //I don't want the last notes of the measures to be 0
        melRiff[15]=-1;
        //Generating 16-note melody
        for(int i=1;i<=3;i++){
          if(melRiff[i]==0) melRiff[i]=0;
          else{
            melRiff[i]=NoteGeneration(melRiff,i,0);
            while(melRiff[i]==0)
              melRiff[i]=NoteGeneration(melRiff,i,0);
          }
        }
        for(int i=9;i<=11;i++)
          melRiff[i]=melRiff[i-8];
        for(int i=4;i<=7;i++){
          if(melRiff[i]==0) melRiff[i]=0;
          else{
            melRiff[i]=NoteGeneration(melRiff,i,0);
            while(melRiff[i]==0)
              melRiff[i]=NoteGeneration(melRiff,i,0);
          }
        }
        for(int i=12;i<=15;i++){
          if(melRiff[i]==0) melRiff[i]=0;
          else{
            melRiff[i]=NoteGeneration(melRiff,i,0);
            while(melRiff[i]==0)
              melRiff[i]=NoteGeneration(melRiff,i,0);
          }
        }
        for(int i=15;i>=0;i--){ //transforming 3the 16-note riff to 32-note; as above
          melRiff[2*i+1]=melRiff[i];
          melRiff[2*i]=melRiff[i];
        }
        if(rhState==1){ //Rhythm Generation is ON
          for(int i=0;i<32;i+=2){
            if(melRiff[i]==0){
              rnd=random(1,4); //33% chance of making it "gallop"
              if(rnd==1&&i!=6&&i!=14&&i!=22&&i!=30){ //when not in the end of the measures
                rhythm[i]=1;
                rhythm[i+1]=1;
                i+=2; //skipping the next 2; to achieve "1122" from "2222"
              }
            }
            else{
              rnd=random(1,5); //25% of making in longer note
              if(rnd==1&&i!=6&&i!=14&&i!=22&&i!=30){ //same as above
                rhythm[i]=4;
                rhythm[i+1]=4;
                rhythm[i+2]=4;
                rhythm[i+3]=4;
                melRiff[i+2]=melRiff[i]; //making the next 2 notes same due to the Rhythm Generation
                melRiff[i+3]=melRiff[i];
                i+=2;
              }
            }
          }
          for(int i=16;i<24;i++) //making Setence Form in the rhythm array for more musicality
              rhythm[i]=rhythm[i-16];
        }
      }
      else{ //the 3rd kind of riffs; same as the second but melody is in the upper octave
        for(int i=0;i<32;i++) //rhythm is straight
          rhythm[i]=2;
        for(int i=1;i<16;i++){ //generating 0's in random principle
          rnd3=random(1,6);
          if(rnd3<=2) melRiff[i]=0;
        }
        melRiff[0]=0; //same as the second type
        melRiff[8]=0;
        melRiff[7]=-1;
        melRiff[15]=-1;
        //Below the stuff is same as the second kind of riffs
        for(int i=1;i<=3;i++){
          if(melRiff[i]==0) melRiff[i]=0;
          else{
            melRiff[i]=NoteGeneration(melRiff,i,0);
            while(melRiff[i]==0)
              melRiff[i]=NoteGeneration(melRiff,i,0);
          }
        }
        for(int i=9;i<=11;i++)
          melRiff[i]=melRiff[i-8];
        for(int i=4;i<=7;i++){
          if(melRiff[i]==0) melRiff[i]=0;
          else{
            melRiff[i]=NoteGeneration(melRiff,i,0);
            while(melRiff[i]==0)
              melRiff[i]=NoteGeneration(melRiff,i,0);
          }
        }
        for(int i=12;i<=15;i++){
          if(melRiff[i]==0) melRiff[i]=0;
          else{
            melRiff[i]=NoteGeneration(melRiff,i,0);
            while(melRiff[i]==0)
              melRiff[i]=NoteGeneration(melRiff,i,0);
          }
        }
        for(int i=1;i<16;i++)
          if(melRiff[i]!=0) melRiff[i]+=(sizeof(scaleTab)/2)-2; //transforming the generated melody in the upper octave
        for(int i=15;i>=0;i--){
          melRiff[2*i+1]=melRiff[i];
          melRiff[2*i]=melRiff[i];
        }
        if(rhState==1){
          for(int i=0;i<32;i+=2){
            if(melRiff[i]==0){
              rnd=random(1,4);
              if(rnd==1&&i!=6&&i!=14&&i!=22&&i!=30){
                rhythm[i]=1;
                rhythm[i+1]=1;
                i+=2;
              }
            }
            else{
              rnd=random(1,5);
              if(rnd==1&&i!=6&&i!=14&&i!=22&&i!=30){
                rhythm[i]=4;
                rhythm[i+1]=4;
                rhythm[i+2]=4;
                rhythm[i+3]=4;
                melRiff[i+2]=melRiff[i];
                melRiff[i+3]=melRiff[i];
                i+=2;
              }
            }
          }
          for(int i=16;i<24;i++)
              rhythm[i]=rhythm[i-16];
        }
      }
      
      PlayMusic(melRiff,rhythm,32,actualTempo); 
      PlayMusic(melRiff,rhythm,32,actualTempo);
      progN=0;
      melN=0;
    }
    else if(reset==1){
      PlayMusic(melRiff,rhythm,32,actualTempo);
      PlayMusic(melRiff,rhythm,32,actualTempo);
      progN=0;
      melN=0;
    }
  }
  //Tab Generating
  if(tabState==1){
    if(progLast==1) MakeTab(progRiff,rhythm,32,0); //Checking which is the last played
    else if(melLast==1) MakeTab(melRiff,rhythm,32,1);
  }
}
