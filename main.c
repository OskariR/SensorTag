
#include <stdio.h>

/* XDCtools files */
#include <xdc/std.h>
#include <xdc/runtime/System.h>

/* BIOS Header files */
#include <ti/sysbios/BIOS.h>
#include <ti/sysbios/knl/Clock.h>
#include <ti/sysbios/knl/Task.h>
#include <ti/drivers/PIN.h>
#include <ti/drivers/pin/PINCC26XX.h>
#include <ti/drivers/I2C.h>
#include <ti/drivers/Power.h>
#include <ti/drivers/power/PowerCC26XX.h>
#include <ti/mw/display/Display.h>
#include <ti/mw/display/DisplayExt.h>
#include <ti/mw/remotecontrol/buzzer.h>

/* Board Header files */
#include "Board.h"

#include "wireless/comm_lib.h"
#include "sensors/bmp280.h"
#include "wireless/address.h"

/* Task */
#define STACKSIZE 2048
Char labTaskStack[STACKSIZE];
Char commTaskStack[STACKSIZE];

/* Display */
Display_Handle hDisplay;

// JTKJ: Pin configuration and variables here
// JTKJ: Painonappien konfiguraatio ja muuttujat
static PIN_Handle buttonHandle;
static PIN_State buttonState;

static PIN_Handle buttonHandle1;
static PIN_State buttonState1;

static PIN_Handle ledHandle;
static PIN_State ledState;

static PIN_Handle buzzHandle;
static PIN_State buzzState;

int mselect = 0;

PIN_Config buttonConfig[] = {
   Board_BUTTON0  | PIN_INPUT_EN | PIN_PULLUP | PIN_IRQ_NEGEDGE,
   PIN_TERMINATE
};

PIN_Config buttonConfig1[] = {
   Board_BUTTON1  | PIN_INPUT_EN | PIN_PULLUP | PIN_IRQ_NEGEDGE,
   PIN_TERMINATE
};


PIN_Config ledConfig[] = {
   Board_LED1 | PIN_GPIO_OUTPUT_EN | PIN_GPIO_LOW | PIN_PUSHPULL | PIN_DRVSTR_MAX, 
   PIN_TERMINATE
};





void buttonFxn(PIN_Handle handle, PIN_Id pinId) {
    
   //PIN_setOutputValue( ledHandle, Board_LED1, !PIN_getOutputValue( Board_LED1 ) );
   
   mselect++;
   
   if (mselect == 3) {
       mselect = 0;
   }

    System_printf("mselect = %x \n", mselect);
    System_flush();


  //char msg[4] = "Doge";
  // sprintf(msg, "Doge");
   
  // Send6LoWPAN(IEEE80154_SERVER_ADDR, msg, 4);
   
  // StartReceive6LoWPAN();
   
}

void buttonFxn1(PIN_Handle handle, PIN_Id pinId) {
    
   mselect--;
   
   if (mselect == -1) {
       mselect = 2;
   }
    
}

Void labTaskFxn(UArg arg0, UArg arg1) {

    I2C_Handle      i2c;
    I2C_Params      i2cParams;
    
    /* Create I2C for sensors */
    I2C_Params_init(&i2cParams);
    i2cParams.bitRate = I2C_400kHz;
    i2c = I2C_open(Board_I2C0, &i2cParams);
    if (i2c == NULL) {
        System_abort("Error Initializing I2C\n");
    }
    else {
        System_printf("I2C Initialized!\n");
    }

    // JTKJ: Setup the BMP280 sensor here, before its use
    // JTKJ: Sensorin alustus t�ss� kirjastofunktiolla
    
    //bmp280_setup(i2c);

    /* Display */
    Display_Params displayParams;
	displayParams.lineClearMode = DISPLAY_CLEAR_BOTH;
    Display_Params_init(&displayParams);

    hDisplay = Display_open(Display_Type_LCD, &displayParams);
    if (hDisplay == NULL) {
        System_abort("Error initializing Display\n");
    }
    Display_clear(hDisplay);

    // JTKJ: Some text to screen?
    // JTKJ: Teksti� ruudulle?
    
    
    //char str[16];
    //sprintf(str, "%x", IEEE80154_MY_ADDR);
    //Display_print0(hDisplay, 0, 0, str);
    
    

    while (1) {
        // DO SOMETHING HERE..
    
        if (mselect == 0) {
            Display_print0(hDisplay, 4, 3, ">Gesture<");
        }
        else {
            Display_print0(hDisplay, 4, 4, "Gesture");
        }
    
    
        if (mselect == 1) {
            Display_print0(hDisplay, 5, 3, ">Sensors<");
        }
        else {
            Display_print0(hDisplay, 5, 4, "Sensors");
        }
        
        
        if (mselect == 2) {
            Display_print0(hDisplay, 6, 4, ">Music<");
        }
        else {
            Display_print0(hDisplay, 6, 5, "Music");
        }
        
        
    	// 60fps
    	Task_sleep(16666 / Clock_tickPeriod);
    }

    while (1) {
        // DO SOMETHING HERE..
        
        
        
    	// Once per second
    	Task_sleep(1000000 / Clock_tickPeriod);
    }
}

