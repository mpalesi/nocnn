#include <iostream>
#include <iomanip>
#include <climits>
#include <cfloat>
#include "noc.h"


// ----------------------------------------------------------------------

NoC::NoC(void)
{
}

// ----------------------------------------------------------------------

bool NoC::searchClock(ifstream& f, float& cf)
{
  f.clear();
  f.seekg(0, ios::beg);

  string line;
  while (!f.eof())
    {
      getline(f, line);
      if (sscanf(line.c_str(), "clock: %f", &cf) == 1)
	return true;
    }

  return false;
}

// ----------------------------------------------------------------------

bool NoC::searchNoCSize(ifstream& f, int& w, int& h)
{
  f.clear();
  f.seekg(0, ios::beg);

  string line;
  while (!f.eof())
    {
      getline(f, line);
      if (sscanf(line.c_str(), "size: %dx%d", &w, &h) == 2)
	return true;
    }

  return false;
}

// ----------------------------------------------------------------------

bool NoC::searchRouting(ifstream& f, int& ra)
{
  char rstr[256];
  
  f.clear();
  f.seekg(0, ios::beg);

  string line;
  while (!f.eof())
    {
      getline(f, line);
      if (sscanf(line.c_str(), "routing: %s", rstr) == 1)
	{
	  if (strcmp(rstr, "xy") == 0)
	    ra = ROUTING_XY;
	  else if (strcmp(rstr, "fa") == 0)
	    ra = ROUTING_FA;
	  else
	    return false;
	  
	  return true;
	}
    }

  return false;
}

// ----------------------------------------------------------------------

bool NoC::searchLocalMemory(ifstream& f, int& lms, double& epb, double& leakpwr)
{
  f.clear();
  f.seekg(0, ios::beg);

  string line;
  while (!f.eof())
    {
      getline(f, line);
      if (sscanf(line.c_str(), "local_memory: %d,%lf,%lf", &lms, &epb, &leakpwr) == 3)
	return true;
    }

  return false;
}

// ----------------------------------------------------------------------

bool NoC::searchMainMemory(ifstream& f, float& bw, double& epb, double& leakpwr)
{
  f.clear();
  f.seekg(0, ios::beg);

  string line;
  while (!f.eof())
    {
      getline(f, line);
      if (sscanf(line.c_str(), "main_memory: %f,%lf,%lf", &bw, &epb, &leakpwr) == 3)
	return true;
    }

  return false;
}

// ----------------------------------------------------------------------

bool NoC::searchRouter(ifstream& f, int& rl, double& epb, double& leakpwr)
{
  f.clear();
  f.seekg(0, ios::beg);

  string line;
  while (!f.eof())
    {
      getline(f, line);
      if (sscanf(line.c_str(), "router: %d,%lf,%lf", &rl, &epb, &leakpwr) == 3)
	return true;
    }

  return false;
}

// ----------------------------------------------------------------------

bool NoC::searchLink(ifstream& f, int& lw, double& epb, double& leakpwr)
{
  f.clear();
  f.seekg(0, ios::beg);

  string line;
  while (!f.eof())
    {
      getline(f, line);
      if (sscanf(line.c_str(), "link: %d,%lf,%lf", &lw, &epb, &leakpwr) == 3)
	return true;
    }

  return false;
}

// ----------------------------------------------------------------------

bool NoC::searchPE(ifstream& f, int& macopc, int& poolopc,
		   double& epop_mac, double& epop_pool,
		   double& leakpwr)
{
  f.clear();
  f.seekg(0, ios::beg);

  string line;
  while (!f.eof())
    {
      getline(f, line);
      if (sscanf(line.c_str(), "pe: %d,%d,%lf,%lf,%lf", &macopc, &poolopc,
		 &epop_mac, &epop_pool, &leakpwr) == 5)
	return true;
    }

  return false;
}

// ----------------------------------------------------------------------

bool NoC::searchMemoryInterfaces(ifstream& f, set<pair<int,int> >& mi)
{
  f.clear();
  f.seekg(0, ios::beg);

  string line;
  bool found = false;
  while (!f.eof() && !found)
    {
      getline(f, line);
      if (strncmp(line.c_str(), "memory_interface:", 17) == 0)
	found = true;
    }

  if (!found)
    return false;
  
  mi.clear();
  while (!f.eof())
    {
      getline(f, line);
      
      pair<int,int> pos;
      if (sscanf(line.c_str(), "%d,%d", &pos.first, &pos.second) == 2)
	mi.insert(pos);
    }

  return (mi.size() != 0);
}

