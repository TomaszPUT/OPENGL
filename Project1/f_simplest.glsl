#version 330 core

in vec2 iTexCoord;
in vec3 iNormal;
in vec3 iFragPos;

out vec4 pixelColor;

uniform vec4      objectColor;
uniform sampler2D tex;
uniform int       useTexture;
uniform float     lightIntensity;   // GLOWNE POKRETLO JASNOSCI (main.cpp)
uniform float     time;
uniform int       useProceduralFish;
uniform int       useProceduralPlant;
uniform int       isRock;
uniform int       isEmissive;
uniform int       isPearl;
uniform int       isBubble;
uniform int       isGold;
uniform vec3      camPos;          // pozycja kamery (orbitujacej) - do odbic i mgly
uniform float     pulseStrength;    // sila pulsowania diod (0 = brak, 1 = mocne)
uniform float     pulseSpeed;       // tempo pulsowania (wieksze = szybsze)
uniform float     sceneScale;       // skala calej sceny (do zachowania proporcji swiatla)
uniform float     bubbleClear;      // 0 = babel rownomierny, 1 = srodek przezroczysty (Fresnel)

// Dwa dolne, kolorowe zrodla swiatla (kuleczki). Pozycje i kolory podaje main.cpp.
uniform vec3 botLight1Pos; uniform vec3 botLight1Col;
uniform vec3 botLight2Pos; uniform vec3 botLight2Col;

float hash(vec2 p) { return fract(sin(dot(p, vec2(127.1, 311.7))) * 43758.5453); }

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
    float c = 1.0 - smoothstep(0.0, 0.06, abs(uv.x - 0.5));
    float s = pow(sin(uv.y * 22.0) * 0.5 + 0.5, 6.0);
    return clamp(c * 0.7 + s * 0.25, 0.0, 1.0);
}

vec3 sandColor(vec2 uv, vec3 texRGB) {
    float ripple = sin(uv.x * 55.0 + sin(uv.y * 9.0) * 3.0) * 0.5 + 0.5;
    ripple = pow(ripple, 2.0);
    float grain = hash(floor(uv * 420.0));
    vec3  base  = texRGB;
    base *= (0.80 + 0.30 * ripple);
    base += (grain - 0.5) * 0.10;
    base = mix(base, base * vec3(1.10, 1.00, 0.80), 0.4);
    return clamp(base, 0.0, 1.0);
}

vec3 rockColor(vec2 uv, vec3 texRGB) {
    vec2  s  = uv * 6.0;
    float n  = sin(s.x * 4.3 + s.y * 3.1) * 0.5 + 0.5;
    float m  = cos(s.x * 2.7 - s.y * 5.3) * 0.5 + 0.5;
    float rough = hash(floor(uv * 90.0)) * 0.22;
    vec3  base = texRGB;
    base *= (0.65 + 0.55 * (n * 0.6 + m * 0.4));
    base -= rough;
    return clamp(base, 0.0, 1.0);
}

// Kolorowe swiatlo punktowe (kuleczka na dnie). Swieci we wszystkie strony,
// wiec ryby NAD nia dostaja kolor od dolu.
vec3 pointLight(vec3 lpos, vec3 lcol, vec3 norm) {
    vec3  d    = lpos - iFragPos;
    float dist = length(d) / sceneScale;   // dzielimy przez skale -> te same proporcje przy wiekszym akwarium
    float att  = 1.0 / (1.0 + 0.20 * dist + 0.30 * dist * dist);
    float diff = max(dot(norm, normalize(d)), 0.0);
    return (diff * 0.85 + 0.15) * lcol * att * 11.0;
}

