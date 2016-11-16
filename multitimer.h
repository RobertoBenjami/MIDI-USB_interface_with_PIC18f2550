#ifndef __MULTITIMER_H
#define __MULTITIMER_H

//==============================================================================
// multitimer interfész v0.11 (itt adjuk meg a paramétereket)
//==============================================================================

// a projectre globális paraméterek itt vannak megadva
#include "HardwareProfile.h"

// idõzítõ forrásórajele (Hz)
// #define SystemClock 48000000

// ha valamelyik idõzítõ eltérõ órajelrõl megy, az külön megadható pl:
// (ebben az esetben az inicializáló függvényhívás után az órajelforrást is állítsuk be)
// #define Timer0SrcClock 32768
// #define Timer1SrcClock 32768
// #define Timer2SrcClock 32768
// #define Timer3SrcClock 32768
// #define Timer4SrcClock 32768
// #define Timer5SrcClock 32768

// frekvencia megadása (Hz)
#define TIME0CLK          4
// #define TIME1CLK    1000
// #define TIME2CLK    1000
// #define TIME3CLK    1000
// #define TIME4CLK    1000
// #define TIME5CLK    1000

// PIC18: timer IRQ prioritása (-1 = nincs prioritás, 0 = alacsony, 1 = magas)
// ha prioritás nélküli megszakításkezelést haználunk, akkor mind az legyen!)
#define TIMER0PR18    -1
#define TIMER1PR18    -1
#define TIMER2PR18    -1
#define TIMER3PR18    -1

// PIC24: timer IRQ prioritása (1=legalacsonyabb, 7=legmagasabb)
#define TIMER1PR24     1
#define TIMER2PR24     1
#define TIMER3PR24     1
#define TIMER4PR24     1
#define TIMER5PR24     1

// Összefoglaló a különféle vezérlõcsaládok esetén felhasználható függvényekrõl
// (a * jelölésû üres függvény, elhagyható)
/*==============================================================================|
|        AVR        |       PIC16       |       PIC18       |       PIC24       |
|===================|===================|===================|===================|
|                                 Inicializálás                                 |
|   Timer0Init()    |   Timer0Init()    |   Timer0Init()    |                   |
|   Timer1Init()    |   Timer1Init()    |   Timer1Init()    |   Timer1Init()    |
|   Timer2Init()    |   Timer2Init()    |   Timer2Init()    |   Timer2Init()    |
|                   |                   |   Timer3Init()    |   Timer3Init()    |
|                   |                   |                   |   Timer4Init()    |
|                   |                   |                   |   Timer5Init()    |
|-------------------|-------------------|-------------------|-------------------|
|                          Globélis megszakítás engedélyezése                   |
|   (inicializálás után ezt is érdemes megtenni, ha nem máshol tesszük meg)     |
|                   | TimersGlobalIrq() | TimersGlobalIrq() |                   |
|-------------------|-------------------|-------------------|-------------------|
|                      Timer Comparátor hiányának pótlása                       |
|                (megszakításkiszolgáló függvénybe kell berakni)                |
|                   | Timer0Compless()  | Timer0Compless()  |                   |
|                   | Timer1Compless()  | Timer1Compless()  |                   |
|                   | Timer2Compless()* | Timer2Compless()* |                   |
|                   |                   | Timer3Compless()  |                   |
|                   |                   |                   |                   |
|                   |                   |                   |                   |
|-------------------|-------------------|-------------------|-------------------|
|                  Timer Start (megszakításának engedélyezése)                  |
|   Timer0Start()   |   Timer0Start()   |   Timer0Start()   |                   |
|   Timer1Start()   |   Timer1Start()   |   Timer1Start()   |   Timer1Start()   |
|   Timer2Start()   |   Timer2Start()   |   Timer2Start()   |   Timer2Start()   |
|                   |                   |   Timer3Start()   |   Timer3Start()   |
|                   |                   |                   |   Timer4Start()   |
|                   |                   |                   |   Timer5Start()   |
|-------------------|-------------------|-------------------|-------------------|
|                     Timer Stop (megszakításának tiltása)                      |
|   Timer0Stop()    |   Timer0Stop()    |   Timer0Stop()    |                   |
|   Timer1Stop()    |   Timer1Stop()    |   Timer1Stop()    |   Timer1Stop()    |
|   Timer2Stop()    |   Timer2Stop()    |   Timer2Stop()    |   Timer2Stop()    |
|                   |                   |   Timer3Stop()    |   Timer3Stop()    |
|                   |                   |                   |   Timer4Stop()    |
|                   |                   |                   |   Timer5Stop()    |
|-------------------|-------------------|-------------------|-------------------|
|               Timer megszakításkiszolgáló függvény definiálása                |
|               (a void visszatérõ érték nem szabad kiírni elé!)                |
|    Timer0Int()    |    Timer0Int()    |    Timer0Int()    |                   |
|    Timer1Int()    |    Timer1Int()    |    Timer1Int()    |    Timer1Int()    |
|    Timer2Int()    |    Timer2Int()    |    Timer2Int()    |    Timer2Int()    |
|                   |                   |    Timer3Int()    |    Timer3Int()    |
|                   |                   |                   |    Timer4Int()    |
|                   |                   |                   |    Timer5Int()    |
|-------------------|-------------------|-------------------|-------------------|
|        Timer megszakítákiszolgáló függvényben a megszakítás nyugtázása        |
|                   |  Timer0IrqAck()   |  Timer0IrqAck()   |                   |
|                   |  Timer1IrqAck()   |  Timer1IrqAck()   |  Timer1IrqAck()   |
|                   |  Timer2IrqAck()   |  Timer2IrqAck()   |  Timer2IrqAck()   |
|                   |                   |  Timer3IrqAck()   |  Timer3IrqAck()   |
|                   |                   |                   |  Timer4IrqAck()   |
|                   |                   |                   |  Timer5IrqAck()   |
|-------------------|-------------------|-------------------|-------------------|
|                   Közös megszakítákiszolgáló függvénybõl a                    |
|                  timer megszakításkiszolgáló függvény hívása                  |
| pl. Timer0IntProcess(Timer0Process) -> Timer0Process függvény fog meghívódni  |
|                   |Timer0IntProcess(f)|Timer0IntProcess(f)|                   |
|                   |Timer1IntProcess(f)|Timer1IntProcess(f)|                   |
|                   |Timer2IntProcess(f)|Timer2IntProcess(f)|                   |
|                   |                   |Timer3IntProcess(f)|                   |
|==============================================================================*/


//==============================================================================
// multitimer v0.11 (innent?l nem szükséges belenyúlni
//==============================================================================

#if defined(__AVR__)
#define TimerSrcClock (1UL*SystemClock)
#elif (defined(_PIC14) || defined(_PIC14E) || defined(__18CXX) || defined(__PICC18__) || defined(__dsPIC30F__))
#define TimerSrcClock (1UL*SystemClock/4)
#elif (defined(__dsPIC33F__) || defined(__PIC24F__) || defined(__PIC24FK__) || defined(__PIC24H__))
#define TimerSrcClock (1UL*SystemClock/2)
#else
#error "ismeretlen processzortípus"
#endif // defined(xxx proci)

