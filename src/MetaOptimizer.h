#ifndef MetaOptimizer_
#define MetaOptimizer_

#include "Net.h"
#include "NetTrain.h"

class MetaOptimizer
{
public:
	MetaOptimizer();
	~MetaOptimizer();
	
	void set_net(Net& net);
	void set_train(NetTrain& train);
	void set_nb_thread(int iNbThread); // default: use max available or if iNbThread set to zero
	void run();
	
private:
	static int run_thread(int iThread, MetaOptimizer* self);
	Net* _pNet;
	NetTrain* _pTrain;
	int _iNbThread;
};

#endif