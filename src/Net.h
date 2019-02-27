#ifndef Net_
#define Net_

#include <vector>
#include <string>
using namespace std;

class Layer;
#include "Matrix.h"

class Net
{
public:
    Net();
    virtual ~Net();

	void clear();
	void init();

    void add_layer(string sType, int inSize, int outSize);
    void add_dropout_layer(int inSize, float fRatio);

    const vector<Layer*> layers() const;
    Layer* layer(size_t iLayer);

    void forward(const MatrixFloat& mIn,MatrixFloat& mOut) const;
    int classify(const MatrixFloat& mIn) const;
    void classify_all(const MatrixFloat& mIn, MatrixFloat& mClass) const;

	void set_train_mode(bool bTrainMode); // set to true if training, set to false if testing
private:
	bool _bTrainMode;
    vector<Layer*> _layers;
};

#endif