// ennyivel kell osztani a rendszerórajelet
#ifdef TIME0CLK
#ifndef Timer0SrcClock
#define Timer0SrcClock TimerSrcClock
#endif
#define TIME0DIV  ((Timer0SrcClock+TIME0CLK/2)/TIME0CLK)
#endif

#ifdef TIME1CLK
#ifndef Timer1SrcClock
#define Timer1SrcClock TimerSrcClock
#endif
#define TIME1DIV  ((Timer1SrcClock+TIME1CLK/2)/TIME1CLK)
#endif

#ifdef TIME2CLK
#ifndef Timer2SrcClock
#define Timer2SrcClock TimerSrcClock
#endif
#define TIME2DIV  ((Timer2SrcClock+TIME2CLK/2)/TIME2CLK)
#endif

#ifdef TIME3CLK
#ifndef Timer3SrcClock
#define Timer3SrcClock TimerSrcClock
#endif
#define TIME3DIV  ((Timer3SrcClock+TIME3CLK/2)/TIME3CLK)
#endif

#ifdef TIME4CLK
#ifndef Timer4SrcClock
#define Timer4SrcClock TimerSrcClock
#endif
#define TIME4DIV  ((Timer4SrcClock+TIME4CLK/2)/TIME4CLK)
#endif

#ifdef TIME5CLK
#ifndef Timer5SrcClock
#define Timer5SrcClock TimerSrcClock
#endif
#define TIME5DIV  ((Timer5SrcClock+TIME5CLK/2)/TIME5CLK)
#endif

//==============================================================================
// AVR
#if defined(__AVR__)
#include <avr/interrupt.h>
#include <compat/ina90.h>

//------------------------------------------------------------------------------
// AVR timer0
#ifdef TIME0CLK

// osztó0
#if TIME0DIV < 256
#define TM0CLKDIV        1
#define TM0CLKSEL        1
#elif TIME0DIV < (256*8)
#define TM0CLKDIV        8
#define TM0CLKSEL        2
#elif TIME0DIV < (256*64)
#define TM0CLKDIV       64
#define TM0CLKSEL        3
#elif TIME0DIV < (256*256)
#define TM0CLKDIV      256
#define TM0CLKSEL        4
#elif TIME0DIV < (256*1024)
#define TM0CLKDIV     1024
#define TM0CLKSEL        5
#else
#error "AVR: timer0 frekvencia túl alacsony"
#endif // TIME0DIV

// timer0 komparátorba ennyit kell berakni hogy a kívánt frekvencia legyen
#define TM0COMP (((Timer0SrcClock/TM0CLKDIV)+TIME0CLK/2)/TIME0CLK-1)

// timer0 inicializálás
#ifdef  TCCR0A
#ifdef  TCCR0B // (A/B regiszteres TIMER0)
#define Timer0Init() {\
  OCR0A = TM0COMP;                      /* comparator */\
  TCCR0A = (1<<WGM01);                  /* MODE1 */\
  TCCR0B = (TM0CLKSEL<<CS00);           /* frekvencia osztás */\
  sei();}
#else // TCCR0B (A regiszteres TIMER0)
#define Timer0Init() {\
  OCR0A = TM0COMP;                      /* comparator */\
  TCCR0A = (1<<CTC0)|(TM0CLKSEL<<CS00); /* CTC mode, frekvencia osztás */\
  sei();}
#endif // else TCCR0B
#else  // TCCR0A
#define Timer0Init() {\
  OCR0 = TM0COMP;                       /* comparator */\
  TCCR0 = (1<<WGM01)|(TM0CLKSEL<<CS00); /* CTC mode, frekvencia osztás */\
  sei();}
#endif // TCCR0A

// timer0 bekapcsolás
#ifdef TIMSK0
#define Timer0Start()  TIMSK0 = (1<<OCIE0A)
#else  // TIMSK0
#define Timer0Start()  TIMSK |= (1<<OCIE0)
#endif // TIMSK0

// timer0 kikapcsolás
#ifdef TIMSK0
#define Timer0Stop()   TIMSK0 = 0
#else  // TIMSK0
#define Timer0Stop()   TIMSK &= ~(1<<OCIE0)
#endif // TIMSK0

// timer0 megszakítás kiszolgáló függvény
#ifdef  TIMER0_COMP_vect
#define Timer0Int()    ISR (TIMER0_COMP_vect)
#else
#define Timer0Int()    ISR (TIMER0_COMPA_vect)
#endif

#endif // TIME0CLK

//------------------------------------------------------------------------------
// AVR timer1
#ifdef TIME1CLK

// osztó1
#if TIME1DIV < 65536
#define TM1CLKDIV        1
#define TM1CLKSEL        1
#elif TIME1DIV < (65536*8)
#define TM1CLKDIV        8
#define TM1CLKSEL        2
#elif TIME1DIV < (65536*64)
#define TM1CLKDIV       64
#define TM1CLKSEL        3
#elif TIME1DIV < (65536*256)
#define TM1CLKDIV      256
#define TM1CLKSEL        4
#elif TIME1DIV < (65536*1024)
#define TM1CLKDIV     1024
#define TM1CLKSEL        5
#else
#error "AVR: timer1 frekvencia túl alacsony"
#endif // TIME1DIV

// timer1 komparátorba ennyit kell berakni hogy a kívánt frekvencia legyen
#define TM1COMP (((Timer1SrcClock/TM1CLKDIV)+TIME1CLK/2)/TIME1CLK-1)

// timer1 inicializálás
#define Timer1Init() {\
  OCR1AH = TM1COMP >> 8;                /* comparator HI */\
  OCR1AL = (unsigned char)TM1COMP;      /* comparator LO */\
  TCCR1A = (0<<WGM10);                  /* mode4 (CTC) */\
  TCCR1B = (1<<WGM12)|(TM1CLKSEL<<CS10);/* mode4, Clk = ClkIO/1..8..64..256..1024 */\
  sei();}
  
// timer1 bekapcsolás
#ifdef TIMSK1
#define Timer1Start()  TIMSK1 = (1<<OCIE1A)
#else  // TIMSK1
#define Timer1Start()  TIMSK |= (1<<OCIE1A)
#endif // TIMSK1

// timer1 kikapcsolás
#ifdef TIMSK1
#define Timer1Stop()   TIMSK1 = 0
#else  // TIMSK1
#define Timer1Stop()   TIMSK &= ~(1<<OCIE1A)
#endif // TIMSK1

// timer1 megszakítás kiszolgáló függvény
#define Timer1Int()    ISR (TIMER1_COMPA_vect)

#endif // TIME1CLK
//------------------------------------------------------------------------------
// AVR timer2
#ifdef TIME2CLK

// osztó2
#if TIME2DIV < 256
#define TM2CLKDIV        1
#define TM2CLKSEL        1
#elif TIME2DIV < (256*8)
#define TM2CLKDIV        8
#define TM2CLKSEL        2
#elif TIME2DIV < (256*32)
#define TM2CLKDIV       32
#define TM2CLKSEL        3
#elif TIME2DIV < (256*64)
#define TM2CLKDIV       64
#define TM2CLKSEL        4
#elif TIME2DIV < (256*128)
#define TM2CLKDIV      128
#define TM2CLKSEL        5
#elif TIME2DIV < (256*256)
#define TM2CLKDIV      256
#define TM2CLKSEL        6
#elif TIME2DIV < (256*1024)
#define TM2CLKDIV     1024
#define TM2CLKSEL        7
#else
#error "AVR: timer2 frekvencia túl alacsony"
#endif // TIME2DIV

