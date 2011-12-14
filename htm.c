#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <GL/glut.h>

#define PROX_THRESH 0.5
#define DIST_THRESH 0.5
#define CONNECTED 128


int cycles=0;
#define TICKS 2
#define TICK(t) ((cycles-(t))%TICKS)

int I=16; // input 2d dimension
int F=8; // input->column fanout
int D=4; // node root dimension

typedef long long Bits64;
typedef char Coarse;
typedef float Fine;

typedef unsigned short Seed[3];
#define LRAND(seed) (nrand48(seed))
#define DRAND(seed) (drand48(seed))


#define DIM2(x,y) (((x)*D)|(y))
#define DIM3(x,y,z) DIM2(DIM2((x),(y)),(z))

#define GETBIT(bits,x,y,z) (((bits)&((typeof(bits) 1)<<(DIM3((x),(y),(z)))))!=0)
#define SETBIT(bits,x,y,z) ((bits)|=((typeof(bits) 1)<<(DIM3((x),(y),(z)))))
#define CLRBIT(bits,x,y,z) ((bits)&=~((typeof(bits) 1)<<(DIM3((x),(y),(z)))))

#define GETNIB(bits,x,y) (((bits)>>(DIM3((x),(y),0)))&(typeof(bits) 0xf))
#define SETNIB(bits,x,y,val) ((bits)=((bits)&~((typeof(bits) 0xf)<<(DIM3((x),(y),0)))) | ((bits)|(((val)&(typeof(bits) 0xf))<<(DIM3((x),(y),0)))))

typedef struct
{
    short  col_output     [TICKS] [D][D];                     // each nibble id's the active column in each node
    short  cell_active    [TICKS] [D][D][D];                  // bit per cell per plane per node
    Coarse col_potential          [D][D] [D][D];              // node (xy)(column(xy))
    Coarse col_bias               [D][D] [D][D];              // node (xy)(column(xy))
    Coarse distal                 [D][D] [D][D][D] [D][D][D]; // synapse permanence of node(xy)(cell_to(xyz),cell_from(xyz))

    int inputsize;
    char *inputs[TICKS];
    Coarse *proximal; // synapse permanence from input x to col y
    Seed seed;

} Region;

#define LOOP(r,v) for ((v)=0;(v)<(r);(v)++)

#define FFS(b) __builtin_ffs(b)
#define FFSLL(b) __builtin_ffsll(b)
#define POPCOUNT(b) __builtin_popcount(b)
#define POPCOUNTLL(b) __builtin_popcountll(b)

void region_init(Region *region)
{
    int nx,ny,x,y,z,x2,y2,z2,fanout;
    Seed seed;
       
    BZERO(region);
    LRAND(region->seed);
    seed=region->seed;

       
    LOOP(D,nx) LOOP(D,ny)
        LOOP(D,x) LOOP(D,y) LOOP(D,z)
        LOOP(D,x2) LOOP(D,y2) LOOP(D,z2)
        region->synapse[nx][ny][x][y][z][x2][y2][z2]=LRAND(seed);
}
   
// accept a count and series of bytes, each of which ids a column(nx:2,ny:2,cx:2,cy:2) to sensitize
void region_read(Region *region,int size,char *data)
{
    int i;
   
    if (!inputsize)
    {
        inputsize=size;
        inputs=malloc(size*TICKS);
        proximal=malloc(sizeof(proximal[0])*size*8);

    }
   
    if (!region->inputMap[dir])
    {
        int samples=width*height*words*bits;
        region->fanout=((D*D*D*D*F)/samples)+1; // at least F samples per region-columns
        region->inputMap[dir]=malloc(samples*region->fanout);

    }
   
    BZERO(region->node[x][y].col_input);

    LOOP(width,x) LOOP(height,y) LOOP(words,w) LOOP(bits,b)


        BZERO(region->node[x][y].col_input);
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
    float cell[D][D] [D][D][D];
    Seed seed=region->seed;

   
    // predicting?
    LOOP(D,nx) LOOP(D,ny)
        LOOP(D,x) LOOP(D,y) LOOP(D,z)
        LOOP(D,ix) LOOP(D,iy) LOOP(D,iz)
        if (region->cell_active[nx][ny][z]&DIM2(x,y))
            cell[nx][ny][x][y][z]+=SYNAPSE[nx][ny][x][y][z][ix][iy][iz];

    // sensitized?
    // go through inputs bit by bit, setting cols using _repeating_ "random" series

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
    printf("%llx",region->col_output);
}

void iterate()

{
    while (region_input(&region) &&
           region_propagate(&region) &&
           region_update(&region) &&
           region_output(&region))
        cycles++;
}



int gwidth=200,gheight=200;


// here's a few hardcoded RGBA color values
#define R 0xf30f
#define W 0xffff
#define X 0x0000
#define G 0x5c6c
#define B 0x111f

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

    void texture(int id,unsigned short *data)
    {
        glBindTexture(GL_TEXTURE_2D, id);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA4, 16, 16, 0, GL_RGBA, GL_UNSIGNED_SHORT_4_4_4_4, data);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    }

    void display()
    {
        glEnable(GL_LIGHTING);
        glEnable(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D,0);

        glBegin(GL_QUADS);
        glTexCoord2f(0, 0); glVertex2f(-1,-1);
        glTexCoord2f(1, 0); glVertex2f(1,-1);
        glTexCoord2f(1, 1); glVertex2f(1,1);
        glTexCoord2f(0, 1); glVertex2f(-1,1);
        glEnd();
        glDisable(GL_TEXTURE_2D);
        glDisable(GL_LIGHTING);

        glBegin(GL_LINES);
        glColor3f(1,0,0);
        glVertex2f(-1,-1);
        glColor3f(0,1,0);
        glVertex2f(1,1);
        glEnd();
    }
 
    void reshape(int w,int h)
    {
        gwidth=w;
        gheight=h;
        // change view to screen coordinates
        //glViewport(0,0,(GLsizei) w,(GLsizei) h);
        //glMatrixMode(GL_PROJECTION);
        //glLoadIdentity();
        //glOrtho(0,w,0,h,-1.0,1.0);
        //glMatrixMode(GL_MODELVIEW);
    }
   
    void keyboard(unsigned char key,int x,int y)
    {
        switch (key)
        {
            case 'q': case 27: exit(0); // esc
            case '1': t++; break;
            case '2': t--; break;
        }
        texture(0,tex+(16*(t%16)));
    }
   
    void mouse(int button,int state,int x,int y) { printf("%d,%d,%d,%d\n",button,state,x,y); }
    void motion(int x,int y) { printf("%d,%d\n",x,y); }

    void idle()
    {
        glutPostRedisplay();
        glutSwapBuffers();
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
    //glutReshapeFunc(reshape);
    glutKeyboardFunc(keyboard);
    glutMouseFunc(mouse);
    glutMotionFunc(motion);
    glutIdleFunc(idle);
   
    menu();
    glutAttachMenu(GLUT_RIGHT_BUTTON);

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    glShadeModel(GL_SMOOTH);
    glEnable(GL_LIGHT0);
    glEnable(GL_TEXTURE_2D);
    //glEnable(GL_LINE_SMOOTH);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    //reshape(gwidth,gheight);
    texture(0,tex);

    glutMainLoop();
    return 0;
}
