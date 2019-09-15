#ifndef GWINT_GWINT_H
#define GWINT_GWINT_H

#define ILE_KART_WIEZ 4
#define START_ZYCIA 2
#define ILE_STRON 4

#include "karta.h"
#include "list.h"

#define UNUSED(x) (void)x
#define SYGNAL_MEM					0 // arg - wska�nik na funkcj�, w kt�rej by� b��d
#define SYGNAL_RESET				1 // arg - NULL
#define SYGNAL_NOWA_KARTA			2 // arg - karta_t*
#define SYGNAL_PUNKTY				3 // arg - punkty_t*
#define SYGNAL_ZYCIA				4 // arg - uint8_t* - nowe �ycia
#define SYGNAL_KONIEC_GRY			5 // arg - koniec_info_t*
#define SYGNAL_KONIEC_RUNDY			6 // arg - koniec_info_t*
#define SYGNAL_STRONA_AKTYWOWANA	7 // arg - uint8_t* - numer strony
#define SYGNAL_CALLBACK				8 // arg - funkcja, z kt�rej mia�by i�� nieprawid�owy skok

typedef enum _proces{PROCES_GLOWNY, PROCES_RZAD, PROCES_STRONA, PROCES_KONIEC_GRY, PROCES_KONIEC_RUNDY, PROCES_BEZ_ZMIAN} nastepny_proces_t;
typedef nastepny_proces_t(*gwint_proces_t)( void* );

typedef struct _punkty
{
	uint16_t rzedy[3];
	uint16_t suma;
} punkty_t;

typedef struct _koniec_info
{
	uint8_t zwyciezca;
	uint8_t zywi;
	uint8_t status;
} koniec_info_t;

typedef struct _sygnal
{
	uint8_t rzad;
	uint8_t strona;
	uint8_t kod;
	void * arg;
} gwint_sygnal_info_t;

typedef struct _rzad
{
	list_handle_t karty;
	uint8_t flagi;
	uint16_t suma_punktow;

	uint8_t nr;
	struct _strona * parent;
} rzad_t, *rzad_handle_t;

typedef struct _strona
{
	rzad_t rzedy[3];
	uint8_t zycia;
	uint8_t flagi;
	punkty_t punkty;

	uint8_t nr;
	struct _gwint * parent;
} strona_t, *strona_handle_t;

typedef struct _gwint_system
{
	// wej�cie
	uint8_t(*wczytaj_karte)(karta_t*, void*);
	uint8_t(*wybierz_rzad)(uint8_t*, void*);
	uint8_t(*wybierz_strone)(uint8_t*, void*);

	// wyj�cie
//	void* - arg całego systemu
//	arg komendy jest w gwint_sygnal_info_t
	void(*sygnal)(gwint_sygnal_info_t*, void*);

	// argument og�lnego przeznaczenia, przekazywany do wszystkich funkcji wy�ej
	void * arg;
} gwint_system_t;

typedef struct _gwint
{
	karta_t karta_buf;
	gwint_proces_t aktywny_proces;

	uint8_t aktywna_strona_idx;
	list_handle_t stos_stron;
	uint8_t id_karty_mnozacej_sie;
	koniec_info_t koniec_info;

	strona_t strony[ILE_STRON];

	gwint_system_t system;
} gwint_t, *gwint_handle_t;



void gwint_proces( gwint_handle_t gra );
void gwint_init( gwint_handle_t gra );
void gwint_pokaz_punkty( gwint_handle_t gra );
void gwint_zakoncz_gre( koniec_info_t info );
void gwint_zakoncz_runde( koniec_info_t info );

void gwint_spasuj( gwint_handle_t gra );
void gwint_przerwij_mnozenie( gwint_handle_t gra );



#endif // !GWINT_GWINT_H
