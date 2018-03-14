/******************************************************************************
* @file			main.c
* @brief		Project Sonceboz_Motion_Control
* @author		BFH-TI / riedm1
* @version	2.0
* @date			29.05.2017
* @hardware NucleoF446RE
*******************************************************************************/

//TEST

/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx.h"
#include <stdio.h>									// Includes Standard Library für C
#include "NucleoF446_LCD.h"					// Includes Library für LCD File

extern volatile int msTicks;				// Externe Variablen aus Interrupt File definieren
extern volatile int flag_10ms;			// Externe Variablen aus Interrupt File definieren
int msTicksHold = 0;								// Variable für Delay Funktion
int k = 0;													// Variable für Array - Initialisierung
volatile int p = 0;									// Variable für Array - Abfrage
int commands = 0;										// Variable für Array - Daten Beschriftung
volatile int loop = 0;							// Variable für Schlaufe zur Positionsinitialisierung
int status = 0;											// Variable für Keypad Werte auslesen
int arrayLeft[11];									// Array für Motorpositionen links
int arrayRight[11];									// Array für Motorpositionen rechts
int drive = 0;											// Variable für Streckenbestätigung
int write = 0;											// Variable für Positionsbestätigung 
int direction = 0;									// Variable für Richtungsangabe
int distance = 0;										// Variable für Distanzangabe
int valMotorL = 0;									// Variable für Positionsberechnung linker Motor (Funktion Position_Values_left)
int valMotorR = 0;									// Variable für Positionsberechnung rechter Motor (Funktion Position_Values_rechter)
int positionMotorL = 2105540607;		// Variable für Positionsspeicherung Motor links
int positionMotorR = 2105540607;		// Variable für Positionsspeicherung Motor rechts
int ADC_Value = 0;									// Variable für Speicherung des ADC Wertes von Poti1
int i = 0;													// Variable für for-Schlaufe zur Mittelung des ADC Wertes
int sum = 0;												// Variable für for-Schlaufe zur Mittelung des ADC Wertes
int counter = 0;										// Variable für for-Schlaufe zur Mittelung des ADC Wertes
int meanValue = 0;									// Variable für Speicherung des gemittelten ADC Wertes
int speedValue = 0;									// Variable für Speicherung des berechneten Geschwindigkeitswert
int finalValue = 0;									// Variable für Speicherung des Finalen Wertes für CAN Nachricht
int ADC_Value2 = 0;									// Variable für Speicherung des ADC Wertes von Poti1
int j = 0;													// Variable für for-Schlaufe zur Mittelung des ADC Wertes
int sum2 = 0;												// Variable für for-Schlaufe zur Mittelung des ADC Wertes
int counter2 = 0;										// Variable für for-Schlaufe zur Mittelung des ADC Wertes
int meanValue2 = 0;									// Variable für Speicherung des gemittelten ADC Wertes
int speedValue2 = 0;								// Variable für Speicherung des berechneten Geschwindigkeitswert
int finalValue2 = 0;								// Variable für Speicherung des Finalen Wertes für CAN Nachricht
char text[32];											// Variable für Display Text
/* Private variables ---------------------------------------------------------*/
CanTxMsg TxMessage;									// Variable für CAN Nachricht senden definieren

/* Private function prototypes -----------------------------------------------*/
void RCC_Configuration(void);				// Funktions-Prototyp für System Clock Configuration
void GPIO_Configuration(void);			// Funktions-Prototyp für GPIO Configuration
void CAN_Config(void);							// Funktions-Prototyp für CAN Configuration
void Delay(unsigned long);					// Funktions-Prototyp für Delay
int Position_Values_left(void);			// Funktions-Prototyp für Position_Values_left
int Position_Values_right(void);		// Funktions-Prototyp für Position_Values_right
void ADC_Configuration(void);				// Funktions-Prototyp für ADC Configuration
float calculateValue(void);					// Funktions-Prototyp für calculateValue (links)
float calculateValue2(void);				// Funktions-Prototyp für calculateValue (rechts)
/* Private functions ---------------------------------------------------------*/

/**
 * @name		main
 * @brief		Main program
 *
 */
