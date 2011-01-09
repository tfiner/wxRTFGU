#include <wx/wx.h>

#include "tracer_debug.h"

#include <RGBColor.h>
#include <Ray.h>

#include <cmath>


using namespace std;


TracerDebug::TracerDebug(WorldPtr w, const RGBColor& color ) :
    Tracer(w), color_(color) {}

TracerDebug::~TracerDebug() {}


RGBColor TracerDebug::trace_ray(const Ray& ray) const {
    wxLogMessage(wxT("Ray: %1.4f  %1.4f"), ray.o.x, ray.o.y);
    return color_;
}


RGBColor TracerDebug::trace_ray(const Ray ray, const int depth) const {
    wxLogMessage(wxT("Ray: %1.4f  %1.4f"), ray.o.x, ray.o.y);
    return color_;
}

