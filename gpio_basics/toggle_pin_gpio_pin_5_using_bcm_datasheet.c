#include<sys/mman.h>
#include<stdio.h>
#include<stdlib.h>
#include<fcntl.h>
#include<unistd.h>


//setting up the initial address
#define BCM2837_PERI_ADDR 0x3F000000 //base address for peripherals
#define GPIO_BASE (BCM2837_PERI_ADDR + 0x200000) //starting of the gpio address
#define BLOCK_SIZE (4*1024) //block size for reading in the mmap

volatile unsigned int *gpio;


//Constants and Macros
#define INPUT  0
#define OUTPUT 1
#define ALT0   4
#define ALT1   5
#define ALT2   6
#define ALT3   7
#define ALT4   3
#define ALT5   2
#define GPFSEL0 (unsigned int *)(gpio+0);  //this should be equivalent of 0x7E200000 + 0x0
#define GPSET0  (unsigned int *)(gpio+7); //this should be equivalent of 0x7E200000 + 0x0000001C using 7 because even though the difference is 28 bits that is converted to 7 bytes
#define GPCLR0  (unsigned int *)(gpio+10); //this should be equivalent of 0x7E200000 + 0x00000028 using 10 because even though the difference is 40 bits that is converted to 10 bytes
#define GPLEV0  (unsigned int *)(gpio+13); //this should be equivalent of 0x7E200000 + 0x00000034 using 13 because even though the difference is 52 bits that is converted to 13 bytes

int main(){
	
	//access the memory of bcm2837
	int mem_fd; //memory file descriptor
	void *reg_map; //map of the memory, its a void pointer so it wont'contain the type of data type it is
	//accessing /dev/mem for memory
	if ((mem_fd = open("/dev/mem", O_RDWR|O_SYNC) ) < 0) { //O
	      printf("can't open /dev/mem \n");
	      exit(-1);
	}
	//doing the memory map of the peripherals and it starts from GPIO_BASE and reads as large as BLOCK sized
	reg_map = mmap(
            NULL,             //Address at which to start local mapping (null means don't-care)
            BLOCK_SIZE,       //Size of mapped memory block
            PROT_READ|PROT_WRITE,// Enable both reading and writing to the mapped memory
            MAP_SHARED,       // This program does not have exclusive access to this memory. All changes made to the memory mapped written region will be written back to file/device
				mem_fd,           // Map to /dev/mem
            GPIO_BASE);       // Offset to GPIO peripheral
	
	if (reg_map == MAP_FAILED) {
            printf("gpio mmap error %d\n", (int)reg_map);
            close(mem_fd);
            exit(-1);
        }
    gpio = (volatile unsigned *)reg_map;
    unsigned int offset = 5/10; //I understand you have to move the pointer to that location but I dont know how this math works out 
    //printf("GPFSEL0 0x%x\n", (*(gpio+7))&(0b00000000000000001000000000000000));
    printf("GPLEV0  0x%x\n", *(gpio+13));
    
    //GPIO select 5th pin
    //GPFSEL0 = GPFSEL0 | (0b00000000000000001000000000000000);
    printf("GPFSEL0 0x%x\n", *(gpio));
    //*(gpio) |= 0b00000000000000 001 000000000000000;
    *(gpio) &= (0x0<<15);//0b00000000000000000000000000000000;
    printf("GPFSEL0 0x%x\n", *(gpio));
    
    //get initial read of pin
    //GPLEV0[offset] = GPLEV0[offset] & (0x1<<(5%32));
    printf("GPLEV0: 0x%x\n",*(gpio+13));
    //*(gpio+13) &= (0x1<<(5%32));
    *(gpio+13) |= (0x1<<5);
    printf("GPLEV0: 0x%x\n",*(gpio+13));

    //clear the 0 registers
    //GPCLR0[offset] = GPCLR0[offset] | (0x1<<5%32);
    printf("GPCLR0: 0x%x\n",*(gpio+10));
    //*(gpio+10) |= (0x1<<5%32);
    *(gpio+10) |= (0x1<<5);
    printf("GPCLR0: 0x%x\n",*(gpio+10));

    //set the pin to output
    //GPSET0[offset] = GPSET0[offset] | (0x1<<5%32);
    printf("GPSET0: 0x%x\n",*(gpio+7));
    //*(gpio+7) |= (0x1<<5%32);
    *(gpio+7) |= (0x1<<5);
    //*(gpio+7) &= (0x1<<5%32);
    printf("GPSET0: 0x%x\n",*(gpio+7));

    //read pin status
    //GPLEV0[offset] =  GPLEV0[offset] & (0x1<<5%32);
    printf("GPLEV0: 0x%x\n",*(gpio+13));
    //*(gpio+13) &= (0x1<<(5%32));
    *(gpio+13) |= (0x1<<5);
    printf("GPLEV0: 0x%x\n",*(gpio+13));
}























