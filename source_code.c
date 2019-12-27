#include <inttypes.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>
#include <util/delay.h>
#include<stdlib.h>
#include<string.h>

#define lcd_ddr DDRB
#define lcd_prt PORTB
#define lcd_pin PINB
#define lcd_rs 0
#define lcd_rw 1
#define lcd_en 2

unsigned char gpsdata[45], lt[10],lg[10];
unsigned char a,i;

void usart_init()
{
   
UBRRH=00; 
UBRRL=103; 
UCSRB|=(1<<RXEN)|(1<<TXEN); 
UCSRC|=(1<<URSEL)|(1<<UCSZ0)|(1<<UCSZ1);
}

unsigned int usart_getch()
{
 
while ((UCSRA & (1 << RXC)) == 0); // Do nothing until data have been recieved and is ready to be read from UDR
return(UDR); // return the byte
}

void delay_us(int a)
{
   _delay_us(a);
}

void delay_ms(int b)
{
   _delay_ms(b);
}

void lcd_cmd(unsigned char cmd)
{
   lcd_prt = (lcd_prt & 0x0F) | (cmd & 0xF0);
   lcd_prt &= ~(1<<lcd_rs);
   lcd_prt &= ~(1<<lcd_rw);
   lcd_prt |= (1<<lcd_en);
   delay_us(1);
   lcd_prt &= ~(1<<lcd_en);
   delay_us(20);
   lcd_prt = (lcd_prt & 0x0F) | (cmd<<4);
   lcd_prt |= (1<<lcd_en);
   delay_us(1);
   lcd_prt &= ~(1<<lcd_en);
   delay_us(20);
}

void lcd_data(unsigned char dat)
{
   lcd_prt = (lcd_prt & 0x0F) | (dat & 0xF0);
   lcd_prt |= (1<<lcd_rs);
   lcd_prt &= ~(1<<lcd_rw);
   lcd_prt |= (1<<lcd_en);
   delay_us(1);
   lcd_prt &= ~(1<<lcd_en);
   delay_us(20);
   lcd_prt = (lcd_prt & 0x0F) | (dat<<4);
   lcd_prt |= (1<<lcd_en);
   delay_us(1);
   lcd_prt &= ~(1<<lcd_en);
   delay_us(100);
}

void lcd_init()
{
   lcd_ddr = 0xFF;
   lcd_prt &= ~(1<<lcd_en);
   delay_us(2000);
   lcd_cmd(0x33);
   delay_us(100);
   
   lcd_cmd(0x32);
   delay_us(100);
   
   lcd_cmd(0x28);
   delay_us(100);
   
   lcd_cmd(0x0E);
   delay_us(100);
   
   lcd_cmd(0x01);
   delay_us(2000);
   
   lcd_cmd(0x06);
   delay_us(100);
}

void lcd_gotoxy(unsigned char x, unsigned char y)
{
   unsigned char first_ad[] = {0x80,0xC0,0x94,0xD4};
   lcd_cmd(first_ad[y-1] + x-1);
   delay_us(100);
}

void lcd_print(char *str)
{
   unsigned char i = 0;
   while(str[i] != 0)
   {
      lcd_data(str[i]);
      i++;
   }
}
void lcd_printt(int m)
{
   unsigned char i[3]=" ";
   signed char j;
   for(j=0;j<3;j++)
   {
      i[j] = m%10+48;
      m=m/10;
   }
   
   for(j=2;j>=0;j--)
   {
      lcd_data(i[j]);
   }
}

