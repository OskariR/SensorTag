/*
 * bmp280.c
 *
 *  Created on: 29.10.2016
 *  Author: Teemu Leppanen / UBIComp / University of Oulu
 *
 * 	Datasheet: https://ae-bst.resource.bosch.com/media/_tech/media/datasheets/BST-BMP280-DS001-12.pdf
 */

#include <xdc/runtime/System.h>
#include <stdio.h>
#include "Board.h"
#include "bmp280.h"

// konversiovakiot
uint16_t dig_T1;
int16_t  dig_T2;
int16_t  dig_T3;
uint16_t dig_P1;
int16_t  dig_P2;
int16_t  dig_P3;
int16_t  dig_P4;
int16_t  dig_P5;
int16_t  dig_P6;
int16_t  dig_P7;
int16_t  dig_P8;
int16_t  dig_P9;
int32_t	 t_fine = 0;

// i2c
I2C_Transaction i2cTransaction;
char itxBuffer[4];
char irxBuffer[24];
char str[80];

void bmp280_set_trimming(char *v) {

	dig_T1 = (v[1] << 8) | v[0];
	dig_T2 = (v[3] << 8) | v[2];
	dig_T3 = (v[5] << 8) | v[4];
	dig_P1 = (v[7] << 8) | v[6];
	dig_P2 = (v[9] << 8) | v[8];
	dig_P3 = (v[11] << 8) | v[10];
	dig_P1 = (v[7] << 8) | v[6];
	dig_P2 = (v[9] << 8) | v[8];
	dig_P3 = (v[11] << 8) | v[10];
	dig_P4 = (v[13] << 8) | v[12];
	dig_P5 = (v[15] << 8) | v[14];
	dig_P6 = (v[17] << 8) | v[16];
	dig_P7 = (v[19] << 8) | v[18];
	dig_P8 = (v[21] << 8) | v[20];
	dig_P9 = (v[23] << 8) | v[22];
}

double bmp280_temp_compensation(uint32_t adc_T) {

	double ret = 0.0;
	int32_t var1, var2;

	var1 = ((((adc_T>>3) - ((int32_t)dig_T1 <<1))) * ((int32_t)dig_T2)) >> 11;
	var2 = (((((adc_T>>4) - ((int32_t)dig_T1)) * ((adc_T>>4) - ((int32_t)dig_T1))) >> 12) * ((int32_t)dig_T3)) >> 14;
	t_fine = var1 + var2;

	ret = (double)((t_fine * 5 + 128) >> 8);
	ret /= 100.0;
	return ret;
}

double bmp280_convert_pres(uint32_t adc_P) {

	double ret = 0.0;
	int64_t var1, var2, p;

	var1 = ((int64_t)t_fine) - 128000;
	var2 = var1 * var1 * (int64_t)dig_P6;
	var2 = var2 + ((var1*(int64_t)dig_P5)<<17);
	var2 = var2 + (((int64_t)dig_P4)<<35);
	var1 = ((var1 * var1 * (int64_t)dig_P3)>>8) + ((var1 * (int64_t)dig_P2)<<12);
	var1 = (((((int64_t)1)<<47)+var1))*((int64_t)dig_P1)>>33;
	if (var1 == 0) {
	    return 0.0;  // avoid exception caused by division by zero
	}
	p = 1048576 - adc_P;
	p = (((p<<31) - var2)*3125) / var1;
	var1 = (((int64_t)dig_P9) * (p>>13) * (p>>13)) >> 25;
	var2 = (((int64_t)dig_P8) * p) >> 19;

	ret = ((p + var1 + var2) >> 8) + (((int64_t)dig_P7)<<4);
	ret /= 256.0;
	return ret;
}

