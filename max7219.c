#include <avr/io.h>
#include <util/delay.h>
#include <stdlib.h>

#include "max7219.h"

#include "zkslibi2c.h"

void max7219_clock_out(unsigned char Data)
{
	char i;	
	
	for (i = 0; i < 8; i++)
	 {	
		max7219_PORT &= ~(1 << max7219_CLK); //Clock auf Low
		
		if (Data & (1 << (7 - i)))
		{
			max7219_PORT |= (1 << max7219_DIN);
		}
		else
		{
			max7219_PORT &= ~(1 << max7219_DIN);
		}
		max7219_PORT |= (1 << max7219_CLK); //Clock auf High, Daten werden übernommen	
	}
	return;
}

void max7219_send(unsigned char Data, unsigned char Register)
{
	max7219_PORT &= ~(1 << max7219_LOAD);
	max7219_clock_out(Register); //Adresse senden
	max7219_clock_out(Data); //Daten senden
	max7219_PORT |= (1 << max7219_LOAD);
}

void ClearRegisters()
{
	max7219_PORT &= ~(1 << max7219_LOAD);
	max7219_clock_out(0);
	max7219_clock_out(0);
	max7219_clock_out(0);
	max7219_clock_out(0);
	max7219_clock_out(0);
	max7219_clock_out(0);
	max7219_clock_out(0);
	max7219_clock_out(0);
	max7219_PORT |= (1 << max7219_LOAD);
}

void max7219_sendAllMatrix(unsigned char Data, unsigned char Register, unsigned char Matrix)
{
	Register &= 0x0F; //Obere 4 Bits der Adresse werden nciht gelesen.
	max7219_PORT &= ~(1 << max7219_LOAD); // LOAD auf Low (Chip aktivieren)
	
	max7219_clock_out(Register); //Adresse senden
	max7219_clock_out(Data); //Daten senden
	
	for (char i = 0; i < Matrix; i++)
	{
		max7219_clock_out(Register); //Adresse senden
		max7219_clock_out(0); //Daten senden
	}
	max7219_PORT |= (1 << max7219_LOAD); // LOAD auf High. Steigende Flanke an High übernimmt Daten in Adressiertes Register.
	
	ClearRegisters();
}

typedef struct node 
{
	unsigned char data;
	struct node *next;
}node;

node *row= NULL;
unsigned char matrixes = 1;

node * createLinkedList(unsigned char data)
{	
	node* head = NULL;
	node* temp = NULL;
	
	temp = (node*)malloc(sizeof(node));
	temp->data = data;
	temp->next = NULL;
	
    head = temp;

	return head;
}

node* GetAtIndex(node* list, unsigned char index)
{	
	node* temp = list;
	
	for (unsigned char d = 0; d < index; d++)
	{
		if (temp -> next != NULL)
		{
	       temp = temp -> next;
		}
		else break;	
	}
	return temp;
}

unsigned char GetCount(node* list)
{
	unsigned char i = 1;
	node* temp = list;
	
	while (temp -> next != NULL)
	{
		i++;
		temp = temp -> next;
	}
	return i;
}

void Add(node* list, unsigned char add)
{	
	GetAtIndex(list, GetCount(list) - 1) -> next = createLinkedList(add);
}

void RemoveAt(node* List, unsigned char index)
{	
	node* temp = GetAtIndex(List, index);
	
	GetAtIndex(List, index - 1) -> next = temp -> next;	
}

void Remove(node* List, node* toRemove)
{	
	for (unsigned char i = GetCount(List); i != 0; i--)
	{
		if (toRemove == GetAtIndex(List, i - 1))
		{
			GetAtIndex(List, i - 2) -> next = toRemove -> next;
		}
	}
}

void AddToScrollText(unsigned char Data, unsigned char registerToAdd, unsigned char startMatrix)
{	
	GetAtIndex(row, registerToAdd + startMatrix * 8)->data |= Data;
}


void RemoveFromScrollText(unsigned char Data, unsigned char registerToAdd, unsigned char startMatrix)
{
	GetAtIndex(row, registerToAdd + startMatrix * 8) -> data &= ~Data;
}

