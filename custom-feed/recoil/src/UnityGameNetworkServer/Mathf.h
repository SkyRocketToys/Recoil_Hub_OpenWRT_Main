/// <summary>
/// Mathf.h
/// 
/// Description:
/// Reimplementation of the CLASS 'Mathf' used by the UNITY ENGINE.
/// In order to convert the code for the GAME SERVER written in C#
/// with the UNITY ENGINE, to C++.
/// 
/// Created:  22/02/17
/// Authors:  Rodney Quaye
/// 
/// Copyright (c) 2017 Hotgen Ltd. All rights reserved.
/// </summary>

#ifndef _MATHF_H
#define _MATHF_H

class Mathf
{

public:
	static float Clamp(float value, float min, float max) {
		if (value > max) {
			return max;
		}
		else if (value < min) {
			return min;
		}
		else {
			return value;
		}
	}

	static int Clamp(int value, int min, int max) {
		if (value > max) {
			return max;
		}
		else if (value < min) {
			return min;
		}
		else {
			return value;
		}
	}

};

#endif 