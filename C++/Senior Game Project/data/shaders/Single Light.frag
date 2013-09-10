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

uniform Light LIGHT;

uniform vec3 LightPosition;
uniform vec3 CameraPosition;

uniform vec4 lightAmbientColor;
uniform vec4 lightDiffuseColor;
uniform vec4 lightSpecColor;

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
	
	//gl_FragColor = texture2D(Texture, gl_TexCoord[0].st) * gl_Color;
	gl_FragColor = vec4(0.0);
	
	vec3 L = LIGHT.pos - position;
	float light_len = length(L);
	L /= light_len;
	vec3 R = normalize(-reflect(L,N));
	
	float const_att = LIGHT.DistAtt.x,
		  linear_att = LIGHT.DistAtt.y,
		  quadratic_att = LIGHT.DistAtt.z;
		  
	if(light_len > 190.0)
		light_len = 190.0;
	
	float att = 1.0 / ( const_att + 
	(linear_att * light_len) + 
	(quadratic_att * light_len * light_len) );
	
	float att_2 = 1.0 / ( const_att +
	(linear_att * light_len) +
	(0.000055 * light_len * light_len) );
	
	float lambertTerm = dot(N,L);
	
	vec4 Iamb = vec4(0.0); //LIGHT.ambient * att;//
	vec4 Idiff = vec4(0.0);
	vec4 Ispec = vec4(0.0);
	
	float NS = 17.0;
	vec4 matDiffColor = texture2D(Texture, gl_TexCoord[0].st) * gl_Color;
	vec4 matSpecColor = texture2D(HeightMap, gl_TexCoord[0].st) * gl_Color;
	
	if(lambertTerm > 0.0)
	{
		Idiff = LIGHT.diffuse * matDiffColor * lambertTerm;// * att;
		Idiff = clamp(Idiff, 0.0, 1.0);
		
		Ispec = LIGHT.specular * matSpecColor * pow(max(dot(R, E), 0.0), NS);// * att;
		Ispec = clamp(Ispec, 0.0, 1.0);
	}
	
	vec4 LightTotal = (Iamb + Idiff + Ispec);
	gl_FragColor += LightTotal * att_2;
} 