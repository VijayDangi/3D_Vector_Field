#version 430 core

in float v_normalizedValue;

out vec4 o_fragColor;

// A helper function to create a smooth color gradient based on a value from 0.0 to 1.0
vec3 colorMap(float t) {
    // Clamp t to ensure it stays between 0.0 and 1.0
    t = clamp(t, 0.0, 1.0);
    
    // A simple Blue -> Green -> Red gradient
    vec3 slowColor = vec3(0.0, 0.2, 1.0);   // Deep Blue
    vec3 midColor  = vec3(0.0, 1.0, 0.5);   // Spring Green
    vec3 fastColor = vec3(1.0, 0.1, 0.1);   // Bright Red
    
    // Mix the colors based on the threshold
    if (t < 0.5) {
        // Map 0.0-0.5 to Blue-Green
        return mix(slowColor, midColor, t * 2.0); 
    } else {
        // Map 0.5-1.0 to Green-Red
        return mix(midColor, fastColor, (t - 0.5) * 2.0); 
    }
}

void main() {
    vec3 finalColor = colorMap(v_normalizedValue);

    o_fragColor = vec4(finalColor, 1.0); // 1.0 is fully opaque
}