// timer2 komparátorba ennyit kell berakni hogy a kívánt frekvencia legyen
#define TM2COMP (((Timer2SrcClock/TM2CLKDIV)+TIME2CLK/2)/TIME2CLK-1)

// timer2 inicializálás
#ifdef TCCR2A
#define Timer2Init() {\
  OCR2A = TM2COMP;                      /* comparator */\
  TCCR2A = (1<<WGM21);                  /* mode 1 (CTC) */\
  TCCR2B = (TM2CLKSEL<<CS20);           /* osztó */\
  sei();}
#else  // TCCR2A
#define Timer2Init() {\
  OCR2  = TM2COMP;                      /* comparator */\
  TCCR2  = (1<<WGM21)|(TM2CLKSEL<<CS20);/* mode (CTC), osztó */\
  sei();}
#endif // TCCR2A
  
// timer2 bekapcsolás
#ifdef TIMSK2
#define Timer2Start()  TIMSK2 = (1<<OCIE2A)
#else  // TIMSK1
#define Timer2Start()  TIMSK |= (1<<OCIE2)
#endif // TIMSK1

// timer2 kikapcsolás
#ifdef TIMSK2
#define Timer2Stop()   TIMSK2 = 0
#else  // TIMSK1
#define Timer2Stop()   TIMSK &= ~(1<<OCIE2)
#endif // TIMSK1

// timer2 megszakítás kiszolgáló függvény
#ifdef  TIMER0_COMP_vect
#define Timer2Int() ISR (TIMER2_COMP_vect)
#else
#define Timer2Int() ISR (TIMER2_COMPA_vect)
#endif

#endif // TIME2CLK

// AVR
//==============================================================================
// PIC16

#elif (defined(_PIC14) || defined(_PIC14E))

//------------------------------------------------------------------------------
// PIC16 timer0
#ifdef TIME0CLK

// osztó0
#if TIME0DIV < 256
#define TM0CLKDIV      1
#define TM0CLKSEL      OPTION_REGbits.PSA = 1
#define TM0COMPCOR     2
#elif TIME0DIV < (256*2)
#define TM0CLKDIV      2
#define TM0CLKSEL      OPTION_REGbits.PS = 0; OPTION_REGbits.PSA = 0
#define TM0COMPCOR     1
#elif TIME0DIV < (256*4)
#define TM0CLKDIV      4
#define TM0CLKSEL      OPTION_REGbits.PS = 1; OPTION_REGbits.PSA = 0
#define TM0COMPCOR     0
#elif TIME0DIV < (256*8)
#define TM0CLKDIV      8
#define TM0CLKSEL      OPTION_REGbits.PS = 2; OPTION_REGbits.PSA = 0
#define TM0COMPCOR     0
#elif TIME0DIV < (256*16)
#define TM0CLKDIV      16
#define TM0CLKSEL      OPTION_REGbits.PS = 3; OPTION_REGbits.PSA = 0
#define TM0COMPCOR     0
#elif TIME0DIV < (256*32)
#define TM0CLKDIV      32
#define TM0CLKSEL      OPTION_REGbits.PS = 4; OPTION_REGbits.PSA = 0
#define TM0COMPCOR     0
#elif TIME0DIV < (256*64)
#define TM0CLKDIV      64
#define TM0CLKSEL      OPTION_REGbits.PS = 5; OPTION_REGbits.PSA = 0
#define TM0COMPCOR     0
#elif TIME0DIV < (256*128)
#define TM0CLKDIV      128
#define TM0CLKSEL      OPTION_REGbits.PS = 6; OPTION_REGbits.PSA = 0
#define TM0COMPCOR     0
#elif TIME0DIV < (256*256)
#define TM0CLKDIV      256
#define TM0CLKSEL      OPTION_REGbits.PS = 7; OPTION_REGbits.PSA = 0
#define TM0COMPCOR     0
#else
#error "PIC16: timer0 frekvencia túl alacsony"
#endif  // TIME0DIV

// timer0 komparátorba ennyit kell berakni hogy a kívánt frekvencia legyen
#define TM0COMP (((Timer0SrcClock/TM0CLKDIV)+TIME0CLK/2)/TIME0CLK-1)

// timer0 komparátor hiányának pótlása
#define Timer0Compless()  TMR0 += (TM0COMPCOR-TM0COMP)

// timer0 inicializálás
#define Timer0Init() {\
  OPTION_REGbits.T0CS = 0;              /* forrás órajel = systemclock */\
  TM0CLKSEL;                            /* osztó */\
  TMR0 = (255+TM0COMPCOR-TM0COMP);}     /* timer kezdõérték feltöltése */

// timer0 bekapcsolás (nem bekapcsolható, csak IRQ-t lehet engedélyezni)
#define Timer0Start()  if(!INTCONbits.T0IE){TMR0 = (255+TM0COMPCOR-TM0COMP); INTCONbits.T0IE = 1;}

// timer0 kikapcsolás (nem kikapcsolható, csak IRQ-t lehet tiltani)
#define Timer0Stop()   INTCONbits.T0IE = 0

// timer0 megszakítás nyugtázás
#define Timer0IrqAck() INTCONbits.T0IF = 0

// Közös megszakítákiszolgáló függvénybõl a timer0 megszakításkiszolgáló függvény hívása
#define Timer0Intprocess(f) if(INTCONbits.T0IE && INTCONbits.T0IF) f()

#endif // TIME0CLK

//------------------------------------------------------------------------------
// PIC16 timer1
#ifdef TIME1CLK

// osztó1
#if TIME1DIV < 65536
#define TM1CLKDIV      1
#define TM1CLKSEL      T1CONbits.T1CKPS = 0
#define TM1COMPCOR     1
#elif TIME1DIV < (65536*2)
#define TM1CLKDIV      2
#define TM1CLKSEL      T1CONbits.T1CKPS = 1
#define TM1COMPCOR     0
#elif TIME1DIV < (65536*4)
#define TM1CLKDIV      4
#define TM1CLKSEL      T1CONbits.T1CKPS = 2
#define TM1COMPCOR     0
#elif TIME1DIV < (65536*8)
#define TM1CLKDIV      8
#define TM1CLKSEL      T1CONbits.T1CKPS = 3
#define TM1COMPCOR     0
#else
#error "PIC16: timer1 frekvencia túl alacsony"
#endif  // TIME1DIV

// timer1 komparátorba ennyit kell berakni hogy a kívánt frekvencia legyen
#define TM1COMP (((Timer1SrcClock/TM1CLKDIV)+TIME1CLK/2)/TIME1CLK-1)

// timer1 komparátor hiányának pótlása
#define Timer1Compless()  TMR1 += (65535+TM1COMPCOR-TM1COMP)

