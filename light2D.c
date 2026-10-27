/*
  CSC D18 - Assignment 1 - 2D light propagation.
  Starter code: F.J. Estrada, Updated Aug. 2025

  This assignment is intended to help you practice with
  vector and point operations, as well as with simple
  transformations in 2D. 

  You will do this in the context of light propagation
  inside a 2D box that contains objects (circles) with
  different properties - some reflect light like a mirror,
  others scatter light, and yet others behave like
  transparent media.

  Your goal is to write the fundamental components of 
  the light propagation computation, namely:

  - Determining the starting point and direction of rays
  - Determining whether a ray intersects an object or
    hits the wall of the box
  - Whenever a ray hits an object or a wall, bouncing
    or refracting the ray appropriately.

  To complete this assignment you need a solid
  understanding of:

  - Points, vectors, and rays (parametric equation)
  - 2D curves (circles) and their mathematical
    representation
  - 2D transformations (particularly rotation and
    translation)
  
  You will need also to have a solid handle on basic
  C programming with recursion. If you should feel the
  need to brush up on your C programming, do spend some
  time studying on your favorite on-line C programming
  resource.

  ****************************************************
  * You MUST NOT MODIFY THE API PROVIDED. That means
  * you must not change the function definitions 
  * provided by the starter code. You are welcome to
  * add any functions you need, but the standard
  * call to the program should work as per the starter
  ****************************************************

  Note: This project is based on the Tantalum 2D light
        transport project by Benedikt Betterli.
        Really cool stuff. Have a look here:

        https://benedikt-bitterli.me/tantalum/tantalum.html
        (needs a modern browser and a WebGL capable device)
*/

//#define __DEBUG_MODE
#define USE_GL          // This controls whether or not the OpenGL
                        // GUI is active. Comment out to disable
                        // the GUI, compile with the corresponding
                        // script.

#include "light2D.h"	       // READ THIS HEADER FILE CAREFULLY!
#include "buildScene.c"        // This reads and parses the scene description.
                               // DO NOT change the way regular objects or lights
                               // work, but you can add your own types of 
                               // objects or lights, modify the parser to
                               // support your extensions

#include "rays2D.h"     // You also should READ THIS HEADER FILE CAREFULLY
#include "rays2D.c"     // And finally, this will import your solution code

// Let's get some work done!
inline void hue2RGB(double H, double *R, double *G, double *B)
{
  FILE *f;
  static int read=0;
  char line[1024];
  static unsigned char *rt=NULL;

  if (read==0)
  {
   rt=(unsigned char *)calloc(700*72*3,sizeof(unsigned char));
   f=fopen("rainbow.ppm","r");
   if (f==NULL)
   {
    fprintf(stderr,"Unable to open rainbow.ppm\n");
    return;
   }
   fgets(&line[0],1023,f);
   fgets(&line[0],1023,f);
   fgets(&line[0],1023,f);
   fgets(&line[0],1023,f);
   fread(rt,700*72*3*sizeof(unsigned char),1,f);
   fclose(f);
   read=1;
  }

  H=(1.0-H)*700;
  if (H<0) H=0;
  if (H>699) H=699;

  *R=(double)(*(rt+(((int)H)*3)+0))/255.0;
  *G=(double)(*(rt+(((int)H)*3)+1))/255.0;
  *B=(double)(*(rt+(((int)H)*3)+2))/255.0;

  return;
}

