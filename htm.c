#include <stdlib.h>
#include <stdio.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/freeglut.h>

#define PROX_THRESH 0.5
#define DIST_THRESH 0.5
#define CONNECTED 128

typedef unsigned short Seed[3];
#define LRAND(seed) (nrand48(seed))
#define DRAND(seed) (drand48(seed))

int cycles=0;
#define TICKS 8
#define TICK(t) ((cycles+(t))%TICKS)

int I=16; // input 2d dimension
int F=16; // input->column fanout
int D=4; // node root dimension

#define DIM2(x,y) (((x)*D)|(y))
#define DIM3(x,y,z) DIM2(DIM2((x),(y)),(z))

#define GETBIT(bits,x,y,z) (((bits)&((typeof(bits) 1)<<(DIM3((x),(y),(z)))))!=0)
#define SETBIT(bits,x,y,z) ((bits)|=((typeof(bits) 1)<<(DIM3((x),(y),(z)))))
#define CLRBIT(bits,x,y,z) ((bits)&=~((typeof(bits) 1)<<(DIM3((x),(y),(z)))))

#define GETNIB(bits,x,y) (((bits)>>(DIM3((x),(y),0)))&(typeof(bits) 0xf))
#define SETNIB(bits,x,y,val) ((bits)=((bits)&~((typeof(bits) 0xf)<<(DIM3((x),(y),0)))) | ((bits)|(((val)&(typeof(bits) 0xf))<<(DIM3((x),(y),0)))))

typedef long long Bits64;

typedef struct
{
    short size;
    Float *strength;
} InputMap;

void map_init(InputMap *map,int size,int unpack)
{
    int i;
    Seed seed;
    map->size=size*8*F;
    map->strength=malloc(size);
    seed=(Seed) map;
    LOOP(map->size,i) map->strength[i]=DRAND(seed);
}


typedef struct
{
    short  col_active  [TICKS] [D][D];                     // each nibble id's the active column in each node
    Bits64 cell_active [TICKS] [D][D];                     // bit per cell per node
    Float  col_bias            [D][D] [D][D];              // node (xy)(column(xy))
    Float  dist                [D][D] [D][D][D] [D][D][D];
    InputMap prox;
} Region;

#define LOOP(r,v) for ((v)=0;(v)<(r);(v)++)

#define FFS(b) (__builtin_ffs(b)-1)
#define FFSLL(b) (__builtin_ffsll(b)-1)
#define POPCOUNT(b) __builtin_popcount(b)
#define POPCOUNTLL(b) __builtin_popcountll(b)

void region_init(Region *region,int inputbits)
{
    int i;
    Seed seed=(Seed) region;
        
    BZERO(region);
    LOOP(D*D*D*D*D*D*D*D,i) region->dist+i=DRAND(seed); // full mesh between cells
    map_init(region->prox,inputbits);
}

void region_update(Region *region)
{
    int nx,ny,x,y,z,ix,iy,iz;
    int i,bit,fan,input;
    int cel[D][D] [D][D][D];
    int col[D][D] [D][D];
    int strongest;
    Seed seed=region->seed;
    int colmask=D*D*D*D-1;

    bzero(cel);
    bzero(col);
    
    // feed-forward input
    seed=(Seed) &region->prox;
    input=0;
    LOOP(region->inputsize,i)
    {
        c=getc();
        LOOP(8,bit)
        {
            LOOP(F,fan)
            {
                column=LRAND(seed)&colmask;
                if ((c&1) && (map->strength[input] * (region->col_bias)+column)>CONNECTED)
                    (col+column)++;
            }
            c>>=1;
        }
    }

    // predicting?
    LOOP(D,nx) LOOP(D,ny) LOOP(D,x) LOOP(D,y) LOOP(D,z) LOOP(D,ix) LOOP(D,iy) LOOP(D,iz)
        if (GETBIT(region->cell_active[TICK(-1)][nx][ny],x,y,z) &&
            (SYNAPSE[nx][ny][x][y][z][ix][iy][iz] * region->col_bias[nx][ny][x][y])>CONNECTED)
            cel[nx][ny][x][y][z]++;

    // activate cells
    LOOP(D,nx) LOOP(D,ny) LOOP(D,x) LOOP(D,y) LOOP(D,z)
        if (cel[nx][ny][x][y][z]>ACTIVE)
        {
            SETBIT(region->cell_active[TICK(0)][nx][ny],x,y,z);
            col[nx][ny][x][y]++;
        }

    // activate strongest column
    LOOP(D,nx) LOOP(D,ny)
    {
        strongest=ACTIVE;
        LOOP(D,x) LOOP(D,y)
            if (col[nx][ny][x][y]>strongest)
                SETBIT(region->col_active[TICK(0)][nx][ny],0,x,y); // not quite random
    }

    LOOP(D,nx) LOOP(D,ny) LOOP(D,x) LOOP(D,y)
        if (GETBIT(region->col_active[TICK(0)][nx][ny],0,x,y))
        {
            region->col_bias[nx][ny][x][y]-=16;
            if (!GETNIB(region->cell_active[TICK(0)][nx][ny],x,y))
                SETNIB(region->cell_active[TICK(0)][nx][ny],x,y,0xf);
        }
        else region->col_bias[nx][ny][x][y]++;
    
    // update synapses
}

