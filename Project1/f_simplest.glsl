#version 330 core

in vec2 iTexCoord;
in vec3 iNormal;
in vec3 iFragPos;

out vec4 pixelColor;

uniform vec4      objectColor;
uniform sampler2D tex;
uniform int       useTexture;
uniform float     lightIntensity;
uniform int       useProceduralFish;
uniform int       useProceduralPlant;
uniform int       isRock;
uniform int       isEmissive;   // 1 = lampa akwariowa (świeci bez obliczeń)

// ─── WZORY PROCEDURALNE ────────────────────────────────

float scalePattern(vec2 uv) {
    vec2  sc    = uv * vec2(8.0, 12.0);
    float shift = mod(floor(sc.x), 2.0) * 0.5;
    vec2  cell  = fract(sc + vec2(0.0, shift));
    float d     = length(cell - 0.5);
    float edge  = smoothstep(0.35, 0.50, d);
    float shine = 1.0 - smoothstep(0.0, 0.15, d);
    return edge - shine * 0.3;
}

float veinPattern(vec2 uv) {
    float c = 1.0 - smoothstep(0.0, 0.08, abs(uv.x - 0.5));
    float s = pow(sin(uv.y * 20.0) * 0.5 + 0.5, 8.0);
    return clamp(c * 0.6 + s * 0.3, 0.0, 1.0);
}

vec3 sandColor(vec2 uv) {
    vec2  s = uv * 12.0;
    float n = sin(s.x * 3.7 + s.y * 2.3) * 0.5 + 0.5;
    float m = cos(s.x * 1.9 - s.y * 5.1) * 0.5 + 0.5;
    float k = sin((s.x + s.y) * 4.3)     * 0.5 + 0.5;
    return mix(vec3(0.55, 0.42, 0.24), vec3(0.88, 0.74, 0.50),
               n * 0.5 + m * 0.3 + k * 0.2);
}

vec3 rockColor(vec2 uv) {
    vec2  s = uv * 7.0;
    float n = sin(s.x * 4.3 + s.y * 3.1) * 0.5 + 0.5;
    float m = cos(s.x * 2.7 - s.y * 5.3) * 0.5 + 0.5;
    float k = sin((s.x + s.y) * 7.1)     * 0.5 + 0.5;
    return mix(vec3(0.22, 0.20, 0.18), vec3(0.55, 0.52, 0.48),
               n * 0.4 + m * 0.35 + k * 0.25);
}

void main(void) {

    // Lampa akwariowa: brak obliczeń — czyste świecenie
    if (isEmissive == 1) {
        pixelColor = vec4(objectColor.rgb, objectColor.a);
        return;
    }

    vec3 norm    = normalize(iNormal);
    vec3 viewDir = normalize(vec3(0.0, 0.0, -15.0) - iFragPos);

    // ═══════════════════════════════════════════════════════
    //  ŹRÓDŁO ŚWIATŁA 1 — Lampa akwariowa
    //  Pozycja: tuż nad akwarium, ciepłe białe światło
    // ═══════════════════════════════════════════════════════
    vec3  L1pos  = vec3(0.0, 2.3, 0.3);
    vec3  L1dir  = normalize(L1pos - iFragPos);
    float L1dist = length(L1pos - iFragPos);
    float L1att  = 1.0 / (1.0 + 0.05 * L1dist + 0.003 * L1dist * L1dist);
    float L1diff = max(dot(norm, L1dir), 0.0);
    vec3  L1col  = vec3(1.00, 0.96, 0.85);
    vec3  L1dif  = L1diff * L1col * L1att * 5.5;
    // Odbłysk spekullarny
    vec3  L1refl = reflect(-L1dir, norm);
    float L1spec = pow(max(dot(viewDir, L1refl), 0.0), 48.0);
    vec3  L1sp   = L1spec * L1col * 0.80 * L1att;

    // ═══════════════════════════════════════════════════════
    //  ŹRÓDŁO ŚWIATŁA 2 — Boczne niebieskie
    //  Symuluje odbicia światła przez szkło i wodę
    // ═══════════════════════════════════════════════════════
    vec3  L2pos  = vec3(5.5, 1.0, 0.0);
    vec3  L2dir  = normalize(L2pos - iFragPos);
    float L2dist = length(L2pos - iFragPos);
    float L2att  = 1.0 / (1.0 + 0.07 * L2dist + 0.005 * L2dist * L2dist);
    float L2diff = max(dot(norm, L2dir), 0.0);
    vec3  L2col  = vec3(0.15, 0.45, 0.95);
    vec3  L2dif  = L2diff * L2col * L2att * 1.8;
    vec3  L2refl = reflect(-L2dir, norm);
    float L2spec = pow(max(dot(viewDir, L2refl), 0.0), 24.0);
    vec3  L2sp   = L2spec * L2col * 0.30 * L2att;

    // Ambient — niebieskawy odcień wody
    vec3 ambient = vec3(0.30, 0.38, 0.50);

    // Efekt głębi — dno nieco ciemniejsze (min 0.60, nie 0.28!)
    float depth = clamp((iFragPos.y + 2.0) / 4.5, 0.60, 1.0);

    vec3 lighting = (ambient + L1dif + L1sp + L2dif + L2sp)
                    * lightIntensity * depth;
    lighting = clamp(lighting, 0.0, 1.0);

    // ═══════════════════════════════════════════════════════
    //  KOLOR KOŃCOWY
    // ═══════════════════════════════════════════════════════

    if (useProceduralFish == 1) {
        float sv     = scalePattern(iTexCoord);
        vec3  base   = objectColor.rgb;
        vec3  bright = mix(base, vec3(1.0), 0.30);
        vec3  dark   = base * 0.28;
        vec3  sc     = mix(bright, dark, sv);
        sc += (L1sp + L2sp) * base * 0.4;
        pixelColor = vec4(lighting * sc, 1.0);

    } else if (useProceduralPlant == 1) {
        // ─────────────────────────────────────────────────
        // KLUCZOWA POPRAWKA: normalna płaskiego liścia jest
        // ZAWSZE prostopadła do obu świateł → dot() = 0
        // Dlatego używamy stałego oświetlenia dla roślin!
        // ─────────────────────────────────────────────────
        float vein  = veinPattern(iTexCoord);
        vec3  base  = objectColor.rgb;
        vec3  vc    = mix(base, base + vec3(0.02, 0.22, 0.02), vein);

        // Stałe oświetlenie — niezależne od normalnej
        vec3  pLight = vec3(0.55, 0.75, 0.55)   // zielona poświata własna
                     + L1col * 0.55 * L1att      // lampa górna
                     + L2col * 0.20 * L2att;     // lampa boczna
        pLight = clamp(pLight * lightIntensity, 0.0, 1.0);
        pixelColor = vec4(pLight * vc, objectColor.a);

    } else if (isRock == 1) {
        pixelColor = vec4(lighting * rockColor(iTexCoord), 1.0);

    } else if (useTexture == 1) {
        vec4  t   = texture(tex, iTexCoord);
        float b   = dot(t.rgb, vec3(0.333));
        vec3  col = (b > 0.88 && t.r > 0.82) ? sandColor(iTexCoord) : t.rgb;
        pixelColor = vec4(lighting * col, objectColor.a);

    } else {
        pixelColor = vec4(lighting * objectColor.rgb, objectColor.a);
    }
}