//Referenced from https://github.com/ControlEverythingCommunity/TMP112/blob/master/C/TMP112.c

#include <stdio.h>
#include <stdlib.h>
#include <linux/i2c-dev.h>
#include <sys/ioctl.h>
#include <fcntl.h>

void main() 
{
	// Create I2C bus
	int file;
	char *bus = "/dev/i2c-1";
	if((file = open(bus, O_RDWR)) < 0) 
	{
		printf("Failed to open the bus. \n");
		exit(1);
	}
	// Get I2C device, TMP112 I2C address is 0x48(72)
	ioctl(file, I2C_SLAVE, 0x48);

	// Select configuration register(0x01)
	// Continous Conversion mode, 12-Bit Resolution, Fault Queue is 1(0x60)
	// Polarity low, Thermostat in Comparator mode, Disables Shutdown mode(0xA0)
	char config[3] = {0};
	config[0] = 0x01;
	config[1] = 0x60;
	config[2] = 0xA0;
	write(file, config, 3);
	sleep(1);

	// Read 2 bytes of data from register(0x00)
	// temp msb, temp lsb
	char reg[1] = {0x00};
	write(file, reg, 1);
	char data[2] = {0};
	if(read(file, data, 2) != 2)
	{
		printf("Error : Input/Output error \n");
		exit(1);
	}
	else
	{
		// Convert the data to 12-bits
		int temp = (data[0] * 256 + data[1]) / 16;
		if(temp > 2047)
		{
			temp -= 4096;
		}
		float cTemp = temp * 0.0625;
		float fTemp = (cTemp * 1.8) + 32;
	
		// Output data to screen
		printf("Temperature in Celsius : %.2f C \n", cTemp);
		printf("Temperature in Fahrenheit : %.2f F \n", fTemp);
	}
}
