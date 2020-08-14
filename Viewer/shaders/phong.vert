#version 330

layout (location = 0) in vec3 vert;
layout (location = 1) in vec3 normal;

uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;
uniform mat3 normal_matrix;
uniform int num_instances;
uniform float offset;

uniform int myInstance;
uniform vec3 meshPosition;

smooth out vec3 eye_normal;
smooth out vec3 eye_vertex;

void main(void)  {
/*
    int i = gl_InstanceID / num_instances;
    int j = gl_InstanceID % num_instances;
    vec3 posOffset = vec3( offset*i, 0.0f, offset*j );
*/

    int i = myInstance / num_instances;
    int j = myInstance % num_instances;
    vec3 posOffset = vec3( offset*i, 0.0f, offset*j ) + meshPosition;

    vec4 view_vertex = view * model * vec4(vert + posOffset, 1);
    eye_vertex = view_vertex.xyz;
    eye_normal = normalize(normal_matrix * normal);

    gl_Position = projection * view_vertex;
}
