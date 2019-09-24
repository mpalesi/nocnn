#include <iostream>
#include <fstream>
#include <cmath>
#include "cnn.h"

// ----------------------------------------------------------------------

CNN::CNN(void)
{
  layers.clear();
}

// ----------------------------------------------------------------------

bool CNN::findInputSize(ifstream& f, TFeatureMap& fm)
{
  string line;
  while (!f.eof())
    {
      getline(f, line);
      if (sscanf(line.c_str(), "input: %dx%dx%d %d",
		 &fm.w, &fm.h, &fm.ch, &fm.bits) == 4)
	return true;
    }

  return false;
}

// ----------------------------------------------------------------------

bool CNN::findFeatureMapBitSize(ifstream& f, int& fmbs)
{
  string line;
  while (!f.eof())
    {
      getline(f, line);
      if (sscanf(line.c_str(), "fm_bitsize: %d", &fmbs) == 1)
	return true;
    }

  return false;
}

// ----------------------------------------------------------------------

bool CNN::findLayer(ifstream& f, string& lname, int& ltype)
{
  string line;
  char name_str[256], type_str[256];
  
  while (!f.eof())
    {
      getline(f, line);
      if (sscanf(line.c_str(), "layer: %s %s",
		 name_str, type_str) == 2)
	{
	  lname = string(name_str);
	  
	  if (strcmp(type_str, "conv") == 0)
	    ltype = LT_CONV;
	  else if (strcmp(type_str, "avgpool") == 0)
	    ltype = LT_AVG_POOL;
	  else if (strcmp(type_str, "maxpool") == 0)
	    ltype = LT_MAX_POOL;
	  else if (strcmp(type_str, "fc") == 0)
	    ltype = LT_FC;
	  else if (strcmp(type_str, "convdw") == 0)
	    ltype = LT_CONV_DW;
	  else
	    {
	      cerr << "Invalid type '" << type_str << "'" << endl;
	      return false;
	    }
	  
	  return true;
	}
    }

  return false;  
}

// ----------------------------------------------------------------------

bool CNN::readConvAttr(ifstream& f, TConvAttr& attr)
{
  char padding_str[256];
  string line;
  
  getline(f, line);
  if (sscanf(line.c_str(), "%d %dx%d %d %d %s",
	     &attr.nf, &attr.w, &attr.h, &attr.bits, &attr.stride,
	     padding_str) == 6)
    {
      if (strcmp(padding_str, "yes") == 0)
	attr.padding = true;
      else if (strcmp(padding_str, "no") == 0)
	attr.padding = false;
      else
	{
	      cerr << "Invalid padding '" << padding_str << "'" << endl;
	      return false;
	}

      return true;
    }
  
  return false;
}

// ----------------------------------------------------------------------

bool CNN::readPoolAttr(ifstream& f, TPoolAttr& attr)
{
  string line;
  
  getline(f, line);
  if (sscanf(line.c_str(), "%dx%d %d %d",
	     &attr.w, &attr.h, &attr.bits, &attr.stride) == 4)
    return true;
      
  return false;
}

// ----------------------------------------------------------------------

bool CNN::readFCAttr(ifstream& f, TFullyConnectedAttr& attr)
{
  string line;
  
  getline(f, line);
  if (sscanf(line.c_str(), "%d %d",
	     &attr.n, &attr.bits) == 2)
    return true;
      
  return false;
}

// ----------------------------------------------------------------------

void CNN::computeOutputFM_Conv(TFeatureMap& ifm, TFeatureMap& ofm,
			       TConvAttr& filter)
{
  int px, py;

  if (filter.padding)
    {
      px = (ifm.w * (filter.stride - 1) + filter.w - filter.stride) / 2;
      py = (ifm.h * (filter.stride - 1) + filter.h - filter.stride) / 2;
    }
  else
    {
      px = 0;
      py = 0;
    }
  
  ofm.w = (int)ceil((double)(ifm.w + 2*px - filter.w)/filter.stride + 1);
  ofm.h = (int)ceil((double)(ifm.h + 2*py - filter.h)/filter.stride + 1);

  ofm.ch = filter.nf;

  ofm.bits = fm_bit_size; // (ifm.bits > filter.bits) ? ifm.bits : filter.bits;
}

