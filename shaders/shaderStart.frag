#version 410 core

in vec3 fNormal;
in vec4 fPosEye;
in vec2 fTexCoords;
in vec4 fragPosLightSpace;

out vec4 fColor;

//lighting direct
uniform vec3 lightDir;
uniform vec3 lightColor;
uniform bool lightOn;


//lighting point
uniform vec3 lightPct;      
uniform vec3 lightPctColor; 
uniform bool pctLightOn;

//effects
uniform bool pointLightEffect;
uniform vec3 light1PctColor;
uniform vec3 light1Pct;

//fog
uniform bool enableFog;

//texture
uniform sampler2D diffuseTexture;
uniform sampler2D specularTexture;
uniform sampler2D shadowMap;

vec3 ambient;
float ambientStrength = 0.2f;
vec3 diffuse;
vec3 specular;
float specularStrength = 0.5f;
float shininess = 32.0f;
float shadow;

float computeFog()
{
    float fogDensity = 0.05f;
    float fragmentDistance = length(fPosEye);
    float fogFactor = exp(-pow(fragmentDistance * fogDensity, 2));

    return clamp(fogFactor, 0.0f, 1.0f);
}



void computeLightDirectional()
{		
    vec3 cameraPosEye = vec3(0.0f);
    vec3 normalEye = normalize(fNormal);	
	
    vec3 lightDirN = normalize(lightDir);
	
    vec3 viewDirN = normalize(cameraPosEye - fPosEye.xyz);
		
    // ambient light
    ambient = ambientStrength * lightColor;
	
    // diffuse light
    diffuse = max(dot(normalEye, lightDirN), 0.0f) * lightColor;
	
    // specular light
    vec3 reflection = reflect(-lightDirN, normalEye);
    float specCoeff = pow(max(dot(viewDirN, reflection), 0.0f), shininess);
    specular = specularStrength * specCoeff * lightColor;
}


void computeLightPoint() {
    vec3 cameraPosEye = vec3(0.0f);
    vec3 normalEye = normalize(fNormal);
    
    vec3 lightVec = lightPct - fPosEye.xyz;
    float distance = length(lightVec);
    vec3 lightDirN = normalize(lightVec);
    
    // control the light's reach
    float constant = 1.0f;
    float linear = 0.09f;
    float quadratic = 0.032f;
    float attenuation = 1.0f / (constant + linear * distance + quadratic * (distance * distance));

    // view direction
    vec3 viewDirN = normalize(- fPosEye.xyz);

    // ambient light
    ambient += ambientStrength * lightPctColor * attenuation;

    // diffuse light
    float diff = max(dot(normalEye, lightDirN), 0.0f);
    diffuse += diff * lightPctColor * attenuation;

    // specular light
    vec3 reflection = reflect(-lightDirN, normalEye);
    float specCoeff = pow(max(dot(viewDirN, reflection), 0.0f), shininess);
    specular += specularStrength * specCoeff * lightPctColor * attenuation;
}


float computeShadow()
{
    // perform perspective divide
	vec3 normalizedCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
	// Transform to [0,1] range
	normalizedCoords = normalizedCoords * 0.5 + 0.5;
	// Get closest depth value from light's perspective
	float closestDepth = texture(shadowMap, normalizedCoords.xy).r;
	// Get depth of current fragment from light's perspective
	float currentDepth = normalizedCoords.z;
	if (normalizedCoords.z > 1.0f)
		return 0.0f;
	// Check whether current frag pos is in shadow
	float bias = 0.005f;
	float shadow = currentDepth - bias > closestDepth ? 1.0f : 0.0f;
	return shadow;

}


void main() 
{
    if (lightOn) {
        computeLightDirectional();
    }

    if (pctLightOn) {
        computeLightPoint();
    }

    
    vec3 baseColor = vec3(0.9f, 0.35f, 0.0f); // Orange color
    
    ambient *= texture(diffuseTexture, fTexCoords).rgb;
    diffuse *= texture(diffuseTexture, fTexCoords).rgb;
    specular *= texture(specularTexture, fTexCoords).rgb;
    shadow = computeShadow();

    // Final color without fog
    vec3 color = min((ambient + (1.0f - shadow) * diffuse) + (1.0f - shadow) * specular, 1.0f);
    
    // Compute fog factor
    float fogFactor = enableFog ? computeFog() : 1.0f;
    vec4 fogColor = vec4(0.5f, 0.5f, 0.5f, 1.0f); // Light gray fog
    
    //Fragment discard
    vec4 colorFromTexture = texture(diffuseTexture, fTexCoords);
    if(colorFromTexture.a < 0.5)
        discard;
    vec4 colorWithAlpha = vec4(color, 1.0f) * colorFromTexture;


    // Blend the color with fog
    fColor = mix(fogColor, colorWithAlpha, fogFactor);
    fColor.a = 0.8f;
}
