/*
 * OSC.h
 *
 * Created: 2014-10-25 21:42:37
 *  Author: tmf
 */ 


#ifndef OSC_H_
#define OSC_H_


_Bool RC32M_en();    //Odblokuj generator RC 32 MHz
void SelectPLL(OSC_PLLSRC_t src, uint8_t mult);  //Wybierz Ÿród³o i mno¿nik PLL

static inline void CLK_Sel(CLK_SCLKSEL_t clk)    //Wybierz Ÿród³o taktowania MCU
{
	CPU_CCP=CCP_IOREG_gc;                //Odblokuj zmianê konfiguracji
	CLK.CTRL=clk;                        //Wybierz Ÿród³o taktowania
}

#endif /* OSC_H_ */