int main(void)																	// Start der main Funktion (Initialisierungsteil wird nur einmal ausgeführt)
{	
  /* System Clocks Configuration */
  RCC_Configuration();													// Aufrufen der Funktion für die Konfiguration der System Clocks
      
  /* Configure the GPIO ports */
 	GPIO_Configuration();													// Aufrufen der Funktion für die Konfiguration der GPIOs
	
	/* LCD Initialisieren */
	NUC_LCD_Init();																// Aufrufen der Funktion für die Konfiguration des LCDs
	
	/* System Timer Configurieren */
	SysTick_Config(SystemCoreClock / 1000);				// Aufrufen der Funktion für die Konfiguration der System Timers und Einstellung auf 1ms
	
	/* CAN Configurieren */
	CAN_Config();																	// Aufrufen der Funktion für die Konfiguration der CAN Kommunikation
	
	/* Configure ADC */
	ADC_Configuration();													// Aufrufen der Funktion für die Konfiguration des ADC Wandlers
	
	/* Start ADC */
	ADC_SoftwareStartConv(ADC1);									// Aufrufen der Funktion für den Start des ADC1 Wandlers
	
	/* Start ADC2 Software Conversion */ 
  ADC_SoftwareStartConv(ADC2);									// Aufrufen der Funktion für den Start des ADC2 Wandlers
	

	if(GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_13) == 1)						// Überprüfen ob Autonomer Modus aktiviert ist
	{
	for(k=0;k<11;k++)																							// Array mit 10 Zellen erstellen
		{
			arrayLeft[k] = 2105540607;																// Array für linken Motor mit Wert für Nullposition beschreiben
			arrayRight[k] = 2105540607;																// Array für rechts Motor mit Wert für Nullposition beschreiben
		}
	
while( drive != 1){																							// Solange die Variable Drive nicht = 1 ist, Schlaufe für Keypad druchführen
		
		GPIO_SetBits(GPIOC, GPIO_Pin_2);														// Kolonne Nr.1 auf "HIGH" setzen

		
		if(GPIO_ReadInputDataBit(GPIOC, GPIO_Pin_10) == 1)					// Lesen ob Reihe Nr.1 "HIGH" ist
		{
			status = 1;																																									
		}
		else if(GPIO_ReadInputDataBit(GPIOC, GPIO_Pin_11) == 1)			// Lesen ob Reihe Nr.2 "HIGH" ist
		{
			status = 4;
		}
		else if(GPIO_ReadInputDataBit(GPIOC, GPIO_Pin_12) == 1)			// Lesen ob Reihe Nr.3 "HIGH" ist
		{
			status = 7;
		}
		else if(GPIO_ReadInputDataBit(GPIOD, GPIO_Pin_2) == 1)			// Lesen ob Reihe Nr.4 "HIGH" ist
		{
			status = 11;
		}
		
		
			GPIO_ResetBits(GPIOC, GPIO_Pin_2);												// Kolonne Nr.1 wieder auf "LOW"
			GPIO_SetBits(GPIOC, GPIO_Pin_3);													// Kolonne Nr.2 auf "HIGH" setzen
		
		
		
		if(GPIO_ReadInputDataBit(GPIOC, GPIO_Pin_10) == 1)					// Lesen ob Reihe Nr.1 "HIGH" ist
		{
			status = 2;
		}
		else if(GPIO_ReadInputDataBit(GPIOC, GPIO_Pin_11) == 1)			// Lesen ob Reihe Nr.2 "HIGH" ist
		{
			status = 5;
		}
		else if(GPIO_ReadInputDataBit(GPIOC, GPIO_Pin_12) == 1)			// Lesen ob Reihe Nr.3 "HIGH" ist
		{
			status = 8;
		}
		else if(GPIO_ReadInputDataBit(GPIOD, GPIO_Pin_2) == 1)			// Lesen ob Reihe Nr.4 "HIGH" ist
		{
			status = 10;
		}
		
		
			GPIO_ResetBits(GPIOC, GPIO_Pin_3);												// Kolonne Nr.2 wieder auf "LOW"
			GPIO_SetBits(GPIOB, GPIO_Pin_14);													// Kolonne Nr.3 auf "HIGH" setzen
		
		
		
		if(GPIO_ReadInputDataBit(GPIOC, GPIO_Pin_10) == 1)					// Lesen ob Reihe Nr.1 "HIGH" ist
		{
			status = 3;
		}
		else if(GPIO_ReadInputDataBit(GPIOC, GPIO_Pin_11) == 1)			// Lesen ob Reihe Nr.2 "HIGH" ist
		{
			status = 6;
		}
		else if(GPIO_ReadInputDataBit(GPIOC, GPIO_Pin_12) == 1)			// Lesen ob Reihe Nr.3 "HIGH" ist
		{
			status = 9;
		}
		else if(GPIO_ReadInputDataBit(GPIOD, GPIO_Pin_2) == 1)			// Lesen ob Reihe Nr.4 "HIGH" ist
		{
			status = 12;
		}
		
		
			GPIO_ResetBits(GPIOB, GPIO_Pin_14);												// Kolonne Nr.3 wieder auf "LOW"
			GPIO_SetBits(GPIOB, GPIO_Pin_15);													// Kolonne Nr.4 auf "HIGH" setzen
		
		
		
		if(GPIO_ReadInputDataBit(GPIOC, GPIO_Pin_10) == 1)					// Lesen ob Reihe Nr.1 "HIGH" ist
		{
			status = 16;
		}
		else if(GPIO_ReadInputDataBit(GPIOC, GPIO_Pin_11) == 1)			// Lesen ob Reihe Nr.2 "HIGH" ist
		{
			status = 15;
		}
		else if(GPIO_ReadInputDataBit(GPIOC, GPIO_Pin_12) == 1)			// Lesen ob Reihe Nr.3 "HIGH" ist
		{
			status = 14;
		}
		else if(GPIO_ReadInputDataBit(GPIOD, GPIO_Pin_2) == 1)			// Lesen ob Reihe Nr.4 "HIGH" ist
		{
			status = 13;
		}
		
		
		GPIO_ResetBits(GPIOB, GPIO_Pin_15);													// Kolonne Nr.4 wieder auf "LOW"
		
		Delay(150);																									// Delay von 150ms einfügen zur einfacheren eingabe von einem Wert
		
		
		if (status >= 13)																						// Überprüfen ob "status" einen Wert höher als 13 hat, was bedeutet es handelt sich um eine Richtungsangabe
		{
			direction = status;																				// Wert von "status" in "direction" speichern
		}
		
		
		if (status < 10)																						// Überprüfen ob "status" kleiner als 10 ist, somit handelt es sich um eine Distanzangabe
		{ if (status != 0)																					// Überprüfen ob "status" nicht gleich null ist
			{
				distance = status;																			// Wert von "status" in "distance" speichern
			}
		}
		
		if (status == 11)																						// Überprüfen ob "status" gleich 11 ist
		{
			write = 1;																								// Variable "write" auf 1 setzen
		}
		
		if (status == 12)																						// Überprüfen ob "status" gleich 12 ist
		{
			drive = 1;																								// Variable "drive" auf 1 setzen
		}
		
		if (write == 1)																							// Überprüfen ob "write" gleich 1 ist
		{ if ( commands < 10)																				// Überprüfen ob "commands" kleiner als 10 ist
			{
				positionMotorL = Position_Values_left();								// Berechneter Wert von Funktion "Position_Values_left" in "positionMotorL" speichern
				positionMotorR = Position_Values_right();								// Berechneter Wert von Funktion "Position_Values_right" in "positionMotorR" speichern
				
				arrayLeft[commands] = positionMotorL;										// Wert von "positionMotorL" in aktuelles Array Feld speichern
				arrayRight[commands] = positionMotorR;									// Wert von "positionMotorL" in aktuelles Array Feld speichern
				commands = commands + 1;																// Variable "commands" um eins erhöhen, um in die nächste Spalte des Arrays zu gelangen
				
				direction = 0;																					// "direction" gleich null setzen
				distance = 0;																						// "distance" gleich null setzen
				write = 0;																							// "write" gleich null setzen
			}
		}
		
									NUC_LCD_Clear();
									sprintf(text, "Commands: = %d", commands);			// Anzeigen der Variable text und Definition deren Inhalt
									NUC_LCD_SetString (0,20,text,8);							// Definition der Parameter Postion und Schriftgrösse der Variable text
		
									sprintf(text, "Dist: = %d", distance);				// Anzeigen der Variable text und Definition deren Inhalt
									NUC_LCD_SetString (0,0,text,8);								// Definition der Parameter Postion und Schriftgrösse der Variable text
		
									sprintf(text, "Direc: = %d", direction);			// Anzeigen der Variable text und Definition deren Inhalt
									NUC_LCD_SetString (0,10,text,8);							// Definition der Parameter Postion und Schriftgrösse der Variable text
		
									NUC_LCD_Show();																// Daten an LCD senden
									
									status = 0;																		// "status" gleich null setzen
		
		
		
		
									
									
            }
					}
	else
	{
		sprintf(text, "Transmiter Mode ON");												// Anzeigen der Variable text und Definition deren Inhalt
		NUC_LCD_SetString (0,0,text,8);															// Definition der Parameter Postion und Schriftgrösse der Variable text

		NUC_LCD_SetString(0,24,"meanvalue = %d", meanValue);
		NUC_LCD_Show();																							// Daten an LCD senden
	}
	
		
	/* Endless loop */
	while(1){																														// Start der main Schlaufe (endlose Wiederholung)
		
		finalValue = calculateValue();																		// Wert aus Funktion calculateValue in Variable finalValue speichern
		finalValue2 = calculateValue2();																	// Wert aus Funktion calculateValue2 in Variable finalValue2 speichern
		
		if(flag_10ms) {																										// überprüfen ob Variable flag_10ms = "1" dementsprechend Funktion ausführen																						
			flag_10ms = 0;																									// Variable flag_10ms wieder auf "0" setzen
			
			if(GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_13) == 1)							// Überprüfen ob Autonomer Modus aktiviert ist
	{
			if(loop < 10)																										// Überprüfen ob "loop" kleiner als 10 ist
	{
			/* Transmit Structure preparation */
				TxMessage.IDE = CAN_ID_EXT;																		// Auswahl der Adressart für CAN-Übertragung -> Extended Address
				TxMessage.ExtId = 0x18FF3330;																	// Definiere Adresse mit dem Wert 0x18FF3330 für Motoransteuerung "MotorControl1"
				TxMessage.RTR = CAN_RTR_DATA;																	// Definiere Art der Datenübertragung
				TxMessage.DLC = 8;																						// Defniere Anzahl der Datenbytes -> 8
			
			/* aktuelle Motorposition als Nullposition setzen für Motor links */
			
				TxMessage.Data[0] = 0xFF;																			// der Wert 0xFF wird in Datenbyte "0" gespeichert
				TxMessage.Data[1] = 0xFF;																			// der Wert 0xFF wird in Datenbyte "1" gespeichert
				TxMessage.Data[2] = 0xFF;																			// der Wert 0xFF wird in Datenbyte "2" gespeichert
				TxMessage.Data[3] = 0xFF;																			// der Wert 0xFF wird in Datenbyte "3" gespeichert
				TxMessage.Data[4] = 0xFE;																			// der Wert 0xFE wird in Datenbyte "4" gespeichert
				TxMessage.Data[5] = 0xFF;																			// der Wert 0xFF wird in Datenbyte "5" gespeichert
				TxMessage.Data[6] = 0xFF;																			// der Wert 0xFF wird in Datenbyte "6" gespeichert
				TxMessage.Data[7] = 0xFF;																			// der Wert 0xFF wird in Datenbyte "7" gespeichert
				
				/* Daten senden */
//				CAN_Transmit(CAN1, &TxMessage);																// Die Daten welche in der Variable TxMessage definiert wurden werden auf CAN1 gesendet
		
		/* Transmit Structure preparation */
				TxMessage.IDE = CAN_ID_EXT;																		// Auswahl der Adressart für CAN-Übertragung -> Extended Address
				TxMessage.ExtId = 0x18FF3331;																	// Definiere Adresse mit dem Wert 0x18FF3331 für Motoransteuerung "MotorControl1"
				TxMessage.RTR = CAN_RTR_DATA;																	// Definiere Art der Datenübertragung
				TxMessage.DLC = 8;																						// Defniere Anzahl der Datenbytes -> 8
			
			/* aktuelle Motorposition als Nullposition setzen für Motor rechts */
			
				TxMessage.Data[0] = 0xFF;																			// der Wert 0xFF wird in Datenbyte "0" gespeichert
				TxMessage.Data[1] = 0xFF;																			// der Wert 0xFF wird in Datenbyte "1" gespeichert
				TxMessage.Data[2] = 0xFF;																			// der Wert 0xFF wird in Datenbyte "2" gespeichert
				TxMessage.Data[3] = 0xFF;																			// der Wert 0xFF wird in Datenbyte "3" gespeichert
				TxMessage.Data[4] = 0xFE;																			// der Wert 0xFE wird in Datenbyte "4" gespeichert
				TxMessage.Data[5] = 0xFF;																			// der Wert 0xFF wird in Datenbyte "5" gespeichert
				TxMessage.Data[6] = 0xFF;																			// der Wert 0xFF wird in Datenbyte "6" gespeichert
				TxMessage.Data[7] = 0xFF;																			// der Wert 0xFF wird in Datenbyte "7" gespeichert
				
				/* Daten senden */
//				CAN_Transmit(CAN1, &TxMessage);																// Die Daten welche in der Variable TxMessage definiert wurden werden auf CAN1 gesendet
	}
	
				if (loop > 10)																								// Überprüfen ob "loop" grösser als 10 ist
				{
					
				
				
			/* Transmit Structure preparation */
				TxMessage.IDE = CAN_ID_EXT;																		// Auswahl der Adressart für CAN-Übertragung -> Extended Address
				TxMessage.ExtId = 0x18FF3330;																	// Definiere Adresse mit dem Wert 0x18FF3330 für Motoransteuerung "MotorControl1"
				TxMessage.RTR = CAN_RTR_DATA;																	// Definiere Art der Datenübertragung
				TxMessage.DLC = 8;																						// Defniere Anzahl der Datenbytes -> 8
			
			/* gewünschte Motorposition aus Array in Variable "TxMessage" speichern für Motor links */
			
				TxMessage.Data[0] = 0xFF;																			// der Wert 0xFF wird in Datenbyte "0" gespeichert
				TxMessage.Data[1] = 0xFF;																			// der Wert 0xFF wird in Datenbyte "1" gespeichert
				TxMessage.Data[2] = 0xFF;																			// der Wert 0xFF wird in Datenbyte "2" gespeichert
				TxMessage.Data[3] = 0xFF;																			// der Wert 0xFF wird in Datenbyte "3" gespeichert
				TxMessage.Data[4] = arrayLeft[p];															// die acht least significant Bits werden aus dem aktuellen Array Feld in Datenbyte "4" gespeichert
				TxMessage.Data[5] = arrayLeft[p] >> 8;												// die Bits werden um 8 Stellen nach rechts verschoben und die neuen least significant Bits werden in Datenbyte "5" gespeichert
				TxMessage.Data[6] = arrayLeft[p] >> 16;												// die Bits werden um 16 Stellen nach rechts verschoben und die neuen least significant Bits werden in Datenbyte "6" gespeichert
				TxMessage.Data[7] = arrayLeft[p] >> 24;												// die Bits werden um 24 Stellen nach rechts verschoben und die neuen least significant Bits werden in Datenbyte "7" gespeichert
			
			
			
			/* Daten senden */
//				CAN_Transmit(CAN1, &TxMessage);																// Die Daten welche in der Variable TxMessage definiert wurden werden auf CAN1 gesendet
			
				
					/* Transmit Structure preparation */
				TxMessage.IDE = CAN_ID_EXT;																		// Auswahl der Adressart für CAN-Übertragung -> Extended Address
				TxMessage.ExtId = 0x18FF3331;																	// Definiere Adresse mit dem Wert 0x18FF3331 für Motoransteuerung "MotorControl1"
				TxMessage.RTR = CAN_RTR_DATA;																	// Definiere Art der Datenübertragung
				TxMessage.DLC = 8;																						// Defniere Anzahl der Datenbytes -> 8
			
			/* gewünschte Motorposition aus Array in Variable "TxMessage" speichern für Motor rechts */
			
				TxMessage.Data[0] = 0xFF;																			// der Wert 0xFF wird in Datenbyte "0" gespeichert
				TxMessage.Data[1] = 0xFF;																			// der Wert 0xFF wird in Datenbyte "1" gespeichert
				TxMessage.Data[2] = 0xFF;																			// der Wert 0xFF wird in Datenbyte "2" gespeichert
				TxMessage.Data[3] = 0xFF;																			// der Wert 0xFF wird in Datenbyte "3" gespeichert
				TxMessage.Data[4] = arrayRight[p];														// die acht least significant Bits werden aus dem aktuellen Array Feld in Datenbyte "4" gespeichert
				TxMessage.Data[5] = arrayRight[p] >> 8;												// die Bits werden um 8 Stellen nach rechts verschoben und die neuen least significant Bits werden in Datenbyte "5" gespeichert
				TxMessage.Data[6] = arrayRight[p] >> 16;											// die Bits werden um 16 Stellen nach rechts verschoben und die neuen least significant Bits werden in Datenbyte "6" gespeichert
				TxMessage.Data[7] = arrayRight[p] >> 24;											// die Bits werden um 24 Stellen nach rechts verschoben und die neuen least significant Bits werden in Datenbyte "7" gespeichert
			
			
			
			/* Daten senden */
//				CAN_Transmit(CAN1, &TxMessage);																// Die Daten welche in der Variable TxMessage definiert wurden werden auf CAN1 gesendet
				
			}
				loop++;																												// "loop" um einen Schritt erhöhen
		
			NUC_LCD_Clear();																								// angezeigte Werte auf Display löschen	
		sprintf(text, "DRIVE");																						// Anzeigen der Variable text und Definition deren Inhalt
		NUC_LCD_SetString (0,0,text,24);																	// Position und Grösse von "text" definieren
		
		NUC_LCD_Show();																										// definierte Werte anzeigen
			
			
		
		}
	else
	{
			/* Transmit Structure preparation */
				TxMessage.IDE = CAN_ID_EXT;																		// Auswahl der Adressart für CAN-Übertragung -> Extended Address
				TxMessage.ExtId = 0x18FF3330;																	// Definiere Adresse mit dem Wert 0x18FF3330 für Motoransteuerung "MotorControl1"
				TxMessage.RTR = CAN_RTR_DATA;																	// Definiere Art der Datenübertragung
				TxMessage.DLC = 8;																						// Defniere Anzahl der Datenbytes -> 8
			
			/* Variable finalValue in Datenbyte 0 und 1 schreiben */
			
				TxMessage.Data[0] = finalValue;																// die 8 least significant Bits der Variable finalValue in Datenbyte "0" der CAN Nachricht speichern
				TxMessage.Data[1] = finalValue >> 8;													// die Bits in der Variable finalValue werden um 8 Stellen nach rechts verschoben und die neuen least significant Bits werden in Datenbyte "1" gespeichert
				TxMessage.Data[2] = 0xFF;																			// der Wert 0xFF wird in Datenbyte "2" gespeichert
				TxMessage.Data[3] = 0xFF;																			// der Wert 0xFF wird in Datenbyte "3" gespeichert
				TxMessage.Data[4] = 0xFF;																			// der Wert 0xFF wird in Datenbyte "4" gespeichert
				TxMessage.Data[5] = 0xFF;																			// der Wert 0xFF wird in Datenbyte "5" gespeichert
				TxMessage.Data[6] = 0xFF;																			// der Wert 0xFF wird in Datenbyte "6" gespeichert
				TxMessage.Data[7] = 0xFF;																			// der Wert 0xFF wird in Datenbyte "7" gespeichert
			
			
			
			/* Daten senden */
				CAN_Transmit(CAN1, &TxMessage);																// Die Daten welche in der Variable TxMessage definiert wurden werden auf CAN1 gesendet
			
			
			/* Transmit Structure preparation */
				TxMessage.IDE = CAN_ID_EXT;																		// Auswahl der Adressart für CAN-Übertragung -> Extended Address
				TxMessage.ExtId = 0x18FF3331;																	// Definiere Adresse mit dem Wert 0x18FF3331 für Motoransteuerung "MotorControl1"
				TxMessage.RTR = CAN_RTR_DATA;																	// Definiere Art der Datenübertragung
				TxMessage.DLC = 8;																						// Defniere Anzahl der Datenbytes -> 8
			
			/* Variable finalValue2 in Datenbyte 0 und 1 schreiben */
			
				TxMessage.Data[0] = finalValue2;															// die 8 least significant Bits der Variable finalValue in Datenbyte "0" der CAN Nachricht speichern
				TxMessage.Data[1] = finalValue2 >> 8;													// die Bits in der Variable finalValue werden um 8 Stellen nach rechts verschoben und die neuen least significant Bits werden in Datenbyte "1" gespeichert
				TxMessage.Data[2] = 0xFF;																			// der Wert 0xFF wird in Datenbyte "2" gespeichert
				TxMessage.Data[3] = 0xFF;																			// der Wert 0xFF wird in Datenbyte "3" gespeichert
				TxMessage.Data[4] = 0xFF;																			// der Wert 0xFF wird in Datenbyte "4" gespeichert
				TxMessage.Data[5] = 0xFF;																			// der Wert 0xFF wird in Datenbyte "5" gespeichert
				TxMessage.Data[6] = 0xFF;																			// der Wert 0xFF wird in Datenbyte "6" gespeichert
				TxMessage.Data[7] = 0xFF;																			// der Wert 0xFF wird in Datenbyte "7" gespeichert
			
			
			
			/* Daten senden */
				CAN_Transmit(CAN1, &TxMessage);																// Die Daten welche in der Variable TxMessage definiert wurden werden auf CAN1 gesendet
	}
} 
}
}

