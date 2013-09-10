varying vec3 position;
varying vec3 normal;

varying vec3 tangent2;
varying vec3 bitangent2;

uniform vec3 LightPosition;
uniform vec3 CameraPosition;
uniform vec4 lightDiffuseColor;
uniform vec4 lightSpecColor;

uniform sampler2D Texture;
uniform sampler2D NormalMap;
uniform sampler2D HeightMap;


vec3 NM_CalculateNormal()
{
	vec3 NM_Normal = texture2D(NormalMap, gl_TexCoord[0].st).xyz;
	NM_Normal = 2.0 * NM_Normal - vec3(1.0, 1.0, 1.0);
	vec3 normal2;
	mat3 TBN_mat = mat3(tangent2, bitangent2, normal);
	normal2  = TBN_mat * NM_Normal;
	normal2 = normalize(normal2);
	return normal2;
}

void main()
{
	vec3 N = NM_CalculateNormal();
	vec3 E = normalize(-position); //CameraPosition - LightPosition = 0 with light at camera
	vec3 L = E;//normalize( (LightPosition - CameraPosition) - position);
	vec3 R = normalize(-reflect(L,N));
	
	vec4 Iamb = vec4(0.0);
	
	vec4 Idiff = lightDiffuseColor * (texture2D(Texture, gl_TexCoord[0].st) * gl_Color) * max(dot(N, L), 0.0);
	Idiff = clamp(Idiff, 0.0, 1.0);
	
	float NS = 45.0;
	vec4 matSpecColor = texture2D(Texture, gl_TexCoord[0].st) * gl_Color;
	vec4 Ispec = lightSpecColor * matSpecColor * pow(max(dot(R, E), 0.0), NS);
	Ispec = clamp(Ispec, 0.0, 1.0);
	
	gl_FragColor = (Iamb + Idiff + Ispec); //texture & light
} 