#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/freeglut.h>
#include <countof.h>

#define DENDRITE_CACHE 0x1000
#define DENDRITES 16
#define SYNAPSES 16

#define PROX_THRESH 0x30
#define DIST_THRESH 0x30
#define INIT_THRESH 0x3f // mask, 0-63
#define IS_ACTIVE 0x80
#define WAS_ACTIVE 0x40

#define BZERO(x) bzero((&x),sizeof(x))

typedef struct { unsigned short seed[0]; void *d1; void *d2 } Seed;
#define RESEED(seed,ptr) (seed.d1=seed.d2=ptr,seed.seed)
#define LRAND(seed) (nrand48(&seed))
#define DRAND(seed) (erand48(&seed))

typedef struct { int v[0]; int x,y,z,vol; } D3;
#define VOL3V(i) ((i)[0]*(i)[1]*(i)[2])
#define DIM3(x,y,z,d) ((x)*(d)[1]*(d)[2] + (y)*(d)[2] + (z))
#define DIM3V(i,d) (((i)[4]=DIM3((i)[0],(i)[1],(i)[2],(d))),((i)[4]<0?-1:(i)[4]>(d)[4]?-1:(i)[4])))

#define LOOP(i,lim) for ((i)=0;(i)<(lim);(i)++)
#define LOOPD3(iv,limv) LOOP((iv)[0],(limv)[0]) LOOP((iv)[0],(limv)[0]) LOOP((iv)[0],(limv)[0])

#define DENDRITE(interface,pos,d) ((interface)->dendrite[DIM3V((pos),(interface)->output.size.v)*DENDRITES + (d)])

typedef struct { int i[3];  } ivec;
typedef struct { char c[3]; } cvec;
typedef struct { char f[3]; } fvec;

typedef struct
{
    char permanence;
    char sensitivity;
} Synapse;

typedef struct
{
    Synapse synapse[SYNAPSES];
} Dendrite;
   
typedef struct
{
    D3 size;
    char *signal; // msb represents current signal, 50% decay per tick
} SignalMap;

typedef struct
{
    SignalMap *input;
    SignalMap *output;
    Dendrite *dendrite;
    float *bias;
} Interface;

typedef struct
{
    SignalMap cells;
    SignalMap sensors;
    Interface bottom_up; // per sensor
    Interface lateral;   // per cell, references own SignalMap
    Interface top_down;  // per cell
} Region;


typedef cvec DendriteMap[SYNAPSES];
DendriteMap gDendriteMap[DENDRITE_CACHE];
int cycles=0;
Seed gseed;

#define GDENDRITE(seed) (gDendriteMap[LRAND(seed)%DENDRITE_CACHE])

void DendriteMap_init()
{
    int dendrite;
   
    void generate()
    {
        int synapse;
        unsigned long long r=LRAND(gseed);
        int v[]={1,(r&0x10000000)?1:-1};
        int xx=(r&0x2000000)?1:-1; // flip l/r
        int xy=(r&0x4000000)?1:0; // switch axes
        int mx[]={6,2};

        r<<=31;
        r|=LRAND(gseed);
       
        LOOP(synapse,SYNAPSES)
        {
            //if ((r&mx[v[0]]) && !(v[0]=!v[0]) && (r&1)) v[1]=-v[1];
            if (r&mx[v[0]] && v[0]=!v[0] && r&1) v[1]=-v[1];
            GDENDRITE(dendrite)[synapse][xy]=v[0]*xx;
            GDENDRITE(dendrite)[synapse][!xy]=v[0]?0:v[1];
            GDENDRITE(dendrite)[synapse][2]=p[2]=(r&0xff000)>>12;
            r>>=1;
        }
    }
   
    LOOP(dendrite,DENDRITE_CACHE) generate();
}

int SignalMap_init(SignalMap *sigmap,D3 *size)
{
    int i;

    if (!sigmap) return !0;
    size->vol=VOL3(size->v);
    sigmap->size=*size;
    sigmap->sig=malloc(size->vol);
    bzero(sigmap->sig[i],signals);
    return 0;
}

int Interface_init(Interface *interface,SignalMap *input,SignalMap *output,int sensitivity)
{
    int d,s;
    int dendrites;

    if (!interface || !output || !input) return !0;
    interface->input=input;
    interface->output=output;
    dendrites=output->size.vol*DENDRITES;
    interface->dendrite=malloc(sizeof(Dendrite)*dendrites);
    interface->bias=malloc(sizeof(interface->bias[0])*dendrites);
   
    LOOP(d,dendrites) LOOP(s,SYNAPSES)
    {
        interface->dendrite[d].synapse[s].permanence=LRAND(gseed)&INIT_THRESH;
        interface->dendrite[d].synapse[s].sensitivity=sensitivity?sensitivity:0xff>>(LRAND(gseed)&7); // 2^n, n=1 through 8
    }
    return 0;
}

