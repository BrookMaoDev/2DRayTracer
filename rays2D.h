/*
  CSC D18 - Assignment 1 - 2D light propagation

  This header file provides the function prototypes for the critical functions
  you will be implementing for your assignment. You do not need to change the
  header file. 

  Starter by: F.J. Estrada, Updated Aug. 2035
*/
#ifndef __rays2D_header

#define __rays2D_header

// Viewport window size defined by the top, bottom, left, and right distances
#define W_TOP -2.00
#define W_BOTTOM 2.00
#define W_LEFT -2.00
#define W_RIGHT 2.00

struct ray2D makeLightSourceRay(void);		  // Returns a ray that would be emitted
                                              // by the lightsource as defined in
                                              // buildScene.c
void propagateRay(struct ray2D *ray, int depth);  // Carries out the propagation process
                                                  // for the ray.

// The function below finds the intersection between a ray and objects/walls in the scene,
// then it returns the *closest* intersection point in p, the normal at the intersection
// in n, the value of lambda at the intersection, and the object's material type. 
// Now with material colour!
void intersectRay(struct ray2D *ray, struct point2D *p, struct point2D *n, double *lambda, int *type, double *r_idx, double *R, double *G, double *B);

#endif
