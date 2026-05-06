#version 450

struct PointLight {
    vec4 position;  // w не используется
    vec4 color;     // w = intensity
};

layout(std140, set = 0, binding = 0) uniform GlobalUBO {
    mat4       view;
    mat4       projection;
    vec3       cameraPos;
    int        lightCount;
    PointLight lights[8];
} ubo;

layout(push_constant) uniform PushConstants {
    mat4  model;
    vec4  color;
    float roughnessOverride;
    float metallicOverride;
    float pad[2];
} pc;

layout(set = 1, binding = 0) uniform sampler2D albedoMap;
layout(set = 1, binding = 1) uniform sampler2D normalMap;
layout(set = 1, binding = 2) uniform sampler2D roughnessMetallicMap;

layout(location = 0) in vec3 fragWorldPos;
layout(location = 1) in vec3 fragNormal;
layout(location = 2) in vec2 fragTexCoord;
layout(location = 3) in vec3 fragTangent;

layout(location = 0) out vec4 outColor;

const float PI = 3.14159265359;

float DistributionGGX(vec3 N, vec3 H, float roughness) {
    float a      = roughness * roughness;
    float a2     = a * a;
    float NdotH  = max(dot(N, H), 0.0);
    float NdotH2 = NdotH * NdotH;
    float denom  = (NdotH2 * (a2 - 1.0) + 1.0);
    return a2 / (PI * denom * denom);
}

float GeometrySchlickGGX(float NdotV, float roughness) {
    float r = roughness + 1.0;
    float k = (r * r) / 8.0;
    return NdotV / (NdotV * (1.0 - k) + k);
}

float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness) {
    float NdotV = abs(dot(N, V)); 
    float NdotL = max(dot(N, L), 0.0);
    return GeometrySchlickGGX(NdotV, roughness) * GeometrySchlickGGX(NdotL, roughness);
}

vec3 FresnelSchlick(float cosTheta, vec3 F0) {
    return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

vec3 CalcLight(vec3 N, vec3 V, vec3 L, vec3 lightColor, float intensity,
               vec3 albedo, float roughness, float metallic, vec3 F0)
{
    vec3  H     = normalize(V + L);
    float NdotL = max(dot(N, L), 0.0);
    if (NdotL <= 0.0) return vec3(0.0);

    float NDF      = DistributionGGX(N, H, roughness);
    float G        = GeometrySmith(N, V, L, roughness);
    vec3  F        = FresnelSchlick(max(dot(H, V), 0.0), F0);
    float NdotV = abs(dot(N, V)); 
    vec3  specular = (NDF * G * F) / (4.0 * NdotV * NdotL + 0.0001);
    vec3  kD       = (vec3(1.0) - F) * (1.0 - metallic);
    vec3  diffuse  = kD * albedo / PI;

    return (diffuse + specular) * lightColor * intensity * NdotL;
}

void main() {
    vec3 albedo = pow(texture(albedoMap, fragTexCoord).rgb, vec3(2.2)) * pc.color.rgb;

    vec3 normalSample = texture(normalMap, fragTexCoord).rgb * 2.0 - 1.0;

    vec2  rm        = texture(roughnessMetallicMap, fragTexCoord).rg;
    float roughness = pc.roughnessOverride >= 0.0 ? pc.roughnessOverride : clamp(rm.x, 0.04, 1.0);
    float metallic  = pc.metallicOverride  >= 0.0 ? pc.metallicOverride  : clamp(rm.y, 0.0,  1.0);

    vec3 N = normalize(fragNormal);
    vec3 T = normalize(fragTangent - dot(fragTangent, N) * N);
    vec3 B = cross(N, T);
    mat3 TBN = mat3(T, B, N);
    N = normalize(TBN * normalSample);

    vec3 V  = normalize(ubo.cameraPos - fragWorldPos);
    vec3 F0 = mix(vec3(0.04), albedo, metallic);

    vec3 Lo = vec3(0.0);

    if (ubo.lightCount <= 0 || ubo.lightCount > 8) {
        outColor = vec4(1.0, 0.0, 0.0, 1.0); // красный = проблема с lightCount
        return;
    }

    for (int i = 0; i < ubo.lightCount; i++) {
        vec3  lightPos    = ubo.lights[i].position.xyz;
        vec3  lightColor  = ubo.lights[i].color.rgb;
        float intensity   = ubo.lights[i].color.w;

        vec3  L           = normalize(lightPos - fragWorldPos);
        float dist        = length(lightPos - fragWorldPos);
        float attenuation = 1.0 / (dist * dist);

        Lo += CalcLight(N, V, L, lightColor, intensity * attenuation, albedo, roughness, metallic, F0);
    }

    vec3 ambient = vec3(0.02) * albedo; 
    vec3 color   = ambient + Lo;

    color = color / (color + vec3(1.0));
    color = pow(color, vec3(1.0 / 2.2));

    outColor = vec4(color, 1.0);
    // нормали 
    // outColor = vec4(normalize(fragNormal) * 0.5 + 0.5, 1.0);
    // outColor = vec4(abs(ubo.cameraPos) * 0.01, 1.0);
}