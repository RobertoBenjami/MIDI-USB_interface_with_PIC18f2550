// Roberto Benjami : MIDI <-> USB átalakíto
// Compiler: Microchip C18

#include <p18cxxx.h>
#include "USB\usb.h"
#include "usb_config.h"
#include "usb_function_midi.h"
#include "hardwareprofile.h"
#include "bootloaders.h"
#include "uart.h"
#include "multitimer.h"

#define  CABLENUM  0

#pragma udata USB_VARIABLES = USBBUFFERADDR

unsigned char UsbRecBuffer1[64];        // USB -> MIDI
unsigned char UsbRecBuffer2[64];        // USB -> MIDI

USB_AUDIO_MIDI_EVENT_PACKET MidiData;   // 4 bájtos tömb (csatorna üzeneteknek)
USB_AUDIO_MIDI_EVENT_PACKET CommonMidiData;// 4 bájtos tömb (rendszerüzeneteknek)

#pragma udata

// unsigned char MidiRecBuffer[64];     // USB -> MIDI csomag másolat

USB_HANDLE USBTxHandle = 0;
USB_HANDLE USBRxHandle = 0;

USB_VOLATILE BYTE msCounter;

volatile unsigned char ActSenzTime = 0;

#ifdef   __18F14K50_H
rom char progID[] = {"USB-Midi Interface PIC18F14K50 (by Roberto Benjami v2016.01.16)"};
#endif
#ifdef   __18F2550_H
rom char progID[] = {"USB-Midi Interface PIC18F2550 (by Roberto Benjami v2016.01.16)"};
#endif


/** PRIVATE PROTOTYPES *********************************************/
void InitializeSystem(void);
void UsbToMidiProcess(void);
void MidiToUsbProcess(void);
void YourHighPriorityISRCode();
void YourLowPriorityISRCode();
void USBCBSendResume(void);

#define TRIS_(p, m)            TRIS ## p ## bits.TRIS ## p ## m
#define LAT_(p, m)             LAT ## p ## bits.LAT ## p ## m
#define PORT_(p, m)            PORT ## p ## bits.R ## p ## m

#define IOIN(x)                x(TRIS_) = 1
#define IOOUT(x)               x(TRIS_) = 0
#define SET(x)                 x(LAT_) = 1
#define CLR(x)                 x(LAT_) = 0
#define GET(x)                 x(PORT_)

#pragma code

/******************************************************************************
 * Function:                                     void YourHighPriorityISRCode()
 *****************************************************************************/
#pragma interrupt YourHighPriorityISRCode
void YourHighPriorityISRCode()
{
  #if defined(USB_INTERRUPT)
  USBDeviceTasks();
  #endif

  UartRxIntProcess();
  UartTxIntProcess();

  if(INTCONbits.TMR0IF)
  {
    Timer0Compless();
    Timer0IrqAck();
    ActSenzTime = 1;
  }
} //This return will be a "retfie fast", since this is in a #pragma interrupt section

/******************************************************************************
 * Function:                                           YourLowPriorityISRCode()
 *****************************************************************************/
#pragma interruptlow YourLowPriorityISRCode
void YourLowPriorityISRCode()
{
} //This return will be a "retfie", since this is in a #pragma interruptlow section

/******************************************************************************
 * Function:                                                    void main(void)
 *****************************************************************************/
void main(void)
{
  InitializeSystem();

  #if defined(USB_INTERRUPT)
  USBDeviceAttach();
  #endif

  // #include "debugtestdata.h"

  while(1)
  {
    #if defined(USB_POLLING)
    // Check bus status and service USB interrupts.
    USBDeviceTasks(); // Interrupt or polling method.  If using polling, must call
    #endif

    // User Application USB tasks
    if((USBDeviceState == CONFIGURED_STATE) && (!USBSuspendControl))
    { // USB üzemben van
      SET(LEDUSB);
      UsbToMidiProcess();
      MidiToUsbProcess();
    }
    else
      CLR(LEDUSB);

  }//end while
}//end main


/******************************************************************************
 * Function:                                       void InitializeSystem(void)
 *****************************************************************************/
