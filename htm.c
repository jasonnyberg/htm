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

static long long bit1=1;
static long long DMASK[D*D] = { 0xf<<0x0, 0xf<<0x1, 0xf<<0x2, 0xf<<0x3,
                                0xf<<0x4, 0xf<<0x5, 0xf<<0x6, 0xf<<0x7,
                                0xf<<0x8, 0xf<<0x9, 0xf<<0xa, 0xf<<0xb,
                                0xf<<0xc, 0xf<<0xd, 0xf<<0xe, 0xf<<0xf };


#define RAND(bits,mask) ((rand()>>(31-bits)) & (mask))

#define BIT(b) (1<<(b))
#define BIT2D(x,y) (1<<(x)<<D<<(y))
#define BIT3D(x,y,z) (1<<(x)<<D<<(y)<<D<<(z))

#define GETB(bits,mask) ((bits)&(mask))
#define SETB(bits,mask) ((bits)|=(mask))
#define CLRB(bits,mask) ((bits)&=~(mask))

#define SETNIB16(bits,val) ((bits)=(bits)&
#define GETNIB16(bits,val) ((val)=(bits)&DMASK[],(bits)>>D


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
    Bits column_active[TICKS];

    Column column [D][D] [D][D]; // node  (xy)(column(xy))
    Coarse synapse[D][D] [D][D][D] [D][D][D]; // permanence from node(xy)(cell(xyz),cell(xyz))
    unsigned short inputMap[I][I][F]; // input(xy)(fanout) is index into node(xy)(colum(xy))
} Region;

#define LOOP(r,v) for (v=0;v<D;v++)

#define COLUMN(nx,ny,x,y) (region->column[nx][ny][x][y])
#define SYNAPSE(nx,ny,sx,sy,sz,dx,dy,dz) (region->cell[nx][ny][sx][sy][sz][dx][dy][dz])


void region_input(Region *region)
{
    int x,y,fanout,bit;
    char c;
   
    LOOP(D,x) LOOP(D,y) BZERO(region->node[x][y].col_input);
   
    LOOP(I/8,x) LOOP(I,y) LOOP(F,fanout)
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
