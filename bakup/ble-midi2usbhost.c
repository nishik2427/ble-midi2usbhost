/**

*/
#include <inttypes.h>
#include <stdio.h>
#include "midi_service_stream_handler.h"
#include "pico/stdlib.h"
#include "ble-midi2usbhost.h"
#define _TUSB_HID_H_ // prevent tinyusb HID namespace conflicts with btstack
#include "bsp/board_api.h"
#include "tusb.h"
#include "usb_midi_host.h"
#include "pico/multicore.h"
#include "hardware/clocks.h"
#include "hardware/vreg.h"
#include "hardware/uart.h"

#include "M5UnitSynth.h"


// システムクロック周波数指定 
#define CLK_SYS	((uint32_t)208800000)
//#define CLK_SYS	((uint32_t)250000000)

//UART1 define
#define UART_ID uart1
#define BAUD_RATE 31250
#define DATA_BITS 8
#define STOP_BITS 1
#define PARITY    UART_PARITY_NONE
#define UART_TX_PIN 4
#define UART_RX_PIN 5

static uint8_t midi_dev_addr = 0;
int EWI = 0;


bool print_midi_event(const uint8_t msg[3]) {
    int ch;
    int event;
    int cont_num;

    ch = msg[0] & 0xf;
    event = (msg[0] >> 4) & 0xf;
    if (event != 0xf) {
	    printf("Ch: %02d | ", ch);
    } else {
	    printf("System | ");
    }

    switch (event) {
    case 8:
	    printf("Note Off  : key=%d velocity=%d", msg[1], msg[2]);
	    break;
    case 9:
	    printf("Note On   : key=%d velocity=%d", msg[1], msg[2]);
	    break;
    case 10:
	    printf("PolyPress : key=%d velocity=%d", msg[1], msg[2]);
	    break;
    case 11:
        cont_num = msg[1] & 0xf;
        if (cont_num == 0x2) {
	        printf("Breath Cont : velocity=%d", msg[2]);
        } else {
	        printf("Control   : num=%d value=%d", msg[1], msg[2]);
        }
	    break;
    case 12:
	    printf("Program   : %d", msg[1]);
	    break;
    case 13:
	    printf("ChPress   : %d", msg[1]);
	    break;
    case 14:
	    printf("PitchBend : %d", msg[1] + 128 * msg[2] - 8192);
	    break;
    case 15:
	    if (ch == 0) {
	        printf("SysEx     : %02x %02x %02x", msg[0], msg[1], msg[2]);
	    } else if (ch == 1) {
	        printf("MIDI TC   : %d", msg[1]);
	    }else if (ch == 2) {
	        printf("SongPos   : %d", msg[1] + 128 * msg[2]);
	    } else if (ch == 3) {
	        printf("SongSelect: %d", msg[1]);
	    } else if (ch == 6) {
	        printf("TuneReqest");
	    } else if (ch == 8) {
	        printf("TimingClock");
	    } else if (ch == 10) {
	        printf("START");
	    } else if (ch == 11) {
	        printf("CONTINUE");
	    } else if (ch == 12) {
	        printf("STOP");
	    } else if (ch == 14) {
	        printf("ActiveSensing");
	    } else if (ch == 15) {
	        printf("RESET");
	    }
	    break;
    default:
	    break;
    }
    printf("\n");
}

void dump_message(const uint8_t msg[3], int n_data) {
    int i;
    static bool sysex = false;
    
    for(i = 0; i < n_data; i++) {
	    printf("%02x", msg[i]);
    }
    for(; i < 3; i++) {
	    printf("--");
    }
    printf(" | ");

    if (sysex) {
		printf("       |           : ");
		for (int i = 0; i < 3; i++) {
		    printf("%02x ", msg[i]);
		    if (msg[i] == 0xf7) {
			    sysex = false;
			    break;
		    }
		}
		printf("\n");
	} else {
		sysex = (msg[0] == 0xf0);
		print_midi_event(msg);
	}
}

void sendCMD(uint8_t *buffer, int size) {
    for(int i = 0; i < size; i++) {
	    uart_putc_raw(UART_ID, buffer[i]);
    }
}

void __time_critical_func(core1_entry)() {
    tusb_init();

    while(1){
        //sleep_ms(1);
        tuh_task();
        bool usb_connected = midi_dev_addr != 0 && tuh_midi_configured(midi_dev_addr);

        if (usb_connected){
            tuh_midi_stream_flush(midi_dev_addr);
        }    
    }
}

