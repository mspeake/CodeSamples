#define LIGHT_NUM_MAX		16

varying vec3 position;
varying vec3 normal;

varying vec3 tangent2;
varying vec3 bitangent2;

struct Light
{
	vec3 pos;

	vec4 ambient;
	vec4 diffuse;
	vec4 specular;
	
	vec3 DistAtt;
};

uniform int LightNum;
uniform Light LIGHTS[LIGHT_NUM_MAX];

uniform vec3 CameraPosition;

uniform vec3 lightDistAtt;

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
	vec3 E = normalize(-position);
	
	gl_FragColor = vec4(0.0);
	
	for (int i = 0; i < LIGHT_NUM_MAX; ++i)
	{
		if(i >= LightNum)
			break;
			
		vec3 L = LIGHTS[i].pos - position;
		float light_len = length(L);
		L /= light_len;
		vec3 R = normalize(-reflect(L,N));
		
		float const_att = LIGHTS[i].DistAtt.x,
			  linear_att = LIGHTS[i].DistAtt.y,
			  quadratic_att = LIGHTS[i].DistAtt.z;
		
		if(light_len > 230.0)
			light_len = 230.0;
		
		float att = 1.0 / ( const_att + 
		(linear_att * light_len) + 
		(quadratic_att * light_len * light_len) );
		
		float lambertTerm = dot(N,L);
		
		vec4 Iamb = vec4(0.0); //LIGHTS[i].ambient * att;//
		vec4 Idiff = vec4(0.0);
		vec4 Ispec = vec4(0.0);
		
		float NS = 17.0; //45.0;  //17.0 is good for a non-shiny surface
		vec4 matDiffColor = texture2D(Texture, gl_TexCoord[0].st) * gl_Color;
		vec4 matSpecColor = texture2D(HeightMap, gl_TexCoord[0].st) * gl_Color;
		
		if(lambertTerm > 0.0)
		{
			Idiff = LIGHTS[i].diffuse * matDiffColor * lambertTerm * att;
			Idiff = clamp(Idiff, 0.0, 1.0);
			
			Ispec = LIGHTS[i].specular * matSpecColor * pow(max(dot(R, E), 0.0), NS) * att;
			Ispec = clamp(Ispec, 0.0, 1.0);
		}
		
		vec4 LightTotal = (Iamb + Idiff + Ispec);
		gl_FragColor += LightTotal;
	}
	
	gl_FragColor /= LightNum; //texture & light
} 