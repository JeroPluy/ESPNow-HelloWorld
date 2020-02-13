/*
  Color.h - Library for defining Colors for LED.
  Created by Jero A. January 13. 2020.
  Released into the public domain.
*/

#ifndef Color_h
#define Color_h

#include "Arduino.h"

	class Color
	{
		public:
		  Color(short redPin, short greenPin, short bluePin);
		  void green();
		  void blue();
		  void red();
		  void violette();
		  void cyan();
		  void yellow();
		  void white();
		  void black();
		private:
		  short _greenPin;
		  short _bluePin; 
		  short _redPin; 
	};
#endif /* Color_h */