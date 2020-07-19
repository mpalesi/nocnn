#ifndef __NOC_H__
#define __NOC_H__

#include <fstream>
#include <string>
#include <vector>
#include <map>
#include <set>
#include <cstring>
#include <cassert>

using namespace std;

// ----------------------------------------------------------------------

#define ROUTING_XY 1
#define ROUTING_FA 2

#define LT_WIRED    1
#define LT_WIRELESS 2
#define LT_GRS      3

#define MAP_ZIGZAG   1
#define MAP_DIAGONAL 2

// ----------------------------------------------------------------------

typedef struct EnergyComponents
{
  double e_comm_wired, e_comm_wireless, e_comm_grs;
  double e_mmem, e_lmem, e_comp;
  double e_comm_wired_leakage, e_comm_wireless_leakage, e_comm_grs_leakage;
  double e_mmem_leakage, e_lmem_leakage,  e_comp_leakage;
  
  EnergyComponents() :
    e_comm_wired(0.0), e_comm_wireless(0.0), e_comm_grs(0.0),
    e_mmem(0.0), e_lmem(0.0), e_comp(0.0),
    e_comm_wired_leakage(0.0), e_comm_wireless_leakage(0.0),
    e_comm_grs_leakage(0.0),
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
  int      link_type; // LT_WIRED, LT_WIRELESS
  set<int> comm_ids; // set of ids of communications that pass on the link
  long     total_load; // in bytes
} TLinkAttr;

typedef map<pair<int,int>, TLinkAttr> TLinks;

typedef vector<pair<int,int> > TPath;

// ----------------------------------------------------------------------

typedef pair<int,int> TCoordinate;
typedef set<int> TNodeSet;
typedef map<string, TNodeSet> TRadioHubSet;
typedef set<pair<int, int> > TGRSLinksPositionsSet;

// ----------------------------------------------------------------------

class NoC
{
 public:

  float        clock_frequency; // in Hz
  int          mesh_width, mesh_height;
  int          routing;
  int          local_memory_size; // in bytes
  int          link_width; // in bits
  int          grslink_width; // in bits
  int          macopc, poolopc;
  int          router_latency; // in clock cycles
  float        memory_bandwidth; // in Bps
  double       epb_link, epb_router; // in joule
  double       epb_grslink; // in joule
  double       epb_mmemory; // in joule
  double       epop_mac, epop_pool; // in joule
  double       epb_lmemory; // in joule
  double       epb_wireless; // in joule
  double       leak_pwr_lmemory; // in watt
  double       leak_pwr_mmemory; // in watt
  double       leak_pwr_router; // in watt
  double       leak_pwr_link; // in watt
  double       leak_pwr_grslink; // in watt
  double       leak_pwr_pe; // in watt
  double       leak_pwr_wireless; // in watt
  TNodeSet     memory_interfaces;
  TRadioHubSet radio_hubs;
  float        wireless_bandwidth; // in bps
  bool         use_winoc;
  bool         use_grslinks;
  bool         use_multicast;
  int          node_mapping;
  
 public:


  void testRouting();
  void testEnergy();
  void testCommunication();
  void showLinksLoad();
  
  NoC();

  bool loadNoC(const string& fname);

  void showNoC();
  void showTopology();
  
  int getNumberOfCores();
  int getNumberOfMIs();

  TLatencyComponents getLatencyM2C(long nbytes, int dst_first, int dst_last, bool same_data);
  
  TLatencyComponents getLatencyC2M(long nbytes, int src_first, int src_last);

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
  TEnergyComponents getEnergyC2M(long nbytes_lm, long nbytes_mm,
				 int src_first, int src_last);
  TEnergyComponents getEnergyC2C(long nbytes,
				 int src_first, int src_last,
				 int dst_first, int dst_last);
  TEnergyComponents getEnergyMC2C(long nbytes,
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
  bool searchMulticastUsage(ifstream& f);
  bool searchLink(ifstream& f, int& lw, double& epb, double& leakpwr);
  bool searchGRSLink(ifstream& f, int& lw, double& epb, double& leakpwr);
  bool searchMemoryInterfaces(ifstream& f, TNodeSet& mi);
  void searchRadioHubs(ifstream& f, TRadioHubSet& rh);
  bool searchWiNoCData(ifstream& f, float& bw, double& epb, double& leakpwr);
  bool searchWiNoCUsage(ifstream& f);
  void searchGRSLinksPositions(ifstream& f, TGRSLinksPositionsSet& grslinks_pos);
  bool searchGRSLinksUsage(ifstream& f);
  bool searchNodeMapping(ifstream& f, int& nm);

  int coord2node(const pair<int,int>& coord);
  pair<int,int> node2coord(int node);
  int node2pe(int node);

  int getDistance(int n1, int n2);
  pair<int,int> getDistanceWiredWireless(int src_node, int dst_node);
  int getNumberOfHops(int n1, int n2);

  TPath getRoutingPath(int src_node, int dst_node);
  TPath getRoutingPathXY(int src_node, int dst_node);
  TPath getRoutingPathFA(int src_node, int dst_node);
  TPath getRoutingPathXYWired(int src_node, int dst_node);
  TPath getRoutingPathXYWireless(int src_node, int dst_node);

  int closestMI(int node);
  pair<int,int> getClosestRHNode(int node);

  void resetLinks();
  void addCommunication(int src_node, int dst_node, long nbytes, int comm_id);

  double getBottleneckLinkCapacity(int src_node, int dst_node,
				   long nbytes);
  long getCommunicationLatency(int src_node, int dst_node, long nbytes);

  void makeClosestMIMap();

  void makeLinks();
  void makeWiredLinks();
  void makeWirelessLinks();
  void makeGRSLinks();
  
  void makePE2Node();
  void makePE2NodeZigZag();
  void makePE2NodeDiagonal();
  void makeRH2Node();

  void computeEnergyComm(double& e_wired, double& e_wireless, double& e_grs);
  
  double computeEnergyLMem(long nbytes);
  double computeEnergyMMem(long nbytes);

  
  TLinks                links;
  map<int,int>          pe2node;
  map<string,int>       rh2node;
  map<int,int>          closest_mi;
  TGRSLinksPositionsSet grslinks_pos;
  float                 one_hop_wireless; // equivalent wired hops
};

#endif
