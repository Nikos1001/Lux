
#version 330 core
out vec4 FragColor;

in vec2 pUV;

#define NUM_SPHERES 32

int uNumSamples = 15;

uniform vec3 uSpherePos[NUM_SPHERES];
uniform float uSphereRadius[NUM_SPHERES];
uniform vec3 uSphereColor[NUM_SPHERES];
uniform float uSphereMetalness[NUM_SPHERES];
uniform float uSphereFuzz[NUM_SPHERES];
uniform vec3 uSphereEmmision[NUM_SPHERES];
uniform int uNumSpheres;

uniform vec3 uSky1, uSky2;

uniform float uTimeSeed;

vec2 seed = vec2(0.0f);
float rand() {
    seed.x += 3;
    return fract(sin(dot(seed.xy ,vec2(12.9898,78.233))) * 43758.5453);
}

struct ray {
    vec3 origin;
    vec3 dir;
};

vec3 rayAt(ray r, float t) {
    return r.origin + t * normalize(r.dir);
}

vec3 reflect(vec3 v, vec3 n) {
    return v - 2 * dot(v, n) * n;
}

vec3 randomOnUnitSphere() {
    vec3 p = vec3(rand(), rand(), rand()) * 2.0 - vec3(1.0);
    p = normalize(p);
    return p;
}

vec3 randomInUnitSphere() {
    vec3 p = vec3(rand(), rand(), rand()) * 2.0 - vec3(1.0);
    p = normalize(p) * rand();
    return p;
}

float hitSphere(vec3 center, float radius, ray r) {
    vec3 oc = r.origin - center;
    float a = dot(r.dir, r.dir);
    float b = 2.0 * dot(oc, r.dir);
    float c = dot(oc, oc) - radius*radius;
    float discriminant = b*b - 4*a*c;
    if(discriminant < 0.0f)
        return -1.0f;
    else
        return (-b - sqrt(discriminant)) / (2.0f * a);
}

struct ifPair {
    int i;
    float f;
};

ifPair closestSphere(ray r) {
    int ans = -1;
    float minT = 1000000.0f;
    for(int i = 0; i < uNumSpheres; i++) {
        float t = hitSphere(uSpherePos[i], uSphereRadius[i], r);
        if(t > 0.0f && t < minT) {
            minT = t;
            ans = i;
        }
    }
    ifPair res;
    res.i = ans;
    res.f = minT;
    return res;
}

#define RAY_FUNC(idx, prevIdx) \
    vec3 rayColor ## idx(ray r) { \
        ifPair sphere = closestSphere(r); \
        if(sphere.i != -1) { \
            vec3 p = rayAt(r, sphere.f); \
            vec3 N = normalize(p - uSpherePos[sphere.i]); \
            vec3 target = p + N + randomOnUnitSphere(); \
            ray newRay; \
            newRay.origin = p; \
            if(rand() > uSphereMetalness[sphere.i]) { \
                newRay.dir = target - p; \
            } else { \
                newRay.dir = reflect(r.dir, N) + uSphereFuzz[sphere.i] * randomInUnitSphere(); \
            } \
            return uSphereEmmision[sphere.i] + uSphereColor[sphere.i] * rayColor ## prevIdx(newRay); \
        } \
        vec3 unit = normalize(r.dir); \
        float t = 0.5f * (unit.y + 1.0f); \
        return ((1.0 - t) * uSky1 + t * uSky2); \ // vec3(0.5, 0.7, 1.0)
    }

vec3 rayColor0(ray r) {
    return vec3(0.0f);
}

RAY_FUNC(1, 0)
RAY_FUNC(2, 1)
RAY_FUNC(3, 2)
RAY_FUNC(4, 3)
RAY_FUNC(5, 4)
RAY_FUNC(6, 5)
RAY_FUNC(7, 6)
RAY_FUNC(8, 7)
RAY_FUNC(9, 8)
RAY_FUNC(10, 9)
RAY_FUNC(11, 10)
RAY_FUNC(12, 11)
RAY_FUNC(13, 12)
RAY_FUNC(14, 13)
RAY_FUNC(15, 14)
RAY_FUNC(16, 15)

void main() {
    seed = pUV + vec2(uTimeSeed);

    vec3 result = vec3(0.0f);
    for(int i = 0; i < uNumSamples; i++) {
        ray r;
        r.origin = vec3(0.0, 0.0, 0.5f);
        vec2 rnd = vec2(rand(), rand()) / 500.0f;
        r.dir = normalize(vec3(pUV.x + rnd.x, pUV.y + rnd.y, -2.0f));
        vec3 c = rayColor5(r);
        result += c;
    }
    result /= uNumSamples;
    result = sqrt(result);
    FragColor = vec4(result.r, result.g, result.b, 1.0f);
}