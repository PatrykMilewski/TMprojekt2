#include <stdlib.h>

// wersja debug
#define DEBUG

#ifdef DEBUG

int display;
int *XBYTE = NULL;
int P1_7;

#else

#include <at89x52.h>
#include <absacc.h>

#endif // DEBUG

#pragma region CONSTVALUES

#define TRUE 1
#define FALSE 0

// adres rejestru CSKB1
#define CSKB1 0xf022

// kod klawisza aktwacji, do podawania sygnalow w alfabecie Morse'a
#define KLAWISZAKTYWACJI 0x7F

// kod znaku "-" myslinik
#define MYSLNIK 0x40

// kod znaku "n" mala litera n
#define LITERAN 0x54

// kod znaku "L" duza litera L
#define LITERAL 0x38

// minimalny czas w ms na krotki znak w alfabecie Morse'a
#define shortMinTime 100

// maksymalny czas w ms na krotki znak w alfabecie Morse'a
#define shortMaxTime 700

// minimalny czas w ms na dlugi znak w alfabecie Morse'a
#define longMinTime 700

// maksymalny czas w ms na dlugi znak w alfabecie Morse'a
#define longMaxTime 2000

// stala okreslajaca ile razy musi wykonac sie petla for
// zeby opoznic program o 1 milisekunde
#define oneMilisecondConst 1275

// okresla, ile mozemy wyswietlic maksymalnie cfyr
#define digitsAmount 6

// okresla, ile jest liter w alfabecie
#define lettersAmount 26

// okresla, ile jest cyfr mozliwych roznych
#define numbersAmount 10

// czas w milisekundach okreslajacy, co ile przelaczamy sie
// pomiedzy cyframi w wyswietlaczu 7 segmentowym
// mozna dostosowac jak jest efekt mrugania
#define displayNumberDelay 10	

// czas w milisekundach potrzebny do wyswietlenia
// czasu reakcji na wyswietlaczu 7 segmentowym
#define displayValueDelay (digitsAmount * displayNumberDelay)

// stala okreslajaca ile czasu wykonywala sie petla,
// ktora mierzy ile czasu wciskano klawisz aktywacji
#define loopTimeLength (oneMilisecondConst / 2)

#pragma endregion

// kody cyfr na wyswietlaczu 7 segmentowym
const char numbers7seg[numbersAmount] = { 
	0x3F,	// 0
	0x06,	// 1
	0x5B,	// 2
	0x4F,	// 3
	0x66,	// 4
	0x6D,	// 5
	0x7D,	// 6
	0x07,	// 7
	0x7F,	// 8
	0x6F 	// 9
};

// kody liter na wyswietlaczu 7 segmentowym
const char letters7seg[lettersAmount] = {
	0x77,	// a
	0x7c,	// b
	0x39,	// c
	0x5e,	// d
	0x79,	// e
	0x71,	// f
	0x6f,	// g
	0x76,	// h
	0x30,	// i
	0x1e,	// j
	0x74,	// k
	0x38,	// l
	0x15,	// m
	0x54,	// n
	0x3f,	// o
	0x73,	// p
	0x67,	// q
	0x50,	// r
	0x6d,	// s
	0x78,	// t
	0x3e,	// u
	0x1c,	// v
	0x2a,	// w
	0x68,	// x
	0x6e,	// y
	0x5b	// z
};

// litery zakodowane w kodzie Morse'a
const char letters[lettersAmount][5] = {
	{ -1, -1, -1, 1, 0 },   //A
	{ -1, 0, 1, 1, 1 },		//B
	{ -1, 0, 1, 0, 1 },		//C
	{ -1, -1, 0, 1, 1 },	//D
	{ -1, -1, -1, -1, 1 },	//E
	{ -1, 1, 1, 0, 1 },		//F
	{ -1, -1, 0, 0, 1 },	//G
	{ -1, 1, 1, 1, 1 },		//H
	{ -1, -1, -1, 1, 1 },	//I
	{ -1, 1, 0, 0, 0 },		//J
	{ -1, -1, 0, 1, 0 },	//K
	{ -1, 1, 0, 1, 1 },		//L
	{ -1, -1, -1, 0, 0 },	//M
	{ -1, -1, -1, 0, 1 },	//N
	{ -1, -1, 0, 0, 0 },	//O
	{ -1, 1, 0, 0, 1 },		//P
	{ -1, 0, 0, 1, 0 },		//Q
	{ -1, -1, 1, 0, 1 },	//R
	{ -1, -1, 1, 1, 1 },	//S
	{ -1, -1, -1, -1, 0 },	//T
	{ -1, -1, 1, 1, 0 },	//U
	{ -1, 1, 1, 1, 0 },		//V
	{ -1, -1, 1, 0, 0 },	//W
	{ -1, 0, 1, 1, 0 },		//X
	{ -1, 0, 1, 0, 0 },		//Y
	{ -1, 0, 0, 1, 1 },		//Z
};

