layout (location = 0) in vec2 Position;
layout (location = 1) in vec2 UV;
layout (location = 2) in vec4 Color;

out vec2 Frag_UV;
out vec4 Frag_Color;

layout (binding = 3, std140) uniform cbImGui 
{ 
	mat4 ProjectionMatrix;
};

void main()
{
    Frag_UV = UV;
    Frag_Color = Color;
    gl_Position = ProjectionMatrix * vec4(Position.xy, 0, 1);
}