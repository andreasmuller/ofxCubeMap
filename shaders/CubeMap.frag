uniform samplerCube EnvMap;

varying vec3  texcoord;

void main (void)
{
   
    vec3 envColor = vec3 (textureCube(EnvMap, texcoord));
	//vec3 envColor = vec3 (textureCube(EnvMap, gl_TexCoord[0]));

    gl_FragColor = vec4 (envColor, 1.0);
}