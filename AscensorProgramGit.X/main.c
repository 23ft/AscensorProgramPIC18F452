/*
Cambios pendientes [Arreglar bugs jajajaja]
 ~* Manejo de llamadas a piso con interrupciones, utilizar structura switch para cada caso en PORTB.
 ~* Flags de referencia para cada modo, para los botones de panel y llamadas a piso.
 ~* Administar finalizacion modo subida llevando a cabo el inicio d emodo bajada, ordenando las llamadas a piso en la cola de llamadas de bajada. 
 ~* Terminar else de cada piso en modoUp
 ~* Definir flag para "change_state" 1 si a cambiado de estado, 0 si no. (para manejo de variables tasksDown y tasksUp)
 ~*Funcion now_level cambiar a modeUpControl, modeUpControl cambiar a 'select_task', ahora en modeUpControl (antes now_level) determinar que cada vez
 que llegue a un sensor determinelo como nowlevel y condicione si en la lista queueUp[0] efectivamente es FLOOR1 para parar y ejecutar lo correspondiente
 al modo subida
 
 ~* Terminar funcion callsControl, integrar condicional sobre modo actual para determinar como administrar las llamadas.
 ~* Terminar funcion rutine_ascensor (rutine_up) para integrar condicionales del modo.
 ~* Cambiar ordenamiento a al reves cuando cambies de modo. (line 479)
 ~* Prueba iniciar en funcion con bucle, saltar a otra con bucle y volver a la inicial. (prueba para comprender si es verdad la idea de que el codigo va y lo busca a la memoria)
   
*/

/*
  [Pruebas pendietes]
 ~* Prueba displays. y comunicacion.
 ~* Hardware sistema de puertas y prueba de software.
 ~* Prueba de inicializacion ascensor para determinar pines UP_ASC y DOWN_ASC
 ~* Prueba interrupciones en hardware con todos los botones de piso.
 ~* Prueba ordenamiento arreglos.
 
 
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
#define _XTAL_FREQ 20000000
#include <stdlib.h>
#include <stdio.h>
#include "I2C_LCD.c"
#include <pic18f452.h>


/* Switchs for floor */
#define btnUp1 PORTBbits.RB0
#define btnUp2 PORTBbits.RB2
#define btnDown2 PORTBbits.RB1
#define btnUp3 PORTBbits.RB5
#define btnDown3 PORTBbits.RB4
#define btnDown4 PORTBbits.RB6

/* Switchs User panel  */
#define btnPn1 PORTAbits.RA0
#define btnPn2 PORTAbits.RA1
#define btnPn3 PORTAbits.RA2
#define btnPn4 PORTAbits.RA3
#define btnPnCD PORTAbits.RA4
#define btnPnOD PORTAbits.RA5
#define btnPnSTP PORTBbits.RB7

/* Sensors hall */
#define senFL1 PORTDbits.RD4
#define senFL2 PORTDbits.RD5 
#define senFL3 PORTDbits.RD6 
#define senFL4 PORTDbits.RD7 

/* Pins for control doors */
#define DOOR1 LATDbits.LD0
#define DOOR2 LATDbits.LD1
#define DOOR3 LATDbits.LD2
#define DOOR4 LATDbits.LD3

/* Control principal motor */
#define UP_ASC LATCbits.LC0
#define DOWN_ASC LATCbits.LC1

/* Variables globales */
#define case_FL1_s 0x10
#define case_FL2_s 0x20 
#define case_FL3_s 0x40
#define case_FL4_s 0x80 

#define FLOOR1 1
#define FLOOR2 2
#define FLOOR3 3
#define FLOOR4 4

#define modeUpON 1
#define modeDownON 0

int tasksDown, tasksUp, callsU, callsD, call_init, nowFL, prevFL, queueUp[100], queueDown[100], callsInUp[100], callsInDown[100];        
unsigned int modeUp_F, modeDown_F, from_up_change, from_down_change;
char buffer[40], pLCD = 1;

/* Prototypes functions */
/* ~~~ ~~~ ~~~ ~~~ ~~~ ~~~ ~~~ ~~~ ~~~ ~~~ ~~~ ~~~ ~~~ ~~~ ~~~ ~~~ ~~~ ~~~ ~~~ ~~~ ~~~ ~~~ ~~~ ~~~ ~~~ ~~~ ~~~ ~~~*/

#define size(x, type) (sizeof(x) / sizeof(type))

void bootAscensor(void);            // Funcion para incializar ascensor.

int rutine_up(void);               // FUncion rutina en motor de subida.
int rutine_down(void);             // Funcion rutina en motor de bajada.
void dataPanelUp(void);            // Funcion para el manejo del panel en cada parada [cambiar nombre].

void modeControl(unsigned select_mode);           // COntrol para inicial el modo.
void selectionMode(unsigned mode_s);              // Motor para seleccionar modo.
void measureBtn(void);                            // Funcion para sondeo botones a piso.

