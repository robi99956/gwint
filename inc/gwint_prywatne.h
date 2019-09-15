#ifndef GWINT_PRYWATNE
#define GWINT_PRYWATNE

#include "list.h"

#define BRAK_STRON_DO_AKTYWACJI 0xff

#define NIE_KONIEC		0
#define KONIEC_RUNDY	1
#define KONIEC_GRY		2

#define BRAK_ZWYCIEZCY	0xff

#define POGODA				0x01
#define PODWOJENIE			0x02
#define PODWOJENIE_JASKRA	0x04

#define AKTYWNA 0x01
#define PAS		0x02
#define WRZOD	0x04
#define PORAZKA 0x08
#define CZEKA	0x10

#define GLOBALNA	0xff
#define NIEUSTALONY 0xfe
#define BRAK_RZEDU	0xfd

void rzad_init( rzad_handle_t rzad, strona_handle_t parent, uint8_t nr );
void rzad_dodaj_karte( rzad_handle_t rzad, karta_t * karta );
void rzad_usun_karte_id( rzad_handle_t rzad, uint8_t id );
void rzad_usun_karty_pkt( rzad_handle_t rzad, uint8_t pkt );
uint8_t rzad_znajdz_max( rzad_handle_t rzad );
void rzad_zniszcz_max( rzad_handle_t rzad );

void rzad_dodaj_efekt( rzad_handle_t rzad, uint8_t efekt );
void rzad_usun_efekt( rzad_handle_t rzad, uint8_t efekt );
uint16_t rzad_policz_punkty( rzad_handle_t rzad );
void rzad_reset( rzad_handle_t rzad );

void strona_init( strona_handle_t strona, gwint_handle_t parent, uint8_t nr );
void strona_reset_rundy( strona_handle_t strona );
void strona_reset_gry( strona_handle_t strona );
uint8_t strona_czy_pas( strona_handle_t strona );
void strona_pasuj( strona_handle_t strona );
uint8_t strona_get_zycia( strona_handle_t strona );
uint8_t strona_zmniejsz_zycia( strona_handle_t strona );
void strona_policz_punkty( strona_handle_t strona );
void strona_dodaj_karte( strona_handle_t strona, karta_t * karta );
void strona_dodaj_flagi( strona_handle_t strona, uint8_t flagi );
void strona_usun_flagi( strona_handle_t strona, uint8_t flagi );

void gwint_ustaw_aktywna_strone( gwint_handle_t gra, uint8_t strona );

#define MAX(a, b) ((a>b) ? a:b)
#define MAX3(a, b, c) MAX(MAX(a, b), c)

#define RZAD_FOR_EACH(karta, rzad, akcja) {list_handle_t __lista__ = rzad->karty; \
	karta_t * karta; \
	LIST_FOR_EACH(karta, __lista__, akcja); \
}
#endif
