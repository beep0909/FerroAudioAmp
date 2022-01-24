#define avgNum  5
#define bandsNum  3


int analogPin = A1; // read from multiplexer using analog input 0
int strobePin = A2; // strobe is attached to digital pin 2
int resetPin = A0; // reset is attached to digital pin 3
int LEDPin = 3;
int megPin = 2;

int spectrumValue[7]; // to hold a2d values
int spectrumMax[7] = {90, 100, 100, 200, 200, 200, 200};
int spectrumMin[7] = {65, 65, 65, 65, 70, 90, 120};
int mapValue[7];

int BAND = 0;
int BASS  =  0;
int MID   =  0;
int TREM  =  0;
int count = 0;
long int sumBand = 0;
int avgBand = 0;

int flag = 0;
int avgFlag;
int lastRead[3][bandsNum];
int avgRead[avgNum][bandsNum];
int avgMax[bandsNum];

unsigned long lastTime = 0;
unsigned long Delay = 150;
unsigned long LEDLastTime = 0;
unsigned long LEDDelay = 60000;

void setup()
{
  //  Serial.begin(9600);
  pinMode(analogPin, INPUT);
  pinMode(strobePin, OUTPUT);
  pinMode(resetPin, OUTPUT);
  pinMode(megPin, OUTPUT);
  pinMode(LEDPin, OUTPUT);
  analogReference(DEFAULT);

  digitalWrite(resetPin, LOW);
  digitalWrite(strobePin, HIGH);
  digitalWrite(LEDPin, LOW);
}

void loop()
{
  readMSGEQ7();
  writeBands();
  writeEMag(BASS, 0);
  writeEMag(MID, 1);
  //  writeEMag(BASS + MID, 2);
  megOff();
  LEDOff();
}
void readMSGEQ7()
{
  digitalWrite(resetPin, HIGH);
  digitalWrite(resetPin, LOW);
  for (int i = 0; i < 7; i++)
  {
    digitalWrite(strobePin, LOW);
    delayMicroseconds(30); // to allow the output to settle
    spectrumValue[i] = analogRead(analogPin);
    if (spectrumMax[i] < spectrumValue[i])
      spectrumMax[i] = spectrumValue[i];
    spectrumValue[i] = constrain(spectrumValue[i], spectrumMin[i], 1023);
    mapValue[i] = map(spectrumValue[i], spectrumMin[i], spectrumMax[i], 0, 255);
    digitalWrite(strobePin, HIGH);
  }
}

void writeBands()
{
  BAND = 0;
  for (int i = 0; i < 4; i++)
  {
    BAND = mapValue[i] + BAND;
  }
  BASS  = (mapValue[0] + mapValue[1]);
  MID   = (mapValue[2] + mapValue[3]);
  TREM  = (mapValue[4] + mapValue[5]);


  sumBand += BAND;
  count++;
  if (count >= 1000) {
    avgBand = sumBand / 1000;
    sumBand = 0;
    count = 0;
    //    Serial.println(avgBand);
  }

}

void writeEMag(int bands, int index)
{
  int TH = 2 * avgBand;
  int sum = 0 ;
  int avg = 0 ;

  for (int i = 0; i < 3; i++) {
    sum = sum + lastRead[i][index];
  }
  lastRead[flag][index] = bands;

  for (int i = 0; i < avgNum; i++) {
    avg = avg + avgRead[i][index];
  }
  avgRead[avgFlag][index] = bands;

  avg = avg / avgNum;
  if (avg > avgMax[index])
    avgMax[index] = avg;
  //  Serial.print("avg:");
  //  Serial.print(avg);
  //  Serial.print(" - avgMax:");
  //  Serial.print(avgMax[index]);
  //  Serial.print(" - sum:");
  //  Serial.print(sum);
  //  Serial.print(" - bands:");
  //  Serial.println(bands);
  if (avgMax[index] / 3 > TH)
    TH = avgMax[index] / 3;

  if ((bands > TH) && ((bands > avg + 30) || ((bands) > (sum)) || (bands > avgMax[index]))) {
    digitalWrite(megPin, HIGH);
    LEDLastTime = millis();
    digitalWrite(LEDPin, HIGH);
    //    if ((bands > TH) && (bands > avg + 30)) {
    //      Serial.print(" A ");
    //      Serial.print(avg);
    //      Serial.print("-");
    //      Serial.print(bands);
    //    }
    //    if (((bands) > (sum))) {
    //      Serial.print(" B ");
    //      Serial.print(sum);
    //      Serial.print("-");
    //      Serial.print(bands);
    //    }
    //    if ((bands > avgMax)) {
    //      Serial.print(" C ");
    //      Serial.print(avgMax[index]);
    //      Serial.print("-");
    //      Serial.print(bands);
    //    }
    //    Serial.println();
  }
  avgMax[index] -= 1;
}

void megOff() {

  if ((millis() - lastTime) > Delay) {
    digitalWrite(megPin, LOW);
    lastTime = millis();
  }
  flag += 1;
  avgFlag += 1;
  if (flag > 2)
    flag = 0;
  if (avgFlag > avgNum - 1) {
    avgFlag = 0;
  }

}

void LEDOff() {
  if ((millis() - LEDLastTime) > LEDDelay) {
    digitalWrite(LEDPin, LOW);
  }
}
