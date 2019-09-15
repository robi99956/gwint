#ifndef KARTA_H
#define KARTA_H

#include <stdint.h>

typedef struct _karta
{
	uint8_t punkty_wzorcowe;
	uint8_t punkty;
	uint8_t rzad_wlasny; // tu trafi karta
	uint8_t rzad_cel; // tu trafi efekt jej dzia�ania
	uint8_t id;

	char * nazwa;
	uint8_t usun_nazwe; // 1 je�eli nazwa jest w pami�ci dynamicznej, b�dzie wtedy zwolniona przy niszczeniu karty

	uint16_t flagi;
} karta_t;

#define ZWYKLA			0x0001
#define WIEZ			0x0002
#define SZPIEG			0x0004
#define ODZYSKANIE		0x0008
#define BOHATER			0x0010
#define WZMOCNIENIE		0x0020
#define ROG_DOWODCY		0x0040
#define POZOGA			0x0080
#define CZYSTE_NIEBO	0x0100
#define DESZCZ			0x0200
#define MGLA			0x0400
#define MROZ			0x0800
#define MANEKIN			0x1000
#define MNOZENIE		0x2000
#define KONIEC_MNOZENIA 0x4000
#define KARTA_PASS		0x8000

#define POGODA_MASK				(DESZCZ|MROZ|MGLA|CZYSTE_NIEBO)
#define JEDNOSTKA_MASK			(ZWYKLA|BOHATER)
#define SPECJALNY_EFEKT_MASK	(SZPIEG|ODZYSKANIE|ROG_DOWODCY|POZOGA)

#define ID_JASKIER 0x01

#endif // !
