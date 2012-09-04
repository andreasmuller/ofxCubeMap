varying vec3  texcoord;

void main(void) 
{
 	vec4	texcoord0 = gl_ModelViewMatrix * gl_Vertex;
	//texcoord = texcoord0.xyz;
	texcoord = normalize(gl_Vertex.xyz);

    gl_Position    = ftransform();
   
}