/**
 * @name		RCC_Configuration
 * @brief		Configures the different system clocks.
 *
 */
void RCC_Configuration(void)																					// Funktion für Clock Aktivierung
{  
  
  /* Enable GPIO clock for CAN */
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);								// Clock auf AHB1 Bus wird für GPIOA, GPIOB, GPIOC, GPIOD aktiviert
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);								
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC, ENABLE);
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD, ENABLE);
	
  /* Enable CAN clock */
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_CAN1, ENABLE);								// Clock auf APB1 Bus wird für CAN1 aktiviert
	
  /* Enable USART clock */
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2,ENABLE);								// Clock auf APB1 Bus wird für USART2 aktiviert
	
	/* Enable ADC1 clock */
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, ENABLE);								// Clock auf APB2 Bus wird für ADC1, ADC2 aktiviert
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC2,ENABLE);	
						
}

/**
 * @name		GPIO_Configuration
 * @brief		Configures the different GPIO ports.
 *
 */
void GPIO_Configuration(void)																					// Funktion für GPIO Konfiguration
{
	GPIO_InitTypeDef GPIO_InitStructure;																// Benutzung der GPIO_InitStructure Bibliothek
	
	
	
	//configure PA0 (POT1) and PA1 (POT2) as input.
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1;							// Pin 0, Pin 1 wird festgelegt für diesen Parameterblock
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;										// Geschwindigkeit von für diesen Pin wird auf 2MHz gesetzt
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AN;												// Pin wird im Analogen Modus verwendet
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;										// Pin wird im NOPULL Modus verwendet
	GPIO_Init(GPIOA, &GPIO_InitStructure);															// Definierte Parameter gelten für GPIOA
	

	 /* Connect CAN pins to AF9 */
  GPIO_PinAFConfig(GPIOA, GPIO_PinSource11, GPIO_AF_CAN1);						// GPIOA Pin 11 wird der GPIO_AF_CAN1 (Alternate Function für CAN1) zugeordnet
  GPIO_PinAFConfig(GPIOA, GPIO_PinSource12, GPIO_AF_CAN1); 						// GPIOA Pin 12 wird der GPIO_AF_CAN1 (Alternate Function für CAN1) zugeordnet
  
  /* Configure CAN RX and TX pins */
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11 | GPIO_Pin_12;						// Pin 11 und Pin 12 werden festgelegt für diesen Parameterblock
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;												// Pin wird im ALternate Function Modus verwendet
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;										// Geschwindigkeit von für diesen Pin wird auf 2MHz gesetzt
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;											// Pin wird im PushPull Modus verwendet
  GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_UP;												// Pin wird im PullUp Modus verwendet
  GPIO_Init(GPIOA, &GPIO_InitStructure);															// Definierte Parameter gelten für GPIOA
	
	//configure ROW Pins as PullDown inputs.
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10 | GPIO_Pin_11 | GPIO_Pin_12;														// Pin 10, Pin 11, Pin 12 wird festgelegt für diesen Parameterblock
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;																									// Geschwindigkeit von für diesen Pin wird auf 2MHz gesetzt
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;																										// Output Type wird als PUSHPULL gewählt
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;																											// Pin wird im Input Modus verwendet
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_DOWN;																										// Pin wird im PULLDOWN Modus verwendet
	GPIO_Init(GPIOC, &GPIO_InitStructure);																														// Definierte Parameter gelten für GPIOC
	
	//configure ROW Pins as PullDown inputs.
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;																													// Pin 2 wird festgelegt für diesen Parameterblock
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;																									// Geschwindigkeit von für diesen Pin wird auf 2MHz gesetzt
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;																										// Output Type wird als PUSHPULL gewählt
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;																											// Pin wird im Input Modus verwendet
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_DOWN;																										// Pin wird im PULLDOWN Modus verwendet
	GPIO_Init(GPIOD, &GPIO_InitStructure);																														// Definierte Parameter gelten für GPIOD
	
	
	
	
	//configure Column Pins as NoPull Outputs.
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2 | GPIO_Pin_3;																						// Pin 2, Pin 3 wird festgelegt für diesen Parameterblock
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;																									// Geschwindigkeit von für diesen Pin wird auf 2MHz gesetzt
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;																										// Output Type wird als PUSHPULL gewählt
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;																											// Pin wird im Output Modus verwendet
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;																									// Pin wird im NOPULL Modus verwendet
	GPIO_Init(GPIOC, &GPIO_InitStructure);																														// Definierte Parameter gelten für GPIOC
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_14 | GPIO_Pin_15;																					// Pin 14, Pin 15 wird festgelegt für diesen Parameterblock
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;																									// Geschwindigkeit von für diesen Pin wird auf 2MHz gesetzt
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;																										// Output Type wird als PUSHPULL gewählt
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;																											// Pin wird im Output Modus verwendet
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;																									// Pin wird im NOPULL Modus verwendet
	GPIO_Init(GPIOB, &GPIO_InitStructure);																														// Definierte Parameter gelten für GPIOB
	
	
	//configure PB13 as Input Pin for Switch
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13;																												// Pin 13 wird festgelegt für diesen Parameterblock
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;																									// Geschwindigkeit von für diesen Pin wird auf 2MHz gesetzt
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;																										// Output Type wird als PUSHPULL gewählt
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;																											// Pin wird im Input Modus verwendet
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_DOWN;																										// Pin wird im PULLDOWN Modus verwendet
	GPIO_Init(GPIOB, &GPIO_InitStructure);																														// Definierte Parameter gelten für GPIOB
}