void ShowScrollText()
{
	for (unsigned char i = GetCount(row) - 1; i != 0; i--)
	{
		if (i % 8 != 0)
		{
			max7219_sendAllMatrix(GetAtIndex(row, i) -> data, i % 8, i / 8);
		}
		else
		{
			max7219_sendAllMatrix(GetAtIndex(row, i) -> data, 8, i / 9);
		}
	}
}

void ClearScrollText()
{
	for (unsigned char i = GetCount(row) - 1; i != 0; i--)
	{
		GetAtIndex(row, i) -> data = 0;
	}
}

void StartScrolling()
{	
	ShowScrollText();
	
	for (unsigned char i = GetCount(row) - 1; i != 0; i--)
	{
		if (i % 8 != 0)
		{
			if(i / 8 < matrixes)GetAtIndex(row, i + 8) -> data |= GetAtIndex(row, i) -> data >> 7;
		}
		else
		{
			if(i / 9 < matrixes)GetAtIndex(row, i + 8) -> data |= GetAtIndex(row, i) -> data >> 7;
		}
		GetAtIndex(row, i) -> data = GetAtIndex(row, i) -> data << 1;
	}	
}

node* buffer;
void StartScrollingContinuous()
{	
	if (buffer == NULL)
	{
		buffer = createLinkedList(0);
		for (unsigned char i = 0; i <= 7; i++)
		{
			Add(buffer, 0); 
		}
	}

	for (unsigned char i = 1; i <= 8; i++)
	{
		GetAtIndex(buffer, i) -> data = GetAtIndex(row, GetCount(row) - 9 + i)->data >> 7;
	}

	StartScrolling();

	for (unsigned char i = GetCount(buffer) - 8; i <= GetCount(buffer) - 1; i++)
	{
		GetAtIndex(row, i) -> data |= GetAtIndex(buffer, i) -> data;
	}
}

void max7219_set_decodemode(unsigned char Decodemode)
{
	max7219_send(Decodemode, 0x09);
}

void max7219_intensity(unsigned char Intensity)
{
	max7219_send(Intensity, 0x0A);
}

void max7219_set_scanlimit(unsigned char Scanlimit)
{
	max7219_send(Scanlimit, 0x0B);
} 

void max7219_shutdown(void)
{
	max7219_send(0x00, 0x0C);
}
 
void max7219_wakeup(void)
{
	max7219_send(0x01, 0x0C);
} 

void max7219_displaytest_on(void)
{
	max7219_send(0x01, 0x0F);
}
 
void max7219_displaytest_off(void)
{
	max7219_send(0x00, 0x0F);
} 

void max7219_nop(void)
{
	max7219_send(0x00, 0x00);
}

void max7219_clear(void)
{
	for (char i = 1; i < 9; i++)
	{
		max7219_send(0x00, i);
	}
}

void max7219_init(unsigned char matrixAmount)
{
	max7219_DDR |= (1 << max7219_DIN) | (1 << max7219_LOAD) | (1 << max7219_CLK); // Die 3 Steurpins zu Ausgängen machen
	max7219_PORT &= ~( (1 << max7219_DIN) | (1 << max7219_CLK) ); // Clock und DIN auf Low Level
	max7219_PORT |= (1 << max7219_LOAD); //LOAD (CS bei 7221) auf High
	
	for (unsigned char i = 0; i <= matrixAmount - 1; i++)
	{
		max7219_set_decodemode(0); //Keine BCD Wandler. Aller Segmente können individuell angesprochen werden.
		max7219_set_scanlimit(7); //Alle zeichen aktivieren
		max7219_intensity(1);	//Helligkeitsstufe 8 ist ein gutes Mittelmaß
		max7219_clear(); //Display zunächst leeren.
		max7219_wakeup();
	}
	
	matrixes = matrixAmount - 1;
	
	if (row == NULL)
	{
		row = createLinkedList(0);
	
		for(unsigned char i = 0; i <= matrixes; i++)
		{
			for (unsigned char d = 1; d <= 8; d++)
			{
				Add(row, 0);
			}
		}
	}
}

