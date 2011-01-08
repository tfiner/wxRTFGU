#ifndef TRACER_DEBUG_H_INCLUDED
#define TRACER_DEBUG_H_INCLUDED

#include <Tracer.h>

class World;
typedef boost::shared_ptr<World> WorldPtr;

class RGBColor;


class TracerDebug : public Tracer {
public:
    TracerDebug(WorldPtr w, const RGBColor& color);

    virtual ~TracerDebug();

    virtual RGBColor trace_ray(const Ray& ray) const;
    virtual RGBColor trace_ray(const Ray ray, const int depth) const;

private:
    RGBColor color_;
};


#endif // TRACER_DEBUG_H_INCLUDED
