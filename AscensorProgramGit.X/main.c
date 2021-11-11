/*
Cambios pendientes [11 Nov]
 checkxx* Manejo de llamadas a piso con interrupciones, utilizar structura switch para cada caso en PORTB.
 checkxx* Flags de referencia para cada modo, para los botones de panel y llamadas a piso.
 checkxxx* Administar finalizacion modo subida llevando a cabo el inicio d emodo bajada, ordenando las llamadas a piso en la cola de llamadas de bajada. 
 
 * Cambiar ordenamiento a al reves cuando cambies de modo. (line 479)
 * Terminar else de cada piso en modoUp
 * Prueba iniciar en funcion con bucle, saltar a otra con bucle y volver a la inicial. (prueba para comprender si es verdad la idea de que el codigo va y lo busca a la memoria)

 
*/

/*
  [Pruebas pendietes]
 * Hardware sistema de puertas y prueba de software.
 * Prueba de inicializacion ascensor para determinar pines UP_ASC y DOWN_ASC
 * Prueba interrupciones en hardware con todos los botones de piso.
 * Prueba ordenamiento arreglos.
 
 
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
#define btnUpFL1 PORTBbits.RB0
#define btnUpFL2 PORTBbits.RB1
#define btnDownFL2 PORTBbits.RB2
#define btnUpFL3 PORTBbits.RB4
#define btnDownFL3 PORTBbits.RB5
#define btnDownFL4 PORTBbits.RB6

/* Switchs User panel  */
#define btnPn1 PORTAbits.RA0
#define btnPn2 PORTAbits.RA1
#define btnPn3 PORTAbits.RA2
#define btnPn4 PORTAbits.RA3
#define btnPnCD PORTAbits.RA4
#define btnPnOD PORTAbits.RA5
#define btnPnSTP PORTBbits.RB7

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

int tasksDown, tasksUp, calls, callsD, callFL_up, callFL_down, nowFL, queueUp[100], queueDown[100], callsInUp[100];
 
int *UpTasks, (*func_norepeat)();         
unsigned int modeUp_F, modeDown_F, btn1_f, btn2_f, btn3_f, btn4_f, btnCD_f, btnOD_f;    
unsigned int up1_f, up2_f, down2_f, up3_f, down3_f, down4_f;
     

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

    /* Reference of variables */
    tasksDown = 0;       // Number tasks for mode down.
    tasksUp = 0;         // Number tasks for mode up     
    calls = 0;           // Calls floors in mode up.
    callsD = 0;          // Calls floors in mode Down.
    callFL_up = 0;          // Piso de la ultima llamada.
    nowFL = 0;           // Last floor stoped.
    
    /* Flags */
    modeUp_F = 0;       // Mode up flag
    modeDown_F = 0;     // Mode down flag
    
    //btn1_f = 0;         // btn1 panel flag
    //btn2_f = 0;         // btn2 panel flag
    //btn3_f = 0;         // btn3 panel flag
    //btn4_f = 0;         // btn4 panel flag
    //btnCD_f = 0;        // btn CloseDoor panel flag
    //btnOD_f = 0;        // btn OpenDoor panel flag
        
    //up1_f = 0;          // btn floor 1 up.
    //up2_f = 0;          // btn floor 2 up.    
    //down2_f = 0;        // btn floor 2 down.
    //up3_f = 0;          // btn floor 3 up.
    //down3_f = 0;        // btn floor 3 down.
    //down4_f  = 0;       // btn floor 4 down.

    UpTasks = &queueUp[0];            // Pointer to array of task in mode up.
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

int noRepeat(int *po, int sixes){
    int size = sixes;
    /*

        *temp es la posicion para el elemento de comparacion.
        *comp es la posicion para el elemento a comparar.
        *shift es la variable de la posicion para el corrimiento.
    */

    for (int temp = 0; temp < size; temp++){                        // (For seleccion) para seleccionar el 'elemento de comparacion' del array.
            
        for(int comp = temp + 1; comp < size; comp++){              // (For comparacion) para comparar ese elemento con todos los demas.
            
            if (*(po + temp) == *(po + comp)){     // Determinamos si el 'elemento de comparacion' es igual al 'elemento comparado'. 
                int t = comp;

                for (int shift = comp; shift < size; shift++){      // (For corrimiento) corremos el arreglo para sacar el dato repetido.
                    *(po + t) = *(po + (shift + 1));
                    t++;
                }
                size--;
                comp--;                         // Mantengo mi posicion en el "for de comparacion" suponiendo que se corrio el arreglo una casilla.
                                                                           
            } 
        
        }
    }
    return size;

}

