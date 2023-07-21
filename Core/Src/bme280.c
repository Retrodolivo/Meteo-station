#include "bme280.h"
#include "delay.h"


static bool trim_read(bme280_t *dev)
{
	// Read NVM from 0x88 to 0xA1
	if (!i2c_mem_read(dev->i2c_port, dev->i2c_addr, 0x88, REG_8BIT, dev->trimdata, 25, 100))
	{
		return false;
	}

	// Read NVM from 0xE1 to 0xE7
	if (!i2c_mem_read(dev->i2c_port, dev->i2c_addr, 0xE1, REG_8BIT, dev->trimdata + 25, 7, 100))
	{
		return false;
	}
	// Arrange the data as per the datasheet (page no. 24)
	dev->comp_pars.dig_T1 = (dev->trimdata[1]<<8) | dev->trimdata[0];
	dev->comp_pars.dig_T2 = (dev->trimdata[3]<<8) | dev->trimdata[2];
	dev->comp_pars.dig_T3 = (dev->trimdata[5]<<8) | dev->trimdata[4];
	dev->comp_pars.dig_P1 = (dev->trimdata[7]<<8) | dev->trimdata[5];
	dev->comp_pars.dig_P2 = (dev->trimdata[9]<<8) | dev->trimdata[6];
	dev->comp_pars.dig_P3 = (dev->trimdata[11]<<8) | dev->trimdata[10];
	dev->comp_pars.dig_P4 = (dev->trimdata[13]<<8) | dev->trimdata[12];
	dev->comp_pars.dig_P5 = (dev->trimdata[15]<<8) | dev->trimdata[14];
	dev->comp_pars.dig_P6 = (dev->trimdata[17]<<8) | dev->trimdata[16];
	dev->comp_pars.dig_P7 = (dev->trimdata[19]<<8) | dev->trimdata[18];
	dev->comp_pars.dig_P8 = (dev->trimdata[21]<<8) | dev->trimdata[20];
	dev->comp_pars.dig_P9 = (dev->trimdata[23]<<8) | dev->trimdata[22];
	dev->comp_pars.dig_H1 = dev->trimdata[24];
	dev->comp_pars.dig_H2 = (dev->trimdata[26]<<8) | dev->trimdata[25];
	dev->comp_pars.dig_H3 = (dev->trimdata[27]);
	dev->comp_pars.dig_H4 = (dev->trimdata[28]<<4) | (dev->trimdata[29] & 0x0f);
	dev->comp_pars.dig_H5 = (dev->trimdata[30]<<4) | (dev->trimdata[29]>>4);
	dev->comp_pars.dig_H6 = (dev->trimdata[31]);

	return true;
}


/* Configuration for the BME280

 * @osrs is the oversampling to improve the accuracy
 *       if osrs is set to OSRS_OFF, the respective measurement will be skipped
 *       It can be set to OSRS_1, OSRS_2, OSRS_4, etc. Check the header file
 *
 * @mode can be used to set the mode for the device
 *       MODE_SLEEP will put the device in sleep
 *       MODE_FORCED device goes back to sleep after one measurement. You need to use the BME280_WakeUP() function before every measurement
 *       MODE_NORMAL device performs measurement in the normal mode. Check datasheet page no 16
 *
 * @t_sb is the standby time. The time sensor waits before performing another measurement
 *       It is used along with the normal mode. Check datasheet page no 16 and page no 30
 *
 * @filter is the IIR filter coefficients
 *         IIR is used to avoid the short term fluctuations
 *         Check datasheet page no 18 and page no 30
 */


