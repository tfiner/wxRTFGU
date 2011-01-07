#include "tracer_math.h"

#include <RGBColor.h>
#include <Ray.h>

#include <cmath>


using namespace std;


TracerMath::TracerMath(WorldPtr w) : Tracer(w) {}
TracerMath::~TracerMath() {}


namespace {


    const double DEG_PER_RADS = 0.0174532925;
    inline float degToRad(double deg) {
        return DEG_PER_RADS * deg;
    }


    /*
        f(x,y) = 1/2 * (1 + sin(x^2 y^2))
    */
    RGBColor sinusoid(const Ray& ray) {
        double x = degToRad(ray.o.x);
        double y = degToRad(ray.o.y);
        float fxy = 0.5f * (1 + sin(x*x * y*y));
        return RGBColor(fxy,fxy,fxy);
    }


}


RGBColor TracerMath::trace_ray(const Ray& ray) const {
    return sinusoid(ray);
}


RGBColor TracerMath::trace_ray(const Ray ray, const int depth) const {
    return sinusoid(ray);
}