// timer1 inicializálás
#define Timer1Init() {\
  T1CONbits.TMR1CS = 0;                 /* forrás órajel = systemclock */\
  TM1CLKSEL;                            /* osztó */\
  PIE1bits.TMR1IE = 1;                  /* megszakítás engedélyezés */\
  TMR1 = 65535+TM1COMPCOR-TM1COMP;}     /* timer kezdõérték feltöltése */

// timer1 bekapcsolás
#define Timer1Start()  if(!T1CONbits.TMR1ON){TMR1 = 65535+TM1COMPCOR-TM1COMP; T1CONbits.TMR1ON = 1;}

// timer1 kikapcsolás
#define Timer1Stop()   T1CONbits.TMR1ON = 0

// timer1 megszakítás nyugtázás
#define Timer1IrqAck() PIR1bits.TMR1IF = 0

// Közös megszakítákiszolgáló függvénybõl a timer1 megszakításkiszolgáló függvény hívása
#define Timer1IntProcess(f) if(PIR1bits.TMR1IF) f()

#endif // TIME1CLK

//------------------------------------------------------------------------------
// PIC16 timer2
#ifdef TIME2CLK

// osztó2
#if TIME2DIV < 256
#define TM2CLKDIV      1
#define TM2CLKSEL      T2CONbits.T2CKPS = 0; T2CONbits.TOUTPS = 0
#elif TIME2DIV < (256*2)
#define TM2CLKDIV      2
#define TM2CLKSEL      T2CONbits.T2CKPS = 0; T2CONbits.TOUTPS = 1
#elif TIME2DIV < (256*4)
#define TM2CLKDIV      4
#define TM2CLKSEL      T2CONbits.T2CKPS = 1; T2CONbits.TOUTPS = 0
#elif TIME2DIV < (256*4*2)
#define TM2CLKDIV      8
#define TM2CLKSEL      T2CONbits.T2CKPS = 1; T2CONbits.TOUTPS = 1
#elif TIME2DIV < (256*16*1)
#define TM2CLKDIV      16
#define TM2CLKSEL      T2CONbits.T2CKPS = 2; T2CONbits.TOUTPS = 0
#elif TIME2DIV < (256*16*2)
#define TM2CLKDIV      32
#define TM2CLKSEL      T2CONbits.T2CKPS = 2; T2CONbits.TOUTPS = 1
#elif TIME2DIV < (256*16*4)
#define TM2CLKDIV      64
#define TM2CLKSEL      T2CONbits.T2CKPS = 2; T2CONbits.TOUTPS = 3
#elif TIME2DIV < (256*16*8)
#define TM2CLKDIV      128
#define TM2CLKSEL      T2CONbits.T2CKPS = 2; T2CONbits.TOUTPS = 7
#elif TIME2DIV < (256*16*16)
#define TM2CLKDIV      256
#define TM2CLKSEL      T2CONbits.T2CKPS = 2; T2CONbits.TOUTPS = 15
#else
#error "PIC16: timer2 frekvencia túl alacsony"
#endif // TIME2DIV

// timer2 komparátorba ennyit kell berakni hogy a kívánt frekvencia legyen
#define TM2COMP (((Timer2SrcClock/TM2CLKDIV)+TIME2CLK/2)/TIME2CLK-1)

// mivel van timer komparátor, nem kell szoftverbõl pótolni
#define Timer2Compless() ;

// timer2 inicializálás
#define Timer2Init() {\
  TM2CLKSEL;                            /* osztó */\
  T2CONbits.TMR2ON = 1;                 /* timer engedélyezés */\
  PR2 = TM2COMP;}

// timer2 bekapcsolás (megszakítás engedély be)
#define Timer2Start()  PIE1bits.TMR2IE = 1

// timer2 kikapcsolás (megszakítás engedély ki)
#define Timer2Stop()   PIE1bits.TMR2IE = 0

// timer2 megszakítás nyugtázás
#define Timer2IrqAck() PIR1bits.TMR2IF = 0

// Közös megszakítákiszolgáló függvénybõl a timer2 megszakításkiszolgáló függvény hívása
#define Timer2IntProcess(f) if(PIR1bits.TMR2IF) f()

#endif // TIME2CLK

// periféria IRQ és globális IRQ eng
#define TimersGlobalIrq() {INTCONbits.PEIE = 1; INTCONbits.GIE = 1;}

// PIC16
//==============================================================================
// PIC18
#elif defined(__18CXX)

//------------------------------------------------------------------------------
// PIC18 timer0
#ifdef TIME0CLK

// PIC18: timer IRQ prioritása (0 = alacsony, 1 = magas)
#if TIMER0PR18 == -1
// prioritás nélküli megszakítás
#define TIMER0PR18REG   1
#elif TIMER0PR18 == 0
// alacsony prioritás
#define TIMER0PR18REG   0
#elif TIMER0PR18 == 1
// magas prioritás
#define TIMER0PR18REG   1
#else
#error "LCD TIMER0PR18 csak -1, 0, 1 lehet!"
#endif

// osztó0
#if TIME0DIV < 65536
#define TM0CLKDIV      1
#define TM0CLKSEL      T0CONbits.PSA = 1  /* elõosztó tiltás */
#define TM0COMPCOR     7
#elif TIME0DIV < (65536*2)
#define TM0CLKDIV      2
#define TM0CLKSEL      T0CONbits.PSA = 0; T0CONbits.T0PS2 = 0; T0CONbits.T0PS1 = 0; T0CONbits.T0PS0 = 0  /* elõosztó eng, osztás = 2 */
#define TM0COMPCOR     4
#elif TIME0DIV < (65536*4)
#define TM0CLKDIV      4
#define TM0CLKSEL      T0CONbits.PSA = 0; T0CONbits.T0PS2 = 0; T0CONbits.T0PS1 = 0; T0CONbits.T0PS0 = 1  /* elõosztó eng, osztás = 4 */
#define TM0COMPCOR     2
#elif TIME0DIV < (65536*8)
#define TM0CLKDIV      8
#define TM0CLKSEL      T0CONbits.PSA = 0; T0CONbits.T0PS2 = 0; T0CONbits.T0PS1 = 1; T0CONbits.T0PS0 = 0  /* elõosztó eng, osztás = 8 */
#define TM0COMPCOR     0
#elif TIME0DIV < (65536*16)
#define TM0CLKDIV      16
#define TM0CLKSEL      T0CONbits.PSA = 0; T0CONbits.T0PS2 = 0; T0CONbits.T0PS1 = 1; T0CONbits.T0PS0 = 1  /* elõosztó eng, osztás = 16 */
#define TM0COMPCOR     0
#elif TIME0DIV < (65536*32)
#define TM0CLKDIV      32
#define TM0CLKSEL      T0CONbits.PSA = 0; T0CONbits.T0PS2 = 1; T0CONbits.T0PS1 = 0; T0CONbits.T0PS0 = 0  /* elõosztó eng, osztás = 32 */
#define TM0COMPCOR     0
#elif TIME0DIV < (65536*64)
#define TM0CLKDIV      64
#define TM0CLKSEL      T0CONbits.PSA = 0; T0CONbits.T0PS2 = 1; T0CONbits.T0PS1 = 0; T0CONbits.T0PS0 = 1  /* elõosztó eng, osztás = 64 */
#define TM0COMPCOR     0
#elif TIME0DIV < (65536*128)
#define TM0CLKDIV      128
#define TM0CLKSEL      T0CONbits.PSA = 0; T0CONbits.T0PS2 = 1; T0CONbits.T0PS1 = 1; T0CONbits.T0PS0 = 0  /* elõosztó eng, osztás = 128 */
#define TM0COMPCOR     0
#elif TIME0DIV < (65536*256)
#define TM0CLKDIV      256
#define TM0CLKSEL      T0CONbits.PSA = 0; T0CONbits.T0PS2 = 1; T0CONbits.T0PS1 = 1; T0CONbits.T0PS0 = 1  /* elõosztó eng, osztás = 256 */
#define TM0COMPCOR     0
#else
#error "PIC18: timer0 frekvencia túl alacsony"
#endif // TIME0DIV

