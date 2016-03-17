/*
 * WS2812B_DMA.c
 *
 * Created: 2014-10-30 18:00:32
 *  Author: tmf
 */ 


#include <avr/io.h>
#include <util/delay.h>
#include "OSC.h"

void USART_init()
{
	PORTC_OUTSET=PIN3_bm;
	PORTC_DIRSET=PIN3_bm;                          //Pin TxD musi byæ wyjœciem
	PORTC_PIN3CTRL=PORT_INVEN_bm;                  //Zaneguj wyjœcie - domyœlnie bêdzie w stanie niskim
	USARTC0.CTRLC=USART_CHSIZE_7BIT_gc;            //Ramka 7 bitów, bez parzystoœci, 1 bit stopu
	//Szybkoœæ transmisji 2 Mbps, jeden bit trwa ok. 500 ns - pamiêtaj, aby nie korzystaæ z generatora frakcyjnego
	USARTC0.BAUDCTRLA=1;
	USARTC0.BAUDCTRLB=0;
	USARTC0.CTRLB=USART_TXEN_bm | USART_CLK2X_bm;  //W³¹cz nadajnik USART
}

void DMA_init(void *from, uint16_t size)
{
	DMA.CTRL=DMA_ENABLE_bm | DMA_DBUFMODE_DISABLED_gc | DMA_PRIMODE_RR0123_gc; //W³¹camy DMA, round robin, bez podwójnego buforowania
	DMA.CH3.ADDRCTRL=DMA_CH_SRCRELOAD_BLOCK_gc | DMA_CH_SRCDIR_INC_gc | DMA_CH_DESTRELOAD_NONE_gc | DMA_CH_DESTDIR_FIXED_gc; //Nie prze³adowujemy adresu docelowego
	DMA.CH3.SRCADDR0=(uint16_t)from;
	DMA.CH3.SRCADDR1=((uint16_t)from) >> 8;
	DMA.CH3.SRCADDR2=0;            //Adres sk¹d bierzemy dane
	DMA.CH3.DESTADDR0=(uint16_t)&USARTC0_DATA & 0xff;
	DMA.CH3.DESTADDR1=((uint16_t)&USARTC0_DATA) >> 8;
	DMA.CH3.DESTADDR2=0;           //Dane transmitujemy do rejestru nadajnika USART
	DMA.CH3.TRIGSRC=DMA_CH_TRIGSRC_USARTC0_DRE_gc;  //Transfer bêdzie wyzwalany przez wolne miejsce w buforze
	DMA.CH3.TRFCNT=size;             //Ile transmitujemy danych
	DMA.CH3.CTRLB=DMA_CH_TRNIF_bm;   //Skasuj flagê zakoñczenia transferu
	DMA.CH3.CTRLA=DMA_CH_ENABLE_bm | DMA_CH_SINGLE_bm | DMA_CH_BURSTLEN_1BYTE_gc;  //Odpal transfer - po jednym bajcie na ¿¹danie	
}

void USART_putchar(USART_t * const usart, char ch)
{
	while(!(usart->STATUS & USART_DREIF_bm));
	USARTC0.DATA=ch;
}

void WS2812B_transcodeGRB(uint8_t green, uint8_t red, uint8_t blue, void *buffer)
{
	asm volatile(
	"LDI R16, 8" "\n\t"
	"L_dl1%=:" "\n\t"
	"LDI R17, 0b00100100" "\n\t"
	"BST %0,7" "\n\t"
	"BLD R17,0" "\n\t"
	"BST %0,6" "\n\t"
	"BLD R17,3" "\n\t"
	"BST %0,5" "\n\t"
	"BLD R17,6" "\n\t"
	"ROL %2" "\n\t"
	"ROL %1" "\n\t"
	"ROL %0" "\n\t"
	"ROL %2" "\n\t"
	"ROL %1" "\n\t"
	"ROL %0" "\n\t"
	"ROL %2" "\n\t"
	"ROL %1 " "\n\t"
	"ROL %0" "\n\t"
	"COM R17" "\n\t"
	"ST %a3+, R17" "\n\t"
	"DEC R16" "\n\t"
	"BRNE L_dl1%=" "\n\t"
	::"r" (green), "r" (red), "r" (blue), "e" (buffer): "r16", "r17"
	);
}

uint8_t bufor[30*8];

int main(void)
{
	SelectPLL(OSC_PLLSRC_RC2M_gc, 17);   //Taktowanie CPU - 32 MHz (tak naprawdê 34 MHz, ¿eby nieco skróciæ czas trwania bitu
	CLK_Sel(CLK_SCLKSEL_PLL_gc);
	
	USART_init();          //Zainicjuj obs³ugê LEDa
	
	_delay_us(50);         //Zresetuj diody
	uint8_t g0=0; uint8_t r0=20	; uint8_t b0=20	;//background colour
	uint8_t g1=100; uint8_t r1=0; uint8_t b1=0;//foreground colour
	uint8_t pos;
	uint8_t i;
	uint8_t speed=250;//in miliseconds
	while(1)
	{
		for (pos=0; pos<30; pos++)
		{
			for (i=0; i<30; i++)
			{
				if(i==pos)//main diode
				{
					WS2812B_transcodeGRB(g1, r1, b1, &bufor[i*8]);
				}
				else if ((i==pos-1) || (i==pos+1))//neighbour diodes
				{
					WS2812B_transcodeGRB(g1/2, r1/2, b1/2, &bufor[i*8]);
				}
				else if ((i==pos-2) || (i==pos+2))//neighbour +1 dodes
				{
					WS2812B_transcodeGRB(g1/4, r1/4, b1/4, &bufor[i*8]);
				}
				else
				{
					WS2812B_transcodeGRB(g0, r0, b0, &bufor[i*8]);
				}
			}
				DMA_init(bufor, sizeof(bufor));
				_delay_us(50);//delay for lock data on diodes
			};
		_delay_ms(speed);		
		}
			
		for (pos=29; pos>=0; pos--)
		{
			for (i=0; i<30; i++)
			{
				if(i==pos)//main diode
				{
					WS2812B_transcodeGRB(g1, r1, b1, &bufor[i*8]);
				}
				else if ((i==pos-1) || (i==pos+1))//neighbour diodes
				{
					WS2812B_transcodeGRB(g1/2, r1/2, b1/2, &bufor[i*8]);
				}
				else if ((i==pos-2) || (i==pos+2))//neighbour +1 dodes
				{
					WS2812B_transcodeGRB(g1/4, r1/4, b1/4, &bufor[i*8]);
				}
				else
				{
					WS2812B_transcodeGRB(g0, r0, b0, &bufor[i*8]);
				}
				DMA_init(bufor, sizeof(bufor));
				_delay_us(50);//delay for lock data on diodes
			};
			_delay_ms(speed);
		}
		//	while (!(DMA.CH3.CTRLB & DMA_CH_TRNIF_bm));   //Czekaj na koniece transferu
	};	