/**
 * @name		CAN_Config
 * @brief		Configures the CAN Interface
 *
 */

void CAN_Config(void)																														// Funktion für CAN Konfiguration starten
{
	CAN_InitTypeDef					CAN_InitStructure;																		// Benutzung der CAN-InitStructure Bibliothek
	CAN_FilterInitTypeDef		CAN_FilterInitStructure;															// Benutzung der CAN-FilterInitStructure Bibliothek
	NVIC_InitTypeDef  			NVIC_InitStructure;																		// Benutzung der NVIC-InitStructure Bibliothek

  /* CAN register init */
  CAN_DeInit(CAN1);																															// Zuordnung zu CAN1

  /* CAN cell init */																														// In diesem Block wurde der CAN auf Normalmodus eingestellt, Sonderfunktionen wurden alle deaktiviert
  CAN_InitStructure.CAN_TTCM = DISABLE;																	
  CAN_InitStructure.CAN_ABOM = DISABLE;
  CAN_InitStructure.CAN_AWUM = DISABLE;
  CAN_InitStructure.CAN_NART = DISABLE;
  CAN_InitStructure.CAN_RFLM = DISABLE;
  CAN_InitStructure.CAN_TXFP = DISABLE;
  CAN_InitStructure.CAN_Mode = CAN_Mode_Normal;
  CAN_InitStructure.CAN_SJW = CAN_SJW_1tq;
    
  /* CAN Baudrate = 250 kbps (CAN clocked at 45 MHz) */													// In diesem Block wird die CAN Baudrate mittels zweier Timequants und dem Prescaler eingestellt
  CAN_InitStructure.CAN_BS1 = CAN_BS1_12tq;																			// Timequant 1 wird auf 12 gestellt
  CAN_InitStructure.CAN_BS2 = CAN_BS2_2tq;																			// Timequant 2 wird auf 2 gestellt
  CAN_InitStructure.CAN_Prescaler = 12;																					// Prescaler wird auf 12 gesetzt, damit die 250 kbps erreicht werden
  CAN_Init(CAN1, &CAN_InitStructure);																						// Baudraten Einstellungen für CAN1 anwenden

  /* CAN filter init */																													// In diesem Abschnitt werden Standardeinstellungen zum CAN Filter gemacht
  CAN_FilterInitStructure.CAN_FilterNumber = 0;	
  CAN_FilterInitStructure.CAN_FilterMode = CAN_FilterMode_IdMask;
  CAN_FilterInitStructure.CAN_FilterScale = CAN_FilterScale_32bit;
  CAN_FilterInitStructure.CAN_FilterIdHigh = 0x0000;
  CAN_FilterInitStructure.CAN_FilterIdLow = 0x0000;
  CAN_FilterInitStructure.CAN_FilterMaskIdHigh = 0x0000;
  CAN_FilterInitStructure.CAN_FilterMaskIdLow = 0x0000;
  CAN_FilterInitStructure.CAN_FilterFIFOAssignment = 0;
  CAN_FilterInitStructure.CAN_FilterActivation = ENABLE;
  CAN_FilterInit(&CAN_FilterInitStructure);																			// Anwendung der Parameter für CAN_FilterInitStructure

  NVIC_InitStructure.NVIC_IRQChannel = CAN1_RX0_IRQn;														// In diesem Block werden Einstellungen zum Empfangen von CAN Nachrichten gemacht
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x0;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x0;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);																								// Anwendung der Parameter für NVIC_InitStructure
  
  /* Enable FIFO 0 message pending Interrupt */
  CAN_ITConfig(CAN1, CAN_IT_FMP0, ENABLE);
}

