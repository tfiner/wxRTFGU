#include "tracer_debug.h"

#include <RGBColor.h>
#include <Ray.h>

#include <cmath>


using namespace std;


TracerDebug::TracerDebug(WorldPtr w, const RGBColor& color ) :
    Tracer(w), color_(color) {}

TracerDebug::~TracerDebug() {}



RGBColor TracerDebug::trace_ray(const Ray& ray) const {
    return color_;
}


RGBColor TracerDebug::trace_ray(const Ray ray, const int depth) const {
    return color_;
}

