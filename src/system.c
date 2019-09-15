#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "gwint.h"
#include "gwint_prywatne.h"

karta_t karty[10];

void generuj_karty( void )
{
	karty[0].punkty_wzorcowe = 20;
	karty[0].id = 1;
	karty[0].nazwa = "Bury Zenek";
	karty[0].flagi = BOHATER;

	karty[1].punkty_wzorcowe = 5;
	karty[1].id = 2;
	karty[1].nazwa = "Moher";
	karty[1].flagi = ZWYKLA;

	karty[2].punkty_wzorcowe = 6;
	karty[2].id = 3;
	karty[2].nazwa = "Pe�zacz";
	karty[2].flagi = WIEZ | ZWYKLA;

	karty[3].id = 4;
	karty[3].nazwa = "R�g";
	karty[3].flagi = ROG_DOWODCY;
	karty[3].rzad_cel = NIEUSTALONY;

	karty[4].id = 5;
	karty[4].nazwa = "Ziobro ziobro 7";
	karty[4].flagi = ZWYKLA | SZPIEG;

	karty[5].id = 6;
	karty[5].nazwa = "Lekarz";
	karty[5].punkty_wzorcowe = 5;
	karty[5].flagi = ZWYKLA | ODZYSKANIE;

	karty[6].id = 7;
	karty[6].nazwa = "Lodowy golem";
	karty[6].flagi = MROZ | BOHATER;
	karty[6].punkty_wzorcowe = 8;

	karty[7].id = 8;
	karty[7].nazwa = "Po�oga";
	karty[7].flagi = POZOGA;
	karty[7].rzad_cel = GLOBALNA;
	karty[7].rzad_wlasny = BRAK_RZEDU;

	karty[8].id = 9;
	karty[8].nazwa = "Manekin";
	karty[8].punkty_wzorcowe = 0;
	karty[8].flagi = ZWYKLA | MANEKIN;

	karty[9].id = 10;
	karty[9].nazwa = "Debil";
	karty[9].punkty_wzorcowe = 5;
	karty[9].flagi = ZWYKLA | MNOZENIE;
}


void gwint_pokaz_punkty( gwint_handle_t gra )
{
	for( uint8_t i=0; i<ILE_STRON; i++ )
	{
		punkty_t punkty = gra->strony[i].punkty;

		printf("Strona %d: suma: %02d rz1: %02d rz2: %02d rz3: %02d\n", i, punkty.suma, punkty.rzedy[0], punkty.rzedy[1], punkty.rzedy[2] );
	}

	puts("--------------------------------------------------------------------");
}

void gwint_wypisz_karte( karta_t * karta )
{
	printf("%s-%d, ", karta->nazwa, karta->punkty);
}

void gwint_rysuj_plansze( gwint_handle_t gra )
{
	printf("Aktywna strona: %d", gra->aktywna_strona_idx);

	for( uint8_t i=0; i<ILE_STRON; i++ ) // strony
	{
		printf("\nStrona %d:", i);

		for( uint8_t j=0; j<3; j++ ) // rz�dy
		{
			printf("\nRzad %d:", j);
			list_handle_t karty = gra->strony[i].rzedy[j].karty;
			list_element_t * elem;

			list_to_begin(karty);

			while( (elem = list_actual(karty)) ) // karty
			{
				gwint_wypisz_karte( elem->data );
				list_to_next(karty);
			}
		}
	}
}

void gwint_zakoncz_gre( koniec_info_t info )
{
	printf("Koniec gry, wygrana: %d", info.zwyciezca);
	system("pause");
}

void gwint_zakoncz_runde( koniec_info_t info )
{
	printf("Koniec rundy, wygrana: %d", info.zwyciezca);
	system("pause");
}
