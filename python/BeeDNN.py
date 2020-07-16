"""
    Copyright (c) 2019, Etienne de Foras and the respective contributors
    All rights reserved.

    Use of this source code is governed by a MIT license that can be found
    in the LICENSE.txt file.
"""

import numpy as np
import copy

################################################################################################### Layers
class Layer:
  def __init__(self):
    self.dydx = 1.
    self.learnable = False # set to False to freeze layer or if not learnable
    self.training = False # set to False in testing mode or True in training mode
    self.learnBias= False;

  def forward(self,x):
    pass

  #compute input loss from output loss
  def backpropagation(self,dldy):
    return dldy * self.dydx

# see https://stats.stackexchange.com/questions/115258/comprehensive-list-of-activation-functions-in-neural-networks-with-pros-cons
class LayerAbsolute(Layer):
  def forward(self,x):
    if self.training:
      self.dydx = np.sign(x)
    return np.abs(x)
    
# see http://mathworld.wolfram.com/InverseHyperbolicSine.html    
class LayerArcSinh(Layer):
  def forward(self,x):
    if self.training:
      self.dydx = 1. / np.sqrt(1. + x * x)
    return np.arcsinh(x)

class LayerArcTan(Layer):
  def forward(self,x):
    if self.training:
      self.dydx = 1. / (1. + x * x)
    return np.arctan(x)

class LayerBent(Layer):
  def forward(self,x):
    if self.training:
      self.dydx = x/(2.*np.sqrt(x*x+1.))+1.
    return (np.sqrt(x*x+1.)-1.)*0.5+x

#see Binary Step as in: https://en.wikipedia.org/wiki/Activation_function
class LayerBinaryStep(Layer):
  def forward(self,x):
    if self.training:
      self.dydx = x * 0.
    return (x > 0.) * 1.

# see https://stats.stackexchange.com/questions/115258/comprehensive-list-of-activation-functions-in-neural-networks-with-pros-cons
class LayerBipolar(Layer):
  def forward(self,x):
    if self.training:
      self.dydx = x*0.
    return np.sign(x)

#see https://adl1995.github.io/an-overview-of-activation-functions-used-in-neural-networks.html
class LayerBipolarSigmoid(Layer):
  def forward(self,x):
    s=np.exp(x)
    if self.training:
      self.dydx=2.*s/((s+1.)*(s+1.))
    return (s-1.)/(s+1.)

# see https://stats.stackexchange.com/questions/115258/comprehensive-list-of-activation-functions-in-neural-networks-with-pros-cons
class LayerComplementaryLogLog(Layer):
  def forward(self,x):
    if self.training:
      self.dydx = np.exp(x - np.exp(x))
    return 1. - np.exp(-np.exp(x))

class LayerDivideBy256(Layer): # usefull for fixpt code conversion
  def forward(self,x):
    if self.training:
      self.dydx = 0.00390625 # 0.00390625 == 1./256.
    return x*0.00390625

# see Sigmoid-Weighted Linear Units for Neural Network Function ; Stefan Elfwinga Eiji Uchibea Kenji Doyab
class LayerdSiLU(Layer):
  def forward(self,x):
    ex=np.exp(-x)
    exinv=1./(1.+ex)
    if self.training:
      self.dydx = ex*exinv*exinv*(2.+x*(2.*ex*exinv-1.))
    return exinv*(1+x*ex*exinv)

class LayerExponential(Layer):
  def forward(self,x):
    e=np.exp(x)
    if self.training:
      self.dydx = e
    return e

class LayerGauss(Layer):
  def forward(self,x):
    u=np.exp(-x*x)
    if self.training:
      self.dydx = -2.*x*u
    return u

# see https://cs224d.stanford.edu/lecture_notes/LectureNotes3.pdf
class LayerHardTanh(Layer):
  def forward(self,x):
    if self.training:
      self.dydx=np.ones(x.shape,dtype=np.float32)
      self.dydx[x<-1.]=0.
      self.dydx[x>1.]=0.
    u=x
    u[x<-1.] = -1.
    u[x>1.] = 1.
    return u

