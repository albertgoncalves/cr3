#version 330 core

layout(location = 0) out vec4 FRAG_OUT_COLOR;

uniform vec2  WINDOW;
uniform float TIME;

struct Plane {
    vec3  normal;
    float d;
};

struct Sphere {
    vec3  center;
    float radius;
};

#define EPSILON 0.0001f

#define INF (1.0f / 0.0f)

#define N_SPHERES 4

void main() {
    vec3 camera_position = vec3(TIME * 3.0f, (TIME * 5.0f) + 0.25f, 12.5f);
    vec3 camera_target = vec3(0.0f, 0.0f, 0.0f);
    vec3 camera_z = normalize(camera_position - camera_target);
    vec3 camera_x = normalize(cross(vec3(TIME / 10.0f, 1.0f, 0.0f), camera_z));
    vec3 camera_y = normalize(cross(camera_z, camera_x));

    vec2 pixel = ((gl_FragCoord.xy / WINDOW) * 2.0f) - 1.0f;

    vec2  film = (vec2(1.0f) * WINDOW) / WINDOW.y;
    float film_dist = 1.0f;
    vec3  film_center = camera_position - (camera_z * film_dist);

    vec2 ray_thru_film = pixel * (film / 2.0f);
    vec3 ray_direction =
        normalize((film_center + (camera_x * ray_thru_film.x) +
                   (camera_y * ray_thru_film.y)) -
                  camera_position);

    float hit_dist = INF;
    {
        Plane plane = Plane(normalize(vec3(0.0f, 1.0f, 0.0)), -1.0f);
        float denom = dot(plane.normal, ray_direction);
        if (EPSILON < abs(denom)) {
            float new_dist =
                (plane.d - dot(plane.normal, camera_position)) / denom;
            if ((0 < new_dist) && (new_dist < hit_dist)) {
                hit_dist = new_dist;
                vec3  color = vec3(pixel, TIME);
                float brightness = abs(dot(ray_direction, plane.normal));
                FRAG_OUT_COLOR = vec4(color * brightness, 1.0f);
            }
        }
    }
    Sphere spheres[N_SPHERES] =
        Sphere[](Sphere(vec3(5.0f, -0.5f, 1.0f), 2.0f),
                 Sphere(vec3(0.0f), 4.0f),
                 Sphere(vec3(-5.0f, 0.5f, 1.0f), 1.0f),
                 Sphere(vec3(0.0f, -1.0f, 7.0f), 0.75f));
    {
        for (int i = 0; i < N_SPHERES; ++i) {
            Sphere sphere = spheres[i];
            vec3   relative_position = camera_position - sphere.center;
            float  a = dot(ray_direction, ray_direction);
            float  b = 2.0f * dot(ray_direction, relative_position);
            float  c = dot(relative_position, relative_position) -
                      (sphere.radius * sphere.radius);
            float denom = a * 2.0f;
            if (EPSILON < abs(denom)) {
                float root = sqrt((b * b) - (4.0f * a * c));
                if (EPSILON < abs(root)) {
                    float t0 = (-b + root) / denom;
                    float t1 = (-b - root) / denom;
                    float new_dist = t0;
                    if ((0 < t1) && (t1 < t0)) {
                        new_dist = t1;
                    }
                    if ((0 < new_dist) && (new_dist < hit_dist)) {
                        hit_dist = new_dist;
                        vec3 color = vec3(pixel, TIME);
                        vec3 sphere_normal = normalize(
                            camera_position + (ray_direction * hit_dist) -
                            sphere.center);
                        float brightness =
                            abs(dot(ray_direction, sphere_normal));
                        FRAG_OUT_COLOR = vec4(color * brightness, 1.0f);
                    }
                }
            }
        }
    }
    if (INF <= hit_dist) {
        discard;
    }
}
