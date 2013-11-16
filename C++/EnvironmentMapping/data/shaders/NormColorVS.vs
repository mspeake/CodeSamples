// ---------------------------------------------------------------------------
// varyings
varying vec3 VertNormal;
varying vec3 VertPosition;
// ---------------------------------------------------------------------------

void main()
{
	VertNormal = normalize(gl_Normal.xyz);
	VertPosition = (gl_Vertex).xyz;
	
	gl_Position		=	gl_ModelViewProjectionMatrix * gl_Vertex;
	gl_FrontColor	=	gl_Color;
	gl_TexCoord[0]	=	gl_MultiTexCoord0;
}