# HardELU, ELU fixed point approximation ; Author is Minh Tri LE
class LayerHardELU(Layer):
  def forward(self,x):
    if self.training:
      self.dydx=np.ones(x.shape,dtype=np.float32)
      self.dydx[ (x<-0.) & (x>-2.) ]=0.5
      self.dydx[ x<-2. ]=0.
    u=x
    u[ (x<-0.) & (x>-2.) ] *= 0.5
    u[x<-2.] = -1.
    return u


# see  https://nn.readthedocs.io/en/rtd/transfer/ (lambda=0.5)	
class LayerHardShrink(Layer):
  def forward(self,x):
    if self.training:
      self.dydx=np.ones(x.shape,dtype=np.float32)
      self.dydx[(x<0.5) & (x>-0.5)]=0.
    u=x
    u[ (x<0.5) & (x>-0.5) ] = 0.
    return u	

class LayerIdentity(Layer):
  def forward(self,x):
    return x 

class LayerLeakyRELU(Layer):
  def forward(self,x):
    if self.training:
      self.dydx=np.ones(x.shape,dtype=np.float32)
      self.dydx[x<0.]=0.01
    u=x
    u[u<0.] *= 0.01
    return u

# easy to convert in fixedpoint (0.00390625= 1/256 = (1>>8) )
class LayerLeakyRELU256(Layer):
  def forward(self,x):
    if self.training:
      self.dydx=np.ones(x.shape,dtype=np.float32)
      self.dydx[x<0.]=0.00390625
    u=x
    u[u<0.] *= 0.00390625
    return u

#see LogSigmoid as in : https://nn.readthedocs.io/en/rtd/transfer/
class LayerLogSigmoid(Layer):
  def forward(self,x):
    if self.training:
      self.dydx = 1./(1.+np.exp(x))
    return np.log(1./(1+np.exp(-x)))

#see Logit as in : https://adl1995.github.io/an-overview-of-activation-functions-used-in-neural-networks.html
class LayerLogit(Layer):
  def forward(self,x):
    if self.training:
      self.dydx = -x/(x-1.)
    return np.log(x/(1.-x))

class LayerRELU(Layer):
  def forward(self,x):
    if self.training:
      self.dydx = (x > 0.) * 1.
    return (x > 0.) * x

class LayerRELU6(Layer):
  def forward(self,x):
    if self.training:
      self.dydx=np.ones(x.shape,dtype=np.float32)
      self.dydx[x<0.]=0.
      self.dydx[x>6.]=0.
    u=x
    u[x<0.] = 0.
    u[x>6.] = 6.
    return u

class LayerSigmoid(Layer):
  def forward(self,x):
    y = 1. / (1. + np.exp(-x))
    if self.training:
      self.dydx = y * (1. - y)
    return y

class LayerSoftPlus(Layer):
  def forward(self,x):
    if self.training:
      self.dydx = 1. / (1. + np.exp(-x))
    return np.log1p(np.exp(x))

class LayerSoftSign(Layer):
  def forward(self,x):
    d=1.+np.abs(x)
    if self.training:
      self.dydx = 1. / (d*d)
    return x/d

# see paper: Swish: A Self-Gated Activation Function
class LayerSwish(Layer):
  def forward(self,x):
    y = 1. / (1. + np.exp(-x))
    if self.training:
      self.dydx = y * (x + 1. - x * y)
    return x * y

# see Sigmoid-Weighted Linear Units for Neural Network Function ; Stefan Elfwinga Eiji Uchibea Kenji Doyab
class LayerSiLU(Layer):
  def forward(self,x):
    ex=np.exp(-x)
    exinv=1./(1.+ex)
    if self.training:
      self.dydx = exinv*(1+x*ex*exinv)
    return x *exinv

# see https://en.wikipedia.org/wiki/Activation_function
class LayerSin(Layer):
  def forward(self,x):
    if self.training:
      self.dydx = np.cos(x)
    return np.sin(x)

class LayerTanh(Layer):
  def forward(self,x):
    y = np.tanh(x)
    if self.training:
      self.dydx = 1. - y * y
    return y

