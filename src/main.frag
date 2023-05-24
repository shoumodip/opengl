#version 330 core

in vec2 uv;
in vec2 mouse;
in float scale;
out vec4 color;

bool inside_rect(vec2 uv, vec2 start, vec2 end) {
  return uv.x >= start.x * scale && uv.x <= end.x * scale && uv.y >= start.y && uv.y <= end.y;
}

bool inside_circle(vec2 uv, vec2 center, float radius) {
  return distance(uv, center) <= radius;
}

void main() {
  color = vec4(
    inside_circle(uv, mouse, 0.1),
    inside_rect(uv, vec2(0.3, 0.4), vec2(0.7, 0.6)),
    0, 1);
}
