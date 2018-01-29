varying vec3 lightDir, normal;
varying vec4 position;

void main()
{
	float intensity;
	vec4 color;
	vec4 outline = vec4(0.0, 0.0, 0.0, 1.0);
	vec4 finalColor;
	vec3 n = normalize(normal);
	vec3 l = normalize(lightDir);
	intensity = dot(lightDir,n);

	//vec3 viewDirection = normalize(vec3(0.0, 5.0, 5.0) - vec3(position));
	vec3 viewDirection = normalize(vec3(0.0, 0.0, 5.0) - vec3(position));
	float camDir = dot(viewDirection, n);
	if(camDir < mix(0.6, 0.1, max(0, dot(n, l)))) //< 0.3
		finalColor = outline;
	else
	{
		if (intensity > 0.9)
			color = vec4(0.5, 0.5, 1.0, 1.0);
		else if (intensity > 0.5)
			color = vec4(0.3, 0.3, 0.6, 1.0);
		else if (intensity > 0.25)
			color = vec4(0.2, 0.2, 0.4, 1.0);
		else
			color = vec4(0.1, 0.1, 0.2, 1.0);
	
		finalColor = color;
		//finalColor = vec4(0.5, 0.5, 1.0, 1.0);
	}
	//gl_FragColor = vec4(camDir);
	//gl_FragColor = outline * (1.0 - camDir);
	//gl_FragColor = color * camDir + outline * (1.0 - camDir);
	gl_FragColor = finalColor;
} 
