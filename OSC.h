/*
 * OSC.h
 *
 * Created: 2014-10-25 21:42:37
 *  Author: tmf
 */ 


#ifndef OSC_H_
#define OSC_H_


_Bool RC32M_en();    //Odblokuj generator RC 32 MHz
void SelectPLL(OSC_PLLSRC_t src, uint8_t mult);  //Wybierz �r�d�o i mno�nik PLL

static inline void CLK_Sel(CLK_SCLKSEL_t clk)    //Wybierz �r�d�o taktowania MCU
{
	CPU_CCP=CCP_IOREG_gc;                //Odblokuj zmian� konfiguracji
	CLK.CTRL=clk;                        //Wybierz �r�d�o taktowania
}

#endif /* OSC_H_ */