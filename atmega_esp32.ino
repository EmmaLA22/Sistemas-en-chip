/*ñ
 * Control_motores_DC_ANSI_C.pdf.cpp
 *codigo para arduino ATMEGA 
 * Created: 02/05/2024 09:05:32 a. m.
 * Author : Emmanuel LechArr
 * código que ya quedo bien.
 */ 
//ROBOT USADO, SMART 000163 
//MEMORIA USADA 404 BYTES. 
#include <avr/io.h>
#include <Arduino_FreeRTOS.h>
#define F_CPU 16000000UL //DEFINICIÓN DE OSCILADOR PARA EL DELAY
#include "queue.h"
#include "task.h"
 
void Forward(); // función de mov enfrente
void Backward(); //función de mov atras
void Stopmotor(); //función inmovilidad
void Release(); //función desactivar motores
void enables();//funcion enable.
//-------------librerias para sensores
#include <Arduino.h>
//------------------------------------------------------
#include "src/MeSingleLineFollower.h"
#include "src/MeCollisionSensor.h"
#include "src/MeBarrierSensor.h"
#include <MeMegaPi.h>
//---funciones para sensores colision y barrera. 
MeBarrierSensor barrier_60(60);
MeBarrierSensor barrier_61(61);
MeBarrierSensor barrier_62(62);
MeCollisionSensor collision_65(65);
MeCollisionSensor collision_66(66);
//------------------------------------------------------
//definición de funciones motores ---------------------
void Forward(){ //función para mover hacia enfrente
  //MOTOR 1
  //PORTB |= (1<<5); //E1
  PORTC |= (1<<4); //L1
  PORTC &= ~(1<<5); //L2
  //MOTOR 2
  //PORTB |= (1<<6);//E2
  PORTC |= (1<<2);//L1
  PORTC &= ~(1<<3);//L2
  //MOTOR 3
  //PORTH |= (1<<4);//E3
  PORTG |= (1<<0);//L1
  PORTG &= ~(1<<1);//L2
  //MOTOR 4
  //PORTH |= (1<<5);//E4
  PORTC |= (1<<0);//L1
  PORTC &= ~(1<<1);//L2
}
void Backward(){ //función para mover hacia atras
  //MOTOR 1
  //PORTB |= (1<<5); //E1
  PORTC &= ~(1<<4); //L1
  PORTC |= (1<<5); //L2
  //MOTOR 2
  //PORTB |= (1<<6); //E2
  PORTC &= ~(1<<2); //L1
  PORTC |= (1<<3); //L2
  //MOTOR 3
  //PORTH |= (1<<4); //E3
  PORTG &= ~(1<<0); //L1
  PORTG |= (1<<1); //L2
  //MOTOR 4
  //PORTH |= (1<<5);//E4
  PORTC &= ~(1<<0);//L1
  PORTC |= (1<<1);//L2
}
void MovRight(){ //función para mover hacia atras
  //MOTOR 1 avanza hacia delante
  PORTC &= ~(1<<4); //L1
  PORTC |=  (1<<5); //L2
  //MOTOR 2 avanza hacia adelante
  PORTC &= ~(1<<2); //L1
  PORTC |=  (1<<3); //L2
  //MOTOR 3 avanza hacia atras
  PORTG &= ~(1<<1); //L1
  PORTG |=  (1<<0); //L2
  //MOTOR 4 avanza hacia adelante
  PORTC &= ~(1<<1);//L1
  PORTC |=  (1<<0);//L2
}
void MovLeft(){ //función para mover hacia atras
  //MOTOR 1
  PORTC |= (1<<4); //L1
  PORTC &= ~(1<<5); //L2
  //MOTOR 2
  PORTC |= (1<<2); //L1
  PORTC &= ~(1<<3); //L2
  //MOTOR 3
  PORTG &=  ~(1<<0); //L1
  PORTG |= (1<<1); //L2
  //MOTOR 4
  PORTC &= ~(1<<0);//L1
  PORTC |= (1<<1);//L2
}
void Stopmotor(){ //función para detener
  //MOTOR 1
  //PORTB |= (1<<5); //E1
  PORTC &= ~(1<<4); //L1
  PORTC &= ~(1<<5); //L2
  //MOTOR 2
  //PORTB |= (1<<6);//E2
  PORTC &= ~(1<<2);//L1
  PORTC &= ~(1<<3);//L2
  //MOTOR 3
  //PORTH |= (1<<4);//E3
  PORTG &= ~(1<<0);//L1
  PORTG &= ~(1<<1);//L2
  //MOTOR 4
  //PORTH |= (1<<5);//E4
  PORTC &= ~(1<<0);//L1
  PORTC &= ~(1<<1);//L2
}
void Release(){ //desactivar motor.
  //MOTOR 1
  PORTB &= ~(1<<5); //E1
  //MOTOR 2
  PORTB &= ~(1<<6);//E2
  //MOTOR 3
  PORTH &= ~(1<<4);//E3
  //MOTOR 4
  PORTH &= ~(1<<5);//E4
}
void enables (){
    //
    OCR1A = 64;   //  potencia a 25 %  
    OCR1B = 64;
    TCCR1A =  0xA1 ;  //act- OCR0A &  WGM00 -- 10100001
    TCCR1B =  0x03; //clock/64
    // 
    OCR4B = 64;
    OCR4C = 64;
    TCCR4A = 0x29; //activa pwm (WGM10) y OCR4c, OCR4b--00101001
    TCCR4B = 0x03;
}

