/*
  CSC D18 - Assignment 1 - 2D light propagation

  This is the place where you will be doing most of your work for solving this
  assignment. Before you start working here, you shold have become familiar
  with the data structures and functions available to you from light2D.h, and
  with the way a scene is built in buildScene.c

  Go over each of the functions below, and implement the different components
  of the solution in the sections marked

  /************************
  / TO DO:
  ************************ /

  Do not add or modify code outside these sections.

  Details about what needs to be implemented are described in the comments, as
  well as in the assignment handout. You must read both carefully.

  Starter by: F.J. Estrada, Updated Aug. 2025
*/

/****************************************************************************
 * Uncomment the #define below to enable debug code, add whatever you need
 * to help you debug your program between #ifdef - #endif blocks
 * ************************************************************************/
#include "rays2D.h"
#include "light2D.h"
#include <math.h>
#include <stdlib.h>

struct ray2D makeLightSourceRay(void) {
  /*
    This function should return a light ray that has its origin at the light
    source, and whose direction depends on the type of light source.

    For point light sources (which emit light in all directions) the direction
     has to be chosen randomly and uniformly over a unit circle (i.e. any
     direction is equally likely)

    For a laser type light source, the direction of the ray is exactly the same
     as the direction of the lightsource.

    Set the colour of the ray to the same colour as the light source, and
     set the inside_outside flag to 0 (we assume light sources are
     outside objects)

    In either case, the direction vector *must be unit length*.
 */

  /************************************************************************
   *  TO DO: Complete this function so that we can sample rays from the
   *         lightsource for the light propagation process.
   ************************************************************************/

  /** IMPORTANTLY:
   * Without a proper spectral distribution function, it's not possible
   * to get coloured rays from a lightsource. So, work with white light
   * sources, set the MONOCHROMATIC flag to 0, and only create
   * monochromatic rays when a ray from the lightsource hits an object
   * that is refractive, and __DISPERSION is NOT zero.
   *
   * i.e.   Rays from this function will all be monochromatic
   *        Once you hit a refractive object, choose a colour
   *        randomly from the colour spectrum, and create a
   *        monochromatic ray with that colour
   *
   * If you have coloured light sources, set the ray colour here to be
   * the light source's colour, but you need an SPD to determine how
   * to sample monochromatic rays from this source
   **/

  struct ray2D ray = {0};

  if (numLights <= 0)
    return ray;

  // Pick a random light.
  double light_fraction = (double)rand() / (RAND_MAX + 1.0);
  int light_index = (int)(light_fraction * numLights);
  struct light2D *source = &lss[light_index];

  ray.p = source->l.p;

  if (source->light_type == 0) {
    // Point
    double theta = 2.0 * PI * rand() / (RAND_MAX + 1.0);
    ray.d.px = cos(theta);
    ray.d.py = sin(theta);
  } else {
    // Directional
    ray.d = source->l.d;
    normalize(&ray.d);
  }
  // Compensate for sampling one of numLights sources per ray.
  ray.R = source->R * numLights;
  ray.G = source->G * numLights;
  ray.B = source->B * numLights;
  ray.inside_out = 0;
  ray.monochromatic = 0;
  ray.H = 0;

  return (ray);
}

