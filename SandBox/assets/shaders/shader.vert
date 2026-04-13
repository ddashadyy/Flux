#version 450

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec2 inTexCoord;

layout(set = 0, binding = 0) uniform GlobalUBO {
    mat4 view;
    mat4 projection;
} globalUBO;

layout(push_constant) uniform PushConstants {
    mat4 model;
    vec4 color;
    float timeOffset;
} pc;

layout(location = 0) out vec2 fragTexCoord;
layout(location = 1) out vec4 fragColor;

void main() {
    mat4 mvp = globalUBO.projection * globalUBO.view * pc.model;
    gl_Position = mvp * vec4(inPosition, 1.0);
    
    fragTexCoord = inTexCoord;
    fragColor = pc.color;  
}