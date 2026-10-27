/*
  CSC D18 - Assignment 1 - 2D light propagation
  
  Have a look below and be sure you understand the
  different data structures and functions provided
  to help you complete your work.
  
  By F. J. Estrada, Updated Aug. 2025
*/

#ifndef __light2D_header
#define __light2D_header

#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<math.h>

#ifdef USE_GL
 // Open GL headers - must be installed on the system's include dirs.
 #include <GL/gl.h>
 #include <GL/glu.h>
 #include <GL/glut.h>
#endif

#define PI 3.1415926535
#define TOL .000001
#define MAX_OBJECTS 1000
#define MAX_LIGHTS 1000

/*****************************************************************************************************
* Useful Data Structures
* ***************************************************************************************************/
// A 2D point or vector
struct point2D{
  double px;
  double py;
};

// This holds a 2D ray using the parametric representation
struct ray2D{
  struct point2D p;	        // Ray origin
  struct point2D d;	        // Ray direction
  double R,G,B;		        // Colour of this light ray
  int inside_out;	        // Toggle flag
                            	//   0 - ray is travelling in air (outside an object)
                            	//   1 - ray is travelling inside an object
  int monochromatic;	    	// Flag to indicate whether this ray is monochromatic (true if set to 1)
  double H;		        // For monochromatic rays, HUE value (used to obtain colour, and as
                            	//   a convenient substitute for wavelength) values in [0 1] go
                            	//   from deep red to purple
};

// Holds information about 1 object (a circle)
struct circ2D{
  struct point2D c;	        // Center location
  double r;		        // Circle radius
  int material_type;	    	// Type of object:
                            	//   0 - Mirror
                            	//   1 - Scattering
                            	//   2 - Refracting (transparent)
  double r_idx;		        // Index of refraction for transparent materials
  double R,G,B;             	// Object colour
};

// Holds information about one of the 4 walls
struct wall2D{
  struct ray2D w;	        // The actual wall is just a ray, the origin is one endpoint, and origin + d
                            	// gives the second endpoind (i.e. with lambda=1)
  int material_type;	    	// Type of material, can only be 0 or 1 with the same meaning as for circles
  double R,G,B;             	// Wall colour
};

// Structure to hold information about a light source
struct light2D{
  struct ray2D l;	        // Contains the location and if needed the direction of the
                            	// lightsource
  int light_type;	        // Type of light source
                            	// 0 - Point. Only the location matters, ignore the direction
                            	// 1 - Directional. Emits light exclusively along a single direction
                            	//     from its location.
  double R,G,B;		        // Colour of this light source
};

/***********************************************************************************************************
* GLOBAL DATA (yes, it's actually useful!)
************************************************************************************************************/
double *imRGB, *iT;				// Pointer to the array where the image will be rendered
unsigned char *im;			        // Final image for storage
int sx,sy;				        // Image resolution
int max_depth;				        // Maximum recursion depth
int num_rays;				        // Number of light-source rays to propagate
struct circ2D objects[MAX_OBJECTS];		// Array to hold scene objects (up to MAX_OBJECTS)
                                    		// Empty entries are initialized in main() to nonsense values!
struct wall2D walls[4];			    	// Four walls stored in order TOP, RIGHT, BOTTOM, LEFT
struct light2D lss[MAX_LIGHTS];     		// An array for light sources
struct light2D lightsource;			// Holds one light source
int numLights=0;                    		// Number of lightsources
double w_x,w_y;
double CUTOFF_H,CUTOFF_L;			// Cutoffs for image normalization
double __DISPERSION=0;				// Dispersion amount
double spectrum[2048*3];			// Array to hold light spectrum data
char scene_name[1024];				// Name of the scene file

#ifdef USE_GL
 int windowID;
 int mouseState=0;                  // 0-unclicked, 1-clicked left, 2-clicked right
 int mouseButton=0;                 // Which mouse button was clicked
 int dragObjectType;                // It's 0 if it is a circle, it is 1 if light
 int dragObjectIndex=-1;            // Index of the object being dragged
 double lightDistThresh=.25;        // Distance threshold for picking lights (in world coordinates)
 double prevX,prevY;                // Need to keep track of the previous location in world coordinates
#endif

/************************************************************************************************************
* Function definitions
*************************************************************************************************************/
void hue2RGB(double H, double *R, double *G, double *B);		    // Returns colour for a given hue
void renderObjects(void);						                    // Renders objects onto image

// This function renders a ray onto the image between the two given points, with the specified RGB colour
void renderRay(struct point2D *p1, struct point2D *p2, double R, double G, double B);
void setPixel(double x, double y, double R, double G, double B);	// Set image pixel at (x,y) to the 
                                                                    // specified colour.
double dot(struct point2D *p, struct point2D *q);	                // Dot product between vectors p and q 
void normalize(struct point2D *d);			                        // Normalize a vector to unit length

// Add one object to the scene - it goes into the next empty location in the objects array
void addCirc(struct point2D *c, double r, int type, double n, double R, double G, double B);    
void buildWalls(void);		// This function makes the walls of the box. Implemented in buildScene.c
void buildScene(void);		// This creates the list of objects and sets up the
                            // light source. Implemented in buildScene.c, so you can
                            // change the scene by updating buildScene.c accordingly

#ifdef USE_GL
 // Open GL Initialization functions
 void initGlut(const char *winName, int sizeX, int sizeY, int positionX, int positionY);
 // Open GL callbacks for handling events in glut
 void WindowReshape(int w, int h);
 void WindowDisplay(void);
 void kbHandler(unsigned char key, int x, int y);
 void mouseClickHandler(int button, int state, int x, int y);
 void mouseDragHandler(int x, int y);
#endif
 
#endif

