#include <stdio.h>
#include <stdlib.h>

#include "gwint.h"
#include "gwint_prywatne.h"

void strona_wyslij_sygnal( strona_handle_t strona, uint8_t kod, void * arg )
{
	gwint_sygnal_info_t info;
	info.rzad = GLOBALNA;
	info.strona = strona->nr;
	info.kod = kod;
	info.arg = arg;

	gwint_system_t * sys = &strona->parent->system;

	if( sys->sygnal ) sys->sygnal( &info, sys->arg );
}

void strona_init( strona_handle_t strona, gwint_handle_t parent, uint8_t nr )
{
	for( uint8_t i=0; i<3; i++ ) rzad_init( &strona->rzedy[i], strona, i );

	strona->flagi = 0;
	strona->zycia = START_ZYCIA;
	strona->punkty = (punkty_t){0};

	strona->parent = parent;
	strona->nr = nr;
}

uint8_t strona_czy_pas( strona_handle_t strona )
{
	return (strona->flagi & PAS);
}

void strona_pasuj( strona_handle_t strona )
{
	strona->flagi |= PAS;
}

uint8_t strona_get_zycia( strona_handle_t strona )
{
	return strona->zycia;
}

uint8_t strona_zmniejsz_zycia( strona_handle_t strona )
{
	if( strona->zycia ) strona->zycia--;

	strona_wyslij_sygnal(strona, SYGNAL_ZYCIA, &strona->zycia);
	return strona->zycia;
}

uint8_t strona_znajdz_max( strona_handle_t strona )
{
	uint8_t r1 = rzad_znajdz_max( &strona->rzedy[0] );
	uint8_t r2 = rzad_znajdz_max( &strona->rzedy[1] );
	uint8_t r3 = rzad_znajdz_max( &strona->rzedy[2] );

	return (MAX3(r1, r2, r3));
}

void strona_dodaj_pozoge( strona_handle_t strona, uint8_t rzad_nr )
{
	if( rzad_nr != GLOBALNA ) rzad_zniszcz_max( &strona->rzedy[rzad_nr] );
	else
	{
		uint8_t max = strona_znajdz_max(strona);
		rzad_usun_karty_pkt( &strona->rzedy[0], max );
		rzad_usun_karty_pkt( &strona->rzedy[1], max );
		rzad_usun_karty_pkt( &strona->rzedy[2], max );
	}
}

void strona_policz_punkty( strona_handle_t strona )
{
	punkty_t * punkty = &strona->punkty;

	punkty->suma += punkty->rzedy[0] = rzad_policz_punkty( &strona->rzedy[0] );
	punkty->suma += punkty->rzedy[1] = rzad_policz_punkty( &strona->rzedy[1] );
	punkty->suma += punkty->rzedy[2] = rzad_policz_punkty( &strona->rzedy[2] );
}

void strona_dodaj_jednostke( strona_handle_t strona, karta_t * karta )
{
	if( karta->flagi & ROG_DOWODCY )
	{
		rzad_dodaj_efekt( &strona->rzedy[karta->rzad_cel], PODWOJENIE_JASKRA );
	}
	else if( karta->flagi & POZOGA )
	{
		strona_dodaj_pozoge( strona, karta->rzad_cel );
	}

	rzad_dodaj_karte( &strona->rzedy[karta->rzad_wlasny], karta);
}

void strona_dodaj_specjalne( strona_handle_t strona, karta_t * karta )
{
	if( karta->flagi & POGODA_MASK )
	{
		if( karta->flagi & CZYSTE_NIEBO) 
		{
			rzad_usun_efekt( &strona->rzedy[0], POGODA );
			rzad_usun_efekt( &strona->rzedy[1], POGODA );
			rzad_usun_efekt( &strona->rzedy[2], POGODA );
		}
		else
		{
			rzad_dodaj_efekt( &strona->rzedy[karta->rzad_cel], POGODA );
		}
	} 
	else if( karta->flagi & ROG_DOWODCY )
	{
		rzad_dodaj_efekt( &strona->rzedy[karta->rzad_cel], PODWOJENIE );
	}
	else if( karta->flagi & POZOGA )
	{
		strona_dodaj_pozoge( strona, karta->rzad_cel );
	}
}

void strona_dodaj_karte( strona_handle_t strona, karta_t * karta )
{
	if( karta->flagi & JEDNOSTKA_MASK )
	{
		strona_dodaj_jednostke(strona, karta);
	}
	else
	{
		strona_dodaj_specjalne(strona, karta);
	}

	strona_policz_punkty(strona);

	strona_wyslij_sygnal(strona, SYGNAL_NOWA_KARTA, karta);
	strona_wyslij_sygnal(strona, SYGNAL_PUNKTY, &strona->punkty);
}

void strona_dodaj_flagi( strona_handle_t strona, uint8_t flagi )
{
	strona->flagi |= flagi;
}

void strona_usun_flagi( strona_handle_t strona, uint8_t flagi )
{
	strona->flagi &= ~flagi;
}

void strona_reset_rundy( strona_handle_t strona )
{
	strona->flagi = 0;
	
	rzad_reset( &strona->rzedy[0] );
	rzad_reset( &strona->rzedy[1] );
	rzad_reset( &strona->rzedy[2] );
}

void strona_reset_gry( strona_handle_t strona )
{
	strona_reset_rundy(strona);
	strona->zycia = START_ZYCIA;

	strona_wyslij_sygnal(strona, SYGNAL_ZYCIA, &strona->zycia);
}
