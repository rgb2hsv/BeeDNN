/*
    Copyright (c) 2019, Etienne de Foras and the respective contributors
    All rights reserved.

    Use of this source code is governed by a MIT-style license that can be found
    in the LICENSE.txt file.
*/

#include "Loss.h"

////////////////////////////////////////////////////////////////
Loss::Loss()
{ }
////////////////////////////////////////////////////////////////
Loss::~Loss()
{ }
//////////////////////////////////////////////////////////////////////////////
class LossMeanSquareError : public Loss
{
public:
	string name() const override
	{
		return "MeanSquareError";
	}
	
	float compute(const MatrixFloat& mPredicted,const MatrixFloat& mTarget) const
	{
		if (mTarget.rows() == 0)
			return 0.f;

		return (mPredicted -mTarget ).cwiseAbs2().sum() / mTarget.rows();
	}
	
	void compute_gradient(const MatrixFloat& mPredicted,const MatrixFloat& mTarget, MatrixFloat& mGradientLoss) const
	{
		mGradientLoss = mPredicted - mTarget;
	}
};
//////////////////////////////////////////////////////////////////////////////
class LossCrossEntropy : public Loss   // as in: https://gombru.github.io/2018/05/23/cross_entropy_loss/
{
public:
	string name() const override
	{
		return "CrossEntropy";
	}
	
	float compute(const MatrixFloat& mPredicted,const MatrixFloat& mTarget) const
	{
		return -(mTarget.cwiseProduct(cwiseLog(mPredicted.cwiseMax(1.e-8f))).sum()); //to avoid computing log(0)
	}
	
	void compute_gradient(const MatrixFloat& mPredicted, const MatrixFloat& mTarget, MatrixFloat& mGradientLoss) const
	{
		mGradientLoss = -(mTarget.cwiseQuotient(mPredicted.cwiseMax(1.e-8f))); //to avoid computing 1/0
	}
};
//////////////////////////////////////////////////////////////////////////////
class LossBinaryCrossEntropy : public Loss   // as in: https://ml-cheatsheet.readthedocs.io/en/latest/loss_functions.html
{
public:
	string name() const override
	{
		return "BinaryCrossEntropy";
	}

	float compute(const MatrixFloat& mPredicted, const MatrixFloat& mTarget) const
	{
		assert(mTarget.cols() == 1);
		assert(mTarget.size() == 1);
		assert(mPredicted.size() == 1);

		float p = mPredicted(0);
		float y = mTarget(0);
		return -(y*log(max(p,1.e-8f)) + (1.f-y)*log(max(1.e-8f,1.f-p)));
	}

	void compute_gradient(const MatrixFloat& mPredicted, const MatrixFloat& mTarget, MatrixFloat& mGradientLoss) const
	{
		assert(mTarget.cols() == 1);
		assert(mTarget.size() == 1);
		assert(mPredicted.size() == 1);

		float p = mPredicted(0);
		float y = mTarget(0);

		//https://math.stackexchange.com/questions/2503428/derivative-of-binary-cross-entropy-why-are-my-signs-not-right
		mGradientLoss.resize(1, 1);
        mGradientLoss(0)= -(y / max(p,1.e-8f) - (1.f - y) / max((1.f - p),1.e-8f));
	}
};
//////////////////////////////////////////////////////////////////////////////
Loss* create_loss(const string& sLoss)
{
    if(sLoss =="MeanSquareError")
        return new LossMeanSquareError;

    if(sLoss =="CrossEntropy")
        return new LossCrossEntropy;

	if (sLoss == "BinaryCrossEntropy")
		return new LossBinaryCrossEntropy;
	
	return nullptr;
}
//////////////////////////////////////////////////////////////////////////////
void list_loss_available(vector<string>& vsLoss)
{
    vsLoss.clear();

    vsLoss.push_back("MeanSquareError");
	vsLoss.push_back("CrossEntropy");
	vsLoss.push_back("BinaryCrossEntropy");
}
//////////////////////////////////////////////////////////////////////////////
