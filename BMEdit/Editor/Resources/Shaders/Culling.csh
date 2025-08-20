#version 460 core

struct ObjectTransformDescription {
    mat4 Matrix;    // CPU: Write, GPU: Read
    vec4 BoundsMin; // CPU: Write, GPU: Read | World space
    vec4 BoundsMax; // CPU: Write, GPU: Read | World space
    vec4 Status;    // CPU: Read,  GPU: Write
};

layout(std430, binding = 0) buffer ObjectTransformsBuffer {
    ObjectTransformDescription objects[];
};

layout(local_size_x = 64, local_size_y = 1, local_size_z = 1) in;

uniform mat4 cameraProjView;

bool aabbInFrustum(vec3 bmin, vec3 bmax, mat4 projView)
{
    vec4 planes[6];
    planes[0] = vec4(projView[0].w + projView[0].x,
    projView[1].w + projView[1].x,
    projView[2].w + projView[2].x,
    projView[3].w + projView[3].x);
    planes[1] = vec4(projView[0].w - projView[0].x,
    projView[1].w - projView[1].x,
    projView[2].w - projView[2].x,
    projView[3].w - projView[3].x);
    planes[2] = vec4(projView[0].w + projView[0].y,
    projView[1].w + projView[1].y,
    projView[2].w + projView[2].y,
    projView[3].w + projView[3].y);
    planes[3] = vec4(projView[0].w - projView[0].y,
    projView[1].w - projView[1].y,
    projView[2].w - projView[2].y,
    projView[3].w - projView[3].y);
    planes[4] = vec4(projView[0].w + projView[0].z,
    projView[1].w + projView[1].z,
    projView[2].w + projView[2].z,
    projView[3].w + projView[3].z);
    planes[5] = vec4(projView[0].w - projView[0].z,
    projView[1].w - projView[1].z,
    projView[2].w - projView[2].z,
    projView[3].w - projView[3].z);

    for (int i = 0; i < 6; i++) {
        float len = length(planes[i].xyz);
        planes[i] /= len;
    }

    for (int i = 0; i < 6; i++) {
        float x = (planes[i].x > 0.0) ? bmax.x : bmin.x;
        float y = (planes[i].y > 0.0) ? bmax.y : bmin.y;
        float z = (planes[i].z > 0.0) ? bmax.z : bmin.z;
        float distance = planes[i].x * x + planes[i].y * y + planes[i].z * z + planes[i].w;

        if (distance <= 0.0)
        {
            return false;
        }
    }

    return true;
}

void main() {
    uint idx = gl_GlobalInvocationID.x;
    ObjectTransformDescription obj = objects[idx];

    vec3 bmin = obj.BoundsMin.xyz;
    vec3 bmax = obj.BoundsMax.xyz;

    bool visible = aabbInFrustum(bmin, bmax, cameraProjView);
    objects[idx].Status.x = visible ? 1.0 : 0.0;
}
