#include <FastLED.h>
#include <Audio.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <SerialFlash.h>
#define LED_PIN     0
#define NUM_LEDS    54
#define BRIGHTNESS  64
#define LED_TYPE    NEOPIXEL
#define COLOR_ORDER GRB
CRGB leds[NUM_LEDS];
#define UPDATES_PER_SECOND 100
CRGBPalette16 currentPalette;
TBlendType    currentBlending;

extern CRGBPalette16 myRedWhiteBluePalette;
extern const TProgmemPalette16 myRedWhiteBluePalette_p PROGMEM;


// GUItool: begin automatically generated code
AudioInputI2S            i2s1;           //xy=139,91
AudioMixer4              mixer1;         //xy=312,134
AudioOutputI2S           i2s2;           //xy=392,32
AudioAnalyzeFFT1024      fft1024;        //xy=467,147
AudioConnection          patchCord1(i2s1, 0, mixer1, 0);
AudioConnection          patchCord2(i2s1, 0, i2s2, 0);
AudioConnection          patchCord3(i2s1, 1, mixer1, 1);
AudioConnection          patchCord4(i2s1, 1, i2s2, 1);
AudioConnection          patchCord5(mixer1, fft1024);
AudioControlSGTL5000     audioShield;    //xy=366,225
// GUItool: end automatically generated code


//const int myInput = AUDIO_INPUT_LINEIN;
const int myInput = AUDIO_INPUT_MIC;


// The scale sets how much sound is needed in each frequency range to
// show all 8 bars.  Higher numbers are more sensitive.
float scale = 60.0;

// An array to hold the 16 frequency bands
float level[12];

// This array holds the on-screen levels.  When the signal drops quickly,
// these are used to lower the on-screen level 1 bar per update, which
// looks more pleasing to corresponds to human sound perception.
int   shown[16];
int LEDsFFTBin1[] = {1,2,5};
int LEDsFFTBin2[] = {3,4,6,7};
int LEDsFFTBin3[] = {8,9,12,13};
int LEDsFFTBin4[] = {10,11,14,15};
int LEDsFFTBin5[] = {16,17,18,22,23,24};
int LEDsFFTBin6[] = {19,20,21,25,26,27};
int LEDsFFTBin7[] = {28,29,30,34,35,36};
int LEDsFFTBin8[] = {31,32,33,37,38,39};
int LEDsFFTBin9[] = {40,41,44,45};
int LEDsFFTBin10[] = {42,43,46,47};
int LEDsFFTBin11[] = {50,51,53,54};
int LEDsFFTBin12[] = {48,49,52};

float lowTreshold = 0.05;
float highTreshold = 1;
float lightVolumeScale = 1;

void setup() {

    delay( 3000 ); // power-up safety delay
    FastLED.addLeds<NEOPIXEL, LED_PIN>(leds, NUM_LEDS);
    FastLED.setBrightness(  BRIGHTNESS );
    
    currentPalette = RainbowColors_p;
    currentBlending = LINEARBLEND;

  
  // Audio requires memory to work.
  AudioMemory(12);

  // Enable the audio shield and set the output volume.
  audioShield.enable();
  audioShield.inputSelect(myInput);
  audioShield.volume(0.5);

  
  // configure the mixer to equally add left & right
  mixer1.gain(0, 0.5);
  mixer1.gain(1, 0.5);
}


void loop() {

   ChangePalettePeriodically();
   static uint8_t startIndex = 0;
   startIndex = startIndex + 1; /* motion speed */
    
  // FillLEDsFromPaletteColors( startIndex);
    
   
 //  FastLED.delay(1000 / UPDATES_PER_SECOND);
   
  if (fft1024.available()) {
    // read the 512 FFT frequencies into 16 levels
    // music is heard in octaves, but the FFT data
    // is linear, so for the higher octaves, read
    // many FFT bins together.
    level[0] =  fft1024.read(0);
    level[1] =  fft1024.read(1);
    level[2] =  fft1024.read(2, 4);
    level[3] =  fft1024.read(5, 8);
    level[4] =  fft1024.read(9, 14);
    level[5] =  fft1024.read(15, 19);
    level[6] =  fft1024.read(20, 25);
    level[7] =  fft1024.read(26, 40);
    level[8] =  fft1024.read(41, 47);
    level[9] =  fft1024.read(48, 120);
    level[10] = fft1024.read(121, 258);
    level[11] = fft1024.read(258, 511);

    
    // See this conversation to change this to more or less than 16 log-scaled bands?
    // https://forum.pjrc.com/threads/32677-Is-there-a-logarithmic-function-for-FFT-bin-selection-for-any-given-of-bands

    // if you have the volume pot soldered to your audio shield
    // uncomment this line to make it adjust the full scale signal
    //scale = 8.0 + analogRead(A1) / 5.0;


    for (int i=0; i<12; i++) {
      Serial.print(level[i]);

      // TODO: conversion from FFT data to display bars should be
      // exponentially scaled.  But how keep it a simple example?
      int val = level[i] * scale;
      if (val > 8) val = 8;

      if (val >= shown[i]) {
        shown[i] = val;
      } else {
        if (shown[i] > 0) shown[i] = shown[i] - 1;
        val = shown[i];
      }

      //Serial.print(shown[i]);
      Serial.print(" ");

    }
    Serial.print(" cpu:");
    Serial.println(AudioProcessorUsageMax());
  }

//for ( int i = 1; i< 13 ; i++){
  
FillLEDBinAndScale (LEDsFFTBin1, 0, startIndex);
//  FillLEDBinAndScale (LEDsFFTBin1, level[i-1], startIndex);


//}

  
FastLED.show();

}

