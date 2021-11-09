/*
Cambios pendientes [8 Nov]
 checkxx* Manejo de llamadas a piso con interrupciones, utilizar structura switch para cada caso en PORTB.
 * Flags de referencia para cada modo, para los botones de panel y llamadas a piso.
 * Administar finalizacion modo subida llevando a cabo el inicio d emodo bajada, ordenando las llamadas a piso en la cola de llamadas de bajada. 
 
 
 * Administrar llamadas a piso en el modo subida con el arreglo de modo subida, ordenarlo dependiendo de si cumple las espectativas de condiconamiento
 * en la interrupciones tales como que el piso de llamada sea mayor al actual, caso no sera enviado al arreglo de modo bajada si es llamada a piso para 
 * bajada. 

 
*/





// PIC18F452 Configuration Bit Settings
// CONFIG1H
#pragma config OSC = HS         // Oscillator Selection bits (HS oscillator)
#pragma config OSCS = OFF       // Oscillator System Clock Switch Enable bit (Oscillator system clock switch option is disabled (main oscillator is source))
// CONFIG2L
#pragma config PWRT = OFF       // Power-up Timer Enable bit (PWRT disabled)
#pragma config BOR = OFF        // Brown-out Reset Enable bit (Brown-out Reset disabled)
#pragma config BORV = 20        // Brown-out Reset Voltage bits (VBOR set to 2.0V)
// CONFIG2H
#pragma config WDT = OFF        // Watchdog Timer Enable bit (WDT disabled (control is placed on the SWDTEN bit))
#pragma config WDTPS = 128      // Watchdog Timer Postscale Select bits (1:128)
// CONFIG3H
#pragma config CCP2MUX = OFF    // CCP2 Mux bit (CCP2 input/output is multiplexed with RB3)
// CONFIG4L
#pragma config STVR = OFF       // Stack Full/Underflow Reset Enable bit (Stack Full/Underflow will not cause RESET)
#pragma config LVP = OFF        // Low Voltage ICSP Enable bit (Low Voltage ICSP disabled)
// CONFIG5L
#pragma config CP0 = ON         // Code Protection bit (Block 0 (000200-001FFFh) code protected)
#pragma config CP1 = OFF        // Code Protection bit (Block 1 (002000-003FFFh) not code protected)
#pragma config CP2 = OFF        // Code Protection bit (Block 2 (004000-005FFFh) not code protected)
#pragma config CP3 = OFF        // Code Protection bit (Block 3 (006000-007FFFh) not code protected)
// CONFIG5H
#pragma config CPB = OFF        // Boot Block Code Protection bit (Boot Block (000000-0001FFh) not code protected)
#pragma config CPD = OFF        // Data EEPROM Code Protection bit (Data EEPROM not code protected)
// CONFIG6L
#pragma config WRT0 = OFF       // Write Protection bit (Block 0 (000200-001FFFh) not write protected)
#pragma config WRT1 = OFF       // Write Protection bit (Block 1 (002000-003FFFh) not write protected)
#pragma config WRT2 = OFF       // Write Protection bit (Block 2 (004000-005FFFh) not write protected)
#pragma config WRT3 = OFF       // Write Protection bit (Block 3 (006000-007FFFh) not write protected)
// CONFIG6H
#pragma config WRTC = OFF       // Configuration Register Write Protection bit (Configuration registers (300000-3000FFh) not write protected)
#pragma config WRTB = OFF       // Boot Block Write Protection bit (Boot Block (000000-0001FFh) not write protected)
#pragma config WRTD = OFF       // Data EEPROM Write Protection bit (Data EEPROM not write protected)
// CONFIG7L
#pragma config EBTR0 = OFF      // Table Read Protection bit (Block 0 (000200-001FFFh) not protected from Table Reads executed in other blocks)
#pragma config EBTR1 = OFF      // Table Read Protection bit (Block 1 (002000-003FFFh) not protected from Table Reads executed in other blocks)
#pragma config EBTR2 = OFF      // Table Read Protection bit (Block 2 (004000-005FFFh) not protected from Table Reads executed in other blocks)
#pragma config EBTR3 = OFF      // Table Read Protection bit (Block 3 (006000-007FFFh) not protected from Table Reads executed in other blocks)
// CONFIG7H
#pragma config EBTRB = OFF      // Boot Block Table Read Protection bit (Boot Block (000000-0001FFh) not protected from Table Reads executed in other blocks)

/* Star program in main.c
 *  23ft. 
 *  Tecnicas Digitales
 *  2021
 *  PIC18F452
 */
#include <xc.h>
#include <stdlib.h>
#define _XTAL_FREQ 20000000

