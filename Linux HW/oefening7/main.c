#include <stdio.h>
#include <stdbool.h>
#include "/usr/include/linux/i2c-dev.h"
#include <stdlib.h>
#include <fcntl.h>
#include <math.h>
#include <time.h>

void sensorInit(int file);
int convertData(int decNumber);
int binaryToDec(int binNumber[], int size);
int toThePower(int i);
void calculateAngle(int x,int y,int z);

int main(void)
{
	int file;
	int res;
	int slaveAddr = 0x69;
	int x_component_addr = 0x28; //and also 0x29
	int y_component_addr = 0x2A; //and also 0x2B
	int z_component_addr = 0x2C; //ans also 0x2D
	int x, y, z , x_H, x_L, y_H, y_L, z_H, z_L;
	time_t new_t=0, prev_t;
	double x_angle=0, y_angle=0, z_angle=0, diff_t;
	
	if( (file = open("/dev/i2c-1",O_RDWR)) < 0)
		return 0;

	if((res=ioctl(file,I2C_SLAVE,slaveAddr)) <0 )
		return 0;

	sensorInit(file);
	prev_t = new_t;
	time(&new_t);
	diff_t = difftime(new_t, prev_t);
	while(1)
	{
		x_L = i2c_smbus_read_byte_data(file, x_component_addr);
		x_H = i2c_smbus_read_byte_data(file, x_component_addr+1);
		x = (x_H << 8) + x_L;
		x = convertData(x)* 0.00875;
		
		y_L = i2c_smbus_read_byte_data(file, y_component_addr);
		y_H = i2c_smbus_read_byte_data(file, y_component_addr+1);
		y = (y_H << 8) + y_L;
		y = convertData(y) * 0.00875;
		
		z_L = i2c_smbus_read_byte_data(file, z_component_addr);
		z_H = i2c_smbus_read_byte_data(file, z_component_addr+1);
		z = (z_H << 8) + z_L;
		z = convertData(z) * 0.00875;		
		
		
		printf("x = %d\n",x);
		printf("y = %d\n",y);
		printf("z = %d\n",z);
		printf("-----\n");
		
		prev_t = new_t;
		time(&new_t);
		diff_t = difftime(new_t, prev_t);
		
		x_angle += x * diff_t;
		y_angle += y * diff_t;
		z_angle += z * diff_t;
		
		printf("x angle = %f\n",x_angle);
		printf("y angle = %f\n",y_angle);
		printf("z angle = %f\n",z_angle);
		printf("-----\n");
	
	}
	
	
	return 0;
}

void sensorInit(int file)
{
	int CTRL_REG1 = 0x20;
	int CTRL_REG4 = 0x23;
			
	(int) i2c_smbus_write_byte_data(file, CTRL_REG1, 0x0F);
	(int) i2c_smbus_write_byte_data(file, CTRL_REG4, 0x80);
}



int convertData(int decNumber)
{
	int i,binary[16] ={0}, binary_complement[15] = {0}, temp;
	
	for(i=0; decNumber>0; i++)
	{
		binary[i]=decNumber%2;
		decNumber=decNumber/2;
	}
	
	if(binary[15] ==1)
	{
		binary[15] =0;
		temp =binaryToDec(binary,16);
		temp--;
		
		for(i=0; temp>0; i++)
		{
			binary_complement[i]=temp%2;
			temp=temp/2;
		}
		
		for(i=0;i<15;i++)
		{
			if(binary_complement[i] == 0)
				binary_complement[i] = 1;
			else
				binary_complement[i] = 0;
		}
		return -binaryToDec(binary_complement,15);
	}
	
	return binaryToDec(binary,16);
}

int binaryToDec(int binNumber[], int size)
{
	int i, decNumber=0;
	for(i=0; i<size; i++)
	{
		decNumber += binNumber[i] * toThePower(i);
	}
	return decNumber;
}

int toThePower(int i)
{
	int out =1, j;
		
	for(j=1;j<=i;j++)
	{
		out *= 2;
	}
	return out;
}