void main(void) {

    if (isEmissive == 1) {            // lampa gorna i kuleczki-swiatla: czysty kolor
        pixelColor = vec4(objectColor.rgb, objectColor.a);
        return;
    }

    vec3 norm    = normalize(iNormal);
    vec3 viewDir = normalize(camPos - iFragPos);
    float fresnel = pow(1.0 - max(dot(norm, viewDir), 0.0), 3.0);

    // ── GORNE BIALE SWIATLO - tylko GORNA POLOWA akwarium ──
    // Wygaszamy je z wysokoscia: pelne u gory, znika ok. polowy zbiornika.
    vec3  Lpos   = vec3(0.0, 2.6, 0.0) * sceneScale;
    vec3  Ldir   = normalize(Lpos - iFragPos);
    float Ldist  = length(Lpos - iFragPos) / sceneScale;
    float Latt   = 1.0 / (1.0 + 0.05 * Ldist + 0.010 * Ldist * Ldist);
    float topReach = smoothstep(-0.3, 1.3, iFragPos.y / sceneScale);   // 0 w dolnej polowie, 1 u gory
    float Ldiff  = max(dot(norm, Ldir), 0.0);
    vec3  Lcol   = vec3(1.00, 0.96, 0.86);
    vec3  Ldif   = Ldiff * Lcol * Latt * 9.0 * topReach;
    vec3  Lrefl  = reflect(-Ldir, norm);
    float Lspec  = pow(max(dot(viewDir, Lrefl), 0.0), 48.0);
    vec3  Lsp    = Lspec * Lcol * Latt * topReach;

    // ── DWA DOLNE KOLOROWE SWIATLA (kuleczki) + sterowane pulsowanie ──
    // pulse = 1 gdy sila=0; przy sile>0 oscyluje. Tempo = pulseSpeed.
    float pulse = (1.0 - pulseStrength) + pulseStrength * (0.5 + 0.5 * sin(time * pulseSpeed));
    vec3 b1 = pointLight(botLight1Pos, botLight1Col, norm) * pulse;
    vec3 b2 = pointLight(botLight2Pos, botLight2Col, norm) * pulse;

    vec3 ambient = vec3(0.05, 0.07, 0.10);   // ciemno - swiatla robia robote

    vec3 lighting = (ambient + Ldif + Lsp + b1 + b2) * lightIntensity;
    lighting = clamp(lighting, 0.0, 1.0);

    if (isBubble == 1) {


        float rim = pow(1.0 - max(dot(norm, viewDir), 0.0), 2);
        vec3  col = mix(vec3(0.55, 0.75, 1.0), vec3(1.0), rim);  // brzeg bielszy
        col += Lsp * 1.5;                                       // maly blik na bance
        

        float base = objectColor.a;                            // losowe krycie z C++ (0..0.3)
        float a = base * mix(1.0, rim, bubbleClear) + rim * 0.15 * bubbleClear;
        pixelColor = vec4(col, clamp(a, 0.0, 0.9));
        return;
    }

    vec3  col;
    float alpha = objectColor.a;

    if (isPearl == 1) {
        vec3 irid = 0.5 + 0.5 * cos(vec3(0.0, 1.6, 3.2) + fresnel * 4.0 + norm.y * 2.0);
        vec3 base = mix(vec3(0.80, 0.74, 0.60), irid, 0.18);
        col   = base * lighting + Lsp * 1.2 + fresnel * vec3(0.20, 0.22, 0.26);
        alpha = 1.0;

    } else if (useProceduralFish == 1) {
        float sv     = scalePattern(iTexCoord);
        vec3  base   = objectColor.rgb;
        vec3  bright = mix(base, vec3(1.0), 0.30);
        vec3  dark   = base * 0.28;
        vec3  sc     = mix(bright, dark, sv);
        sc += Lsp * base * 0.4;
        col   = lighting * sc;
        alpha = 1.0;

    } else if (useProceduralPlant == 1) {
        float vein  = veinPattern(iTexCoord);
        vec3  base  = objectColor.rgb;
        vec3  vc    = mix(base, base + vec3(0.04, 0.30, 0.06), vein);
        vec3  pLight = vec3(0.08, 0.13, 0.10)
                     + Lcol * 0.9 * Latt * topReach
                     + b1 * 0.7 + b2 * 0.7;
        col   = clamp(pLight * lightIntensity, 0.0, 1.0) * vc;

    } else if (isGold == 1) {
        col  = lighting * objectColor.rgb;
        col += Lsp * 2.5;
        alpha = 1.0;

    } else if (isRock == 1) {
        vec3 t = texture(tex, iTexCoord).rgb;
        col  = lighting * rockColor(iTexCoord, t);
        col += Lsp * 0.4;
        alpha = 1.0;

    } else if (useTexture == 1) {
        // DNO: sam piasek (kafelek x3). Bez okregow, bez kaustyki - czysto i ciemno.
        vec2 suv = iTexCoord * 3.0;
        vec3 t   = texture(tex, suv).rgb;
        col  = lighting * sandColor(suv, t);
        alpha = objectColor.a;

    } else {
        col   = lighting * objectColor.rgb;
        alpha = objectColor.a;
    }

    // Mgla wodna - ciemna ton w glebi
    float dist = length(camPos - iFragPos) / sceneScale;
    float fog  = clamp((dist - 12.0) / 16.0, 0.0, 0.7);
    vec3  haze = vec3(0.015, 0.03, 0.06);
    col = mix(col, haze, fog);

    pixelColor = vec4(clamp(col, 0.0, 1.0), alpha);
}