status_bme280_t bme280_init(bme280_t *dev, uint8_t osrs_t, uint8_t osrs_p, uint8_t osrs_h, uint8_t mode, uint8_t t_sb, uint8_t filter)
{
	if (!trim_read(dev))
	{
		dev->status_dev = WRITE_READ_ERR;
		return dev->status_dev;
	}

	uint8_t data_write = 0;
	uint8_t data_check = 0;

	// Reset the device
	data_write = 0xB6;  // reset sequence
	if (!i2c_mem_write(dev->i2c_port, dev->i2c_addr, RESET_REG, REG_8BIT, &data_write, 1, 100))
		return dev->status_dev = WRITE_READ_ERR;

	delay_ms(10);

	// write the humidity oversampling to 0xF2
	data_write = osrs_h;
	if (!i2c_mem_write(dev->i2c_port, dev->i2c_addr, CTRL_HUM_REG, REG_8BIT, &data_write, 1, 100))
		return dev->status_dev = WRITE_READ_ERR;

	delay_ms(10);

	i2c_mem_read(dev->i2c_port, dev->i2c_addr, CTRL_HUM_REG, REG_8BIT, &data_check, 1, 100);
	if (data_check != data_write)
		return dev->status_dev = WRITE_READ_ERR;

	delay_ms(10);

	// write the standby time and IIR filter coeff to 0xF5
	data_write = (t_sb <<5) |(filter << 2);
	if (!i2c_mem_write(dev->i2c_port, dev->i2c_addr, CONFIG_REG, REG_8BIT, &data_write, 1, 100))
		return dev->status_dev = WRITE_READ_ERR;

	delay_ms(10);

	i2c_mem_read(dev->i2c_port, dev->i2c_addr, CONFIG_REG, REG_8BIT, &data_check, 1, 100);
	if (data_check != data_write)
		return dev->status_dev = WRITE_READ_ERR;

	delay_ms(10);

	// write the pressure and temp oversampling along with mode to 0xF4
	data_write = (osrs_t << 5) |(osrs_p << 2) | mode;
	if (!i2c_mem_write(dev->i2c_port, dev->i2c_addr, CTRL_MEAS_REG, REG_8BIT, &data_write, 1, 100))
		return dev->status_dev = WRITE_READ_ERR;

	delay_ms(10);

	i2c_mem_read(dev->i2c_port, dev->i2c_addr, CTRL_MEAS_REG, REG_8BIT, &data_check, 1, 100);
	if (data_check != data_write)
		return dev->status_dev = WRITE_READ_ERR;

	dev->status_dev = OK;

	bme280_measure(dev);

	return dev->status_dev;
}

static status_bme280_t bme280_read_raw(bme280_t *dev)
{
	uint8_t raw_data[8];

	// Check the chip ID before reading
	i2c_mem_read(dev->i2c_port, dev->i2c_addr, ID_REG, REG_8BIT, &dev->chip_id, 1, 100);

	if (dev->chip_id == 0x60)
	{
		// Read the Registers 0xF7 to 0xFE
		i2c_mem_read(dev->i2c_port, dev->i2c_addr, PRESS_MSB_REG, REG_8BIT, raw_data, sizeof(raw_data), 100);
		/* Calculate the Raw data for the parameters
		 * Here the Pressure and Temperature are in 20 bit format and humidity in 16 bit format
		 */
		dev->p_raw = (raw_data[0]<<12) | (raw_data[1]<<4)  |(raw_data[2]>>4);
		dev->t_raw = (raw_data[3]<<12) | (raw_data[4]<<4) | (raw_data[5]>>4);
		dev->h_raw = (raw_data[6]<<8) | (raw_data[7]);

		return dev->status_dev = OK;
	}
	else
	{
		dev->status_dev = ID_FAIL;
		return dev->status_dev;
	}
}

int32_t t_fine;
static int32_t bme280_compensate_T_int32(bme280_t *dev)
{
	int32_t var1, var2, T;
	var1 = ((((dev->t_raw>>3) - ((int32_t)dev->comp_pars.dig_T1<<1))) * ((int32_t)dev->comp_pars.dig_T2)) >> 11;
	var2 = (((((dev->t_raw>>4) - ((int32_t)dev->comp_pars.dig_T1)) * ((dev->t_raw>>4) - ((int32_t)dev->comp_pars.dig_T1)))>> 12) *((int32_t)dev->comp_pars.dig_T3)) >> 14;
	t_fine = var1 + var2;
	T = (t_fine * 5 + 128) >> 8;
	return T;
}