void adc_init()
{
   DDRA |= 0x00;
   ADMUX |= (1<<REFS0) | (1<<REFS1) | (1<<ADLAR) | (1<<MUX0);
   ADCSRA |= (1<<ADEN) | (1<<ADPS0) | (1<<ADPS1) | (1<<ADPS2);
}
void getGPS(){

    while(1){
      a=0;
	 while(a!='$')
	 {
	    a=usart_getch();
	 }
	 i=0;
	 a=usart_getch();
	 i=i+1;
	 a=usart_getch();
	 i=i+1;

	 a=usart_getch();
	 if(a=='R')
	 {
	    a=usart_getch();
	    i=i+1;
	    if(a=='M')
	    {
	       a=usart_getch();
	       i=i+1;
	       if(a=='C')
	       {
		  a=usart_getch();
		  i=i+1;
		  while(i<43)
		  {
		     gpsdata[i]=usart_getch();
		     i++;
		  }
		  }
		  int c;
		  c=0;
		  for(i=17;i<26;i++)
		  {
		     lt[c]=gpsdata[i];
		     c++;
		     //lcd_data(gpsdata[i]);
		  }
		  //lcd_print(" ");
		  //lcd_data(gpsdata[28]);
		  //lcd_cmd(0xc0);
		  //lcd_print(" LG ");
		  c=0;
		  for(i=31;i<40;i++)
		  {
		     lg[c]=gpsdata[i];
		     c++;
		     //lcd_data(gpsdata[i]);
		  }
		  //lcd_print(" ");
		  //lcd_data(gpsdata[41]);
		  delay_ms(1000);
	       }
	    }
		
	if(gpsdata[27]==',')
		{
		   lcd_cmd(0x01);
		   lcd_print("  LT ");
		   lcd_print(lt);
		   lcd_cmd(0xc0);
		   lcd_print(" LG ");
		   lcd_print(lg);
		   delay_ms(5000);
			break;
		}
		
		else
		{
		    lcd_cmd(0x01);
			lcd_print(" LOADING...!");
			delay_ms(1000);
		}
	 }
} 
uint8_t get_value(uint8_t ch)
{
   char temp;
   ch = (ch&0b00000111);
   ADMUX = (ADMUX & 0xE0) | ch;
   ADCSRA |= (1<<ADSC);
   while((ADCSRA&(1<<ADIF))==0);
   ADCSRA |= (1<<ADIF);
   temp = ADCH;
  
   return temp;
}