void __time_critical_func(core0_loop)() {
    while(1) {
        //sleep_ms(1);
    }
}


int main()
{
    uint8_t buffer[3];

    board_init();
    printf("Pico USB-MIDI to Serial Adapter\r\n");

    // Core電圧Up 1.1V->1.3V
    //vreg_set_voltage(VREG_VOLTAGE_1_30);
	// 最適なCPU周波数の設定(オーバクロック CLK_SYS=208800000[Hz])
    //set_sys_clock_khz(CLK_SYS/1000, true);

    multicore_reset_core1();

    stdio_init_all();

    // UARTデバッグ高速化
    // printf出力が多いとDAC再生が途切れる場合があるため
    //uart_init(uart0, 3000000);
	printf(">>>\nCLK_SYS=%ldHz\r\n", clock_get_hz(clk_sys));

    sleep_ms(1000); 

    // Set up UART1
    uart_init(UART_ID, BAUD_RATE);
    gpio_set_function(UART_TX_PIN, GPIO_FUNC_UART);
    gpio_set_function(UART_RX_PIN, GPIO_FUNC_UART);
    uart_set_hw_flow(UART_ID, false, false);
    uart_set_format(UART_ID, DATA_BITS, STOP_BITS, PARITY);
    //uart_set_fifo_enabled(UART_ID, true);

    //Set up M5UnitSynth
    /*
    buffer[0] = 0xb0;
    //uart_putc_raw(UART_ID, buffer[0]);
    sendCMD(buffer, sizeof(buffer));
    */
    //setMasterVolume(127);
    setVolume(0, 127);
    setInstrument(0, 0, AltoSax);

    multicore_launch_core1(core1_entry);
    core0_loop();

    return 0; // never gets here
}


//--------------------------------------------------------------------+
// TinyUSB Callbacks
//--------------------------------------------------------------------+

// Invoked when device with USB MIDI interface is mounted
void tuh_midi_mount_cb(uint8_t dev_addr, uint8_t in_ep, uint8_t out_ep, uint8_t num_cables_rx, uint16_t num_cables_tx)
{
  printf("midiserial2usbhost: MIDI device address = %u, IN endpoint %u has %u cables, OUT endpoint %u has %u cables\r\n",
      dev_addr, in_ep & 0xf, num_cables_rx, out_ep & 0xf, num_cables_tx);

  if (midi_dev_addr == 0) {
    // then no MIDI device is currently connected
    midi_dev_addr = dev_addr;
  }
  else {
    printf("midiserial2usbhost: A different USB MIDI Device is already connected.\r\nOnly one device at a time is supported in this program\r\nDevice is disabled\r\n");
  }
}

// Invoked when device with hid interface is un-mounted
void tuh_midi_umount_cb(uint8_t dev_addr, uint8_t instance)
{
  if (dev_addr == midi_dev_addr) {
    midi_dev_addr = 0;
    printf("ble-midi2usbhost: MIDI device address = %d, instance = %d is unmounted\r\n", dev_addr, instance);
  }
  else {
    printf("ble-midi2usbhost: Unused MIDI device address = %d, instance = %d is unmounted\r\n", dev_addr, instance);
  }
}

void __time_critical_func(tuh_midi_rx_cb)(uint8_t dev_addr, uint32_t num_packets)
{
    int ch;
    if (midi_dev_addr == dev_addr)
    {
        if (num_packets != 0)
        {
            uint8_t cable_num;
            //uint8_t buffer[48];
            uint8_t buffer[3];
            while (1) {
                //uint32_t bytes_read = tuh_midi_stream_read(dev_addr, &cable_num, buffer, sizeof(buffer));
                uint32_t bytes_read = tuh_midi_stream_read(dev_addr, &cable_num, buffer, 3);
                if (bytes_read == 0)
                    return; // done
                if (cable_num == 0) {
                    dump_message(buffer, bytes_read);
                    if ( (buffer[0] & 0xf0) == 0xB0 & (buffer[1] & 0x0f) == 0x02 ) { //Breath Control
                        EWI = 1;
	                    buffer[1] = 0x07;
                        /*
                        if(buffer[2] <= 15){
                            ch = buffer[0] & 0x0f;
                            setAllNotesOff(ch);
                            printf("x1");
                            printf("\n");

                        }
                        */
                    }
                    if ( EWI == 1 ) {
                        if ( (buffer[0] & 0xf0) == 0x90 ) { //EWI NoteON
                            ch = buffer[0] & 0x0f;
                            setAllNotesOff(ch);
                            printf("x2");
                            printf("\n");
                        }
                    }
                    sendCMD(buffer, bytes_read);
                    
                }
            }
        }
    }
}

