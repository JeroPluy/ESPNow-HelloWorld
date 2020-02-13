/*
  Color.cpp - Library for all Color modes for the RGB-LED-Pins.
  Created by Jero A. January 13. 2020.
  Released into the public domain.
*/

#include "Arduino.h"
#include "Color.h"

Color::Color(short redPin, short greenPin, short bluePin){
  pinMode(greenPin, OUTPUT);
  pinMode(bluePin, OUTPUT);
  pinMode(redPin, OUTPUT);
  _greenPin = greenPin;
  _bluePin = bluePin;
  _redPin = redPin;
  yellow();
}


void Color::green()
{  
  digitalWrite(_greenPin, HIGH); 
  digitalWrite(13, LOW); 
  digitalWrite(15, LOW); 

}

void Color::blue()
{
  digitalWrite(_greenPin, LOW); 
  digitalWrite(13, HIGH); 
  digitalWrite(15, LOW); 

}

void Color::red()
{
  digitalWrite(_greenPin, LOW); 
  digitalWrite(13, LOW); 
  digitalWrite(15, HIGH); 

}

void Color::violette()
{
  digitalWrite(12, LOW); 
  digitalWrite(13, HIGH); 
  digitalWrite(15, HIGH); 

}

void Color::cyan()
{
  digitalWrite(12, HIGH); 
  digitalWrite(13, HIGH); 
  digitalWrite(15, LOW); 

}

void Color::yellow()
{
  digitalWrite(12, HIGH); 
  digitalWrite(13, LOW); 
  digitalWrite(15, HIGH); 

}

void Color::white()
{
  digitalWrite(12, HIGH); 
  digitalWrite(13, HIGH); 
  digitalWrite(15, HIGH); 

}

void Color::black()
{
  digitalWrite(12, LOW); 
  digitalWrite(13, LOW); 
  digitalWrite(15, LOW); 
}