void FillLEDBinAndScale(int currentBinArray[], float scale, uint8_t colorIndex){

  float normalizedScale = normalizeScale(scale); 
  uint8_t brightness = normalizedScale *255;

  int arraySize = sizeof(currentBinArray);
 Serial.print("arraySize:  ");
 Serial.println(arraySize); 
  for( int i = 0; i < arraySize; i++) {
      leds[currentBinArray[i]] = ColorFromPalette( currentPalette, colorIndex, brightness, currentBlending);
      Serial.print("CurrentArray[i]: ");
      Serial.print(i);
      Serial.println(currentBinArray[i]); 
  }

}

float normalizeScale(float scale){

//  float lowTreshold = 0.05;
//float highTreshold = 1;
//float lightVolumeScale = 1;
  float scaleWithVolume = lightVolumeScale * scale;
  if (scaleWithVolume < lowTreshold){
    return 0;
  }
  else if (scaleWithVolume>highTreshold){
    return highTreshold;
  }
  else return scaleWithVolume;
}

//void FillLEDsFromPaletteColors( uint8_t colorIndex)
//{
//    uint8_t brightness = 255;
//    
//    for( int i = 0; i < NUM_LEDS; i++) {
//        leds[i] = ColorFromPalette( currentPalette, colorIndex, brightness, currentBlending);
//        colorIndex += 3;
//    }
//}

void ChangePalettePeriodically()
{
    uint8_t secondHand = (millis() / 1000) % 60;
    static uint8_t lastSecond = 99;
    currentPalette = RainbowColors_p;
    currentBlending = LINEARBLEND;
    
//    if( lastSecond != secondHand) {
//        lastSecond = secondHand;
//        if( secondHand ==  0)  { currentPalette = RainbowColors_p;         currentBlending = LINEARBLEND; }
//        if( secondHand == 10)  { currentPalette = RainbowStripeColors_p;   currentBlending = NOBLEND;  }
//        if( secondHand == 15)  { currentPalette = RainbowStripeColors_p;   currentBlending = LINEARBLEND; }
//        if( secondHand == 20)  { SetupPurpleAndGreenPalette();             currentBlending = LINEARBLEND; }
//        if( secondHand == 25)  { SetupTotallyRandomPalette();              currentBlending = LINEARBLEND; }
//        if( secondHand == 30)  { SetupBlackAndWhiteStripedPalette();       currentBlending = NOBLEND; }
//        if( secondHand == 35)  { SetupBlackAndWhiteStripedPalette();       currentBlending = LINEARBLEND; }
//        if( secondHand == 40)  { currentPalette = CloudColors_p;           currentBlending = LINEARBLEND; }
//        if( secondHand == 45)  { currentPalette = PartyColors_p;           currentBlending = LINEARBLEND; }
//        if( secondHand == 50)  { currentPalette = myRedWhiteBluePalette_p; currentBlending = NOBLEND;  }
//        if( secondHand == 55)  { currentPalette = myRedWhiteBluePalette_p; currentBlending = LINEARBLEND; }
//    }
}

// This function fills the palette with totally random colors.
void SetupTotallyRandomPalette()
{
    for( int i = 0; i < 16; i++) {
        currentPalette[i] = CHSV( random8(), 255, random8());
    }
}

// This function sets up a palette of black and white stripes,
// using code.  Since the palette is effectively an array of
// sixteen CRGB colors, the various fill_* functions can be used
// to set them up.
void SetupBlackAndWhiteStripedPalette()
{
    // 'black out' all 16 palette entries...
    fill_solid( currentPalette, 16, CRGB::Black);
    // and set every fourth one to white.
    currentPalette[0] = CRGB::White;
    currentPalette[4] = CRGB::White;
    currentPalette[8] = CRGB::White;
    currentPalette[12] = CRGB::White;
    
}

// This function sets up a palette of purple and green stripes.
void SetupPurpleAndGreenPalette()
{
    CRGB purple = CHSV( HUE_PURPLE, 255, 255);
    CRGB green  = CHSV( HUE_GREEN, 255, 255);
    CRGB black  = CRGB::Black;
    
    currentPalette = CRGBPalette16(
                                   green,  green,  black,  black,
                                   purple, purple, black,  black,
                                   green,  green,  black,  black,
                                   purple, purple, black,  black );
}


// This example shows how to set up a static color palette
// which is stored in PROGMEM (flash), which is almost always more
// plentiful than RAM.  A static PROGMEM palette like this
// takes up 64 bytes of flash.
const TProgmemPalette16 myRedWhiteBluePalette_p PROGMEM =
{
    CRGB::Red,
    CRGB::Gray, // 'white' is too bright compared to red and blue
    CRGB::Blue,
    CRGB::Black,
    
    CRGB::Red,
    CRGB::Gray,
    CRGB::Blue,
    CRGB::Black,
    
    CRGB::Red,
    CRGB::Red,
    CRGB::Gray,
    CRGB::Gray,
    CRGB::Blue,
    CRGB::Blue,
    CRGB::Black,
    CRGB::Black
};