class LayerTanhShrink(Layer):
  def forward(self,x):
    y = np.tanh(x)
    if self.training:
      self.dydx = y * y
    return x-y

####################################### special layers
class LayerGlobalBias(Layer):
  def __init__(self):
    super().__init__()
    self.learnBias= True;
    self.b = np.zeros(1,dtype=np.float32)

  def forward(self,x):
    return x + self.b[0]

  def backpropagation(self,dldy):
    self.dldb = np.atleast_1d(dldy.mean())
    return dldy

class LayerBias(Layer):
  def __init__(self,outSize):
    super().__init__()
    self.learnBias= True;
    self.b = np.zeros((1,outSize),dtype=np.float32)

  def forward(self,x):
    return x + self.b

  def backpropagation(self,dldy):
    self.dldb = np.atleast_2d(np.mean(dldy,axis=0))
    return dldy

class LayerGlobalGain(Layer):
  def __init__(self):
    super().__init__()
    self.learnable = True
    self.w = np.zeros(1,dtype=np.float32)
    self.w[0] = 1.
 
  def forward(self,x):
    if self.training:
      self.dydx = self.w[0]
      self.dydw = x
    return x * self.w[0]
  
  def backpropagation(self,dldy):
    self.dldw = np.atleast_1d((self.dydw * dldy).mean())
    return dldy * self.dydx

class LayerAddGaussianNoise(Layer):
  def __init__(self,stdev=0.1):
    super().__init__()
    self.stdev=stdev
 
  def forward(self,x):
    if self.training:
      x += np.random.normal(0, self.stdev,x.shape)
    return x

class LayerAddUniformNoise(Layer):
  def __init__(self,noise=0.1):
    super().__init__()
    self.noise=noise
 
  def forward(self,x):
    if self.training:
      x += np.random.rand(*(x.shape))*self.noise
    return x

class LayerDenseNoBias(Layer):
  def __init__(self,inSize,outSize):
    super().__init__()
    self.learnable = True
    a=np.sqrt(6./(inSize+outSize)) # Xavier uniform initialization
    self.w = a*(np.random.rand(inSize,outSize) * 2. - 1.)

  def forward(self,x):
    if self.training:
      self.dydw = x
    return x @ self.w

  def backpropagation(self,dldy):
    self.dldw = self.dydw.transpose() @ dldy
    self.dldw *= (1./(self.dydw.shape[0]))
    return dldy @ (self.w.transpose())

class LayerDense(Layer): # with bias
  def __init__(self,inSize,outSize):
    super().__init__()
    self.learnable = True
    self.learnBias = True
    a=np.sqrt(6./(inSize+outSize)) # Xavier uniform initialization
    self.w = a*(np.random.rand(inSize,outSize) * 2. - 1.)
    self.b = np.zeros((1,outSize),dtype=np.float32);
 
  def forward(self,x):
    if self.training:
      self.dydw = x
    return (x @ self.w) + self.b

  def backpropagation(self,dldy):
    self.dldw = self.dydw.transpose() @ dldy
    self.dldw *= (1./(self.dydw.shape[0]))
    self.dldb = np.atleast_2d(np.mean(dldy,axis=0))
    return dldy @ (self.w.transpose())

class LayerSoftmax(Layer):
  def __init__(self):
    super().__init__()

  def forward(self,x):
    max_row=np.max(x,axis=1)
    ex=np.exp(x-max_row[:,None])
    sum_row=np.atleast_2d(np.sum(ex,axis=1)).transpose()
    if self.training:
      self.dydx=-ex*(ex-sum_row)/(sum_row*sum_row)
    return ex/sum_row

class LayerDropout(Layer):
  def __init__(self,rate=0.3):
    super().__init__()
    self.rate=rate
 
  def forward(self,x):
    if self.training:
      z = np.random.binomial(size=(1,x.shape[1]), n=1, p= 1-self.rate)*(1./(1.-self.rate))
      x=x*z
    return x

############################ losses
class LayerLoss(Layer):
    def __init__(self):
      super().__init__()
    
    def set_truth(self,truth):
      self.truth = truth