int main(void)
{
	unsigned char final[]= "Accident Detected at Latitude:";
	unsigned char longitude[] = " Longitude: ";
	unsigned int x,y,z;
	
	unsigned char cmd1[]={"AT"};
	unsigned char cmd2[]={"AT+CMGF=1"};
	unsigned char cmd3[]={"AT+CMGS="};
	unsigned char cmd5[]={"+9779804166134"};


   lcd_init();
   adc_init();
	usart_init();
	lcd_print("Acelerometer");
	_delay_ms(1000);
	lcd_cmd(0x01);
	lcd_cmd(0x80);

	
  do
   {
   /* lcd_gotoxy(1,1);
   lcd_print("line 1       ");
   lcd_gotoxy(1,2);
   lcd_print("line 2      ");
   delay_ms(1000);
   lcd_gotoxy(1,1);
   lcd_print("line 3       ");
   lcd_gotoxy(1,2); 
   lcd_print("line 4       ");
   delay_ms(1000); */
	   
   
    uint8_t x,y,z;
      x = get_value(5);
      _delay_ms(10);
      y = get_value(6);
      _delay_ms(10);
      z = get_value(7);
      _delay_ms(10);
      
     lcd_cmd(0x80);
      lcd_print("x    y    z");
      lcd_cmd(0xC0);
      lcd_printt(x);
	  lcd_print("    ");
	  lcd_printt(y);
	  lcd_print("   ");
	  lcd_printt(z);
	  if((x<=135) || (x>=185) || (y<=135) || (y>=180))
	  {
		  break;
	  }
	  
      
   }while(1);
   

    lcd_cmd(0x01);
   lcd_cmd(0x80);
   lcd_print("Accident Detected");
   _delay_ms(1000);
   
   lcd_cmd(0x01);
   lcd_cmd(0x80);
   lcd_print("Press reset");
   _delay_ms(15000);
   
   lcd_cmd(0x01);
   lcd_cmd(0x80);
	lcd_print("USART");
	lcd_cmd(0xC0);
	lcd_print("Initializing");
   _delay_ms(1000);
	
/*
   do
   {
      value=usart_getch();
	   
      if(value=='$')
      {
			value=usart_getch();
		
			if(value=='G')
			{
				value=usart_getch();
				if(value=='P')
				{
					value=usart_getch();
					if(value=='G')
					{
						value=usart_getch();
						if(value=='G')
						{
							value=usart_getch();
							if(value=='A')
							{
								value=usart_getch();
								if(value==',')
								{
									value=usart_getch();
									for(i=0;value!=',';i++)
									{ 
									value=usart_getch();
									}
		  
      
									lati_value[0]=usart_getch();
									value=lati_value[0];
	  
									for(i=1;value!=',';i++)
									{
									lati_value[i]=usart_getch();
									value=lati_value[i];
									j=1;
									}
	  
									lati_dir=usart_getch();
									value=usart_getch();
	  
									longi_value[0]=usart_getch();
								
									value=longi_value[0];
      
									for(i=1;value!=',';i++)
									{
									longi_value[i]=usart_getch();
									value=longi_value[i];
									}
								
									longi_dir=usart_getch();
								
									lcd_cmd(0x01);
									_delay_ms(1);
									
									
								
									lcd_cmd(0x80);
									_delay_ms(1);
      
									for(i=0;lati_value[i]!='\0';i++)
									{
									lcd_data(lati_value[i]);
									}
									lcd_data(lati_dir);
	  
									lcd_cmd(0xC0);
									_delay_ms(500);
	  
									for(i=0;longi_value[i]!='\0';i++)
									{
									lcd_data(longi_value[i]);
									}
									lcd_data(longi_dir);
									_delay_ms(500);
									
									if(j!=0)
									{
										goto below;
									}
									else
									{
										lcd_cmd(0x01);
										lcd_print(" Wait");
									}
									
								}
							}
						}
					}
				}
			}
		}
	}while(1);

below:

strcat(final,lati_value);
strcat(final,lati_dir);
strcat(final,"Longitude:");
strcat(final,longi_value);
strcat(final,longi_dir);*/

getGPS();


lcd_cmd(0x01);
lcd_gotoxy(1,1);
   lcd_print("GSM");// GSM
      
      for(z=0;cmd1[z]!='\0';z++)
      {
	 UDR = cmd1[z];
	 _delay_ms(100);
      }
      
      UDR = ('\r');
      _delay_ms(500);
      
      for(z=0;cmd2[z]!='\0';z++)
      {
	 UDR = cmd2[z];
	 _delay_ms(100);
      }
      
      UDR = ('\r');
      _delay_ms(500);
      
      for(z=0;cmd3[z]!='\0';z++)
      {
	 UDR = cmd3[z];
	 _delay_ms(100);
      }
      
      UDR = ('"');
      _delay_ms(100);
      
      for(z=0;cmd5[z]!='\0';z++)
      {
	 UDR = cmd5[z];
	 _delay_ms(100);
      }
      
      UDR = ('"');
      _delay_ms(100);
      
      UDR = ('\r');
      _delay_ms(500);
      
      for(z=0;final[z]!='\0';z++)
      {
	 UDR = final[z];
	 _delay_ms(100);
      }
	  
	   for(z=0;lt[z]!='\0';z++)
      {
	 UDR = lt[z];
	 _delay_ms(100);
      }
	 
	  for(z=0;longitude[z]!='\0';z++)
      {
	 UDR = longitude[z];
	 _delay_ms(100);
      }
	  
	    for(z=0;lg[z]!='\0';z++)
      {
		UDR = lg[z];
		_delay_ms(100);
      }
	  
	  
	  
	  
	  
      
      UDR = (26);// ctrlZ-> to send the messge
      _delay_ms(100);
	  lcd_cmd(0x80);
	  lcd_print("Msg sent");

   


   return 0;
}
   
  