void __time_critical_func(tuh_midi_tx_cb)(uint8_t dev_addr)
{
    (void)dev_addr;
}


//--------------------------------------------------------------------+
// M5UnitSynth
//--------------------------------------------------------------------+

void setInstrument(uint8_t bank, uint8_t channel, uint8_t value) {
    uint8_t CMD_CONTROL_CHANGE_1[] = {
        (uint8_t)(MIDI_CMD_CONTROL_CHANGE | (channel & 0x0f)), 0x00, bank};

    sendCMD(CMD_CONTROL_CHANGE_1, sizeof(CMD_CONTROL_CHANGE_1));

    uint8_t CMD_PROGRAM_CHANGE_2[] = {
        (uint8_t)(MIDI_CMD_PROGRAM_CHANGE | (channel & 0x0f)), value};
    sendCMD(CMD_PROGRAM_CHANGE_2, sizeof(CMD_PROGRAM_CHANGE_2));
}

void setNoteOn(uint8_t channel, uint8_t pitch, uint8_t velocity) {
    uint8_t CMD_NOTE_ON[] = {(uint8_t)(MIDI_CMD_NOTE_ON | (channel & 0x0f)),
                             pitch, velocity};
    sendCMD(CMD_NOTE_ON, sizeof(CMD_NOTE_ON));
}

void setNoteOff(uint8_t channel, uint8_t pitch, uint8_t velocity) {
    uint8_t CMD_NOTE_OFF[] = {(uint8_t)(MIDI_CMD_NOTE_OFF | (channel & 0x0f)),
                              pitch, 0x00};
    sendCMD(CMD_NOTE_OFF, sizeof(CMD_NOTE_OFF));
}
void setAllNotesOff(uint8_t channel) {
    uint8_t CMD_CONTROL_CHANGE[] = {
        (uint8_t)(MIDI_CMD_CONTROL_CHANGE | (channel & 0x0f)), 0x7b, 0x00};
    sendCMD(CMD_CONTROL_CHANGE, sizeof(CMD_CONTROL_CHANGE));
}

void setPitchBend(uint8_t channel, int value) {
    value                    = map(value, 0, 1023, 0, 0x3fff);
    uint8_t CMD_PITCH_BEND[] = {
        (uint8_t)(MIDI_CMD_PITCH_BEND | (channel & 0x0f)),
        (uint8_t)(value & 0xef), (uint8_t)((value >> 7) & 0xff)};
    sendCMD(CMD_PITCH_BEND, sizeof(CMD_PITCH_BEND));
}
void setPitchBendRange(uint8_t channel, uint8_t value) {
    uint8_t CMD_CONTROL_CHANGE[] = {
        (uint8_t)(MIDI_CMD_CONTROL_CHANGE | (channel & 0x0f)),
        0x65,
        0x00,
        0x64,
        0x00,
        0x06,
        (uint8_t)(value & 0x7f)};
    sendCMD(CMD_CONTROL_CHANGE, sizeof(CMD_CONTROL_CHANGE));
}

void setMasterVolume(uint8_t level) {
    uint8_t CMD_SYSTEM_EXCLUSIVE[] = {MIDI_CMD_SYSTEM_EXCLUSIVE,
                                      0x7f,
                                      0x7f,
                                      0x04,
                                      0x01,
                                      0x00,
                                      (uint8_t)(level & 0x7f),
                                      MIDI_CMD_END_OF_SYSEX};
    sendCMD(CMD_SYSTEM_EXCLUSIVE, sizeof(CMD_SYSTEM_EXCLUSIVE));
}
void setVolume(uint8_t channel, uint8_t level) {
    uint8_t CMD_CONTROL_CHANGE[] = {
        (uint8_t)(MIDI_CMD_CONTROL_CHANGE | (channel & 0x0f)), 0x07, level};
    sendCMD(CMD_CONTROL_CHANGE, sizeof(CMD_CONTROL_CHANGE));
}

