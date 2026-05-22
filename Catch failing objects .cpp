/*
 ============================================================
  CATCH THE FALLING OBJECTS
  graphics.h | Code::Blocks 10.05 | Windows

  CONTROLS:
  - LEFT  arrow = move basket left
  - RIGHT arrow = move basket right
  - ESC         = quit
  - SPACE       = start / restart
 ============================================================
*/

#include <graphics.h>
#include <conio.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <windows.h>

// ─── Screen ──────────────────────────────────────────────
#define SCR_W       640
#define SCR_H       480

// ─── Basket ──────────────────────────────────────────────
#define BAS_W        80
#define BAS_H        20
#define BAS_Y       430
#define BAS_SPEED    15

// ─── Objects ─────────────────────────────────────────────
#define MAX_OBJ      5
#define OBJ_STAR     1
#define OBJ_BOMB     2
#define OBJ_GEM      3

// ═════════════════════════════════════════════════════════
//  GLOBALS
// ═════════════════════════════════════════════════════════
int  basX    = SCR_W/2 - BAS_W/2;
int  score   = 0;
int  best    = 0;
int  lives   = 3;
int  level   = 1;
bool running = true;

struct Obj { int x, y, type, spd; bool on; };
Obj obj[MAX_OBJ];

// ═════════════════════════════════════════════════════════
//  KEY HELPER
// ═════════════════════════════════════════════════════════
bool keyDown(int vk){
    return (GetAsyncKeyState(vk) & 0x8000) != 0;
}
void flushKeys(){
    while(kbhit()) getch();
}
void waitRelease(int vk){
    while(GetAsyncKeyState(vk) & 0x8000) Sleep(10);
}

// ═════════════════════════════════════════════════════════
//  DRAW HELPERS
// ═════════════════════════════════════════════════════════

void drawStar(int cx, int cy, int r, int col){
    setcolor(col);
    float pi = 3.14159f;
    // draw 5-point star by connecting every other vertex
    for(int i=0;i<5;i++){
        float a1 = -pi/2 + i*2*pi/5;
        float a2 = -pi/2 + (i+2)*2*pi/5;
        line(cx+(int)(r*cos(a1)), cy+(int)(r*sin(a1)),
             cx+(int)(r*cos(a2)), cy+(int)(r*sin(a2)));
    }
    setfillstyle(SOLID_FILL,col);
    fillellipse(cx,cy,3,3);
}

void drawGem(int cx, int cy, int r, int col){
    setcolor(col);
    int pts[]={cx,cy-r, cx+r,cy, cx,cy+r, cx-r,cy, cx,cy-r};
    drawpoly(5,pts);
    int c2 = (col+1<16)? col+1 : col;
    setcolor(c2);
    line(cx-r/2,cy, cx+r/2,cy);
    line(cx,cy-r/2, cx,cy+r/2);
}

void drawBomb(int cx, int cy, int r, int col){
    setcolor(col);
    setfillstyle(SOLID_FILL, col);
    fillellipse(cx, cy+2, r, r);
    setcolor(14);
    line(cx, cy-r+2, cx+5, cy-r-5);
    putpixel(cx+6, cy-r-6, 15);
    putpixel(cx+7, cy-r-5, 14);
}

void drawBasket(int bx){
    int by = BAS_Y;
    // shadow
    setfillstyle(SOLID_FILL,8); setcolor(8);
    bar(bx+4, by+4, bx+BAS_W+4, by+BAS_H+4);
    // body
    setfillstyle(SOLID_FILL,2); setcolor(2);
    bar(bx, by, bx+BAS_W, by+BAS_H);
    // thick rim (3 lines)
    setcolor(10);
    line(bx,by,   bx+BAS_W,by);
    line(bx,by+1, bx+BAS_W,by+1);
    line(bx,by+2, bx+BAS_W,by+2);
    // weave lines
    setcolor(6);
    for(int i=1;i<=3;i++)
        line(bx+i*(BAS_W/4), by, bx+i*(BAS_W/4), by+BAS_H);
    line(bx, by+BAS_H/2, bx+BAS_W, by+BAS_H/2);
    // border
    setcolor(10);
    rectangle(bx, by, bx+BAS_W, by+BAS_H);
}