// ----------------------------------------------------------------------

bool NoC::loadNoC(const string& fname)
{
  cout << "Reading " << fname << "..." << endl;
  
  ifstream f(fname);

  if (f.fail())
    return false;

  if (!searchClock(f, clock_frequency))
    {
      cerr << "Unspecified clock frequency or invalid format" << endl;
      return false;
    }

  if (!searchNoCSize(f, mesh_width, mesh_height))
    {
      cerr << "Unspecified NoC size or invalid format" << endl;
      return false;
    }

  if (!searchRouting(f, routing))
    {
      cerr << "Unspecified routing type or invalid format" << endl;
      return false;
    }

  if (!searchLocalMemory(f, local_memory_size, epb_lmemory, leak_pwr_lmemory))
    {
      cerr << "Unspecified local memory or invalid format" << endl;
      return false;
    }

  if (!searchMainMemory(f, memory_bandwidth, epb_mmemory, leak_pwr_mmemory))
    {
      cerr << "Unspecified main memory or invalid format" << endl;
      return false;
    }

  if (!searchPE(f, macopc, poolopc, epop_mac, epop_pool, leak_pwr_pe))
    {
      cerr << "Unspecified PE or invalid format" << endl;
      return false;
    }

  if (!searchRouter(f, router_latency, epb_router, leak_pwr_router))
    {
      cerr << "Unspecified router or invalid format" << endl;
      return false;
    }  
    
  if (!searchLink(f, link_width, epb_link, leak_pwr_link))
    {
      cerr << "Unspecified link or invalid format" << endl;
      return false;
    }  

  if (!searchMemoryInterfaces(f, memory_interfaces))
    {
      cerr << "Unspecified memory interfaces or invalid format" << endl;
      return false;
    }  

  makeLinks();

  makePE2Node();

  return true;
}

// ----------------------------------------------------------------------

int NoC::node2pe(int node)
{
  for (map<int,int>::iterator i=pe2node.begin(); i!=pe2node.end(); i++)
    if (i->second == node)
      return i->first;

  return -1;
}

// ----------------------------------------------------------------------

int NoC::coord2node(const pair<int,int>& coord)
{
  return coord.first * mesh_width + coord.second;
}

// ----------------------------------------------------------------------

pair<int,int> NoC::node2coord(int node)
{
  pair<int,int> coord(node / mesh_width, node % mesh_width);

  return coord;
}

// ----------------------------------------------------------------------

void NoC::makeLinks()
{
  links.clear();
  
  TLinkAttr lattr;
  lattr.total_load = 0;
  lattr.ncomms = 0;

  // horizontal links
  for (int r=0; r<mesh_height; r++)
    {
      for (int c=0; c<mesh_width-1; c++)
	{
	  pair<int,int> l(coord2node(pair<int,int>(r,c)),
			  coord2node(pair<int,int>(r,c+1)));
	  links[l] = lattr;
	}

      for (int c=1; c<mesh_width; c++)
	{
	  pair<int,int> l(coord2node(pair<int,int>(r,c)),
			  coord2node(pair<int,int>(r,c-1)));
	  links[l] = lattr;
	}
    }
  
  // vertical links
  for (int c=0; c<mesh_width; c++)
    {
      for (int r=0; r<mesh_height-1; r++)
	{
	  pair<int,int> l(coord2node(pair<int,int>(r,c)),
			  coord2node(pair<int,int>(r+1,c)));
	  links[l] = lattr;
	}

      for (int r=1; r<mesh_height; r++)
	{
	  pair<int,int> l(coord2node(pair<int,int>(r,c)),
			  coord2node(pair<int,int>(r-1,c)));
	  links[l] = lattr;
	}
    }
}

// ----------------------------------------------------------------------

void NoC::makePE2Node()
{
  pe2node.clear();

  int pe_count = 0;
  for (int r=0; r<mesh_height; r++)
    for (int c=0; c<mesh_width; c++)
      if (memory_interfaces.find(pair<int,int>(r,c)) == memory_interfaces.end())
	{
	  pe2node[pe_count] = coord2node(pair<int,int>(r,c));
	  pe_count++;
	}
}

// ----------------------------------------------------------------------

int NoC::getNumberOfCores()
{
  return (mesh_width * mesh_height) - memory_interfaces.size();
}

// ----------------------------------------------------------------------

int NoC::getNumberOfMIs()
{
  return memory_interfaces.size();
}

