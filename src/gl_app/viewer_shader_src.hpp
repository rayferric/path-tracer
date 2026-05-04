#pragma once

inline const char *viewer_shader_vert = R"(
#version 460 core
out vec2 v_TexCoord;
void main() {
    v_TexCoord = vec2((gl_VertexID << 1) & 2, gl_VertexID & 2);
    gl_Position = vec4(v_TexCoord * 2.0f - 1.0f, 0.0f, 1.0f);
}
)";
inline const char *viewer_shader_frag = R"(
#version 460 core
in vec2 v_TexCoord;
out vec4 out_FragColor;
uniform sampler2D tex_Image;

layout(std140, binding = 0) uniform ViewportData {
    vec2 u_WindowSize;
    vec2 u_ImageSize;
    vec2 u_Pan;
    float u_Zoom;
};

void main() {
    float baseScale = min(u_WindowSize.x / u_ImageSize.x, u_WindowSize.y / u_ImageSize.y);
    float scale = baseScale * u_Zoom;
    
    vec2 drawSize = u_ImageSize * scale;
    vec2 drawBL = (u_WindowSize - drawSize) * 0.5 + u_Pan;
    
    vec2 localPos = gl_FragCoord.xy - drawBL;
    vec2 texUV = localPos / drawSize;

    texUV.y = 1.0 - texUV.y;
    vec4 imageColor = texture(tex_Image, texUV);
    
    // ^ chessboard pattern in image space (not screen space)
    float checkerSize = 16.0; // size in pixels of each checker square
    vec2 checker = floor(localPos / checkerSize);
    float pattern = mod(checker.x + checker.y, 2.0);
    vec3 lightGray = vec3(0.02);
    vec3 darkGray = vec3(0.01);
    vec3 checkerColor = mix(darkGray, lightGray, pattern);
    
    vec3 blendedColor = mix(checkerColor, imageColor.rgb, imageColor.a);

    vec3 borderColor = vec3(0.005);
    bool isBorder = texUV.x < 0.0 || texUV.x > 1.0 || texUV.y < 0.0 || texUV.y > 1.0;
    out_FragColor = vec4(mix(blendedColor, borderColor, isBorder ? 1.0 : 0.0), 1.0);
}
)";
