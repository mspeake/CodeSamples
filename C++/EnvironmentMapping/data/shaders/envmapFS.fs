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

varying vec3 VertNormal;
varying vec3 VertPosition;

varying vec3 Tangent;
varying vec3 Bitangent;

// Function that simulates environment mapping functionality in OpenGL
vec4 SampleEnvMap(in vec3 r)
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
	mat3 TBN_mat = mat3(Tangent, Bitangent, VertNormal);
	vec3 FinalNorm  = TBN_mat * NM_Normal;
	FinalNorm = normalize(FinalNorm);
	return FinalNorm;
}

void main()
{		
	vec3 Norm = VertNormal;
	if(NormFlag == 1)
		Norm = NM_CalculateNormal();
	vec3 nrm = Norm/dot(Norm, Norm);
	vec3 L  = CameraPos - VertPosition;
	float eta = 1.0/1.33;
	float eta2 = eta * eta;
	
	float NdotL = dot(nrm, L);
	float NdotL2 = NdotL * NdotL;
	
	vec3 I = -normalize(L);

	vec3 refract;
	float K = 1.0 - eta2 * (1.0 - dot(nrm, I)*dot(nrm, I));
	if(K < 0.0)
		refract = vec3(0.0, 0.0, 0.0);
	else
		refract = eta * I - (eta * dot(nrm, I) + sqrt(K)) * nrm;

	vec3 reflect = (2.0*nrm*NdotL) - L;	

	vec3 R;
	//if reflection: R = reflect, if refraction: R = refract
	
	if(Option == 1)
		R = reflect;
	else
		R = refract;

	gl_FragColor = SampleEnvMap(R);
}