void sort(int *p, int sizes) {
    int temp, nums = 0, pos = 0, sizesMod = sizes;
    int result[100];                 // cambiar a global (en caso sea lento el proceso de ordenado en el pic) 

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

/* Pensar en volver global para modo subida y bajada */
void dataPanelUp() {
    do {
        if (btnPn1) {
                       
                if (nowFL < FLOOR1) {
                    queueUp[tasksUp] = FLOOR1;
                    tasksUp++;
                } else {
                    queueDown[tasksDown] = FLOOR1;
                    tasksDown++;
                }
                        
        }
        if (btnPn2) {
            
                if (nowFL < FLOOR2) {
                    queueUp[tasksUp] = FLOOR2;
                    tasksUp++;
                /* Ver esto como llamada a piso */
                } else {
                    queueDown[tasksDown] = FLOOR2; // cambiar 
                    tasksDown++;
                }
           
        }
        if (btnPn3) {

                if (nowFL < FLOOR3) {
                    queueUp[tasksUp] = FLOOR3;
                    tasksUp++;
                } else {
                    queueDown[tasksDown] = FLOOR3;
                    tasksDown++;

                }

        }
        if (btnPn4) {

                if (nowFL < FLOOR4) {
                    queueUp[tasksUp] = FLOOR4;
                    tasksUp++;
                } else {
                    queueDown[tasksDown] = FLOOR4;
                    tasksDown++;

                }
        }
    } while (!btnPnCD);   
 
    /*Oranize and trash the numbers repeat.*/
    sort(&queueUp[0], tasksUp);
    int new_size = noRepeat(&queueUp[0], tasksUp);
    tasksUp = new_size;

}

void modeDownControl(){
    
}

void modeUpControl() {

    static int cont = 0;

    switch (queueUp[cont]) {
        case FLOOR1:
            if (SenFL1) {
                
                
                // STOP in floor1
                DOWN_ASC = 1;
                
                
                for (int i = 0; i < tasksUp; i++) {       // clear call 0 in array of calls.
                    queueUp[i] = queueUp[i + 1];
                }
                tasksUp--;
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

                for (int i = 0; i < tasksUp; i++) {
                    queueUp[i] = queueUp[i + 1];
                }
                tasksUp--;
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

                for (int i = 0; i < tasksUp; i++) {
                    queueUp[i] = queueUp[i + 1];
                }
                tasksUp--;
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

                for (int i = 0; i < tasksUp; i++) {
                    queueUp[i] = queueUp[i + 1];
                }
                tasksUp--;
                dataPanelUp();
                DOOR4 = 0;
                __delay_ms(300);

                DOWN_ASC = 0;
            }
            break;
    }

}

void rutine_up(){
    modeUpControl();
                    
    if (tasksUp == 0){        // when the calls is complete...
        if((tasksDown > 0) || callsD > 0){
            for(int i = 0; i < callsD; i++){
                queueDown[tasksDown + i] = callsInUp[i];
            }
                            
            sort(&queueDown[0], tasksDown + callsD);                    // Ordenar de mayor a menor.
            tasksDown = noRepeat(&queueDown[0], tasksDown + callsD);    // Nueva lista sin repeticiones de ruta en modo bajada.
                            
            callFL_down = queueDown[0];
            for (int re = 0; re < tasksDown; re++){
                queueDown[re] = queueDown[re + 1];
            }
            tasksDown--;
                            
            /* Initialize mode Down */
            modeDown();
        }
        else{
            /* No calls en down mode and up mode, init waiting mode */
            callFL_up = 0;
            modeUp_F = 0;
            
        }
    }
    
}

void modeDown() {
    modeUp_F = 0;
    modeDown_F = 1;     // Set flag mode down.
    
    callsD = 0;
    
    tasksUp = 0;
}

void modeUp() {
    modeDown_F = 0;
    modeUp_F = 1;        // Set flag mode up.
    
    /*Contadores llamadas a piso */
    callsD = 0;
    calls = 0;
    
    /*Numero de tareas en cola */
    //tasksUp = 0;
    tasksDown = 0;
    
    unsigned int end_size = 0;

    switch (callFL_up) {
        case FLOOR1:
            if (nowFL == FLOOR1) {
                
                DOOR1 = 1;
                dataPanelUp();
                DOOR1 = 0;
                __delay_ms(300);

                /* Modo subida inicando! */
                DOWN_ASC = 0;
                
                /* Ejecutando motor de modo subida. */
                while (1) {
                    rutine_up();
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
            
        case FLOOR2:
            if (nowFL == FLOOR2) {
                
                DOOR2 = 1;
                dataPanelUp();
                DOOR2 = 0;
                __delay_ms(300);

                /* Modo subida inicando! */
                DOWN_ASC = 0;

                while (1) {
                    rutine_up();                   
                }
                           
            } else {
                if (nowFL < FLOOR2){
                    do{
                        DOWN_ASC = 0;
                    }while(!SenFL2);
                    DOWN_ASC = 0;
                    
                }
            }
    }
}

void main(void) {
    boot();
    interruptsInit();
    bootAscensor();
    
    while (1) {
        if (btnUpFL1) {
            callFL_up = FLOOR1;
            modeUp();
        }
        else if (btnUpFL2) {
            callFL_up = FLOOR2;
            modeUp();
        } 
        else if (btnUpFL3) {
            callFL_up = FLOOR3;
            modeUp();
        }
        
        if(btnDownFL2){
            
        }
        
    }

    return;
}
        

//I use a intrrupts in INT0-INT1-INT2 and changes in portb with RB port change interrupt.
void __interrupt() ISR() {
    
    switch (PORTB) {
        case 0x01: // (RB0) Button up 1 is press.
            if (modeUp_F){             
                    if (nowFL < FLOOR1){
                        queueUp[tasksUp] = FLOOR1;
                        tasksUp++;
                        sort(&queueUp[0], tasksUp);
                        int si = noRepeat(UpTasks, tasksUp);
                        tasksUp = si;
                       
                    }
            }
            break;

        case 0x02: // (RB1) button Up2 press 0x02 is 2 in decimal. 
            if (modeUp_F){
                    if (nowFL < FLOOR2){
                        queueUp[tasksUp] = FLOOR2;
                        tasksUp++;
                        
                        sort(&queueUp[0], tasksUp);
                        int si = noRepeat(&queueUp[0], tasksUp);
                        tasksUp = si;
                    }
            }
            break;

        case 0x04: // (RB2) button Down2 press 0x04 is 4 in decimal.
            if (modeUp_F){
                    callsInUp[callsD] = FLOOR2;
                    callsD++;
                    down2_f = 1;    // set flag button dwon floor 2.
            }
            break;

        case 0x08: // (RB3) not use.
            
            break;
            
        case 0x10: // (RB4) button Up3 press 0x08 is 8 in decimal.
            if (modeUp_F){
                    if (nowFL < FLOOR3){
                        queueUp[tasksUp] = FLOOR3;
                        tasksUp++;
                        sort(&queueUp[0], tasksUp);
                        int si = noRepeat(&queueUp[0], tasksUp);
                        tasksUp = si;
                    }
            }
            break;

        case 0x20: // (RB5) button Down3
            if (modeUp_F){
                    callsInUp[callsD] = FLOOR3;
                    callsD++;
                    down3_f = 1;    // set flag button dwon floor 3.
            }
            break;

        case 0x40: // (RB6)button down4 
            if (modeUp_F){
                    callsInUp[callsD] = FLOOR4;
                    callsD++;
                    down4_f = 1;    // set flag button dwon floor 4.
            }
            break;
            
        case 0x80: // (RB7) PNL_BTN_STP
            break;
    }
}