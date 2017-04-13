/*
 * GccApplication1.c
 *
 * Created: 17/01/04 10:56:57 AM
 * Author : Bassem El-Sawy
 */ 

#ifndef F_CPU
#define F_CPU 8000000UL // 16MHz clock speed
#endif

#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <avr/eeprom.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
 

/****************Baud Rate******************/

#define BAUD 9600
#define MYUBRR F_CPU/16/BAUD-1



/*************KEYPAD****************/

#define rowA PINC7
#define rowB PINC6
#define rowC PINC5
#define rowD PINC4


#define column1 PINC3
#define column2 PINC2
#define column3 PINC1
#define column4 PINC0


/**************EEPROM**********/

#define read_eeprom_word(address) eeprom_read_word ((const uint16_t*)address)
#define write_eeprom_word(address,value) eeprom_write_word ((uint16_t*)address,(uint16_t)value)
#define update_eeprom_word(address,value) eeprom_update_word ((uint16_t*)address,(uint16_t)value)

/***************************Intializations****************************/

int BufferSize=5;
char ReceivedByte[6];
int i=0,MuxValue, click=0;
int DoneFlag=0 , TimeOut=0 ,totalflag=0,KeypadFlag = 0 , AdminFlag=0 , UpdatePrice=0  ,  ProductNo =0  ,AdminPanelFlag=0;
unsigned char data_t,c;
char arr[5],arr2[2];
int AdminPass[3]={1,2,3} , EntringAdminPass[3], EnteringPrice[10];
unsigned int SendingPrice=0 ,size=1;
unsigned int EEMEM  my_eeprom_array[12];
unsigned int  ProductPrice[12];

/*************************Functions*******************************/

void lcd4_out();
void Lcd4_Port(char a);
void Lcd4_Cmd(char a);
void Lcd4_Clear();
void Lcd4_Set_Cursor(char a, char b);
void Lcd4_Init();
void Lcd4_Write_Char(char a);
void Lcd4_Write_String(char *a);
void Lcd4_Shift_Right();
void Lcd4_Shift_Left();
void keypad();
void keypad_reset();
void AdminPanel();
int ReturnPrice(int [],int );
void split(int number);

/*******************UART*******************/

void USART_Transmit( unsigned char data )
{
	/* Wait for empty transmit buffer */
	while ( !( UCSRA & (1<<UDRE)) )
	_delay_ms(1);
	/* Put data into buffer, sends the data */
	UDR = data;
}
void USART_Init( unsigned int baud )
{
	GIFR=0;
	// Set baud rate
	UBRRH = (unsigned char)(baud>>8);
	UBRRL = (unsigned char)baud;
	// Enable receiver and transmitter and receiver interrupt routine
	UCSRB = (1<<RXEN)|(1<<TXEN)|(1<<RXCIE);
	// Set frame format: 8data, 2stop bit
	UCSRC = (1<<URSEL)|(1<<USBS)|(3<<UCSZ0);
}

/*************************UART INTERRUPT*****************************/
ISR(USART_RXC_vect)
{
	
while ( !(UCSRA & (1<<RXC)) );

if(i==BufferSize){
	i=0;
}

ReceivedByte[i]=UDR;

if (ReceivedByte[i]=='D'){
	DoneFlag=1;
	totalflag=0;
	i=-1;
	
}
else if (ReceivedByte[i]=='T'){
	totalflag=0;
	TimeOut=1;
	i=-1;
}

else{
	totalflag=1;
	
	}

i++;
	
}