// ----------------------------------------------------------------------

void NoC::showNoC()
{
  cout << endl
       << "NoC" << endl
       << "==============================" << endl;

  cout << "clock frequency: " << (clock_frequency/1e6) << " MHz" << endl
       << "mesh size: " << mesh_width << "x" << mesh_height
       << " (" << getNumberOfCores() << " cores, "
       << getNumberOfMIs() << " memory interfaces)" << endl
       << "local memory size: " << (local_memory_size/1024) << " KB" << endl
       << "main memory bandwidth: " << (memory_bandwidth/1e9) << " GBps (" << getMainMemoryBandwidth() << " Bpc)" << endl
       << "link width / bandwidth: " << link_width << " bits / "
       << (clock_frequency * (link_width/8) / 1e9) << " GBps" << endl
       << "router latency: " << router_latency << " cycles" << endl
       << "MAC/Pool operations per cycle: " << macopc << ", " << poolopc << endl
       << "Energy per bit link/router: " << epb_link << "/" << epb_router << " J" << endl
       << "Energy per bit main memory: " << epb_mmemory << " J" << endl
       << "Energy per bit local memory: " << epb_lmemory << " J" << endl
       << "Energy per operation MAC/Pool: " << epop_mac << "/" << epop_pool << " J" << endl
       << "Leakage power link/router/PE/lmem/mmem: " << leak_pwr_link << "/" << leak_pwr_router << "/" << leak_pwr_pe << "/" << leak_pwr_lmemory << "/" << leak_pwr_mmemory << " W" << endl
       << "memory interfaces: ";

  for (set<pair<int,int> >::iterator i=memory_interfaces.begin();
       i!=memory_interfaces.end(); i++)
    cout << "(" << i->first << "," << i->second << ") ";
  cout << endl;
}

// ----------------------------------------------------------------------

void NoC::showTopology()
{
  for (int r=0; r<mesh_height; r++)
    {
      for (int c=0; c<mesh_width; c++)
	{
	  pair<int,int> coord(r,c);
	  
	  if (memory_interfaces.find(coord) != memory_interfaces.end())
	    cout << setw(4) << "MI";
	  else
	    cout << setw(4) << node2pe(coord2node(coord));
	}
      cout << endl;
    }
}

// ----------------------------------------------------------------------

int NoC::getMainMemoryBandwidth()
{
  return (memory_bandwidth/clock_frequency);
}

// ----------------------------------------------------------------------

void NoC::resetLinks()
{
  for (TLinks::iterator l=links.begin(); l!=links.end(); l++)
    {
      l->second.total_load = 0;
      l->second.ncomms = 0;
    }
}

// ----------------------------------------------------------------------

vector<pair<int,int> > NoC::getRoutingPath(int src_node, int dst_node)
{
  if (routing == ROUTING_XY)
    return getRoutingPathXY(src_node, dst_node);
  else if (routing == ROUTING_FA)
    return getRoutingPathFA(src_node, dst_node);
  else    
    assert(true);

  return vector<pair<int,int> >();
}

// ----------------------------------------------------------------------

vector<pair<int,int> > NoC::getRoutingPathXY(int src_node, int dst_node)
{
  vector<pair<int,int> > path;

  pair<int,int> coord_s = node2coord(src_node);
  pair<int,int> coord_d = node2coord(dst_node);

  int delta;
  
  // routing X
  if (coord_d.second > coord_s.second)
    delta = 1;
  else if (coord_d.second < coord_s.second)
    delta = -1;
  else delta = 0;

  int col = coord_s.second;
  while (col != coord_d.second)
    {
      int node1 = coord2node(pair<int,int>(coord_s.first, col));
      col += delta;
      int node2 = coord2node(pair<int,int>(coord_s.first, col));

      path.push_back(pair<int,int>(node1,node2));
    }

  // routing Y
  if (coord_d.first > coord_s.first)
    delta = 1;
  else if (coord_d.first < coord_s.first)
    delta = -1;
  else delta = 0;

  int row = coord_s.first;
  while (row != coord_d.first)
    {
      int node1 = coord2node(pair<int,int>(row, coord_s.second));
      row += delta;
      int node2 = coord2node(pair<int,int>(row, coord_s.second));

      path.push_back(pair<int,int>(node1,node2));
    }    
  
  return path;
}

// ----------------------------------------------------------------------

