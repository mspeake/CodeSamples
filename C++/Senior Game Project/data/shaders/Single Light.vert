varying vec3 position;
varying vec3 normal;

attribute vec3 tangent;
attribute vec3 bitangent;

varying vec3 tangent2;
varying vec3 bitangent2;

void main()
{
	position = vec3(gl_ModelViewMatrix * gl_Vertex); //camera space
	normal = normalize(gl_NormalMatrix * gl_Normal);
	
	//---------------------------------------------------------------------------
	vec3 getTan = normalize(gl_NormalMatrix * tangent);    //--------------------
	tangent2 = getTan;                                     //--------------------
	                                                       //Normal Mapping stuff
	vec3 getBit = normalize(gl_NormalMatrix * bitangent);  //--------------------
	bitangent2 = getBit;                                   //--------------------
	//---------------------------------------------------------------------------

	gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;//ftransform(); 
	gl_FrontColor = gl_Color;
	gl_TexCoord[0]	=	gl_MultiTexCoord0;
}