// cyfry zakodowane w kodzie Morse'a
const char numbers[numbersAmount][5] = {
	{ 0, 0, 0, 0, 0 },		//0
	{ 1, 0, 0, 0, 0 },		//1
	{ 1, 1, 0, 0, 0 },		//2
	{ 1, 1, 1, 0, 0 },		//3
	{ 1, 1, 1, 1, 0 },		//4
	{ 1, 1, 1, 1, 1 },		//5
	{ 0, 1, 1, 1, 1 },		//6
	{ 0, 0, 1, 1, 1 },		//7
	{ 0, 0, 0, 1, 1 },		//8
	{ 0, 0, 0, 0, 1 },		//9
};

// funkcja pauzujaca program
void delayProgram(unsigned int value) {
	unsigned int i = 0, j = 0;
	for (; i < value; i++)
		for (; j < oneMilisecondConst; j++);	// czas wykonania wewnetrznej petli = 1 ms
}

// wyswietlenie litery/cyfry na wyswietlaczu 7 segemntowym
// wyswietlanie wyglada tak: L-A / n-9
// gdzie L/n oznacza "litera"/"muber", A/9 dowolona litere/cyfre
void display7Seg(char value, char *codeTable, char type) {

	// jezeli litera
	if (value >= 65)
		value -= 65;
	// jezeli nie litera (cyfra)
	else
		value -= 48;

	for (char i = 0; i < 3; i++) {

		XBYTE[0xF030] = i + 1;

		// wyswietlenie litery
		if (i == 0)
			XBYTE[0xF038] = codeTable[value];

		// wyswietlenie myslnika
		else if (i == 1)
			XBYTE[0xF038] = MYSLNIK;

		// wyswietlenie duzej litery "L" - typ
		else if (i == 2)
			XBYTE[0xF038] = type;

		display = 0;

		// opoznienie pomiedzy znakami na wyswietlaczu
		delayProgram(displayNumberDelay);

		display = 1;
	}
}

// tlumaczy kod w alfabecie Morse'a na literke
char translateToChar(char code[5]) {
	char match, characterCode;

	// jezeli jest litera
	if (code[4] == -1) {
		for (char i = 0; i < lettersAmount; i++) {
			match = 1;
			for (char j = 0; j < 4; j++) {

				// sprawdzamy, czy kolejne znaki w kodzie Morse'a sie zgadzaja
				if (code[j] != letters[i][j])
					// jezeli nie, to mamy inny znak i ustawiamy match na 0
					match = 0;
			}

			// jezeli znalezlismy kod naszej litery, zwracamy jej kod ASCII
			if (match == 1)
				return i + 65;
		}
	}
	// jezeli nie jest litera (cyfra)
	else {
		for (char i = 0; i < numbersAmount; i++) {
			match = 1;
			for (char j = 0; j < 4; j++) {
				
				// sprawdzamy, czy kolejne znaki w kodzie Morse'a sie zgadzaja
				if (code[j] != numbers[i][j])
					// jezeli nie, to mamy inny znak i ustawiamy match na 0
					match = 0;
			}

			// jezeli znalezlismy kod naszej cyrfy, zwracamy jej kod ASCII
			if (match == 1)
				return i + 48;
		}
	}

	// jezeli bledny kod wejsciowy, zwraca kod litery 'X'
	return letters[23];
}

// tlumaczy litere/cyfre na kod w alfabecie Morse'a
char * translateToMors(char character, char **codeTable) {

	// tablica w ktorej bedzie zakodowany znak
	char *result = malloc(sizeof(char) * 5);
	
	// jezeli litera
	if (character >= 65)
		character -= 65;
	// jezeli nie litera (cyfra)
	else
		character -= 48;

	// przepisanie z tablicy z kodami Morse'a kodu znaku do tablicy wynikowej
	for (char i = 0; i < 5; i++)
		result[i] = codeTable[character][i];

	// zwracamy zakodowany znak
	return result;
}

int main() {
	loopTimeLength;
	unsigned int time;
	char character[5];

	// petla glowna programu
	while (TRUE) {

		// petla rozpoznajaca caly znak (maksymalnie 5 sygnalow w alfabecie Morse'a)
		for (char i = 0; i < 5; i++) {

			//zerowanie tablicy z rozpoznanymi sygnalami
			character[i] = -1;
			
			//liczy czas po jakim nacisnieto przycisk
			time = 0;

			// petla od rozpoznawania jednego konkretnego sygnalu (dlugi/krotki)
			while (TRUE) {

				// jezeli nacisnieto klawisz aktywacji, przerywamy petle i mamy czas nacisniecia przycisku
				if (XBYTE[CSKB1] == KLAWISZAKTYWACJI)
					break;
				time++;
			}

			// obliczamy po jakim czasie w ms nacisnieto przycisk
			time = time / loopTimeLength;

			// czy czas nacisniecia zawiera sie w przedziale czasowym krotkiego znaku
			if (time >= shortMinTime && time < shortMaxTime)
				character[i] = 0;

			// czy czas nacisniecia zawiera sie w przedziale czasowym dlugiego znaku
			if (time >= longMinTime && time < longMaxTime)
				character[i] = 1;
		}


	}

	return 0;
}