class LossMSE(LayerLoss): #mean square error
  def __init__(self):
    super().__init__()

  def forward(self,x):
    d = x - self.truth
    if self.training:
      self.dydx = d
    return d * d * 0.5

class LossMAE(LayerLoss): #mean absolute error
  def __init__(self):
    super().__init__()
 
  def forward(self,x):
    d = x - self.truth
    if self.training:
      self.dydx = np.sign(d)
    return np.abs(d)

class LossLogCosh(LayerLoss):
  def __init__(self):
    super().__init__()
  
  def forward(self,x):
    d = x - self.truth
    if self.training:
      self.dydx = np.tanh(d)
    return  np.log(np.cosh(d))

# see https://ml-cheatsheet.readthedocs.io/en/latest/loss_functions.html
class LossBinaryCrossEntropy(LayerLoss):
  def __init__(self):
    super().__init__()
  
  def forward(self,x): #todo test x.size ==1
    p=x
    t=self.truth
    if self.training:
      self.dydx = np.atleast_2d(-t/np.maximum(1.e-8,p) +(1.-t)/np.maximum(1.e-8,1.-p) )
    return  np.atleast_2d(-t*np.log(np.maximum(p,1.e-8)) -(1.-t)*np.log(np.maximum(1.e-8,1.-p)))

# see https://gombru.github.io/2018/05/23/cross_entropy_loss
class LossCrossEntropy(LayerLoss):
  def __init__(self):
    super().__init__()
  
  def forward(self,x):
    p_max=np.maximum(x,1.e-8)
    t=self.truth
    if self.training:
      ump_max=np.maximum(1.-x,1.e-8)
      self.dydx = -t/p_max+(1.-t)/ump_max
    return  np.atleast_2d(np.mean(-t*np.log(p_max),axis=1))

################################################################################################### optimizers
class Optimizer:
  def optimize(self,w,dw):
    pass

class OptimizerStep(Optimizer):
  lr = 0.01
  def optimize(self,w,dw):
    return w - np.sign(dw) * self.lr

class OptimizerSGD(Optimizer):
  lr = 0.01
  def optimize(self,w,dw):
    return w - dw * self.lr
    
# Momentum from https://cs231n.github.io/neural-networks-3/#sgd
class OptimizerMomentum(Optimizer):
  lr = 0.01
  momentum = 0.9
  init = False
  def optimize(self,w,dw):
    if not self.init:
      self.v = 0. * w
      self.init = True

    self.v = self.v * self.momentum + dw * self.lr
    return w - self.v

# Nesterov from https://cs231n.github.io/neural-networks-3/#sgd
class OptimizerNesterov(Optimizer):
  lr = 0.01
  momentum = 0.9
  init = False
  def optimize(self,w,dw):
    if not self.init:
      self.v = 0. * w
      self.init = True

    v_prev = self.v # back this up
    self.v = self.momentum * self.v - self.lr * dw # velocity update stays the same
    w += -self.momentum * v_prev + (1. + self.momentum) * self.v # position update changes form
    return w
	
# RPROP-  as in : https://pdfs.semanticscholar.org/df9c/6a3843d54a28138a596acc85a96367a064c2.pdf
# or in paper : Improving the Rprop Learning Algorithm (Christian Igel and Michael Husken)
class OptimizerRPROPm(Optimizer):
  init = False
  
  def optimize(self,w,dw):
    if not self.init:
      self.mu = 0.0125 * np.ones(w.shape)
      self.olddw = dw
      self.init = True
      
    sg = np.sign(dw * self.olddw)
    self.mu[sg < 0.] *= 0.5
    self.mu[sg > 0.] *= 1.2
    np.maximum(self.mu,1.e-6,self.mu)
    np.minimum(self.mu,50.,self.mu)

    w = w - self.mu * np.sign(dw)
    self.olddw = dw
    return w

