/* Start Header -------------------------------------------------------
Copyright (C) 2011 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the prior written
consent of DigiPen Institute of Technology is prohibited.
File Name: envmapVS.vs
Purpose: vertex shader for environment mapping
Language: C++
Platform: MSVC2008 compiler, Windows XP/Vista/7
Project: matthew.speake_CS300_4
Author: Matt Speake, matthew.speake, 50006908
Creation date: 3/5/2012
End Header --------------------------------------------------------*/

// ---------------------------------------------------------------------------
// varyings

varying vec3 vertNormal;
varying vec3 vertPosition;

attribute vec3 tangent;
attribute vec3 bitangent;

varying vec3 tangent2;
varying vec3 bitangent2;

// ---------------------------------------------------------------------------

void main()
{
	vertNormal = normalize(gl_Normal.xyz); //normalize(gl_NormalMatrix * gl_Normal);//
	vertPosition = (gl_Vertex).xyz; //vec3(gl_ModelViewMatrix * gl_Vertex);//
	
	//---------------------------------------------------------------------------
	vec3 getTan = normalize(gl_NormalMatrix * tangent);    //--------------------
	tangent2 = getTan;                                     //--------------------
	                                                       //Normal Mapping stuff
	vec3 getBit = normalize(gl_NormalMatrix * bitangent);  //--------------------
	bitangent2 = getBit;                                   //--------------------
	//---------------------------------------------------------------------------
	
	gl_Position		=	gl_ModelViewProjectionMatrix * gl_Vertex;
	gl_FrontColor	=	gl_Color;
	gl_TexCoord[0]	=	gl_MultiTexCoord0;
}