void ADC_Configuration(void)																										//Funktion für ADC Wandler Konfiguration
{
	ADC_InitTypeDef				ADC_InitStruct;																					// Benutzung der ADC_InitStruct Bibliothek
	ADC_CommonInitTypeDef	ADC_CommonInitStruct;																		// Benutzung der ADC_CommonInitStruct Bibliothek
	
	/* Configure CommonInitStrcuture */
	ADC_CommonInitStruct.ADC_DMAAccessMode = ADC_DMAAccessMode_Disabled;					// DMA (Direct Memory Access) wird deaktiviert
	ADC_CommonInitStruct.ADC_Mode = ADC_Mode_Independent;													// ADC wird im unabhängigen Modus verwendet
	ADC_CommonInitStruct.ADC_Prescaler = ADC_Prescaler_Div2;											// Prescaler wurde mit 2 der kleinste gewählt, somit läuft ADC auf schnellster Stufe
	ADC_CommonInitStruct.ADC_TwoSamplingDelay = ADC_TwoSamplingDelay_5Cycles;			// Es wurde der kleinste Delay von 5Cycles ausgewählt
	ADC_CommonInit(&ADC_CommonInitStruct);
	
	/* Configure ADC1 InitStrcuture */
	ADC_InitStruct.ADC_ContinuousConvMode = ENABLE;																// ContinousConvMode wird aktiviert
	ADC_InitStruct.ADC_DataAlign = ADC_DataAlign_Right;														// Daten werden Rechtsorientiert umgewandelt
	ADC_InitStruct.ADC_ExternalTrigConvEdge = ADC_ExternalTrigConvEdge_None;			// Es wird nicht mit einem externen Trigger gearbeitet
	ADC_InitStruct.ADC_NbrOfConversion = 1;																				// NbrOfConversion wird auf 1 gestellt, da ContinousConvMode aktiviert wurde
	ADC_InitStruct.ADC_Resolution = ADC_Resolution_12b;														// Auflösung wird auf 12Bit sprich 4096 Schritte gestellt
	ADC_InitStruct.ADC_ScanConvMode = DISABLE;																		// ScanConvMode wird deaktiviert
	ADC_Init(ADC1, &ADC_InitStruct);																							// definierte Parameter auf ADC1 anwenden
		
	ADC_RegularChannelConfig(ADC1, ADC_Channel_0, 1, ADC_SampleTime_3Cycles);			// Channel 0 mit ADC1 verbinden, Priorität 1 (nicht wichtig da nicht mehrere Channel aktiv) Sample Time auf 3 Zyklen einstellen
	
	ADC_Cmd(ADC1, ENABLE);																												// Definierte Einstellungen auf ADC1 anwenden
	
	
	ADC_CommonInitStruct.ADC_Mode = ADC_Mode_Independent;													// gleiche Einstellungen wie bei ADC1 für ADC2 definieren
  ADC_CommonInitStruct.ADC_Prescaler = ADC_Prescaler_Div2;
  ADC_CommonInitStruct.ADC_DMAAccessMode = ADC_DMAAccessMode_Disabled;
  ADC_CommonInitStruct.ADC_TwoSamplingDelay = ADC_TwoSamplingDelay_5Cycles;
  ADC_CommonInit(&ADC_CommonInitStruct);
	
  ADC_InitStruct.ADC_Resolution = ADC_Resolution_12b;
  ADC_InitStruct.ADC_ScanConvMode = DISABLE;
  ADC_InitStruct.ADC_ContinuousConvMode = ENABLE;
  ADC_InitStruct.ADC_ExternalTrigConvEdge = ADC_ExternalTrigConvEdge_None;
  ADC_InitStruct.ADC_ExternalTrigConv = ADC_ExternalTrigConv_T1_CC1;
  ADC_InitStruct.ADC_DataAlign = ADC_DataAlign_Right;
  ADC_InitStruct.ADC_NbrOfConversion = 1;
	ADC_Init(ADC2, &ADC_InitStruct);
	

	/* ADC2 regular channel1 configuration */	
	ADC_RegularChannelConfig(ADC2,ADC_Channel_1,1,ADC_SampleTime_3Cycles);				// Channel 1 mit ADC2 verbinden, Priorität 1 (nicht wichtig da nicht mehrere Channel aktiv) Sample Time auf 3 Zyklen einstellen
	
	/* Enable ADC2 */
  ADC_Cmd(ADC2, ENABLE);																												// Definierte Einstellungen auf ADC1 anwenden
	
}

