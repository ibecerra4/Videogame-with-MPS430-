#include <msp430.h>
#include <libTimer.h>
#include <lcdutils.h>
#include <lcddraw.h>
#include <p2switches.h>
#include "shape.h"
#include <abCircle.h>
#include "buzzer.h"
#include "stdlib.h"
#include "statemachine.h"
#define GREEN_LED BIT6

int sound = 0;//returned sound depending on the limb it collided 
int destroy = 0;//count number of limbs destroyed to check if player won
int movePlayer = 0;//permission to move paddle

AbTri triangle = {abTriGetBounds, abTriCheck, 50};//LEGS
AbRect rect10 = {abRectGetBounds, abRectCheck, {15,2}};//PADDLE
AbRect rect11 = {abRectGetBounds, abRectCheck, {15,15}};//HEAD
AbRect rectB= {abRectGetBounds, abRectCheck, {15,10}};//BODY


AbRectOutline fieldOutline = {/* playing field */
  abRectOutlineGetBounds, abRectOutlineCheck,
  {screenWidth/2 - 7, screenHeight/2 - 7}
};

//Layer of the monster's limbs. Each limb is one point and if reached 4 points then player won

Layer head = {
  (AbShape *)&circle11,
  {(screenWidth/2), (screenHeight/2 - 50)},
  {0,0}, {0,0},
  COLOR_GREEN,
    0
};

Layer body = {
  (AbShape *)&rectB,
  {(screenWidth/2), (screenHeight/2 - 25)},
  {0,0},{0,0},
  COLOR_GREEN,
    &head
};

Layer rightLeg = {
  (AbShape *)&triangle,
  {(screenWidth/2)+15, (screenHeight/2 - 10)},
  {0,0}, {0,0},
  COLOR_GREEN,
    &body
};


Layer leftLeg = {
  (AbShape *)&triangle,
  {(screenWidth/2)-15, (screenHeight/2 - 10)},
  {0,0}, {0,0},
  COLOR_GREEN,
    &rightLeg
};

Layer player = {
  (AbShape *)&rect10,
  {(screenWidth/2), (screenHeight - 15)},
  {0,0}, {0,0},
  COLOR_RED,
    &leftLeg
};

Layer ball = {
  (AbShape *)&circle4,
  {(screenWidth/2), (screenHeight - 22)},
  {0,0}, {0,0},
  COLOR_BLUE,
    &player
};


Layer fieldLayer = {
  (AbShape *) &fieldOutline,
  {screenWidth/2, screenHeight/2},
  {0,0}, {0,0},
  COLOR_BLACK,
    &ball
};


/** Moving Layer
 *  Linked list of layer references
 *  Velocity represents one iteration of change (direction & magnitude)
 */
typedef struct MovLayer_s {
  Layer *layer;
  Vec2 velocity;
  struct MovLayer_s *next;
} MovLayer;



MovLayer ml1 = {&player, {0,0}, 0};
MovLayer ml0 = {&ball, {0,0}, &ml1};


void movLayerDraw(MovLayer *movLayers, Layer *layers){
  int row, col;
  MovLayer *movLayer;

  and_sr(~8);/**< disable interrupts (GIE off) */
  for (movLayer = movLayers; movLayer; movLayer = movLayer->next) { /* for each moving layer */
    Layer *l = movLayer->layer;
    l->posLast = l->pos;
    l->pos = l->posNext;
  }
  or_sr(8);/**< disable interrupts (GIE on) */


  for (movLayer = movLayers; movLayer; movLayer = movLayer->next) { /* for each moving layer */
    Region bounds;
    layerGetBounds(movLayer->layer, &bounds);
    lcd_setArea(bounds.topLeft.axes[0], bounds.topLeft.axes[1],
		bounds.botRight.axes[0], bounds.botRight.axes[1]);
    for (row = bounds.topLeft.axes[1]; row <= bounds.botRight.axes[1]; row++) {
      for (col = bounds.topLeft.axes[0]; col <= bounds.botRight.axes[0]; col++) {
	Vec2 pixelPos = {col, row};
	u_int color = bgColor;
	Layer *probeLayer;
	for (probeLayer = layers; probeLayer;
	     probeLayer = probeLayer->next) { /* probe all layers, in order */
	  if (abShapeCheck(probeLayer->abShape, &probeLayer->pos, &pixelPos)) {
	    color = probeLayer->color;
	    break;
	  } /* if probe check */
	} // for checking all layers at col, row
	lcd_writeColor(color);
      } // for col
    } // for row
  } // for moving layer being updated
}


