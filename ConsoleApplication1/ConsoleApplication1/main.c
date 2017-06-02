#include <stdlib.h>


#define DEBUG

#ifdef DEBUG

int P1_7, P1_5;
int *XBYTE;

#else

#include <at89x52.h>
#include <absacc.h>
#include "LCD_lib.h"

#endif

#define TRUE 1
#define FALSE 0

// adres rejestru CSKB1
#define CSKB1 0xf022

// adres rejestru CSKB0
#define CSKB0 0xf021

// kod klawisza aktwacji, do podawania sygnalow w alfabecie Morse'a
#define KLAWISZAKTYWACJI 0x7F

// kod klawisza 1
#define KLAWISZJEDEN 0xFD

// kod klawisza 2
#define KLAWISZDWA 0xFB

// czas w milisekundach, ile bedzie dzialal brzeczyk przy odczycie
#define BRZECZYKCZAS 100

// ile czasu bedzie brzeczal brzeczyk dla kropki
#define BRZECZYKKROPKA 200

// ile czasu bedzie brzeczal brzeczyk dla kreski
#define BRZECZYKKRESKA 600

// ile czasu bedzie trwala przerwa pomiedzy kropkami i kreskami
#define BRZECZYKPRZERWA 200

// ile czasu bedzie trwala przerwa pomiedzy znakami w alfabecie
#define BRZECZYKPRZERWADLUGA 600

// minimalny czas w ms na krotki znak w alfabecie Morse'a
#define shortMinTime 50

// maksymalny czas w ms na krotki znak w alfabecie Morse'a
#define shortMaxTime 400

// minimalny czas w ms na dlugi znak w alfabecie Morse'a
#define longMinTime 400

// maksymalny czas w ms na dlugi znak w alfabecie Morse'a
#define longMaxTime 1200

// stala okreslajaca ile razy musi wykonac sie petla for
// zeby opoznic program o 1 milisekunde
#define oneMilisecondConst 6

// okresla, ile jest liter w alfabecie
#define lettersAmount 26

// okresla, ile jest cyfr mozliwych roznych
#define numbersAmount 10

// okresla, na ilu maksymalnie znakach kodujemy
// cyfry w alfabecie Morse'a
#define maxNumbersMorse 5

// okresla, na ilu maksymalnie znakach kodujemy
// litery w alfabecie Morse'a
#define maxLettersMorse 4

// stala okreslajaca ile czasu wykonywala sie petla,
// ktora mierzy ile czasu wciskano klawisz aktywacji
#define loopTimeLength 44

// litery zakodowane w kodzie Morse'a
// bity ustawione na:
// 10 = brak znaku, 01 = kreska, 00 = kropka
char letters[lettersAmount] = {
	0xa4, //A 	10 10  01 00
	0x01,	//B 	00 00  00 01
	0x11,	//C 	00 01  00 01
	0x81,	//D 	10 00  00 01
	0xa8,	//E 	10 10  10 00
	0x10,	//F 	00 01  00 00
	0x85,	//G 	10 00  01 01
	0x00,	//H 	00 00  00 00
	0xa0,	//I 	10 10  00 00
	0x54,	//J 	01 01  01 00
	0x91,	//K 	10 01  00 01
	0x04,	//L 	00 00  01 00
	0xa5,	//M 	10 10  01 01
	0xa4,	//N 	10 10  01 00
	0x95,	//O 	10 01  01 01
	0x14,	//P 	00 01  01 00
	0x45,	//Q 	01 00  01 01
	0x84,	//R 	10 00  01 00
	0x80,	//S 	10 00  00 00
	0xa9,	//T 	10 10  10 01
	0x90,	//U 	10 01  00 00
	0x40,	//V 	01 00  00 00
	0x94,	//W 	10 01  01 00
	0x41,	//X 	01 00  00 01
	0x51,	//Y 	01 01  00 01
	0x05,	//Z 	00 00  01 01
};

// cyfry zakodowane w kodzie Morse'a
// bity ustawione na:
// 1 = kreska, 0 = kropka
// 3 najstarsze bity sa nieistotne
const char numbers[numbersAmount] = {
	0x1f,	//0 	000 1  1 1 1 1
	0x1e,	//1 	000 1  1 1 1 0
	0x1c,	//2 	000 1  1 1 0 0
	0x18,	//3 	000 1  1 0 0 0
	0x10,	//4 	000 1  0 0 0 0
	0x00,	//5 	000 0  0 0 0 0
	0x01,	//6 	000 0  0 0 0 1
	0x03,	//7 	000 0  0 0 1 1
	0x07,	//8 	000 0  0 1 1 1
	0x0f,	//9 	000 0  1 1 1 1
};