void renderObjects(void)
{
 /*
  * Useful for debugging - will overlay the objects in the scene onto the
  * final image in green - turn on/off with the debug flag in rays2D.c
  */
 
 double x,y;
 int xx,yy;
 
 for (int i=0;i<MAX_OBJECTS;i++)
 {
  if (objects[i].r<=0) break;
  for (double ang=0; ang<2*PI; ang+=.001)
  {
   x=objects[i].c.px+(cos(ang)*objects[i].r);
   y=objects[i].c.py+(sin(ang)*objects[i].r);
   x-=W_LEFT;
   y-=W_TOP;
   x=x/(W_RIGHT-W_LEFT);
   y=y/(W_BOTTOM-W_TOP);
   x=x*(sx-1);
   y=y*(sy-1);
   xx=(int)round(x);
   yy=(int)round(y);
   if (xx>=0&&xx<sx&&yy>=0&&yy<sy)
   {
    *(im+((xx+(yy*sx))*3)+0)=0;
    *(im+((xx+(yy*sx))*3)+1)=255;
    *(im+((xx+(yy*sx))*3)+2)=0;   
   }
  }
 }
}

inline void renderRay(struct point2D *p1, struct point2D *p2, double R, double G, double B)
{
 /*
   This function renders a ray onto the image from point p1 to point p2, with the
   specified colour R,G,B
 */

 double x1,y1,x2,y2,xt,yt;
 int xx,yy;
 double dx,dy;
 double inc;
 double pixInc_x,pixInc_y;

 pixInc_x=1.0*(W_RIGHT-W_LEFT)/sx;
 pixInc_y=1.0*(W_TOP-W_BOTTOM)/sy;

 if (p1->px<W_LEFT-TOL||p1->px>W_RIGHT+TOL||p1->py<W_TOP-TOL||p1->py>W_BOTTOM+TOL||\
     p2->px<W_LEFT-TOL||p2->px>W_RIGHT+TOL||p2->py<W_TOP-TOL||p2->py>W_BOTTOM+TOL)
 {
#ifndef __QUIET
  fprintf(stderr,"renderRay() - at least one endpoint is outside the image bounds, somewhere there's an error...\n");
  fprintf(stderr,"p1=(%f,%f)\n",p1->px,p1->py);
  fprintf(stderr,"p2=(%f,%f)\n",p2->px,p2->py);
#endif
  return;
 }
 
 x1=p1->px-W_LEFT+(pixInc_x*(drand48()-.5));
 y1=p1->py-W_TOP+(pixInc_y*(drand48()-.5));
 x2=p2->px-W_LEFT+(pixInc_x*(drand48()-.5));
 y2=p2->py-W_TOP+(pixInc_y*(drand48()-.5));
 
 x1=x1/(W_RIGHT-W_LEFT);
 y1=y1/(W_BOTTOM-W_TOP);
 x2=x2/(W_RIGHT-W_LEFT);
 y2=y2/(W_BOTTOM-W_TOP);

 x1=x1*(sx-1);
 y1=y1*(sy-1);
 x2=x2*(sx-1);
 y2=y2*(sy-1);

 dx=x2-x1;
 dy=y2-y1;
 
 if (abs(dx)>=abs(dy))
 {
  if (x2<x1)
  {
   xt=x1;
   yt=y1;
   x1=x2;
   y1=y2;
   x2=xt;
   y2=yt;
  }
  
   yt=y1;
   inc=(y2-y1)/abs(x2-x1);
   for (double xt=x1; xt<=x2; xt+=1)
   {
    xx=(int)round(xt);
    yy=(int)round(yt);
    if (xx>=0&&xx<sx&&yy>=0&&yy<sy)
    {
      (*(imRGB+((xx+(yy*sx))*3)+0))+=R;
      (*(imRGB+((xx+(yy*sx))*3)+1))+=G;
      (*(imRGB+((xx+(yy*sx))*3)+2))+=B;
    }
    yt+=inc;
   }
      
 }
 else
 {
  if (y2<y1)
  {
   xt=x1;
   yt=y1;
   x1=x2;
   y1=y2;
   x2=xt;
   y2=yt;
  }

   xt=x1;
   inc=(x2-x1)/abs(y2-y1);
   for (double yt=y1; yt<=y2; yt+=1)
   {
    xx=(int)round(xt);
    yy=(int)round(yt);
    if (xx>=0&&xx<sx&&yy>=0&&yy<sy)
    {
      (*(imRGB+((xx+(yy*sx))*3)+0))+=R;
      (*(imRGB+((xx+(yy*sx))*3)+1))+=G;
      (*(imRGB+((xx+(yy*sx))*3)+2))+=B;
    }
    xt+=inc;
   }
   
 }
 
}

