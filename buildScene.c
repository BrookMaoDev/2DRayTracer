#define W_TOP -2.00
#define W_BOTTOM 2.00
// Square image		MUST USE SQUARE for movie rendering
#define W_LEFT -2.00
#define W_RIGHT 2.00
// Widescreen
//#define W_LEFT -2.00*(16.0/9.0)
//#define W_RIGHT 2.00*(16.0/9.0)

void buildWalls(void) {
  struct point2D p,d;
  p.px=W_LEFT;
  p.py=W_TOP;
  d.px=W_RIGHT-W_LEFT;
  d.py=0;
  walls[0].w.p=p;
  walls[0].w.d=d;
  walls[0].material_type=1;
  walls[0].R=1.0;
  walls[0].G=1.0;
  walls[0].B=1.0;
  
  p.px=W_RIGHT;
  p.py=W_TOP;
  d.px=0;
  d.py=W_BOTTOM-W_TOP;
  walls[1].w.p=p;
  walls[1].w.d=d;
  walls[1].material_type=1;
  walls[1].R=1.0;
  walls[1].G=1.0;
  walls[1].B=1.0;

  p.px=W_RIGHT;
  p.py=W_BOTTOM;
  d.px=W_LEFT-W_RIGHT;
  d.py=0;
  walls[2].w.p=p;
  walls[2].w.d=d;
  walls[2].material_type=1;
  walls[2].R=1.0;
  walls[2].G=1.0;
  walls[2].B=1.0;

  p.px=W_LEFT;
  p.py=W_BOTTOM;
  d.px=0;
  d.py=W_TOP-W_BOTTOM;
  walls[3].w.p=p;
  walls[3].w.d=d;
  walls[3].material_type=1;
  walls[3].R=1.0;
  walls[3].G=1.0;
  walls[3].B=1.0;
}

void parseScene(char *name)
{
    // Open the scene specified in scene.txt and parse it to
    // create the objects and lights for the render 
  
    FILE *f;
    char line[1024];
    char *cp;
    double X,Y,dX,dY,r,idx,R,G,B;
    int type;
    struct point2D c,d;
    struct ray2D l;
    
    f=fopen(name,"r");
    if (f==NULL)
    {
      fprintf(stderr,"Unable to open scene description file\n");
      exit(0);
    }
    
    while(fgets(&line[0],1024,f))
    {
      if (line[0]!='#')
      {
        cp=strstr(line,"circle");
        if (cp!=NULL)
        {
          sscanf(cp+6,"%lf %lf %lf %lf %lf %lf %lf %d",&X, &Y, &r, &idx, &R, &G, &B, &type);
          fprintf(stderr,"Adding a circle, with parameters (%lf, %lf), r=%lf, idx=%lf, [%lf, %lf, %lf], %d\n",X,Y,r,idx,R,G,B,type);
          c.px=X;
          c.py=Y;
          addCirc(&c,r,type,idx,R,G,B);
        }
        cp=strstr(line,"light");
        if (cp!=NULL)
        {
          sscanf(cp+5,"%lf %lf %lf %lf %lf %lf %lf %d",&X,&Y,&dX,&dY,&R,&G,&B,&type);
          fprintf(stderr,"Adding a light with parameters c=(%lf, %lf), d=(%lf, %lf), [%lf, %lf, %lf], %d\n",X,Y,dX,dY,R,G,B,type);
          c.px=X;
          c.py=Y;
          d.px=dX;
          d.py=dY;
          l.p=c;
          l.d=d;
          lss[numLights].l=l;
          lss[numLights].light_type=type;
          lss[numLights].R=R;
          lss[numLights].G=G;
          lss[numLights].B=B;
          numLights++;
        }
      }
    }
    fclose(f);
}

void buildScene(void) {
  parseScene(scene_name);
}
