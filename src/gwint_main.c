#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "gwint.h"
#include "gwint_prywatne.h"

void gwint_reset_rundy( gwint_handle_t gra );
void gwint_reset_gry( gwint_handle_t gra );
nastepny_proces_t gwint_glowny_proces( void * karta );
nastepny_proces_t gwint_proces_rzad( void * karta );
nastepny_proces_t gwint_proces_strona( void * karta );
nastepny_proces_t gwint_proces_koniec_gry( void * karta );
nastepny_proces_t gwint_proces_koniec_rundy( void * karta );

void gwint_kolejna_aktywna_strona( gwint_handle_t gra );
void gwint_kolejna_aktywna_std( gwint_handle_t gra );

const gwint_proces_t procesy[] = {gwint_glowny_proces, gwint_proces_rzad, gwint_proces_strona, gwint_proces_koniec_gry, gwint_proces_koniec_rundy};

void gwint_wyslij_sygnal( gwint_handle_t gra, uint8_t kod, void * arg )
{
	gwint_sygnal_info_t info;
	info.rzad = GLOBALNA;
	info.strona = GLOBALNA;
	info.kod = kod;
	info.arg = arg;

	gwint_system_t * sys = &gra->system;

	if( sys->sygnal ) sys->sygnal( &info, sys->arg );
}

uint8_t stos_pop( gwint_handle_t gra )
{
	list_element_t * element = list_pop_front( gra->stos_stron );
	if( element == NULL ) gwint_wyslij_sygnal(gra, SYGNAL_MEM, stos_pop);

	uint8_t data = *(uint8_t*)element->data;
	
	if( list_delete_elem(element) != LIST_OK ) gwint_wyslij_sygnal(gra, SYGNAL_MEM, stos_pop);

	return data;
}

void stos_push( gwint_handle_t gra )
{
	if( list_push_front( gra->stos_stron, &gra->aktywna_strona_idx ) != LIST_OK )
	{
		gwint_wyslij_sygnal(gra, SYGNAL_MEM, stos_push);
	}
}

void gwint_init( gwint_handle_t gra )
{
	gra->aktywny_proces = gwint_glowny_proces;
	gra->stos_stron = list_create( sizeof(gra->aktywna_strona_idx) );

	if( gra->stos_stron == NULL ) gwint_wyslij_sygnal(gra, SYGNAL_MEM, gwint_init);

	for( uint8_t i=0; i<ILE_STRON; i++ )
	{
		strona_init( &gra->strony[i], gra, i );
	}
}

void gwint_kolejna_aktywna_ze_stosu( gwint_handle_t gra )
{
	strona_handle_t strona;

	while( gra->stos_stron->size > 0 )
	{
		gra->aktywna_strona_idx = stos_pop( gra );
		strona = &gra->strony[ gra->aktywna_strona_idx ];

		if( strona->flagi & AKTYWNA ) return; // strona nadal aktywna, przeka� do niej sterowanie
	}

	// na stosie powrot�w nie by�o aktywnej strony, przeka� sterowanie do kolejnej po najni�szej ze stosu
	gwint_kolejna_aktywna_std(gra);
}

void gwint_kolejna_aktywna_std( gwint_handle_t gra )
{
	uint8_t ile_sprawdzonych=0;
	strona_handle_t strona;

	while( ile_sprawdzonych < (2*ILE_STRON+1) )
	{
		// standardowy obieg po okr�gu
		gra->aktywna_strona_idx++;
		if( gra->aktywna_strona_idx == ILE_STRON ) gra->aktywna_strona_idx = 0;

		strona = &gra->strony[ gra->aktywna_strona_idx ];

		// omini�cie strony, na kt�r� dzia�a manekin i tych spasowanych
		if( (strona->flagi & (CZEKA|PAS)) || (strona->zycia == 0) )
		{
			strona_usun_flagi( strona, CZEKA );
			ile_sprawdzonych++;
			continue;
		}

		return;
	}

	gra->aktywna_strona_idx = BRAK_STRON_DO_AKTYWACJI;
}

// og�lna funkcja s�u��ca do zmiany strony
void gwint_kolejna_aktywna_strona( gwint_handle_t gra )
{
	if( gra->strony[gra->aktywna_strona_idx].flagi & AKTYWNA ) return; // aktualna strona mo�e przyj�� jeszcze jedn� kart�

	if( gra->stos_stron->size == 0 ) // stos powrot�w pusty
	{
		gwint_kolejna_aktywna_std(gra);
	}
	else
	{
		gwint_kolejna_aktywna_ze_stosu(gra);
	}

	if( gra->aktywna_strona_idx != BRAK_STRON_DO_AKTYWACJI )
	{
		strona_dodaj_flagi( &gra->strony[gra->aktywna_strona_idx], AKTYWNA ); // oznacz jako gotow� na przyj�cie karty
	}

	gwint_wyslij_sygnal(gra, SYGNAL_STRONA_AKTYWOWANA, &gra->aktywna_strona_idx);
}