void Delay (unsigned long tick)																									// Delay: Zwingt das Programm für die gewünschte Zeit zu warten
	{
		msTicksHold = msTicks;																											// Der aktuelle Wert des System Clocks wird gespeichert auf "TicksHold"
		while((msTicks - msTicksHold ) <= tick );																		// Hier wird solange gewartet, bis die Differenz zwischen dem gespeicherten und dem aktuellen System Clock Wert gleichgross wie "tick" ist
	}
	
	int Position_Values_left(void)																								// Position_Values_left: Berechnet die gewümschte Position in Grad für den linken Motor in Abhängigkeit von "distance" und "direction"
{
	if(direction == 16)		//Direction "Forward"																		// Überprüft ob Direction Forward gewählt wurde
	{
		valMotorL = 2105540607 - (6519 * distance);																	// Berechnet den Wert anhand der gewählten Distanz
	}
	
	if(direction == 15)		//Direction "Backwards"																	// Überprüft ob Direction Backwards gewählt wurde
	{
		valMotorL = 2105540607 + (6519 * distance);																	// Berechnet den Wert anhand der gewählten Distanz
	}
	
	if(direction == 14)		//Direction "Right"																			// Überprüft ob Direction Right gewählt wurde
	{
		valMotorL = 2105540607 + (147 * distance);																	// Berechnet den Wert anhand der gewählten Distanz
	}
	
	if(direction == 13)		//Direction "Left"																			// Überprüft ob Direction Left gewählt wurde
	{
		valMotorL = 2105540607 + (147 * distance);																	// Berechnet den Wert anhand der gewählten Distanz
	}
	
	return valMotorL;																															// Ausgabe der Funktion ist der berechnete Positionswert
}