void InitializeSystem(void)
{
  ADINIT;                               // A/D beállítás

  CLR(LEDUSB); CLR(LEDIN); CLR(LEDOUT);
  IOOUT(LEDUSB); IOOUT(LEDIN); IOOUT(LEDOUT);
  IOIN(BOOTLOADER);

  UartInit();

  USBDeviceInit(); // usb_device.c.  Initializes USB module SFRs and firmware variables to known states.

  Timer0Init();

  INTCONbits.GIE = 1; INTCONbits.PEIE = 1;
}//end InitializeSystem

//*******************************************************************
void UsbToMidiProcess(void)
{
  // csomag index szám                     0, 1, 2, 3, 4, 5, 6, 7, 8, 9, A, B, C, D, E, F
  static rom unsigned char CinToLengt[] = {0, 0, 2, 3, 3, 1, 2, 3, 3, 3, 3, 3, 2, 2, 3, 1}; // üzenet hosszak
  static unsigned char packcount = 0;        // ennyi adatcsomag érkezett eddig az USB-n *******
  static unsigned char packetcounter = 0;    // csomagon belüli bájtszámláló
  static unsigned char packetsize = 0;       // aktuális csomag mérete
  static unsigned char* packetpointer = (char *)&UsbRecBuffer1; // mutató a csomag olvasásához

  static unsigned char i = 0;           // MIDI üzenet bájt számláló (0..3)
  unsigned char c;                      // aktuális MIDI bájt

  if(!packetsize)
  { // csomagméret == 0

    // Active senzing küldése
    if(ActSenzTime && uTxBufFree)
    {
      UartTx(0xFE);
      ActSenzTime = 0;
    }

    CLR(LEDOUT);
    if(!USBHandleBusy(USBRxHandle))
    { // USB nem foglalt
      SET(LEDOUT);
      packetsize = USBHandleGetLength(USBRxHandle);
      packetpointer = USBHandleGetAddr(USBRxHandle);

      packcount++;
      packetcounter = 0;

      if(packetpointer == UsbRecBuffer1)
        USBRxHandle = USBRxOnePacket(MIDI_EP, (BYTE*)&UsbRecBuffer2, 64);
      else
        USBRxHandle = USBRxOnePacket(MIDI_EP, (BYTE*)&UsbRecBuffer1, 64);
    }
  }
  else
  { // van küldeni való csomag

    // ------------------------- adatcsomag elküldése MIDI OUT-ra
    if(uTxBufFree)                      // lehet irni az UART kimenetre
    {
      c = *packetpointer;
      if(!(packetcounter & 0x03))       // 0, 4, 8, ... 60. bájt a csomag
      { // csomag azonosító (CIN, CN)
        i = CinToLengt[c & 0x0F];       // hossz táblázat
      }
      else
      { // adat bájtok
        if(i)
        { // ------------------------- még tart a csomag
//          spiDebugPutChar(c);
          UartTx(c);
          i--;
        }
      }

      packetpointer++;
      if(++packetcounter == packetsize)
        packetsize = 0;
    }
  }
}

