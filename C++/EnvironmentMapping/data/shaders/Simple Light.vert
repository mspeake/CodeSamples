varying vec3 VertPosition;
varying vec3 VertNormal;

attribute vec3 ModelTangent;
attribute vec3 ModelBitangent;

varying vec3 Tangent;
varying vec3 Bitangent;

void main()
{
	VertNormal = normalize(gl_NormalMatrix * gl_Normal);
	VertPosition = vec3(gl_ModelViewMatrix * gl_Vertex);
	
	Tangent = normalize(gl_NormalMatrix * ModelTangent);
	Bitangent = normalize(gl_NormalMatrix * ModelBitangent);

	gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;
	gl_FrontColor = gl_Color;
	gl_TexCoord[0] = gl_MultiTexCoord0;
}