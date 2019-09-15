/*
 * gwint.c
 *
 *  Created on: 2 wrz 2019
 *      Author: robert
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "gwint.h"

extern karta_t karty[];

void generuj_karty( void );
void gwint_rysuj_plansze( gwint_handle_t gra );

uint8_t czytnik_wczytaj_karte( karta_t * karta, void * arg )
{
	UNUSED(arg);

	int nr;
	printf("\nPodaj id karty: ");
	scanf("%d", &nr);

	if( nr == -1 ) // wymu� zmian� strony - przerwij mno�enie
	{
		karta->flagi |= KONIEC_MNOZENIA;
		return 1;
	}

	if( nr == -2 ) // pas
	{
		karta->flagi |= KARTA_PASS;
		return 1;
	}

	*karta = karty[nr-1];
//	printf("Wybrana karta: %s\n", karta->nazwa);

	return 1;
}

uint8_t gwint_wybierz_strone( uint8_t * strona, void * arg )
{
	UNUSED(arg);

	int nr;
	printf("\nwybierz strone: ");
	scanf("%d", &nr);
	*strona = nr;
	return 1;
}

uint8_t gwint_wybierz_rzad( uint8_t * rzad, void * arg )
{
	UNUSED(arg);

	int nr;
	printf("\nWybierz rzad: ");
	scanf("%d", &nr);
	*rzad = nr;
	return 1;
}

void gwint_sygnal( gwint_sygnal_info_t * info, void * arg )
{
	UNUSED(arg);
	static char* sygnaly[] =
	{
			"MEM", "RESET", "NOWA_KARTA", "PUNKTY", "ZYCIA", "KONIEC_GRY",
			"KONIEC_RUNDY", "STRONA_AKTYWOWANA", "CALLBACK"
	};


	printf("Sygnal %s - rzad: %d strona: %d\n", sygnaly[info->kod], info->rzad, info->strona);
}

int main()
{
	gwint_t gwint = {0};

	gwint_system_t gwint_system =
	{
		.wczytaj_karte = czytnik_wczytaj_karte,
		.wybierz_rzad = gwint_wybierz_rzad,
		.wybierz_strone = gwint_wybierz_strone,

		.sygnal = gwint_sygnal
	};

	gwint.system = gwint_system;

	generuj_karty();

	gwint_init(&gwint);

	while(1)
	{
		gwint_proces(&gwint);
//		system("clear");
		gwint_pokaz_punkty(&gwint);
		gwint_rysuj_plansze(&gwint);
	}

    return 0;
}

