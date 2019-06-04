#include <iostream>
#include <cstdlib>
#include "cnn.h"
#include "noc.h"
#include "estimation.h"

using namespace std;

// ----------------------------------------------------------------------

typedef struct CommandLine
{
  string cnn_filename;
  string noc_filename;
} TCommandLine;

// ----------------------------------------------------------------------

void ShowCommandLineOptions(char* app_name)
{
  cerr << "Use " << app_name << " <cnn file name> <noc file name>" << endl;
}

// ----------------------------------------------------------------------

TCommandLine ProcessCommandLine(int argc, char* argv[])
{
  TCommandLine cl;

  if (argc != 3)
    {
      ShowCommandLineOptions(argv[0]);
      exit(-1);
    }
  
  cl.cnn_filename = string(argv[1]);
  cl.noc_filename = string(argv[2]);
  
  return cl;
}

// ----------------------------------------------------------------------

int main(int argc, char* argv[])
{
  TCommandLine cl = ProcessCommandLine(argc, argv);

  CNN cnn;

  
  if (!cnn.loadCNN(cl.cnn_filename))
    {
      cerr << "Error loading " << cl.cnn_filename << endl;
      return -1;
    }
  
  cnn.showCNN();
  
  NoC noc;

  if (!noc.loadNoC(cl.noc_filename))
    {
      cerr << "Error loading " << cl.noc_filename << endl;
      return -1;
    }

  noc.showNoC();
  noc.showTopology();


  Estimation estimation(noc, cnn);
  TGlobalStats stats;
  if (estimation.stime(stats))
    {
      estimation.showStats(stats);
    }
  else
    cerr << "Estimation failed!" << endl;
  
  return 0;
}
