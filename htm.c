#include <stdlib.h>
#include <stdio.h>
#include <GL/glut.h>


#define PROX_THRESH 0.5
#define DIST_THRESH 0.5

int cycles=0;
#define TICKS 2
#define TICK(t) ((cycles-(t))%TICKS)

int I=16; // input 2d dimension
int F=8;  // input->column fanout
int D=4;  // node root dimension

#define CONNECTED 128

typedef long long Bits64;
typedef char Coarse;
typedef float Fine;

#define RAND(bits,mask) ((rand()>>(31-bits)) & (mask))

#define BIT0 = ((long long) 1)
#define NIB0 = ((long long) 0xf)

#define DIM2(x,y) (((x)*D)|(y))
#define DIM3(x,y,z) DIM2(DIM2((x),(y)),(z))

#define GETBIT(bits,x,y,z) (((bits)&(BIT0<<(DIM3((x),(y),(z)))))!=0)
#define SETBIT(bits,x,y,z) ((bits)|=(BIT0<<(DIM3((x),(y),(z)))))
#define CLRBIT(bits,x,y,z) ((bits)&=~(BIT0<<(DIM3((x),(y),(z)))))

#define GETNIB(bits,x,y) (((bits)>>(DIM3((x),(y),0)))&NIB0)
#define SETNIB(bits,x,y,val) ((bits)=((bits)&~(NIB0<<(DIM3((x),(y),0)))) | ((bits)|(((val)&NIB0)<<(DIM3((x),(y),0)))))



typedef struct
{
    Bits cell_active[TICKS];
    Bits cell_predicting[TICKS];
    Coarse potential;
    Coarse bias;
    Coarse input;
} Column;

typedef struct
{
    Bits col_output[TICKS];

    Column column [D][D] [D][D]; // node  (xy)(column(xy))
    Coarse synapse[D][D] [D][D][D] [D][D][D]; // permanence from node(xy)(cell(xyz),cell(xyz))
    unsigned short *inputMap[6]; // create maps (one per cube face) to fan inputs to F * column(nx,ny,cx,cy)
    char fanout;
} Region;

#define LOOP(r,v) for ((v)=0;(v)<(r);(v)++)


#define COLUMN(nx,ny,x,y) (region->column[nx][ny][x][y])
#define SYNAPSE(nx,ny,sx,sy,sz,dx,dy,dz) (region->cell[nx][ny][sx][sy][sz][dx][dy][dz])

void region_input(Region *region,int dir,int x,int y,int words,int bits)
{
    int w,x,y,z,fanout,bit;
    char c;

    if (!region->inputMap[dir])
    {
        int samples=x*y*words*bits;
       
       
        region->inputMap[dir]=malloc(sizeof(short)*(((samples/(D*D*D*D))+1)*F)); // ~F samples per region's column

    }
   
    LOOP(D,x) LOOP(D,y) BZERO(region->node[x][y].col_input);
   
    LOOP(d1,x) LOOP(d2,y) LOOP(F,fanout)

    {
        c=getc(stdin); // just bottom-up for now
        LOOP(8,bit)
            (region->column+(region->inputMap[x+bit][y][fanout]))->input += (c>>bit)&1;
    }
}

