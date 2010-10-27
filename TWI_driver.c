/*********** AVR155 : Accessing I2C LCD display using the AVR TWI ***********

Device      : 	ATmega163

File name   : 	TWI_driver.c

Ver nr.     : 	1.0

Description : 	TWI_driver.c is a driver to ease the approach to AVRs TWI 
				module. It requires a package containing slave adresse, 
				number of bytes to handle and a pointer. The pointer tells 
				where to find the bytes to send and where the temp buffer for
				bytes to receive are located. Dependent on a succesfull 
				communication or not it will return SUCCESS if no problems or 
				the AVR TWSR if error occured. The driver will handle all 
				signaling and handeling of START/STOP and bus error.
				
				Se appnote for more documentation on the drivers input 
				and output.
							
Author      : 	Asmund Saetre

Change log  : 	Created 12.05.2000
 
****************************************************************************/
#include "TWI_driver.h"															


/****************************************************************************
	Function : char Init_TWI(void)
	Setup the TWI module
	Baudrate 	: 100kHz @ 4MHz system clock
	Own address : OWN_ADR (Defined in TWI_driver.h)
****************************************************************************/

char Init_TWI(void)
{
	TWAR = OWN_ADR;							//Set own slave address
	TWBR = 32;                  //Set baud-rate to 100 KHz at 
                              //4 MHz xtal
	TWCR = (1<<TWEN);						//Enable TWI-interface

    return 1;
}    

/****************************************************************************
	Function : void Wait_TWI_int(void)
	Loop until TWI interrupt flag is set
****************************************************************************/
void Wait_TWI_int(void)
{

	while (!(TWCR & (1<<TWINT)))
	    ; 
}    

/****************************************************************************
	Function :unsigned char	Send_start(void)
	Send a START condition to the bus and wait for the TWINT get set set to 
	see the result. If it failed return the TWSR value, if succes return 
	SUCCESS.
****************************************************************************/
unsigned char	Send_start(void)
{
	TWCR = (1<<TWINT)|(1<<TWSTA)|(1<<TWEN);  //Send START
	
	Wait_TWI_int();							//Wait for TWI interrupt flag set

    if((TWSR != START)&&(TWSR != REP_START))//If status other than START 
		return TWSR;						//transmitted(0x08) or Repeated
	return SUCCESS;							//START transmitted(0x10) 
    										//-> error  and return TWSR.
    										//If success return	SUCCESS
}							

/****************************************************************************
	Function :
	Send a STOP condition to the bus
****************************************************************************/
void Send_stop(void)
{
	TWCR = ((1<<TWEN)+(1<<TWINT)+(1<<TWSTO));//Send STOP condition
}        
    
/****************************************************************************
	Function : unsigned char Send_to_TWI(tx_type *data_pack)
	Main TWI communication function. Handles all signaling against the bus.
****************************************************************************/
unsigned char Send_to_TWI(tx_type *data_pack) {

	unsigned char state,i,j;
  state = SUCCESS;
	
	for(i=0;(data_pack[i].slave_adr != OWN_ADR)&&(state == SUCCESS);i++) {
		state = Send_start();				//Send START and repeated START
		if (state == SUCCESS)				
			state = Send_adr(data_pack[i].slave_adr);//Send slave address+W/R
		
		/*Dependent on the R/W in the slave address it will receive or 
		transmitt data.*/
		if(!(data_pack[i].slave_adr & R))
		{
			/*If W, it will transmitt data and loops until all data have been
			 shifted out*/
			for(j=0;((j<data_pack[i].size)&&(state == SUCCESS));j++)
				state = Send_byte(data_pack[i].data_ptr[j]);
		}	
		else								//Else read from slave 
		{
			/*If R, it will receive data and loops until all data have been
			 shifted in. When j == 0x00 this is the last byte and a NACK 
			 will be sent to the slave to make it stop transmitting */
			for(j=0;((j<data_pack[i].size)&&(state == SUCCESS));j++)
				state = Get_byte(data_pack[i].data_ptr++,j);
		}					
	}
  Send_stop();							//Send STOP
  	  
	return state;							//Return SUCCESS if OK, 
											//else return TWSR 
}

/****************************************************************************
	Function : unsigned char Send_byte(unsigned char data)
	Send one byte to the bus.
****************************************************************************/
unsigned char Send_byte(unsigned char data)
{


	Wait_TWI_int();							//Wait for TWI interrupt flag set

	TWDR = data;
 	TWCR = ((1<<TWINT)+(1<<TWEN));   		//Clear int flag to send byte 

	Wait_TWI_int();							//Wait for TWI interrupt flag set

	if(TWSR != MTX_DATA_ACK)				//If NACK received return TWSR
		return TWSR;																
	return SUCCESS;							//Else return SUCCESS
}	

/****************************************************************************
	Function : unsigned char Send_adr(unsigned char adr)							
	Send a SLA+W/R to the bus
****************************************************************************/
unsigned char Send_adr(unsigned char adr)
{
	Wait_TWI_int();							//Wait for TWI interrupt flag set

	TWDR = adr;
	TWCR = ((1<<TWINT)+(1<<TWEN));   		//Clear int flag to send byte 

	Wait_TWI_int();							//Wait for TWI interrupt flag set

	if((TWSR != MTX_ADR_ACK)&&(TWSR != MRX_ADR_ACK))//If NACK received return
											//TWSR
		return TWSR;
	return SUCCESS;							//Else return SUCCESS
}	

/****************************************************************************
	Function : unsigned char Get_byte(unsigned char *rx_ptr,char last_byte)
	Wait for TWINT to receive one byte from the slave and send ACK. If this 
	is the last byte the master will send NACK to tell the slave that it 
	shall stop transmitting.  
****************************************************************************/
unsigned char Get_byte(unsigned char *rx_ptr,char last_byte)
{
	Wait_TWI_int();							//Wait for TWI interrupt flag set

	/*When receiving the last byte from the slave it will be sent a NACK to 
	make the slave stop transmitting, all bits before the last will get 
	a ACK*/
	if(last_byte)							//Not the last byte
		//Clear int flag and enable acknowledge to receive data.
		TWCR = ((1<<TWINT)+(1<<TWEA)+(1<<TWEN));
	else									//Last byte
		/*Clear int flag to and do not enable acknowledge to tell the slave 
		to stop transmitting*/
		TWCR = ((1<<TWINT)+(1<<TWEN)); 			
	Wait_TWI_int();							//Wait for TWI interrupt flag set

	*rx_ptr = TWDR;							//Save received byte

	/*If ACK has been transmitted or this was the last byte and NACK has been
	sent -> return SUCCESS, else return TWSR*/	
 	if(((TWSR == MRX_DATA_NACK)&&(last_byte == 0))||(TWSR == MRX_DATA_ACK))
		return SUCCESS;	  
	return TWSR;
}