void propagateRay(struct ray2D *ray, int depth) {
  /*
    This function carries out the light propagation process. It is provided with
    access to a ray data structure, and must perform the following steps (in
    order!):

    - Check if maximum recursion depth has been reached (in which case, it just
    returns)
    - Find the *closest* intersection between the ray and objects in the scene.
    This means you have to check against the 4 walls, and any circles added in
    buildScene, determine which intersection is closest, and obtain the
    intersection point, the normal at the intersection, and the lambda value at
    which the intersection happens.
    - Renders the ray onto the image from its starting point all the way up to
    the intersection point.
    - At the intersection, use the material properties to determine how the
    propagation process proceeds:
          * For mirror materials, compute the mirror reflection direction,
    create a ray along that direction whose origin is the intersection point,
    and propagate that ray
          * For scattering materials, choose a random direction within +- 90
    degrees of the normal, create a ray with that direction with origin at the
    intersection point, and propagate that ray
          * For refracting materials you will need to propagate two rays - one
    in the mirror reflection direction (just like for reflecting materials), and
            another in the refraction direction. Propagate both rays one after
    the other!

    NOTE: You should only care about intersections for which lambda is POSITIVE
    (in front of the ray), and greater than zero (e.g. if the ray is reflected
    from some object, you do not care about the intersection with the object
    itself which will have a lambda of *very close* to zero)

    In every case, make sure the ray's direction vector has unit length. You
    will need to complete other functions as part of your work here.
 */

  /*********************************************************************************
   * TO DO: Complete this function to implement light propagation!
   ********************************************************************************/

  if (depth >= max_depth)
    return;

  // Steps
  //  a) Find intersections with walls and objects ( see intersectRay() )
  //     keeping the closest intersection
  struct point2D intersection_point;
  struct point2D surface_normal;
  double intersection_lambda = 0.0; // Zero means no hit was found.
  int material_type = 0;
  double refractive_index = 0.0;
  double material_R = 0.0;
  double material_G = 0.0;
  double material_B = 0.0;

  intersectRay(ray, &intersection_point, &surface_normal, &intersection_lambda,
               &material_type, &refractive_index, &material_R, &material_G,
               &material_B);
  if (intersection_lambda <= 0.0)
    return;

  //  b) Determine the type of bounce for the ray based on the material for the
  //     wall/object it hit
  int is_mirroring = (material_type == 0);
  int is_scattering = (material_type == 1);
  int is_refracting = (material_type == 2);

  //  c) Render a ray from its origin to the intersection, with the right colour
  //     see renderRay() - ensure the endpoints are within the image!
  renderRay(&ray->p, &intersection_point, ray->R, ray->G, ray->B);

  //  d) Determine the direction of the new ray, as dictated by the bounce
  struct ray2D outgoing_ray = *ray;
  outgoing_ray.p = intersection_point;

  if (is_mirroring || is_refracting) {
    // Both materials create a reflected ray with this same geometry.
    double direction_dot_normal =
        ray->d.px * surface_normal.px + ray->d.py * surface_normal.py;
    outgoing_ray.d.px =
        ray->d.px - 2.0 * direction_dot_normal * surface_normal.px;
    outgoing_ray.d.py =
        ray->d.py - 2.0 * direction_dot_normal * surface_normal.py;
    normalize(&outgoing_ray.d);
    outgoing_ray.R *= material_R;
    outgoing_ray.G *= material_G;
    outgoing_ray.B *= material_B;
  } else if (is_scattering) {
    // Choose a scatter angle uniformly from [-PI/2, PI/2].
    double random_fraction = (double)rand() / (double)RAND_MAX;
    double scatter_angle = PI * (random_fraction - 0.5);

    // A 2D rotation by angle a uses this matrix:
    // [rotated_x]   [cos(a) -sin(a)] [normal_x]
    // [rotated_y] = [sin(a)  cos(a)] [normal_y]
    // Apply the two row equations directly to the normal vector.
    double cosine = cos(scatter_angle);
    double sine = sin(scatter_angle);
    outgoing_ray.d.px = surface_normal.px * cosine - surface_normal.py * sine;
    outgoing_ray.d.py = surface_normal.px * sine + surface_normal.py * cosine;
    normalize(&outgoing_ray.d);
    outgoing_ray.R *= material_R;
    outgoing_ray.G *= material_G;
    outgoing_ray.B *= material_B;
  }

  // Refraction creates another ray, calculated separately from reflection.
  if (is_refracting) {
    // Flip the normal so it points in the incoming ray direction.
    double incoming_dot_normal =
        ray->d.px * surface_normal.px + ray->d.py * surface_normal.py;
    if (incoming_dot_normal < 0.0) {
      surface_normal.px = -surface_normal.px;
      surface_normal.py = -surface_normal.py;
      incoming_dot_normal = -incoming_dot_normal;
    }

    const double air_refractive_index = 1.0;
    double n1 = ray->inside_out ? refractive_index : air_refractive_index;
    double n2 = ray->inside_out ? air_refractive_index : refractive_index;
    // Both vectors are unit length, so their dot product is cos(theta1).
    double cos_theta1 = incoming_dot_normal;
    // ts should not happen: cos(theta1) should be in [0, 1].
    if (cos_theta1 < 0.0)
      cos_theta1 = 0.0;
    if (cos_theta1 > 1.0)
      cos_theta1 = 1.0;

    // Part 1: use Snell law to find the refracted angle and direction.
    double theta1 = acos(cos_theta1);
    double sin_theta2 = (n1 / n2) * sin(theta1);
    // If sin(theta2) is over 1, no real refracted angle exists.
    // This is total internal reflection.
    int can_refract = (sin_theta2 <= 1.0);
    struct ray2D refracted_ray;

    if (can_refract) {
      double theta2 = asin(sin_theta2);
      double cosine = cos(theta2);
      double sine = sin(theta2);

      // Rotate the normal both ways; the larger dot product is closer
      // to the incoming ray.
      double clockwise_x =
          surface_normal.px * cosine + surface_normal.py * sine;
      double clockwise_y =
          -surface_normal.px * sine + surface_normal.py * cosine;
      double counterclockwise_x =
          surface_normal.px * cosine - surface_normal.py * sine;
      double counterclockwise_y =
          surface_normal.px * sine + surface_normal.py * cosine;

      double clockwise_dot = clockwise_x * ray->d.px + clockwise_y * ray->d.py;
      double counterclockwise_dot =
          counterclockwise_x * ray->d.px + counterclockwise_y * ray->d.py;

      refracted_ray = *ray;
      refracted_ray.p = intersection_point;
      if (clockwise_dot > counterclockwise_dot) {
        refracted_ray.d.px = clockwise_x;
        refracted_ray.d.py = clockwise_y;
      } else {
        refracted_ray.d.px = counterclockwise_x;
        refracted_ray.d.py = counterclockwise_y;
      }
      normalize(&refracted_ray.d);
      refracted_ray.inside_out = !ray->inside_out;
    }

    // Part 2: use Schlick approximation for reflected and transmitted shares.
    double R0 = (n1 - n2) / (n1 + n2);
    R0 *= R0;
    double Rs = 1.0;
    double Rt = 0.0;
    if (can_refract) {
      Rs = R0 + (1.0 - R0) * pow(1.0 - cos_theta1, 5.0);
      Rt = 1.0 - Rs;
    }

    // Apply the reflected share. Rs is 1 during total internal reflection.
    outgoing_ray.R *= Rs;
    outgoing_ray.G *= Rs;
    outgoing_ray.B *= Rs;

    if (can_refract) {
      // With dispersion enabled, choose a spectrum color for a white ray.
      if (__DISPERSION != 0.0 && !ray->monochromatic) {
        refracted_ray.H = (double)rand() / (double)RAND_MAX;
        refracted_ray.monochromatic = 1;
        hue2RGB(refracted_ray.H, &refracted_ray.R, &refracted_ray.G,
                &refracted_ray.B);
      }
      refracted_ray.R *= Rt * material_R;
      refracted_ray.G *= Rt * material_G;
      refracted_ray.B *= Rt * material_B;

      propagateRay(&refracted_ray, depth + 1);
    }
  }

  //  e) Recursively propagate the reflected or scattered ray.
  propagateRay(&outgoing_ray, depth + 1);

  return;
}