void region_propagate(Region *region)
{
    int nx,ny,x,y,z,ix,iy,iz;

    // predicting?
    LOOP(D,nx) LOOP(D,ny)
        LOOP(D,x) LOOP(D,y)
    {
        LOOP(D,z)
        {
        LOOP(D,ix) LOOP(D,iy) LOOP(D,iz)
            if (COLUMN(nx,ny,x,y).active[ * SYNAPSE(nx,ny,x,y,z,ix,iy,iz) > DIST_THRESH)
                CELL(nx,ny,x,y,z).predicting++;
                }
        }
       
           
        // adjust synapses

        (region->column+activeColumn)->bias++;
       
}

void region_update(Region *region)
{
    int nx,ny,x,y,z,x2,y2,z2,fanout;

    LOOP(D,nx) LOOP(D,ny)
    {
        // pick winning column
        LOOP(D,x) LOOP(D,y)
       
        // adjust synapses
        LOOP(D,x) LOOP(D,y) region->column[nx][ny][x][y].bias++;
       
        (region->column[nx][ny]+activeColumn)->bias=;
    }
}

void region_output(Region *region)
{
    int nx,ny;
   
    LOOP(D,nx) LOOP(D,ny)
        printf("%x",region->activeColumn;
              
               }
       
        void init();
   
void iterate()
{
    Region region;

    void init()
    {
        int nx,ny,x,y,z,x2,y2,z2,fanout;
       
        BZERO(region);
       
        LOOP(I,x) LOOP(I,y) LOOP(F,fanout) region.inputMap=RAND(16,0xffff);
       
        LOOP(D,nx) LOOP(D,ny)
            LOOP(D,x) LOOP(D,y)
        {
            LOOP(D,z)
                LOOP(D,x2) LOOP(D,y2) LOOP(D,z2)
                region->synapse[nx][ny][x][y][z][x2][y2][z2]=RAND(8,0xff);
        }
    }

    region_init(&region);

    while (region_input(&region) &&
           region_propagate(&region) &&
           region_update(&region) &&
           region_output(&region))
        cycles++;
}




 

// here's a few hardcoded RGBA color values
#define R 0xf30f
#define W 0xffff
#define X 0x0000
#define G 0x5c6c
#define B 0x111f

int t=0;

unsigned short tex[2][(16 * 16)] = {
    {
        X,X,X,B,B,B,B,B,B,B,B,B,B,X,X,X,
        X,X,B,B,W,W,W,W,W,W,W,W,B,B,X,X,
        X,X,B,W,W,W,W,W,W,W,W,W,W,B,X,X,
        X,B,B,W,W,W,R,W,W,R,W,W,W,B,B,X,
        B,B,B,B,W,W,R,W,W,R,W,W,B,B,B,B,
        B,W,W,B,B,B,B,B,B,B,B,B,B,W,W,B,
        B,W,W,W,W,W,G,G,G,G,W,W,W,W,W,B,
        B,G,G,W,W,G,G,G,G,G,G,W,W,G,G,B,
        B,G,G,G,W,G,G,G,G,G,G,W,G,G,G,B,
        B,G,G,G,W,G,G,G,G,G,G,W,G,G,G,B,
        B,B,G,G,W,W,G,G,G,G,W,W,G,G,B,B,
        X,B,G,W,W,W,W,W,W,W,W,W,W,G,B,X,
        X,B,B,W,G,G,W,W,W,W,G,G,W,B,B,X,
        X,X,B,B,G,G,G,W,W,G,G,G,B,B,X,X,
        X,X,X,B,B,B,G,W,W,G,B,B,B,X,X,X,
        X,X,X,X,X,B,B,B,B,B,B,X,X,X,X,X,
    },
    {
        X,X,X,X,X,B,B,B,B,B,B,X,X,X,X,X,
        X,X,X,B,B,B,G,W,W,G,B,B,B,X,X,X,
        X,X,B,B,G,G,G,W,W,G,G,G,B,B,X,X,
        X,B,B,W,G,G,W,W,W,W,G,G,W,B,B,X,
        X,B,G,W,W,W,W,W,W,W,W,W,W,G,B,X,
        B,B,G,G,W,W,G,G,G,G,W,W,G,G,B,B,
        B,G,G,G,W,G,G,G,G,G,G,W,G,G,G,B,
        B,G,G,G,W,G,G,G,G,G,G,W,G,G,G,B,
        B,G,G,W,W,G,G,G,G,G,G,W,W,G,G,B,
        B,W,W,W,W,W,G,G,G,G,W,W,W,W,W,B,
        B,W,W,B,B,B,B,B,B,B,B,B,B,W,W,B,
        B,B,B,B,W,W,R,W,W,R,W,W,B,B,B,B,
        X,B,B,W,W,W,R,W,W,R,W,W,W,B,B,X,
        X,X,B,W,W,W,W,W,W,W,W,W,W,B,X,X,
        X,X,B,B,W,W,W,W,W,W,W,W,B,B,X,X,
        X,X,X,B,B,B,B,B,B,B,B,B,B,X,X,X,
    }
};

 
void init()
{
    glEnable(GL_TEXTURE_2D);
    //glDisable(GL_TEXTURE_2D);
   
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
   
    glBindTexture(GL_TEXTURE_2D, 0);   
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA4, 16, 16, 0, GL_RGBA, GL_UNSIGNED_SHORT_4_4_4_4,tex[0]);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
   
    glBindTexture(GL_TEXTURE_2D, 1);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA4, 16, 16, 0, GL_RGBA, GL_UNSIGNED_SHORT_4_4_4_4,tex[1]);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
}

void display()
{
    //glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glBindTexture(GL_TEXTURE_2D,t);
    glBegin(GL_QUADS);
    glTexCoord2f(0, 0); glVertex2f(-1,-1);
    glTexCoord2f(1, 0); glVertex2f(1,-1);
    glTexCoord2f(1, 1); glVertex2f(1,1);
    glTexCoord2f(0, 1); glVertex2f(-1,1);
    glEnd();
}

void keyboard(unsigned char key, int x, int y)
{
    switch (key)
    {
        case 27: // ESCAPE key
            exit(0);
            break;
        case '1':
            t=0;
            break;
        case '2':
            t=1;
            break;
    }
}

void idle()
{
    glutPostRedisplay();
    glutSwapBuffers();
}

int main(int argc, char **argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DEPTH | GLUT_SINGLE | GLUT_RGBA);
    glutInitWindowPosition(100,100);
    glutInitWindowSize(320,320);
    glutCreateWindow("HTM");
    init();
    glutDisplayFunc(display);
    glutKeyboardFunc(keyboard);
    glutIdleFunc(idle);
    glutMainLoop();
}