int Region_init(Region *region,D3 *size)
{
    D3 s;
    if (!region || !ff) return !0;
    BZERO(*region);
    SignalMap_init(&region->cells,size);
    s=size;
    s->z=1; // for sensor map
    SignalMap_init(&region->sensors,s);
    Interface_init(&region->lateral,&region->output,WAS_ACTIVE);
    return 0;
}

int Interface_score(Interface *interface,float *score)
{
    Seed seed;
    D3 ipos,opos;
    fvec delta;
    int dendrite,axis;
    DendriteMap *offset;

    if (!interface || !score) return !0;
    LOOP(axis,3) delta.f[axis]=((float) interface->input.size.v[axis])/((float) interface->output.size.v[axis]);
   
    LOOPD3(opos.v,interface->output.size.v)
    {
        // score synapses
        DIM3V(opos.v,interface->output.size.v); // populate opos.vol
        RESEED(seed,&interface->output.signal[opos.vol]); // reseed when traversing a cell's dendrites
        LOOP(axis,3) ipos.v[axis]=(int) (opos.v[axis]*delta.f[axis]);
        LOOP(dendrite,DENDRITES)
        {
            offset=GDENDRITE(seed);
            LOOP(synapse,SYNAPSES)
            {
                LOOP(a,3) ipos->v[a]+=offset[i].v[a];
                if (DIM3V(ipos->v,interface->input.size.v)>=0 &&
                    dendrite[i].sensitivity & input->signal[ipos.vol] &&
                    dendrite[i].permanence > DIST_THRESH)
                    score[opos.vol]+=input->signal[ipos.vol];
            }
        }

        // suppress neighbors
        RESEED(seed,&interface->output.signal[opos.vol]);
        LOOP(axis,3) ipos.v[axis]=(int) (opos.v[axis]*delta.f[axis]);
        LOOP(dendrite,DENDRITES)
        {
            offset=GDENDRITE(seed);
            LOOP(synapse,SYNAPSES)
            {
                LOOP(a,3) ipos->v[a]+=offset[i].v[a];
                if (DIM3V(ipos->v,interface->input.size.v)>=0)
                    score[ipos.vol] -= score[opos.vol] * ((SYNAPSES-synapse)/SYNAPSES);
            }
        }
    }
    return 0;
}


int Region_update(Region *region)
{
    if (!region) return !0;

    if (!region->bottom_up.size.vol)
    {
        D3 i;
        LOOPD3(i.v,region->cells.output.size.v) region->cells.output->signal[DIM3V(i.v,region->cells.output.size.v)]=getchar();
    }
    else
    {
        D3 cell,cells=region->cells.output.size;
        D3 sensors=region->sensors.output.size;
        D3 feedbacks=region->sensors.output.size;
        int i,x,y,z;
        int active;
        float cell_score[cells.vol];
        float sensor_score[sensors.vol];
        float feedback_score[feedbacks.vol];

        BZERO(cell_score);
        BZERO(sensor_score);
        BZERO(feedback_score);

        Interface_score(&region->bottom_up,sensor_score);
        Interface_score(&region->lateral,cell_score);
        if (feedbacks.vol)
            Interface_score(&region->top_down,feedback_score);

        LOOP(i,cells.vol) if (cell_score[i]*region->lateral.bias[i]>PROX_THRESH)
            region->cells.output->signal[i]|=IS_ACTIVE;

        // column convolution/sparsification
       
        LOOP(i,sensors.vol) if (sensor_score[i]*region->bottom_up.bias[i]>PROX_THRESH)
            region->sensors.output->signal[i]|=IS_ACTIVE;

        // if sensor is active and cells aren't, turn them all on
        LOOP(x,sensors.x) LOOP(y,sensors.y) if (region->sensors.output->signal[DIM3(x,y,1,sensors)]&IS_ACTIVE)
        {
            active=0;
            LOOP(z,cells.z) active|=region->cells.output->signal[cell=DIM3(x,y,z,cells)];
            if (!(active&IS_ACTIVE))
                LOOP(z,cells.z) region->cells.output->signal[cell]|=IS_ACTIVE;
        }
       
        // update synapses

        // display
        glBegin(GL_POINTS);
        LOOPD3(cell,cells) if (region->cells.output->signal[DIM3V(cell,cells)]&IS_ACTIVE) glVertex3iv(cell.v);
        glEnd();
       
        // age inputs
        d=region->output.d;
        LOOP(d[0],i[0]) LOOP(d[1],i[1]) LOOP(d[2],i[2]) region->output.signal[DIM3(i,d)]>>=1;
    }
}


typedef struct
{
    D3 size;
    int lowerlayer; // relative offset from this layer to it's lower-layer
    //D3 ul,lr; // portion of lower layer this layer reads from
} RegionDesc;

typedef struct
{
    int regions;
    Region *region;
} Htm;