// zamiana tablicy inputCode na kod odpowiadajacy
// wartosci danej litery w alfabecie Morsea
char compressLetter(char *inputCode) {
	char i;

	// ustawiamy wszystkie bity w result na 0
	char result = 0;

	// petla zamieniajaca ciag kresek i kropek na wartosc
	// zaczynamy od i = maxLettersMorse, gdyz pozniej bedziemy
	// przesuwac bity w result w lewo, dlatego najpierw musimy
	// zajac sie najbardziej znaczacymi znakami w inputCode
	for (i = maxLettersMorse; i >= 0; i--) {

		// jezeli kropka, to zmieniamy najmlodszy bit w result na 1
		if (inputCode[i] == 1)
			result += 1;

		// jezeli kreska, to zmieniamy 2 najmlodsze bity w result na 10
		else if (inputCode[i] == 2)
			result += 2;

		// jezeli nie wejdzie w zaden warunek, to wartosc bitu sie nie zmienia (zostaje 0)

		// jezeli ostatni obieg petli, to nie musimy juz przesuwac 
		// results bitowo, gdyz skonczylismy prace
		if (i != 0)
			// przesuniecie bitowe o 2 bity, gdyz mamy 3 stany -> 00, 01, 10, po dopisaniu
			// kazdego trzeba przesunac o 2 bity w lewo, zeby zachowac poprawnosc kodowania
			result = result << 2;
	}

	return result;
}

// funkcja analogiczna do compressLetter
char compressNumber(char *inputCode) {
	char i, result = 0;

	for (i = maxNumbersMorse - 1; i >= 0; i--) {

		if (inputCode[i] == 1)
			result += 1;

		if (i != 0)
			result = result << 1;
	}

	return result;
}

// tlumaczy kod w alfabecie Morse'a na literke
char translateToChar(char *inputCode) {
	char compressedCode, i;

	// jezeli jest litera
	if (inputCode[4] == 2) {

		// przeliczamy kropki i kreski na konkretna wartosc funkcja kompresujaca
		compressedCode = compressLetter(inputCode);

		// szukamy odpowiadajacej wartosci w zbiorze liter
		for (i = 0; i < lettersAmount; i++) {

			// jezeli znajdziemy taka sama wartosc, to znaczy ze znalezlismy nasza litere
			// i trzeba ja zwrocic, litery zaczynaja sie od 65 w ASCII, dlatego tyle dodajemy
			// zmienna "i" okresla polozenie w tablicy letters
			if (letters[i] == compressedCode)
				return i + 65;
		}
	}

	// jezeli nie jest litera (cyfra)
	else {

		// analogicznie jak dla liter
		compressedCode = compressNumber(inputCode);

		for (i = 0; i < numbersAmount; i++) {
			if (numbers[i] == compressedCode)
				return i + 48;
		}
	}

	// jezeli bledny kod wejsciowy, zwraca 0
	return 0;
}

// tlumaczy litere/cyfre na kod w alfabecie Morse'a
char getMorseCode(char input) {
	// jezeli litera
	if (input >= 65) {
		input -= 65;
		return letters[input];
	}
	// jezeli nie litera (cyfra)
	else {
		input -= 48;
		return numbers[input];
	}
}

// funkcja sygnalizujaca, ze nie rozpoznano znaku w alfabecie Morse'a
// za pomoca diody TEST
void failedToTranslate() {
	char i;

	// dwa razy mruga dioda
	for (i = 0; i < 2; i++) {
		P1_7 = 0;
		wait(500);
		P1_7 = 1;
		wait(500);
	}
}

// funkcja sygnalizujaca pomyslne odczytanie jednego znaku w alfabecie Morse'a
// za pomoca brzeczyka
// 1 brzekniecie = kropka, 2 brzekniecia = kreska, 3 brzekniecia = koniec znaku
void successfullyRead(int input) {
	char i;

	// w zaleznosci od podanej wartosci input, tyle razy wlaczy i wylaczy brzeczyk
	for (i = 0; i < input + 1; i++) {
		P1_5 = 0;
		wait(BRZECZYKCZAS);
		P1_5 = 1;
		wait(BRZECZYKCZAS);
	}
}

