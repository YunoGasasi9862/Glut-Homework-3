/*********
   CTIS164 - Template Source Program
----------
STUDENT : MUHAMMAD BILAL
SECTION : 04
HOMEWORK: 03
----------
PROBLEMS: I'm only able to use the vertex function if the center is origin, otherwise, the program creates problem. Like it's unable to give the enemy transformation if the center is not origin
----------
ADDITIONAL FEATURES:
//background sinosoidal effect of two circles (one sin, and the other cos)
//random color for decoration
//increase/decrease speed of the enemy
//Red light reflection on the boundaries

//ADDITIONAL NOTES:
//kindly run the progrma on x86 version, and change the gL to GL (for you).
*********/

#ifdef _MSC_VER
#define _CRT_SECURE_NO_WARNINGS
#endif
#include <gL/glut.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <stdarg.h>
#include "vec.h"
#define WINDOW_WIDTH  1400
#define WINDOW_HEIGHT 800
#define TIMER_PERIOD  16 // Period for the timer.
#define TIMER_ON         1 // 0:disable timer, 1:enable timer
#define D2R 0.0174532
#define SIZE 20

#include <time.h>
//structures

typedef struct
{
    float r, g, b;
}RGB_t;  //color strcuture

typedef struct
{
    vec_t MonsterCenter;
    vec_t velocity; //for velocity
    vec_t pos; //for changing the pos
    RGB_t color;
    bool visible;
    double angle;

}Enemy_t;
vec_t temp;

typedef struct {
    vec_t pos;
    vec_t N;
} pointsonBoundary;  //structure for reflection

typedef struct
{
    vec_t velocity; //for velocity
    vec_t pos;
    vec_t newpos;
    double bulletradius;
}bullet;
bullet fire;
//Variable declarations
double boundaryX, boundaryY, Width, Height;
vec_t Character;
vec_t tempVel;
Enemy_t ene;
RGB_t gra;
RGB_t title;
double radius = 30;
double yfactor = 0;
double factor = 0;
double lastangle;
bool visible = false;
const char* INSIDE = "Inside";
const char* OUTSIDE = "Outside";
bool pause = false;
int ENEMYPSEED = 5;
bool bulletappear = false;
float A = 100, //amplitude
fq = 1,  //frequency
C = 0,   //horizontal phase shift
B = 0;   //vertical phase shift
float angle = boundaryX;

/* Global Variables for Template File */
bool up = false, down = false, right = false, left = false;
int  winWidth, winHeight; // current Window width and height
bool initialmode=false;
//
// to draw circle, center at (x,y)
// radius r
//

RGB_t mulColor(float k, RGB_t c) {
    RGB_t tmp = { k * c.r, k * c.g, k * c.b };  //multiplies the color
    return tmp;
}

RGB_t addColor(RGB_t c1, RGB_t c2) {  //add the two colors
    RGB_t tmp = { c1.r + c2.r, c1.g + c2.g, c1.b + c2.b };
    return tmp;
}
double distanceImpact(double d) {  //calculates the distance
    return (-1.0 / 400) * d + 1.0;  //assuming that after the distance of 400, the color doesnt appear
}
RGB_t calculateColor(Enemy_t ene, pointsonBoundary p)  //calculates color on each boundary
{
    vec_t L = subV(ene.pos, p.pos);
    vec_t unitL = unitV(L);
    float factor = dotP(unitL, p.N) * distanceImpact(magV(L));  //distance impact checks how far the object is from the wall
    return mulColor(factor, ene.color);  //enemy color is red, and it depends on the factor!
}
void circle(int x, int y, int r)
{
#define PI 3.1415
    float angle;
    glBegin(GL_POLYGON);
    for (int i = 0; i < 100; i++)
    {
        angle = 2 * PI * i / 100;
        glVertex2f(x + r * cos(angle), y + r * sin(angle));
    }
    glEnd();
}

void circle_wire(int x, int y, int r)
{
#define PI 3.1415
    float angle;

    glBegin(GL_LINE_LOOP);
    for (int i = 0; i < 100; i++)
    {
        angle = 2 * PI * i / 100;
        glVertex2f(x + r * cos(angle), y + r * sin(angle));
    }
    glEnd();
}

void print(int x, int y, const char* string, void* font)
{
    int len, i;

    glRasterPos2f(x, y);
    len = (int)strlen(string);
    for (i = 0; i < len; i++)
    {
        glutBitmapCharacter(font, string[i]);
    }
}

