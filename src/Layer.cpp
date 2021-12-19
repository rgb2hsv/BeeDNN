/*
    Copyright (c) 2019, Etienne de Foras and the respective contributors
    All rights reserved.

    Use of this source code is governed by a MIT-style license that can be found
    in the LICENSE.txt file.
*/

#include "Layer.h"

////////////////////////////////////////////////////////////////
Layer::Layer(const string& sType):
_sType(sType)
{ 
	_bTrainMode = false;
	_bFirstLayer = false;

	_sWeightInitializer = "";
	_sBiasInitializer = "";
}
////////////////////////////////////////////////////////////////
Layer::~Layer()
{ }
////////////////////////////////////////////////////////////////
void Layer::init()
{ }
///////////////////////////////////////////////////////////////
string Layer::type() const
{
    return _sType;
}
///////////////////////////////////////////////////////////////
void Layer::set_first_layer(bool bFirstLayer)
{
	_bFirstLayer = bFirstLayer;
}
///////////////////////////////////////////////////////////////
void Layer::set_train_mode(bool bTrainMode)
{
	_bTrainMode = bTrainMode;
}
///////////////////////////////////////////////////////////////
bool Layer::has_weight() const
{
    return _weight.size()!=0.;
}
///////////////////////////////////////////////////////////////
MatrixFloat& Layer::weights()
{
    return _weight;
}
///////////////////////////////////////////////////////////////
MatrixFloat& Layer::gradient_weights()
{
    return _gradientWeight;
}
///////////////////////////////////////////////////////////////
bool Layer::has_bias() const
{
	return _bias.size() != 0.;
}
///////////////////////////////////////////////////////////////
MatrixFloat& Layer::bias()
{
	return _bias;
}
///////////////////////////////////////////////////////////////
MatrixFloat& Layer::gradient_bias()
{
	return _gradientBias;
}
///////////////////////////////////////////////////////////////
void Layer::set_weight_initializer(string sWeightInitializer)
{
	_sWeightInitializer = sWeightInitializer;
}
///////////////////////////////////////////////////////////////
void Layer::set_bias_initializer(string sBiasInitializer)
{
	_sBiasInitializer = sBiasInitializer;
}
///////////////////////////////////////////////////////////////
string Layer::weight_initializer() const
{
	return _sWeightInitializer;
}
///////////////////////////////////////////////////////////////
string Layer::bias_initializer() const
{
	return _sBiasInitializer;
}
///////////////////////////////////////////////////////////////