void toNormalAlphabet() {
	char character[5], input, i;
	unsigned long int time;

	// petla glowna programu
	while (TRUE) {

		// zerowanie tablicy do wartosci 2, ktore interpetujemy 
		// jako ani kropka ani kreska, po prostu pusty znak
		for (i = 0; i < 5; i++) {
			character[i] = 2;
		}

		// petla rozpoznajaca caly znak (maksymalnie 5 sygnalow w alfabecie Morse'a)
		for (i = 0; i < 5; i++) {

			// zerowanie licznika czasu
			time = 0;

			// petla czekajaca na nacisniecie klawisza
			while (TRUE) {
				// jezeli nacisnieto klawisz aktywacji, przerywamy petle
				if (XBYTE[CSKB1] == KLAWISZAKTYWACJI)
					break;
			}

			// liczymy ile czasu wcisniety jest klawisz
			while (TRUE) {
				// jezeli klawisz nie jest juz wciskany, przerywamy petle
				if (XBYTE[CSKB1] != KLAWISZAKTYWACJI)
					break;

				time++;
			}

			// obliczamy dlugosc naciskania przycisku
			time /= loopTimeLength;

			// czy czas nacisniecia zawiera sie w przedziale czasowym kropki
			if (time >= shortMinTime && time < shortMaxTime) {
				character[i] = 0;
				successfullyRead(0);
			}

			// czy czas nacisniecia zawiera sie w przedziale czasowym kreski
			if (time >= longMinTime && time < longMaxTime) {
				character[i] = 1;
				successfullyRead(1);
			}

			// czy czas nisniecia jest dluzszy niz maksymalny czas dlugiego znaku
			// jezeli tak, to znaczy, ze koniec podawania znaku
			if (time >= longMaxTime) {
				successfullyRead(2);
				break;
			}
		}

		// wczytalismy znak od uzytkownika, teraz go rozpoznamy

		// translacja z morsa na chara
		input = translateToChar(character);

		// jezeli bledny kod - nie rozpoznano znaku
		if (input == 0)
			failedToTranslate();

		// jezeli poprawnie rozpoznano znak, wyswietlamy go na lcd
		else
			lcd(input);
	}
}

void toMorseAlphabet(char *text) {
	unsigned int i;
	char charCode, last2BitsMask = 0x03, last2Bits, j;

	for (i = 0; text[i] != NULL; i++) {
		charCode = getMorseCode(text[i]);

		// jezeli jest cyfra
		if (text[i] < 65) {
			for (j = 0; j < 5; j++) {

				// jezeli kropka
				if (charCode % 2 == 0) {
					P1_5 = 0;
					wait(BRZECZYKKROPKA);
				}

				//jezeli kreska
				else {
					P1_5 = 0;
					wait(BRZECZYKKRESKA);
				}

				P1_5 = 1;

				// przerwa pomiedzy kropkami/kreskami
				wait(BRZECZYKPRZERWA);

				// przesuwamy bitowo w prawo, gdyz kazda kropka/kreska zapisana jest
				// na jednym bicie i po odczytaniu jej, musimy przesunac calosc w prawo
				// 0 0 0 1  0 1 0 1 >> 1 = x 0 0 0  1 0 1 0
				charCode = charCode >> 1;
			}

			// przerwa pomiedzy znakami
			wait(BRZECZYKPRZERWADLUGA);
		}

		// jezeli jest litera
		else {
			for (j = 0; j < 4; j++) {

				last2Bits = charCode & last2BitsMask;

				// jezeli kropka
				if (last2Bits == 0) {
					P1_5 = 0;
					wait(BRZECZYKKROPKA);
				}

				// jezeli kreska
				else if (last2Bits == 1) {
					P1_5 = 0;
					wait(BRZECZYKKRESKA);
				}

				//jezeli koniec znaku
				else
					break;

				P1_5 = 1;

				// przerwa pomiedzy kropkami/kreskami
				wait(BRZECZYKPRZERWA);

				// przesuwamy bitowo w prawo, gdyz kazda kropka/kreska zapisana jest
				// na dwoch bitach i po odczytaniu jednej, musimy przesunac calosc w prawo
				// 10 01 00 10 >> 2 = xx 10 01 00
				charCode = charCode >> 2;
			}

			// przerwa pomiedzy literami
			wait(BRZECZYKPRZERWADLUGA);
		}
	}
}

void wyswietlLCD(char *text, char noDelayClear) {
	char i;

	lcd_clr();

	for (i = 0; text[i] != NULL; i++) {
		lcd(text[i]);
	}

	// jezeli noDelayClear == 1, to nie opozniamy programu i nie czyscimy lcd
	if (noDelayClear == 1)
		return;

	for (i = 0; i < 3; i++) {
		wait(1000);
	}

	lcd_clr();
}

void main() {
	char text[] = "123";
	char flag = 0;

	lcd_init();

	while (TRUE) {

		wyswietlLCD("Wybierz tryb\npracy 1 lub 2", 1);

		while (TRUE) {

			// alfabet Morse'a -> zwykly alfabet
			if (XBYTE[CSKB0] == KLAWISZJEDEN) {
				wyswietlLCD("Wybrano\nMorse -> normal", 0);
				flag = 1;
				break;
			}

			// zwykly alfabet -> alfabet Morse'a
			if (XBYTE[CSKB0] == KLAWISZDWA) {
				wyswietlLCD("Wybrano\nnormal -> Morse", 0);
				flag = 2;
				break;
			}
		}

		// w zaleznosci od wybranego trybu, wywolujemy odpowiednia funkcje
		if (flag == 1)
			toNormalAlphabet();

		else if (flag == 2)
			toMorseAlphabet(text);

	}
}