vector<pair<int,int> > NoC::getRoutingPathFA(int src_node, int dst_node)
{
  vector<pair<int,int> > path;

  pair<int,int> coord_s = node2coord(src_node);
  pair<int,int> coord_d = node2coord(dst_node);

  pair<int,int> coord = coord_s;
  
  while (coord != coord_d)
    {
      pair<int,int> coord_prev = coord;

      if (coord.first == coord_d.first) // vertical
	if (coord.second > coord_d.second)
	  coord.second--;
	else
	  coord.second++;
      else if (coord.second == coord_d.second) // horizontal
	if (coord.first > coord_d.first)
	  coord.first--;
	else
	  coord.first++;
      else if (coord_d.first > coord.first && coord_d.second > coord.second) // dst south east
	if ((src_node+dst_node+coord.first+coord.second) % 2)
	  coord.first++;
	else
	  coord.second++;
      else if (coord_d.first < coord.first && coord_d.second > coord.second) // dst south west
	if ((src_node+dst_node+coord.first+coord.second) % 2)
	  coord.first--;
	else
	  coord.second++;
      else if (coord_d.first < coord.first && coord_d.second < coord.second) // dst north west
	if ((src_node+dst_node+coord.first+coord.second) % 2)
	  coord.first--;
	else
	  coord.second--;
      else if (coord_d.first > coord.first && coord_d.second < coord.second) // dst north east
	if ((src_node+dst_node+coord.first+coord.second) % 2)
	  coord.first++;
	else
	  coord.second--;
      else
	assert(true);

      int node1 = coord2node(coord_prev);
      int node2 = coord2node(coord);

      path.push_back(pair<int,int>(node1,node2));
    }
  
  return path;
}

// ----------------------------------------------------------------------

void NoC::addCommunication(int src_node, int dst_node, long nbytes)
{
  vector<pair<int,int> > path = getRoutingPath(src_node, dst_node);

  assert(!path.empty());

  for (int i=0; i<path.size(); i++)
    {
      pair<int,int> l = path[i];
      assert(links.find(l) != links.end());
      links[l].ncomms++;
      links[l].total_load += nbytes;
    }
}

// ----------------------------------------------------------------------

double NoC::getBottleneckLinkCapacity(int src_node, int dst_node,
				      long nbytes)
{
  vector<pair<int,int> > path = getRoutingPath(src_node, dst_node);

  assert(!path.empty());

  double min_bw_utilization = DBL_MAX;
  double link_capacity = link_width / 8;  // capacity in bytes per cycle

  for (int i=0; i<path.size(); i++)
    {
      TLinkAttr lattr = links.at(path[i]);

      double utilization = (double)nbytes / lattr.total_load;

      double bw_utilization = link_capacity * utilization;
      /*
	cout << "src " << src_node << " dst " << dst_node << " nbytes " << nbytes
	<< " link " << path[i].first << "-->" << path[i].second
	<< " total_load " << lattr.total_load
	<< " bw_utilization " << bw_utilization
	<< " link_capacity " << link_capacity
	<< endl;
      */
      
      assert(bw_utilization <= link_capacity);
      
      if (bw_utilization < min_bw_utilization)
	min_bw_utilization = bw_utilization;
    }

  return min_bw_utilization;

  /*
  
    double min_capacity = DBL_MAX;
    double max_capacity = link_width/8; // Bytes per cycle 
    for (int i=0; i<path.size(); i++)
    {
    TLinkAttr lattr = links.at(path[i]);
    double link_bw = max_capacity * nbytes/lattr.total_load;  // Bytes per cycle
    assert(link_bw <= max_capacity);

    if (link_bw < min_capacity)
    min_capacity = link_bw;
    }

    return min_capacity;
  */
}

// ----------------------------------------------------------------------

long NoC::getCommunicationLatency(int src_node, int dst_node, long nbytes)
{
  double link_bw = getBottleneckLinkCapacity(src_node, dst_node, nbytes);

  /*
    cout << "[" << src_node << "-->" << dst_node << ", " << nbytes
    << "] " << (getDistance(src_node, dst_node)*router_latency) + (nbytes / link_bw)       << endl;
  */
  
  return (getDistance(src_node, dst_node)*router_latency) +
    (nbytes / link_bw);
}

// ----------------------------------------------------------------------

int NoC::getDistance(int n1, int n2)
{
  pair<int,int> n1c = node2coord(n1);
  pair<int,int> n2c = node2coord(n2);

  return abs(n1c.first - n2c.first) + abs(n1c.second - n2c.second);
}
		
// ----------------------------------------------------------------------

