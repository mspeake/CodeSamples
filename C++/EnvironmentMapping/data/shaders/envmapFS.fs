/* Start Header -------------------------------------------------------
Copyright (C) 2011 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the prior written
consent of DigiPen Institute of Technology is prohibited.
File Name: envmapFS.fs
Purpose: fragment shader for environment mapping
Language: C++
Platform: MSVC2008 compiler, Windows XP/Vista/7
Project: matthew.speake_CS300_4
Author: Matt Speake, matthew.speake, 50006908
Creation date: 3/5/2012
End Header --------------------------------------------------------*/

// ---------------------------------------------------------------------------
// uniforms

uniform sampler2D TexLeft;
uniform sampler2D TexRight;
uniform sampler2D TexTop;
uniform sampler2D TexBottom;
uniform sampler2D TexFront;
uniform sampler2D TexBack;
uniform sampler2D NormalMap;

uniform vec3 CameraPos;

uniform int Option;
uniform int NormFlag;
// ---------------------------------------------------------------------------
// varyings

varying vec3 vertNormal;
varying vec3 vertPosition;

varying vec3 tangent2;
varying vec3 bitangent2;

// ---------------------------------------------------------------------------

// Function that simulates environment mapping functionality in OpenGL
vec4 sampleEnvMap(in vec3 r)
{
	vec3 r1 = abs(r);

	if ((r1.x >= r1.y) && (r1.x >= r1.z))
	{
		if (r.x < 0.0)
			return texture2D(TexLeft, r.zy / r1.x * vec2(-1, 1) * 0.5 + 0.5);
		else
			return texture2D(TexRight, r.zy / r1.x * vec2(1, 1) * 0.5 + 0.5);
	}
	else if((r1.y >= r1.x) && (r1.y >= r1.z))
	{
		if(r.y < 0.0)
			return texture2D(TexBottom, r.xz / r1.y * vec2(-1, 1) * 0.5 + 0.5);
		else
			return texture2D(TexTop, r.xz / r1.y * vec2(1, 1) * 0.5 + 0.5);
	}
	else
	{
		if(r.z < 0.0)
			return texture2D(TexBack, r.xy / r1.z * vec2(1, 1) * 0.5 + 0.5);
		else
			return texture2D(TexFront, r.xy / r1.z * vec2(-1, 1) * 0.5 + 0.5);
	}
}

vec3 NM_CalculateNormal()
{
	vec3 NM_Normal = texture2D(NormalMap, gl_TexCoord[0].st).xyz;
	NM_Normal = 2.0 * NM_Normal - vec3(1.0, 1.0, 1.0);
	vec3 normal2;
	mat3 TBN_mat = mat3(tangent2, bitangent2, vertNormal);
	normal2  = TBN_mat * NM_Normal;
	normal2 = normalize(normal2);
	return normal2;
}

void main()
{		
	vec3 Norm = vertNormal;
	if(NormFlag == 1)
		Norm = NM_CalculateNormal();
	vec3 nrm = Norm/dot(Norm, Norm);// vertNormal/dot(vertNormal, vertNormal);
	vec3  L     = CameraPos - vertPosition;
	float eta = 1.0/1.33;
	float eta2 = eta * eta;
	
	float NdotL = dot(nrm, L);
	float NdotL2 = NdotL * NdotL;
	
	vec3 I = -normalize(L);

	vec3 refract;// = ((K*NdotL) - sqrt(1.0 - K2*(1.0 - NdotL2)))*nrm - (K * L);
	float K = 1.0 - eta2 * (1.0 - dot(nrm, I)*dot(nrm, I));
	if(K < 0.0)
		refract = vec3(0.0, 0.0, 0.0);
	else
		refract = eta * I - (eta * dot(nrm, I) + sqrt(K)) * nrm;

	vec3 reflect = (2.0*nrm*NdotL) - L;	

	vec3 R;
	//if reflection: R = reflect, if refraction: R = refract
	
	if(Option == 0)
		R = reflect; //no model->world transformations, so these are already in world space
	else
		R = refract;
	
	//transform the R vec to world space 
	vec3 Rw = R;
	R = Rw;
	
	gl_FragColor = sampleEnvMap(R);
		
	//gl_FragColor = vec4(nrm, 1.0);
}


