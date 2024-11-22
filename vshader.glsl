#version 120

attribute vec4 vPosition;
attribute vec4 vNormal;
attribute vec2 vTexCoord;

varying vec2 texCoord;
varying vec4 N, V, L;

uniform mat4 ctm, model_view, projection;

uniform vec4 light_position;

void main()
{
    N = normalize(model_view * ctm * vNormal);
    L = normalize(model_view * (light_position - ctm * vPosition));
    V = normalize(vec4(0, 0, 0, 1) - model_view * ctm * vPosition);

    texCoord = vTexCoord;
    gl_Position = projection * model_view * ctm * vPosition;
}
