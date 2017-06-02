
#define DEBUG

#ifdef DEBUG

int LCDWC, LCDWD, LCDRC, LCDRD;

#else

unsigned char xdata LCDWC _at_ 0xff80; //zapis rozkazow
unsigned char xdata LCDWD _at_ 0xff81; //zapis danych
unsigned char xdata LCDRC _at_ 0xff82; //odczyt stanu
unsigned char xdata LCDRD _at_ 0xff83; //odczyt danych

#endif // DEBUG


void wait(unsigned int d) {
	unsigned int i, j;
	for (j = 0; j<d; j++)
		for (i = 0; i<144; i++);
}


void write_control(unsigned char a) {	//zapis rozakazu, zapis danej
	while (LCDRC & 0x80) wait(1);
	LCDWC = a;
}


void write_data(unsigned char a) {	// zapis danej
	while (LCDRC & 0x80) wait(1);
	LCDWD = a;
}


/*
*czyszczenie wyswietlacza
*/
void lcd_clr(void) {
	write_control(0x01);
	wait(10);
}

/*
*inicjalizacja wyswietlacza
*wywolac przed pierwszym uzyciem
*/

#ifdef DEBUG

void lcd_init() {
	return;
}

#else

void lcd_init(void) {
	unsigned char i;
	static const unsigned char code init_tab[] = { 0x33, 0x32, 0x38, 0x08, 0x0f, 0x06 };
	for (i = 0; i < sizeof init_tab; ++i) {
		write_control(init_tab[i]);
		wait(1);
	}
	lcd_clr();
	write_control(0x80);            // wybierz adres DD RAM
	wait(1);
}

#endif // DEBUG




/*
*wys³anie znaku na wyswietlacza
*  zaimplementowano obsluge dwoch znakow kontrolnych
*  LF (\n) - znak przejscia do nowej linii
*  CR (\r) - znak powrotu karetki
*/
void lcd(unsigned char c) {
	unsigned char x;
	switch (c) {
	case'\n':			//od nowego wiersza
		x = LCDRC;
		if (x <= 0x0f) write_control(0xc0);
		if ((x >= 0x41) && (x <= 0x50)) {
			write_control(0x80);
		}
		break;
	case'\r':
		x = LCDRC;
		if ((x >= 0x00)&(x <= 0x0f))write_control(0x80);	//rozkaz dana
		if ((x >= 0x41)&(x <= 0x50))write_control(0xc0);	//
		break;
	default:
		if (c>0x1f && c<0x80) write_data(c);
		wait(1);
		break;
	}
	wait(1);
	x = LCDRC;
	if (x == 0x10)write_control(0xc0);			//rozkaz dana
	if (x == 0x50)write_control(0x80);			//
}