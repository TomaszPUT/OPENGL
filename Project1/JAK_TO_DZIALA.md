# Akwarium 3D — jak to działa (ściąga pod obronę)

Projekt: animowane akwarium (OpenGL 3.3 core, GLEW, GLFW, GLM, lodepng).
Pływające ryby, kołyszące się rośliny, bąbelki, oświetlenie, model wczytany z pliku.

---

## 1. Pliki

- `main.cpp` — inicjalizacja okna/OpenGL, pętla główna, rysowanie sceny, sterowanie.
- `params.cpp` / `params.h` — wszystkie proste ustawienia (liczba obiektów, jasność,
  kolory świateł, pulsowanie, model, wielkość akwarium).
- `v_simplest.glsl` / `f_simplest.glsl` — vertex i fragment shader (geometria + kolor/światło).
- `Cube` — pudełko (akwarium, dno, woda, szkło, podstawa, lampa).
- `Fish` — ryba (kula jako ciało + ostrosłup jako ogon).
- `Rock` — kamień (zniekształcona kula).
- `Plant` — roślina (kępa łodyg z kołyszących się prostokątów).
- `Bubbles` — strumień bąbelków.
- `Model` — wczytywanie siatki 3D z pliku `.obj`.
- `shaderprogram`, `lodepng` — biblioteki z zajęć (kompilacja shaderów, wczytywanie PNG).

---

## 2. Potok rysowania (funkcja `drawScene`)

Każdą klatkę czyścimy ekran i rysujemy scenę w 4 etapach. Kolejność jest celowa,
głównie ze względu na przezroczystość:

1. **Obiekty nieprzezroczyste** — lampa, kuleczki-światła, dno (piasek), podstawa,
   kamienie, model, ryby. Zapis do bufora głębi włączony.
2. **Rośliny** — rysowane po obiektach stałych.
3. **Szkielet akwarium** — krawędzie pudełka (tryb linii).
4. **Przezroczyste** — bąbelki, woda, szkło.

---

## 3. Przezroczystość szkła i wody (częste pytanie!)

Szkło i woda to półprzezroczyste ściany pudełka. Żeby wyglądały dobrze, robimy trzy rzeczy:

1. **Kolejność rysowania.** Najpierw wszystko, co nieprzezroczyste (zapisuje głębię),
   a szkło i wodę **na samym końcu**. Dzięki temu mieszają się z gotowym obrazem tego,
   co jest za nimi.
2. **Mieszanie kolorów (blending).** Włączamy `glBlendFunc(GL_SRC_ALPHA,
   GL_ONE_MINUS_SRC_ALPHA)`. Kolor końcowy = kolor szkła·alfa + tło·(1−alfa).
   Szkło ma bardzo niską alfę (ok. 0.07), woda trochę wyższą (ok. 0.24) — stąd są
   ledwo widoczne, ale dają refleks i niebieskawy odcień.
3. **Wyłączenie zapisu głębi dla przezroczystych** — `glDepthMask(GL_FALSE)`.
   Test głębi nadal działa (przezroczyste nie przebijają obiektów przed nimi),
   ale **nie zapisują** swojej głębi, więc nie zasłaniają się nawzajem i nie ucinają
   tego, co za nimi. Po etapie przezroczystym wracamy `glDepthMask(GL_TRUE)`.

Dodatkowo wodę rysujemy przed szkłem (od środka na zewnątrz), więc kolejność mieszania
jest naturalna.

---

## 4. Oświetlenie

Model oświetlenia liczymy we fragment shaderze (per piksel). To uproszczony model Phonga:

- **Ambient** — stałe, słabe światło tła (scena jest celowo ciemna).
- **Diffuse** — `max(dot(N, L), 0)`: im bardziej powierzchnia zwrócona do światła,
  tym jaśniejsza (N = normalna, L = kierunek do światła).
- **Specular** — `pow(max(dot(V, R), 0), n)`: jasny błysk od strony obserwatora
  (V = kierunek do kamery, R = odbity promień).
- **Tłumienie (attenuation)** — `1/(1 + a·d + b·d²)`: światło słabnie z odległością `d`.

Mamy trzy źródła:

1. **Górna lampa** — białe światło z góry. Wygaszamy je z wysokością
   (`smoothstep` po `iFragPos.y`), więc oświetla tylko górną połowę akwarium — ryby.
2. i 3. **Dwie dolne diody** — kolorowe światła punktowe (domyślnie niebieskie i czerwone)
   reprezentowane przez świecące kuleczki na dnie. Rzucają kolor do góry na ryby.
   Mają lekkie **pulsowanie** sterowane parametrami `pulseStrength` i `pulseSpeed`.

Jasność całości reguluje `lightIntensity` (klawisze Q/E w trakcie).

---

## 5. Materiały (flagi w shaderze)

Jeden shader rysuje wszystko; o sposobie kolorowania decydują uniformy-przełączniki
ustawiane przed każdym obiektem: `isEmissive` (świeci sam — lampa, kuleczki),
`isRock` (kamień z teksturą), `useTexture` (piasek), `useProceduralFish`/`Plant`
(wzór łusek / liścia), `isGold` (podstawa), `isBubble` (bąbel). Gdy żadna nie jest
ustawiona, obiekt dostaje zwykły kolor + oświetlenie.

---

## 6. Kamera i sterowanie

Kamera **orbituje** wokół środka sceny — obracamy widok, a nie scenę (to ważne:
dzięki temu światła trzymają się akwarium i kolory są poprawne z każdej strony,
także z góry). Pozycję kamery liczymy z dwóch kątów (`camPitch`, `camYaw`) na okręgu.

