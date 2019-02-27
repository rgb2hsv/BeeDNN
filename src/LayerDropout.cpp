#include "LayerDropout.h"

///////////////////////////////////////////////////////////////////////////////
LayerDropout::LayerDropout(int iSize,float fRate):
    Layer(0,0,"Dropout"),
	_fRate(fRate)
{
	create_mask(iSize);
}
///////////////////////////////////////////////////////////////////////////////
LayerDropout::~LayerDropout()
{ }
///////////////////////////////////////////////////////////////////////////////
void LayerDropout::forward(const MatrixFloat& mIn,MatrixFloat& mOut) const
{
	if(_bTrainMode)
		mOut = mIn.cwiseProduct(_mask); //in train mode
	else
		mOut = mIn; // in test mode
}
///////////////////////////////////////////////////////////////////////////////
void LayerDropout::backpropagation(const MatrixFloat &mInput,const MatrixFloat &mDelta, float fLearningRate, MatrixFloat &mNewDelta)
{
    (void)fLearningRate;
	mNewDelta= mDelta.cwiseProduct(_mask);
	
	create_mask(mInput.cols());
}
///////////////////////////////////////////////////////////////////////////////
void LayerDropout::to_string(string& sBuffer)
{
    sBuffer+="Dropout: rate="+std::to_string(_fRate);
}
///////////////////////////////////////////////////////////////////////////////
void LayerDropout::create_mask(int iSize)
{
	_mask.resize(1, iSize);
	_mask.setConstant(1.f/(1.f - _fRate)); //inverse dropout as in: https://pgaleone.eu/deep-learning/regularization/2017/01/10/anaysis-of-dropout/);

	for (int i = 0; i < iSize; i++)
	{
		if ( (rand()/(double)RAND_MAX) < _fRate)
			_mask(0, i) = 0.f;
	}
}
///////////////////////////////////////////////////////////////////////////////