int main(void)
{
	DDRB=0xFF;
	DDRA=0xFF;
	DDRD=0xFF;
	

	Lcd4_Init();
	Lcd4_Clear();

	PORTD &= ~(1 << 5);

	/*  -----------------------------
	set keypad rows as outputs
	----------------------------- */
	DDRC = (1<<rowA | 1<<rowB | 1<<rowC | 1<<rowD);
	
/*  --------------------------------------------------------------------------------------
	set keypad columns as inputs and enable the pull-up resistors on the associated port pins
	-------------------------------------------------------------------------------------- */
	DDRC &= ~(1<<column1 | 1<<column2 | 1<<column3 | 1<<column4);
	
	PORTC |= (1<<column1);
	PORTC |= (1<<column2);
	PORTC |= (1<<column3);
	PORTC |= (1<<column4);

	/********************Reading Prices from EEPROM**********************/
	int i;
	for(i=0 ;i<=12;i++){
		
		ProductPrice[i] = read_eeprom_word(&my_eeprom_array[i]); 
	}		
	
	
	sei();
	USART_Init(MYUBRR);	
	
	while (1) 
    {
		
	/***********************Calling Admin Panel*************************/
		keypad();
		if(AdminPanelFlag==1)
		{
		AdminPanel();
		}	
		
		/***Enter Product Number*******/
		keypad();
		Lcd4_Clear();
		Lcd4_Write_String("Welcome");
		_delay_ms(3000);
		Lcd4_Clear();
		Lcd4_Write_String("Select Product");
		KeypadFlag=0;
		click=0;
		i=0;
		
		/****************If not Cancel pressed********************/
		while(KeypadFlag!=15 ){

			keypad();
			
			/**********If Any number pressed*********/
			if (click==1){
				
				Lcd4_Set_Cursor(2,(i));
				Lcd4_Write_Char(c);
				_delay_ms(500);
				
				ProductNo=KeypadFlag;
				
				_delay_ms(500);
				i++;
				click=0;
			}
			/**********If # pressed for clear*********/
			else if(click==2){
				Lcd4_Clear();
				Lcd4_Write_String("Select Product");
				i=0;
				click=0;
			}
			
			/**********If (#&D) pressed for admin panel *********/
			else if(AdminPanelFlag==1){
				click=0;
				KeypadFlag=0;
				main();
				
			}
			/**********If C pressed for cancel*********/
			else if(KeypadFlag==12){
				
				click=0;
				KeypadFlag=0;
				main();
				
			}
			keypad();
		}			
			/***********************Show Price************************/
			keypad();

				
			Lcd4_Clear();
			Lcd4_Write_String("The Price is ");
			
			/**********take prices from EEPROM array*********/
			int a=ProductPrice[ProductNo];
			char buffer[10];
			itoa(a,buffer,10);
			
			/**********Print the Price*********/
			Lcd4_Set_Cursor(2,0);
			Lcd4_Write_String(buffer);
			click=0;
			AdminPanelFlag=0;
			KeypadFlag=0;
			_delay_ms(1000);
			
			/**********Sending Serial to the Payment Module to enable it*********/
			
			USART_Transmit('E');
			
			USART_Transmit('\n');
			
			_delay_ms(500);
			
	
			SendingPrice=ProductPrice[ProductNo];
			
			/**********Split function to put the price in array and return the size of the array*********/
			
			split(SendingPrice);
			
			/**********Sending the price serially to the payment module*********/
			for(i=0;i<size;i++)
			{
				while ( !( UCSRA & (1<<UDRE)) )
				_delay_ms(100);
				UDR=arr[i];
			}


				Lcd4_Clear();
				Lcd4_Write_String("Enter Money");


			while (!TimeOut)
			{
		/**********Receiving the Total price from the payment module and print it*********/
					if(totalflag)
					{
						Lcd4_Set_Cursor(2,0);
						for(i=0;i<BufferSize;i++){
							
							Lcd4_Write_Char(ReceivedByte[i]);
						}
						totalflag=0;
					}
				
				
				keypad();
			/**********if C Pressed for cancel and sending C char to payment module to disable*********/
				if(KeypadFlag==12){
					USART_Transmit('\n');
					USART_Transmit('C');
					USART_Transmit('\n');
					Lcd4_Clear();
					Lcd4_Write_String("Canceled");
					_delay_ms(2000);
					main();
					break;
				}
				
			/**********if product price is received payment module send D from done*********/
				if (DoneFlag)
				{
			/**********Putting the demuxing values depend on the product number*********/
					switch(ProductNo) {
						case 1 :
						MuxValue=0x10;
						break;
						case 2 :
						MuxValue=0x20;
						break;
						case 3 :
						MuxValue=0x30;
						break;
						case 4 :
						MuxValue=0x40;
						break;
						case 5 :
						MuxValue=0x50;
						break;
						case 6 :
						MuxValue=0x60;
						break;
						case 7 :
						MuxValue=0x70;
						break;
						case 8 :
						MuxValue=0x80;
						break;
						case 9 :
						MuxValue=0x90;
						break;
						case 10 :
						MuxValue=0xA0;
						break;
						case 11 :
						MuxValue=0xB0;
						break;
						case 12 :
						MuxValue=0xC0;
						break;
						
					
					}
					
					PORTB |= MuxValue;
					
				
					_delay_ms(500);
					
			/**********Moving motor*********/
					PORTA = 0x01; //00000001
					_delay_ms(800);

					//Stops Motor
					PORTA = 0x00; //00000000
					_delay_ms(1000);

					//Rotates Motor in Clockwise
					PORTA = 0x02; //00000010
					_delay_ms(800);

					//Stops Motor
					PORTA = 0x03; //00000011
					_delay_ms(1000);
					

							Lcd4_Clear();
							Lcd4_Write_String("Thank You !!");
							DoneFlag=0;
							_delay_ms(3000);
							main();
							break;
				}
				
			}
		
				/**********if T is received from payment module it's connection Timeout*********/
			if (TimeOut)
			{
				Lcd4_Clear();
				Lcd4_Write_String("Timeout");
				_delay_ms(2000);
				TimeOut=0;
				main();
				
			}
			

}

return 0;
}
		


