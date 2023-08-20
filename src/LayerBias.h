/*
    Copyright (c) 2019, Etienne de Foras and the respective contributors
    All rights reserved.

    Use of this source code is governed by a MIT-style license that can be found
    in the LICENSE.txt file.
*/

#ifndef LayerBias_
#define LayerBias_

#include "Layer.h"
#include "Matrix.h"

namespace bee {
class LayerBias : public Layer
{
public:
    explicit LayerBias(const string& sBiasInitializer = "Zeros");
    virtual ~LayerBias();

    virtual Layer* clone() const override;

    virtual void init() override;
    virtual void forward(const MatrixFloat& mIn, MatrixFloat &mOut) override;
    virtual void backpropagation(const MatrixFloat &mIn,const MatrixFloat &mGradientOut, MatrixFloat &mGradientIn) override;
};
}
#endif