int Position_Values_right(void)																									// Position_Values_right: Berechnet die gewümschte Position in Grad für den rechten Motor in Abhängigkeit von "distance" und "direction"
{																																								// gleiches Prinzip wie Position_Values_left
	if(direction == 16)		//Direction "Forward"
	{
		valMotorR = 2105540607 + (6519 * distance);
	}
	
	if(direction == 15)		//Direction "Backwards"
	{
		valMotorR = 2105540607 - (6519 * distance);
	}
	
	if(direction == 14)		//Direction "Right"
	{
		valMotorR = 2105540607 - (147 * distance);
	}
	
	if(direction == 13)		//Direction "Left"
	{
		valMotorR = 2105540607 - (147 * distance);
	}
	
	return valMotorR;
}

float calculateValue()																													// Funktion für Speicherung der ADC Werte und Umrechnung für Weiterverwendung																				
{
	for(i=0;i<50;i++)																															// for Schlaufe für Bildung des Mittelwertes Anzahl Durchgänge kann bestimmt werden. Momentan auf 50 Durchgänge gesetzt
		{																																						// Solange Bedingung von for Schlaufe "true" werden folgende Funktionen ausgeführt bis Bedingungen "false"
			ADC_Value = ADC_GetConversionValue(ADC1);																	// Bei jedem Durchgang wird der aktuelle Wert des ADC Wandlers in die Variable ADC_Value gespeichert																										
			sum = sum + ADC_Value;																										// "ADC_Value" wird bei jedem Durchgang zu sum addiert
			counter = counter + 1;																										// Variable counter wird bei jedem Durchgang um eins erhöht
		}
		
		meanValue = sum/counter;																										// "sum" wird durch "counter" geteilt um Mittelwert zu erhalten. Dieser Wert wird in "meanValue" gespeichert.
		sum = 0;																																		// "sum" wird wieder auf null gesetzt
		counter = 0;																																// "counter" wird wieder auf null gesetzt
		
		if(meanValue > 1900){																												// Hier wird eine Deadband gesetzt. Wenn der ausgelesene ADC Wert zwischen 1900 und 2100 liegt, wird dem Motor immer die Nullposition gesendet
			if(meanValue < 2100){
				speedValue = 32127;
			}
			else{
			speedValue = 20 * (meanValue - 2100) + 32127;															// Wert von "meanValue" wird mittels Formeln der Firma Sonceboz umgerechnet. (Berechnungen siehe Anhang)
			}
		}
		else{
		speedValue = 20 * (meanValue - 1900) + 32127;																// Wert von "meanValue" wird mittels Formeln der Firma Sonceboz umgerechnet. (Berechnungen siehe Anhang)
		}
		
		return speedValue;																													// Ausgabe der Funktion entspricht dem Wert in Variable speedValue
}

