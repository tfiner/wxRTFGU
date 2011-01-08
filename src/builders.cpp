#include "builders.h"

#include <World.h>

#include <IRenderer.h>
#include <IBuilder.h>
#include <MultipleObjects.h>
#include <RayCast.h>
#include <Matte.h>
#include <Plane.h>
#include <Regular.h>
#include <Pinhole.h>

#include "tracer_math.h"
#include "tracer_debug.h"

namespace {

    const RGBColor BLACK    = RGBColor(0.0,0.0,0.0);
    const RGBColor WHITE    = RGBColor(1.0,1.0,1.0);
    const RGBColor RED      = RGBColor(1.0,0.0,0.0);
    const RGBColor GREEN    = RGBColor(0.0,1.0,0.0);
    const RGBColor YELLOW   = RGBColor(1.0,1.0,0.0);

}


void build3_1(WorldPtr w) {
    Sphere s(Point3D(0,0,0), 100.0);
    w->set_sphere(s);
    w->set_tracer( TracerPtr(new SingleSphere(w)) );
}


void build3_2(WorldPtr w) {
    w->set_background( BLACK );
    ViewPlane vp = w->get_viewplane();
    SamplerPtr sampler = vp.get_sampler();
    sampler->init(16, 83);
    w->set_viewplane(vp);

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


void build4_4a(WorldPtr w) {
    int num_samples = 1;  // use 1 for 4.4(a) and 16 for 4.4(b)

    SamplerPtr uniform_ptr( new Regular(num_samples) );

    ViewPlane vp;
    vp.set_hres(32);
    vp.set_vres(32);
    vp.set_pixel_size(1.0);
    vp.set_sampler(uniform_ptr);

    w->set_background( BLACK );
    w->set_tracer( TracerPtr(new RayCast(w)) );

    Camera* camera = new Pinhole;
    camera->set_eye(0, 0, 100);
    camera->set_lookat(0.0);
    w->set_camera(camera);

    w->set_ambient( new Ambient );

//    PointLight* light_ptr = new PointLight();
//    light_ptr->set_location(100, 100, 200);
//    light_ptr->scale_radiance(2.0);
//    w->add_light(light_ptr);

    Matte* matte_ptr = new Matte;
    matte_ptr->set_ka(0.2);
    matte_ptr->set_kd(0.8);
    matte_ptr->set_cd( YELLOW );

    Sphere* sphere_ptr = new Sphere(Point3D(0.0), 13.0);
    sphere_ptr->set_material(MaterialPtr(matte_ptr));
    w->add_object(sphere_ptr);
}


void build_math(WorldPtr w) {
    ViewPlane vp = w->get_viewplane();
    vp.set_pixel_size(0.85);

    SamplerPtr sampler = vp.get_sampler();
    sampler->init(4, 83);
    sampler->generate_samples();

    w->set_viewplane(vp);
    w->set_tracer( TracerPtr(new TracerMath(w)) );

}


void build_debug(WorldPtr w) {
    ViewPlane vp;
    vp.set_hres(320);
    vp.set_vres(320);
    vp.set_pixel_size(1);
    SamplerPtr sampler = vp.get_sampler();
    sampler->init(16, 83);
    sampler->generate_samples();
    vp.set_sampler(sampler);
    w->set_viewplane(vp);


    w->set_background( WHITE );
    w->set_tracer( TracerPtr(new TracerDebug(w, BLACK)) );
}