void drawHUD(){
    // top bar
    setfillstyle(SOLID_FILL,1); setcolor(1);
    bar(0,0,SCR_W,22);
    setcolor(3); line(0,22,SCR_W,22);

    char buf[50];
    settextstyle(DEFAULT_FONT,HORIZ_DIR,1);

    setcolor(14); sprintf(buf,"SCORE:%d",score);  outtextxy(8,5,buf);
    setcolor(11); sprintf(buf,"BEST:%d",best);    outtextxy(150,5,buf);
    setcolor(10); sprintf(buf,"LEVEL:%d",level);  outtextxy(280,5,buf);

    // hearts
    setcolor(12);
    char lv[]="LIVES:"; outtextxy(400,5,lv);
    for(int i=0;i<lives;i++){
        int hx=460+i*24, hy=11;
        setfillstyle(SOLID_FILL,12);
        fillellipse(hx-4,hy,4,4);
        fillellipse(hx+4,hy,4,4);
        int hp[]={hx-7,hy+2, hx+7,hy+2, hx,hy+10, hx-7,hy+2};
        fillpoly(4,hp);
    }

    // bottom controls bar
    setfillstyle(SOLID_FILL,1); setcolor(1);
    bar(0,SCR_H-16,SCR_W,SCR_H);
    setcolor(8);
    char ct[]="LEFT / RIGHT = move    ESC = quit";
    outtextxy(180,SCR_H-13,ct);
}

void drawGround(){
    setcolor(3);
    line(0,BAS_Y+BAS_H+4, SCR_W,BAS_Y+BAS_H+4);
}

// ═════════════════════════════════════════════════════════
//  OBJECT LOGIC
// ═════════════════════════════════════════════════════════
void resetObjs(){
    for(int i=0;i<MAX_OBJ;i++) obj[i].on=false;
}

void spawnObj(){
    for(int i=0;i<MAX_OBJ;i++){
        if(!obj[i].on){
            obj[i].x    = 20 + rand()%(SCR_W-40);
            obj[i].y    = -16;
            int r       = rand()%100;
            if(r<50)       obj[i].type=OBJ_STAR;
            else if(r<75)  obj[i].type=OBJ_GEM;
            else           obj[i].type=OBJ_BOMB;
            obj[i].spd  = 2 + level + rand()%2;
            obj[i].on   = true;
            break;
        }
    }
}

void updateObjs(){
    for(int i=0;i<MAX_OBJ;i++){
        if(!obj[i].on) continue;
        obj[i].y += obj[i].spd;

        int cx=obj[i].x, cy=obj[i].y;

        // basket collision
        if(cy+14 >= BAS_Y       && cy-14 <= BAS_Y+BAS_H &&
           cx+14 >= basX        && cx-14 <= basX+BAS_W){
            obj[i].on = false;
            if(obj[i].type==OBJ_BOMB){
                lives--;
                if(lives<=0) running=false;
            } else {
                score += (obj[i].type==OBJ_GEM) ? 3 : 1;
                if(score>best) best=score;
                level = score/10 + 1;
                if(level>10) level=10;
            }
            continue;
        }

        // off screen
        if(obj[i].y > SCR_H){
            obj[i].on=false;
        }
    }
}

void drawObjs(){
    for(int i=0;i<MAX_OBJ;i++){
        if(!obj[i].on) continue;
        if(obj[i].type==OBJ_STAR) drawStar(obj[i].x,obj[i].y,13,14);
        if(obj[i].type==OBJ_GEM)  drawGem (obj[i].x,obj[i].y,13, 9);
        if(obj[i].type==OBJ_BOMB) drawBomb(obj[i].x,obj[i].y,13, 8);
    }
}

// ═════════════════════════════════════════════════════════
//  SCREENS
// ═════════════════════════════════════════════════════════
void splashScreen(){
    cleardevice();

    // title
    setcolor(14); settextstyle(DEFAULT_FONT,HORIZ_DIR,3);
    char t1[]="CATCH THE";       outtextxy(170,60,t1);
    setcolor(11);
    char t2[]="FALLING OBJECTS"; outtextxy(80,110,t2);

    settextstyle(DEFAULT_FONT,HORIZ_DIR,1);

    // instruction box
    setcolor(3); rectangle(80,170,560,390);

    setcolor(15); char h[]="HOW TO PLAY:"; outtextxy(250,180,h);

    drawStar(115,225,12,14);
    setcolor(15); char a1[]="Star    = +1 point  (catch it!)";  outtextxy(140,218,a1);

    drawGem(115,258,12,9);
    setcolor(15); char a2[]="Diamond = +3 points (rare & valuable!)"; outtextxy(140,251,a2);

    drawBomb(115,291,12,8);
    setcolor(15); char a3[]="Bomb    = -1 life   (avoid it!)";   outtextxy(140,284,a3);

    setcolor(8);
    char a4[]="Speed increases every 10 points.";  outtextxy(140,320,a4);
    char a5[]="You have 3 lives. Don't let bombs hit the basket!"; outtextxy(140,338,a5);
    char a6[]="LEFT / RIGHT arrow keys to move the basket."; outtextxy(140,356,a6);

    setcolor(7);
    char a7[]="Missing a star/diamond = no penalty"; outtextxy(140,374,a7);

    // start prompt
    setcolor(10); settextstyle(DEFAULT_FONT,HORIZ_DIR,2);
    char s[]="Press SPACE to START"; outtextxy(130,410,s);
    settextstyle(DEFAULT_FONT,HORIZ_DIR,1);

    flushKeys();

    // wait for SPACE
    while(true){
        if(keyDown(VK_SPACE))  { waitRelease(VK_SPACE);  break; }
        if(keyDown(VK_RETURN)) { waitRelease(VK_RETURN); break; }
        if(keyDown(VK_ESCAPE)) { closegraph(); exit(0); }
        Sleep(20);
    }
}