/* Switchs for floor */
#define btnUpFL1 PORTBbits.RB6
#define btnUpFL2 PORTBbits.RB1
#define btnDownFL2 PORTBbits.RB2
#define btnUpFL3 PORTBbits.RB3
#define btnDownFL3 PORTBbits.RB4
#define btnDownFL4 PORTBbits.RB5

/* Switchs User panel  */
#define btnPn1 PORTAbits.RA0
#define btnPn2 PORTAbits.RA1
#define btnPn3 PORTAbits.RA2
#define btnPn4 PORTAbits.RA3
#define btnPnCD PORTAbits.RA4
#define btnPnOD PORTAbits.RA5
#define btnPnSTP PORTBbits.RB0

/* Sensors hall */
#define SenFL1 PORTDbits.RD4
#define SenFL2 PORTDbits.RD5 
#define SenFL3 PORTDbits.RD6 
#define SenFL4 PORTDbits.RD7 

/* Pins for control doors */
#define DOOR1 LATDbits.LD0
#define DOOR2 LATDbits.LD1
#define DOOR3 LATDbits.LD2
#define DOOR4 LATDbits.LD3

/* Control principal motor */
#define UP_ASC LATCbits.LC0
#define DOWN_ASC LATCbits.LC1

/* Variables globales */
#define FLOOR1 1
#define FLOOR2 2
#define FLOOR3 3
#define FLOOR4 4

int calls, callsD, callFL, nowFL, queueUp[100], queueDown[100], callsInUp[100], numUps;

unsigned int upF = 0, downF = 0;    // flags.

/* Prototypes functions */
#define size(x, type) (sizeof(x) / sizeof(type))

void bootAscensor();             // Funcion para incializar ascensor.

void modeUp();                   // Funcion modo subiendo.
void callsUp();                  // Funcion administrar llamadas en subida.

void modeDown();                 // Funcion modo bajando.
void sort(int *p, int sizes);    // Funcion ordenar array unidimensional, recibe puntero a arreglo y tamaño.

void interruptsInit(void);       // Function to config interrupts, in this case use a interrupts unmasked (low and high priority interrupts is not used)


/* Start Program */
void boot() {
    ADCON1 = 0x06;          // Registro ADCON1 para ADC, configuracion pines como I/O
    TRISB = 0xFF;           // Registro TRISB definimos todos como entradas.
    TRISD = 0xF0;           // RD0-RD3 (Outputs), RD4-RD7 (Inputs).   
    TRISA = 0xFF;           // Registro TRISA definimos todas como salidas.
    TRISC = 0xFC;           // Registro TRISC definimos RC0-RC1 salidas, RC2-RC7 entradas.
}

void interruptsInit(void) {
    RCONbits.IPEN = 0;      // Habilitiamos interrupciones sin mask.
    INTCONbits.GIE = 1;     // Enabled unmasked interrupts (Global INterrupt Enabled Bit)
    INTCONbits.PEIE = 1;    // Enabled peripheral interrups (Peripheral Interrupt Enabled bit)
}

void bootAscensor() {
    UP_ASC = 1;
    DOWN_ASC = 1;

    if (SenFL1 == 1) {
        nowFL = FLOOR1;
        return;
    } else {
        UP_ASC = 0;
        while (!SenFL1) {
            continue;
        }
        UP_ASC = 1;
        nowFL = FLOOR1;
        return;
    }
}

void sort(int *p, int sizes) {
    int temp, nums = 0, pos = 0, sizesMod = sizes;
    static int result[100];                 // cambiar a global (en caso sea lento el proceso de ordenado en el pic) 

    // Por ser XC8 para espacios con RAM pequeña no es recomendable trabajar con espacios dinamicos, requeriria muchos recursos.
    //result = (int*)malloc(sizes*sizeof(int));

    do {
        temp = *(p + 0);

        for (int h = 0; h < sizesMod; h++) {

            if (*(p + h) == temp) {
                temp = temp;
                pos = h;
            }
            if (*(p + h) > temp) {
                temp = temp;
                pos = pos;
            }

            if (*(p + h) < temp) {
                temp = *(p + h);
                pos = h;
            }
        }

        /* This for function is desplaced the array and remove a element select in previus For's*/
        for (int k = pos; k < sizesMod; k++) {
            *(p + k) = *(p + (k + 1));
        }
        result[nums] = temp;
        sizesMod--; // size of principal arry in iterations.
        nums++; // sentinel variable for carrier a nums listed in "result"
    } while (nums <= sizes - 1);

    /* The for function is place a data in result in a arrary pointer */
    for (int j = 0; j < sizes; j++) {
        *(p + j) = result[j];
    }
}