inline void setPixel(double x, double y, double R, double G, double B)
{
 /* 
  * This function updates the image so that the location corresponding to (x,y) has
  * the specified colour. It handles conversion of coordinates from scene coordinates
  * to image pixel coordinates, and performs a bit of antialiasing
  */
 
 int xx,yy;
 int ii,jj;
 double W;

 if (R<0||G<0||B<0||R>1||G>1||B>1)
   fprintf(stderr,"Invalid RGB colours passed to setPixel() - image will have artifacts!\n");
 
 // Convert to image coordinates
 x-=W_LEFT;
 y-=W_TOP;
 x=x/(W_RIGHT-W_LEFT);
 y=y/(W_BOTTOM-W_TOP);
 x=x*(sx-1);
 y=y*(sy-1);
  
 xx=(int)round(x);
 yy=(int)round(y);
 
 for (int i=xx-1;i<=xx+1;i++)
   for (int j=yy-1;j<=yy+1;j++)
   {
    W=exp(-(((x-i)*(x-i))+((y-j)*(y-j)))*.5);
    if (i>=0&&j>=0&&i<sx&&j<sy)
    {
     (*(imRGB+((i+(j*sx))*3)+0))+=W*R;
     (*(imRGB+((i+(j*sx))*3)+1))+=W*G;
     (*(imRGB+((i+(j*sx))*3)+2))+=W*B;
    }
   }
}

inline double dot(struct point2D *p, struct point2D *q)
{
 return((p->px*q->px)+(p->py*q->py)); 
}

inline void normalize(struct point2D *d)
{
 double l;
 l=d->px*d->px;
 l+=(d->py*d->py);
 if (l>0)
 {
  l=sqrt(l);
  d->px=d->px/l;
  d->py=d->py/l;
 }
}

void addCirc(struct point2D *c, double r, int type, double r_idx, double R, double G, double B)
{
 // This adds an object to the object array. Parameters specify
 // the circle's center c, the radius r, type of material, and
 // index of refraction (only meaningful for transparent objects)
 static int num_obj=0;
 
 if (num_obj>=MAX_OBJECTS)
 {
  fprintf(stderr,"List of objects is full!\n");
  return;
 }
 objects[num_obj].c=*c;
 objects[num_obj].r=r;
 objects[num_obj].material_type=type;
 objects[num_obj].r_idx=r_idx;
 objects[num_obj].R=R;
 objects[num_obj].G=G;
 objects[num_obj].B=B;
 num_obj++;
}

