/*
    Copyright (c) 2019, Etienne de Foras and the respective contributors
    All rights reserved.

    Use of this source code is governed by a MIT-style license that can be found
    in the LICENSE.txt file.
*/

#ifndef Initializers_
#define Initializers_

#include "Layer.h"
#include "Matrix.h"

class Initializers
{
public:
	static void XavierUniform(MatrixFloat &m,Index iInputSize,Index iOutputSize);
	static void Zero(MatrixFloat &m,Index iInputSize,Index iOutputSize);
};

#endif
