/// <summary>
/// Vector3.h
/// 
/// Description:
/// Reimplementation of the CLASS 'Vector3' used by the UNITY ENGINE.
/// In order to convert the code for the GAME SERVER written in C#
/// with the UNITY ENGINE, to C++.
/// 
/// Created:  20/02/17
/// Authors:  Rodney Quaye
/// 
/// Copyright (c) 2017 Hotgen Ltd. All rights reserved.
/// </summary>

#ifndef _VECTOR3_H
#define _VECTOR3_H

class Vector3
{

	public:
		static Vector3 zero;

	public:
        float x;
        float y;
        float z;

		Vector3();

		Vector3(float x, float y, float z);

};

#endif 