void bmp280_setup(I2C_Handle *i2c) {

    i2cTransaction.slaveAddress = Board_BMP280_ADDR;
    itxBuffer[0] = BMP280_REG_CONFIG;
    itxBuffer[1] = 0x40;
    i2cTransaction.writeBuf = itxBuffer;
    i2cTransaction.writeCount = 2;
    i2cTransaction.readBuf = NULL;
    i2cTransaction.readCount = 0;

    if (I2C_transfer(*i2c, &i2cTransaction)) {

        System_printf("BMP280: Config write ok\n");
    } else {
        System_printf("BMP280: Config write failed!\n");
    }
    System_flush();

    i2cTransaction.slaveAddress = Board_BMP280_ADDR;
    itxBuffer[0] = BMP280_REG_CTRL_MEAS;
    itxBuffer[1] = 0x2F;
    i2cTransaction.writeBuf = itxBuffer;
    i2cTransaction.writeCount = 2;
    i2cTransaction.readBuf = NULL;
    i2cTransaction.readCount = 0;

    if (I2C_transfer(*i2c, &i2cTransaction)) {

        System_printf("BMP280: Ctrl meas write ok\n");
    } else {
        System_printf("BMP280: Ctrl meas write failed!\n");
    }
    System_flush();

    i2cTransaction.slaveAddress = Board_BMP280_ADDR;
    itxBuffer[0] = BMP280_REG_T1;
    i2cTransaction.writeBuf = itxBuffer;
    i2cTransaction.writeCount = 1;
    i2cTransaction.readBuf = irxBuffer;
    i2cTransaction.readCount = 24;

    if (I2C_transfer(*i2c, &i2cTransaction)) {

        System_printf("BMP280: Trimming read ok\n");
    } else {
        System_printf("BMP280: Trimming read failed!\n");
    }
    System_flush();

    bmp280_set_trimming(irxBuffer);
}

void bmp280_get_data(I2C_Handle *i2c, double *pres, double *temp) {

	uint32_t raw_t = 0;
	uint32_t raw_p = 0;

	// JTKJ: Uncomment and fill in the data structure below with correct values..
	// JTKJ: Kommentit pois ja t�ytet��n i2c-viestirakenne luentomateriaalin avulla..
	
	/*
	uint8_t txBuffer[1];
    uint8_t rxBuffer[6];
    i2cTransaction.slaveAddress = Board_BMP280_ADDR;
    txBuffer[0] = BMP280_REG_PRES;
    i2cTransaction.writeBuf = txBuffer;
    i2cTransaction.writeCount = 1;
    i2cTransaction.readBuf = rxBuffer;
    i2cTransaction.readCount = 6;
    */

    if (I2C_transfer(*i2c, &i2cTransaction)) {

    	// JTKJ: Parse the actual temperature and pressure values received in rxBuffer.
    	// JTKJ: Laske mittausarvot i2c-viestin puskurista rxBuffer. K�ytt�k�� hyv�ksi harjoitusteht�v�n koodia..
/*
        uint32_t xlsb_tmp = rxBuffer[0] >> 4;
        uint32_t lsb_tmp = rxBuffer[1] << 4;
        uint32_t msb_tmp = rxBuffer[2] << 12;
        
        uint32_t lampotila = xlsb_tmp | lsb_tmp | msb_tmp;
        
        uint32_t xlsb_pre = rxBuffer[3] >> 4;
        uint32_t lsb_pre = rxBuffer[4] << 4;
        uint32_t msb_pre = rxBuffer[5] << 12;
        
        uint32_t pressure = xlsb_pre | lsb_pre | msb_pre;
    

    	// JTKJ: Convert the parsed raw data into temperature and pressure values
    	// JTKJ: Muunna rekisterin raakadata l�mp�tila- ja ilmanpainearvoiksi
    	
    	int larvo = bmp280_bmp280_temp_compensation(lampotila);
        int parvo = bmp280_convert_pres(pressure);
        
        System_printf("%i", larvo);
        
        //  char tmpstr[16];
        // sprintf(tmpstr, "%x", larvo);
        //Display_print0(hDisplay, 1, 0, tmpstr);
        
*/
    } else {

        System_printf("BMP280: Data read failed!\n");
    }
}

