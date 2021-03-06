/** \file lcddemo.c
 *  \brief A simple demo that draws a string and circle
 */

#include <libTimer.h>
#include "lcdutils.h"
#include "lcddraw.h"

/** Initializes everything, clears the screen, draws "hello" and the circle */
int
main()
{
  configureClocks();
  lcd_init();
  u_char width = screenWidth, height = screenHeight;

   clearScreen(COLOR_BLUE);

   /*THIS CREATES A PIXEL
     drawPixel(30, 30, COLOR_BLACK);*/

   /*this makes a VERTICAL line
   for (int i=30; i<80;i++){
   drawPixel(30, i,COLOR_BLACK);}*/

   /*this makes a HORIZONTAL line
   for(int i=30; i<80; i++){
   drawPixel(i, 30, COLOR_BLACK);}*/

   /*this makes a DIAGONAL line
   int j=80;
   for(int i=30;i<80;i++){
     drawPixel(i, j++, COLOR_BLACK);
     }(*/

   
   /*THIS CREATES A TRIAGLE
   for(int i=30; i<80; i++){
     for(int j =30; j<i; j++){
       drawPixel(i, j, COLOR_BLACK);}
       }*/

   /*this creates a two equal side triangle*/
   for(int i=0; i<10; i++){
     for(int j=0; i<j;j++){
       drawPixel(j-1, i+1, COLOR_BLACK);}
   }
  //drawString5x7(20,20, "hello", COLOR_GREEN, COLOR_RED);

  //fillRectangle(30,30, 60, 60, COLOR_ORANGE);
  
}
