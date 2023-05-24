#version 330 core

out vec2 uv;
out vec2 mouse;
out float scale;

uniform vec2 iMouse;
uniform vec2 iResolution;

void main() {
  uv = vec2(gl_VertexID & 1, gl_VertexID >> 1);
  mouse = iMouse;

  gl_Position = vec4(2 * uv - 1, 0, 1);

  scale = iResolution.x / iResolution.y;
  uv.x *= scale;
  mouse.x *= scale;
}
