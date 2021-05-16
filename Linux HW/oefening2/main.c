#include <stdio.h>
#include <stdbool.h>
#include <unistd.h>
#include <dirent.h>

void initGPIO(int pin);
void setDirection(int pin, bool inOut);
void ledFlashing(int pin, int times);
void readGPIO(int);


int main(void)
{
	setDirection(22,1);
	ledFlashing(17,20);
	readGPIO(27);

	return 0;
}

//Function to enable a GPIO line 
void initGPIO(int pin)
{
	FILE* fp;
	fp = fopen("/sys/class/gpio/export","w");
	fprintf(fp,"%d",pin);
	fclose(fp);
}
	
//Function to enable a GPIO line and assign direction
//Input : int pin -> GPIO pin number
//Input : bool inOut -> 0 is in,1 is out	
void setDirection(int pin, bool inOut)
{
	FILE* fp;
	DIR* dir;
	char buffer[100];
	
	//check if directory exist otherwise create gpio line
	sprintf(buffer, "/sys/class/gpio/gpio%d", pin);
	dir = opendir(buffer);
	if(dir != 1)
		initGPIO(pin);
		
	
	//sprintf(buffer, "/sys/class/gpio/gpio%d/direction", pin);
	sprintf(buffer, "/sys/class/gpio/gpio%d/direction", pin);
	fp = fopen(buffer,"w");
	if(inOut)
	{
		fprintf(fp, "out");
	}
	else
	{
	fprintf(fp, "in");
	}
		
	fclose(fp);
}

void readGPIO(int pin)
{
	int i;
	FILE* fp;
	char value[5];
	char buf[100];
	//check if directory exist otherwise create gpio line and/or set to in
	setDirection(pin,0);
	
	sprintf(buf, "/sys/class/gpio/gpio%d/value", pin);

	fp = fopen(buf,"r");
	fgets(value,5,fp);
	printf("%s \n",value);
	fclose(fp);

}
	
void ledFlashing(int pin, int times)
{
	FILE* fp;
	char buffer[100];
	
	// will set the dircetion correct and check if gpio pin is already enabled
	setDirection(pin,1);
	
	sprintf(buffer, "/sys/class/gpio/gpio%d/value",pin);
	
	fp = fopen(buffer,"w+");
	setvbuf(fp,NULL, _IONBF,1);
	
	while(1)
	{

		fprintf(fp,"1");
		fprintf(fp,"0");

	}
	fclose(fp);	
	
}