// display text with variables.
// vprint(-winWidth / 2 + 10, winHeight / 2 - 20, GLUT_BITMAP_8_BY_13, "ERROR: %d", numClicks);
void vprint(int x, int y, void* font, const char* string, ...)
{
    va_list ap;
    va_start(ap, string);
    char str[1024];
    vsprintf_s(str, string, ap);
    va_end(ap);

    int len, i;
    glRasterPos2f(x, y);
    len = (int)strlen(str);
    for (i = 0; i < len; i++)
    {
        glutBitmapCharacter(font, str[i]);
    }
}

// vprint2(-50, 0, 0.35, "00:%02d", timeCounter);
void vprint2(int x, int y, float size, const char* string, ...) {
    va_list ap;
    va_start(ap, string);
    char str[1024];
    vsprintf_s(str, string, ap);
    va_end(ap);
    glPushMatrix();
    glTranslatef(x, y, 0);
    glScalef(size, size, 1);

    int len, i;
    len = (int)strlen(str);
    for (i = 0; i < len; i++)
    {
        glutStrokeCharacter(GLUT_STROKE_ROMAN, str[i]);
    }
    glPopMatrix();
}


//for transformations
void Vertex(vec_t Pos, vec_t tr, double angle) {
    double xp = (Pos.x * cos(angle) - Pos.y * sin(angle)) + tr.x;
    double yp = (Pos.x * sin(angle) + Pos.y * cos(angle)) + tr.y;
    glVertex2d(xp, yp);
}


