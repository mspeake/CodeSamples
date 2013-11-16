varying vec3 VertPosition;
varying vec3 VertNormal;

varying vec3 Tangent;
varying vec3 Bitangent;

uniform vec3 LightPosition;
uniform vec3 CameraPosition;
uniform vec4 lightDiffuseColor;
uniform vec4 lightSpecColor;

uniform sampler2D Texture;
uniform sampler2D NormalMap;


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
	vec3 N = NM_CalculateNormal();
	vec3 E = normalize(-VertPosition);
	vec3 L = E;
	vec3 R = normalize(-reflect(L,N));
	
	vec4 Iamb = vec4(0.0);
	
	vec4 Idiff = lightDiffuseColor * (texture2D(Texture, gl_TexCoord[0].st) * gl_Color) * max(dot(N, L), 0.0);
	Idiff = clamp(Idiff, 0.0, 1.0);
	
	float NS = 45.0;
	vec4 matSpecColor = texture2D(Texture, gl_TexCoord[0].st) * gl_Color;
	vec4 Ispec = lightSpecColor * matSpecColor * pow(max(dot(R, E), 0.0), NS);
	Ispec = clamp(Ispec, 0.0, 1.0);
	
	gl_FragColor = (Iamb + Idiff + Ispec);
} 