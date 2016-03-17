/*
 * OSC.c
 *
 * Created: 2014-10-25 21:41:11
 *  Author: tmf
 */ 

#include <avr/io.h>
#include <util/delay.h>

_Bool OSC_wait_for_rdy(uint8_t clk)
{
	uint8_t czas=255;
	while ((!(OSC.STATUS & clk)) && (--czas)) // Czekaj na ustabilizowanie si� generatora
	_delay_ms(1);
	return czas;   //false je�li generator nie wystartowa�, true je�li jest ok
}

_Bool RC32M_en()
{
	OSC.CTRL |= OSC_RC32MEN_bm; //W��cz generator RC 32 MHz
	return OSC_wait_for_rdy(OSC_RC32MEN_bm); //Zaczekaj na jego poprawny start
}

void SelectPLL(OSC_PLLSRC_t src, uint8_t mult)
{
	mult&=OSC_PLLFAC_gm;
	OSC.PLLCTRL=src | mult;              //Ustaw �r�d�o i mno�nik PLL
	OSC.CTRL|=OSC_PLLEN_bm;				 //W��cz uk�ad PLL
	OSC_wait_for_rdy(OSC_PLLRDY_bm);     //Poczekaj na ustabilizowanie si� PLL
}