void gwint_ustaw_aktywna_strone( gwint_handle_t gra, uint8_t strona )
{
	stos_push(gra);
	gra->aktywna_strona_idx = strona;
	strona_dodaj_flagi( &gra->strony[gra->aktywna_strona_idx], AKTYWNA );

	gwint_wyslij_sygnal(gra, SYGNAL_STRONA_AKTYWOWANA, &gra->aktywna_strona_idx);
}

void gwint_spasuj( gwint_handle_t gra )
{
	strona_dodaj_flagi( &gra->strony[gra->aktywna_strona_idx], PAS );
	strona_usun_flagi( &gra->strony[gra->aktywna_strona_idx], AKTYWNA );
	gwint_kolejna_aktywna_strona(gra);
}

void gwint_przerwij_mnozenie( gwint_handle_t gra )
{
	strona_usun_flagi( &gra->strony[gra->aktywna_strona_idx], AKTYWNA );
	gra->id_karty_mnozacej_sie = 0;
	gwint_kolejna_aktywna_strona(gra);
}

uint16_t gwint_znajdz_max_pkt( gwint_handle_t gra, punkty_t * punkty )
{
	uint16_t max_pkt = 0;

	for( uint8_t i=0; i<ILE_STRON; i++ ) // znalezienie maksimum punkt�w
	{
		punkty[i] = gra->strony[i].punkty;

		if( punkty[i].suma > max_pkt )
		{
			max_pkt = punkty[i].suma;
		}
	}

	return max_pkt;
}

void gwint_wybierz_zwyciezce( gwint_handle_t gra, punkty_t * punkty, koniec_info_t * info, uint16_t max_pkt )
{
	for( uint8_t i=0; i<ILE_STRON; i++ )
	{
		if( punkty[i].suma != max_pkt || punkty[i].suma == 0 ) // nie jeden ze zwyci�zc�w albo nie ma punkt�w(remis po 0), uwal �ycie
		{
			//if( gra->strony[i].zycia ) gra->strony[i].zycia--;
			strona_zmniejsz_zycia( &gra->strony[i] );

			if( gra->strony[i].zycia == 0 ) info->zywi--; // strona nie ma �y�, zminiejsz licznik �ywych
			else info->zwyciezca = i; // strona jeszcze ma �ycia, ustaw jako zwyci�sc�, ma to sens tylko jak zostaje jedna strona �ywa
		}
	}
}

void gwint_sprawdz_koniec( gwint_handle_t gra )
{
	koniec_info_t info = {BRAK_ZWYCIEZCY, ILE_STRON, NIE_KONIEC};

	if( gra->aktywna_strona_idx == BRAK_STRON_DO_AKTYWACJI ) // koniec rundy
	{
		punkty_t punkty[ILE_STRON] = {0};
		info.status = KONIEC_RUNDY;

		uint16_t max_pkt = gwint_znajdz_max_pkt(gra, punkty);

		gwint_wybierz_zwyciezce(gra, punkty, &info, max_pkt);

		// zosta� jeden lub wszyscy padli - koniec gry
		if( info.zywi <= 1 )
		{
			info.status = KONIEC_GRY;
		}
	}

	gra->koniec_info = info;
}

nastepny_proces_t gwint_koniec_rundy( gwint_handle_t gra )
{
	gwint_sprawdz_koniec(gra);
	nastepny_proces_t retval = PROCES_GLOWNY;

	switch( gra->koniec_info.status )
	{
	case KONIEC_RUNDY:
		retval = PROCES_KONIEC_RUNDY;
		break;
	case KONIEC_GRY:
		retval = PROCES_KONIEC_GRY;
		break;
	default:
		retval = PROCES_GLOWNY;
		break;
	}

	return retval;
}

nastepny_proces_t gwint_proces_koniec_rundy( void * arg )
{
	gwint_handle_t gra = (gwint_handle_t)arg;

	gwint_wyslij_sygnal(gra, SYGNAL_KONIEC_RUNDY, &gra->koniec_info);

	gwint_reset_rundy(gra);
	return PROCES_GLOWNY;
}