int NoC::closestMI(int node)
{
  vector<int> closest_list;

  int min_distance = INT_MAX;

  for (set<pair<int,int> >::iterator mi=memory_interfaces.begin();
       mi != memory_interfaces.end(); mi++)
    {
      int mi_node = coord2node(*mi);
      int distance = getDistance(node, mi_node);

      if (distance < min_distance)
	{
	  min_distance = distance;
	  closest_list.clear();
	  closest_list.push_back(mi_node);
	}
      else if (distance == min_distance)
	closest_list.push_back(mi_node);
    }

  assert(!closest_list.empty());

  return closest_list[rand() % closest_list.size()];
}

// ----------------------------------------------------------------------

TLatencyComponents NoC::getLatencyM2C(long nbytes, int dst_first, int dst_last)
{
  // map communications to links
  resetLinks();

  map<int,int> closest_mi;
  
  for (int d=dst_first; d<=dst_last; d++)
    {
      int dst_node = pe2node.at(d);
      int src_node = closestMI(dst_node);
      closest_mi[dst_node] = src_node;

      addCommunication(src_node, dst_node, nbytes);
    }

  // compute max latency
  long max_latency = -1;
  for (int d=dst_first; d<=dst_last; d++)
    {
      int dst_node = pe2node.at(d);

      long latency = getCommunicationLatency(closest_mi[dst_node], dst_node, nbytes);
      
      if (latency > max_latency)
	max_latency = latency;
    }
  
  // additional cycles to load data from main memory
  int mem_latency = nbytes/getMainMemoryBandwidth(); 

  TLatencyComponents lc;
  lc.l_comm = max_latency;
  lc.l_mmem = mem_latency;

  return lc;
}

// ----------------------------------------------------------------------

TLatencyComponents NoC::getLatencyC2C(long nbytes,
				      int src_first, int src_last,
				      int dst_first, int dst_last)
{
  // map communications to links
  resetLinks();

  for (int s=src_first; s<=src_last; s++)
    for (int d=dst_first; d<=dst_last; d++)
      if (s != d)
	addCommunication(pe2node.at(s), pe2node.at(d), nbytes);

  // compute max latency
  long max_latency = -1;
  for (int s=src_first; s<=src_last; s++)
    for (int d=dst_first; d<=dst_last; d++)
      if (s != d)
	{
	  long latency = getCommunicationLatency(pe2node.at(s), pe2node.at(d), nbytes);
	  if (latency > max_latency)
	    max_latency = latency;
	}

  TLatencyComponents lc;
  lc.l_comm = max_latency;
  lc.l_mmem = 0;
  return lc;
}

// ----------------------------------------------------------------------

TLatencyComponents NoC::getLatencyMC2C(long nbytes_from_memory, long nbytes_from_core,
				       int src_first, int src_last,
				       int dst_first, int dst_last)
{
  resetLinks();

  // map communications to links: traffic from core to core
  for (int s=src_first; s<=src_last; s++)
    for (int d=dst_first; d<=dst_last; d++)
      if (s != d)
	addCommunication(pe2node.at(s), pe2node.at(d), nbytes_from_core);

  // map communications to links: traffic from memory to core
  map<int,int> closest_mi;
  for (int d=dst_first; d<=dst_last; d++)
    {
      int dst_node = pe2node.at(d);
      int src_node = closestMI(dst_node);
      closest_mi[dst_node] = src_node;

      addCommunication(src_node, dst_node, nbytes_from_memory);
    }

  // compute max latency: core to core
  long max_latency = -1;
  for (int s=src_first; s<=src_last; s++)
    for (int d=dst_first; d<=dst_last; d++)
      if (s != d)
	{
	  long latency = getCommunicationLatency(pe2node.at(s), pe2node.at(d), nbytes_from_core);
	  if (latency > max_latency)
	    max_latency = latency;
	}

  
  // compute max latency: memory to core
  long mem_latency = nbytes_from_memory/getMainMemoryBandwidth(); // additional cycles to load data from main memory
  bool due_to_memory = false;
  for (int d=dst_first; d<=dst_last; d++)
    {
      int dst_node = pe2node.at(d);

      long latency = getCommunicationLatency(closest_mi[dst_node], dst_node,
					     nbytes_from_memory) +
	mem_latency;
      if (latency > max_latency)
	{
	  due_to_memory = true;
	  max_latency = latency;
	}
    }

  TLatencyComponents lc;
  if (due_to_memory)
    {
      lc.l_comm = max_latency - mem_latency;
      lc.l_mmem = mem_latency;
    }
  else
    {
      lc.l_comm = max_latency;
      lc.l_mmem = 0;
    }
  
  return lc;
}

