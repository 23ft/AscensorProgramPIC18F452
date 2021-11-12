/*
Cambios pendientes [11 Nov]
 ~* Manejo de llamadas a piso con interrupciones, utilizar structura switch para cada caso en PORTB.
 ~* Flags de referencia para cada modo, para los botones de panel y llamadas a piso.
 ~* Administar finalizacion modo subida llevando a cabo el inicio d emodo bajada, ordenando las llamadas a piso en la cola de llamadas de bajada. 
 
 * Cambiar ordenamiento a al reves cuando cambies de modo. (line 479)
 ~* Terminar else de cada piso en modoUp
 * Prueba iniciar en funcion con bucle, saltar a otra con bucle y volver a la inicial. (prueba para comprender si es verdad la idea de que el codigo va y lo busca a la memoria)
 ~* Definir flag para "change_state" 1 si a cambiado de estado, 0 si no. (para manejo de variables tasksDown y tasksUp)
 
 *Funcion now_level cambiar a modeUpControl, modeUpControl cambiar a 'select_task', ahora en modeUpControl (antes now_level) determinar que cada vez
 que llegue a un sensor determinelo como nowlevel y condicione si en la lista queueUp[0] efectivamente es FLOOR1 para parar y ejecutar lo correspondiente
 al modo subida  
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

#define case_FL1_s 0x10
#define case_FL2_s 0x20 
#define case_FL3_s 0x40
#define case_FL4_s 0x80 


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

int tasksDown, tasksUp, calls, callsD, callFL_up, callFL_down, nowFL, prevFL, queueUp[100], queueDown[100], callsInUp[100];
 
int *UpTasks;         
unsigned int modeUp_F, modeDown_F, from_up_change, from_down_change, sen1_f, sen2_f, sen3_f, sen4_f;    

     

/* Prototypes functions */
/* ~~~ ~~~ ~~~ ~~~ ~~~ ~~~ ~~~ ~~~ ~~~ ~~~ ~~~ ~~~ ~~~ ~~~ ~~~ ~~~ ~~~ ~~~ ~~~ ~~~ ~~~ ~~~ ~~~ ~~~ ~~~ ~~~ ~~~ ~~~*/

#define size(x, type) (sizeof(x) / sizeof(type))

void bootAscensor(void);            // Funcion para incializar ascensor.

void rutine_up(void);               // FUncion rutina en motor de subida.
void dataPanelUp(void);             // Funcion para el manejo del panel en cada parada en modo subida.

void modeUpControl(void);           // Funcion motor del modo subida.
void modeDownControl(void);         // Funcion motor del modo bajada.

void modeUp(void);                  // Funcion modo subiendo.
void modeDown(void);                // Funcion modo bajando.

void sort(int *p, int sizes);      // Funcion ordenar array unidimensional, recibe puntero a arreglo y tama�o.
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
    calls = 0;           // Calls floors in mode up.
    callsD = 0;          // Calls floors in mode Down.
    callFL_up = 0;          // Piso de la ultima llamada.
    prevFL = 0;           // Last floor stoped.
    nowFL = 0;            // Piso actual.
    
    sen1_f = 0;           // Flag sensor piso 1.
    sen2_f = 0;           // Flag sensor piso 2.
    sen3_f = 0;           // Flag sensor piso 3.
    sen4_f = 0;           // Flag sensor piso 4.
    
    /* Flags */
    modeUp_F = 0;       // Mode up flag
    modeDown_F = 0;     // Mode down flag
    
    from_up_change = 0;
    from_down_change = 0;
    

    UpTasks = &queueUp[0];            // Pointer to array of task in mode up.
}

void interruptsInit(void) {
    RCONbits.IPEN = 0;      // Habilitiamos interrupciones sin mask.
    INTCONbits.GIE = 1;     // Enabled unmasked interrupts (Global INterrupt Enabled Bit)
    INTCONbits.PEIE = 1;    // Enabled peripheral interrups (Peripheral Interrupt Enabled bit)
}