void processSerial2Input() {
  if (Serial2.available() > 0) {
    String str = Serial2.readString();  // Leer la cadena del puerto serial 2
    str.trim();  // Eliminar espacios en blanco al principio y al final

    for (int i = 0; i < str.length(); i++) {
      char ch = str.charAt(i);
      switch (ch) {
        case '7':
          Forward();
          Serial.print("Hola");
          return; // Salir de la función si encontramos un match
        case '8':
          Backward();
          return;
        case '9':
          MovLeft();
          return;
        case 'C':
          MovRight();
          return;
        case '*':
          Stopmotor();
          return;
      }
    }
    Release(); // Si no se encuentra ningún match, se ejecuta Release
  }
}
//__________________________________________________________________________________________________________________________
  void SensoresColisionBarrier() {
  if ((collision_65.isCollision() == true) || (collision_66.isCollision() == true)) {
      Serial2.println("pause");
      
      Serial.println("pause");}
     else if (barrier_61.isBarried()) { // el de en medio
      Serial2.println("play");
      Serial.println("play");
    } else if (barrier_60.isBarried()) { // derecho de enfrente
      Serial2.println("siguiente");
    } else if (barrier_62.isBarried()) { // izquierdo de frente
      Serial2.println("anterior");
    } else {
   }
}
//__________________________________________________________________________________________________________________________

void setup(){
  //poner los pines como salida. 
  //DDRB &= 00110000; // sensor is input  
  //motor m1 
  DDRB |= (1<<5); //sensor is in output E1
  DDRC |= (1<<4); //salida l1
  DDRC |= (1<<5); //salida l2
  //motor m2 
  DDRB |= (1<<6); //salida E2
  DDRC |= (1<<2); //salida L1 
  DDRC |= (1<<3); //salida L2
  //motor m3
  DDRH |= (1<<4); //salida E3
  DDRG |= (1<<0); //salida L1
  DDRG |= (1<<1); //Salida L2
  //motor m4
  DDRH |= (1<<5); //salida E4
  DDRC |= (1<<0); //salida L1
  DDRC |= (1<<1); //salida L2 
  //activacion de Enables.
  enables();
       
//______________parte de la lectura y escritura del puerto serial.--------
  //inicialización del puerto serial
  Serial.begin(9600);
   Serial2.begin(9600);
  }
void loop() 
{
     processSerial2Input();
    SensoresColisionBarrier();

}