#from https://towardsdatascience.com/adam-latest-trends-in-deep-learning-optimization-6be9a291375c
class OptimizerAdam(Optimizer): #Adam, with first step bias correction
  beta1=0.9;
  beta2=0.999;
  lr = 0.001
  epsilon=1.e-8
  init = False

  def optimize(self,w,dw):
    if not self.init:
      self.v = 0. * dw
      self.m=self.v
      self.beta1_prod=self.beta1
      self.beta2_prod=self.beta2
      self.init = True

    self.m = self.m*self.beta1+(1.-self.beta1)*dw
    self.v = self.v*self.beta2+(1.-self.beta2)*(dw**2)
    w -= self.lr/(1.-self.beta1_prod) * self.m / (np.sqrt(self.v/(1.-self.beta2_prod)) + self.epsilon);
    self.beta1_prod=self.beta1_prod*self.beta1
    self.beta2_prod=self.beta2_prod*self.beta2
    return w

class OptimizerAdamW(Optimizer):
  beta1=0.9;
  beta2=0.999;
  lr = 0.01
  epsilon=1.e-8
  init = False
  lambda_regul=0.000001

  def optimize(self,w,dw):
    if not self.init:
      self.v = 0. * dw
      self.m=self.v
      self.beta1_prod=self.beta1
      self.beta2_prod=self.beta2
      self.init = True

    self.m = self.m*self.beta1+(1.-self.beta1)*dw
    self.v = self.v*self.beta2+(1.-self.beta2)*(dw**2)
    w -= self.lr/(1.-self.beta1_prod) * self.m / (np.sqrt(self.v/(1.-self.beta2_prod)) + self.epsilon)+self.lambda_regul*w;
    self.beta1_prod=self.beta1_prod*self.beta1
    self.beta2_prod=self.beta2_prod*self.beta2
    return w

class OptimizerAdamax(Optimizer):
  beta1=0.9;
  beta2=0.999;
  lr = 0.01
  epsilon=1.e-8
  init = False

  def optimize(self,w,dw):
    if not self.init:
      self.v = 0. * dw
      self.m=self.v
      self.beta1_prod=self.beta1
      self.init = True

    self.m = self.m*self.beta1+(1.-self.beta1)*dw
    self.v = np.maximum(self.beta2 * self.v, np.abs(dw))+self.epsilon
    w -= self.lr/(1.-self.beta1_prod) * self.m / self.v
    self.beta1_prod=self.beta1_prod*self.beta1
    return w

class OptimizerNadam(Optimizer):
  beta1=0.9;
  beta2=0.999;
  lr = 0.01
  epsilon=1.e-8
  init = False

  def optimize(self,w,dw):
    if not self.init:
      self.v = 0. * dw
      self.m=self.v
      self.beta1_prod=self.beta1
      self.beta2_prod=self.beta2
      self.init = True

    self.m = self.m*self.beta1+(1.-self.beta1)*dw
    self.v = self.v*self.beta2+(1.-self.beta2)*(dw**2)

    m_hat = self.m / (1 - self.beta1_prod) + (1 - self.beta1) * dw / (1 - self.beta1_prod)
    w -=  self.lr * m_hat / (np.sqrt(self.v / (1 - self.beta2_prod)) + self.epsilon)

    self.beta1_prod=self.beta1_prod*self.beta1
    self.beta2_prod=self.beta2_prod*self.beta2
    return w

class OptimizerAmsgrad(Optimizer):
  beta1=0.9;
  beta2=0.999;
  lr = 0.01
  epsilon=1.e-8
  init = False

  def optimize(self,w,dw):
    if not self.init:
      self.v = 0. * dw
      self.v_hat=self.v
      self.m=self.v
      self.init = True

    self.m = self.m*self.beta1+(1.-self.beta1)*dw
    self.v = self.v*self.beta2+(1.-self.beta2)*(dw**2)
    self.v_hat = np.maximum(self.v, self.v_hat)
    w -= self.lr * self.m / (np.sqrt(self.v_hat) + self.epsilon)
    return w

###################################################################################################
class Net:
  classification_mode=True
  def __init__(self):
    self.layers = list()

  def set_classification_mode(self,bClassification):
    self.classification_mode=bClassification;

  def append(self,layer):
    self.layers.append(layer)

  def forward(self,x):
    out = x
    for l in self.layers:
      s=np.sum(out)
      out = l.forward(out)

    if self.classification_mode:
      if out.shape[1]>1:
        out=np.argmax(out,axis=1)
      else:
        out=np.around(out)
        out=out.astype(int)
    return out