int main(int argc, char *argv[])
{
 struct ray2D ray;
 struct point2D p,d;
 double mx,mi,rng;
 int hist[256];
 int idx,low,hi,acc;
 int cs,ms;
 double lnorm,hnorm,hrange;
 FILE *f;

 // Parse command line arguments and validate their range 
 if (argc<7)
 {
  fprintf(stderr,"USAGE: light2D max_depth dispersion cutoff_low cutoff_high num_samples scene_name\n");
  fprintf(stderr,"  max_depth - Maximum recursion depth (in [1 25])\n");
  fprintf(stderr,"  dispersion - value between 0 (disabled) and .5\n");
  fprintf(stderr,"  cutoff_low - histogram equalization threshold for dark regions, try values in 0.01 to 0.1\n");
  fprintf(stderr,"  cutoff_high - histogram equalization threshold for bright regions, try values in 0.01 to 0.1\n");
  fprintf(stderr,"  num_samples - maximum number of samples for command-line mode\n");  
  fprintf(stderr,"  scene_name - name of the text file containing the scene description\n");  
  exit(0);
 }
 sx=1024;
 sy=1024;
 num_rays=1000000;
 max_depth=atoi(argv[1]);
 __DISPERSION=atof(argv[2]);
 CUTOFF_L=atof(argv[3]);
 CUTOFF_H=atof(argv[4]);
 ms=atoi(argv[5]);
 strcpy(&scene_name[0],argv[6]);

 fprintf(stderr,"Working with:\n");
 fprintf(stderr,"Image size (%d, %d)\n",sx,sy);
 fprintf(stderr,"Max. recursion depth: %d\n",max_depth);
 fprintf(stderr,"Dispersion=%f\n",__DISPERSION);
 fprintf(stderr,"Cutoff low=%f\n",CUTOFF_L);
 fprintf(stderr,"Cutoff high=%f\n",CUTOFF_H);
 fprintf(stderr,"Num samples=%d\n",ms);
 fprintf(stderr,"Scene name: %s\n",scene_name);
 
 if (max_depth<1||max_depth>25||__DISPERSION<0||__DISPERSION>1.0||CUTOFF_L<.01||CUTOFF_L>.25||CUTOFF_H<.01||CUTOFF_H>.25||ms<1000||ms>100000000)
 {
   fprintf(stderr,"Image size must be in [100,5000]\n");
   fprintf(stderr,"Max depth must be in [1,25]\n");
   fprintf(stderr,"Dispersion must be in [0,1.0]\n");
   fprintf(stderr,"Cutoffs must be in [.01, .25]\n");
   fprintf(stderr,"Num samples must be in [1000,100000000]\n");
   exit(0);
 }

 // Initialize a blank image for our render
 imRGB=(double *)calloc(sx*sy*3,sizeof(double));
 iT=(double *)calloc(sx*sy*3,sizeof(double));
 im=(unsigned char *)calloc(sx*sy*3,sizeof(unsigned char));
 if (imRGB==NULL||iT==NULL||im==NULL)
 {
  fprintf(stderr,"Out of memory?! is this a Commodore 64 you're running on???\n");
  exit(0);
 }

 // Reset walls and objects arrays
 memset(&objects[0],0,MAX_OBJECTS*sizeof(struct circ2D));
 memset(&lss[0],0,MAX_LIGHTS*sizeof(struct light2D));
 memset(&walls[0],0,4*sizeof(struct ray2D));
 for (int i=0;i<10;i++) objects[i].r=-1;	// Make sure we can tell any objects 
						// not added by buildScene() have
						// negative radius!
 
 // Load spectrum data
 f=fopen("spectrum.dat","r");
 if (f==NULL)
 {
  fprintf(stderr,"Unable to load light spectrum data!\n");
  for (int i=0; i<2048*3; i++) spectrum[i]=1.0;
 }
 else
 {
  fread(&spectrum[0],2048*3*sizeof(double),1,f);
  fclose(f);
 }

 // Initialize the walls, scene objects, and lightsource
 buildWalls();
 buildScene();

 // Set image width for coordinate conversion
 w_x=(W_RIGHT-W_LEFT);
 w_y=(W_BOTTOM-W_TOP);

 // If using the OpenGL GUI - at this point the program passes control to the
 // GLUT main loop function and never comes back - otherwise it proceeds with
 // a regular rendering loop.

#ifdef USE_GL
 // READY - The loop below will generate rays in a way that suits the type of lightsource
 // defined in buildScene.c, and will initiate the propagation process.
 glutInit(&argc, argv);
 initGlut("Light2D - OpenGL Mode",1024,1024,10,10);
 glutMainLoop();   // <--- from this point on, GLUT controls the program.
#endif

 // I guess we're not doing OpenGL! oh well... command line it is!
#pragma omp parallel for schedule(dynamic,250) private(ray)
 for (cs=0; cs<ms; cs++)
 {
  if (cs%(ms/10)==0)
  {
   fprintf(stderr,"Rendering, %f %% done!\n",((double)cs/(double)ms)*100);
  }
  ray=makeLightSourceRay();
  propagateRay(&ray,0);
 }

 // Output image
  // First adjust gamma (simple log transform)
 for (int i=0; i<sx*sy*3; i++)
   *(iT+i)=log((*(imRGB+i))+1.5);

 memset(im,0,sx*sy*3*sizeof(unsigned char));
 mx=-1;
 mi=10e15;
 for (int i=0; i<sx*sy*3; i++)
 {
   if (*(iT+i)<mi) mi=*(iT+i);
   if (*(iT+i)>mx) mx=*(iT+i);
 } 
 rng=mx-mi;
 for (int i=0; i<sx*sy*3; i++)
   *(iT+i)=((*(iT+i))-mi)/rng;

 // Histogram and contrast stretching
 memset(&hist[0],0,256*sizeof(int));
 for (int i=0; i<sx*sy*3; i++)
 {
   idx=(int)(255.0*(*(iT+i)));
   if (idx>255) idx=255;
   if (idx<0) idx=0;
   hist[idx]++;
 }
 // Find cutoff points
 low=(int)(CUTOFF_L*(sx*sy*3));
 hi=(int)((1.0-CUTOFF_H)*(sx*sy*3));
 lnorm=-1;
 hnorm=-1;
 acc=0;
 for (int i=0; i<256; i++)
 {
  acc+=hist[i];
  if (acc>=low&&lnorm==-1) lnorm=((double)i)/255.0;
  if (acc>=hi&&hnorm==-1) hnorm=((double)i)/255.0;
 }
 // Contrast stretch
 hrange=hnorm-lnorm;
 for (int i=0; i<sx*sy*3; i++)
 {
   *(iT+i)=((*(iT+i))-lnorm)/hrange;
   if (*(iT+i)>1.0) *(iT+i)=1.0;
   if (*(iT+i)<0.0) *(iT+i)=0.0;
 }

 for (int i=0; i<sx*sy*3; i++)
  *(im+i)=(unsigned char)(255.0*(*(iT+i)));  
  
 f=fopen("Render.ppm","w");
 if (f!=NULL)
 {
    fprintf(f,"P6\n");
    fprintf(f,"# Output from Light2D.c\n");
    fprintf(f,"%d %d\n",sx,sy);
    fprintf(f,"255\n");
    fwrite(im,sx*sy*3*sizeof(unsigned char),1,f);
    fclose(f);
 }
 else fprintf(stderr,"Can not create output image file\n");

 system("mogrify -format jpg -quality 100 Render*.ppm");
 
 // Release resources
 free(imRGB);
 free(im);
 free(iT);
 exit(0);       
}

