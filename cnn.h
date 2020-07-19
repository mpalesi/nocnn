#ifndef __CNN_H__
#define __CNN_H__

#include <vector>
#include <string>
#include <cstring>
#include <cassert>

using namespace std;

// ----------------------------------------------------------------------

#define LT_CONV     1
#define LT_FC       2
#define LT_AVG_POOL 3
#define LT_MAX_POOL 4
#define LT_CONV_DW  5

// ----------------------------------------------------------------------

typedef struct FeatureMap
{
  int  w, h, ch; // width, height, and number of channels
  int  bits;
 
  int getSize() { // in bytes
    return w*h*ch*bits/8;
  }
  
} TFeatureMap;

// ----------------------------------------------------------------------

typedef struct ConvAttr
{
  int  w, h, ch; // width, height, and number of channels
  int  nf;       // number of filters
  int  bits;
  int  stride;
  bool padding;

  int getSizeSingle() { // size of a filter in bytes
    return w*h*ch*bits/8;
  }

  int getSizeAll() { // size of all filters in byte
    return getSizeSingle() * nf;
  }
  
} TConvAttr;

// ----------------------------------------------------------------------

typedef struct PoolAttr
{
  int stride;
  int w, h;
  int bits;
} TPoolAttr;

// ----------------------------------------------------------------------

typedef struct FullyConnectedAttr
{
  int n;
  int bits;
} TFullyConnectedAttr;

// ----------------------------------------------------------------------

typedef struct Layer
{
  string              lname;
  int                 ltype;
  TFeatureMap         input_fm;
  TFeatureMap         output_fm;

  TConvAttr           filter; // valid for LT_CONV and LT_CONV_DW
  TPoolAttr           pool;   // valid for LT_AVG_POOL and LT_MAX_POOL
  TFullyConnectedAttr fc;     // valid for LT_FC

  double              compression_ratio; // weights compression ratio
} TLayer;

// ----------------------------------------------------------------------

class CNN
{
 public:
  
  CNN();

  bool loadCNN(const string& fname);

  bool loadCompressionRatios(const string& fname);
  
  void showCNN();

  string ltype2str(int ltype);

  vector<TLayer> layers;

 private:
  
  bool findInputSize(ifstream& f, TFeatureMap& fm);
  bool findFeatureMapBitSize(ifstream& f, int& fmbs);
  bool findLayer(ifstream& f, string& lname, int& ltype);
  bool readConvAttr(ifstream& f, TConvAttr& attr);
  bool readPoolAttr(ifstream& f, TPoolAttr& attr);
  bool readFCAttr(ifstream& f, TFullyConnectedAttr& attr);
  void computeOutputFM_Conv(TFeatureMap& ifm, TFeatureMap& ofm,
			    TConvAttr& filter);
  void computeOutputFM_FC(TFeatureMap& ifm, TFeatureMap& ofm,
			  TFullyConnectedAttr& attr);
  void computeOutputFM_Pool(TFeatureMap& ifm, TFeatureMap& ofm,
			    TPoolAttr& attr);
  void computeOutputFM(TLayer& layer);

  long getWeightsSize(int layer);

  int findLayerName(const string& layer_name);

  int fm_bit_size;
  
};

// ----------------------------------------------------------------------

#endif