void lcd4_out()
{
	PORTB= data_t;
	  
}
void Lcd4_Port(char a)
{
	PORTA |= a<<4;
	PORTA &= (a<<4)+ 0x0f;
	
	lcd4_out();
}
void Lcd4_Cmd(char a)
{
	data_t &= ~0x02;  // => RS = 0
	lcd4_out();
	Lcd4_Port(a);
	data_t |= 0x08; // => E = 1
	lcd4_out();
	_delay_ms(1);
	data_t &= ~0x08;   // => E = 0
	lcd4_out();
	_delay_ms(1);
}
void Lcd4_Clear()
{
	Lcd4_Cmd(0);
	Lcd4_Cmd(1);
}
void Lcd4_Set_Cursor(char a, char b)
{
	char temp,z,y;
	if(a == 1)
	{
		temp = 0x80 + b;
		z = temp>>4;
		y = (0x80+b) & 0x0F;
		Lcd4_Cmd(z);
		Lcd4_Cmd(y);
	}
	else if(a == 2)
	{
		temp = 0xC0 + b;
		z = temp>>4;
		y = (0xC0+b) & 0x0F;
		Lcd4_Cmd(z);
		Lcd4_Cmd(y);
	}
}
void Lcd4_Init()
{
	Lcd4_Port(0x00);
	_delay_ms(20);
	///////////// Reset process from datasheet /////////
	Lcd4_Cmd(0x03);
	_delay_ms(5);
	Lcd4_Cmd(0x03);
	_delay_ms(11);
	Lcd4_Cmd(0x03);
	/////////////////////////////////////////////////////
	Lcd4_Cmd(0x02);
	Lcd4_Cmd(0x02);
	Lcd4_Cmd(0x08);
	Lcd4_Cmd(0x00);
	Lcd4_Cmd(0x0C);
	Lcd4_Cmd(0x00);
	Lcd4_Cmd(0x06);
}
void Lcd4_Write_Char(char a)
{
	char temp,y;
	temp = a&0x0F;
	y = a&0xF0;
	data_t |= 0x02;   // => RS = 1
	lcd4_out();
	Lcd4_Port(y>>4);             //Data transfer
	data_t |= 0x08; // en=1
	lcd4_out();
	_delay_ms(1);
	data_t &= ~0x08; //en =0
	lcd4_out();
	_delay_ms(1);
	Lcd4_Port(temp);
	data_t |= 0x08; //en =1
	lcd4_out();
	_delay_ms(1);
	data_t &= ~0x08; //en =0
	lcd4_out();
	_delay_ms(1);
}
void Lcd4_Write_String(char *a)
{
	int i;
	for(i=0;a[i]!='\0';i++)
	Lcd4_Write_Char(a[i]);
}
void Lcd4_Shift_Right()
{
	Lcd4_Cmd(0x01);
	Lcd4_Cmd(0x0C);
}
void Lcd4_Shift_Left()
{
	Lcd4_Cmd(0x01);
	Lcd4_Cmd(0x08);
}

