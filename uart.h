#ifndef __UART_H
#define __UART_H

//==============================================================================
// Uart paramétereket itt lehet beállítani
// - Inicializálás: UartInit()
// - Adás: UartTx(unsigned char ch) (ha a hívás elõtt uTxBufFree == 0 -> nem érdemes elküldeni, mert meg van telve)
// - Vétel: ch = UartRx() (csak akkor ad eredményt, ha a hívás elõtt uRxLength > 0)

//------------------------------------------------------------------------------
#include "hardwareprofile.h"
// Ha külsõ hardverprofilt használunk akkor azt itt adhatjuk meg
// - belsõ hardverprofil esetén kommenteljük ki
// - külsõ hardverprofil esetén az ott megadott paramétereket ebben a file-ban kommenteljük ki

//------------------------------------------------------------------------------
// Adás puffer mérete (4, 8, 16, 32, 64, 128)
// #define UTXBUFFERSIZE 64

//------------------------------------------------------------------------------
// Vételi puffer mérete (4, 8, 16, 32, 64, 128)
// #define URXBUFFERSIZE 64

//------------------------------------------------------------------------------
// Rendszer órajel (UART osztó beállításához)
// #define SystemClock   48000000

//------------------------------------------------------------------------------
// Soros port kívánt sebessége
// #define BAUDRATE      31250

//------------------------------------------------------------------------------
// Uart lábak I/O beállítása (inicializáláskor történik ez meg)
// #define UARTPININIT   TRISCbits.TRISC6 = 0; TRISCbits.TRISC7 = 1

//------------------------------------------------------------------------------
// Teszt (vételi puffer programból is fetölthetõ lesz, ha engedélyezzük)
// #define UARTDEBUG

//------------------------------------------------------------------------------
// csak PIC18xxx: melyik prioritású megszakítást használja a soros port
// lehetséges értékei:
// - -1 : nincs a többszintû prioritás használva (ilyenkor minden csak a HIGH interruptban megy)
// -  0 : alacsony prioritás
// -  1 : magas prioritás
#define RXPR18        -1
#define TXPR18        -1

//******************************************************************************
// Innentõl kezdve nem kell belenyúlni !
//
//==============================================================================
extern volatile unsigned char uRxLength; // vételi pufferban levõ byte-ok száma
extern volatile unsigned char uRxBufFree;// vételi pufferben mennyi a szabad hely
extern volatile unsigned char uTxLength; // adás pufferben levõ byte-ok száma
extern volatile unsigned char uTxBufFree;// adás pufferben mennyi a szabad hely

void UartInit(void);                   // inicializálás

unsigned char UartRx(void);            // egy karakter vétele (ha van mit)
unsigned char UartRxTop(void);         // a vételi puffer legelsõnek berakott eleme, de nem törlõdik a pufferbõl
#ifdef  UARTDEBUG
void UartRxWrite(unsigned char ch);    // teszteléshez fel lehet tölteni a vételi puffert
#else
#define UartRxWrite(ch)  ;
#endif

void UartRxProcess(void);              // uart vétel polling vagy megszakítás kiszolgáló függvény
#define UartRxIntProcess() if(PIE1bits.RCIE && PIR1bits.RCIF)UartRxProcess()

void UartTx(unsigned char ch);         // egy karakter küldése (pufferelt módon)
void UartTxProcess(void);              // uart adás polling vagy megszakítás kiszolgáló függvény
#define UartTxIntProcess() if(PIE1bits.TXIE && PIR1bits.TXIF)UartTxProcess()

#endif // __UART_H