// OpenGL code below this point - you DON'T need to look through this if you don't
// feel like it. However, you may find it useful to add your own commands to the
// GUI, in which case read through the code below and modify it as needed.
// DOCUMENT all your changes in the report file so we know what to expect

#ifdef USE_GL

void initGlut(const char* winName, int sizeX, int sizeY, int positionX, int positionY)
{
 // This is the GLUT library initialization funtion. GLUT provides a simple
 // API for initializing OpenGL and setting up a window on screen with
 // specified properties.
 //
 // Input arguments: 
 //    winName - Name of the OpenGL window being created (displayed on title bar)
 //    sizeX, sizeY - Window size in pixels
 //    positionX, positionY - Position of the window on the screen

 // Set video mode: double-buffered, color, depth-buffered
 glutInitDisplayMode (GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);

 // Create window
 glutInitWindowSize(sizeX,sizeY);
 glutInitWindowPosition (positionX,positionY);
 windowID = glutCreateWindow(winName);

 // Setup callback functions to handle window-related events.
 // In particular, OpenGL has to be informed of which functions
 // to call when the image needs to be refreshed, and when the
 // image window is being resized.
 glutReshapeFunc(WindowReshape);   	// Call WindowReshape() whenever window resized
 glutDisplayFunc(WindowDisplay);   	// Call WindowDisplay() whenever new frame needed
 glutKeyboardFunc(kbHandler);       // Keyboard handler function
 glutMouseFunc(mouseClickHandler);
 glutMotionFunc(mouseDragHandler);
}

