#version 450

layout(binding = 1) uniform sampler2D texSampler;

layout(location = 0) in vec2 fragTexCoord;
layout(location = 1) in vec4 fragColor;  
layout(location = 0) out vec4 outColor;

void main() {
    vec4 texColor = texture(texSampler, fragTexCoord);
    
    // Комбинируем текстуру с цветом из Push Constants
    // Это позволяет tint'овать текстуру разными цветами для разных кубов
    outColor = texColor * fragColor;
    
    // Альтернативы:
    // outColor = texColor;                    // Только текстура
    // outColor = fragColor;                   // Только цвет
    // outColor = mix(texColor, fragColor, 0.5); // Смешивание
}