void intersectRay(struct ray2D *ray, struct point2D *p, struct point2D *n,
                  double *lambda, int *type, double *r_idx, double *tR,
                  double *tG, double *tB) {
  /*
   This function checks for intersection between the ray and any objects in the
   objects array. The objects are circles, so we are in fact solving for the
   intersection between a ray and a circle.

   For a unit circle centered at the origin, we would have the equation

   x^2 + y^2 = 1

   Using vector notation, with C=[x y]', we get

   ||C||^2 = 1

   A point on the ray is given by p + lambda*d

   Substituting in the equation for the circle we have

   (p + lambda*d)(p + lambda*d) - 1 = 0

   If we expand the product above (here the product of two vectors is a DOT
   product), we can form a quadratic equation

   A*lambda^2 + B*lambda + C = 0

   Which as you know, has a very simple solution.

   Your task is to
   * Figure out A, B, and C, considering that your circles don't necessarily
   have r=1
   * Figure out how to deal with the fact that circles in the scene are likely
     *not* centered at the origin

   Then implement the code that will find the value(s) of lambda at the
   intersection(s).

   Note that you can have no intersections, 1 intersection, or 2 intersections

   This function *must* find the closest intersection (if any) and update the
   value of lambda, the intersection point p, the normal n at the intersection,
   the corresponding object's material type (needed outside here to figure out
   how to handle the light's bouncing off this object), and the index of
   refraction for the object (needed if this is a transparent object).

   You must make sure that n is a unit-length vector.
  */

  ////////////////////////////////////////////////////////////////////////
  // TO DO - complete this function
  //   Notice there is NO return value, the parameters are updated directly
  //   using the pointers passed into the function
  ////////////////////////////////////////////////////////////////////////

  double closest_lambda = HUGE_VAL;
  const double air_refractive_index = 1.0;
  struct point2D box_center;
  box_center.px = (W_LEFT + W_RIGHT) / 2.0;
  box_center.py = (W_TOP + W_BOTTOM) / 2.0;

  for (int wall_index = 0; wall_index < 4; wall_index++) {
    struct point2D wall_start = walls[wall_index].w.p;
    struct point2D wall_direction = walls[wall_index].w.d;

    // The ray point is ray->p + ray_lambda * ray->d.
    // The wall point is wall_start + wall_fraction * wall_direction.
    // At an intersection, set these equal and move the unknowns to the left:
    //   ray_lambda * ray->d - wall_fraction * wall_direction
    //       = wall_start - ray->p
    // Let D=ray->d, S=wall_direction, O=ray->p, and Q=wall_start.
    // The system is M * [ray_lambda, wall_fraction] = b:
    //   [D.px  -S.px] [ray_lambda   ] = [Q.px - O.px]
    //   [D.py  -S.py] [wall_fraction]   [Q.py - O.py]
    // Use Cramer rule: each unknown is the determinant with its column
    // replaced by b, divided by the determinant of M.
    double determinant =
        ray->d.px * (-wall_direction.py) - (-wall_direction.px) * ray->d.py;
    if (fabs(determinant) <= TOL)
      continue; // Parallel: no unique intersection.

    // Replace the first column of M by b to solve for ray_lambda.
    double lambda_determinant =
        (wall_start.px - ray->p.px) * (-wall_direction.py) -
        (-wall_direction.px) * (wall_start.py - ray->p.py);
    double ray_lambda = lambda_determinant / determinant;

    // Replace the second column of M by b to solve for wall_fraction.
    double fraction_determinant = ray->d.px * (wall_start.py - ray->p.py) -
                                  (wall_start.px - ray->p.px) * ray->d.py;
    double wall_fraction = fraction_determinant / determinant;

    // The hit must be in front of the ray and on the finite wall segment.
    if (ray_lambda <= TOL || wall_fraction < -TOL ||
        wall_fraction > 1.0 + TOL || ray_lambda >= closest_lambda) {
      continue;
    }

    closest_lambda = ray_lambda;
    p->px = ray->p.px + ray_lambda * ray->d.px;
    p->py = ray->p.py + ray_lambda * ray->d.py;

    // A perpendicular to the wall is a normal. Point it toward the box
    // interior.
    n->px = -wall_direction.py;
    n->py = wall_direction.px;
    normalize(n);
    double wall_mid_x = wall_start.px + 0.5 * wall_direction.px;
    double wall_mid_y = wall_start.py + 0.5 * wall_direction.py;
    if (n->px * (box_center.px - wall_mid_x) +
            n->py * (box_center.py - wall_mid_y) <
        0.0) {
      n->px = -n->px;
      n->py = -n->py;
    }

    *lambda = ray_lambda;
    *type = walls[wall_index].material_type;
    *r_idx = air_refractive_index;
    *tR = walls[wall_index].R;
    *tG = walls[wall_index].G;
    *tB = walls[wall_index].B;
  }

  // A point on a circle is exactly one radius away from its center.
  // Substitute the ray point ray->p + circle_lambda * ray->d into that rule.
  for (int object_index = 0; object_index < MAX_OBJECTS; object_index++) {
    if (objects[object_index].r <= 0.0)
      break;

    // From the circle center to the ray origin.
    double center_to_ray_x = ray->p.px - objects[object_index].c.px;
    double center_to_ray_y = ray->p.py - objects[object_index].c.py;

    // Squaring the distance equation gives:
    // A * circle_lambda^2 + B * circle_lambda + C = 0
    // A = ray direction dot ray direction
    // B = 2 * (center-to-origin vector dot ray direction)
    // C = center-to-origin length squared - radius squared
    double quadratic_a = ray->d.px * ray->d.px + ray->d.py * ray->d.py;
    double quadratic_b =
        2.0 * (center_to_ray_x * ray->d.px + center_to_ray_y * ray->d.py);
    double quadratic_c = center_to_ray_x * center_to_ray_x +
                         center_to_ray_y * center_to_ray_y -
                         objects[object_index].r * objects[object_index].r;
    double discriminant =
        quadratic_b * quadratic_b - 4.0 * quadratic_a * quadratic_c;

    // A negative discriminant means the ray misses the circle.
    if (quadratic_a <= TOL || discriminant < 0.0)
      continue;

    // The quadratic roots are the distances to the two possible hit points.
    // Keep the nearest root that is in front of the ray.
    double square_root = sqrt(discriminant);
    double near_lambda = (-quadratic_b - square_root) / (2.0 * quadratic_a);
    double far_lambda = (-quadratic_b + square_root) / (2.0 * quadratic_a);
    double circle_lambda = HUGE_VAL;
    if (near_lambda > TOL)
      circle_lambda = near_lambda;
    if (far_lambda > TOL && far_lambda < circle_lambda) {
      circle_lambda = far_lambda;
    }

    // Ignore misses and circle hits farther away than the closest hit so far.
    if (circle_lambda == HUGE_VAL || circle_lambda >= closest_lambda)
      continue;

    closest_lambda = circle_lambda;
    p->px = ray->p.px + circle_lambda * ray->d.px;
    p->py = ray->p.py + circle_lambda * ray->d.py;

    // The circle normal points from its center toward the hit point.
    n->px = p->px - objects[object_index].c.px;
    n->py = p->py - objects[object_index].c.py;
    normalize(n);

    *lambda = circle_lambda;
    *type = objects[object_index].material_type;
    *r_idx = objects[object_index].r_idx;
    *tR = objects[object_index].R;
    *tG = objects[object_index].G;
    *tB = objects[object_index].B;
  }

  return;
}
