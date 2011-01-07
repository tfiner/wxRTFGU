#ifndef TRACER_MATH_H_INCLUDED
#define TRACER_MATH_H_INCLUDED

#include <Tracer.h>

class World;
typedef boost::shared_ptr<World> WorldPtr;

class RGBColor;


class TracerMath : public Tracer {
public:
    TracerMath(WorldPtr w);

    virtual ~TracerMath();

    virtual RGBColor trace_ray(const Ray& ray) const;
    virtual RGBColor trace_ray(const Ray ray, const int depth) const;
};


#endif // TRACER_MATH_H_INCLUDED
