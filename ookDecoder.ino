// 2012-06-21 - Oregon V2 decoder added - Dominique Pierre
// 2012-06-21 - Oregon V3 decoder revisited - Dominique Pierre
// New code to decode OOK signals from weather sensors, etc.
// 2010-04-11 <jcw@equi4.com> http://opensource.org/licenses/mit-license.php

#include "DecodeOOK.h"
#include "CrestaDecoder.h"
#include "HezDecoder.h"
#include "KakuDecoder.h"
#include "OregonDecoderV2.h"
#include "OregonDecoderV3.h"
#include "XrfDecoder.h"
#include "EMxDecoder.h"
#include "FSxDecoder.h"
#include "KSxDecoder.h"
#include "VisonicDecoder.h"

//433
OregonDecoderV2 orscV2;
OregonDecoderV3 orscV3;
CrestaDecoder cres;
KakuDecoder kaku;
XrfDecoder xrf;
HezDecoder hez;
//868
//VisonicDecoder viso;
//EMxDecoder emx;
//KSxDecoder ksx;
//FSxDecoder fsx;

#define PORT 2

volatile word pulse;

#if defined(__AVR_ATmega1280__)//Arduino
void ext_int_1(void) {
#else//JeeNode
ISR(ANALOG_COMP_vect) {
#endif
    static word last;
    // determine the pulse length in microseconds, for either polarity
    pulse = micros() - last;
    last += pulse;
}

void reportSerial (const char* s, class DecodeOOK& decoder) {
    byte pos;
    const byte* data = decoder.getData(pos);
    Serial.print(s);
    Serial.print(' ');
    for (byte i = 0; i < pos; ++i) {
        Serial.print(data[i] >> 4, HEX);
        Serial.print(data[i] & 0x0F, HEX);
    }
    
    // Serial.print(' ');
    // Serial.print(millis() / 1000);
    Serial.println();
    
    decoder.resetDecoder();
}


void setup () {
    Serial.begin(115200);
    Serial.println("\n[ookDecoder]");

#if !defined(__AVR_ATmega1280__)//JeeNode code
    pinMode(13 + PORT, INPUT);  // use the AIO pin
    digitalWrite(13 + PORT, 1); // enable pull-up

    // use analog comparator to switch at 1.1V bandgap transition
    ACSR = _BV(ACBG) | _BV(ACI) | _BV(ACIE);

    // set ADC mux to the proper port
    ADCSRA &= ~ bit(ADEN);
    ADCSRB |= bit(ACME);
    ADMUX = PORT - 1;
#else//Arduino
//attach our interrupt handler on port int1 (arduino uno port2)
   attachInterrupt(1, ext_int_1, CHANGE);
//no idea what this is for...
   DDRE  &= ~_BV(PE5);
   PORTE &= ~_BV(PE5);
#endif
}

void loop () {
    static int i = 0;
    cli();
    word p = pulse;
    
    pulse = 0;
    sei();

    //if (p != 0){ Serial.print(++i); Serial.print('\n');}

    //433Mhz
    if (p != 0) {
        if (orscV2.nextPulse(p))
            reportSerial("OSV2", orscV2);  
        if (orscV3.nextPulse(p))
            reportSerial("OSV3", orscV3);        
        if (cres.nextPulse(p))
            reportSerial("CRES", cres);        
        if (kaku.nextPulse(p))
            reportSerial("KAKU", kaku);        
        if (xrf.nextPulse(p))
            reportSerial("XRF", xrf);        
        if (hez.nextPulse(p))
            reportSerial("HEZ", hez);        
    }

    //868Mhz
//    if (p != 0) {
//        if (viso.nextPulse(p))
//            reportSerial("VISO", viso);
//        if (emx.nextPulse(p))
//            reportSerial("EMX", emx);
//        if (ksx.nextPulse(p))
//            reportSerial("KSX", ksx);
//        if (fsx.nextPulse(p))
//            reportSerial("FSX", fsx);
//    }
}
