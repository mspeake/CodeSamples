varying vec3 VertNormal;
varying vec3 VertPosition;

attribute vec3 ModelTangent;
attribute vec3 ModelBitangent;

varying vec3 Tangent;
varying vec3 Bitangent;

void main()
{
	VertNormal = normalize(gl_Normal.xyz);
	VertPosition = (gl_Vertex).xyz;
	
	Tangent = normalize(gl_NormalMatrix * ModelTangent);
	Bitangent = normalize(gl_NormalMatrix * ModelBitangent);
	
	gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;
	gl_FrontColor = gl_Color;
	gl_TexCoord[0] = gl_MultiTexCoord0;
}