void keypad_reset()
{
	PORTC |= 0b11110000;
	
}	
void keypad()
{

	keypad_reset();
	PORTC &= ~(1<<rowA); // Make rowB high
	if( (PINC & (1<<column1 |1<< column2)) == 0x00 );

	else if( (PINC & (1<<column1)) == 0x00 ) //1
	{
		KeypadFlag=1;
		click=1;
		c='1';
	
		_delay_ms(50);

	}
	else if( (PINC & (1<<column2)) == 0x00)//2
	{
		KeypadFlag=2;
		click=1;
		c='2';

		_delay_ms(50);

	}
	else if((PINC & (1<<column3)) == 0x00)//3
	{
		KeypadFlag=3;
		click=1;
		c='3';

		_delay_ms(50);

	}
	else if( (PINC & (1<<column4)) == 0x00)//A
	{
		KeypadFlag=10;
		click=1;
		c='A';

		_delay_ms(50);
	}


	keypad_reset();
	PORTC &= ~(1<<rowB); // Make rowB high

	if( (PINC & (1<<column1 |1<< column2)) == 0x00 );

	else if( (PINC & (1<<column1)) == 0x00 ) //4
	{
		KeypadFlag=4;
		click=1;
		c='4';

		_delay_ms(50);
	}
	else if( (PINC & (1<<column2)) == 0x00)//5
	{
		KeypadFlag=5;
		click=1;
		c='5';
		_delay_ms(50);
	}
	else if((PINC & (1<<column3)) == 0x00)//6
	{
		KeypadFlag=6;
		click=1;

		c='6';

		_delay_ms(50);
	}
	else if( (PINC & (1<<column4)) == 0x00)//B
	{
		KeypadFlag=11;

		click=1;
		c='B';
		_delay_ms(50);
	}


	keypad_reset();
	PORTC &= ~(1<<rowC); // Make rowC high
	if( (PINC & (1<<column1 |1<< column2)) == 0x00 );

	else if( (PINC & (1<<column1)) == 0x00 ) //7
	{
		KeypadFlag=7;
		click=1;

		c='7';

		_delay_ms(50);
	}
	else if( (PINC & (1<<column2)) == 0x00)//8
	{
		KeypadFlag=8;
		click=1;

		c='8';

		_delay_ms(50);
	}
	else if((PINC & (1<<column3)) == 0x00)//9
	{
		KeypadFlag=9;
		click=1;

		c='9';

		_delay_ms(50);
	}
	else if( (PINC & (1<<column4)) == 0x00)//C
	{
		KeypadFlag=12;
		
		_delay_ms(50);
	}


	// continue editing
	keypad_reset();
	PORTC &= ~(1<<rowD); // Make rowD high
	
	

	if( (PINC & (1<<column1 |1<< column2)) == 0x00 );

	else if(( (PINC & (1<<column1)) == 0x00 ) && ( (PINC & (1<<column4)) == 0x00)) //* and D
	{
		AdminPanelFlag=1;
		_delay_ms(50);
	}
	
	
	else if( (PINC & (1<<column1)) == 0x00 ) //*
	{
		//KeypadFlag=14;
		//click=1;


		_delay_ms(50);
	}
	else if( (PINC & (1<<column2)) == 0x00)//0
	{
		KeypadFlag=0;
		click=1;
		c='0';

		_delay_ms(50);
	}
	else if((PINC & (1<<column3)) == 0x00)//#
	{
		KeypadFlag=16;
		click=2;

		c='#';

		_delay_ms(50);
	}
	else if( (PINC & (1<<column4)) == 0x00)//D
	{
		KeypadFlag=15;
		//click=1;
		//c='D';
		_delay_ms(50);
	}




}
void AdminPanel()
{
	int flag=0;
	int EnterFlag=0;
	Lcd4_Write_String("Enter Admin Password");

	int i=0;
	while(i<3 ){
		
		keypad();
		
				/**********entring admin pass to the lcd and to EntringAdminPass array*********/
		if (click==1){
			
			Lcd4_Set_Cursor(2,(i));
			Lcd4_Write_Char(c);
			_delay_ms(500);
				
		EntringAdminPass[i]=KeypadFlag;
		
		_delay_ms(500);
		i++;
		click=0;
		}
		else if(click==2){
			Lcd4_Clear();
			Lcd4_Write_String("Enter Admin Password");
			i=0;
			click=0;
		}					
	}
	
			/**********Checking if the password is correct or not*********/
	for (int i=0;i<3;i++)
	{
		//keypad();
		if (EntringAdminPass[i] == AdminPass[i])
		{
			EnterFlag++;
		}
		
	}

	if (EnterFlag==3)
	{
		Lcd4_Clear();
		Lcd4_Write_String("Right Password");
		_delay_ms(500);
		Lcd4_Clear();
		flag=1;

	}
	else
	{
		Lcd4_Clear();
		Lcd4_Write_String("Please try again");
		_delay_ms(500);
		Lcd4_Clear();
		/*******Please try again *******/
		AdminPanel();
	}

	while (flag==1)
	{
				/**********if C pressed Cancel*********/
		if(KeypadFlag==12){
			AdminPanelFlag=0;
			flag=0;
			main();
			break;
		}
		Lcd4_Write_String("A.Select Product");
		
		i=0;
		/********when D is pressed then it's done ***********/
		while(KeypadFlag!=15 ){

			keypad();
				if(KeypadFlag==12){
					AdminPanelFlag=0;
					flag=0;
					main();
					break;
				}
			/*********selecting product number**********/
			if (click==1){
				
				Lcd4_Set_Cursor(2,(i));
				Lcd4_Write_Char(c);
				_delay_ms(500);
				
				ProductNo=KeypadFlag;
				
				_delay_ms(500);
				i++;
				click=0;
			}
			else if(click==2){
					Lcd4_Clear();
					Lcd4_Write_String("A.Select Product");
					i=0;
					click=0;
				}
			
			keypad();
				if(KeypadFlag==12){
					AdminPanelFlag=0;
					flag=0;
					main();
					break;
				}
		}
		Lcd4_Clear();
		Lcd4_Write_String("Product No. ");
		Lcd4_Write_Char(c);
		_delay_ms(2000);
		Lcd4_Clear();
		Lcd4_Clear();
		Lcd4_Write_String("Enter Price");
	
		i=0;
		KeypadFlag=0;
		click=0;
		while(KeypadFlag!=15 ){

			keypad();
				if(KeypadFlag==12){
					AdminPanelFlag=0;
					flag=0;
					main();
					break;
				}
			
			/**********admin enter the updated price*********/
			if (click==1){
				Lcd4_Set_Cursor(2,(i));
				Lcd4_Write_Char(c);
				_delay_ms(500);
				
				EnteringPrice[i]=KeypadFlag;
				
				
				
				
				_delay_ms(500);
				i++;
				click=0;
			}
			else if(click==2){
				Lcd4_Clear();
				Lcd4_Write_String("Enter Price");
					i=0;
					click=0;	
				}
			keypad();
				if(KeypadFlag==12){
					AdminPanelFlag=0;
					flag=0;
					main();
					break;
				}		
	}
	Lcd4_Clear();
			/*********update the price in the EEPROM **********/
		UpdatePrice=ReturnPrice(EnteringPrice,i);
	update_eeprom_word(&my_eeprom_array[ProductNo], UpdatePrice);
	PORTD |= (1 << 5);
	_delay_ms(1000);
	PORTD &= ~(1 << 5);
	Lcd4_Write_String("DONE !!!! ");
	_delay_ms(2000);
	
	KeypadFlag=0;
	click=0;

	
	keypad();
	Lcd4_Clear();
	if(KeypadFlag==12){
		AdminPanelFlag=0;
		flag=0;
		main();
		break;
	}
	
	}
		
	
	}
	


int ReturnPrice(int array[],int y)
{
	int g=0,k=0;
	int dig=1;

	for(g=(y-1);g>=0;g--)
	{
		k =k+ ( array[g] * dig);
		dig=dig*10;
	}

	return k;
}


void split(int number)
{
			size=1;
	 int counter=number;
	 float num=number;
	 while (counter/=10) size++;

		num=(num/100);
	
	itoa(num, arr,10);
	strcat(arr, ".");                   //append decimal point
	uint16_t i = (num -(int)num) * 100;      //subtract to get the decimals, and multiply by 1000
	itoa(i, arr2, 10);                  //convert to a second string
	strcat(arr, arr2);               //and append to the first

	
	size=size+1;
}
