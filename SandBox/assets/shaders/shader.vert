#version 450

layout(std140, set = 0, binding = 0) uniform GlobalUBO {
    mat4 view;
    mat4 projection;
    vec3 cameraPos;
} ubo;

layout(push_constant) uniform PushConstants {
    mat4 model;
    vec4 color;
} pc;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec2 inTexCoord;
layout(location = 3) in vec3 inTangent;

layout(location = 0) out vec3 fragWorldPos;
layout(location = 1) out vec3 fragNormal;
layout(location = 2) out vec2 fragTexCoord;
layout(location = 3) out vec3 fragTangent;

void main() {
    vec4 worldPos = pc.model * vec4(inPosition, 1.0);
    gl_Position  = ubo.projection * ubo.view * worldPos;

    mat3 normalMatrix = mat3(transpose(inverse(pc.model)));
    fragWorldPos  = worldPos.xyz;
    fragNormal    = normalMatrix * inNormal;
    fragTexCoord  = inTexCoord;
    fragTangent   = normalMatrix * inTangent;
}