void setReverb(uint8_t channel, uint8_t program, uint8_t level,
                            uint8_t delayfeedback) {
    uint8_t CMD_CONTROL_CHANGE_1[] = {
        (uint8_t)(MIDI_CMD_CONTROL_CHANGE | (channel & 0x0f)), 0x50,
        (uint8_t)(program & 0x07)};
    sendCMD(CMD_CONTROL_CHANGE_1, sizeof(CMD_CONTROL_CHANGE_1));
    /* program:(Default:hall2(0x4))
    room1:0x0
    room2:0x1
    room3:0x2
    hall1:0x3
    hall2:0x4
    plate:0x5
    delay:0x6
    pan delay:0x7
    */

    uint8_t CMD_CONTROL_CHANGE_2[] = {
        (uint8_t)(MIDI_CMD_CONTROL_CHANGE | (channel & 0x0f)), 0x5b,
        (uint8_t)(level & 0x7f)};
    sendCMD(CMD_CONTROL_CHANGE_2, sizeof(CMD_CONTROL_CHANGE_2));

    if (delayfeedback > 0) {
        uint8_t CMD_SYSTEM_EXCLUSIVE[] = {MIDI_CMD_SYSTEM_EXCLUSIVE,
                                          0x41,
                                          0x00,
                                          0x42,
                                          0x12,
                                          0x40,
                                          0x01,
                                          0x35,
                                          (uint8_t)(delayfeedback & 0x7f),
                                          0x00,
                                          MIDI_CMD_END_OF_SYSEX};
        sendCMD(CMD_SYSTEM_EXCLUSIVE, sizeof(CMD_SYSTEM_EXCLUSIVE));
    }
}

void setChorus(uint8_t channel, uint8_t program, uint8_t level,
                            uint8_t feedback, uint8_t chorusdelay) {
    uint8_t CMD_CONTROL_CHANGE_1[] = {
        (uint8_t)(MIDI_CMD_CONTROL_CHANGE | (channel & 0x0f)), 0x51,
        (uint8_t)(program & 0x07)};
    sendCMD(CMD_CONTROL_CHANGE_1, sizeof(CMD_CONTROL_CHANGE_1));

    uint8_t CMD_CONTROL_CHANGE_2[] = {
        (uint8_t)(MIDI_CMD_CONTROL_CHANGE | (channel & 0x0f)), 0x5d,
        (uint8_t)(level & 0x7f)};
    sendCMD(CMD_CONTROL_CHANGE_2, sizeof(CMD_CONTROL_CHANGE_2));

    if (feedback > 0) {
        uint8_t CMD_SYSTEM_EXCLUSIVE_1[] = {MIDI_CMD_SYSTEM_EXCLUSIVE,
                                            0x41,
                                            0x00,
                                            0x42,
                                            0x12,
                                            0x40,
                                            0x01,
                                            0x3b,
                                            (uint8_t)(feedback & 0x7f),
                                            0x00,
                                            MIDI_CMD_END_OF_SYSEX};
        sendCMD(CMD_SYSTEM_EXCLUSIVE_1, sizeof(CMD_SYSTEM_EXCLUSIVE_1));
    }

    if (chorusdelay > 0) {
        uint8_t CMD_SYSTEM_EXCLUSIVE_2[] = {MIDI_CMD_SYSTEM_EXCLUSIVE,
                                            0x41,
                                            0x00,
                                            0x42,
                                            0x12,
                                            0x40,
                                            0x01,
                                            0x3c,
                                            (uint8_t)(feedback & 0x7f),
                                            0x00,
                                            MIDI_CMD_END_OF_SYSEX

        };
        sendCMD(CMD_SYSTEM_EXCLUSIVE_2, sizeof(CMD_SYSTEM_EXCLUSIVE_2));
    }
}

void setPan(uint8_t channel, uint8_t value) {
    uint8_t CMD_CONTROL_CHANGE[] = {
        (uint8_t)(MIDI_CMD_CONTROL_CHANGE | (channel & 0x0f)), 0x0A, value};
    sendCMD(CMD_CONTROL_CHANGE, sizeof(CMD_CONTROL_CHANGE));
}

