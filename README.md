**IPK-HTTP-server**

**Autor:** Vojtěch Novotný

**Login:** xnovot1f

**Datum:** 17. 3. 2019

## Projekt čislo 1
# Lightweight HTML Server

### Jazyk
Jako jazyk pro práci na tomto projektu jsem zvolil C protože ho znám nejlépe. Jazyk C má ovšem některé velké nevýhody při práci na tomto projektu. Hlavní z nich je zpracování řetězců, které je v C velmi těžkopádné a náročné. Existují knihovny, které práci s nimi velmi zjednodušují, avšak já jsem byl tvrdohlavý a když už jsem byl připravený nějakou použít (kvůli frustraci), nebyl čas nastudovat jejich funkcionalitu.

### Implementovaná funkcionalita
Nestihl jsem implementovat veškerou funkcionalitu. 

Implementováno je:
- Základní zpracování GET requestů ... Pro správné objekty odešle příslušná data, pro špatné odešle status kód 404 Not Found.
- Detekce Accept-Type ... Pro */* odešle plain/text, pro plain/text nebo application/json odešle data v příslušném formátu, pokud to dává smysl. Pro jiné typy odesílá status kód 406 Not Acceptable.
- Výpočet CPU-Load ... Korektně vypočítá CPU Load procesorů server podle údajů v souboru /proc/stat.
- extra nápověda pro index ... Při dotazu na index (/) odešle krátkou nápovědu o dostupných objektech.

Není implementováno:
- Connection: Keep-Alive ... Připojení se vždy ihned ukončí po odeslání dat pro první request.
- /load?refresh=X ... Odešle status kód 501 Not Implemented.


### Knihovny pro sockety
Pro práci se sockety jsem využil knihoven <unistd.h> a <arpa/inet.h>, které poskytovaly veškerou funkcionalitu.
Knihovna <unistd.h> poskytovala IO operace se sockety, zatímco <arpa/inet.h> poskytovala struktury potřebné k otevření socketů.

### Struktura programu
Soubor je rozdělen na dvě části, v první jsou deklarace proměnných, maker a funkcí a ve druhé části jsou definice funckí a samotný program.
Samotná činnost server je obsluhována převážně funkcí server(port_n), která otevře na portu port_n socket a poté odesílá objekty podle toho, jaké requesty jí příjdou.

Obsluha requestů probíhá v hlavní smyčce, která běží pokud je globální proměnná run rovna 1. Tuto proměnnou lze nastavit na 0 klávesovou zkratkou CTRL+C, která vyvolá signál k ukončení. Tento signál ne odchycen funkcí close_handler, která zajistí změnu proměnné run a tím i korektní ukončení programu s uvolněním všech zdrojů.

Dále volá funkce server(port_n) několik další pomocných funkcí.

### Vyjímky
Pokud dojde k chybě, je volána funkce error(int error_code, char * error_message), která uvolní zdroje, vytiskne na standartní chybový výstup STDERR chybové hlášení error_message a ukončí program s návratovým kódem error_code.

### Makefile
Makefile obsahuje 2 targety. Target server přeloží a sestaví program do spustitelného souboru server, target run provede target server a poté výsledný spustitelný soubor spustí. Spuštění je však úspěšné pouze tehdy pokud je zadán i parametr port=12345, kde 12345 je validní číslo portu.
