#include "rtweekend.h"
#include "camera.h"
#include "color.h"
#include "hittable_list.h"
#include "material.h"
#include "sphere.h"

#include <iostream>

color ray_color(const ray& r, const hittable& world, int depth) {
    hit_record rec;

    // If we've exceeded the ray bounce limit, no more light is gathered.
    if (depth <= 0)
        return color(0,0,0);

    if (world.hit(r, 0.001, infinity, rec)) {
        ray scattered;
        color attenuation;
        if (rec.mat_ptr->scatter(r, rec, attenuation, scattered))
            return attenuation * ray_color(scattered, world, depth-1);
        return color(0,0,0);
    }

    vec3 unit_direction = unit_vector(r.direction());
    auto t = 0.5*(unit_direction.y() + 1.0);
    return (1.0-t)*color(1.0, 1.0, 1.0) + t*color(0.5, 0.7, 1.0);
}

hittable_list random_scene() {
hittable_list world;

    auto ground_material = make_shared<lambertian>(color(0.2, 0.8, 0.2));
    world.add(make_shared<sphere>(point3(0, -1000, 0), 1000, ground_material));

    int numSpheres = 20;

    for (int i = 0; i < numSpheres; ++i) {
        double radius = random_double(0.1, 0.5);
        point3 center(random_double(-10, 10), radius, random_double(-10, 10));

        auto choose_material = random_double();
        shared_ptr<material> sphere_material;

        if (choose_material < 0.6) {
            sphere_material = make_shared<lambertian>(color(
                random_double(),
                random_double(),
                random_double()
            ));
        } else if (choose_material < 0.9) {
            sphere_material = make_shared<metal>(color(
                random_double(0.5, 1.0),
                random_double(0.5, 1.0),
                random_double(0.5, 1.0)
            ), random_double(0.0, 0.5));
        } else {
            sphere_material = make_shared<dielectric>(random_double(1.1, 2.0));
        }

        world.add(make_shared<sphere>(center, radius, sphere_material));
    }

    return world;
}

int main() {
    // Image
    const auto aspect_ratio = 16.0 / 9.0;
    const int image_width = 1200;
    const int image_height = static_cast<int>(image_width / aspect_ratio);
    const int samples_per_pixel = 10;
    const int max_depth = 50;

    // World
    auto world = random_scene();

    // Camera
    point3 lookfrom(13, 2, 3);
    point3 lookat(0, 0, 0);
    vec3 vup(0, 1, 0);
    auto dist_to_focus = 10.0;
    auto aperture = 0.1;

    camera cam(lookfrom, lookat, vup, 20, aspect_ratio, aperture, dist_to_focus);

    // Render
    std::cout << "P3\n" << image_width << ' ' << image_height << "\n255\n";

    for (int j = image_height - 1; j >= 0; --j) {
        std::cerr << "\rScanlines remaining: " << j << ' ' << std::flush;
        for (int i = 0; i < image_width; ++i) {
            color pixel_color(0, 0, 0);
            for (int s = 0; s < samples_per_pixel; ++s) {
                auto u = (i + random_double()) / (image_width - 1);
                auto v = (j + random_double()) / (image_height - 1);
                ray r = cam.get_ray(u, v);
                pixel_color += ray_color(r, world, max_depth);
            }
            write_color(std::cout, pixel_color, samples_per_pixel);
        }
    }

    std::cerr << "\nDone.\n";
}