// ----------------------------------------------------------------------

void CNN::computeOutputFM_FC(TFeatureMap& ifm, TFeatureMap& ofm,
			     TFullyConnectedAttr& attr)
{
  ofm.w = 1;
  ofm.h = attr.n;
  ofm.ch = 1;
  ofm.bits = (ifm.bits > attr.bits) ? ifm.bits : attr.bits;
}

// ----------------------------------------------------------------------

void CNN::computeOutputFM_Pool(TFeatureMap& ifm, TFeatureMap& ofm,
				TPoolAttr& attr)
{
  ofm.w = ifm.w / attr.stride; 
  ofm.h = ifm.h / attr.stride;
  ofm.ch = ifm.ch;
  ofm.bits = ifm.bits;
}

// ----------------------------------------------------------------------

void CNN::computeOutputFM(TLayer& layer)
{
  switch (layer.ltype)
    {
    case LT_CONV:
    case LT_CONV_DW:
      computeOutputFM_Conv(layer.input_fm, layer.output_fm, layer.filter);
      break;
    case LT_FC:
      computeOutputFM_FC(layer.input_fm, layer.output_fm, layer.fc);
      break;
    case LT_AVG_POOL:
    case LT_MAX_POOL:
      computeOutputFM_Pool(layer.input_fm, layer.output_fm, layer.pool);
    }
}

// ----------------------------------------------------------------------

bool CNN::loadCNN(const string& fname)
{
  cout << "Reading " << fname << "..." << endl;
  
  ifstream f(fname);

  if (f.fail())
    return false;

  layers.clear();
  
  TLayer layer;
  
  if (!findInputSize(f, layer.input_fm))
    {
      cerr << "Cannot find input size" << endl; 
      return false;
    }

  if (!findFeatureMapBitSize(f, fm_bit_size))
    {
      cerr << "Cannot find featuremap bit size" << endl; 
      return false;
    }

  while (!f.eof())
    {
      if (layers.size() != 0)
	layer.input_fm = layers[layers.size()-1].output_fm;
      
      if (findLayer(f, layer.lname, layer.ltype))
	{
	  if (layer.ltype == LT_CONV)
	    {
	      if (!readConvAttr(f, layer.filter))
		return false;
	      layer.filter.ch = layer.input_fm.ch;
	    }
	  else if (layer.ltype == LT_AVG_POOL || layer.ltype == LT_MAX_POOL)
	    {
	      if (!readPoolAttr(f, layer.pool))
		return false;
	    }
	  else if (layer.ltype == LT_FC)
	    {
	      if (!readFCAttr(f, layer.fc))
		return false;
	    }
	  else if (layer.ltype == LT_CONV_DW)
	    {
	      if (!readConvAttr(f, layer.filter))
		return false;
	      
	      assert(layer.filter.nf == layer.input_fm.ch);
	      
	      layer.filter.ch = 1;
	    }
	  else
	    {
	      cerr << "Invalid layer type: " << layer.lname << " "
		   << layer.ltype << endl;
	      return false;
	    }

	  computeOutputFM(layer);

	  layer.compression_ratio = 1.0; // default compression ratio
	  
	  layers.push_back(layer);
	}
      
    }
  
  return true;
}

// ----------------------------------------------------------------------

int CNN::findLayerName(const string& layer_name)
{
  for (int i=0; i<layers.size(); i++)
    if (layers[i].lname == layer_name)
      return i;

  return -1;
}

// ----------------------------------------------------------------------