int Htm_imit(Htm *htm,RegionDesc *rd,int regions)
{
    int r,ll;
    if (!htm || !regions || !rd) return !0;

    DendriteMap_init();

    htm->regions=regions;
    htm->region=malloc(sizeof(Region)*regions);
    LOOP(r,regions) Region_init(htm->region[r],htm->rd[r].size);
    LOOP(r,regions) if ((ll=rd[r].lowerlayer))
    {
        Interface_init(&htm->region[r].bottom_up,&htm->region[r-ll].cells,&htm->region[r].sensors,IS_ACTIVE);
        Interface_init(&htm->region[r-ll].top_down,&htm->region[r].cells,&htm->region[r-ll].cells,WAS_ACTIVE);
    }
}

int Htm_update(Htm *htm)
{
    int r;
    if (!htm) return !0;
    LOOP(r,htm->regions) Region_update(htm->region[r]);
    cycles++;

}



#include <stdlib.h>
#include <stdio.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/freeglut.h>

#define ABS(x) ((x)<0?(-x):(x))

int gwidth=800,gheight=800;

float camera[] = { 10,-64,20 };
float center[] = { 0,0,10 };
float viewup[] = { 0,0,1 };
float zoom=.20;


int main(int argc, char **argv)
{
    Htm htm;
    RegionDesc rd[]= {
        {{ 16,16,1,0 },0},
        {{ 16,16,4,0 },-1},
        {{ 16,16,4,0 },-1}
    };
       
    Htm_init(&htm,rd,countof(rd));
   
    void draw_dendrite(Seed seed,int syns,cvec *map)
    {
        void draw_cell(float x,float y,float z)
        {
            float scale=.4;
            glVertex3f(x-scale,y-scale,z);
            glVertex3f(x-scale,y+scale,z);
            glVertex3f(x+scale,y+scale,z);
            glVertex3f(x+scale,y-scale,z);
        }

        int i;
        glColor4f(DRAND(seed),DRAND(seed),DRAND(seed),.4);

        glBegin(GL_QUADS);
        LOOP(syns,i) draw_cell(map[i][0],map[i][1],map[i][2]);
        glEnd();

        glDepthMask(GL_FALSE);
        glBegin(GL_LINE_STRIP);
        LOOP(syns,i) glVertex3i(map[i][0],map[i][1],map[i][2]);
        glEnd();
        glDepthMask(GL_TRUE);
    }
   
    void display()
    {
        float x=0,y=0;
        int i=0;
       
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

        glLoadIdentity();
        gluLookAt(camera[0],camera[1],camera[2],
                  center[0],center[1],center[2],
                  viewup[0],viewup[1],viewup[2]);
       
        Htm_display(htm);
       
        glFlush();
        glutSwapBuffers();
    }
 
    void reshape(int w,int h)
    {
        float r=zoom * ((float) w/(float) h);

        gwidth=w;
        gheight=h;
        glViewport(0,0,(GLsizei) w,(GLsizei) h);
        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        if (w>h) glFrustum (-r,r,-zoom,zoom,0.5,200.0);
        else     glFrustum (-zoom,zoom,-r,r,0.5,200.0);

        glMatrixMode(GL_MODELVIEW);
    }
   
    void keyboard(unsigned char key,int x,int y)
    {
        switch (key)
        {
            case 'q': case 27: exit(0); // esc
        }
    }
   
    void mouse(int button,int state,int x,int y) { printf("%d,%d,%d,%d\n",button,state,x,y); }
    void motion(int x,int y)
    {
        /*
          float inclination = 0;
          float azimuth = 0;
         
          translate(width/2, height/2);
          rotateY(azimuth);
          rotateX(inclination);
         
          //redraw
         
          azimuth = (azimuth + 0.01)%TWO_PI;
          inclination = PI*sin(azimuth);
         */

        printf("%d,%d\n",x,y);
    }

    void idle()
    {
        Htm_update(&htm);
        glutPostRedisplay();

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
    glutInitDisplayMode(GLUT_DEPTH | GLUT_DOUBLE | GLUT_ACCUM | GLUT_ALPHA | GLUT_RGBA | GLUT_STENCIL);

    glutInitWindowPosition(100,100);
    glutInitWindowSize(gwidth,gheight);
    glutCreateWindow("HTM");
   
    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    //glutKeyboardFunc(keyboard);
    //glutMouseFunc(mouse);
    //glutMotionFunc(motion);
    //menu();
    //glutAttachMenu(GLUT_RIGHT_BUTTON);
    //glutIdleFunc(idle);

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    glEnable(GL_POINT_SMOOTH);
    glEnable(GL_LINE_SMOOTH);
    glEnable(GL_POLYGON_SMOOTH);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA); // transparency
    //glBlendFunc(GL_SRC_ALPHA_SATURATE,GL_ONE); // back-to-front compositing
    glShadeModel(GL_SMOOTH);

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
    glutSwapBuffers();
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
    glutSwapBuffers();


   
    reshape(gwidth,gheight);
    glutMainLoop();
   
    return 0;
}