void setEqualizer(uint8_t channel, uint8_t lowband,
                               uint8_t medlowband, uint8_t medhighband,
                               uint8_t highband, uint8_t lowfreq,
                               uint8_t medlowfreq, uint8_t medhighfreq,
                               uint8_t highfreq) {
    uint8_t CMD_CONTROL_CHANGE[] = {
        (uint8_t)(MIDI_CMD_CONTROL_CHANGE | (channel & 0x0f)),
        0x63,
        0x37,
        0x62,
        0x00,
        0x06,
        (uint8_t)(lowband & 0x7f)};
    sendCMD(CMD_CONTROL_CHANGE, sizeof(CMD_CONTROL_CHANGE));

    CMD_CONTROL_CHANGE[4] = 0x01;
    CMD_CONTROL_CHANGE[6] = (medlowband & 0x7f);

    sendCMD(CMD_CONTROL_CHANGE, sizeof(CMD_CONTROL_CHANGE));

    CMD_CONTROL_CHANGE[4] = 0x02;
    CMD_CONTROL_CHANGE[6] = (medhighband & 0x7f);
    sendCMD(CMD_CONTROL_CHANGE, sizeof(CMD_CONTROL_CHANGE));

    CMD_CONTROL_CHANGE[4] = 0x03;
    CMD_CONTROL_CHANGE[6] = (highband & 0x7f);
    sendCMD(CMD_CONTROL_CHANGE, sizeof(CMD_CONTROL_CHANGE));

    CMD_CONTROL_CHANGE[4] = 0x08;
    CMD_CONTROL_CHANGE[6] = (lowfreq & 0x7f);
    sendCMD(CMD_CONTROL_CHANGE, sizeof(CMD_CONTROL_CHANGE));

    CMD_CONTROL_CHANGE[4] = 0x09;
    CMD_CONTROL_CHANGE[6] = (medlowfreq & 0x7f);
    sendCMD(CMD_CONTROL_CHANGE, sizeof(CMD_CONTROL_CHANGE));

    CMD_CONTROL_CHANGE[4] = 0x0A;
    CMD_CONTROL_CHANGE[6] = (medhighfreq & 0x7f);
    sendCMD(CMD_CONTROL_CHANGE, sizeof(CMD_CONTROL_CHANGE));

    CMD_CONTROL_CHANGE[4] = 0x0B;
    CMD_CONTROL_CHANGE[6] = (highfreq & 0x7f);
    sendCMD(CMD_CONTROL_CHANGE, sizeof(CMD_CONTROL_CHANGE));
}

void setTuning(uint8_t channel, uint8_t fine, uint8_t coarse) {
    uint8_t CMD_CONTROL_CHANGE[] = {
        (uint8_t)(MIDI_CMD_CONTROL_CHANGE | (channel & 0x0f)),
        0x65,
        0x00,
        0x64,
        0x01,
        0x06,
        (uint8_t)(fine & 0x7f)};

    sendCMD(CMD_CONTROL_CHANGE, sizeof(CMD_CONTROL_CHANGE));

    CMD_CONTROL_CHANGE[4] = 0x02;
    CMD_CONTROL_CHANGE[6] = (coarse & 0x7f);

    sendCMD(CMD_CONTROL_CHANGE, sizeof(CMD_CONTROL_CHANGE));
}
void setVibrate(uint8_t channel, uint8_t rate, uint8_t depth,
                             uint8_t delay) {
    uint8_t CMD_CONTROL_CHANGE[] = {
        (uint8_t)(MIDI_CMD_CONTROL_CHANGE | (channel & 0x0f)),
        0x63,
        0x01,
        0x62,
        0x08,
        0x06,
        (uint8_t)(rate & 0x7f)};
    sendCMD(CMD_CONTROL_CHANGE, sizeof(CMD_CONTROL_CHANGE));

    CMD_CONTROL_CHANGE[4] = 0x09;
    CMD_CONTROL_CHANGE[6] = (depth & 0x7f);
    sendCMD(CMD_CONTROL_CHANGE, sizeof(CMD_CONTROL_CHANGE));

    CMD_CONTROL_CHANGE[4] = 0x0A;
    CMD_CONTROL_CHANGE[6] = (delay & 0x7f);
    sendCMD(CMD_CONTROL_CHANGE, sizeof(CMD_CONTROL_CHANGE));
}

