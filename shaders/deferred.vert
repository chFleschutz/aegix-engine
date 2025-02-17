#version 450

layout(location = 0) out vec2 outUV;

void main()
{
	vec2 verts[6] = vec2[](
		vec2(-1.0, -1.0), 
		vec2( 1.0, -1.0), 
		vec2(-1.0,  1.0), 
		vec2( 1.0,  1.0),
		vec2(-1.0,  1.0),
		vec2( 1.0, -1.0)
	);

	gl_Position = vec4(verts[gl_VertexIndex], 0.0, 1.0);
	outUV = verts[gl_VertexIndex] * 0.5 + 0.5;
}