//********************************************************************
void MidiToUsbProcess(void)
{
  // index                                         8, 9, A, B, C, D, E, F
  static rom unsigned char MidiMsgChnToLen[]    = {3, 3, 3, 3, 2, 2, 3, 0};

  // Midi adat, USB Midi csomag legels? adatbájtja
  union
  {
    struct
    {
      unsigned chn:  4;
      unsigned cmd:  3;
      unsigned st:   1;
    };
    unsigned char chr;
  }ch;

  // Midi forrás jelzések
  static union
  {
    struct
    {
      unsigned work:   1;
      unsigned midiin: 1;
      unsigned sysex:  1;
    };
    unsigned char ch;
  }Source = 0;

  // static unsigned char cnt = 0;
  static unsigned char MidiStatus = 0;
  static unsigned char MidiMsgLen = 0;
  static unsigned char i = 0;

  // ha USB módban az USB foglalt akkor kilépünk
  if(USBHandleBusy(USBTxHandle))
    return;

  // Midi forrás olvasása
  if(!uRxLength)
  {
    CLR(LEDIN);
    return;                             // nem jött MIDI adat -> nincs mit feldolgozni
  }

  SET(LEDIN);
  ch.chr = UartRx();
  
  // spiDebugPutHex8(ch.chr);
  if(ch.st)
  { // státuszbájt érkezett ( >=0x80 )
    if(ch.chr < 0xF0)
    { // ------------------------- Channel Voice Messages (0x80..0xEF)
      Source.sysex = 0;
      MidiStatus = ch.chr;
      MidiData.v[0] = (ch.chr >> 4) + (CABLENUM << 4); // CIN
      MidiData.v[1] = ch.chr;
      MidiMsgLen = MidiMsgChnToLen[(ch.chr >> 4) & 0x07];
      i = 1;
    }
    else
    { // Egyszerü rendszerüzenetek (0xF0..0xF7) és valós idejü rendszerüzenetek (0xF8..0xFF)
      if(ch.chr < 0xF8)
      { // Egyszerü rendszerüzenetek (0xF0..0xF7) státusztartás törlés
        MidiStatus = ch.chr;
      }
      if(ch.chr == 0xF0)
      { // --------------------------------------------------------- SYSEX start
        Source.sysex = 1;
        MidiData.v[0] = MIDI_CIN_SYSEX_START + i + (CABLENUM << 4);
        MidiData.v[1] = 0xF0;
        MidiMsgLen = 3;
        i = 1;
      }
      else if(ch.chr == 0xF7)
      { // ---------------------------------------------------------- SYSEX stop
        MidiStatus = 0;
        Source.sysex = 0;
        MidiData.v[0] = MIDI_CIN_SYSEX_ENDS_1 + i + (CABLENUM << 4);
        i++;
        MidiData.v[i] = 0xF7;
        USBTxHandle = USBTxOnePacket(MIDI_EP, (BYTE*)&MidiData, 4);
        MidiMsgLen = 0;
        MidiStatus = 0;
        Source.work = 0;
        i = 0;
      }
      else if(ch.chr == 0xF1)
      { // --------------------------------------------------- MTC Quarter-Frame
        MidiMsgLen = 2;
        i = 1;
      }
      else if(ch.chr == 0xF2)
      { // ----------------------------------------------- Song Position Pointer
        MidiMsgLen = 3;
        i = 1;
      }
      else if(ch.chr == 0xF3)
      { // --------------------------------------------------------- Song Select
        MidiMsgLen = 2;
        i = 1;
      }
      else
      { //----------------------------------------------------------- 0xF4..0xFF
        CommonMidiData.v[0] = 5 + (CABLENUM << 4); // CIN
        CommonMidiData.v[1] = ch.chr;
        CommonMidiData.v[2] = 0;
        CommonMidiData.v[3] = 0;
        USBTxHandle = USBTxOnePacket(MIDI_EP, (BYTE*)&CommonMidiData, 4);
        Source.work = 0;
        if(ch.chr < 0xF8)
        {
          MidiStatus = 0;
      }
      }
    } // Egyszerü rendszerüzenetek (0xF0..0xF7)
  }
  else
  { // ------------------------- adatbájt érkezett ( <0x80 )
    if(!MidiStatus)
    { // státusz nélküli adatbájt
      Source.work = 0;
      return;
    }
    i++;
    MidiData.v[i] = ch.chr;

    if(i >= MidiMsgLen)
    { // üzenet vége van
      USBTxHandle = USBTxOnePacket(MIDI_EP, (BYTE*)&MidiData, 4);

      // üzenet vége
      Source.work = 0;
      i = 0;

      if(Source.sysex)
        Source.work = 1;  // Sysex esetén a munkafolyamat csak sysex end-re ér véget
      else
        i++;

      if(MidiStatus >= 0xF1)
        MidiStatus = 0;   // Egyszerü több bájtos rendszerüzeneteknek nincs státusztartása
    } // if(i >= MidiMsgLen)
  }
}

// ******************************************************************************************************
// ************** USB Callback Functions ****************************************************************
// ******************************************************************************************************

/******************************************************************************
 * Function:        void USBCBSuspend(void)
 *****************************************************************************/
void USBCBSuspend(void)
{
}


/******************************************************************************
 * Function:        void _USB1Interrupt(void)
 *****************************************************************************/
#if 0
void __attribute__ ((interrupt)) _USB1Interrupt(void)
{
  #if !defined(self_powered)
  if(U1OTGIRbits.ACTVIF)
  {
    IEC5bits.USB1IE = 0;
    U1OTGIEbits.ACTVIE = 0;
    IFS5bits.USB1IF = 0;

    //USBClearInterruptFlag(USBActivityIFReg,USBActivityIFBitNum);
    USBClearInterruptFlag(USBIdleIFReg,USBIdleIFBitNum);
    //USBSuspendControl = 0;
  }
  #endif
}
#endif

