// ---------------------------------------------------------------------------
// varyings

varying vec3 VertNormal;
varying vec3 VertPosition;

// ---------------------------------------------------------------------------

void main()
{		
	vec3 Nrm = VertNormal / dot(VertNormal, VertNormal);
	gl_FragColor = vec4(Nrm, 1.0);
}