// timer0 komparátorba ennyit kell berakni hogy a kívánt frekvencia legyen
#define TM0COMP (((Timer0SrcClock/TM0CLKDIV)+TIME0CLK/2)/TIME0CLK-1)

// timer0 komparátor hiányának pótlásához átmeneti változó
union
{
  struct
  {
    unsigned char LO;
    unsigned char HI;
  };
  unsigned short W;
}MultiTimer0Temp;

// timer komparátor hiányának pótlása
#define Timer0Compless() {                             \
  MultiTimer0Temp.LO = TMR0L;                        \
  MultiTimer0Temp.HI = TMR0H;                        \
  MultiTimer0Temp.W += 65535 + TM0COMPCOR - TM0COMP; \
  TMR0H = MultiTimer0Temp.HI;                        \
  TMR0L = MultiTimer0Temp.LO;}

// timer0 inicializálás
#define Timer0Init() {\
  T0CONbits.T08BIT = 0;                 /* 16bites timer */\
  T0CONbits.T0CS = 0;                   /* forrás órajel = systemclock */\
  TM0CLKSEL;                            /* osztó */\
  INTCON2bits.TMR0IP = TIMER0PR18REG;   /* prioritás */\
  INTCONbits.TMR0IE = 1;                /* megszakítás engedélyezés */\
  TMR0H = (65535+TM0COMPCOR-TM0COMP) >> 8;/* timerH kezdõérték feltöltése bitidõre */\
  TMR0L = 65535+TM0COMPCOR-TM0COMP;}    /* timerL kezdõérték feltöltése bitidõre */

// timer0 bekapcsolás
#define Timer0Start()  T0CONbits.TMR0ON = 1

// timer0 kikapcsolás
#define Timer0Stop()   T0CONbits.TMR0ON = 0
  
// timer0 megszakítás nyugtázás
#define Timer0IrqAck() INTCONbits.TMR0IF = 0

// Közös megszakítákiszolgáló függvénybõl a timer0 megszakításkiszolgáló függvény hívása
#define Timer0IntProcess(f) if(INTCONbits.TMR0IF) f()

#endif // TIME0CLK

//------------------------------------------------------------------------------
// PIC18 timer1
#ifdef TIME1CLK

// PIC18: timer IRQ prioritása (0 = alacsony, 1 = magas)
#if TIMER1PR18 == -1
// prioritás nélküli megszakítás
#define TIMER1PR18REG   1
#elif TIMER1PR18 == 0
// alacsony prioritás
#define TIMER1PR18REG   0
#elif TIMER1PR18 == 1
// magas prioritás
#define TIMER1PR18REG   1
#else
#error "LCD TIMER1PR18 csak -1, 0, 1 lehet!"
#endif

// osztó1
#if TIME1DIV < 65536
#define TM1CLKDIV      1
#define TM1CLKSEL      T1CONbits.T1CKPS1 = 0; T1CONbits.T1CKPS0 = 0 /* elõosztó 1:1 */
#define TM1COMPCOR     5
#elif TIME1DIV < (65536*2)
#define TM1CLKDIV      2
#define TM1CLKSEL      T1CONbits.T1CKPS1 = 0; T1CONbits.T1CKPS0 = 1 /* elõosztó 1:2 */
#define TM1COMPCOR     3
#elif TIME1DIV < (65536*4)
#define TM1CLKDIV      4
#define TM1CLKSEL      T1CONbits.T1CKPS1 = 1; T1CONbits.T1CKPS0 = 0 /* elõosztó 1:4 */
#define TM1COMPCOR     2
#elif TIME1DIV < (65536*8)
#define TM1CLKDIV      8
#define TM1CLKSEL      T1CONbits.T1CKPS1 = 1; T1CONbits.T1CKPS0 = 1 /* elõosztó 1:8 */
#define TM1COMPCOR     1
#else
#error "PIC18: timer1 frekvencia túl alacsony"
#endif // TIME1DIV

// timer1 komparátorba ennyit kell berakni hogy a kívánt frekvencia legyen
#define TM1COMP (((Timer1SrcClock/TM1CLKDIV)+TIME1CLK/2)/TIME1CLK-1)

// timer1 komparátor hiányának pótlásához átmeneti változó
union
{
  struct
  {
    unsigned char LO;
    unsigned char HI;
  };
  unsigned short W;
}MultiTimer1Temp;

// timer komparátor hiányának pótlása
#define Timer1Compless() {                             \
  MultiTimer1Temp.LO = TMR1L;                        \
  MultiTimer1Temp.HI = TMR1H;                        \
  MultiTimer1Temp.W += 65535 + TM1COMPCOR - TM1COMP; \
  TMR1H = MultiTimer1Temp.HI;                        \
  TMR1L = MultiTimer1Temp.LO;}

// timer1 inicializálás
#define Timer1Init() {\
  T1CONbits.RD16 = 1;                   /* 16bites írás/olv */\
  T1CONbits.TMR1CS = 0;                 /* forrás órajel = systemclock */\
  TM1CLKSEL;                            /* osztó */\
  IPR1bits.TMR1IP = TIMER1PR18REG;      /* prioritás */\
  PIE1bits.TMR1IE = 1;                  /* megszakítás engedélyezés */\
  RCONbits.IPEN = 1;                    /* kétszintû prioritású engedélyezése */\
  TMR1H = (65535+TM1COMPCOR-TM1COMP) >> 8;/* timerH kezdõérték feltöltése bitidõre */\
  TMR1L = 65535+TM1COMPCOR-TM1COMP;}    /* timerL kezdõérték feltöltése bitidõre */

// timer1 bekapcsolás
#define Timer1Start()  T1CONbits.TMR1ON = 1

// timer1 kikapcsolás
#define Timer1Stop()   T1CONbits.TMR1ON = 0
  
// timer1 megszakítás nyugtázás
#define Timer1IrqAck() PIR1bits.TMR1IF = 0

// Közös megszakítákiszolgáló függvénybõl a timer1 megszakításkiszolgáló függvény hívása
#define Timer1IntProcess(f) if(PIR1bits.TMR1IF) f()

#endif // TIME1CLK

//------------------------------------------------------------------------------
// PIC18 timer2
#ifdef TIME2CLK