void setTvf(uint8_t channel, uint8_t cutoff, uint8_t resonance) {
    uint8_t CMD_CONTROL_CHANGE[] = {
        (uint8_t)(MIDI_CMD_CONTROL_CHANGE | (channel & 0x0f)),
        0x63,
        0x01,
        0x62,
        0x20,
        0x06,
        (uint8_t)(cutoff & 0x7f)};
    sendCMD(CMD_CONTROL_CHANGE, sizeof(CMD_CONTROL_CHANGE));

    CMD_CONTROL_CHANGE[4] = 0x21;
    CMD_CONTROL_CHANGE[6] = (resonance & 0x7f);
    sendCMD(CMD_CONTROL_CHANGE, sizeof(CMD_CONTROL_CHANGE));
}
void setEnvelope(uint8_t channel, uint8_t attack, uint8_t decay,
                              uint8_t release) {
    uint8_t CMD_CONTROL_CHANGE[] = {

        (uint8_t)(MIDI_CMD_CONTROL_CHANGE | (channel & 0x0f)),
        0x63,
        0x01,
        0x62,
        0x63,
        0x06,
        (uint8_t)(attack & 0x7f)};
    sendCMD(CMD_CONTROL_CHANGE, sizeof(CMD_CONTROL_CHANGE));

    CMD_CONTROL_CHANGE[4] = 0x64;
    CMD_CONTROL_CHANGE[6] = (decay & 0x7f);
    sendCMD(CMD_CONTROL_CHANGE, sizeof(CMD_CONTROL_CHANGE));

    CMD_CONTROL_CHANGE[4] = 0x66;
    CMD_CONTROL_CHANGE[6] = (release & 0x7f);
    sendCMD(CMD_CONTROL_CHANGE, sizeof(CMD_CONTROL_CHANGE));
}

void setModWheel(uint8_t channel, uint8_t pitch, uint8_t tvtcutoff,
                              uint8_t amplitude, uint8_t rate,
                              uint8_t pitchdepth, uint8_t tvfdepth,
                              uint8_t tvadepth) {
    uint8_t CMD_CONTROL_CHANGE[] = {MIDI_CMD_CONTROL_CHANGE,
                                    0x41,
                                    0x00,
                                    0x42,
                                    0x12,
                                    0x40,
                                    (uint8_t)(0x20 | (channel & 0x0f)),
                                    0x00,
                                    pitch,
                                    0x00,
                                    MIDI_CMD_END_OF_SYSEX};
    sendCMD(CMD_CONTROL_CHANGE, sizeof(CMD_CONTROL_CHANGE));

    CMD_CONTROL_CHANGE[8] = 0x01;
    CMD_CONTROL_CHANGE[9] = tvtcutoff;
    sendCMD(CMD_CONTROL_CHANGE, sizeof(CMD_CONTROL_CHANGE));

    CMD_CONTROL_CHANGE[8] = 0x02;
    CMD_CONTROL_CHANGE[9] = amplitude;
    sendCMD(CMD_CONTROL_CHANGE, sizeof(CMD_CONTROL_CHANGE));

    CMD_CONTROL_CHANGE[8] = 0x03;
    CMD_CONTROL_CHANGE[9] = rate;
    sendCMD(CMD_CONTROL_CHANGE, sizeof(CMD_CONTROL_CHANGE));

    CMD_CONTROL_CHANGE[8] = 0x04;
    CMD_CONTROL_CHANGE[9] = pitchdepth;
    sendCMD(CMD_CONTROL_CHANGE, sizeof(CMD_CONTROL_CHANGE));

    CMD_CONTROL_CHANGE[8] = 0x05;
    CMD_CONTROL_CHANGE[9] = tvfdepth;
    sendCMD(CMD_CONTROL_CHANGE, sizeof(CMD_CONTROL_CHANGE));

    CMD_CONTROL_CHANGE[8] = 0x06;
    CMD_CONTROL_CHANGE[9] = tvadepth;
    sendCMD(CMD_CONTROL_CHANGE, sizeof(CMD_CONTROL_CHANGE));
}

void setAllInstrumentDrums() {
    uint8_t CMD_CONTROL_CHANGE[] = {MIDI_CMD_CONTROL_CHANGE,
                                    0x41,
                                    0x00,
                                    0x42,
                                    0x12,
                                    0x40,
                                    0x10,
                                    0x15,
                                    0x01,
                                    0x00,
                                    MIDI_CMD_END_OF_SYSEX};
    sendCMD(CMD_CONTROL_CHANGE, sizeof(CMD_CONTROL_CHANGE));

    for (uint8_t i = 1; i < 15; i++) {
        CMD_CONTROL_CHANGE[6] = i;
        sendCMD(CMD_CONTROL_CHANGE, sizeof(CMD_CONTROL_CHANGE));
    }
}

void reset() {
    uint8_t CMD_SYSTEM_RESET[] = {MIDI_CMD_SYSTEM_RESET};
    sendCMD(CMD_SYSTEM_RESET, sizeof(CMD_SYSTEM_RESET));
}