void mouseClickHandler(int button, int state, int x, int y)
{
  // Handle mouse clicking for the GUI
  double bd=1e6;
  double dx,dy,d;
  double wx,wy;         // World coordinates!
  
  wx=W_LEFT+(((double)x/(double)sx)*(W_RIGHT-W_LEFT));
  wy=W_TOP+(((double)y/(double)sy)*(W_BOTTOM-W_TOP));
  
  if (mouseState==0&&button==0)
  {
    bd=1e6;
    // Click Down, left button
    // Pick an object to drag
    dragObjectIndex=-1;
    // First check all circles
    for (int i=0; i<MAX_OBJECTS; i++)
    {
      if (objects[i].r<=0) break;
      // It's enough to be within the circle, if overlap, picks the last one
      dx=wx-objects[i].c.px;
      dy=wy-objects[i].c.py;
      d=sqrt((dx*dx)+(dy*dy));
      if (d<bd&&(d<objects[i].r||d<=.1))
      {
        bd=d;
        dragObjectType=0;
        dragObjectIndex=i;
      }
    }

    // Now check for lights
    for (int i=0; i<numLights; i++)
    {
      // Needs to be within distance from the light's center)
      dx=wx-lss[i].l.p.px;
      dy=wy-lss[i].l.p.py;
      d=sqrt((dx*dx)+(dy*dy));
      if (d<lightDistThresh&&d<bd)
      {
        bd=d;
        dragObjectType=1;
        dragObjectIndex=i;
      }
    }
      
    prevX=wx;
    prevY=wy;
    mouseState=1;
  }
  else if (mouseState==1&&button==0)
  {
    // Click Up, left button
    mouseState=0;
    bd=1e6;
    dragObjectIndex=-1;
  }
  else if (mouseState==0&&button==2)
  {
    // Now check for lights
    bd=1e6;
    dragObjectIndex=-1;
    for (int i=0; i<numLights; i++)
    {
      // Needs to be within distance from the light's center)
      dx=wx-lss[i].l.p.px;
      dy=wy-lss[i].l.p.py;
      d=sqrt((dx*dx)+(dy*dy));
      if (d<lightDistThresh&&d<bd)
      {
        bd=d;
        dragObjectType=1;
        dragObjectIndex=i;
      }
    }
    prevX=wx;
    prevY=wy;
    mouseState=2;    
  }
  else if (mouseState==2&&button==2)
  {
    mouseState=0;
    bd=1e6;
    dragObjectIndex=-1;
  }
  
}