nastepny_proces_t gwint_proces_koniec_gry( void * arg )
{
	gwint_handle_t gra = (gwint_handle_t)arg;

	gwint_wyslij_sygnal(gra, SYGNAL_KONIEC_GRY, &gra->koniec_info);

	gwint_reset_gry(gra);
	return PROCES_GLOWNY;
}

/*
* karta specjalna, jednostka
* wczytanie -> wyb�r w�asnego rz�du -> wyb�r strony celu -> wyb�r rz�du celu -> dodanie
*
* karta specjalna, tylko efekt
* wczytanie -> wyb�r strony celu -> wyb�r rz�du celu -> dodanie
*
* karta zwyk�a
* wczytanie -> wyb�r w�asnego rz�du -> dodanie
* stany rz�du - zwarcie, dystans, obl�nicze, nieustalony(tzn do ustalenia), efekt globalny dla ca�ej strony(np po�oga)
*/

void gwint_dodaj_karte_specjalna( gwint_handle_t gra )
{
	karta_t * karta = &gra->karta_buf;

	if( karta->flagi & (ODZYSKANIE|POZOGA|ROG_DOWODCY) ) // karta idzie do aktualnego rz�du, efekt do wybranego
	{
		karta_t karta_bez_efektu = *karta;					// kopiuj w�a�ciwo�ci
		karta_bez_efektu.flagi &= ~SPECJALNY_EFEKT_MASK;	// usu� efekty specjalne
		karta->flagi &= ~JEDNOSTKA_MASK;					// zostaw sam efekt karty

		strona_dodaj_karte( &gra->strony[gra->aktywna_strona_idx], &karta_bez_efektu ); // dodaj kart� bez efektu do aktualnej strony
		strona_usun_flagi( &gra->strony[gra->aktywna_strona_idx], AKTYWNA ); // nie szpieg, wi�c ta strona wi�cej nie przyjmie
	}
}

nastepny_proces_t gwint_dodaj_karte_zwykla( gwint_handle_t gra )
{
	karta_t * karta = &gra->karta_buf;
	strona_handle_t strona = &gra->strony[gra->aktywna_strona_idx];

	if( karta->rzad_wlasny != NIEUSTALONY ) // rz�d ju� zdefiniowany
	{
		strona_dodaj_karte( strona, karta ); // dodaj kart�

		if( karta->flagi & MNOZENIE ) 
		{
			gra->id_karty_mnozacej_sie = karta->id;
			return PROCES_GLOWNY; // nie usuwaj flagi aktywno�ci
		}

		if( karta->flagi & MANEKIN )
		{
			strona_dodaj_flagi( strona, CZEKA );
		}

		strona_usun_flagi( strona, AKTYWNA ); // oznacz, �e nie mo�e przyj�� wi�cej
				
		if( karta->flagi & POGODA_MASK ) // pogoda dzia�a na wszystkich
		{
			karta->flagi &= ~JEDNOSTKA_MASK;

			for( uint8_t i=0; i<ILE_STRON; i++ )
			{
				strona_dodaj_karte( &gra->strony[i], karta );
			}
		}
	}
	else
	{
		return PROCES_RZAD; // zmie� proces na wyb�r rz�du
	}

	return PROCES_BEZ_ZMIAN;
}

uint8_t gwint_obsluz_karty_specjalne( karta_t * karta, gwint_handle_t gra )
{
	if( karta->flagi & KONIEC_MNOZENIA )
	{
		gwint_przerwij_mnozenie(gra);
		return 1;
	}

	if( karta->flagi & KARTA_PASS )
	{
		gwint_spasuj(gra);
		return 1;
	}

	return 0;
}

nastepny_proces_t gwint_glowny_proces( void * arg )
{
	gwint_handle_t gra = (gwint_handle_t)arg;
	gwint_system_t * sys = &gra->system;

	if( sys->wczytaj_karte == NULL ) gwint_wyslij_sygnal(gra, SYGNAL_CALLBACK, gwint_glowny_proces);

	// obs�uga zako�czenia gry/rundy
	nastepny_proces_t nastepny = gwint_koniec_rundy(gra);
	if( nastepny != PROCES_GLOWNY ) return nastepny;

	karta_t * karta = &gra->karta_buf;

	if( sys->wczytaj_karte(karta, sys->arg) ) // jest karta
	{
		if( gwint_obsluz_karty_specjalne(karta, gra) )
		{
			return PROCES_GLOWNY;
		}

		if( gra->id_karty_mnozacej_sie ) // w czasie przyjmowania kart mno��cych si�, nie przyjmuj innych
		{
			if( karta->id != gra->id_karty_mnozacej_sie ) return PROCES_GLOWNY;
		}

		if( karta->flagi & SPECJALNY_EFEKT_MASK ) // karta ma zdolno�� specjaln�, kt�r� trzeba odpowiednio pokierowa�
		{
			gwint_dodaj_karte_specjalna(gra);
			return PROCES_STRONA; // zmie� proces na wyb�r strony
		}
		else // zwyk�a karta, po prostu dodaj
		{
			nastepny_proces_t nastepny = gwint_dodaj_karte_zwykla(gra);
			if( nastepny != PROCES_BEZ_ZMIAN )
			{
				return nastepny;
			}
		}

		gwint_kolejna_aktywna_strona(gra);
	}

	return PROCES_GLOWNY;
}