// PIC18: timer IRQ prioritása (0 = alacsony, 1 = magas)
#if TIMER2PR18 == -1
// prioritás nélküli megszakítás
#define TIMER2PR18REG   1
#elif TIMER2PR18 == 0
// alacsony prioritás
#define TIMER2PR18REG   0
#elif TIMER2PR18 == 1
// magas prioritás
#define TIMER2PR18REG   1
#else
#error "LCD TIMER2PR18 csak -1, 0, 1 lehet!"
#endif

// osztó2
#if TIME2DIV < 256
#define TM2CLKDIV      1
#define TM2CLKSEL      T2CONbits.T2CKPS1=0; T2CONbits.T2CKPS0=0; T2CONbits.T2OUTPS3=0; T2CONbits.T2OUTPS2=0; T2CONbits.T2OUTPS1=0; T2CONbits.T2OUTPS0=0
#elif TIME2DIV < (256*2)
#define TM2CLKDIV      2
#define TM2CLKSEL      T2CONbits.T2CKPS1=0; T2CONbits.T2CKPS0=1; T2CONbits.T2OUTPS3=0; T2CONbits.T2OUTPS2=0; T2CONbits.T2OUTPS1=0; T2CONbits.T2OUTPS0=1
#elif TIME2DIV < (256*4)
#define TM2CLKDIV      4
#define TM2CLKSEL      T2CONbits.T2CKPS1=0; T2CONbits.T2CKPS0=1; T2CONbits.T2OUTPS3=0; T2CONbits.T2OUTPS2=0; T2CONbits.T2OUTPS1=0; T2CONbits.T2OUTPS0=0
#elif TIME2DIV < (256*8)
#define TM2CLKDIV      8
#define TM2CLKSEL      T2CONbits.T2CKPS1=0; T2CONbits.T2CKPS0=1; T2CONbits.T2OUTPS3=0; T2CONbits.T2OUTPS2=0; T2CONbits.T2OUTPS1=0; T2CONbits.T2OUTPS0=1
#elif TIME2DIV < (256*16)
#define TM2CLKDIV      16
#define TM2CLKSEL      T2CONbits.T2CKPS1=1; T2CONbits.T2CKPS0=0; T2CONbits.T2OUTPS3=0; T2CONbits.T2OUTPS2=0; T2CONbits.T2OUTPS1=0; T2CONbits.T2OUTPS0=0
#elif TIME2DIV < (256*16*2)
#define TM2CLKDIV      32
#define TM2CLKSEL      T2CONbits.T2CKPS1=1; T2CONbits.T2CKPS0=0; T2CONbits.T2OUTPS3=0; T2CONbits.T2OUTPS2=0; T2CONbits.T2OUTPS1=0; T2CONbits.T2OUTPS0=1
#elif TIME2DIV < (256*16*4)
#define TM2CLKDIV      64
#define TM2CLKSEL      T2CONbits.T2CKPS1=1; T2CONbits.T2CKPS0=0; T2CONbits.T2OUTPS3=0; T2CONbits.T2OUTPS2=0; T2CONbits.T2OUTPS1=1; T2CONbits.T2OUTPS0=1
#elif TIME2DIV < (256*16*8)
#define TM2CLKDIV      128
#define TM2CLKSEL      T2CONbits.T2CKPS1=1; T2CONbits.T2CKPS0=0; T2CONbits.T2OUTPS3=0; T2CONbits.T2OUTPS2=1; T2CONbits.T2OUTPS1=1; T2CONbits.T2OUTPS0=1
#elif TIME2DIV < (256*16*16)
#define TM2CLKDIV      256
#define TM2CLKSEL      T2CONbits.T2CKPS1=1; T2CONbits.T2CKPS0=0; T2CONbits.T2OUTPS3=1; T2CONbits.T2OUTPS2=1; T2CONbits.T2OUTPS1=1; T2CONbits.T2OUTPS0=1
#else
#error "PIC18: timer frekvencia túl alacsony"
#endif // TIME2DIV

// timer2 komparátorba ennyit kell berakni hogy a kívánt frekvencia legyen
#define TM2COMP (((Timer2SrcClock/TM2CLKDIV)+TIME2CLK/2)/TIME2CLK-1)

// mivel van timer komparátor, nem kell szoftverbõl pótolni
#define Timer2Compless() ;

// timer2 inicializálás
#define Timer2Init() {\
  TM2CLKSEL;                            /* osztó */\
  PR2 = TM2COMP;                        /* komparátor */\
  IPR1bits.TMR2IP = TIMER2PR18REG;      /* prioritás */\
  T2CONbits.TMR2ON = 1;                 /* timer engedélyezés */\
  RCONbits.IPEN = 1;}                   /* kétszintû prioritású engedélyezése */

// timer2 bekapcsolás (megszakítás engedély be)
#define Timer2Start()  PIE1bits.TMR2IE = 1

// timer2 bekapcsolás (megszakítás engedély ki)
#define Timer2Stop()   PIE1bits.TMR2IE = 0
  
// timer2 megszakítás nyugtázás
#define Timer2IrqAck() PIR1bits.TMR2IF = 0

// Közös megszakítákiszolgáló függvénybõl a timer2 megszakításkiszolgáló függvény hívása
#define Timer2IntProcess(f) if(PIR1bits.TMR2IF) f()

#endif // TIME2CLK

//------------------------------------------------------------------------------
// PIC18 timer3
#ifdef TIME3CLK

// PIC18: timer IRQ prioritása (0 = alacsony, 1 = magas)
#if TIMER3PR18 == -1
// prioritás nélküli megszakítás
#define TIMER3PR18REG   1
#elif TIMER3PR18 == 0
// alacsony prioritás
#define TIMER3PR18REG   0
#elif TIMER3PR18 == 1
// magas prioritás
#define TIMER3PR18REG   1
#else
#error "LCD TIMER3PR18 csak -1, 0, 1 lehet!"
#endif

// osztó3
#if TIME3DIV < 65536
#define TM3CLKDIV      1
#define TM3CLKSEL      T3CONbits.T3CKPS0 = 0; T3CONbits.T3CKPS1 = 0 /* elõosztó 1:1 */
#define TM3COMPCOR     5
#elif TIME3DIV < (65536*2)
#define TM3CLKDIV      2
#define TM3CLKSEL      T3CONbits.T3CKPS0 = 0; T3CONbits.T3CKPS1 = 1 /* elõosztó 1:2 */
#define TM3COMPCOR     3
#elif TIME3DIV < (65536*4)
#define TM3CLKDIV      4
#define TM3CLKSEL      T3CONbits.T3CKPS0 = 1; T3CONbits.T3CKPS1 = 0 /* elõosztó 1:4 */
#define TM3COMPCOR     2
#elif TIME3DIV < (65536*8)
#define TM3CLKDIV      8
#define TM3CLKSEL      T3CONbits.T3CKPS0 = 1; T3CONbits.T3CKPS1 = 1 /* elõosztó 1:8 */
#define TM3COMPCOR     1
#else
#error "PIC18: timer3 frekvencia túl alacsony"
#endif // TIME3DIV