void mouseDragHandler(int x, int y)
{
  // Handle mouse dragging for the GUI
  
  double dx,dy,d;
  double wx,wy;         // World coordinates!
  
  wx=W_LEFT+(((double)x/(double)sx)*(W_RIGHT-W_LEFT));
  wy=W_TOP+(((double)y/(double)sy)*(W_BOTTOM-W_TOP));
  
  if (mouseState==1&&dragObjectIndex>=0)
  {
    dx=wx-prevX;
    dy=wy-prevY;
    // Currently dragging
    if (dragObjectType==0)
    {
      // Dragging a circle
      if (objects[dragObjectIndex].c.px+dx>W_LEFT&&objects[dragObjectIndex].c.px+dx<W_RIGHT)
        objects[dragObjectIndex].c.px+=dx;
      if (objects[dragObjectIndex].c.py+dy>W_TOP&&objects[dragObjectIndex].c.py+dy<W_BOTTOM)
        objects[dragObjectIndex].c.py+=dy;
    }
    else
    {
      // Dragging a light!
      if (lss[dragObjectIndex].l.p.px+dx>W_LEFT&&lss[dragObjectIndex].l.p.px+dx<W_RIGHT)
        lss[dragObjectIndex].l.p.px+=dx;
      if (lss[dragObjectIndex].l.p.py+dy>W_TOP&&lss[dragObjectIndex].l.p.py+dy<W_BOTTOM)
        lss[dragObjectIndex].l.p.py+=dy;
    }
    memset(imRGB,0,sx*sy*3*sizeof(double));
    prevX=wx;
    prevY=wy;
  }
  else if (mouseState==2&&dragObjectIndex>=0&&dragObjectType==1)
  {
    dx=wx-lss[dragObjectIndex].l.p.px;
    dy=wy-lss[dragObjectIndex].l.p.py;
    d=sqrt((dx*dx)+(dy*dy));
    dx=dx/d;
    dy=dy/d;
    lss[dragObjectIndex].l.d.px=dx;
    lss[dragObjectIndex].l.d.py=dy;    
    memset(imRGB,0,sx*sy*3*sizeof(double));
  }
}

void kbHandler(unsigned char key, int x, int y)
{
 FILE *f;

 if (key=='+')
 {
   if (max_depth<25) max_depth++;
   memset(imRGB,0,sx*sy*3*sizeof(double));
 }
 if (key=='-')
 {
   if (max_depth>1) max_depth--;
   memset(imRGB,0,sx*sy*3*sizeof(double));
 } 
 if (key=='q') {
   // Here goes all the end-program stuff
   f=fopen("Render.ppm","w");
   if (f!=NULL)
   {
    fprintf(f,"P6\n");
    fprintf(f,"# Output from Light2D.c\n");
    fprintf(f,"%d %d\n",sx,sy);
    fprintf(f,"255\n");
    fwrite(im,sx*sy*3*sizeof(unsigned char),1,f);
    fclose(f);
   }
   else fprintf(stderr,"Can not create output image file\n");

  system("mogrify -format jpg -quality 100 Render*.ppm");
 
  // Release resources
  free(imRGB);
  free(im);
  free(iT);
  exit(0);   
 } 
}

void WindowReshape(int width, int height)
{
 /*
   This function is called whenever the window is resized. It takes care of setting up
   OpenGL world-to-pixel-coordinate conversion matrices so that the image content is
   properly displayed regardless of window size.

   The width and height are provided by OpenGL's GLUT library upon the window being resized
 */

 // Setup projection matrix for new window
 glMatrixMode(GL_PROJECTION);
 glLoadIdentity();
 gluOrtho2D(0, 1024, 1024, 0);
 glViewport(0,0,width,height);
 glutPostRedisplay();
}