void gameOverScreen(){
    // draw dark box
    setfillstyle(SOLID_FILL,0); setcolor(0);
    bar(120,140,520,360);
    setcolor(4);
    rectangle(120,140,520,360);
    rectangle(122,142,518,358);

    setcolor(12); settextstyle(DEFAULT_FONT,HORIZ_DIR,3);
    char g[]="GAME OVER"; outtextxy(172,160,g);

    settextstyle(DEFAULT_FONT,HORIZ_DIR,2);
    setcolor(14); char sc[30]; sprintf(sc,"Score : %d",score); outtextxy(200,240,sc);
    setcolor(11); char bs[30]; sprintf(bs,"Best  : %d",best);  outtextxy(200,275,bs);

    settextstyle(DEFAULT_FONT,HORIZ_DIR,1);
    setcolor(7);
    char r1[]="Press SPACE to Play Again"; outtextxy(195,320,r1);
    char r2[]="Press ESC   to Quit";       outtextxy(210,340,r2);

    flushKeys();

    while(true){
        if(keyDown(VK_SPACE)){
            waitRelease(VK_SPACE);
            score=0; lives=3; level=1;
            basX=SCR_W/2-BAS_W/2;
            running=true;
            resetObjs();
            return;
        }
        if(keyDown(VK_ESCAPE)){ closegraph(); exit(0); }
        Sleep(20);
    }
}

// ═════════════════════════════════════════════════════════
//  MAIN
// ═════════════════════════════════════════════════════════
int main(){
    srand((unsigned)time(NULL));

    int gd=DETECT, gm;
    char p[]="";
    initgraph(&gd,&gm,p);

    resetObjs();
    splashScreen();

    int frame = 0;

    while(true){

        // ── INPUT ─────────────────────────────────────────
        if(keyDown(VK_LEFT)){
            basX -= BAS_SPEED;
            if(basX < 0) basX = 0;
        }
        if(keyDown(VK_RIGHT)){
            basX += BAS_SPEED;
            if(basX > SCR_W - BAS_W) basX = SCR_W - BAS_W;
        }
        if(keyDown(VK_ESCAPE)){ closegraph(); return 0; }

        // ── UPDATE ────────────────────────────────────────
        if(running){
            // spawn interval shrinks with level
            int spawnRate = 50 - (level-1)*4;
            if(spawnRate < 15) spawnRate = 15;
            if(frame % spawnRate == 0) spawnObj();

            updateObjs();
        }

        // ── DRAW ──────────────────────────────────────────
        cleardevice();

        // black background
        setfillstyle(SOLID_FILL,0); setcolor(0);
        bar(0,0,SCR_W,SCR_H);

        drawGround();
        drawObjs();
        drawBasket(basX);
        drawHUD();

        // level label blink (shows for half a second when level changes)
        if(frame%60 < 30 && level > 1){
            setcolor(14);
            settextstyle(DEFAULT_FONT,HORIZ_DIR,2);
            char lb[20]; sprintf(lb,"LEVEL %d !",level);
            outtextxy(SCR_W/2-70, SCR_H/2-20, lb);
            settextstyle(DEFAULT_FONT,HORIZ_DIR,1);
        }

        // show game over overlay when lives = 0
        if(!running){
            gameOverScreen();
            frame = 0;
            continue;
        }

        frame++;
        Sleep(22);   // ~45 FPS
    }

    closegraph();
    return 0;
}
