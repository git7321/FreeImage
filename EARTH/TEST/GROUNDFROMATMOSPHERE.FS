// Atmospheric scattering fragment shader
// Author: Sean O'Neil
// Copyright (c) 2004 Sean O'Neil

#version 120

uniform sampler2D s2Tex1;
uniform sampler2D s2Tex2;
//uniform sampler2D s2Tex3;

void main (void)
{
	vec3 Clouds = texture2D(s2Tex2, gl_TexCoord[0].st).rgb;
	gl_FragColor = vec4(gl_Color.rgb + mix(texture2D(s2Tex1, gl_TexCoord[0].st).rgb, Clouds, dot(vec3(0.33), Clouds)) * gl_SecondaryColor.rgb, 1.0);
}