void dataPanelUp() {
    do {
        if (btnPn1) {
            if (nowFL < FLOOR1) {
                queueUp[calls] = FLOOR1;
                calls++;
            } else {
                queueDown[callsD] = FLOOR1;
                callsD++;
            }

        }
        if (btnPn2) {
            if (nowFL < FLOOR2) {
                queueUp[calls] = FLOOR2;
                calls++;
            } else {
                queueDown[callsD] = FLOOR2;
                callsD++;
            }

        }
        if (btnPn3) {
            if (nowFL < FLOOR3) {
                queueUp[calls] = FLOOR3;
                calls++;
            } else {
                queueDown[callsD] = FLOOR3;
                callsD++;
            }

        }
        if (btnPn4) {
            if (nowFL < FLOOR4) {
                queueUp[calls] = FLOOR4;
                calls++;
            } else {
                queueDown[callsD] = FLOOR4;
                callsD++;
            }

        }
    } while (!btnPnCD);

    int *UpCalls = &queueUp[0];
    int tamano = calls;

    sort(UpCalls, tamano);

}

void modeUpControl() {

    static int cont = 0;

    switch (queueUp[cont]) {
        case FLOOR1:
            if (SenFL1) {
                // STOP in floor1
                DOWN_ASC = 1;
                for (int i = 0; i < calls; i++) {
                    queueUp[i] = queueUp[i + 1];
                }
                calls--;
                DOOR1 = 1;
                nowFL = FLOOR1;
                
                dataPanelUp();
                
                DOOR1 = 0;
                __delay_ms(300);

                DOWN_ASC = 0;
            }
            break;

        case FLOOR2:
            if (SenFL2) {
                // STOP in floor2
                DOWN_ASC = 1;
                DOOR2 = 1;
                nowFL = FLOOR2;

                for (int i = 0; i < calls; i++) {
                    queueUp[i] = queueUp[i + 1];
                }
                calls--;
                dataPanelUp();
                DOOR2 = 0;
                __delay_ms(300);

                DOWN_ASC = 0;
            }
            break;

        case FLOOR3:
            if (SenFL3) {
                // STOP in floor3
                DOWN_ASC = 1;
                DOOR3 = 1;
                nowFL = FLOOR3;

                for (int i = 0; i < calls; i++) {
                    queueUp[i] = queueUp[i + 1];
                }
                calls--;
                dataPanelUp();
                DOOR3 = 0;
                __delay_ms(300);

                DOWN_ASC = 0;
            }
            break;

        case FLOOR4:
            if (SenFL4) {
                // STOP in floor4
                DOWN_ASC = 1;
                DOOR4 = 1;
                nowFL = FLOOR4;

                for (int i = 0; i < calls; i++) {
                    queueUp[i] = queueUp[i + 1];
                }
                calls--;
                dataPanelUp();
                DOOR4 = 0;
                __delay_ms(300);

                DOWN_ASC = 0;
            }
            break;
    }

}

void modeUp() {
    upF = 1;        // Set flag mode up.
    callsD = 0;
    calls = 0;
    numUps = 0;

    switch (callFL) {
        case 1:

            if (nowFL == FLOOR1) {
                DOOR1 = 1;
                dataPanelUp();
                DOOR1 = 0;
                __delay_ms(300);

                /* Modo subida inicando! */
                DOWN_ASC = 0;

                while (1) {

                    modeUpControl();
                    
                }

            } else {
                do {
                    UP_ASC = 0;
                } while (!SenFL1);
                UP_ASC = 1;

                DOOR1 = 1;
                dataPanelUp();
                DOOR1 = 0;
            }
    }
}

void main(void) {
    boot();
    interruptsInit();
    bootAscensor();
    
    while (1) {
        if (btnUpFL1) {
            callFL = FLOOR1;
            modeUp();
        }
        else if (btnUpFL2) {
            callFL = FLOOR2;
            modeUp();
        } 
        else if (btnUpFL3) {
            callFL = FLOOR3;
            modeUp();
        }
    }

    return;
}
        

//I use a intrrupts in INT0-INT1-INT2 and changes in portb with RB port change interrupt.
void __interrupt() ISR() {
    switch (PORTB) {
        case 0x00: // (RB0) Button up 1 is press.
            LATD = 0x00;
            break;

        case 0x02: // (RB1) button Up2 press 0x02 is 2 in decimal. 
            callsInUp[numUps] = FLOOR2;
            numUps++;
            break;

        case 0x04: // (RB2) button Down2 press 0x04 is 4 in decimal.
            
            break;

        case 0x08: // (RB3) not use.
            callsInUp[numUps] = FLOOR3;
            numUps++;
            break;

        case 0x10: // (RB4) button Up3 press 0x08 is 8 in decimal.
            LATD = 0x10;
            break;

        case 0x20: // (RB5) button Down3
            LATD = 0x20;
            break;

        case 0x40: // (RB6)button down4
            LATD = 0x40;
            break;
            
        case 0x80: // (RB7) PNL_BTN_STP
            break;
    }
}