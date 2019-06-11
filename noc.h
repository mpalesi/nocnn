#ifndef __NOC_H__
#define __NOC_H__

#include <fstream>
#include <string>
#include <vector>
#include <map>
#include <set>

using namespace std;

// ----------------------------------------------------------------------

#define ROUTING_XY 1
#define ROUTING_FA 2

// ----------------------------------------------------------------------

typedef struct EnergyComponents
{
  double e_comm, e_mmem, e_lmem, e_comp;
  double e_comm_leakage, e_mmem_leakage, e_lmem_leakage,  e_comp_leakage;
  
  EnergyComponents() :
    e_comm(0.0), e_mmem(0.0), e_lmem(0.0), e_comp(0.0),
    e_comm_leakage(0.0),
    e_mmem_leakage(0.0), e_lmem_leakage(0.0),
    e_comp_leakage(0.0) {}
} TEnergyComponents;

typedef struct LatencyComponents
{
  long l_comm, l_comp, l_mmem;

  LatencyComponents() :
    l_comm(0), l_comp(0), l_mmem(0) {}
} TLatencyComponents;

// ----------------------------------------------------------------------

typedef struct LinkAttr
{
  long ncomms; // number of communications using the link
  long total_load; 
} TLinkAttr;

typedef map<pair<int,int>, TLinkAttr> TLinks;

// ----------------------------------------------------------------------

class NoC
{
 public:

  float  clock_frequency; // in Hz
  int    mesh_width, mesh_height;
  int    routing;
  int    local_memory_size; // in bytes
  int    link_width; // in bits
  int    macopc, poolopc;
  int    router_latency; // in clock cycles
  float  memory_bandwidth; // in Bps
  double epb_link, epb_router; // in joule
  double epb_mmemory; // in joule
  double epop_mac, epop_pool; // in joule
  double epb_lmemory; // in joule
  double leak_pwr_lmemory; // in watt
  double leak_pwr_mmemory; // in watt
  double leak_pwr_router; // in watt
  double leak_pwr_link; // in watt
  double leak_pwr_pe; // in watt
  set<pair<int,int> > memory_interfaces;

  
 public:

  NoC();

  bool loadNoC(const string& fname);

  void showNoC();
  void showTopology();
  
  int getNumberOfCores();
  int getNumberOfMIs();

  TLatencyComponents getLatencyM2C(long nbytes, int dst_first, int dst_last);
  TLatencyComponents getLatencyC2C(long nbytes,
				   int src_first, int src_last,
				   int dst_first, int dst_last);
  TLatencyComponents getLatencyMC2C(long nbytes_from_memory, long nbytes_from_core,
				    int src_first, int src_last,
				    int dst_first, int dst_last);

  TLatencyComponents getLatencyMAC(long nmac);
  TLatencyComponents getLatencyPool(long npool);
  
  TEnergyComponents getEnergyM2C(long nbytes, int dst_first, int dst_last,
				 bool same_data);
  TEnergyComponents getEnergyC2C(long nbytes,
				 int src_first, int src_last,
				 int dst_first, int dst_last);
  TEnergyComponents getEnergyMAC(long nmac, int operand_size);
  TEnergyComponents getEnergyPool(long npool, int operand_size);

  TEnergyComponents getEnergyMMemLeakage(int cycles);
  TEnergyComponents getEnergyLMemLeakage(int ncores, int cycles);
  TEnergyComponents getEnergyCommLeakage(int nrouters, int cycles);
  TEnergyComponents getEnergyCompLeakage(int ncores, int cycles);

  int getMainMemoryBandwidth(); // memory bandwidth in byte per clock cycle

  
 private:

  bool searchClock(ifstream& f, float& cf);
  bool searchNoCSize(ifstream& f, int& w, int& h);
  bool searchRouting(ifstream& f, int& ra);
  bool searchLocalMemory(ifstream& f, int& lms, double& epb, double& leakpwr);
  bool searchMainMemory(ifstream& f, float& bw, double& epb, double& leakpwr);
  bool searchPE(ifstream& f, int& macopc, int& poolopc,
		double& epop_mac, double& epop_pool,
		double& leakpwr);
  bool searchRouter(ifstream& f, int& rl, double& epb, double& leakpwr);
  bool searchLink(ifstream& f, int& lw, double& epb, double& leakpwr);
  bool searchMemoryInterfaces(ifstream& f, set<pair<int,int> >& mi);
  
  int coord2node(const pair<int,int>& coord);
  pair<int,int> node2coord(int node);
  int node2pe(int node);

  int getDistance(int n1, int n2);
  vector<pair<int,int> > getRoutingPath(int src_node, int dst_node);
  vector<pair<int,int> > getRoutingPathXY(int src_node, int dst_node);
  vector<pair<int,int> > getRoutingPathFA(int src_node, int dst_node);
  
  int closestMI(int node);

  void resetLinks();
  void addCommunication(int src_node, int dst_node, long nbytes);

  double getBottleneckLinkCapacity(int src_node, int dst_node,
				   long nbytes);
  long getCommunicationLatency(int src_node, int dst_node, long nbytes);
    
  void makeLinks();
  void makePE2Node();

  double computeEnergyComm(int dist, long nbytes);
  double computeEnergyLMem(long nbytes);
  double computeEnergyMMem(long nbytes);
  
  TLinks links;
  map<int,int> pe2node;
};

#endif