// ----------------------------------------------------------------------

TLatencyComponents NoC::getLatencyMAC(long nmac)
{
  TLatencyComponents lc;

  lc.l_comp = nmac / macopc;
  
  return lc;
}

// ----------------------------------------------------------------------

TLatencyComponents NoC::getLatencyPool(long npool)
{
  TLatencyComponents lc;

  lc.l_comp = npool / poolopc;

  return lc;
}

// ----------------------------------------------------------------------

double NoC::computeEnergyComm(int dist, long nbytes)
{
  return (double)nbytes * 8 * (epb_link * dist + (epb_router * (dist+1)) );
}

// ----------------------------------------------------------------------

double NoC::computeEnergyMMem(long nbytes)
{
  return (double)nbytes * 8 * epb_mmemory;
}

// ----------------------------------------------------------------------

double NoC::computeEnergyLMem(long nbytes)
{
  return (double)nbytes * 8 * epb_lmemory;
}

// ----------------------------------------------------------------------

TEnergyComponents NoC::getEnergyM2C(long nbytes, int dst_first, int dst_last,
				    bool same_data)
{
  TEnergyComponents ec;

  if (same_data)
    ec.e_mmem += computeEnergyMMem(nbytes);
  
  for (int d=dst_first; d<=dst_last; d++)
    {
      int s = closestMI(pe2node.at(d));

      int distance = getDistance(s, pe2node.at(d));

      if (!same_data)
	ec.e_mmem += computeEnergyMMem(nbytes);
      
      ec.e_lmem += computeEnergyLMem(nbytes);
      ec.e_comm += computeEnergyComm(distance, nbytes);
    }

  return ec;
}
  
// ----------------------------------------------------------------------

TEnergyComponents NoC::getEnergyC2C(long nbytes,
				    int src_first, int src_last,
				    int dst_first, int dst_last)
{
  TEnergyComponents ec;

  for (int s=src_first; s<=src_last; s++)
    for (int d=dst_first; d<=dst_last; d++)
      if (s != d)
	{
	  int distance = getDistance(pe2node.at(s), pe2node.at(d));

	  ec.e_comm += computeEnergyComm(distance, nbytes);
	  ec.e_lmem += 2*computeEnergyLMem(nbytes); // read from lmem of src core write to lmem of dst core
	}
  
  return ec;
}

// ----------------------------------------------------------------------

TEnergyComponents NoC::getEnergyMAC(long nmac, int operand_size)
{

  TEnergyComponents ec;

  ec.e_comp += nmac * epop_mac;
  ec.e_lmem += computeEnergyLMem(nmac * 2 * (operand_size/8)); // each MAC has two operands
  
  return ec;
}

// ----------------------------------------------------------------------

TEnergyComponents NoC::getEnergyPool(long npool, int operand_size)
{
  TEnergyComponents ec;
  
  ec.e_comp += npool * epop_pool;  
  ec.e_lmem += computeEnergyLMem(npool * (operand_size/8));
  
  return ec;
}

// ----------------------------------------------------------------------

TEnergyComponents NoC::getEnergyMMemLeakage(int cycles)
{
  TEnergyComponents ec;

  ec.e_mmem_leakage += leak_pwr_mmemory * cycles / clock_frequency;

  return ec;
}

// ----------------------------------------------------------------------

TEnergyComponents NoC::getEnergyLMemLeakage(int ncores, int cycles)
{
  TEnergyComponents ec;

  ec.e_lmem_leakage += ncores * leak_pwr_lmemory * cycles / clock_frequency;
  
  return ec;
}

// ----------------------------------------------------------------------

// We assume 5-port router (4 links: link to PE is not taken into account)
TEnergyComponents NoC::getEnergyCommLeakage(int nrouters, int cycles)
{
  TEnergyComponents ec;

  ec.e_comm_leakage += ((nrouters * leak_pwr_router) + (nrouters * 4 * leak_pwr_link)) * cycles / clock_frequency;
  
  return ec;
}

// ----------------------------------------------------------------------

TEnergyComponents NoC::getEnergyCompLeakage(int ncores, int cycles)
{
  TEnergyComponents ec;

  ec.e_comp_leakage += ncores * leak_pwr_pe * cycles / clock_frequency;
  
  return ec;
}

// ----------------------------------------------------------------------