void bootAscensor(void) {
    UP_ASC = 1;
    DOWN_ASC = 1;

    if (SenFL1 == 1) {
        prevFL = FLOOR1;
        return;
    } else {
        UP_ASC = 0;
        while (!SenFL1) {
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

    // Por ser XC8 para espacios con RAM peque�a no es recomendable trabajar con espacios dinamicos, requeriria muchos recursos.
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
void dataPanelUp(void) {
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

void now_level(){
    switch(PORTD){
        case case_FL1_s:
            sen1_f = 1;
            sen1_f = 1;

            nowFL = FLOOR1;         // Si el sensor del piso 1 esta activo, el piso actual es 1.
            break;
        case case_FL2_s:
            sen2_f = 1;
            nowFL = FLOOR2;
            break;
        case case_FL3_s:
            sen3_f = 1;
            nowFL = FLOOR3;
            break;
        case case_FL4_s:
            sen4_f = 1;
            nowFL = FLOOR4;
            break;    
    }
}


void modeUpControl(void) {

    static int cont = 0;

    switch (queueUp[cont]) {
        case FLOOR1:
            if (SenFL1) {               
                // STOP in floor1
                prevFL = FLOOR1;
                DOWN_ASC = 1;
                
                
                for (int i = 0; i < tasksUp; i++) {       // clear call 0 in array of calls.
                    queueUp[i] = queueUp[i + 1];
                }
                tasksUp--;
                DOOR1 = 1;
                
                dataPanelUp();
                
                DOOR1 = 0;
                __delay_ms(300);

                DOWN_ASC = 0;
            }
            break;

        case FLOOR2:
            if (SenFL2) {
                // STOP in floor2
                prevFL = FLOOR2;
                DOWN_ASC = 1;
                
                for (int i = 0; i < tasksUp; i++) {
                    queueUp[i] = queueUp[i + 1];
                }
                tasksUp--;
                
                DOOR2 = 1;
                dataPanelUp();
                DOOR2 = 0;
                __delay_ms(300);

                DOWN_ASC = 0;
            }
            break;

        case FLOOR3:
            if (SenFL3) {
                

                // STOP in floor3
                prevFL = FLOOR3;
                DOWN_ASC = 1;
                
                for (int i = 0; i < tasksUp; i++) {
                    queueUp[i] = queueUp[i + 1];
                }
                tasksUp--;
                
                DOOR3 = 1;             
                dataPanelUp();
                DOOR3 = 0;
                __delay_ms(300);

                DOWN_ASC = 0;
            }
            break;

        case FLOOR4:
            if (SenFL4) {                                
                // STOP in floor4
                prevFL = FLOOR4;
                DOWN_ASC = 1;
                for (int i = 0; i < tasksUp; i++) {
                    queueUp[i] = queueUp[i + 1];
                }
                tasksUp--;
                DOOR4 = 1;
                dataPanelUp();
                DOOR4 = 0;
                __delay_ms(300);
                
                // DOWN_ASC = 0; Se supone que del cuarto no sube mas, motores quedan 1-1
            }
            break;
    }

}

void rutine_up(void){
    
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
            from_up_change = 1;         // set flag to indicate change from mode up to down.
            
            return 1;
        }
        else{
            /* No calls en down mode and up mode, init waiting mode */
            callFL_up = 0;
            modeUp_F = 0;
            
            return 1;
        }
    }
    return 0;
    
}

void modeDown(void) {
    modeUp_F = 0;
    modeDown_F = 1;     // Set flag mode down.
    
    callsD = 0;
    
    tasksUp = 0;
}

void modeUp(void) {
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
            if (prevFL == FLOOR1) {
                
                DOOR1 = 1;
                dataPanelUp();
                DOOR1 = 0;
                __delay_ms(300);

                /* Modo subida inicando! */
                DOWN_ASC = 0;
                
                /* Ejecutando motor de modo subida. */
                while (1) {
                    if(rutine_up()){
                        break;
                    }
                    
                }
                           
            } else {
                do {
                    UP_ASC = 0;
                } while (!SenFL1);
                UP_ASC = 1;

                DOOR1 = 1;
                dataPanelUp();
                DOOR1 = 0;
                __delay_ms(300);
                
                DOWN_ASC = 0; // UP = 1
                while(1){
                    if(rutine_up()){
                        break;
                    }
                    
                }
            }
            break;
            
        case FLOOR2:
            if (prevFL == FLOOR2) {
                
                DOOR2 = 1;
                dataPanelUp();
                DOOR2 = 0;
                __delay_ms(300);

                /* Modo subida inicando! */
                DOWN_ASC = 0;

                while (1) {
                    if(rutine_up()){
                        break;
                    }
                                       
                }
                           
            } else {
                if (prevFL < FLOOR2){
                    do{
                        DOWN_ASC = 0; // UP = 1;
                        /* Posiblemente definir en este momento el flag de NOW_FL como FLOOR2 */
                    }while(!SenFL2);
                    DOWN_ASC = 1;     // DOWN = 1; brake electric.
                    
                    DOOR2 = 1;
                    dataPanelUp();
                    DOOR2 = 0;
                    __delay_ms(300);

                    /* Modo subida inicando! */
                    DOWN_ASC = 0;

                    while (1) {
                        if(rutine_up()){
                            break;
                        }
                    }
                    
                }else {
                    do {
                        UP_ASC = 0; // DOWN = 1;
                    }while(!SenFL2);
                    UP_ASC = 1;     // DOWN = 1; brake electric.
                    
                    DOOR2 = 1;
                    dataPanelUp();
                    DOOR2 = 0;
                    __delay_ms(300);

                    /* Modo subida inicando! */
                    DOWN_ASC = 0;

                    while (1) {
                        if(rutine_up()){
                            break;
                        }
                    }
                }
                
            }
            break;
        case FLOOR3:
            if (prevFL == FLOOR3) {
                
                DOOR3 = 1;
                dataPanelUp();
                DOOR3 = 0;
                __delay_ms(300);

                /* Modo subida inicando! */
                DOWN_ASC = 0;

                while (1) {
                    if(rutine_up()){
                        break;
                    }
                                       
                }
                           
            } else {
                if (prevFL < FLOOR3){
                    do{
                        DOWN_ASC = 0; // UP = 1;
                    }while(!SenFL3);
                    DOWN_ASC = 1;     // DOWN = 1; brake electric.
                    
                    DOOR3 = 1;
                    dataPanelUp();
                    DOOR3 = 0;
                    __delay_ms(300);

                    /* Modo subida inicando! */
                    DOWN_ASC = 0;

                    while (1) {
                        if(rutine_up()){
                            break;
                        }
                    }
                    
                }else {
                    do {
                        UP_ASC = 0; // DOWN = 1;
                    }while(!SenFL3);
                    UP_ASC = 1;     // DOWN = 1; brake electric.
                    
                    DOOR3 = 1;
                    dataPanelUp();
                    DOOR3 = 0;
                    __delay_ms(300);

                    /* Modo subida inicando! */
                    DOWN_ASC = 0;

                    while (1) {
                        if(rutine_up()){
                            break;
                        }
                    }
                }
                
            }
            break;
        
        case FLOOR4:               
            if (prevFL == FLOOR4) {
                
                DOOR4 = 1;
                dataPanelUp();
                DOOR4 = 0;
                __delay_ms(300);

                /* Modo subida inicando! */
                DOWN_ASC = 0;

                while (1) {
                    if(rutine_up()){
                        break;
                    }
                                       
                }
                           
            } else {
                if (prevFL < FLOOR4){
                    do{
                        DOWN_ASC = 0; // UP = 1;
                    }while(!SenFL4);
                    DOWN_ASC = 1;     // DOWN = 1; brake electric.
                    
                    DOOR4 = 1;
                    dataPanelUp();
                    DOOR4 = 0;
                    __delay_ms(300);

                    /* Modo subida inicando! */
                    DOWN_ASC = 0;

                    while (1) {
                        if(rutine_up()){
                            break;
                        }
                    }
                    
                }else {
                    do {
                        UP_ASC = 0; // DOWN = 1;
                    }while(!SenFL4);
                    UP_ASC = 1;     // DOWN = 1; brake electric.
                    
                    DOOR4 = 1;
                    dataPanelUp();
                    DOOR4 = 0;
                    __delay_ms(300);

                    /* Modo subida inicando! */
                    DOWN_ASC = 0;

                    while (1) {
                        if(rutine_up()){
                            break;
                        }
                    }
                }
                
            }
            break;
    }
}

void main(void) {
    boot();
    interruptsInit();
    bootAscensor();
    
    while (1) {
        
        if (from_up_change){
            from_up_change = 0;
            modeDown();
            continue;
        }
        
        if (btnUpFL1) {
            nowFL = FLOOR1;
            callFL_up = FLOOR1;
            modeUp();
            continue;
        }
        else if (btnUpFL2) {
            nowFL = FLOOR2;     // Deifnimos como 'piso de ahora' para no tener problemas con interrupcuiones y tomar todas las llamadas.
            callFL_up = FLOOR2;
            modeUp();
            continue;
        } 
        else if (btnUpFL3) {
            nowFL = FLOOR3;
            callFL_up = FLOOR3;
            modeUp();
            continue;
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
            }
            break;

        case 0x40: // (RB6)button down4 
            if (modeUp_F){
                    callsInUp[callsD] = FLOOR4;
                    callsD++;
            }
            break;
            
        case 0x80: // (RB7) PNL_BTN_STP
            break;
    }
}