###################################################################################################
class NetTrain:
  optim = OptimizerMomentum()
  loss_layer = None
  epochs = 100
  batch_size = 32
  n = None
  test_data=None
  test_truth=None

  #keep best parameters
  keep_best=True
  best_net=None
  best_accuracy=0;

  def set_loss(self,layerloss):
    self.loss_layer = layerloss

  def set_optimizer(self, optim):
    self.optim = optim

  def set_test_data(self,test_data,test_truth):
    self.test_data=test_data
    self.test_truth=test_truth

  def set_keep_best(self,keep_best):
    self.keep_best=keep_best

  def forward_and_backward(self,x,t):
    #forward pass
    out = x
    for l in self.n.layers:
      out = l.forward(out)

	#backward pass
    self.loss_layer.set_truth(t)
    online_loss = self.loss_layer.forward(out)

    #update and back propagate gradient with loss
    dldy = self.loss_layer.dydx
    for i in range(len(self.n.layers) - 1,-1,-1):
      l = self.n.layers[i]
      dldy = l.backpropagation(dldy)

    return online_loss

  def train(self,n,sample,truth):
    self.n = n
    self.best_net= copy.deepcopy(n)
    nblayer = len(n.layers)
    nbsamples = len(sample)
    self.epoch_loss = np.zeros((0,0),dtype=np.float32)

    if(truth.shape[1]>1):
        truth_categorical=np.argmax(truth,axis=1)
    else:
        truth_categorical=truth

    #init optimizer
    optiml = list()
    for i in range(nblayer*2):
      optiml.append(copy.copy(self.optim)) #potential weight optimizer
      optiml.append(copy.copy(self.optim)) #potential bias optimizer

    if self.batch_size == 0:
      self.batch_size = nbsamples

    #train!
    for epoch in range(self.epochs):
      print("Epoch: ",epoch+1,"/",self.epochs,sep='', end='')

      #shuffle data
      perm = np.random.permutation(nbsamples)
      s2 = sample[perm,:]
      t2 = truth[perm,:]
      sumloss = 0.

      #set layers in train mode
      self.loss_layer.training=True
      for l in n.layers:
        l.training=True

      batch_start = 0
      while batch_start < nbsamples:
        
        #prepare batch data
        batch_end = min(batch_start + self.batch_size,nbsamples)
        x1 = s2[batch_start:batch_end,:]
        out = t2[batch_start:batch_end,:]
        batch_start += self.batch_size

        #forward pass and gradient computation
        online_loss = self.forward_and_backward(x1,out)
        
        #optimization
        for i in range(len(n.layers)):
          l = n.layers[i]
          if l.learnable:
            l.w = optiml[2*i].optimize(l.w,l.dldw)

          if l.learnBias:
            l.b = optiml[2*i+1].optimize(l.b,l.dldb)

        #save stats
        sumloss+=online_loss.sum()

      #set layers in test mode
      self.loss_layer.training=False
      for l in n.layers:
        l.training=False

      self.epoch_loss = np.append(self.epoch_loss,sumloss / nbsamples)

      #compute train accuracy (for now: classification only)
      predicted = n.forward(sample)

      #case of BinaryCrossEntropy
      if(n.classification_mode==True):
        accuracy=100.*np.mean(predicted==truth_categorical)
        print(" Train Accuracy: "  +format(accuracy, ".2f")  + "%", end='')

        #compute test accuracy (for now: classification only)
        if self.test_data is not None:
          predicted = n.forward(self.test_data)
          accuracy=100.*np.mean(predicted==self.test_truth)
          print(" Test Accuracy: "+format(accuracy, ".2f")+"%", end='')
        
        if(self.best_accuracy<accuracy):
          self.best_net=copy.deepcopy(n)
          self.best_accuracy=accuracy
          print(" (new best accuracy)",end='')

      print("")

    # if self.keep_best and (self.best_accuracy!=0):
    #   n=copy.deepcopy(self.best_net)