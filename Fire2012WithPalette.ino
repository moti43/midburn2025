#include <FastLED.h>

#define LED_PIN     5
#define COLOR_ORDER GRB
#define CHIPSET     WS2811
#define NUM_LEDS    250

#define BRIGHTNESS  200
#define FRAMES_PER_SECOND 120

#define COOLING  55   // How quickly does heat dissipate
#define SPARKING 60  // How often sparks ignite recomended 50-200

#define NUM_LED_COLOM 25

#define PIEZO_PIN A0        // Piezo connected to analog pin A0
#define THRESHOLD 50       // Minimum value to consider it a "hit"

CRGBPalette16 currentPalette = HeatColors_p; // RainbowColors_p, OceanColors_p, ForestColors_p, CloudColors_p, HeatColors_p, PartyColors_p.
TBlendType    currentBlending = LINEARBLEND;
CRGB leds_A[250];
CRGB leds_B[50];
int ProgramRun;
int total_time;


// Fire2012 with programmable Color Palette
//
// This code is the same fire simulation as the original "Fire2012",
// but each heat cell's temperature is translated to color through a FastLED
// programmable color palette, instead of through the "HeatColor(...)" function.
//
// Four different static color palettes are provided here, plus one dynamic one.
// 
// The three static ones are: 
//   1. the FastLED built-in HeatColors_p -- this is the default, and it looks
//      pretty much exactly like the original Fire2012.
//
//  To use any of the other palettes below, just "uncomment" the corresponding code.
//
//   2. a gradient from black to red to yellow to white, which is
//      visually similar to the HeatColors_p, and helps to illustrate
//      what the 'heat colors' palette is actually doing,
//   3. a similar gradient, but in blue colors rather than red ones,
//      i.e. from black to blue to aqua to white, which results in
//      an "icy blue" fire effect,
//   4. a simplified three-step gradient, from black to red to white, just to show
//      that these gradients need not have four components; two or
//      three are possible, too, even if they don't look quite as nice for fire.
//
// The dynamic palette shows how you can change the basic 'hue' of the
// color palette every time through the loop, producing "rainbow fire".

CRGBPalette16 gPal;

void setup() {
  delay(3000); // sanity delay
  //FastLED.addLeds<CHIPSET, LED_PIN, COLOR_ORDER>(leds, NUM_LEDS).setCorrection( TypicalLEDStrip );
  FastLED.addLeds<LPD6803, 9, 13, RGB>(leds_A, 250);  
  FastLED.addLeds<LPD6803, 8, 13, RGB>(leds_B, 50);  
  pinMode(PIEZO_PIN, INPUT);


  FastLED.setBrightness( BRIGHTNESS );

  Serial.begin(9600);

  //dancersCounter=0;
  // This first palette is the basic 'black body radiation' colors,
  // which run from black to red to bright yellow to white.
  gPal = HeatColors_p;
  
  // These are other ways to set up the color palette for the 'fire'.
  // First, a gradient from black to red to yellow to white -- similar to HeatColors_p
  //   gPal = CRGBPalette16( CRGB::Black, CRGB::Red, CRGB::Yellow, CRGB::White);
  
  // Second, this palette is like the heat colors, but blue/aqua instead of red/yellow
  gPal = CRGBPalette16( CRGB::Black, CRGB::Blue, CRGB::Aqua,  CRGB::White);
  
  // Third, here's a simpler, three-step gradient, from black to red to white
  //   gPal = CRGBPalette16( CRGB::Black, CRGB::Red, CRGB::White);
  total_time=0;
  
  
  
}



CRGB getColorSparkling(int threshold)
{
 
  CRGB color;
  int trigger;
  static int t=0;
  t++;
  trigger=random(0,99);
  if (trigger>threshold)
    {
      int c=random(0,255);
      color=ColorFromPalette(currentPalette, c, 255, currentBlending);
      c=color[0];
      color[0]=color[1];
      color[1]=c;
    }
    else
    {
      color[0]=0;
      color[1]=0;
      color[2]=0;
    }
    return color;
}


void sparkling()
{
  
  int trigger;
  CRGB color;
  for (int led_ind=0;led_ind<250;led_ind++)
  {
    color=getColorSparkling(80);
    leds_A[led_ind]=color;
  }
  // for (int led_ind=0;led_ind<50;led_ind++)
  // {
  //   leds_B[led_ind]=color;
  //   color=getColorSparkling(80);
  // }
}

void FadeToBlack()
{
  CRGB color;

  for (int led_ind=0;led_ind<250;led_ind++)
  {
    color=leds_A[led_ind];
    if (color[0]>4)
    {
      color[0]=color[0]-5;
    }
    if (color[1]>4)
    {
      color[1]=color[1]-5;
    }
    if (color[2]>4)
    {
      color[2]=color[2]-5;
    }
    leds_A[led_ind]=color;

  } 

}


void allRed(int power)
{
  CRGB color;
  int c1;
  
  for (int led_ind=0;led_ind<250;led_ind++)
  {
    color=ColorFromPalette(currentPalette, power, power, currentBlending);
    c1=color[0];
    color[0]=color[1];
    color[1]=c1;
    leds_A[led_ind]=color;
  }
}

void beating()
{
  static int t2=0;
  t2++;
  if (t2<10)
  {
    allRed(t2*10+30); //Reaching 130 in 10 time steps
  }
  if (t2==10)
  {
    allRed(130);
  }
  if ((t2<20) && (t2>10))
  {
    allRed((t2-10)*10+30);  //Reaching 130 in 10 time steps
  }
  if ((t2<60)&&(t2>20))
  {
    allRed((60-t2)*3+10); //fading out in 40 time steps
  }
  if (t2==60)
  {
    t2=0;
  }
}