void Boundary(float x, float y, float width, float height, float r, float g, float b)  //screen boundary and gradient
{

    glBegin(GL_QUADS);
    glColor3f(r, g, b);
    glVertex2f(x, y);
    glVertex2f(x + width, y);
    glVertex2f(x + width, y - height);
    glVertex2f(x, y - height);
    glEnd();

}
//
// To display onto window using OpenGL commands
//
void displayState()  //displays the state of the game, and all the strings
{
    glColor3f(1, 1, 1);
       vprint(boundaryX, boundaryY - Height - 50, GLUT_BITMAP_HELVETICA_18, "%s", "Mouse is ");
    if (visible)
    {
        glColor3f(title.g, title.r, title.b);

        vprint(boundaryX+80, boundaryY - Height - 50, GLUT_BITMAP_HELVETICA_18, "%s", INSIDE);

    }
    else
    {
        glColor3f(title.g, title.r, title.b);

        vprint(boundaryX+80, boundaryY - Height - 50, GLUT_BITMAP_HELVETICA_18, "%s", OUTSIDE);

    }

    glColor3f(title.g, title.b, title.b);
    if (!pause)
    {
        vprint(boundaryX + 400, boundaryY - Height - 50, GLUT_BITMAP_HELVETICA_18, "Press Spacebar to Pause");


    }
    else
    {
        vprint(boundaryX + 400, boundaryY - Height - 50, GLUT_BITMAP_HELVETICA_18, "Press Spacebar to Re-animate");

    }

    glColor3f(1, 1, 1);
    vprint(boundaryX + 900, boundaryY - Height - 50, GLUT_BITMAP_HELVETICA_18, "Angle:" );
    glColor3f(title.r, title.g, title.r);
    vprint(boundaryX + 960, boundaryY - Height - 50, GLUT_BITMAP_HELVETICA_18, "%0.0f", ene.angle);


}
void ChangeColor(RGB_t *title)  //random color
{
    title->r = rand() % 255;
    title->g = rand() % 150;
    title->b = rand() % 200;
    title->r = title->r / 255.0f;
    title->g = title->g / 255.0f;
    title->b = title->b / 255.0f;

}
float fsin(float x) {  //give sinosuidal movement to the background
    return A * sin((fq * x + C) * D2R) + B;
}
float fcos(float x) {  //give sinosuidal movement to the background
    return A * cos((fq * x + C) * D2R) + B;
}
void FollowPlayer()  //follows player function
{

    ene.angle = atan2(Character.y, Character.x) / D2R ;

    if (ene.angle < 0)  //angle
    {
        ene.angle += 360;
    }
    if (visible == true)
    {
        ene.velocity = mulV(ENEMYPSEED, unitV(subV(Character, ene.pos))); //finding the unit vector of speed
        ene.pos = addV(ene.pos, ene.velocity);//adding the ene.velocity to ene.pos
        temp = ene.pos;
        tempVel = ene.velocity;
        lastangle = ene.angle;


        //bullets
       

    }
    else
    {
        //check for bounce off tomorrow
       
        if (ene.pos.x + radius >= boundaryX + Width  || ene.pos.x - radius <= boundaryX ) {  //for reflection
            ene.velocity.x *= -1;
        }

        if (ene.pos.y + radius >= boundaryY  || ene.pos.y - radius <= boundaryY - Height) {
            ene.velocity.y *= -1;  //for bouncing off the ball
        }

        ene.angle = lastangle;  //last angle
        ene.pos = addV(ene.pos, ene.velocity);
    }
  
  //add light intensity
   
   
 
}
void drawMonster()  //draw the monster
{
    double angle = (ene.angle) * D2R;
   

    glColor3f(1, 0, 0);
    circle(ene.MonsterCenter.x + ene.pos.x ,  ene.MonsterCenter.y+ ene.pos.y , radius);  //for rotating the circle
    glBegin(GL_QUADS);
    Vertex({ ene.MonsterCenter.x - radius , ene.MonsterCenter.y }, ene.pos, angle);
    Vertex({ ene.MonsterCenter.x + radius , ene.MonsterCenter.y }, ene.pos, angle);
    Vertex({ ene.MonsterCenter.x + radius , ene.MonsterCenter.y - radius }, ene.pos, angle);
    Vertex({ ene.MonsterCenter.x - radius , ene.MonsterCenter.y - radius }, ene.pos, angle);
    glEnd();
    glColor3f(gra.r, gra.g, gra.b);

    //triangle 1
    glBegin(GL_TRIANGLES);
    Vertex({ ene.MonsterCenter.x + radius , ene.MonsterCenter.y - radius }, ene.pos,angle);
    factor = 10;
    yfactor = 20;
    Vertex({ ene.MonsterCenter.x + radius - factor , ene.MonsterCenter.y - radius + yfactor }, ene.pos, angle);
    factor = 20;
    yfactor = 0;
    Vertex({ ene.MonsterCenter.x + radius - factor , ene.MonsterCenter.y - radius + yfactor }, ene.pos,angle);

    glEnd();
    //triangle 2
    glBegin(GL_TRIANGLES);
    Vertex({ ene.MonsterCenter.x + radius - factor , ene.MonsterCenter.y - radius + yfactor }, ene.pos, angle);
    yfactor = 20;
    factor = 30;
    Vertex({ ene.MonsterCenter.x + radius - factor , ene.MonsterCenter.y - radius + yfactor }, ene.pos, angle);
    yfactor = 0;
    factor = 40;
    Vertex({ ene.MonsterCenter.x + radius - factor , ene.MonsterCenter.y - radius + yfactor }, ene.pos,angle);
    glEnd();

    //triangle 3
    glBegin(GL_TRIANGLES);
    Vertex({ ene.MonsterCenter.x + radius - factor , ene.MonsterCenter.y - radius + yfactor }, ene.pos,angle);
    yfactor = 20;
    factor = 50;
    Vertex({ ene.MonsterCenter.x + radius - factor , ene.MonsterCenter.y - radius + yfactor }, ene.pos, angle);
    yfactor = 0;
    factor = 60;
    Vertex({ ene.MonsterCenter.x + radius - factor , ene.MonsterCenter.y - radius + yfactor }, ene.pos,angle);
    glEnd();

    glColor3f(1, 1, 1);
    circle(ene.MonsterCenter.x- 14 + ene.pos.x,  ene.MonsterCenter.y + ene.pos.y + 6, 6);  //only ene.pos is required because the vectors are getting added up and it places the circle there
    circle(ene.MonsterCenter.x+ 14 + ene.pos.x, ene.MonsterCenter.y+ 6+ ene.pos.y, 6);

    glColor3f(0, 0, 0);
    circle_wire(ene.MonsterCenter.x - 14 + ene.pos.x, ene.MonsterCenter.y + 6 + ene.pos.y, 6);
    circle_wire (ene.MonsterCenter.x+14 + ene.pos.x, ene.MonsterCenter.y + 6 + ene.pos.y, 6);

    circle(ene.MonsterCenter.x - 11 + ene.pos.x, ene.MonsterCenter.y + 6 + ene.pos.y, 3);  //rotating the circles
    circle(ene.MonsterCenter.x + 17 + ene.pos.x, ene.MonsterCenter.y + 6 + ene.pos.y, 3);
   
  
}
void colorReflection()
{
    for (int x = boundaryX; x < boundaryX + Width; x++)
    {
        pointsonBoundary p = { {x, -320}, {0,1} }; //each point

        RGB_t color = { 0, 0, 0 }; //assuming that the initial color is black
        color = addColor(color, calculateColor(ene, p));  //only one color

        glBegin(GL_LINES);
        //from the surface
        glColor3f(color.r, color.g, color.b);  //surface has black color
        glVertex2f(x, -320);
        //to the bottom
        glColor3f(0, 0, 0);  //bottom has black color!
        glVertex2f(x, -340);
        glEnd();
    }
    for (int x = boundaryX; x < boundaryX + Width; x++)
    {
        pointsonBoundary p = { {x, boundaryY}, {0,-1} }; //each point  (this points downwards)

        RGB_t color = { 0, 0, 0 }; //assuming that the initial color is black
        color = addColor(color, calculateColor(ene, p));  //only one color

        glBegin(GL_LINES);
        //from the surface
        glColor3f(color.r, color.g, color.b);  //surface has black color
        glVertex2f(x, boundaryY);
        //to the bottom
        glColor3f(0, 0, 0);  //bottom has black color!
        glVertex2f(x, boundaryY + 20);
        glEnd();
    }

    for (int y = boundaryY; y > boundaryY-Height; y--)
    {
        pointsonBoundary p = { {boundaryX, y}, {1,0} }; //each point  (this points downwards)

        RGB_t color = { 0, 0, 0 }; //assuming that the initial color is black
        color = addColor(color, calculateColor(ene, p));  //only one color

        glBegin(GL_LINES);
        //from the surface
        glColor3f(color.r, color.g, color.b);  //surface has black color
        glVertex2f(boundaryX, y);
        //to the bottom
        glColor3f(0, 0, 0);  //bottom has black color!
        glVertex2f(boundaryX-20, y);
        glEnd();
    }

    for (int y = boundaryY; y > boundaryY - Height; y--)
    {
        pointsonBoundary p = { {boundaryX + Width, y}, {-1,0} }; //each point  (this points downwards)

        RGB_t color = { 0, 0, 0 }; //assuming that the initial color is black
        color = addColor(color, calculateColor(ene, p));  //only one color

        glBegin(GL_LINES);
        //from the surface
        glColor3f(color.r, color.g, color.b);  //surface has black color
        glVertex2f(boundaryX + Width, y);
        //to the bottom
        glColor3f(0, 0, 0);  //bottom has black color!
        glVertex2f(boundaryX + Width+ 20, y);
        glEnd();
    }



}
void display() {
    //
    // clear window to black
    //
    glClearColor(0, 0, 0, 0);
    glClear(GL_COLOR_BUFFER_BIT);
    
    glColor3f(title.r,title.g, title.b);
    vprint(-150, boundaryY + 40, GLUT_BITMAP_TIMES_ROMAN_24, "PACMAN, EAT THE PLAYER!");
    vprint(-680, boundaryY + 50, GLUT_BITMAP_HELVETICA_18, "<F1> : increase the speed");
    vprint(-680, boundaryY + 30, GLUT_BITMAP_HELVETICA_18, "<F2> : decrease the speed");

    Boundary(boundaryX, boundaryY, Width, Height, gra.r, gra.g, gra.b);  //it's the boundary 
 
   
    glColor4f(0, 0, 1, 0.5);
    if (visible)
    {
        circle(Character.x, Character.y, 20);  //displays circle
        glColor3f(1, 1, 1);
    }
    displayState();  //displays the state
    glColor3f(title.r, title.g, title.r);

    circle(angle, fsin(angle)+ boundaryY-120, 5);  //random background decoration
    circle(angle, fcos(angle) + boundaryY - 500, 5); 

   
    if (ene.visible)  //if enemy is visisble, then make it appear, and do the reflection
    {
        drawMonster();
        colorReflection();

    }
   

    

    //add some extra functionality!! 

    glutSwapBuffers();
}

