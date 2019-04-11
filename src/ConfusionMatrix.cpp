/*
    Copyright (c) 2019, Etienne de Foras and the respective contributors
    All rights reserved.

    Use of this source code is governed by a MIT-style license that can be found
    in the LICENSE.txt file.
*/

#include "ConfusionMatrix.h"

#include <cmath>

///////////////////////////////////////////////////////////////////////////////
ClassificationResult ConfusionMatrix::compute(const MatrixFloat& mRef,const MatrixFloat& mTest,int iNbClass)
{
    if(iNbClass==0)
        iNbClass=(int)mRef.maxCoeff()+1; //guess the nb of class

    ClassificationResult cr;
    cr.mConfMat.resize(iNbClass,iNbClass);
    cr.mConfMat.setZero();

    for(unsigned int i=0;i<(unsigned int)mRef.rows();i++)
    {
        //threshold label
        int iLabel=(int)(std::roundf(mTest(i)));
        iLabel=std::min(iLabel,iNbClass-1);
        iLabel=std::max(iLabel,0);
        cr.mConfMat((unsigned int)mRef(i),iLabel)++;
    }

    //compute accuracy in percent
    double dTrace=cr.mConfMat.trace();
    double dSum=cr.mConfMat.sum();
    cr.accuracy=dTrace/dSum*100.;

    return cr;
}
///////////////////////////////////////////////////////////////////////////////
void ConfusionMatrix::toPercent(const MatrixFloat& mConf, MatrixFloat& mConfPercent)
{
    MatrixFloat mSumRow=rowWiseSum(mConf);
    mConfPercent=rowWiseDivide(mConf,mSumRow)*100.f;
}
///////////////////////////////////////////////////////////////////////////////