static uint32_t bme280_compensate_P_int32(bme280_t *dev)
{
	int32_t var1, var2;
	uint32_t p;
	var1 = (((int32_t)t_fine)>>1) - (int32_t)64000;
	var2 = (((var1>>2) * (var1>>2)) >> 11 ) * ((int32_t)dev->comp_pars.dig_P6);
	var2 = var2 + ((var1 * ((int32_t)dev->comp_pars.dig_P5))<<1);
	var2 = (var2>>2)+(((int32_t)dev->comp_pars.dig_P4)<<16);
	var1 = (((dev->comp_pars.dig_P3 * (((var1>>2) * (var1>>2)) >> 13 )) >> 3) + ((((int32_t)dev->comp_pars.dig_P2) *var1)>>1))>>18;
	var1 =((((32768 + var1)) * ((int32_t)dev->comp_pars.dig_P1))>>15);
	if (var1 == 0)
		return 0; // avoid exception caused by division by zero
	p = (((uint32_t)(((int32_t)1048576)-dev->p_raw)-(var2>>12)))*3125;
	if (p < 0x80000000)
		p = (p << 1) / ((uint32_t)var1);
	else
		p = (p / (uint32_t)var1) * 2;
	var1 = (((int32_t)dev->comp_pars.dig_P9) * ((int32_t)(((p>>3) * (p>>3))>>13)))>>12;
	var2 = (((int32_t)(p>>2)) * ((int32_t)dev->comp_pars.dig_P8))>>13;
	p = (uint32_t)((int32_t)p + ((var1 + var2 + dev->comp_pars.dig_P7) >> 4));
	return p;
}

static uint32_t bme280_compensate_H_int32(bme280_t *dev)
{
	int32_t v_x1_u32r;
	v_x1_u32r = (t_fine - ((int32_t)76800));
	v_x1_u32r = (((((dev->h_raw << 14) - (((int32_t)dev->comp_pars.dig_H4) << 20) - (((int32_t)dev->comp_pars.dig_H5) *\
			v_x1_u32r)) + ((int32_t)16384)) >> 15) * (((((((v_x1_u32r *\
					((int32_t)dev->comp_pars.dig_H6)) >> 10) * (((v_x1_u32r * ((int32_t)dev->comp_pars.dig_H3)) >> 11) +\
							((int32_t)32768))) >> 10) + ((int32_t)2097152)) * ((int32_t)dev->comp_pars.dig_H2) +\
					8192) >> 14));
	v_x1_u32r = (v_x1_u32r - (((((v_x1_u32r >> 15) * (v_x1_u32r >> 15)) >> 7) *\
			((int32_t)dev->comp_pars.dig_H1)) >> 4));
	v_x1_u32r = (v_x1_u32r < 0 ? 0 : v_x1_u32r);
	v_x1_u32r = (v_x1_u32r > 419430400 ? 419430400 : v_x1_u32r);
	return (uint32_t)(v_x1_u32r>>12);
}

/* To be used when doing the force measurement
 * the Device need to be put in forced mode every time the measurement is needed
 */
static void bme280_wakeup(bme280_t *dev)
{
	uint8_t data_write = 0;

	// first read the register
	i2c_mem_read(dev->i2c_port, dev->i2c_addr, CTRL_MEAS_REG, REG_8BIT, &data_write, 1, 100);

	// modify the data with the forced mode
	data_write = data_write | MODE_FORCED;

	// write the new data to the register
	i2c_mem_write(dev->i2c_port, dev->i2c_addr, CTRL_MEAS_REG, REG_8BIT, &data_write, 1, 100);
}

void bme280_measure(bme280_t *dev)
{
	bme280_wakeup(dev);
	if (bme280_read_raw(dev) == OK)
	{
		dev->temperature = (bme280_compensate_T_int32(dev)) / 100.0;  // as per datasheet, the temp is x100
		dev->pressure = bme280_compensate_P_int32(dev);  // as per datasheet, the pressure is Pa
		dev->humidity = (bme280_compensate_H_int32(dev)) / 1024.0;  // as per datasheet, the temp is x1024
	}
}