nastepny_proces_t gwint_proces_strona( void * arg )
{
	gwint_handle_t gra = (gwint_handle_t)arg;
	gwint_system_t * sys = &gra->system;

	if( sys->wybierz_strone == NULL ) gwint_wyslij_sygnal(gra, SYGNAL_CALLBACK, gwint_proces_strona);

	karta_t * karta = &gra->karta_buf;
	uint8_t strona=0;

	if( sys->wybierz_strone(&strona, sys->arg) )
	{
		// strona, kt�ra spasowa�a lub czeka, nie mo�e przyj�� karty z odzyskania, ale negatywne efekty ju� tak
		if( (karta->flagi & ODZYSKANIE) && (gra->strony[strona].flagi & (PAS|CZEKA)) ) return PROCES_STRONA;

		gwint_ustaw_aktywna_strone( gra, strona );

		if( karta->rzad_cel != NIEUSTALONY ) // rz�d jest ustalony, lub og�lny
		{
			if( karta->flagi & ODZYSKANIE )
			{
				return PROCES_GLOWNY; // dla aktualnej strony przejd� do standardowego dodania karty
			}

			strona_dodaj_karte( &gra->strony[gra->aktywna_strona_idx], karta ); // dodaj kart� do wybranej strony
			strona_usun_flagi( &gra->strony[gra->aktywna_strona_idx], AKTYWNA);

			gwint_kolejna_aktywna_strona(gra); // przejd� do kolejnej i ustaw zwyk�e dodawanie karty

			return PROCES_GLOWNY;
		}
		else
		{
			// rz�d niezdefiniowany, trza go wybra�
			return PROCES_RZAD;
		}
	}

	return PROCES_STRONA;
}

nastepny_proces_t gwint_proces_rzad( void * arg )
{
	gwint_handle_t gra = (gwint_handle_t)arg;
	gwint_system_t * sys = &gra->system;

	if( sys->wybierz_rzad == NULL ) gwint_wyslij_sygnal(gra, SYGNAL_CALLBACK, gwint_proces_rzad);

	karta_t * karta = &gra->karta_buf;
	uint8_t rzad=0;

	if( sys->wybierz_rzad(&rzad, sys->arg) )
	{
		if( karta->rzad_cel != BRAK_RZEDU )		karta->rzad_cel = rzad;
		if( karta->rzad_wlasny != BRAK_RZEDU )	karta->rzad_wlasny = rzad;

		strona_dodaj_karte( &gra->strony[gra->aktywna_strona_idx], karta );
		strona_usun_flagi( &gra->strony[gra->aktywna_strona_idx], AKTYWNA );

		gwint_kolejna_aktywna_strona(gra);

		return PROCES_GLOWNY;
	}

	return PROCES_RZAD;
}

void gwint_reset_rundy( gwint_handle_t gra )
{
	gra->aktywny_proces = gwint_glowny_proces;
	gra->aktywna_strona_idx = 0;
	gra->id_karty_mnozacej_sie = 0;
	
	memset(&gra->karta_buf, 0, sizeof(karta_t));
	memset(&gra->koniec_info, 0, sizeof(koniec_info_t));

	list_clear(gra->stos_stron);

	for( uint8_t i=0; i<ILE_STRON; i++ )
	{
		strona_reset_rundy( &gra->strony[i] );
	}
}

void gwint_reset_gry( gwint_handle_t gra )
{
	gwint_reset_rundy(gra);

	for( uint8_t i=0; i<ILE_STRON; i++ )
	{
		strona_reset_gry( &gra->strony[i] );
	}
}

void gwint_proces( gwint_handle_t gra )
{
	nastepny_proces_t nastepny = gra->aktywny_proces(gra);
	gra->aktywny_proces = procesy[nastepny];
}