// timer3 komparátorba ennyit kell berakni hogy a kívánt frekvencia legyen
#define TM3COMP (((Timer3SrcClock/TM3CLKDIV)+TIME3CLK/2)/TIME3CLK-1)

// timer komparátor hiányának pótlásához átmeneti változó
union
{
  struct
  {
    unsigned char LO;
    unsigned char HI;
  };
  unsigned short W;
}MultiTimer3Temp;

// timer komparátor hiányának pótlása
#define Timer3Compless() {                             \
  MultiTimer3Temp.LO = TMR3L;                        \
  MultiTimer3Temp.HI = TMR3H;                        \
  MultiTimer3Temp.W += 65535 + TM3COMPCOR - TM3COMP; \
  TMR3H = MultiTimer3Temp.HI;                        \
  TMR3L = MultiTimer3Temp.LO;}

// timer3 inicializálás
#define Timer3Init() {\
  T3CONbits.RD16 = 1;                   /* 16bites írás/olv */\
  T3CONbits.TMR3CS = 0;                 /* forrás órajel = systemclock */\
  TM3CLKSEL;                            /* osztó */\
  IPR2bits.TMR3IP = TIMER3PR18REG;      /* prioritás */\
  PIE2bits.TMR3IE = 1;                  /* megszakítás engedélyezés */\
  TMR3H = (65535+TM3COMPCOR-TM3COMP) >> 8;/* timerH kezdõérték feltöltése bitidõre */\
  TMR3L = 65535+TM3COMPCOR-TM3COMP;}    /* timerL kezdõérték feltöltése bitidõre */

// timer3 bekapcsolás
#define TIMERSTART  T3CONbits.TMR3ON = 1

// timer3 kikapcsolás
#define TIMERSTOP   T3CONbits.TMR3ON = 0
  
// timer3 megszakítás nyugtázás
#define TIMERIRQACK PIR2bits.TMR3IF = 0

// Közös megszakítákiszolgáló függvénybõl a timer3 megszakításkiszolgáló függvény hívása
#define Timer3IntProcess(f) if(PIR2bits.TMR3IF) f()

#endif // TIME3CLK

 // prioritás beállítás + globális IRQ eng
#if (TIMER0PR18 != -1) || (TIMER1PR18 != -1) || (TIMER2PR18 != -1) || (TIMER3PR18 != -1)
#define TimersGlobalIrq()  {RCONbits.IPEN = 1; INTCONbits.GIEL = 1; INTCONbits.GIEH = 1}
#else
#define TimersGlobalIrq()  {RCONbits.IPEN = 0; INTCONbits.GIE = 1; INTCONbits.PEIE = 1}
#endif

// PIC18
//==============================================================================
// PIC24
#elif defined(__C30__)

//------------------------------------------------------------------------------
// PIC24 timer1
#ifdef TIME1CLK

// osztó1
#if TIME1DIV < 65536
#define TM1CLKDIV      1
#define TM1CLKSEL      T1CONbits.TCKPS = 0
#elif TIME1DIV < (65536*8)
#define TM1CLKDIV      8
#define TM1CLKSEL      T1CONbits.TCKPS = 1
#elif TIME1DIV < (65536*64)
#define TM1CLKDIV      64
#define TM1CLKSEL      T1CONbits.TCKPS = 2
#elif TIME1DIV < (65536*256)
#define TM1CLKDIV      256
#define TM1CLKSEL      T1CONbits.TCKPS = 3
#else
#error "PIC24: timer1 frekvencia túl alacsony"
#endif  // TIME1DIV

// timer1 komparátorba ennyit kell berakni hogy a kívánt frekvencia legyen
#define TM1COMP (((Timer1SrcClock/TM1CLKDIV)+TIME1CLK/2)/TIME1CLK-1)

// mivel van timer komparátor, nem kell szoftverbõl pótolni
#define Timer1Compless() ;

// timer1 inicializálás
#define Timer1Init() {\
  T1CONbits.TCS = 0;                    /* forrás órajel = systemclock */\
  TM1CLKSEL;                            /* elõosztó */\
  IPC0bits.T1IP = TIMER1PR24;           /* prioritás */\
  PR1 = TM1COMP;                        /* idõzítõ beállítása a bitidõhöz */\
  T1CONbits.TON = 1;}                   /* timer engedélyezés */

// timer1 bekapcsolás (megszakítás engedély be)
#define Timer1Start()  IEC0bits.T1IE = 1

// timer bekapcsolás (megszakítás engedély ki)
#define Timer1Stop()   IEC0bits.T1IE = 0
  
// timer megszakítás nyugtázás
#define Timer1IrqAck() IFS0bits.T1IF = 0

// timer megszakítás kiszolgáló függvény
#define Timer1Int() void __attribute__ ((interrupt, auto_psv)) _T1Interrupt(void)

#endif // TIME1CLK

//------------------------------------------------------------------------------
// PIC24 timer2
#ifdef TIME2CLK

// osztó2
#if TIME2DIV < 65536
#define TM2CLKDIV      1
#define TM2CLKSEL      T2CONbits.TCKPS = 0
#elif TIME2DIV < (65536*8)
#define TM2CLKDIV      8
#define TM2CLKSEL      T2CONbits.TCKPS = 1
#elif TIME2DIV < (65536*64)
#define TM2CLKDIV      64
#define TM2CLKSEL      T2CONbits.TCKPS = 2
#elif TIME2DIV < (65536*256)
#define TM2CLKDIV      256
#define TM2CLKSEL      T2CONbits.TCKPS = 3
#else
#error "PIC24: timer2 frekvencia túl alacsony"
#endif  // TIME2DIV

// timer2 komparátorba ennyit kell berakni hogy a kívánt frekvencia legyen
#define TM2COMP (((Timer2SrcClock/TM2CLKDIV)+TIME2CLK/2)/TIME2CLK-1)

// timer2 inicializálás
#define Timer2Init() {\
  T2CONbits.TCS = 0;                    /* forrás órajel = systemclock */\
  TM2CLKSEL;                            /* osztó */\
  IPC1bits.T2IP = TIMER2PR24;           /* prioritás */\
  PR2 = TM2COMP;                        /* idõzítõ beállítása a bitidõhöz */\
  T2CONbits.TON = 1;}                   /* timer engedélyezés */

// timer2 bekapcsolás (megszakítás engedély be)
#define Timer2Start()  IEC0bits.T2IE = 1

// timer2 bekapcsolás (megszakítás engedély be)
#define Timer2Stop()   IEC0bits.T2IE = 0
  
// timer2 megszakítás nyugtázás
#define Timer2IrqAck() IFS0bits.T2IF = 0

// timer2 megszakítás kiszolgáló függvény
#define Timer2Int() void __attribute__ ((interrupt, auto_psv)) _T2Interrupt(void)

#endif // TIME2CLK

//------------------------------------------------------------------------------
// PIC24 timer3
#ifdef TIME3CLK