void WindowDisplay(void)
{
 /*
    This function is called whenever the frame needs to be updated. That means
    once per each iteration of the glutMainLoop, as well as anytime that the
    window is resized.

	Unfortunately, GLUT's display function prototype allows no arguments! that means
	any data initialized by main() and needed for the game has to be made global
    	or we would not be able to use it here. Apologies to Anya.
 */
 char line[1024];
 static int frame=0;
 static GLuint texture;
 struct ray2D ray;
 double mx,mi,rng;
 int hist[256];
 int idx,low,hi,acc;
 double lnorm,hnorm,hrange;
 static double scale_factor=1.0;

 /* Trace a few rays and update the window */
#ifndef __DEBUG_MODE
#pragma omp parallel for schedule(dynamic,10) private(ray) 
 for (int i=0; i<12*10; i++)           // Adjust for the number of cores!
#else
 for (int i=0; i<1; i++)           // Adjust for the number of cores!
#endif
 {
  ray=makeLightSourceRay();
  propagateRay(&ray,0);
 }
  // First adjust gamma (simple log transform)
 for (int i=0; i<sx*sy*3; i++)
   *(iT+i)=log((*(imRGB+i))+1.5);

 memset(im,0,sx*sy*3*sizeof(unsigned char));
 mx=-1;
 mi=10e15;
 for (int i=0; i<sx*sy*3; i++)
 {
   if (*(iT+i)<mi) mi=*(iT+i);
   if (*(iT+i)>mx) mx=*(iT+i);
 } 
 rng=mx-mi;
 for (int i=0; i<sx*sy*3; i++)
   *(iT+i)=((*(iT+i))-mi)/rng;

 // Histogram and contrast stretching
 memset(&hist[0],0,256*sizeof(int));
 for (int i=0; i<sx*sy*3; i++)
 {
   idx=(int)(255.0*(*(iT+i)));
   if (idx>255) idx=255;
   if (idx<0) idx=0;
   hist[idx]++;
 }
 // Find cutoff points
 low=(int)(CUTOFF_L*(sx*sy*3));
 hi=(int)((1.0-CUTOFF_H)*(sx*sy*3));
 lnorm=-1;
 hnorm=-1;
 acc=0;
 for (int i=0; i<256; i++)
 {
  acc+=hist[i];
  if (acc>=low&&lnorm==-1) lnorm=((double)i)/255.0;
  if (acc>=hi&&hnorm==-1) hnorm=((double)i)/255.0;
 }
 // Contrast stretch
 hrange=hnorm-lnorm;
 for (int i=0; i<sx*sy*3; i++)
 {
   *(iT+i)=((*(iT+i))-lnorm)/hrange;
   if (*(iT+i)>1.0) *(iT+i)=1.0;
   if (*(iT+i)<0.0) *(iT+i)=0.0;
 }

 for (int i=0; i<sx*sy*3; i++)
  *(im+i)=(unsigned char)(255.0*(*(iT+i)));  
 /***** End of window state update ****/
 
 /***** Scene drawing start *********/

 glClearColor(0.01f,0.01f,0.01f,1.0f);
 glDisable(GL_BLEND);
 glDisable(GL_LIGHTING);
 glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

 glMatrixMode(GL_MODELVIEW);
 glLoadIdentity();

 // Set up texture only the first time through this function
 glEnable(GL_TEXTURE_2D);
 if (frame==0)
 {
  glGenTextures( 1, &texture);
  glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
  glBindTexture( GL_TEXTURE_2D, texture);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexEnvf( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_DECAL);
  glTexImage2D (GL_TEXTURE_2D, 0, GL_RGB, 1024, 1024, 0, GL_RGB, GL_UNSIGNED_BYTE, im);
  frame++;
 }
 else	// Afterwards just update texture - significantly faster
 {
  glBindTexture(GL_TEXTURE_2D, texture);
  glTexSubImage2D(GL_TEXTURE_2D,0,0,0,1024,1024,GL_RGB,GL_UNSIGNED_BYTE, im);
  frame++;
 }
 // Single quad polygon canvas taking up the whole image
 glBegin (GL_QUADS);
 glTexCoord2f (0.0, 0.0);
 glVertex3f (0.0, 0.0, 0.0);
 glTexCoord2f (scale_factor, 0.0);
 glVertex3f (1024.0, 0.0, 0.0);
 glTexCoord2f (scale_factor, scale_factor);
 glVertex3f (1024.0, 1024.0, 0.0);
 glTexCoord2f (0.0, scale_factor);
 glVertex3f (0.0, 1024.0, 0.0);
 glEnd ();
    
 glFlush();
 glutSwapBuffers();

 /***** Scene drawing end ***********/

 // Tell glut window to update itself
 glutSetWindow(windowID);
 glutPostRedisplay();

 // Wait for enter so we can debug...
#ifdef __DEBUG_MODE
 fgets(&line[0],1023,stdin);
 memset(imRGB,0,sx*sy*3*sizeof(double));
#endif
 
}

#endif        // for the USE_GL compiler def block
