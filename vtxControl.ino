#if defined(ARDUINO) && ARDUINO >= 100
#include "Arduino.h"
#else
#include "wiring.h"
#endif
#include "vtxControl.h"
#include "OSD_Config.h"
#include <EEPROM.h>


#define CHANNEL_MODE 0
#define BAND_MODE 1
#define MODESWITCH_LONGPRESS_TIME 800
#define BLINK_PERIOD 500
#define CHANNEL_LED_PIN A0
#define BAND_LED_PIN A1
#define VTX_SPI_CLK A4
#define VTX_SPI_DATA A2
#define VTX_SPI_RTS A3
#define BUTTON_PIN A5

void vtxSpiWriteData();
void vtxCheckButton();

unsigned int count;
unsigned long tmp;

int start_freq[] = {5865,5733,5705,5740,5658};
char freq_steps[] = {-20,19,-20,20,37}; 
byte channel = 0;
byte old_channel = 1;
byte band = 0;
byte old_band = 1;
byte blinkCount = 0;
byte state = CHANNEL_MODE;
byte buttonState = 0;
boolean flashLeds = false;
unsigned long data;

void vtxSpiWriteData() {
  //pinMode(VTX_SPI_RTS, OUTPUT);
  byte i;
  digitalWrite(VTX_SPI_RTS, LOW);

  for(i=0;i<25;i++) {
    tmp = data>>i & 1;
    if (tmp == 1) {
      //pinMode(VTX_SPI_DATA, INPUT_PULLUP);
      //digitalWrite(VTX_SPI_DATA,LOW);
      digitalWrite(VTX_SPI_DATA,HIGH);
    }else{
      //pinMode(VTX_SPI_DATA, OUTPUT);
      digitalWrite(VTX_SPI_DATA,LOW);
    }
    //pinMode(VTX_SPI_CLK, INPUT_PULLUP);
    //digitalWrite(VTX_SPI_CLK,LOW);
    digitalWrite(VTX_SPI_CLK, HIGH);
    //pinMode(VTX_SPI_CLK, OUTPUT);
    digitalWrite(VTX_SPI_CLK, LOW);
  }    

  //pinMode(VTX_SPI_RTS, INPUT_PULLUP);
  //digitalWrite(VTX_SPI_RTS,LOW);
  digitalWrite(VTX_SPI_RTS, HIGH);

}


void vtxSpiCtrl()
{
  vtxCheckButton();

  if (channel != old_channel || band != old_band) 
  {
    old_channel = channel;
    old_band = band;
	
    data =   ((unsigned long)(start_freq[band]
    + channel*freq_steps[band])*100)<<3;
    data |= 0x11;
    data = ((data & 0xFFFFF800)<<1) | (data & 0xFF7);
    vtxSpiWriteData();
  }
}



void vtxCheckButton() 
{
	if (digitalRead(BUTTON_PIN) == LOW)
	{
		if (buttonState == 0)
		{
			buttonState = 1;
			count = 0;
			flashLeds = false;
		}
		else if ((buttonState >= 1) && (count > MODESWITCH_LONGPRESS_TIME))
		{
			// switch mode
			state = (state==CHANNEL_MODE)?BAND_MODE:CHANNEL_MODE;
			flashLeds = true;
			count = 0;
			buttonState = 2;
		}
	}
	else  /* button == HIGH (released) */
	{
		if (buttonState == 1)
		{
			if (state == CHANNEL_MODE)
			{
				channel++;
                                EEPROM.write(channel,CHANNEL_ADDR);
				channel = channel%8;
			}
			else /* state = BAND_MODE */
			{
				band++;
                                writeEEPROM(band,BAND_ADDR);
				band=band%5;
			}
			flashLeds = true;
		}
		buttonState = 0;
	}
	
	count++;

	if (flashLeds == true)
	{
		if (state == CHANNEL_MODE)
		{
			if (count%BLINK_PERIOD == 0 && blinkCount <= channel)
			{
				digitalWrite(CHANNEL_LED_PIN,HIGH);
				blinkCount++;
			}
			else if (count%BLINK_PERIOD == BLINK_PERIOD/2 )
			{
				digitalWrite(CHANNEL_LED_PIN,LOW);
			}
		}
		else /* BAND_MODE */
		{
			if ((count%BLINK_PERIOD == 0) && (blinkCount <= band))
			{
				digitalWrite(BAND_LED_PIN,HIGH);
				blinkCount++;
			}
			else if (count%BLINK_PERIOD == BLINK_PERIOD/2 )
				
			{
				digitalWrite(BAND_LED_PIN,LOW);
			}
		}		
	}
	else /* flashLeds == false */
	{
		blinkCount = 0;
		digitalWrite(CHANNEL_LED_PIN,LOW);
		digitalWrite(BAND_LED_PIN,LOW);
	}
	

}


void vtxSetup() 
{
    pinMode(BUTTON_PIN, INPUT_PULLUP);
    pinMode(CHANNEL_LED_PIN,OUTPUT);
    pinMode(BAND_LED_PIN,OUTPUT);
    count=0;
    // set the digital pin as output:
    pinMode(VTX_SPI_DATA, OUTPUT);
    pinMode(VTX_SPI_CLK, OUTPUT);
    pinMode(VTX_SPI_RTS, OUTPUT);
    //pinMode(VTX_SPI_RTS, INPUT_PULLUP);
    //digitalWrite(VTX_SPI_RTS,LOW);
    digitalWrite(VTX_SPI_RTS,HIGH);
    digitalWrite(VTX_SPI_DATA,LOW);
    digitalWrite(VTX_SPI_CLK, LOW);
    
    band = readEEPROM(BAND_ADDR);
    channel = readEEPROM(CHANNEL_ADDR);
    
    data = 0x1F;
    vtxSpiWriteData();
    delay(10);

}


