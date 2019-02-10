#include <iostream>
#include <iomanip>
using namespace std;

#include "Net.h"

#include "NetTrainLearningRate.h"

int main()
{
    //build net
    Net net;
    net.add_layer("DenseAndBias",1,20);
    net.add_layer("Tanh",20,20);
    net.add_layer("DenseAndBias",20,20);
    net.add_layer("Tanh",20,20);
    net.add_layer("DenseAndBias",20,1);

    //train data
    MatrixFloat mTruth(64,1);
    MatrixFloat mSamples(64,1);
    for(int i=0;i<64;i++)
    {
        float x=i/10.f;
        mTruth(i,0)=sin(x);
        mSamples(i,0)=x;
    }

    cout << "Learning..." << endl;
    TrainOption tOpt;
    tOpt.epochs=20000;
    NetTrainLearningRate train;
    train.train(net,mSamples,mTruth,tOpt);

    //show results
    MatrixFloat mOnePredict(1,1), mOneSample(1,1), mOneTruth(1,1);
    for(int i=0;i<mSamples.size();i+=4) //show 16 samples
    {
        mOneSample(0,0)=mSamples(i,0);
        mOneTruth(0,0)=mTruth(i,0);
        net.forward(mOneSample,mOnePredict);
        cout << std::setprecision(4) << "x=" << mOneSample(0,0) << "\ttruth=" << mOneTruth(0,0) << "\tpredict=" << mOnePredict(0,0) <<endl;
    }
    return 0;
}
