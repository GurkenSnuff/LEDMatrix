// Ansteuerung eines MAX 7219 LED-Treiberbaustein mit einem Atmel AVR Mikrocontroller

//Diese Bibliothek stellt verschiedene Funktionen bereit um einen MAX 7219 Baustein anzusteuren.
//Beschreibung: Der MAX 7219 ist ein Baustein der bis zu 64 LEDs unabhängig voneinader treiben kann. Angesteuert wird er über eine 3 polige serielle Schnitstelle (CLK, DIN und LOAD(CS)). 

//Die meisten Funktionen dienen dazu eine Zahl auf einer bis zu 8-Stelligen 7-Segment Anzeige, die gemäß des Datenblatts angeschlossen wurde, darzustellen. Die Funktion "max7219_send"
//kann jedoch mit beliebigen Parametern aufgerufen werden und dazu benutzt werden die angeschlossenen LEDs völlig individuell anzusteuren. 
//Damit kann man dann z.B. eine Matrix-Anzeige steuren oder Sonderzeichen/Buchstaben auf einer 7-Segment Anzeige darstellen.

//Diese Bibliothek wurde mit einem MAX 7219 getestet sollte allerdings auch mit dem MAX 7221 funktionieren welcher dem 7219er sehr ähnelt. Genauers bitte dem Datenblatt entnehmen!

//Es wurden keine Waitstates verwendet, da die serielle Schnittstelle das Bausteins sehr schnell ist. Bei Prozessortakten weit über 10 MHz muss die Funktion "max7219_clock_out" gegebenenfalls
//um diese Waitstates ergänzt werden.

// Schema einer 8 stelligen 7-Segment Anzeige
//
//	 Pos8  Pos7  Pos6  Pos5  Pos4  Pos3  Pos2  Pos1
//    __    __    __    __    __    __    __    __
//   |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
//    --    --    --    --    --    --    --    --
//   |__|. |__|. |__|. |__|. |__|. |__|. |__|. |__|.


//Taktet ein Byte in das interne Schieberegister des Bausteins
void max7219_clock_out(unsigned char Data);

//Damit kann man dem Scroll-Text was einfügen
void AddToScrollText(unsigned char Data, unsigned char registerToAdd, unsigned char startMatrix);

//Startet das Scrolling (einmalig)
void StartScrolling();

//Startet das Scrolling (widerholend)
void StartScrollingContinuous();

//Es sendet alle Matrixen
void max7219_sendAllMatrix(unsigned char Data, unsigned char Register, unsigned char Matrix);

//Es enfernt etwas from Scroll-Text
void RemoveFromScrollText(unsigned char Data, unsigned char registerToAdd, unsigned char startMatrix);

//Es zeigt das Scroll-Text an
void ShowScrollText();

//Es löscht den gesamten Scroll-Text
void ClearScrollText();

//Es löscht alle bis jetzt vorhandenen Register
void ClearRegisters();

//Sendet Daten an das Display. Dafür wird erst die Registeradresse und dann die Daten übergeben
//und das Ganze durch eine steigende Flanke an "LOAD" übernommen.
//Data beinhaltet dabei die Information über den Zustand der LEDs.
void max7219_send(unsigned char a, unsigned char b);

//Gibt ein entsprechendes HEX-Zeichen auf der angegebenen Position aus wenn Number zwischen 0 und 15 ist (0123456789AbCdEF). Ansonsten bleibt die Anzeige leer.  
void max7219_hex(unsigned char Number, unsigned char Position);

//Gibt eine vorzeichenlsoe 16 Bit Ganzzahl (0...65535) auf den Positionen 1 bis 5 aus. Führende Nullen werden unterdrückt. Die Positionen 6 bis 8 werden nicht verändert.
void max7219_disp_uint16(uint16_t Number);

//Gibt eine vorzeichenbehaftete 16Bit Ganzzahl (-32768...32767) auf den Positionen 1 bis 6 aus. Führende Nullen werden unterdrückt. 
//Das Minuszeichen wird auch bei nicht 5 stelligen Zahlen direkt vor die 1. Ziffer geschriben. Die Positionen 7 und 8 werden nicht verändert.
void max7219_disp_int16(int16_t Number);

//Gibt eine vorzeichenlsoe 8 Bit Ganzzahl (0...255) auf den Positionen 1 bis 3 aus. Führende Nullen werden unterdrückt. Die Positionen 4 bis 8 werden nicht verändert.
void max7219_disp_uint8(uint8_t Number);

//Gibt eine vorzeichenbehaftete 8 Bit Ganzzahl (-128...127) auf den Positionen 1 bis 4 aus. Führende Nullen werden unterdrückt. 
//Das Minuszeichen wird auch bei nicht 3 stelligen Zahlen direkt vor die 1. Ziffer geschriben. Die Positionen 5 bis 8 werden nicht verändert.
void max7219_disp_int8(int8_t Number);

//Leert alle Anzeigen
void max7219_clear(void);

//Hiermit können die internen BCD -> 7-Segment Wandler an und abgeschaltet werden. Diese Bibliothek benutzt diese Wandler nicht.
void max7219_set_decodemode(unsigned char Decodemode);

//Die Helligkeit kann in 16 Stufen (0-15) eingestellt werden.
void max7219_intensity(unsigned char Intensity);

//Die Anzahl der angesteurerten Anzeigen. Erwartet eine Zahl zwischen 0 und 7. 0 = 1 Anzeige; 7 = 8 Anzeigen
void max7219_set_scanlimit(unsigned char Scanlimit);

//Stromsparmodus aktivieren. Die Anzeige wird Abgeschaltet aber Daten bleiben erhalten. Der Chip verbraucht nur noch 150µA
void max7219_shutdown(void);

// Stromsparmodus beenden
void max7219_wakeup(void);

//Diplaytestmodus aktivieren. Alle Segmente Leuchten.
void max7219_displaytest_on(void);

//Diplaytestmodus beenden.
void max7219_displaytest_off(void);

//Befel ohne Wirkung. Kann bei Seriellschaltung mehrer Chips dazu genutzt werden um den letzten Befehl
// an den nächsten Chip weiter zu geben ohne eine sonstige Ändrung zu machen.
void max7219_nop(void);

//Stellt die Ports ein und Konfiguriert das Display. Helligkeit: 7 (Stufe 8); Anzeigen: 8; BCD->7SEG Wandlung: Aus; Alle Anzeigen leeren.
void max7219_init(unsigned char matrixAmount);

void max7219_skipMatrix(void);

//Port- und Pinbelegung kann hier individuell eingestellt werden.
#define max7219_PORT PORTC
#define max7219_DDR DDRC


#define max7219_CLK PC0
#define max7219_LOAD PC1
#define max7219_DIN PC2