/******************************************************************************
 * Function:        void USBCBWakeFromSuspend(void)
 *****************************************************************************/
void USBCBWakeFromSuspend(void)
{
}

/********************************************************************
 * Function:        void USBCB_SOF_Handler(void)
 *******************************************************************/
void USBCB_SOF_Handler(void)
{
  if(msCounter != 0)
  {
    msCounter--;
  }
}

/*******************************************************************
 * Function:        void USBCBErrorHandler(void)
 *******************************************************************/
void USBCBErrorHandler(void)
{
}

/*******************************************************************
 * Function:        void USBCBCheckOtherReq(void)
 *******************************************************************/
void USBCBCheckOtherReq(void)
{
}//end


/*******************************************************************
 * Function:        void USBCBStdSetDscHandler(void)
 *******************************************************************/
void USBCBStdSetDscHandler(void)
{
  // Must claim session ownership if supporting this request
}//end


/*******************************************************************
 * Function:        void USBCBInitEP(void)
 *******************************************************************/
void USBCBInitEP(void)
{
  //enable the HID endpoint
  USBEnableEndpoint(MIDI_EP,USB_OUT_ENABLED | USB_IN_ENABLED | USB_HANDSHAKE_ENABLED | USB_DISALLOW_SETUP);

  //Re-arm the OUT endpoint for the next packet
  USBRxHandle = USBRxOnePacket(MIDI_EP, (BYTE*)&UsbRecBuffer1, 64);
}

/********************************************************************
 * Function:        void USBCBSendResume(void)
 *******************************************************************/
void USBCBSendResume(void)
{
  static WORD delay_count;
    
  if(USBGetRemoteWakeupStatus() == TRUE) 
  {
    if(USBIsBusSuspended() == TRUE)
    {
      USBMaskInterrupts();
            
      //Clock switch to settings consistent with normal USB operation.
      USBCBWakeFromSuspend();
      USBSuspendControl = 0; 
      USBBusIsSuspended = FALSE;  //So we don't execute this code again, 
                                  //until a new suspend condition is detected.

      delay_count = 3600U;        
      do
      {
        delay_count--;
      }while(delay_count);
            
      //Now drive the resume K-state signalling onto the USB bus.
      USBResumeControl = 1;       // Start RESUME signaling
      delay_count = 1800U;        // Set RESUME line for 1-13 ms
      do
      {
        delay_count--;
      }while(delay_count);
      USBResumeControl = 0;       //Finished driving resume signalling

      USBUnmaskInterrupts();
    }
  }
}


/*******************************************************************
 * Function:        BOOL USER_USB_CALLBACK_EVENT_HANDLER(
 *                        USB_EVENT event, void *pdata, WORD size)
 *******************************************************************/
BOOL USER_USB_CALLBACK_EVENT_HANDLER(USB_EVENT event, void *pdata, WORD size)
{
  switch(event)
  {
    case EVENT_TRANSFER:
            //Add application specific callback task or callback function here if desired.
      break;
    case EVENT_SOF:
      USBCB_SOF_Handler();
      break;
    case EVENT_SUSPEND:
      USBCBSuspend();
      break;
    case EVENT_RESUME:
      USBCBWakeFromSuspend();
      break;
    case EVENT_CONFIGURED:
      USBCBInitEP();
      break;
    case EVENT_SET_DESCRIPTOR:
      USBCBStdSetDscHandler();
      break;
    case EVENT_EP0_REQUEST:
      USBCBCheckOtherReq();
      break;
    case EVENT_BUS_ERROR:
      USBCBErrorHandler();
      break;
    case EVENT_TRANSFER_TERMINATED:
            //Add application specific callback task or callback function here if desired.
            //The EVENT_TRANSFER_TERMINATED event occurs when the host performs a CLEAR
            //FEATURE (endpoint halt) request on an application endpoint which was 
            //previously armed (UOWN was = 1).  Here would be a good place to:
            //1.  Determine which endpoint the transaction that just got terminated was 
            //      on, by checking the handle value in the *pdata.
            //2.  Re-arm the endpoint if desired (typically would be the case for OUT 
            //      endpoints).
      break;
    default:
      break;
  }
  return TRUE;
}
