#version 450

struct PointLight {
    vec4 position;
    vec4 color;
};

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec2 inTexCoord;
layout(location = 3) in vec3 inTangent;
layout(location = 4) in uvec4 inJointIndices;
layout(location = 5) in vec4  inJointWeights;

layout(set = 0, binding = 0) uniform GlobalUBO {
    mat4 View;
    mat4 Projection;
    vec3 CameraPos;
    int  LightCount;
    PointLight lights[8];
} ubo;

layout(set = 2, binding = 0) uniform SkinningUBO {
    mat4 JointMatrices[256];
};

layout(push_constant) uniform PushConstants {
    mat4  Model;
    vec4  Color;
    float RoughnessOverride;
    float MetallicOverride;
    float _pad0;
    float _pad1;
} pc;

layout(location = 0) out vec3 fragWorldPos;
layout(location = 1) out vec3 fragNormal;
layout(location = 2) out vec2 fragTexCoord;
layout(location = 3) out vec3 fragTangent;
layout(location = 4) out vec3 fragBitangent;

void main()
{
    mat4 skin = inJointWeights.x * JointMatrices[inJointIndices.x]
              + inJointWeights.y * JointMatrices[inJointIndices.y]
              + inJointWeights.z * JointMatrices[inJointIndices.z]
              + inJointWeights.w * JointMatrices[inJointIndices.w];

    mat4 worldSkin = pc.Model * skin;

    vec4 worldPos  = worldSkin * vec4(inPosition, 1.0);
    gl_Position    = ubo.Projection * ubo.View * worldPos;
    fragWorldPos   = worldPos.xyz;

    mat3 normalMat = transpose(inverse(mat3(worldSkin)));
    fragNormal     = normalize(normalMat * inNormal);
    fragTangent    = normalize(normalMat * inTangent);
    fragBitangent  = cross(fragNormal, fragTangent);

    fragTexCoord   = inTexCoord;
}