float calculateValue2()
{
	for(j=0;j<50;j++)																															// for Schlaufe für Bildung des Mittelwertes Anzahl Durchgänge kann bestimmt werden. Momentan auf 50 Durchgänge gesetzt
		{																																						// Solange Bedingung von for Schlaufe "true" werden folgende Funktionen ausgeführt bis Bedingungen "false"
			ADC_Value2 = ADC_GetConversionValue(ADC2);																// Bei jedem Durchgang wird der aktuelle Wert des ADC Wandlers in die Variable ADC_Value gespeichert																										
			sum2 = sum2 + ADC_Value2;																									// "ADC_Value" wird bei jedem Durchgang zu sum addiert
			counter2 = counter2 + 1;																									// Variable counter wird bei jedem Durchgang um eins erhöht
		}
		
		meanValue2 = sum2/counter2;																									// "sum" wird durch "counter" geteilt um Mittelwert zu erhalten. Dieser Wert wird in "meanValue" gespeichert.
		sum2 = 0;																																		// "sum" wird wieder auf null gesetzt
		counter2 = 0;																																// "counter" wird wieder auf null gesetzt
		
		if(meanValue2 > 1900){																											// Hier wird eine Deadband gesetzt. Wenn der ausgelesene ADC Wert zwischen 1900 und 2100 liegt, wird dem Motor immer die Nullposition gesendet
			if(meanValue2 < 2100){
				speedValue2 = 32127;
			}
			else{
			speedValue2 = 20 * (meanValue2 - 2100) + 32127;														// Wert von "meanValue2" wird mittels Formeln der Firma Sonceboz umgerechnet. (Berechnungen siehe Anhang)
			}
		}
		else{
		speedValue2 = 20 * (meanValue2 - 1900) + 32127;															// Wert von "meanValue2" wird mittels Formeln der Firma Sonceboz umgerechnet. (Berechnungen siehe Anhang)
		}
		
		return speedValue2;																													// Ausgabe der Funktion entspricht dem Wert in Variable speedValue
}
/******END OF FILE****/