/* Communication Task */
Void commTaskFxn(UArg arg0, UArg arg1) {

    // Radio to receive mode
	int32_t result = StartReceive6LoWPAN();
	if(result != true) {
		System_abort("Wireless receive mode failed");
	}

    while (1) {

        // If true, we have a message
    	if (GetRXFlag() == true) {

    		// Handle the received message..
        }

    	// Absolutely NO Task_sleep in this task!!
    }
}

// JTKJ: Write the interrupt handler for the button and implement its functionality
// JTKJ: Laadi painonapille keskeytyksen k�sittelij�funktio ja toteudu sille vaadittu toiminnallisuus

Int main(void) {

    // Task variables
	Task_Handle labTask;
	Task_Params labTaskParams;
	Task_Handle commTask;
	Task_Params commTaskParams;

    // Initialize board
    Board_initGeneral();

	// JTKJ: Open and configure the button and led pins here
    // JTKJ: Painonappi- ja ledipinnit k�ytt��n t�ss�
    
   buttonHandle = PIN_open(&buttonState, buttonConfig);
   buttonHandle1 = PIN_open(&buttonState1, buttonConfig1);
   
  // buzzHandle = buzzerOpen(BOARD_BUZZER);
   
   if(!buttonHandle) {
      System_abort("Error initializing button pins\n");
   }
   
   if(!buttonHandle1) {
      System_abort("Error initializing button pins\n");
   }
   
   if(!buzzHandle) {
      System_abort("Error initializing button pins\n");
   }
   
   
   ledHandle = PIN_open(&ledState, ledConfig);
   
   if(!ledHandle) {
      System_abort("Error initializing LED pins\n");
   }
	// JTKJ: Register the interrupt handler for the button
    // JTKJ: Rekister�i painonapille keskeytyksen k�sittelij�funktio

   if (PIN_registerIntCb(buttonHandle, &buttonFxn) != 0) {
      System_abort("Error registering button callback function");
   }
   
   if (PIN_registerIntCb(buttonHandle1, &buttonFxn1) != 0) {
      System_abort("Error registering button callback function");
   }

    /* Task */
    Task_Params_init(&labTaskParams);
    labTaskParams.stackSize = STACKSIZE;
    labTaskParams.stack = &labTaskStack;
    labTaskParams.priority=2;

    labTask = Task_create(labTaskFxn, &labTaskParams, NULL);
    if (labTask == NULL) {
    	System_abort("Task create failed!");
    }

    /* Communication Task */
    Init6LoWPAN(); // This function call before use!

    Task_Params_init(&commTaskParams);
    commTaskParams.stackSize = STACKSIZE;
    commTaskParams.stack = &commTaskStack;
    commTaskParams.priority=1;

    commTask = Task_create(commTaskFxn, &commTaskParams, NULL);
    if (commTask == NULL) {
    	System_abort("Task create failed!");
    }

    System_printf("Hello world!\n");
    System_flush();
    
    /* Start BIOS */
    BIOS_start();

    return (0);
}