void max7219_hex(unsigned char Number, unsigned char Position)
{
	unsigned char Segments;

	if (Number == 0) //0
	{
		Segments = 0b01111110;
	}
	
	else if (Number == 1) //1
	{
		Segments = 0b00110000;
	}
	 
	else if (Number == 2) //2
	{
		  Segments = 0b01101101;
	}
	 
	else if (Number == 3) //3
	{
		Segments = 0b01111001;
	}
	
	else if (Number == 4) //4
	{
		Segments = 0b00110011;
	}
	
	else if (Number == 5) //5
	{
		Segments = 0b01011011;
	}
	else if (Number == 6) //6
	{
		Segments = 0b01011111;
	}
	
	else if (Number == 7) //7
	{
		Segments = 0b01110000;
	}
	
	else if (Number == 8) //8
	{
		Segments = 0b01111111;
	}
	 
	else if (Number == 9) //9
	{
		Segments = 0b01111011;
	}
	 
	else if (Number == 10) //A
	{
		Segments = 0b01110111;
	}
	
	else if (Number == 11) //B
	{
		Segments = 0b00011111;
	}
	
	else if (Number == 12) //C
	{
		Segments = 0b01001110;
	}
	
	else if (Number == 13) //D
	{
		Segments = 0b00111101;
	}
	
	else if (Number == 14) //E
	{
		Segments = 0b01001111;
	}
	
	else if (Number == 15) //F
	{
		Segments = 0b01000111;
	}
	
	else //Zeichen aus (Leezeichen)
	{
		Segments = 0b00000000;
	}

	max7219_send(Segments, Position);
}

void max7219_disp_uint16(uint16_t Number)
{
	char Digits = 5; //Maximal 5 Stellen (65535)
	
	if (Number < 10)
	{
		Digits = 1;
	}
	
	else if (Number < 100)
	{
		Digits = 2;
	}
	
	else if (Number < 1000)
	{
		Digits = 3;
	}
	
	else if (Number < 10000)
	{
		Digits = 4;
	}
	
	char i;
	
	for ( i= 1; i <= Digits; i++)
	{	
		max7219_hex(Number % 10, i);
		Number /= 10;
	}
	
	for (; i <= 5; i++)  //Führende Nullen nicht anzeigen
	{
		max7219_send(0b00000000, i);
	}
}

void max7219_disp_int16(int16_t Number)
{
	char Signed = 0;
	
	if (Number < 0)
	{
		Number = 0 - Number;
		Signed = 1;
	}

	char Digits = 5; //Maximal 5 Stellen + Vorzeichen (32767...-32768)
	
	if (Number < 10)
	{
		Digits = 1;
	}
	
	else if (Number < 100)
	{
		Digits = 2;
	}
	
	else if (Number < 1000)
	{
		Digits = 3;
	}
	
	else if (Number < 10000)
	
	{
		Digits = 4;
	}
	
	char i;
	
	for (i  =1; i <= Digits; i++)
	{	
		max7219_hex(Number % 10, i);
		Number /= 10;
	}

	if (Signed)  //Bei negativen Zahlen Minuszeichen davor
	{
		max7219_send(0b00000001, i);
		i++;
	}
	
	for (; i <= 6; i++) //Führende Nullen nicht anzeigen
	{
		max7219_send(0b00000000, i);
	}
}

void max7219_disp_uint8(uint8_t Number)
{
	char Digits = 3; //Maximal 3 Stellen (255)
	
	if (Number < 10)
	{
		Digits = 1;
	}
	
	else if (Number < 100)
    {
		Digits = 2;
	}
	
	char i;
	
	for (i = 1; i <= Digits; i++)
	{	
		max7219_hex(Number % 10, i);
		Number /= 10;
	}
	
	for(; i <= 3; i++) //Führende Nullen nicht anzeigen
	{
		max7219_send(0b00000000, i);
	}
}

void max7219_disp_int8(int8_t Number)
{
	char Signed = 0;
	
	if (Number < 0)
	{
		Number = 0-Number;
		Signed = 1;
	}
	
	char Digits = 3;  //Maximal 3 Stellen + Vorzeichen ((127...-128)
	
	if (Number < 10)
	{
		Digits = 1;
	}
	
	else if (Number < 100)
	{
		Digits = 2;
	}
	
	char i;
	
	for (i = 1; i <= Digits; i++)
	{	
		max7219_hex(Number % 10, i);
		Number /= 10;
	}
	
	if(Signed) //Bei negativen Zahlen Minuszeichen davor
	{
		max7219_send(0b00000001, i);
		i++;
	}
	
	for(; i <= 4; i++) //Führende Nullen nicht anzeigen
	{
		max7219_send(0b00000000, i);
	}
}