//
// key function for ASCII charachters like ESC, a,b,c..,A,B,..Z
//
void onKeyDown(unsigned char key, int x, int y)
{
    // exit when ESC is pressed.
    if (key == 27)
        exit(0);
    if (key == ' ')
    {
        pause = !pause;
    }
    // to refresh the window it calls display() function
    glutPostRedisplay();
}

void onKeyUp(unsigned char key, int x, int y)
{
    // exit when ESC is pressed.
    if (key == 27)
        exit(0);

    // to refresh the window it calls display() function
    glutPostRedisplay();
}

//
// Special Key like GLUT_KEY_F1, F2, F3,...
// Arrow Keys, GLUT_KEY_UP, GLUT_KEY_DOWN, GLUT_KEY_RIGHT, GLUT_KEY_RIGHT
//
void onSpecialKeyDown(int key, int x, int y)
{
    // Write your codes here.
    switch (key) {
    case GLUT_KEY_UP: up = true; break;
    case GLUT_KEY_DOWN: down = true; break;
    case GLUT_KEY_LEFT: left = true; break;
    case GLUT_KEY_RIGHT: right = true; break;
    }

    if (key == GLUT_KEY_F1)
    {
        if (ENEMYPSEED < 15)
        {
            ENEMYPSEED++;  //increases the speed

        }
    }

    if (key == GLUT_KEY_F2)
    {
        if (ENEMYPSEED > 5)
        {
            ENEMYPSEED--;

        }
    }

   
    // to refresh the window it calls display() function
    glutPostRedisplay();
}