Sterowanie:
- **Prawy przycisk myszy** (przytrzymaj i ruszaj) — obrót kamery.
- Strzałki — obrót kamery; O/P — zoom (kąt widzenia).
- Q / E — jaśniej / ciemniej.
- Przytrzymaj I — przewijanie koloru diod (obrót odcienia w HSV 0–360).
- ESC — wyjście.

---

## 7. Model z pliku `.obj`

Klasa `Model` czyta plik Wavefront OBJ: linie `v` (pozycje), `vt` (tekstura),
`vn` (normalne) i `f` (ściany). Składamy z tego jedną tablicę wierzchołków i rysujemy
przez `glDrawArrays`. Parser radzi sobie z różnymi formatami ścian (`v`, `v/t`, `v//n`,
`v/t/n`), czworokątami (dzieli je na trójkąty) i brakiem normalnych (liczy własne).

Po wczytaniu model jest **automatycznie centrowany i stawiany spodem na y=0** oraz
skalowany do rozmiaru 1. Dlatego dowolny model z internetu po ustawieniu pozycji
po prostu stoi/leży na dnie, a nie lewituje.

---

## 8. Wielkość akwarium

`aquariumScale` w `params.cpp` skaluje **całą scenę** naraz (mnożymy macierz `baseM`
przez skalę, a `baseM` jest używana przy każdym obiekcie). Pozycje świateł i odległości
w shaderze dzielimy przez tę skalę, żeby proporcje oświetlenia zostały takie same
przy większym i mniejszym akwarium. Kamera odsuwa się proporcjonalnie.

---

## 9. Obiekty i animacja

- **Ryby** — ciało to kula, ogon to ostrosłup. Co klatkę przesuwają się o prędkość
  i odbijają od ścian (zmiana znaku prędkości na granicy). Obracają się w stronę ruchu.
- **Rośliny** — kępa kilku łodyg; każdy segment lekko obracamy sinusem czasu, kąt rośnie
  ku górze, więc roślina się kołysze. Każda łodyga ma własną fazę.
- **Kamienie** — kula, której promień modulujemy kilkoma sinusami z losową fazą,
  więc każdy kamień jest inny i nieregularny.
- **Bąbelki** — unoszą się ku górze (lekko przyspieszając), delikatnie dryfują w bok,
  po dotarciu do powierzchni wracają na dół. Mają losową, niską przezroczystość.

---

## 10. Zgodność z regulaminem

- **Brak przestarzałych funkcji** — żadnego `glBegin/glVertex/glTranslate/gluLookAt`.
  Geometria w VAO/VBO/EBO, rysowanie `glDrawArrays`/`glDrawElements`, macierze z GLM
  (`translate`, `rotate`, `scale`, `lookAt`, `perspective`).
- **Własne rysowanie** — cały kod rysujący jest nasz; nie używamy gotowych procedur
  rysujących z bibliotek.
- **Model nietrywialny z pliku** — klasa `Model` + plik `.obj`.
- **Co najmniej dwie tekstury** — `sand.png` (dno) i `rock.png` (kamienie), z plików (lodepng).
- **Co najmniej dwa światła** — górne + dwa dolne kolorowe.
- **Animacja** — ryby, rośliny, bąbelki, pulsowanie, ruch kamery.
- **Brak koloru różowego** — losowy kolor ryb jest korygowany, by nie był różowy.
- **Ponad 4 niezależne obiekty 3D** — ryby, kamienie, rośliny, bąbelki, model, akwarium,
  dno, podstawa, lampa, woda, szkło.

---

## 11. Możliwe pytania profesora i odpowiedzi

**Jak działa przezroczystość szkła?** Patrz sekcja 3: rysujemy je na końcu, z włączonym
blendingiem i wyłączonym zapisem głębi (`glDepthMask(GL_FALSE)`), niska alfa.

**Czym jest VAO/VBO/EBO?** VBO trzyma wierzchołki na karcie graficznej, EBO indeksy
(które wierzchołki tworzą trójkąty), a VAO zapamiętuje układ tych danych (atrybuty:
pozycja, tekstura, normalna).

**Co to są atrybuty 0/1/2?** Dane wierzchołka: 0 = pozycja (x,y,z), 1 = współrzędne
tekstury (u,v), 2 = normalna (nx,ny,nz). Ustawiamy je przez `glVertexAttribPointer`.

**Jak liczycie oświetlenie?** Model Phonga we fragment shaderze: ambient + diffuse
(`dot(N,L)`) + specular (`reflect`) + tłumienie z odległością. Patrz sekcja 4.

**Po co normalne?** Mówią, w którą stronę „patrzy" powierzchnia — bez nich nie da się
policzyć światła rozproszonego (`dot(N,L)`).

**Czemu obracacie kamerę, a nie scenę?** Bo światła są w stałym świecie. Gdyby obracała
się scena, oświetlenie raziłoby pod złym kątem przy patrzeniu z góry. Orbita kamery
trzyma światła względem akwarium.

**Jak model trafia na dno i ma dobry rozmiar?** Po wczytaniu liczymy jego bounding box,
centrujemy w X/Z, stawiamy spód na y=0 i skalujemy do jednostki. Potem tylko ustawiamy
pozycję na dnie.

**Jak skalujecie całe akwarium?** Mnożymy wspólną macierz `baseM` przez `aquariumScale`,
więc wszystkie obiekty skalują się razem; w shaderze dzielimy odległości przez skalę,
żeby światło zachowało proporcje.

**Co robi `glDrawElements` vs `glDrawArrays`?** `glDrawElements` rysuje po indeksach z EBO
(oszczędza powtarzanie wierzchołków), `glDrawArrays` rysuje wierzchołki po kolei (tak
rysujemy wczytany model).
