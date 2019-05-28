#ifndef DNNEngineTinyDnn_
#define DNNEngineTinyDnn_

#include "MLEngine.h"

// forward declaration of tiny_dnn
namespace tiny_dnn {
template <typename NetType>
class network;
class sequential;
}

class DNNEngineTinyDnn : public MLEngine
{
public:
    DNNEngineTinyDnn();
    virtual ~DNNEngineTinyDnn() override;

    virtual void write(string& s) override;
    virtual void read(const string& s) override;

    virtual void clear() override;
    virtual void init() override;
    virtual void add_dense_layer(int inSize,int outSize, bool bWithBias) override;
    virtual void add_activation_layer(string sActivation) override;
    virtual void add_dropout_layer(int inSize,float fRatio) override;
    virtual void add_gaussian_noise_layer(int inSize,float fStd) override;
    virtual void add_globalgain_layer(int inSize,float fGain) override;
    virtual void add_poolaveraging1D_layer(int inSize,int iWindowSize) override;

    virtual void learn_epochs(const MatrixFloat& mSamples,const MatrixFloat& mTruth,const DNNTrainOption& dto) override;

    virtual void predict(const MatrixFloat& mIn, MatrixFloat& mOut) override;

private:   
    tiny_dnn::network<tiny_dnn::sequential>* _pNet;
};

#endif
