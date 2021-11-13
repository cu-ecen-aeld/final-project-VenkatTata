//Referenced from https://github.com/ControlEverythingCommunity/TMP112/blob/master/C/TMP112.c

#include <stdio.h>
#include <stdlib.h>
#include <linux/i2c-dev.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <syslog.h>

void main() 
{
	// Create I2C bus
	int file;
	int adapter_nr = 1; //Always adapter 1 on RPi
	char i2c_dev_filename[20];

	snprintf(i2c_dev_filename,"/dev/i2c-%d",adapter_nr);
	file = open(i2c_dev_filename, O_RDWR);
	if(file < 0) 
	{
		syslog("Failed to open the i2c-1 bus");
		exit(1);
	}

	int addr= 0x48; TMP102 I2C address is 0x48(72)
	
	//set the address of the device to address
	if(ioctl(file, I2C_SLAVE, addr) < 0)
	{
		syslog("Failed to set the address of the device to address");
		exit(1);
	}

	// Select configuration register(0x01)
	// Continous Conversion mode, 12-Bit Resolution
	char buf[3] = {0x01,0x60,0xA0};
	//buf[0] = 0x01;
	//buf[1] = 0x60;
	//buf[2] = 0xA0;
	write(file, buf, 3);

	//Wait for the transaction to complete and sensor to initialise and perform measurement
	sleep(1);
	
	//On completing the measurement, the values can be read
	char reg[1] = {0x00};
	write(file, reg, 1);

	//read the measured temperature value
	char measured_temp[2] = {0};
	if(read(file, measured_temp, 2) != 2)
	{
		syslog("i2c read transaction failed");
		exit(1);
	}
	
	//Convert register values to temperature measurements
	int temp = (measured_temp[0] * 256 + measured_temp[1]) / 16;
	if(temp > 2047)
	{
		temp -= 4096;
	}
	syslog("Temperature in Celsius : %d degree C", temp * 0.0625);
}