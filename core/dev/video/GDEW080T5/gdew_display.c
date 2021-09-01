
#include "system.h"

#include "gdew_board.h"

#include "driver.h"
#include "data.h"
#include "epd.h"
#include "power.h"
#include "picture.h"

u8 wf_mode = EPD_MODE_INIT; //EPD refresh mode

int gdew_display_init(void)
{
	// System initialization
	SYS_Init(); 				

	// IO port initialization
	IO_Init();					

	// SPI initialization
	SPIx_Init();		 		
	SYS_WAKEUP_H;
	hal_delay_ms(100);

	// TPS65185 initialization
	tps_init();					
	tps_sleep_to_standby(); 

	// Check whether the configuration command of the AVT is normal and load waveform
	AVT_CONFIG_check();		
	hal_delay_ms(500);
	avt_waveform_update();

	// Initialize AVT and clear display
	avt_init();					

	wf_mode = EPD_MODE_INIT;
	epd_draw_gray(0xff);
	wf_mode = EPD_MODE_GC16;
	tps_standby_to_sleep();
	avt_slp();

	return 0;	
}


int gdew_display_test(void)
{

#if 0 //16-level grayscale demo
	Delay100ms(20);

	wf_mode = EPD_MODE_GC16;
	epd_draw_gray_level_horizontal(16);
	Delay100ms(20);

	epd_draw_gray_level_vertical(16);
	Delay100ms(20);

	epd_draw_gray_level_horizontal(8);
	Delay100ms(20);

	epd_draw_gray_level_vertical(8);
	Delay100ms(20);

	epd_draw_gray(0xff);
#endif

	// Mobile display LOGO
	wf_mode = EPD_MODE_DU;
	avt_run_sys();		 //Partially refresh the picture
	avt_lut_demo(0);	 //LOGO display
	avt_lut_demo(255); //LOGO disappears
	avt_slp();
	Delay100ms(2); //2
	//Full gray
	wf_mode = EPD_MODE_GC16;
	epd_draw_gray(0xff); //Full gray
	Delay100ms(1);			//1

	while (1)
	{
		//Full gray
		wf_mode = EPD_MODE_GC16;
		epd_draw_gray(0xff);																			 //Full gray
		Delay100ms(1);																					 //1
																											 //4 grayscale full screen display
		epd_draw_pic_part_from_rom((u8 *)gImage_1, EPD_DATA_2BPP, 0, 0, 1024, 758); //Full screen display, resolution 1024*758
		while (1)
			;
	}

	return 0;
}
