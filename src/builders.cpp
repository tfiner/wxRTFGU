#include <wx/wx.h>

#include "builders.h"

#include <World.h>
#include <Constants.h>

#include <IRenderer.h>
#include <IBuilder.h>
#include <MultipleObjects.h>
#include <RayCast.h>

#include <Plane.h>
#include <Regular2D.h>
#include <Pinhole.h>

#include "tracer_math.h"
#include "tracer_debug.h"


void build3_1(WorldPtr w) {
    Sphere s(Point3D(0,0,0), 100.0);
    w->set_sphere(s);
    w->set_tracer( TracerPtr(new SingleSphere(w)) );

//    ViewPlane vp = w->get_viewplane();
//    SamplerPtr sampler = vp.get_sampler();
//    sampler->init(16, 16);
//    sampler->generate_samples();
//    w->set_viewplane(vp);
}


void build3_2(WorldPtr w) {
    w->set_tracer( TracerPtr(new MultipleObjects(w)) );

	Sphere*	sphere1 = new Sphere(Point3D(0,-25,0), 80.0);
	sphere1->set_color( RED );
    w->add_object( sphere1 );

	Sphere*	sphere2 = new Sphere(Point3D(0,30,0), 60.0);
	sphere2->set_color( YELLOW );
    w->add_object( sphere2 );

    Plane* plane = new Plane(Point3D(0,0,0), Normal(0,1,1));
    plane->set_color( RGBColor(0.0,0.3,0.0) );
    w->add_object(plane);
}



void build_math(WorldPtr w) {
    w->set_tracer( TracerPtr(new TracerMath(w)) );
}


void build_debug(WorldPtr w) {
    ViewPlane vp = w->get_viewplane();
    SamplerPtr sampler = vp.get_sampler();
    SampleBundle2D samples = sampler->get_next();
    for ( vector<Point2D>::iterator iter = samples.begin(); iter != samples.end(); ++iter )
        wxLogMessage(wxT("Sample: %1.4f  %1.4f"), iter->x, iter->y);
//    int samples = sampler->get_num_samples();
//    vp.set_hres(sqrt(samples));
//    vp.set_vres(sqrt(samples));
    vp.set_hres(1);
    vp.set_vres(1);
    vp.set_pixel_size(1);
    w->set_viewplane(vp);

    w->set_background( WHITE );
    w->set_tracer( TracerPtr(new TracerDebug(w, BLACK)) );
}