void region_output(Region *region)
{
    printf("%llx",region->col_active);
    
}

void region_tick(Region *region)
{
    cycles++;
    bzero(region->col_active[TICK(0)]);
    bzero(region->cell_active[TICK(0)]);
}

void iterate()
{
    region_input(&region);
    region_update(&region);
    region_output(&region);
    region_tick(&region);
}


#include <stdlib.h>
#include <stdio.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/freeglut.h>

int gwidth=400,gheight=400;

// here's a few hardcoded RGBA color values
#define R 0xf30f
#define W 0xffff
#define X 0x0000
#define G 0x5c6c
#define B 0x111f

float camera[] = { 0,2,0 };
float center[] = { 0,0,-1 };
float viewup[] = { 0,1,0 };


int main(int argc, char **argv)
{
    unsigned int t=0;
    
    unsigned short tex[] = {
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
    };

    void texture(int id,int w,int h,unsigned short *data)
    {
        glBindTexture(GL_TEXTURE_2D, id);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA4, w, h, 0, GL_RGBA, GL_UNSIGNED_SHORT_4_4_4_4, data);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    }

    void display_quad(float z)
    {
        glEnable(GL_LIGHTING);
        glEnable(GL_LIGHT0);
        glEnable(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D,0);
        glBegin(GL_QUADS);
        glTexCoord2f(0,0); glVertex3f(-1,-1,z);
        glTexCoord2f(16,0); glVertex3f(1,-1,z);
        glTexCoord2f(16,16); glVertex3f(1,1,z);
        glTexCoord2f(0,16); glVertex3f(-1,1,z);
        glEnd();
        glDisable(GL_TEXTURE_2D);
        glDisable(GL_LIGHTING);
    }
 
    void display_line(float z1,float z2)
    {
        glBegin(GL_LINES);
        glColor3f(1,0,0); glVertex3f(-1,-1,z1);
        glColor3f(0,1,0); glVertex3f(1,1,z2);
        glEnd();
    }
 
    void display()
    {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glLoadIdentity();
        gluLookAt(camera[0],camera[1],camera[2],
                  center[0],center[1],center[2],
                  viewup[0],viewup[1],viewup[2]);
        //glScalef (1.0, 1.0, 1.0);      /* modeling transformation */
        display_quad(-2.0);
        display_line(-2.0,-1.0);
        display_quad(-1.0);
        glFlush();
    }
 
    void reshape(int w,int h)
    {
        float r=w>h?(float) w/(float) h:(float) h/(float) w;
        gwidth=w;
        gheight=h;
        glViewport(0,0,(GLsizei) w,(GLsizei) h);
        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        if (w>h) glFrustum (-r,r,-1.0,1.0,0.5,20.0);
        else     glFrustum (-1.0,1.0,-r,r,0.5,20.0);
        glMatrixMode(GL_MODELVIEW);
    }
    
    void keyboard(unsigned char key,int x,int y)
    {
        switch (key)
        {
            case 'q': case 27: exit(0); // esc
            case '1': t++; break;
            case '2': t--; break;
        }
        texture(0,16,16,tex+(16*(t%16)));
    }
    
    void mouse(int button,int state,int x,int y) { printf("%d,%d,%d,%d\n",button,state,x,y); }
    void motion(int x,int y) { printf("%d,%d\n",x,y); }

    void idle()
    {
        glutPostRedisplay();
        glutSwapBuffers();
        sleep(1);
    }
    
    void menuselect(int id)
    {
        switch (id)
        {
            case 0: exit(0); break;
        }
    }
    
    int menu(void)
    {
        int menu=glutCreateMenu(menuselect);
        glutAddMenuEntry("Exit demo\tEsc",0);
        return menu;

    }

    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DEPTH | GLUT_SINGLE | GLUT_RGBA);
    glutInitWindowPosition(100,100);
    glutInitWindowSize(gwidth,gheight);
    glutCreateWindow("HTM");
    
    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutKeyboardFunc(keyboard);
    glutMouseFunc(mouse);
    glutMotionFunc(motion);
    glutIdleFunc(idle);
    
    menu();
    glutAttachMenu(GLUT_RIGHT_BUTTON);

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    glShadeModel(GL_SMOOTH);

    reshape(gwidth,gheight);
    texture(0,16,16,tex);
    
    glutMainLoop();
    return 0;
}
