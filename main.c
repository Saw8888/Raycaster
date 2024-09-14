#include "GGD.h"
#include <math.h>
#include <time.h>
#include <windows.h> // for Sleep function on Windows

#define pixelScale 1
#define GLW 1000
#define GLH 500
#define PI 3.1415
#define TARGET_FPS 120
#define FRAME_TIME_MS (1000 / TARGET_FPS) // 1000ms divided by target FPS
#define mapX 8
#define mapY 8
#define cubeS 64

int pX = 100;
int pY = 100;
float pDeltaX;
float pDeltaY;
float pAngle = 0;

unsigned int map[mapY][mapX] = {{1,1,1,1,1,1,1,1},
                                {1,0,0,0,0,0,0,1},
                                {1,0,0,0,0,0,0,1},
                                {1,0,0,0,0,0,0,1},
                                {1,0,0,0,0,1,0,1},
                                {1,0,0,0,0,0,0,1},
                                {1,0,0,0,0,0,0,1},
                                {1,1,1,1,1,1,1,1}};

double elapsed_ms;

typedef struct {
 float distX;
 float distY;
 float x;
 float y;
 float length;
 float angle;
} ray;

// Function to delay to maintain FPS
void maintainFPS(clock_t start) {
 clock_t end = clock();
 elapsed_ms = ((double)(end - start) / CLOCKS_PER_SEC) * 1000;
 if (elapsed_ms < FRAME_TIME_MS) {
  Sleep(FRAME_TIME_MS - elapsed_ms);
 }
}

void drawMap(){
 for(int y=0;y<mapY;y++){
  for(int x=0;x<mapX;x++){
   if(map[y][x] == 1){
    DS(x*64,y*64,cubeS-4,cubeS-4,(pixelData){255,255,255});
   }
   else{
    DS(x*64,y*64,cubeS-4,cubeS-4,(pixelData){100,100,100});
   }
  }
 }
}

void movePlayer(){
 if (getKey('w')){
   pY+=pDeltaY*5;
   pX+=pDeltaX*5;
  }
 if (getKey('s')){
  pX-=pDeltaX*5;
  pY-=pDeltaY*5;
 }
 if (getKey('a')){
  pAngle+=0.05;
  if(pAngle > 2*PI) pAngle = 0;
  pDeltaX = cos(pAngle);
  pDeltaY = -sin(pAngle);
 }
 if (getKey('d')){
  pAngle-=0.05;
  if(pAngle < 0) pAngle = 2*PI;
  pDeltaX = cos(pAngle);
  pDeltaY = -sin(pAngle);
 }
}

void raycasting() {
 ray ray;
 ray.angle = pAngle;
 ray.x = pX;
 ray.y = pY;
 int pGridX = floor(pX / cubeS); // Grid position in X
 int pGridY = floor(pY / cubeS); // Grid position in Y
 int pDistY = pY - (pGridY * 64); // Distance to the horizontal line above
 int pDistX = ((pGridX + 1) * 64) - pX; // Distance to the vertical line to the right
 //double ntan = -tan(ray.angle);

 // Horizontal line Intersection
 if((ray.angle > PI-0.05 && ray.angle < PI+0.05) || ray.angle == 0){}
 
 else if (ray.angle < PI && ray.angle != 0) { // Looking up
  float dX = pDistY / tan(ray.angle); // Calculates the X distance to the horizontal collision
  ray.x += dX;
  ray.y -= pDistY;
  float stepY = -(cubeS);
  float stepX = -stepY / tan(ray.angle);
  while (map[(int)floor((ray.y / cubeS) - 0.1)][(int)floor(ray.x / cubeS)] != 1) {
   ray.x += stepX;
   ray.y += stepY;
  }
 }
 else if (ray.angle > PI && ray.angle != 0) { // Looking down
  float dX = (cubeS-pDistY) / tan(2*PI-ray.angle); // Adjusted: Calculates the X distance to the horizontal collision
  ray.x += dX;
  ray.y += cubeS-pDistY;
  float stepY = cubeS;
  float stepX = -stepY / tan(ray.angle);
  while (map[(int)floor((ray.y / cubeS) + 0.1)][(int)floor(ray.x / cubeS)] != 1) {
   ray.x += stepX;
   ray.y += stepY;
  }
 }

 //Vertical line intersection
 if (ray.angle < PI/2 || ray.angle > (3*PI)/2 ) { // Looking right
  float dY = pDistX * -tan(ray.angle); // Calculates the X distance to the vertical collision
  ray.y += dY;
  ray.x += pDistX;
  float stepX = cubeS;
  float stepY = stepX * -tan(ray.angle);

  while (map[(int)floor(ray.y / cubeS)][(int)floor((ray.x / cubeS) + 0.1)] != 1) {
   ray.x += stepX;
   ray.y += stepY;
  }
 }
 else if (ray.angle > PI/2 && ray.angle < (3*PI)/2 ) { // Looking left
  float dY = (cubeS-pDistX) * tan(ray.angle); // Adjusted: Calculates the X distance to the vertical collision
  ray.y += dY;
  ray.x -= cubeS-pDistX;
  float stepX = -cubeS;
  float stepY = stepX * tan(ray.angle);
  
  while (map[(int)floor(ray.y / cubeS)][(int)floor((ray.x / cubeS) - 0.1)] != 1) {
   ray.x += stepX;
   ray.y -= stepY;
  }
 }

 
 // Draw the ray
 DL(pX, pY, ray.x, ray.y, 1, (pixelData){255, 0, 0});
 printf("Ray hits wall at grid coordinates: (%d, %d)\n", (int)ray.x / 64, (int)ray.y / 64);
 printf("Ray distances: X: %f, Y: %f\n", ray.distX, ray.distY);
}

int main(int argc, char** argv) {
 matrix specs;
 screen dimensions;
 dimensions.name = "Main";
 dimensions.width = GLW;
 dimensions.height = GLH;

 setupScreen(&dimensions);

 specs.posX = 0;
 specs.posY = 0;
 specs.height = GLH;
 specs.width = GLW;
 specs.pointSize = pixelScale;
 specs.matrix = createMatrix(specs);

 setActiveMatrix(&specs);

 int frameCount = 0;
 time_t startTime = time(NULL);

 while (!windowClosed(&dimensions)) {
  clock_t frameStart = clock();
  clearBG((pixelData){0,0,0});

  movePlayer();

  drawMap();
  DL(pX,pY,pX+pDeltaX*30,pY+pDeltaY*30,5,(pixelData){255,0,255});
  DC(pX,pY,10,(pixelData){255,0,0});
  drawFPS(5,0,0);
  raycasting();

  updateScreen(&dimensions, &specs);
  
  frameCount++;
  time_t currentTime = time(NULL);
  if (currentTime - startTime >= 1) {
   drawFPS(100,1,(float)frameCount);
   frameCount = 0;
   startTime = currentTime;
  }

  maintainFPS(frameStart);
 }

 terminateScreen(&dimensions, &specs);
 return 0;
}
