#include <stdint.h>
#include <TouchScreen.h>
#include <TFT.h>
 
#if defined(__AVR_ATmega1280__) || defined(__AVR_ATmega2560__) // mega
#define YP A2   // must be an analog pin, use "An" notation!
#define XM A1   // must be an analog pin, use "An" notation!
#define YM 54   // can be a digital pin, this is A0
#define XP 57   // can be a digital pin, this is A3 
 
#elif defined(__AVR_ATmega32U4__) // leonardo
#define YP A2   // must be an analog pin, use "An" notation!
#define XM A1   // must be an analog pin, use "An" notation!
#define YM 18   // can be a digital pin, this is A0
#define XP 21   // can be a digital pin, this is A3 
 
#else //168, 328, something else
#define YP A2   // must be an analog pin, use "An" notation!
#define XM A1   // must be an analog pin, use "An" notation!
#define YM 14   // can be a digital pin, this is A0
#define XP 17   // can be a digital pin, this is A3 
 
#endif
 
//Measured ADC values for (0,0) and (210-1,320-1)
//TS_MINX corresponds to ADC value when X = 0
//TS_MINY corresponds to ADC value when Y = 0
//TS_MAXX corresponds to ADC value when X = 240 -1
//TS_MAXY corresponds to ADC value when Y = 320 -1
 
#define TS_MINX 140
#define TS_MAXX 900
#define TS_MINY 120
#define TS_MAXY 940


#define S_PINK    0xF81F
#define S_PURPLE  0x780F
#define S_WHITE   0xFFFF
#define S_BLACK   0x0000
#define S_BLUE    0x001F
#define S_RED     0xF800
#define S_YELLOW  0xFFE0      /* 255, 255,   0 */
#define S_ORANGE  0xFD20  

//X Coordinates of lines L1 & L4 imaginary
#define L1_X 9        
#define L2_X (L1_X + 74)
#define L3_X (L2_X + 74)
#define L4_X (L3_X + 74)

//Y Coordinates of lines L1 & L4 imaginary
#define L1_Y 58
#define L2_Y (L1_Y + 74)
#define L3_Y (L2_Y + 74)
#define L4_Y (L3_Y + 74)
 
int color = WHITE;  //Paint brush color
 
// For better pressure precision, we need to know the resistance
// between X+ and X- Use any multimeter to read it
// The 2.8" TFT Touch shield has 300 ohms across the X plate
 
TouchScreen ts = TouchScreen(XP, YP, XM, YM, 300); //init TouchScreen port pins

int ttt[3][3] = {{-1, -1, -1}, {-1, -1, -1}, {-1, -1, -1}};
char players[2] = {'O', 'X'};
int curr_player = 0;
bool haveWinner = false;

void setup()
{
    Tft.init();  //init TFT library
    pinMode(0,OUTPUT);
    drawInitScreen();
}

void resetGame() {
  //probably can do better than this.
  haveWinner = false;
  for (int i=0; i<3; i++)
    for (int j=0; j<3; j++) {
      ttt[i][j] = -1;
    }

  curr_player = 0;
  Tft.paintScreenBlack();
  drawInitScreen();
}

void drawInitScreen() 
{
    displayPlayer();
    Tft.drawString(" Plays",14,10,2,S_WHITE); 
    Tft.drawHorizontalLine(0,30,240-1, S_BLUE);
    
    Tft.fillRectangle(158,2,80,28,GRAY1);
    Tft.drawString("Reset",160,10,2,S_BLACK); 

    drawGame();

    Tft.drawHorizontalLine(0,290,240-1, S_BLUE);

}

void drawGame() {
      //74px squares
    Tft.drawVerticalLine  (L2_X,  L1_Y,  222, S_WHITE);
    Tft.drawVerticalLine  (L3_X,  L1_Y,  222, S_WHITE);
    Tft.drawHorizontalLine(L1_X,  L2_Y,  222, S_WHITE);
    Tft.drawHorizontalLine(L1_X,  L3_Y,  222, S_WHITE);
}

void displayPlayer() {
    Tft.drawChar(players[!curr_player],10,10,2,S_BLACK);   
    Tft.drawChar(players[curr_player],10,10,2,S_WHITE);   
}

void switchPlayer() {
  curr_player  = !curr_player;
  displayPlayer();
}

void checkGame() {
  //Check rows & cols
  for (int i=0; i<3; i++) {
    if(ttt[i][0] != -1 && ttt[i][0] == ttt[i][1] && ttt[i][1] == ttt[i][2]) {
            displayWinner(ttt[i][0]);
            return;
    }
    if(ttt[0][i] != -1 && ttt[0][i] == ttt[1][i] && ttt[1][i] == ttt[2][i]) {
            displayWinner(ttt[0][i]);
            return;            
    }
  }
  if (ttt[1][1] == -1) return;
  //Check diags
  if(ttt[0][0] == ttt[1][1] && ttt[1][1] == ttt[2][2]) {
          displayWinner(ttt[1][1]);
          return;
  }
  if(ttt[0][2] == ttt[1][1] && ttt[1][1] == ttt[2][0]) {
          displayWinner(ttt[1][1]);
          return;
  }
}

void displayWinner(int plidx) {
  haveWinner = true;
  Tft.fillRectangle(35,145, 155, 30, S_PURPLE);
  Tft.drawChar(players[plidx],40,150,3,S_YELLOW); 
  Tft.drawString(" Wins!",50,150,3,S_YELLOW); 
}

void loop()
{
    // a point object holds x y and z coordinates.
    TSPoint p = ts.getPoint();
 
    //map the ADC value read to into pixel co-ordinates
 
    p.x = map(p.x, TS_MINX, TS_MAXX, 240, 0);
    p.y = map(p.y, TS_MINY, TS_MAXY, 320, 0);
 
    // we have some minimum pressure we consider 'valid'
    // pressure of 0 means no pressing!
 
    if (p.z > ts.pressureThreshhold) {

        //Reset 
         if(p.x >158 && p.y <28) {
            resetGame();
            return;          
         }
        
        //Play Sequence

        //disable if we have a winner.
        if(haveWinner) return;
        
        int plx = 0;
        int ply = 0;
        int cox = 0;
        int coy = 0;

        if(p.x > L1_X && p.x < L4_X && p.y > L1_Y && p.y < L4_Y) {
          if(p.x < L2_X) {
            plx = 0;
            cox = L1_X;
          }
          else if(p.x < L3_X) {
            plx = 1;
            cox = L2_X;
          } 
          else {
            plx = 2;
            cox = L3_X;
          } 

          if(p.y < L2_Y) {
            ply = 0;
            coy = L1_Y;
          }
          else if(p.y < L3_Y) {
            ply = 1;
            coy = L2_Y;
          } 
          else  {
            ply = 2;
            coy = L3_Y;
          } 
          
          if(ttt[plx][ply] == -1) {
            ttt[plx][ply] = curr_player;
            Tft.drawChar(players[curr_player],cox+25,coy+25,3,S_WHITE);
            checkGame();
            switchPlayer();
          }
          else {
            //reportError();
          }
        }
      
    }
}