bool CNN::loadCompressionRatios(const string& fname)
{
  cout << "Reading " << fname << "..." << endl;
  
  ifstream f(fname);

  if (f.fail())
    return false;

  string line;
  char   layer_name[256];
  double cr;
  while (!f.eof())
    {
      getline(f, line);
      if (sscanf(line.c_str(), "%s %lf", layer_name, &cr) == 2)
	{
	  int lidx = findLayerName(string(layer_name));
	  if (lidx == -1)
	    {
	      cerr << "Invalid compressed layer name '" << layer_name
		   << "'" << endl;
	      return false;
	    }
	  else
	    layers[lidx].compression_ratio = cr;	  
	}
    }
  
  return true;
}

// ----------------------------------------------------------------------

string CNN::ltype2str(int ltype)
{
  switch (ltype)
    {
    case LT_CONV:
      return "CONV";
    case LT_FC:
      return "FC";
    case LT_AVG_POOL:
      return "AVG-POOL";
    case LT_MAX_POOL:
      return "MAX-POOL";
    case LT_CONV_DW:
      return "CONV-DW";
    }

  return "???";
}

// ----------------------------------------------------------------------

long CNN::getWeightsSize(int l)
{
      if (layers[l].ltype == LT_CONV)
	return layers[l].filter.getSizeAll();
      else if (layers[l].ltype == LT_AVG_POOL || layers[l].ltype == LT_MAX_POOL)
	return 0;
      else if (layers[l].ltype == LT_FC)
	return layers[l].input_fm.w *layers[l].input_fm.h *layers[l].input_fm.ch * layers[l].fc.n * layers[l].fc.bits / 8;
      else
	return -1;
}

// ----------------------------------------------------------------------

void CNN::showCNN()
{
  cout << endl
       << "CNN" << endl
       << "==============================" << endl;
  
  for (int l=0; l<layers.size(); l++)
    {
      cout << "layer " << l << ": "
	   << layers[l].lname << " (" << ltype2str(layers[l].ltype) << ")"
	   << endl;

      if (layers[l].ltype == LT_CONV)
	cout << "(" << layers[l].filter.nf << " "
	     << layers[l].filter.w << "x" << layers[l].filter.h << " filters, "
	     << layers[l].filter.bits << " bits, "
	     << "stride " << layers[l].filter.stride << ", "
	     << (layers[l].filter.padding ? "padding" : "no padding")
	     << ")" << endl;
      else if (layers[l].ltype == LT_AVG_POOL || layers[l].ltype == LT_MAX_POOL)
	cout << "(" << layers[l].pool.w << "x" << layers[l].pool.h
	     << ", " << layers[l].pool.bits << " bits, "
	     << "stride " << layers[l].pool.stride
	     << ")" << endl;
      else if (layers[l].ltype == LT_FC)
	cout << "(" << layers[l].fc.n << " neurons, "
	     << layers[l].fc.bits << " bits"
	     << ")" << endl;
      else
	cout << "(unknown layer type)" << endl;
      
      cout << "in: "
	   << layers[l].input_fm.w << "x"
	   << layers[l].input_fm.h << "x"
	   << layers[l].input_fm.ch << " @ "
	   << layers[l].input_fm.bits << " bits"
	   << endl
	   << "out: "
	   << layers[l].output_fm.w << "x"
	   << layers[l].output_fm.h << "x"
	   << layers[l].output_fm.ch << " @ "
	   << layers[l].output_fm.bits << " bits"
	   << endl
	   << "Input/Output fmsize: "
	   << layers[l].input_fm.w * layers[l].input_fm.h * layers[l].input_fm.ch * layers[l].input_fm.bits / 8 / 1024 << "/"
	   << layers[l].output_fm.w * layers[l].output_fm.h * layers[l].output_fm.ch * layers[l].output_fm.bits / 8 / 1024 << " KBytes"
	   << ", weights: " << getWeightsSize(l)/1024 << " KBytes"
	   << ", compression ratio: " << layers[l].compression_ratio
	   << " (compressed weights: " << getWeightsSize(l)/1024/layers[l].compression_ratio << " KBytes)"
	   << endl;	

      cout << endl;
    }
}