/** Advances a moving shape within a fence
 *  
 *  \param ml The moving shape to be advanced
 *  \param fence The region which will serve as a boundary for ml
 */
void mlAdvance(MovLayer *ml, Region *fence){
  Vec2 newPos;
  u_char axis; 
  Region shapeBoundary;
  int velocity = 0; 
  u_char col;
  u_char row;
  for (; ml; ml = ml->next) {
    vec2Add(&newPos, &ml->layer->posNext, &ml->velocity);
    abShapeGetBounds(ml->layer->abShape, &newPos, &shapeBoundary);
    for (axis = 0; axis < 2; axis ++) {
      buzzer_set_period(0);

      if ((shapeBoundary.topLeft.axes[axis] < fence->topLeft.axes[axis]) ||
	  (shapeBoundary.botRight.axes[axis] > fence->botRight.axes[axis]) ) {
	velocity = ml->velocity.axes[axis] = -ml->velocity.axes[axis]; //bouncing
	newPos.axes[axis] += (2*velocity);
      }/**< if outside of fence */

      if(destroy == 4){//Display winning screen if you've destroyed all parts of the monster
	ml0.velocity.axes[0]=0; //Stop ball setting velocity to 0
	ml0.velocity.axes[1]=0;
	drawString5x7(screenWidth/2-20,screenHeight/2+12, "GOT 'EM", COLOR_BLACK, COLOR_YELLOW);
	movePlayer = 1; //Stop player from moving upon winning
      }
      
      //Check ball touched bottomm
      if((shapeBoundary.botRight.axes[1]) > (fence->botRight.axes[1])){
	movePlayer = 1;
	ml0.velocity.axes[0]=0; //Stop ball
	ml0.velocity.axes[1]=0;
	drawString5x7(screenWidth/2-20,screenHeight/2+12, "YOU LOST", COLOR_BLACK, COLOR_YELLOW);
	movePlayer = 1; //Stop paddle

      }
      
      //Handle ball and paddle collision
      if(abRectCheck(&rect10,&(player.pos), &(ml->layer->pos))&& axis==1){
	velocity = ml->velocity.axes[axis] = -ml->velocity.axes[axis];
	newPos.axes[axis] += (2*velocity);

     }

      //Handle ball and body
      if(abRectCheck(&rect11,&(body.pos), &(ml->layer->pos))&& axis==1){
	sound = 1;
	velocity = ml->velocity.axes[axis] = -ml->velocity.axes[axis];
	newPos.axes[axis] += (2*velocity);
	rightLeg.next= &head; //Remove from linked list
	body.pos.axes[0]=screenWidth+5; //moves layer off screen
	destroy+=1;//Increase because limb destroyed
	//cover up layer
	col = (screenWidth/2)- 15;
	row = (screenHeight/2 - 35);
	fillRectangle(col, row, 32, 22, COLOR_YELLOW);
      }

      //Handle ball and head
      if(abRectCheck(&rect11, &(head.pos), &(ml->layer->pos))&& axis==1){
        sound = 2;
	velocity = ml->velocity.axes[axis] = -ml->velocity.axes[axis];
	newPos.axes[axis] += (2*velocity);
	body.next = 0;
	head.pos.axes[0]=screenWidth+5;
	destroy+=1;
	col = (screenWidth/2)-10;
	row = (screenHeight/2 - 61);
	fillRectangle(col, row, 24, 24, COLOR_YELLOW);
    }
		      
      //Handle left leg and ball
      if(abTriCheck(&triangle,&(leftLeg.pos), &(ml->layer->pos))&& axis==1){
	sound = 3;
	velocity = ml->velocity.axes[axis] = -ml->velocity.axes[axis];
	newPos.axes[axis] += (2*velocity);
	rightLeg.next = &body;
	leftLeg.pos.axes[0]=screenWidth+5;
	destroy+=1;
	col = (screenWidth/2)- 28;
	row = (screenHeight/2 - 10);
	fillRectangle(col, row, 31, 16, COLOR_YELLOW);

      }
      
      //Handle right leg and ball
      if(abTriCheck(&triangle,&(rightLeg.pos), &(ml->layer->pos))&& axis==1){
	sound = 4;
	velocity = ml->velocity.axes[axis] = -ml->velocity.axes[axis];
	newPos.axes[axis] += (2*velocity);
	leftLeg.next = &rightLeg;
	rightLeg.pos.axes[0]=screenWidth+7;
	destroy+=1;
	col = (screenWidth/2)+ 2;
	row = (screenHeight/2) - 10;
	fillRectangle(col, row, 31, 16, COLOR_YELLOW);
      }
    } /**< for axis */
    ml->layer->posNext = newPos;
  } /**< for ml **/
}

  //Check which button is pressed
