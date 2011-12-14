typedef enum { CLEAN=0,ACTIVE=1,PREDICTING=2,LEARNING=4 } CellState;

int I=24; // input 2d dimension
int F=8;  // input->column fanout
int D=4;  // node root dimension

#define CONNECTED 128

typedef struct
{
    char cell_state[D][D][D]; // xyz 4xlsb=prev, 4xmsb=cur
    char synapse[D][D][D][D][D][D]; // permanence from xyz to xyz
    char col_firing[D][D];
    char col_bias[D][D];
    char col_input[D][D];
} Node;

typedef struct
{
    Node node[D][D];
    short inputMap[I][I][F]; // input xy to F node/colum ids (DxDxDxD)
} Region;



#define LOOP(r,v) for (v=0;v<D;v++)

char cell_step(char state) { return state<<=4; } // prev=cur, cur=0

void region_init(Region *region)
{
}

void region_input(Region *region)
{
    int x,y,cx,cy,f;
    LOOP(D,x) LOOP(D,y)
        BZERO(region->node[x][y].col_input);
   
    LOOP(I,x) LOOP(I,y) LOOP(F,f)
    {
       
    }
}

void region_update(Region *region)
{
}

void region_output(Region *region)
{
}

void main()
{
    Region region;

    region_init(&region);

    while (region_input(&region) &&
           region_update(&region) &&
           region_output(&region));
}