void shortBeating(int x)
{
  static int t2=0;
  if (x==1)
  {
    t2 = 0;
  }
  t2++;
  if (t2<10)
  {
    allRed(t2*20+30); //Reaching 130 in 10 time steps
  }
  if (t2==10)
  {
    allRed(30);
    t2=0;
  }
  
}


void Fire2012() {
  
  
  static byte heat[NUM_LED_COLOM][10];
  


  // Step 1: Cool down
  for (int j = 0; j < 10; j++){
    for (int i = 0; i < NUM_LED_COLOM; i++) {
      heat[i][j] = qsub8(heat[i][j], random8(0, ((COOLING * 10) / NUM_LED_COLOM) + 2));
    }
  }
  // Step 2: Heat diffusion upward
  for (int j = 0; j < 10; j++){
    for (int k = NUM_LED_COLOM - 1; k >= 2; k--) {
      heat[k][j] = (heat[k - 1][j] + heat[k - 2][j] + heat[k - 2][j]) / 3;
    }
  }

  // Step 3: Random new sparks near the bottom
  for (int j = 0; j < 10; j++){
    if (random8() < SPARKING) {
      int y = random8(7);
      heat[y][j] = qadd8(heat[y][j], random8(160, 255));
    }
  }

  // Step 4: Map heat to colors
  for (int j = 0; j < 10; j++){
    for (int k = 0; k < NUM_LED_COLOM; k++) {
      int c1;
      CRGB color = HeatColor(heat[k][j]);
      color=ColorFromPalette(currentPalette, heat[k][j], 255, currentBlending);
      c1=color[0];
      color[0]=color[1];
      color[1]=c1;
      if (j%2 > 0) 
        leds_A[k+25*j] = color;
      else
        leds_A[25-k+25*j] = color;
    }
  }
  delay(15);
}

void runningLights(int x)
{
  
  CRGB color;
  int c1;
  
  for (int j=0;j<49;j++)
  {
    if (leds_B[j+1][0]+leds_B[j+1][1]+leds_B[j+1][2]>5){
      leds_B[j+1][0]=0;
      leds_B[j+1][1]=0;
      leds_B[j+1][2]=0;
      color=HeatColor(j*10);
      c1=color[0];
      color[0]=color[1];
      color[1]=c1;
      leds_B[j]=color;
    }
    leds_B[j]=leds_B[j+1];
  }
  if (x==1)
  {
    color=HeatColor(250);
    //c1=color[0];
    //color[0]=color[1];
    //color[1]=c1;
    leds_B[49]=color;
  }
}

void increaseIntensity()
{
  CRGB color;
  for (int j = 0; j < 10; j++){
    color=leds_A[j];
    color[0]=color[0]+30;
    color[1]=color[1]+30;
    color[2]=color[2]+30;
    leds_A[j]=color;
  }
}


void backToFront(int x)
{
  
  CRGB color;
  int c1;
  static int t=0;
  t++;
  if (t>10)
  {
    for (int j=250;j>50;j--)
    {
      if (leds_A[j-50][0]+leds_A[j-50][1]+leds_A[j-50][2]>0)
      {
        color=leds_A[j-50];
        leds_A[j-50][0]=0;
        leds_A[j-50][1]=0;
        leds_A[j-50][2]=0;
        
        leds_A[j]=color;
      }
    }
    t=0;
    for (int j=200;j<250;j++)
    {
      if (leds_A[j][0]>0)
      {
        leds_A[j][0]=leds_A[j][0]/2;
      }
      if (leds_A[j][1]>0)
      {
        leds_A[j][1]=leds_A[j][1]/2;
      }
      if (leds_A[j][2]>0)
      {
        leds_A[j][2]=leds_A[j][2]/2;
      }  
    }
  }
  if (x>0)
  {
    int c=random(0,250);
    color=ColorFromPalette(currentPalette, c, 255, currentBlending);
    c1=color[0];
    color[0]=color[1];
    color[1]=c1;
    c=random(0,25);
    leds_A[c]=color;
    leds_A[49-c]=color;
  }
  
}


void loop()
{
  
  static int program_num=0;
  static int t=0;
  static int timeAlone=0;

  int samples = 10;
  int sum = 0;
  for (int i = 0; i < samples; i++) {
    sum += analogRead(PIEZO_PIN); // Read piezo value (0–1023)
  }
  int piezoValue = sum / samples;
     
    if (piezoValue>THRESHOLD){
      Serial.print("Piezo: ");
      Serial.println(piezoValue);

      timeAlone=0;
      
      runningLights(1);
      if (program_num == 3){
        backToFront(1);
      }
      if (program_num == 2){
        shortBeating(1);
        t=0;
      }
      if (program_num == 1){
        program_num=3;
      }
      if (program_num == 0){
        program_num=1;
      }

    }

  total_time++;
  t++; 
  timeAlone++;

  if (timeAlone>60000/FRAMES_PER_SECOND)
  {
    program_num=0;
  }
  //beating();
  //sparkling();
  if (program_num == 0){
    Fire2012();
    //sparkling();
  }
  if (program_num == 1){
    FadeToBlack();
  }
  if (program_num == 2){
    if (t<11)
    { 
       shortBeating(0);
    }
  }
  if (program_num == 3){
     backToFront(0);
  }
  if (timeAlone<50)
  {
    runningLights(0);
  }
  FastLED.show(); // display this frame
  FastLED.delay(1000 / FRAMES_PER_SECOND);

}