void checkPlayer(u_int switches){
  unsigned char S1 = (switches & 1) ? 0 : 1;//First switch
  unsigned char S2 = (switches & 2) ? 0 : 1;//Second switch
  unsigned char S4 = (switches & 8) ? 0 : 1;//Third switch

  static int num = 0;

  static int x = 0;//Change horizontal velocity

  if(S1 && num != 0 && movePlayer != 1){ //Moves left
    x = -5;
  }
  else if(S4 && num != 0 && movePlayer != 1){ //Moves right
    x = 5;
  }
  else{ //Don't move
    x = 0;
  }

  Vec2 newVelocity = {x, 0};
  (&ml1)->velocity = newVelocity;

  if(S2 && num == 0){ //Cover up starting screen
    drawString5x7(13,90, "DESTROY MONSTER", COLOR_YELLOW, COLOR_YELLOW);
    drawString5x7(18,100, "S1 - MOVE LEFT", COLOR_YELLOW, COLOR_YELLOW);
    drawString5x7(18,110, "S2 - RELEASE BALL", COLOR_YELLOW, COLOR_YELLOW);
    drawString5x7(18,120, "S4 - MOVE RIGHT", COLOR_YELLOW, COLOR_YELLOW);

    Vec2 newVelocity1 = {3, -3}; //Moving ball velocity
    (&ml0)->velocity = newVelocity1;
    num++; //Only release once
  }
}


u_int bgColor = COLOR_YELLOW; /**< The background color */
int redrawScreen = 1;        /**< Boolean for whether screen needs to be redrawn */
Region fieldFence;     /**< fence around playing field  */
/** Initializes everything, enables interrupts and green LED, 
 *  and handles the rendering for the screen
 */
void main(){
  P1DIR |= GREEN_LED;        /**< Green led on when CPU on */
  P1OUT |= GREEN_LED;

  configureClocks();
  lcd_init();
  shapeInit();
  buzzer_init();
  p2sw_init(15);//Initialize all switches
  shapeInit();

  layerInit(&fieldLayer);
  layerDraw(&fieldLayer);
  layerGetBounds(&fieldLayer, &fieldFence);
  enableWDTInterrupts();      /**< enable periodic interrupt */
  or_sr(0x8);              /**< GIE (enable interrupts) */

  //Starting message
  drawString5x7(13,90, "DESTROY MONSTER", COLOR_BLACK, COLOR_YELLOW);
  drawString5x7(18,100, "S1| MOVE LEFT", COLOR_BLACK, COLOR_YELLOW);
  drawString5x7(18,110, "S2| RELEASE BALL", COLOR_BLACK, COLOR_YELLOW);
  drawString5x7(18,120, "S4| MOVE RIGHT", COLOR_BLACK, COLOR_YELLOW);
  u_int swi;
  for(;;) {
    swi = p2sw_read();      //Constantly check which switch is down
    while (!redrawScreen) { /**< Pause CPU if screen doesn't need updating */
      P1OUT &= ~GREEN_LED;/**< Green led off witHo CPU */
      or_sr(0x10);    /**< CPU OFF */
    }
    P1OUT |= GREEN_LED;     /**< Green led on when CPU on */
    redrawScreen = 0;
    checkPlayer(swi);       //Constantly check which switch is down to move
    movLayerDraw(&ml0, &ball);//Moving layer
  }
}

/** Watchdog timer interrupt handler. 15 interrupts/sec */
void wdt_c_handler(){
  static short buzzCount = 0;
  buzzCount++;
  int note = playNote(sound); //Get note depending on sound #

  if(note > 0){
    buzzCount = 10;
    buzzer_set_period(note);//Play note
    sound = 0;
  }


  if(buzzCount > 0){
    buzzCount--;
  } else{
    sound = 0;
  }
  static short count = 0;
  P1OUT |= GREEN_LED;      /**< Green LED on when cpu on */
  count ++;
  if (count == 15) {
    mlAdvance(&ml0, &fieldFence);
    redrawScreen = 1;
    count = 0;
  }
  P1OUT &= ~GREEN_LED;  /**< Green LED off when cpu off */
}