// osztó3
#if TIME3DIV < 65536
#define TM3CLKDIV      1
#define TM3CLKSEL      T3CONbits.TCKPS = 0
#elif TIME3DIV < (65536*8)
#define TM3CLKDIV      8
#define TM3CLKSEL      T3CONbits.TCKPS = 1
#elif TIME3DIV < (65536*64)
#define TM3CLKDIV      64
#define TM3CLKSEL      T3CONbits.TCKPS = 2
#elif TIME3DIV < (65536*256)
#define TM3CLKDIV      256
#define TM3CLKSEL      T3CONbits.TCKPS = 3
#else
#error "PIC24: timer3 frekvencia túl alacsony"
#endif  // TIME3DIV

// timer3 komparátorba ennyit kell berakni hogy a kívánt frekvencia legyen
#define TM3COMP (((Timer3SrcClock/TM3CLKDIV)+TIME3CLK/2)/TIME3CLK-1)

// timer3 inicializálás
#if defined(__dsPIC30F__)
#define TIME3RINIT {\
  T3CONbits.TCS = 0;                    /* forrás órajel = systemclock */\
  TM3CLKSEL;                            /* osztó */\
  IPC1bits.T3IP = TIMER3PR24;           /* prioritás */\
  PR3 = TM3COMP;                        /* idõzítõ beállítása a bitidõhöz */\
  T3CONbits.TON = 1;}                   /* timer engedélyezés */
#else
#define TIME3RINIT {\
  T3CONbits.TCS = 0;                    /* forrás órajel = systemclock */\
  TM3CLKSEL;                            /* osztó */\
  IPC2bits.T3IP = TIMER3PR24;           /* prioritás */\
  PR3 = TM3COMP;                        /* idõzítõ beállítása a bitidõhöz */\
  T3CONbits.TON = 1;}                   /* timer engedélyezés */
#endif

// timer3 bekapcsolás (megszakítás engedély be)
#define Timer3Start()  IEC0bits.T3IE = 1

// timer3 bekapcsolás (megszakítás engedély be)
#define Timer3Stop()   IEC0bits.T3IE = 0
  
// timer3 megszakítás nyugtázás
#define Timer3IrqAck() IFS0bits.T3IF = 0

// timer3 megszakítás kiszolgáló függvény
#define Timer3Int() void __attribute__ ((interrupt, auto_psv)) _T3Interrupt(void)

#endif // TIME3CLK

//------------------------------------------------------------------------------
// PIC24 timer4
#ifdef TIME4CLK

// osztó4
#if TIME4DIV < 65536
#define TM4CLKDIV      1
#define TM4CLKSEL      T4CONbits.TCKPS = 0
#elif TIME4DIV < (65536*8)
#define TM4CLKDIV      8
#define TM4CLKSEL      T4CONbits.TCKPS = 1
#elif TIME4DIV < (65536*64)
#define TM4CLKDIV      64
#define TM4CLKSEL      T4CONbits.TCKPS = 2
#elif TIME4DIV < (65536*256)
#define TM4CLKDIV      256
#define TM4CLKSEL      T4CONbits.TCKPS = 3
#else
#error "PIC24: timer4 frekvencia túl alacsony"
#endif  // TIME4DIV

// timer4 komparátorba ennyit kell berakni hogy a kívánt frekvencia legyen
#define TM4COMP (((Timer4SrcClock/TM4CLKDIV)+TIME4CLK/2)/TIME4CLK-1)

// timer4 inicializálás
#if defined(__dsPIC30F__)
#define Timer4Init() {\
  T4CONbits.TCS = 0;                    /* forrás órajel = systemclock */\
  TM4CLKSEL;                            /* osztó */\
  IPC5bits.T4IP = TIMER4PR24;           /* prioritás */\
  PR4 = TM4COMP;                        /* idõzítõ beállítása a bitidõhöz */\
  T4CONbits.TON = 1;}                   /* timer engedélyezés */
#else
#define Timer4Init() {\
  T4CONbits.TCS = 0;                    /* forrás órajel = systemclock */\
  TM4CLKSEL;                            /* osztó */\
  IPC6bits.T4IP = TIMER4PR24;           /* prioritás */\
  PR4 = TM4COMP;                        /* idõzítõ beállítása a bitidõhöz */\
  T4CONbits.TON = 1;}                   /* timer engedélyezés */
#endif

// timer4 bekapcsolás (megszakítás engedély be)
#define Timer4Start()  IEC1bits.T4IE = 1

// timer4 bekapcsolás (megszakítás engedély be)
#define Timer4Stop()   IEC1bits.T4IE = 0
  
// timer4 megszakítás nyugtázás
#define Timer4IrqAck() IFS1bits.T4IF = 0

// timer4 megszakítás kiszolgáló függvény
#define Timer4Int() void __attribute__ ((interrupt, auto_psv)) _T4Interrupt(void)

#endif // TIME4CLK

//------------------------------------------------------------------------------
// PIC24 timer5
#ifdef TIME5CLK

// osztó5
#if TIME5DIV < 65536
#define TM5CLKDIV      1
#define TM5CLKSEL      T5CONbits.TCKPS = 0
#elif TIME5DIV < (65536*8)
#define TM5CLKDIV      8
#define TM5CLKSEL      T5CONbits.TCKPS = 1
#elif TIME5DIV < (65536*64)
#define TM5CLKDIV      64
#define TM5CLKSEL      T5CONbits.TCKPS = 2
#elif TIME5DIV < (65536*256)
#define TM5CLKDIV      256
#define TM5CLKSEL      T5CONbits.TCKPS = 3
#else
#error "PIC24: timer5 frekvencia túl alacsony"
#endif  // TIME5DIV

// timer5 komparátorba ennyit kell berakni hogy a kívánt frekvencia legyen
#define TM5COMP (((Timer5SrcClock/TM5CLKDIV)+TIME5CLK/2)/TIME5CLK-1)

// timer5 inicializálás
#if defined(__dsPIC30F__)
#define Timer5Init() {\
  T5CONbits.TCS = 0;                    /* forrás órajel = systemclock */\
  TM5CLKSEL;                            /* elõosztó */\
  IPC5bits.T5IP = TIMER5PR24;           /* prioritás */\
  PR5 = TM5COMP;                        /* idõzítõ beállítása a bitidõhöz */\
  T5CONbits.TON = 1;}                   /* timer engedélyezés */
#else
#define Timer5Init() {\
  T5CONbits.TCS = 0;                    /* forrás órajel = systemclock */\
  TM5CLKSEL;                            /* elõosztó */\
  IPC7bits.T5IP = TIMER5PR24;           /* prioritás */\
  PR5 = TM5COMP;                        /* idõzítõ beállítása a bitidõhöz */\
  T5CONbits.TON = 1;}                   /* timer engedélyezés */
#endif

// timer5 bekapcsolás (megszakítás engedély be)
#define Timer5Start()  IEC1bits.T5IE = 1

// timer5 bekapcsolás (megszakítás engedély be)
#define Timer5Stop()   IEC1bits.T5IE = 0
  
// timer5 megszakítás nyugtázás
#define Timer5IrqAck() IFS1bits.T5IF = 0

// timer5 megszakítás kiszolgáló függvény
#define Timer5Int() void __attribute__ ((interrupt, auto_psv)) _T5Interrupt(void)

#endif // TIME5CLK

#endif // defined(__C30__)

//==============================================================================
// timer (vége)
//==============================================================================

#endif // __MULTITIMER_H