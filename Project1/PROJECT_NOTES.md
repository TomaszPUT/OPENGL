# Akwarium — co zmienione i jak to wytłumaczyć

## Pliki, które podmieniasz w projekcie
- `main.cpp` — podmień
- `f_simplest.glsl` — podmień (cała logika światła/tekstur)
- `Rock.cpp` — podmień
- `Plant.cpp` + `Plant.h` — podmień oba
- `Model.cpp` + `Model.h` — **dodaj nowe** (wczytywanie .obj)
- `shell.obj` — **dodaj** do folderu obok `.exe` (tam gdzie `sand.png`, `rock.png`)
- `v_simplest.glsl` — **bez zmian**, zostaje jak było

W Visual Studio: dodaj `Model.cpp` i `Model.h` do projektu (Solution Explorer → Add → Existing Item).
`shell.obj`, `sand.png`, `rock.png` muszą leżeć w katalogu roboczym (tam gdzie uruchamia się program).

---

## Gdzie jest każdy punkt z oceniania (cel: 5.0)

**+0.5 — nietrywialny model z pliku.** Klasa `Model` (`Model.cpp`) czyta `shell.obj`
(muszla spiralna) liniami `v`/`vt`/`vn`/`f`, składa tablicę wierzchołków i rysuje przez
`glDrawArrays`. To była jedyna brakująca rzecz do maksa. Model powstał w `gen_shell.py`
(spirala, na którą „nawinięto” okrąg). Rysowanie jest w 100% nasze — biblioteki do
rysowania nie używamy.

**+1 — ruch, animacja, kamera.** Ryby pływają i odbijają się od ścian (`Fish::update`),
rośliny się kołyszą (`Plant`), kamera: strzałki obracają scenę w 2 osiach, `O`/`P` zoom
(zmiana `fov`). Naprawiony błąd: `update` roślin był w pętli ryb (działał 15× za szybko).

**+0.5 — tekstury + poprawne rysowanie.** Dwie tekstury z plików realnie użyte:
`sand.png` na dnie, `rock.png` na kamieniach **i** na muszli. Wszystko przez VAO/VBO/EBO
i `glDrawElements`/`glDrawArrays`. Zero przestarzałych funkcji (brak `glBegin`, `glVertex`,
`glTranslate` itd. — macierze z GLM).

**+1 — oświetlenie.** Trzy światła w `f_simplest.glsl`:
1. górna lampa (ciepła, biała) — wzmocniona,
2. boczne niebieskie (odbicia wody),
3. **światło ujemne** w kamieniach (odejmuje światło → mroczna szczelina przy dnie).
Model Phona: ambient + diffuse (`dot(N,L)`) + specular (`reflect`) + tłumienie z odległością.

---

## Co konkretnie poprawiłem (Twoja lista)
- **Piasek** — `sand.png` + zmarszczki dna (`sin`) + ziarno (`hash`) + drobne błyski.
- **Skały** — nieregularna bryła (promień kuli modulowany falami `sin/cos` z losową fazą,
  każdy kamień inny) + prawdziwa tekstura `rock.png` + chropowatość.
- **Światło górne** — mocniejsze (mnożnik 5.5 → 8.5, mniejsze tłumienie).
- **Światło ujemne** — `L3` przy dnie wśród kamieni, odejmowane od oświetlenia.
- **Rośliny** — własna faza (nie kołyszą się równo), ruch w 2 osiach, mocne zwężanie ku
  górze (spiczasty czubek), losowy odcień (zielony/seledynowy/oliwkowy), świecący czubek.

---

## Możliwe pytania na obronie (i krótkie odpowiedzi)

**Jak wczytujecie model?** Własny parser OBJ: czytamy `v`, `vt`, `vn`, a dla każdej
ściany `f a/b/c …` bierzemy odpowiednie pozycje/UV/normalne i wrzucamy do jednej tablicy
`[x,y,z,u,v,nx,ny,nz]`. Potem `glDrawArrays(GL_TRIANGLES, …)`.

**Czemu nie ma `glBegin`/`glTranslate`?** Bo są przestarzałe i zakazane w regulaminie.
Geometria idzie przez VBO+VAO, a przesunięcia/obroty robi GLM (`translate`, `rotate`,
`perspective`, `lookAt`), wynik wysyłamy jako uniform `M`/`V`/`P`.

**Na czym polega światło ujemne?** To zwykłe światło punktowe, ale jego wkład
**odejmujemy** zamiast dodawać (`lighting -= L3neg`). Im bliżej źródła, tym ciemniej —
robi cień/zagłębienie między kamieniami. Czysto wizualny trik.

**Czemu rośliny mają stałe oświetlenie?** Liść to płaski quad — jego normalna jest
prostopadła do obu świateł, więc `dot(N,L)` ≈ 0 i wyszłyby czarne. Dlatego liczymy dla
nich własne, stałe oświetlenie zależne od lamp, ale nie od normalnej.

**Jak działa „oglądanie z każdej strony”?** Kamera stoi nieruchomo, a my obracamy całą
scenę macierzą `baseM` (strzałki) — efekt jest taki sam jak orbitowanie kamerą.