void sort(int *p, int sizes);      // Funcion ordenar array unidimensional, recibe puntero a arreglo y tamaño.
int noRepeat(int *po, int sixes);  // Funcion para quitar repeticiones en un arreglo.

void interruptsInit(void);         // Function to config interrupts, in this case use a interrupts unmasked (low and high priority interrupts is not used)

/* ~~~ ~~~ ~~~ ~~~ ~~~ ~~~ ~~~ ~~~ ~~~ ~~~ ~~~ ~~~ ~~~ ~~~ ~~~ ~~~ ~~~ ~~~ ~~~ ~~~ ~~~ ~~~ ~~~ ~~~ ~~~ ~~~ ~~~ ~~~*/

void boot() {
    ADCON1 = 0x06;          // Registro ADCON1 para ADC, configuracion pines como I/O
    TRISB = 0xFF;           // Registro TRISB definimos todos como entradas.
    TRISD = 0xF0;           // RD0-RD3 (Outputs), RD4-RD7 (Inputs).   
    TRISA = 0xFF;           // Registro TRISA definimos todas como salidas.
    TRISC = 0xFC;           // Registro TRISC definimos RC0-RC1 salidas, RC2-RC7 entradas.

    /* Reference of variables */
    tasksDown = 0;       // Number tasks for mode down.
    tasksUp = 0;         // Number tasks for mode up     
    callsU = 0;           // Calls floors in mode up.
    callsD = 0;          // Calls floors in mode Down.
    call_init = 0;          // Piso de la ultima llamada.
    prevFL = 0;           // Last floor stoped.
    nowFL = 0;            // Piso actual.
        
    /* Flags */
    modeUp_F = 0;       // Mode up flag
    modeDown_F = 0;     // Mode down flag
    
    from_up_change = 0;
    from_down_change = 0;
    

}

void measureBtn(void){
    
        if(btnUp1){ // (RB0) Button up 1 is press.
            if (modeUp_F){             
                    if (nowFL < FLOOR1){
                        queueUp[tasksUp] = FLOOR1;
                        tasksUp++;
                        sort(&queueUp[0], tasksUp);
                        tasksUp = noRepeat(&queueUp[0], tasksUp);
                        
                       
                    }
            }
        }    

        if(btnUp2){ // RB1 Button up2 is press.
            if (modeUp_F){
                    if (nowFL < FLOOR2){
                        queueUp[tasksUp] = FLOOR2;
                        tasksUp++;
                        
                        sort(&queueUp[0], tasksUp);
                        tasksUp = noRepeat(&queueUp[0], tasksUp);
                    }
            }
        }

        if(btnDown2){ // RB2 button down2 is press.
            if (modeUp_F){
                    callsInUp[callsD] = FLOOR2;
                    callsD++;
            }
        }

            
        if(btnUp3){ // (RB4) button Up3 press 0x08 is 8 in decimal.
            if (modeUp_F){
                    if (nowFL < FLOOR3){
                        queueUp[tasksUp] = FLOOR3;
                        tasksUp++;
                        sort(&queueUp[0], tasksUp);
                        tasksUp = noRepeat(&queueUp[0], tasksUp);
                    }
            }
        }

        if(btnDown3){ // (RB5) button Down3
            if (modeUp_F){
                    callsInUp[callsD] = FLOOR3;
                    callsD++;
            }
        }

        if(btnDown4){ // (RB6)button down4 
            if (modeUp_F){
                    callsInUp[callsD] = FLOOR4;
                    callsD++;
            }
        }
            
        if(btnPnSTP){ // (RB7) PNL_BTN_STP
        }
    
}


void interruptsInit(void) {
    RCONbits.IPEN = 0;      // Habilitiamos interrupciones sin mask.
    INTCONbits.GIE = 1;     // Enabled unmasked interrupts (Global INterrupt Enabled Bit)
    INTCONbits.PEIE = 1;    // Enabled peripheral interrups (Peripheral Interrupt Enabled bit)
    INTCONbits.RBIE = 1; 
}

