#include <stdlib.h>
#include <string.h>

#include "gwint.h"
#include "gwint_prywatne.h"

void rzad_wyslij_sygnal( rzad_handle_t rzad, uint8_t kod, void * arg )
{
	gwint_sygnal_info_t info;
	info.rzad = rzad->nr;
	info.strona = rzad->parent->nr;
	info.arg = arg;
	info.kod = kod;

	gwint_system_t * sys = &rzad->parent->parent->system;

	if( sys->sygnal ) sys->sygnal(&info, sys->arg);
}

void rzad_init( rzad_handle_t rzad, strona_handle_t parent, uint8_t nr )
{
	rzad->karty = list_create( sizeof(karta_t) );
	if( rzad->karty == NULL )
	{
		rzad_wyslij_sygnal(rzad, SYGNAL_MEM, rzad_init);
	}

	rzad->flagi = 0;
	rzad->suma_punktow = 0;

	rzad->parent = parent;
	rzad->nr = nr;
}

void rzad_dodaj_karte( rzad_handle_t rzad, karta_t * karta )
{
	if( list_push_back( rzad->karty, karta ) != LIST_OK )
	{
		rzad_wyslij_sygnal(rzad, SYGNAL_MEM, rzad_dodaj_karte);
	}
}

void rzad_usun_karte_id( rzad_handle_t rzad, uint8_t id )
{
	RZAD_FOR_EACH(karta, rzad, 
	{
		if( karta->id == id )
		{
			if( karta->usun_nazwe ) free(karta->nazwa);

			if( list_delete_actual(rzad->karty, SET_TO_NULL) != LIST_OK )
			{
				rzad_wyslij_sygnal(rzad, SYGNAL_MEM, rzad_usun_karte_id);
			}
			return;
		}
	})
}

uint8_t rzad_znajdz_max( rzad_handle_t rzad )
{
	uint8_t max = 0;

	RZAD_FOR_EACH(karta, rzad, 
	{
		if( karta->punkty > max ) max = karta->punkty;
	})

	return max;
}

void rzad_usun_karty_pkt( rzad_handle_t rzad, uint8_t pkt )
{
	RZAD_FOR_EACH(karta, rzad, 
	{
		if( (karta->punkty == pkt) && ((karta->flagi & BOHATER) == 0) )
		{
			if( karta->usun_nazwe ) free( karta->nazwa );

			if( list_delete_actual(rzad->karty, SET_TO_NEXT) != LIST_OK )
			{
				rzad_wyslij_sygnal(rzad, SYGNAL_MEM, rzad_usun_karty_pkt);
			}
			continue;
		}
	})
}

void rzad_zniszcz_max( rzad_handle_t rzad )
{
	if( rzad->suma_punktow <= 10 ) return;

	uint8_t max = rzad_znajdz_max(rzad);
	rzad_usun_karty_pkt(rzad, max);
}

void rzad_dodaj_pogode( rzad_handle_t rzad )
{
	RZAD_FOR_EACH(karta, rzad, 
	{
		if( (karta->flagi & BOHATER) == 0 && (karta->punkty > 0) ) karta->punkty = 1;
	})
}

void rzad_sprawdz_pogode( rzad_handle_t rzad )
{
	if( rzad->flagi & POGODA ) rzad_dodaj_pogode( rzad );
}

void rzad_dodaj_podwojenie( rzad_handle_t rzad, uint16_t bez_flag )
{
	RZAD_FOR_EACH(karta, rzad, 
	{
		if( (karta->flagi & bez_flag) == 0 ) karta->punkty *= 2;
	})
}

void rzad_sprawdz_podwojenie( rzad_handle_t rzad )
{
	if( (rzad->flagi & PODWOJENIE_JASKRA) != 0 && (rzad->flagi & PODWOJENIE) == 0 ) rzad_dodaj_podwojenie(rzad, BOHATER|ROG_DOWODCY);
	else
		if( (rzad->flagi & PODWOJENIE) != 0 ) rzad_dodaj_podwojenie(rzad, BOHATER);
}

void rzad_dodaj_wzmocnienie( rzad_handle_t rzad, list_element_t * dodajacy )
{
	RZAD_FOR_EACH(karta, rzad, 
	{
		if( list_actual(rzad->karty) != dodajacy )
		{
			if( (karta->flagi & BOHATER) == 0 )
			{
				karta->punkty++;
			}
		}
	})
}

void rzad_sprawdz_wzmocnienie( rzad_handle_t rzad )
{
	RZAD_FOR_EACH(karta, rzad,
	{
		if( karta->flagi & WZMOCNIENIE )
		{
			list_element_t * element = list_actual(rzad->karty); // punkt powrotu

			rzad_dodaj_wzmocnienie(rzad, element);

			if( list_goto(rzad->karty, element) != LIST_OK ) // powr�t
			{
				rzad_wyslij_sygnal(rzad, SYGNAL_MEM, rzad_sprawdz_wzmocnienie);
			}
		}
	})
}

void rzad_dodaj_wiez( rzad_handle_t rzad, int8_t * id, uint8_t * ile )
{
	RZAD_FOR_EACH(karta, rzad, 
	{
		for( uint8_t i=0; i<ILE_KART_WIEZ; i++ )
		{
			if( karta->id == id[i] ) karta->punkty *= ile[i];
		}
	})
}

int8_t czy_jest( int8_t * wsk, int8_t val )
{
	for( int8_t i=0; i<ILE_KART_WIEZ; i++ )
	{
		if( wsk[i] == val && val >= 0 ) return i;
	}

	return -1;
}

int8_t pierwszy_wolny( int8_t * wsk )
{
	for( int8_t i=0; i<ILE_KART_WIEZ; i++ )
	{
		if( wsk[i] == -1 ) return i;
	}

	return -1;
}

void rzad_sprawdz_wiez( rzad_handle_t rzad )
{
	int8_t id[ILE_KART_WIEZ];
	uint8_t ile[ILE_KART_WIEZ] = {0};
	int8_t idx=0;

	memset(id, -1, sizeof(id));

	RZAD_FOR_EACH(karta, rzad, 
	{
		if( (karta->flagi & WIEZ)  != 0 )
		{
			idx = czy_jest(id, karta->id );

			if( idx >= 0 )
			{
				ile[idx]++;
			}
			else
			{
				idx = pierwszy_wolny(id);
				id[idx] = karta->id;
				ile[idx]=1;
			}
		}
	})

	rzad_dodaj_wiez(rzad, id, ile);
}

void rzad_dodaj_efekt( rzad_handle_t rzad, uint8_t efekt )
{
	rzad->flagi |= efekt;
}

void rzad_usun_efekt( rzad_handle_t rzad, uint8_t efekt )
{
	rzad->flagi &= ~efekt;
}

uint16_t rzad_policz_punkty( rzad_handle_t rzad )
{
	uint16_t wynik=0;

	RZAD_FOR_EACH(karta, rzad, 
	{
		karta->punkty = karta->punkty_wzorcowe;
	})

	rzad_sprawdz_pogode(rzad);
	rzad_sprawdz_wzmocnienie(rzad);
	rzad_sprawdz_wiez(rzad);
	rzad_sprawdz_podwojenie(rzad);

	RZAD_FOR_EACH(karta, rzad, 
	{
		wynik += karta->punkty;
	})

	return wynik;
}

void rzad_reset( rzad_handle_t rzad )
{
	rzad->flagi = 0;
	rzad->suma_punktow = 0;
	list_clear( rzad->karty );
}