//
// Special Key like GLUT_KEY_F1, F2, F3,...
// Arrow Keys, GLUT_KEY_UP, GLUT_KEY_DOWN, GLUT_KEY_RIGHT, GLUT_KEY_RIGHT
//
void onSpecialKeyUp(int key, int x, int y)
{
    // Write your codes here.
    switch (key) {
    case GLUT_KEY_UP: up = false; break;
    case GLUT_KEY_DOWN: down = false; break;
    case GLUT_KEY_LEFT: left = false; break;
    case GLUT_KEY_RIGHT: right = false; break;
    }

    // to refresh the window it calls display() function
    glutPostRedisplay();
}

//
// When a click occurs in the window,
// It provides which button
// buttons : GLUT_LEFT_BUTTON , GLUT_RIGHT_BUTTON
// states  : GLUT_UP , GLUT_DOWN
// x, y is the coordinate of the point that mouse clicked.
//
void onClick(int button, int stat, int x, int y)
{
    // Write your codes here.

    // to refresh the window it calls display() function
    glutPostRedisplay();
}

//
// This function is called when the window size changes.
// w : is the new width of the window in pixels.
// h : is the new height of the window in pixels.
//
void onResize(int w, int h)
{
    winWidth = w;
    winHeight = h;
    glViewport(0, 0, w, h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(-w / 2, w / 2, -h / 2, h / 2, -1, 1);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    display(); // refresh window.
}

void onMoveDown(int x, int y) {
    // Write your codes here.



    // to refresh the window it calls display() function   
    glutPostRedisplay();
}

// GLUT to OpenGL coordinate conversion:
//   x2 = x1 - winWidth / 2
//   y2 = winHeight / 2 - y1
void onMove(int x, int y) {
    // Write your codes here.

    Character.x = x - winWidth / 2;   //change of coordinates
    Character.y = winHeight / 2 - y;
    fire.pos.x = Character.x;
    fire.pos.y = Character.y;
   
    glutPostRedisplay();
}

#if TIMER_ON == 1
void onTimer(int v) {

    glutTimerFunc(TIMER_PERIOD, onTimer, 0);
    // Write your codes here.
    if (Character.x > boundaryX && Character.x < boundaryX + Width && Character.y < boundaryY && Character.y > boundaryY - Height)  //checking the boundary of the mouse
    {
        visible = true;   //checks if mouse is inside or outside
        initialmode = true;
    }
    else
    {
        visible = false;

    }
    ChangeColor(&title);
    if (pause == false)   //if pause==false, followplayer
    {
        FollowPlayer();

    }
    //bullets

    if (!pause)
    {
        angle += 5;  //angle increment
    }
  
   

    if (angle > boundaryX + Width)
    {
        angle = boundaryX;
    }
    // to refresh the window it calls display() function
    glutPostRedisplay(); // display()

}
#endif

void Init() {

    // Smoothing shapes
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    boundaryX = -500;  //initializing variables
    boundaryY = 320;
    Width = 1000;
    Height = 640;
    gra.r = 0.5;
    gra.g = 0.6;
    gra.b = 0.8;
    Character.x = -1000;  //outside the bounds of the window for now
    Character.y = 1000;
    ene.velocity.x = 0;
    ene.velocity.y = 0;
    fire.bulletradius = 5;
    ene.color = { 1,0,0 };
    ene.visible = true;
    srand(time(NULL));
}

void main(int argc, char* argv[]) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE);
    glutInitWindowSize(WINDOW_WIDTH, WINDOW_HEIGHT);
    //glutInitWindowPosition(100, 100);
    glutCreateWindow("PacMan! ~ Muhammad Bilal");

    glutDisplayFunc(display);
    glutReshapeFunc(onResize);

    //
    // keyboard registration
    //
    glutKeyboardFunc(onKeyDown);
    glutSpecialFunc(onSpecialKeyDown);

    glutKeyboardUpFunc(onKeyUp);
    glutSpecialUpFunc(onSpecialKeyUp);

    //
    // mouse registration
    //
    glutMouseFunc(onClick);
    glutMotionFunc(onMoveDown);
    glutPassiveMotionFunc(onMove);

#if  TIMER_ON == 1
    // timer event
    glutTimerFunc(TIMER_PERIOD, onTimer, 0);
#endif

    Init();

    glutMainLoop();
}