void bootAscensor(void) {
    UP_ASC = 1;
    DOWN_ASC = 1;
    
    DOOR1 = 0;
    DOOR2 = 0;
    DOOR3 = 0;
    DOOR4 = 0;

    __delay_ms(3000);
    
    if (senFL1 == 1) {
        prevFL = FLOOR1;
        return;
    } else {
        UP_ASC = 0;
        while (!senFL1) {
            continue;
        }
        UP_ASC = 1;
        prevFL = FLOOR1;
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

void sort_down(int *p, int sizes) {
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
                //temp = temp;
                //pos = pos;
                temp = *(p + h);
                pos = h;
            }

            if (*(p + h) < temp) {
                //temp = *(p + h);
                //pos = h;
                temp = temp;
                pos = pos;
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
void dataPanelUp(void) {
    //LCD_Clear();            
    //LCD_Set_Cursor(1,1);
    //LCD_Write_String("Panel btns...");
    __delay_ms(300);
    do {
        if (btnPn1) {
            
            LCD_Clear();
            LCD_Set_Cursor(1,1);
            LCD_Write_String("BtnPn1 Press!");
            
                if(modeUp_F){       
                    if (nowFL < FLOOR1) {
                        queueUp[tasksUp] = FLOOR1;
                        tasksUp++;
                    } else {
                        queueDown[tasksDown] = FLOOR1;
                        tasksDown++;
                    }
                } else {
                    if (nowFL > FLOOR1) {
                        queueDown[tasksDown] = FLOOR1;
                        tasksDown++;
                    } else {
                        queueUp[tasksUp] = FLOOR1;
                        tasksUp++;
                    }
                }
                
        }
        if (btnPn2) {
            
            LCD_Clear();
            LCD_Set_Cursor(1,1);
            LCD_Write_String("BtnPn2 Press!");
            
                if(modeUp_F){       
                    if (nowFL < FLOOR2) {
                        queueUp[tasksUp] = FLOOR2;
                        tasksUp++;
                    } else {
                        queueDown[tasksDown] = FLOOR2;
                        tasksDown++;
                    }
                } else {
                    if (nowFL > FLOOR2) {
                        queueDown[tasksDown] = FLOOR2;
                        tasksDown++;
                    } else {
                        queueUp[tasksUp] = FLOOR2;
                        tasksUp++;
                    }
                }
           
        }
        if (btnPn3) {
            
            LCD_Clear();
            LCD_Set_Cursor(1,1);
            LCD_Write_String("BtnPn3 Press!");
            
                if(modeUp_F){       
                    if (nowFL < FLOOR3) {
                        queueUp[tasksUp] = FLOOR3;
                        tasksUp++;
                    } else {
                        queueDown[tasksDown] = FLOOR3;
                        tasksDown++;
                    }
                } else {
                    if (nowFL > FLOOR3) {
                        queueDown[tasksDown] = FLOOR3;
                        tasksDown++;
                    } else {
                        queueUp[tasksUp] = FLOOR3;
                        tasksUp++;
                    }
                }

        }
        if (btnPn4) {
            LCD_Clear();
            LCD_Set_Cursor(1,1);
            LCD_Write_String("BtnPn4 Press!");

                if(modeUp_F){       
                    if (nowFL < FLOOR4) {
                        queueUp[tasksUp] = FLOOR4;
                        tasksUp++;
                    } else {
                        queueDown[tasksDown] = FLOOR4;
                        tasksDown++;
                    }
                } else {
                    if (nowFL > FLOOR4) {
                        queueDown[tasksDown] = FLOOR4;
                        tasksDown++;
                    } else {
                        queueUp[tasksUp] = FLOOR4;
                        tasksUp++;
                    }
                }
        }
    } while (!btnPnCD);   
 
    /*Oranize and trash the numbers repeat.*/
    if(modeUp_F){
        sort(&queueUp[0], tasksUp); // ordenando de menor a mayor.
        tasksUp = noRepeat(&queueUp[0], tasksUp);
    } else {
        sort_down(&queueDown[0], tasksDown); // ordenando de menor a mayor.
        tasksDown = noRepeat(&queueDown[0], tasksDown);
    }    

}

void controlCalls(){
    
        if(senFL1){                // comporbar si no influye los datos de LATD en los otros pines del puerto D.
            nowFL = FLOOR1;
            if (queueUp[0] == FLOOR1) {
                // STOP in floor1
                prevFL = FLOOR1;
                DOWN_ASC = 1;   // brake.
                   
                for (int i1 = 0; i1 < tasksUp; i1++) {       // clear call 0 in array of calls.
                    queueUp[i1] = queueUp[i1 + 1];
                }
                tasksUp--;
                
                /* Write in LCD data */
                LCD_Clear();
                LCD_Set_Cursor(1,1);
                LCD_Write_String("Up: Stop FL1");
                
                              
                for (int i_1 = 0; i_1 < tasksUp; i_1++){
                    
                    sprintf(buffer, "%d", queueUp[i_1]);
                    LCD_Set_Cursor(2, pLCD);
                    LCD_Write_String(buffer);
                    pLCD++;
                }        
                pLCD = 1;
                __delay_ms(3000);
                /* Normal Work */
                DOOR1 = 1;
                dataPanelUp();
                DOOR1 = 0;
                __delay_ms(300);
                
                if(tasksUp != 0){
                    DOWN_ASC = 0;       // Continue in mode UP.
                } else {
                    LCD_Clear();
                    LCD_Set_Cursor(1,1);
                    LCD_Write_String("Mode Up OFF"); 
                    return;
                }
                
                
            }
            if (queueDown[0] == FLOOR1) {
                // STOP in floor1
                prevFL = FLOOR1;
                UP_ASC = 1;     // brake.    
                
                for (int i1 = 0; i1 < tasksDown; i1++) {       // clear call 0 in array of calls.
                    queueDown[i1] = queueDown[i1 + 1];
                }
                tasksDown--;
                
                /* Write in LCD data */
                LCD_Clear();
                LCD_Set_Cursor(1,1);
                LCD_Write_String("Down: Stop FL1");                        
                
              
                for (int i_2 = 0; i_2 < tasksDown; i_2++){
                    
                    sprintf(buffer, "%d", queueDown[i_2]);
                    LCD_Set_Cursor(2, pLCD);
                    LCD_Write_String(buffer);
                    pLCD++;
                }        
                pLCD = 1;
                __delay_ms(3000);
                
                /* Normal Work */
                DOOR1 = 1;
                dataPanelUp();
                DOOR1 = 0;
                __delay_ms(300);
                // UP_ASC = 0;         // Continue in mode DOWN.
                
            }
                       
        }
        if(senFL2){
            nowFL = FLOOR2;
            if (queueUp[0] == FLOOR2) {
                // STOP in floor2
                prevFL = FLOOR2;
                
                DOWN_ASC = 1;   // mode up.
                   
                for (int i1 = 0; i1 < tasksUp; i1++) {       // clear call 0 in array of calls.
                    queueUp[i1] = queueUp[i1 + 1];
                }
                tasksUp--;
                
                /* Write in LCD data */
                LCD_Clear();
                LCD_Set_Cursor(1,1);
                LCD_Write_String("Up: Stop FL2");
                
                              
                for (int i_3 = 0; i_3 < tasksUp; i_3++){
                    
                    sprintf(buffer, "%d", queueUp[i_3]);
                    LCD_Set_Cursor(2, pLCD);
                    LCD_Write_String(buffer);
                    pLCD++;
                }        
                pLCD = 1;
                __delay_ms(3000);                
                /* Normal work */
                DOOR2 = 1;
                dataPanelUp();
                DOOR2 = 0;
                __delay_ms(300);

                
                if(tasksUp != 0){
                    DOWN_ASC = 0;       // Continue in mode UP.
                } else {
                    LCD_Clear();
                    LCD_Set_Cursor(1,1);
                    LCD_Write_String("Mode Up OFF"); 
                    return;
                }  
                
            }
            if (queueDown[0] == FLOOR2) {
                // STOP in floor2
                prevFL = FLOOR2;
                UP_ASC = 1;     // mode down.    
                
                for (int i1 = 0; i1 < tasksDown; i1++) {       // clear call 0 in array of calls.
                    queueDown[i1] = queueDown[i1 + 1];
                }
                tasksDown--;
                
                /* Write in LCD data */
                LCD_Clear();
                LCD_Set_Cursor(1,1);
                LCD_Write_String("Down: Stop FL2");
                
                              
                for (int i_4 = 0; i_4 < tasksDown; i_4++){
                    
                    sprintf(buffer, "%d", queueDown[i_4]);
                    LCD_Set_Cursor(2, pLCD);
                    LCD_Write_String(buffer);
                    pLCD++;
                }        
                pLCD = 1;
                __delay_ms(3000);
                
                DOOR2 = 1;
                dataPanelUp();
                DOOR2 = 0;
                __delay_ms(300);

                
                
                if(tasksDown != 0){
                    UP_ASC = 0;       // Continue in mode UP.
                } else {
                    LCD_Clear();
                    LCD_Set_Cursor(1,1);
                    LCD_Write_String("Mode Down OFF"); 
                    return;
                }         
                
            }
            
            
        }
        if(senFL3){
            nowFL = FLOOR3;
            if (queueUp[0] == FLOOR3) {
                // STOP in floor2
                prevFL = FLOOR3;
                
                DOWN_ASC = 1;   // mode up.
                   
                for (int i1 = 0; i1 < tasksUp; i1++) {       // clear call 0 in array of calls.
                    queueUp[i1] = queueUp[i1 + 1];
                }
                tasksUp--;
                
                /* Write in LCD data */
                LCD_Clear();
                LCD_Set_Cursor(1,1);
                LCD_Write_String("Up: Stop FL3");
               
                              
                for (int i_5 = 0; i_5 < tasksUp; i_5++){
                    
                    sprintf(buffer, "%d", queueUp[i_5]);
                    LCD_Set_Cursor(2, pLCD);
                    LCD_Write_String(buffer);
                    pLCD++;
                }        
                pLCD = 1;
                
                __delay_ms(3000);
                
                DOOR3 = 1;
                dataPanelUp();
                DOOR3 = 0;
                __delay_ms(300);

                
                if(tasksUp != 0){
                    DOWN_ASC = 0;       // Continue in mode UP.
                } else {
                    LCD_Clear();
                    LCD_Set_Cursor(1,1);
                    LCD_Write_String("Mode Up OFF");
                    return;
                }   
                
            }
            if (queueDown[0] == FLOOR3) {
                // STOP in floor3
                prevFL = FLOOR3;
                UP_ASC = 1;     // mode down.    
                
                for (int i1 = 0; i1 < tasksDown; i1++) {       // clear call 0 in array of calls.
                    queueDown[i1] = queueDown[i1 + 1];
                }
                tasksDown--;
                
                /* Write in LCD data */
                LCD_Clear();
                LCD_Set_Cursor(1,1);
                LCD_Write_String("Down: Stop FL3");                        
                
                for (int i_6 = 0; i_6 < tasksDown; i_6++){
                    
                    sprintf(buffer, "%d", queueDown[i_6]);
                    LCD_Set_Cursor(2, pLCD);
                    LCD_Write_String(buffer);
                    pLCD++;
                }        
                pLCD = 1;
                __delay_ms(3000);
                
                DOOR3 = 1;
                dataPanelUp();
                DOOR3 = 0;
                __delay_ms(300);

                
                if(tasksDown != 0){
                    UP_ASC = 0;       // Continue in mode UP.
                } else {
                    LCD_Clear();
                    LCD_Set_Cursor(1,1);
                    LCD_Write_String("Mode Up OFF"); 
                    return;
                }       
                
            }
        }
        if(senFL4){
            nowFL = FLOOR4;
            if (queueUp[0] == FLOOR4) {
                // STOP in floor2
                prevFL = FLOOR4;
                
                DOWN_ASC = 1;   // mode up.
                   
                for (int i1 = 0; i1 < tasksUp; i1++) {       // clear call 0 in array of calls.
                    queueUp[i1] = queueUp[i1 + 1];
                }
                tasksUp--;
                
                /* Write in LCD data */
                LCD_Clear();
                LCD_Set_Cursor(1,1);
                LCD_Write_String("Up: Stop FL4");                        
                            
                for (int i_7 = 0; i_7 < tasksUp; i_7++){
                    
                    sprintf(buffer, "%d", queueUp[i_7]);
                    LCD_Set_Cursor(2, pLCD);
                    LCD_Write_String(buffer);
                    pLCD++;
                }        
                pLCD = 1;
                __delay_ms(3000);
                
                DOOR4 = 1;
                dataPanelUp();
                DOOR4 = 0;
                __delay_ms(300);

                
                //DOWN_ASC = 0;   
                
            }
            if (queueDown[0] == FLOOR4) {
                // STOP in floor2
                prevFL = FLOOR4;
                UP_ASC = 1;     // mode down.    
                
                for (int i1 = 0; i1 < tasksDown; i1++) {       // clear call 0 in array of calls.
                    queueDown[i1] = queueDown[i1 + 1];
                }
                tasksDown--;
                
                /* Write in LCD data */
                LCD_Clear();
                LCD_Set_Cursor(1,1);
                LCD_Write_String("Down: Stop FL4");                        
                             
                for (int i_8 = 0; i_8 < tasksDown; i_8++){
                    
                    sprintf(buffer, "%d", queueDown[i_8]);
                    LCD_Set_Cursor(2, pLCD);
                    LCD_Write_String(buffer);
                    pLCD++;
                }        
                pLCD = 1;
                __delay_ms(3000);
                
                DOOR4 = 1;
                dataPanelUp();
                DOOR4 = 0;
                __delay_ms(300);

                
                if(tasksDown != 0){
                    UP_ASC = 0;       // Continue in mode UP.
                } else {
                    LCD_Clear();
                    LCD_Set_Cursor(1,1);
                    LCD_Write_String("Mode Up OFF"); 
                    return;
                }         
                
            }   
        }
}

int rutine_down(void){
    
    
    
    controlCalls();
    
        
    if (tasksDown == 0){        // when the calls is complete...
        LCD_Clear();
        LCD_Set_Cursor(1,1);
        LCD_Write_String("End Down Mode");
        __delay_ms(2000);
        if((tasksUp > 0) || callsU > 0){
            LCD_Set_Cursor(2,1);
            LCD_Write_String("callsInDown");
            LCD_Clear();
            
            
            for(int i = 0; i < callsU; i++){
                queueUp[tasksUp + i] = callsInDown[i];
            }
                            
            sort(&queueUp[0], tasksUp + callsU);                    // Ordenar de mayor a menor.
            tasksUp = noRepeat(&queueDown[0], tasksUp + callsU);    // Nueva lista sin repeticiones de ruta en modo bajada.
            
            for (int i = 0; i < tasksUp; i++){
                    
                    sprintf(buffer, "%d", queueUp[i]);
                    LCD_Set_Cursor(2, pLCD);
                    LCD_Write_String(buffer);
                    pLCD++;
            }        
            pLCD = 1;
            __delay_ms(2000);
            call_init = queueUp[0];
            for (int re = 0; re < tasksUp; re++){
                queueDown[re] = queueDown[re + 1];
            }
            tasksUp--;
                            
            /* Initialize mode Down */
            from_down_change = 1;         // set flag to indicate change from mode up to down.
            from_up_change = 0;
            return 1;
        }
        else{
            /* No calls en down mode and up mode, init waiting mode */
            LCD_Set_Cursor(2,1);
            LCD_Write_String("No callsInDown");
            
            call_init = 0;
            modeDown_F = 0;
            tasksDown = 0;    // clear tasksUp.
            
            return 1;
        }
    }
    return 0;
}

int rutine_up(void){
    
    
    controlCalls();              
    if (tasksUp == 0){        // when the calls is complete...
        LCD_Clear();
        LCD_Set_Cursor(1,1);
        LCD_Write_String("End Up Mode");
        __delay_ms(2000);
        if((tasksDown > 0) || callsD > 0){
            LCD_Set_Cursor(2,1);
            LCD_Write_String("callsInUp");
            LCD_Clear();
            
            for(int i = 0; i < callsD; i++){
                queueDown[tasksDown + i] = callsInUp[i];
            }
                            
            sort_down(&queueDown[0], tasksDown + callsD);                    // Ordenar de mayor a menor.
            tasksDown = noRepeat(&queueDown[0], tasksDown + callsD);    // Nueva lista sin repeticiones de ruta en modo bajada.
            
            /*
            for (int i = 0; i < tasksDown; i++){
                    
                    sprintf(buffer, "%d", queueDown[i]);
                    LCD_Set_Cursor(2, pLCD);
                    LCD_Write_String(buffer);
                    pLCD++;
            }        
            pLCD = 1;
            */
            
            
            
            call_init = queueDown[0];
            for (int re = 0; re < tasksDown; re++){
                queueDown[re] = queueDown[re + 1];
            }
            tasksDown--;
                            
            /* Initialize mode Down */
            from_up_change = 1;         // set flag to indicate change from mode up to down.
            from_down_change = 0;
            return 1;
        }
        else{
            /* No calls en down mode and up mode, init waiting mode */
            call_init = 0;
            modeUp_F = 0;
            tasksUp = 0;    // clear tasksUp.
            
            return 1;
        }
    }
    return 0;
    
}

void selectionMode(unsigned mode_s){
    if (mode_s == modeUpON){       
        /* Ejecutando motor de modo subida. */
        DOWN_ASC = 0;
        
        LCD_Clear();
        LCD_Set_Cursor(1,1);
        LCD_Write_String("[Mode Up]");
        for (int i = 0; i < tasksUp; i++){
                    
            sprintf(buffer, "%d", queueUp[i]);
            LCD_Set_Cursor(2, pLCD);
            LCD_Write_String(buffer);
            pLCD++;
        }
        pLCD = 1;

        while (1) {
            if(rutine_up()){
                UP_ASC = 1;
                DOWN_ASC = 1;
                break;
            }
                    
        }
        UP_ASC = 1;
        DOWN_ASC = 1;
    }
    else if(mode_s == modeDownON){
        /* Ejecutando motor de modo bajada. */
        UP_ASC = 0;
        
        LCD_Clear();
        LCD_Set_Cursor(1,1);
        LCD_Write_String("[Mode Down]");
        for (int i = 0; i < tasksDown; i++){
                    
            sprintf(buffer, "%d", queueDown[i]);
            LCD_Set_Cursor(2, pLCD);
            LCD_Write_String(buffer);
            pLCD++;
        }
        pLCD = 1;

        while (1) {
            if(rutine_down()){
                UP_ASC = 1;
                DOWN_ASC = 1;
                break;
            }
                                       
        }
        UP_ASC = 1;
        DOWN_ASC = 1;
    }
}

void modeControl(unsigned select_mode) {
    if (select_mode == modeUpON){
        modeDown_F = 0;
        modeUp_F = 1;        // Set flag mode up.
    
        /*Contadores llamadas a piso */
        callsD = 0;
        callsU = 0;
    
        /*Numero de tareas en cola */
        //tasksUp = 0;
        tasksDown = 0;
        
        LCD_Clear();
        LCD_Set_Cursor(1,1);
        LCD_Write_String("[Mode Up]");
    }
    else if (select_mode == modeDownON){
        modeUp_F = 0;
        modeDown_F = 1;     // Set flag mode down.
    
        callsD = 0;
        callsU = 0;
        
        //tasksDown = 0;
        tasksUp = 0;
        
        LCD_Clear();
        LCD_Set_Cursor(1,1);
        LCD_Write_String("[Mode Down]");
    }   
    
    switch (call_init) {
        case FLOOR1:
            LCD_Set_Cursor(2,1);
            LCD_Write_String("Move-to floor1");
            

            if (prevFL == FLOOR1) {
                nowFL = FLOOR1;
                DOOR1 = 1;
                dataPanelUp();
                DOOR1 = 0;
                __delay_ms(300);
                
                if (tasksUp == 0 && modeUp_F == 1){
                    break;
                }
                if (tasksDown == 0 && modeDown_F == 1){
                    break;
                }
                
                selectionMode(select_mode);
                          
            } else {
                do {
                    UP_ASC = 0;
                } while (!senFL1);
                nowFL = FLOOR1;
                UP_ASC = 1;

                DOOR1 = 1;
                dataPanelUp();
                DOOR1 = 0;
                __delay_ms(300);
                
                if (tasksUp == 0 && modeUp_F == 1){
                    break;
                }
                if (tasksDown == 0 && modeDown_F == 1){
                    break;
                }
                
                selectionMode(select_mode);
            }
            break;
            
        case FLOOR2:
            
            LCD_Set_Cursor(2,1);
            LCD_Write_String("Move-to floor2");
            if (prevFL == FLOOR2) {
                nowFL = FLOOR2;
                DOOR2 = 1;
                dataPanelUp();
                DOOR2 = 0;
                __delay_ms(300);
                
                if (tasksUp == 0 && modeUp_F == 1){
                    break;
                }
                if (tasksDown == 0 && modeDown_F == 1){
                    break;
                }

                // ARREGLO DE SUBIDA ERROR
                selectionMode(select_mode);

                
                
            } else {
                if (prevFL < FLOOR2){
                    do{
                        DOWN_ASC = 0; // UP = 1;
                        /* Posiblemente definir en este momento el flag de NOW_FL como FLOOR2 */
                    }while(!senFL2);
                    DOWN_ASC = 1;     // DOWN = 1; brake electric.
                    nowFL = FLOOR2;
                    DOOR2 = 1;
                    dataPanelUp();
                    DOOR2 = 0;
                    __delay_ms(300);
                    
                    if (tasksUp == 0 && modeUp_F == 1){
                        break;
                    }
                    if (tasksDown == 0 && modeDown_F == 1){
                        break;
                    }

                    selectionMode(select_mode);

                }else {
                    do {
                        UP_ASC = 0; // DOWN = 1;
                    }while(!senFL2);
                    UP_ASC = 1;     // DOWN = 1; brake electric.
                    nowFL = FLOOR2;
                    DOOR2 = 1;
                    dataPanelUp();
                    DOOR2 = 0;
                    __delay_ms(300);
                    
                    if (tasksUp == 0 && modeUp_F == 1){
                        break;
                    }
                    if (tasksDown == 0 && modeDown_F == 1){
                        break;
                    }

                    selectionMode(select_mode);

                }
                
            }
            break;
        case FLOOR3:
            nowFL = FLOOR3;
            LCD_Set_Cursor(2,1);
            LCD_Write_String("Move-to floor3");
            if (prevFL == FLOOR3) {
                
                DOOR3 = 1;
                dataPanelUp();
                DOOR3 = 0;
                __delay_ms(300);
                
                if (tasksUp == 0 && modeUp_F == 1){
                    break;
                }
                if (tasksDown == 0 && modeDown_F == 1){
                    break;
                }
                selectionMode(select_mode);
                           
            } else {
                if (prevFL < FLOOR3){
                    do{
                        DOWN_ASC = 0; // UP = 1;
                    }while(!senFL3);
                    DOWN_ASC = 1;     // DOWN = 1; brake electric.
                    nowFL = FLOOR3;
                    DOOR3 = 1;
                    dataPanelUp();
                    DOOR3 = 0;
                    __delay_ms(300);
                    
                    if (tasksUp == 0 && modeUp_F == 1){
                        break;
                    }
                    if (tasksDown == 0 && modeDown_F == 1){
                        break;
                    }

                    selectionMode(select_mode);

                }else {
                    do {
                        UP_ASC = 0; // DOWN = 1;
                    }while(!senFL3);
                    UP_ASC = 1;     // DOWN = 1; brake electric.
                    nowFL = FLOOR3;
                    DOOR3 = 1;
                    dataPanelUp();
                    DOOR3 = 0;
                    __delay_ms(300);
                    
                    if (tasksUp == 0 && modeUp_F == 1){
                        break;
                    }
                    if (tasksDown == 0 && modeDown_F == 1){
                        break;
                    }
                    
                    selectionMode(select_mode);
                }
                
            }
            break;
        
        case FLOOR4:
            LCD_Set_Cursor(2,1);
            LCD_Write_String("Move-to floor4");
            
            if(modeUp_F){
                select_mode = modeDownON;
            }
            
            if (prevFL == FLOOR4) {
                nowFL = FLOOR4;
                DOOR4 = 1;
                dataPanelUp();
                DOOR4 = 0;
                __delay_ms(300);
                if (tasksUp == 0 && modeUp_F == 1){
                    break;
                }
                if (tasksDown == 0 && modeDown_F == 1){
                    break;
                }

                selectionMode(select_mode);
                           
            } else {
                if (prevFL < FLOOR4){
                    do{
                        DOWN_ASC = 0; // UP = 1;
                    }while(!senFL4);
                    DOWN_ASC = 1;     // DOWN = 1; brake electric.
                    nowFL = FLOOR4;
                    DOOR4 = 1;
                    dataPanelUp();
                    DOOR4 = 0;
                    __delay_ms(300);
                    if (tasksUp == 0 && modeUp_F == 1){
                        break;
                    }
                    if (tasksDown == 0 && modeDown_F == 1){
                        break;
                    }

                    selectionMode(select_mode);
                    
                }else {
                    do {
                        UP_ASC = 0; // DOWN = 1;
                    }while(!senFL4);
                    UP_ASC = 1;     // DOWN = 1; brake electric.
                    nowFL = FLOOR4;
                    DOOR4 = 1;
                    dataPanelUp();
                    DOOR4 = 0;
                    __delay_ms(300);
                    if (tasksUp == 0 && modeUp_F == 1){
                        break;
                    }
                    if (tasksDown == 0 && modeDown_F == 1){
                        break;
                    }
                    selectionMode(select_mode);
                }
                
            }
            break;
    }
}

void main(void) {
    boot();
    //interruptsInit();
    bootAscensor();
    
    I2C_Master_Init();
    LCD_Init(0x4E); // Initialize LCD module with I2C address = 0x4E
 
    LCD_Set_Cursor(1, 1);
    LCD_Write_String("Inciando!");
    LCD_Set_Cursor(2, 1);
    LCD_Write_String("Modo Espera");
    
    /* Modo espera, sondea botones a piso pendiente a la llamada inicial. */ 
    while (1) {
        
        if (from_up_change){
            UP_ASC = 1;
            DOWN_ASC = 1;
            
            from_up_change = 0;
            modeControl(modeDownON);
            continue;
        }
        if(from_down_change){
            UP_ASC = 1;
            DOWN_ASC = 1;
            
            from_down_change = 0;
            modeControl(modeUpON);
        }
        
        /* Mode Up */
        if (btnUp1) {
            LCD_Clear();
            LCD_Set_Cursor(1,1);
            LCD_Write_String("btn up 1");
            nowFL = FLOOR1;
            call_init = FLOOR1;
            modeControl(modeUpON);
            
            LCD_Clear();
            LCD_Set_Cursor(1,1);
            LCD_Write_String("Mode up END");
            
            UP_ASC = 1;
            DOWN_ASC = 1;
            __delay_ms(1000);
            continue;
        }
        if (btnUp2) {
            LCD_Clear();
            LCD_Set_Cursor(1,1);
            LCD_Write_String("btn up 2");
            
            nowFL = FLOOR2;     // Deifnimos como 'piso de ahora' para no tener problemas con interrupcuiones y tomar todas las llamadas.
            call_init = FLOOR2;
            modeControl(modeUpON);
            
            LCD_Clear();
            LCD_Set_Cursor(1,1);
            LCD_Write_String("Mode up END");
            UP_ASC = 1;
            DOWN_ASC = 1;
            __delay_ms(1000);
            continue;
        } 
        if (btnUp3) {
            LCD_Clear();
            LCD_Set_Cursor(1,1);
            LCD_Write_String("btn up 3"); 
             
            //nowFL = FLOOR3;
            call_init = FLOOR3;
            modeControl(modeUpON);
            
            LCD_Clear();
            LCD_Set_Cursor(1,1);
            LCD_Write_String("Mode up END");
            UP_ASC = 1;
            DOWN_ASC = 1;
            __delay_ms(1000);
            continue;
        }
        
        /* Mode down */
        if(btnDown2){
            LCD_Clear();
            LCD_Set_Cursor(1,1);
            LCD_Write_String("btn down 2");
            
            //nowFL = FLOOR2;
            call_init = FLOOR2;
            modeControl(modeDownON);
            
            LCD_Clear();
            LCD_Set_Cursor(1,1);
            LCD_Write_String("Mode down END");
            UP_ASC = 1;
            DOWN_ASC = 1;
            __delay_ms(1000);
            continue;
        }
        if (btnDown3) {
            LCD_Clear();
            LCD_Set_Cursor(1,1);
            LCD_Write_String("btn down 3");
            
            nowFL = FLOOR3;     // Deifnimos como 'piso de ahora' para no tener problemas con interrupcuiones y tomar todas las llamadas.
            call_init = FLOOR3;
            modeControl(modeDownON);
            
            LCD_Clear();
            LCD_Set_Cursor(1,1);
            LCD_Write_String("Mode down END");
            UP_ASC = 1;
            DOWN_ASC = 1;
            __delay_ms(1000);
            continue;
        } 
        if (btnDown4) {
            LCD_Clear();
            LCD_Set_Cursor(1,1);
            LCD_Write_String("btn down 4"); 
             
            nowFL = FLOOR4;
            call_init = FLOOR4;
            modeControl(modeDownON);
            
            LCD_Clear();
            LCD_Set_Cursor(1,1);
            LCD_Write_String("Mode down END");
            UP_ASC = 1;
            DOWN_ASC = 1;
            __delay_ms(1000);
            continue;
        }
        
    }

    return;
}
        
