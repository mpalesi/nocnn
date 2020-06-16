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

bool NoC::searchNodeMapping(ifstream& f, int& nm)
{
  char rstr[256];
  
  f.clear();
  f.seekg(0, ios::beg);

  string line;
  while (!f.eof())
    {
      getline(f, line);
      if (sscanf(line.c_str(), "node_mapping: %s", rstr) == 1)
	{
	  if (strcmp(rstr, "zigzag") == 0)
	    nm = MAP_ZIGZAG;
	  else if (strcmp(rstr, "diagonal") == 0)
	    nm = MAP_DIAGONAL;
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
      if (sscanf(line.c_str(), "local_memory: %d,%lf,%lf",
		 &lms, &epb, &leakpwr) == 3)
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

bool NoC::searchGRSLink(ifstream& f, int& lw, double& epb, double& leakpwr)
{
  f.clear();
  f.seekg(0, ios::beg);

  string line;
  while (!f.eof())
    {
      getline(f, line);
      if (sscanf(line.c_str(), "grslink: %d,%lf,%lf", &lw, &epb, &leakpwr) == 3)
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

bool NoC::searchMemoryInterfaces(ifstream& f, TNodeSet& mi)
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
      
      int node;
      if (sscanf(line.c_str(), "%d", &node) == 1)
	mi.insert(node);
    }

  return (mi.size() != 0);
}

// ----------------------------------------------------------------------

void NoC::searchRadioHubs(ifstream& f, TRadioHubSet& rh)
{
  f.clear();
  f.seekg(0, ios::beg);

  string line;
  bool found = false;
  while (!f.eof() && !found)
    {
      getline(f, line);
      if (strncmp(line.c_str(), "radio_hub:", 10) == 0)
	found = true;
    }

  if (!found)
    return;

  rh.clear();
  char rh_name[32];
  while (!f.eof())
    {
      getline(f, line);
      
      int node, x1, y1, x2, y2;
      if (sscanf(line.c_str(), "%s %d,%d %d,%d", rh_name, &x1, &y1, &x2, &y2) == 5)
	{
	  for (int x=x1; x<=x2; x++)
	    for (int y=y1; y<=y2; y++)
	      {
		node = coord2node(pair<int,int>(y, x));
		rh[string(rh_name)].insert(node);
	      }
	}
      else
	break;
    }
}

// ----------------------------------------------------------------------

void NoC::searchGRSLinksPositions(ifstream& f, TGRSLinksPositionsSet& grslinks_pos)
{
  f.clear();
  f.seekg(0, ios::beg);

  string line;
  bool found = false;
  while (!f.eof() && !found)
    {
      getline(f, line);
      if (strncmp(line.c_str(), "grslink_pos:", 12) == 0)
	found = true;
    }

  if (!found)
    return;

  grslinks_pos.clear();
  while (!f.eof())
    {
      getline(f, line);
      
      int  rc, first, last;
      char dir[16];
      if (sscanf(line.c_str(), "%s %d,%d..%d", dir, &rc, &first, &last) == 4)
	{
	  if (strcmp(dir, "h") == 0)
	    for (int r=first; r<=last; r++)
	      {
		int n1 = coord2node(pair<int,int>(r, rc));
		int n2 = coord2node(pair<int,int>(r, rc+1));
		grslinks_pos.insert(pair<int,int>(n1, n2));
		grslinks_pos.insert(pair<int,int>(n2, n1));
	      }
	  else if (strcmp(dir, "v") == 0)
	    for (int c=first; c<=last; c++)
	      {
		int n1 = coord2node(pair<int,int>(rc, c));
		int n2 = coord2node(pair<int,int>(rc+1, c));
		grslinks_pos.insert(pair<int,int>(n1, n2));
		grslinks_pos.insert(pair<int,int>(n2, n1));
	      }
	}
      else
	break;
    }
}

// ----------------------------------------------------------------------

bool NoC::searchWiNoCUsage(ifstream& f)
{
  f.clear();
  f.seekg(0, ios::beg);

  string line;
  while (!f.eof())
    {
      getline(f, line);
      if (strncmp(line.c_str(), "use_winoc", 9) == 0)
	return true;
    }

  return false;
}

// ----------------------------------------------------------------------

bool NoC::searchMulticastUsage(ifstream& f)
{
  f.clear();
  f.seekg(0, ios::beg);

  string line;
  while (!f.eof())
    {
      getline(f, line);
      if (strncmp(line.c_str(), "use_multicast", 13) == 0)
	return true;
    }

  return false;
}

// ----------------------------------------------------------------------

bool NoC::searchGRSLinksUsage(ifstream& f)
{
  f.clear();
  f.seekg(0, ios::beg);

  string line;
  while (!f.eof())
    {
      getline(f, line);
      if (strncmp(line.c_str(), "use_grslinks", 12) == 0)
	return true;
    }

  return false;
}

// ----------------------------------------------------------------------

bool NoC::searchWiNoCData(ifstream& f, float& bw, double& epb, double& leakpwr)
{
  f.clear();
  f.seekg(0, ios::beg);

  string line;
  while (!f.eof())
    {
      getline(f, line);
      if (sscanf(line.c_str(), "wireless: %f,%lf,%lf",
		 &bw, &epb, &leakpwr) == 3)
	return true;
    }

  return false;
}

// ----------------------------------------------------------------------

bool NoC::loadNoC(const string& fname)
{
  // initialize no mandatory data during (e.g., those related to winoc)
  epb_wireless = 0.0;
  leak_pwr_wireless = 0.0;
  wireless_bandwidth = 0.0;
  epb_grslink = 0.0;
  leak_pwr_grslink = 0.0;
  one_hop_wireless = INT_MAX;
  
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
  
  use_grslinks = searchGRSLinksUsage(f);
  if (use_grslinks)
    {
      searchGRSLinksPositions(f, grslinks_pos);
      if (!grslinks_pos.empty())
	if (!searchGRSLink(f, grslink_width, epb_grslink, leak_pwr_grslink))
	  {
	    cerr << "Unspecified GRS link or invalid format" << endl;
	    return false;
	  }  
    }
  
  use_multicast = searchMulticastUsage(f);
  
  use_winoc = searchWiNoCUsage(f);
  if (use_winoc)
    {
      searchRadioHubs(f, radio_hubs);
      if (!radio_hubs.empty())
	if (!searchWiNoCData(f, wireless_bandwidth, epb_wireless, leak_pwr_wireless))
	  {
	    cerr << "Unspecified wireless data or invalid format" << endl;
	    return false;
	  }
    }

  if (!searchNodeMapping(f, node_mapping))
    {
      cerr << "Unspecified node mapping or invalid format" << endl;
      return false;
    }

  makePE2Node(); // must call before makeLinks
  makeRH2Node(); // must call before makeLinks

  makeLinks();

  makeClosestMIMap(); // must call after makePE2Node

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

void NoC::makeClosestMIMap()
{
  closest_mi.clear();

  int num_pe = getNumberOfCores();
  for (int pe_id=0; pe_id<num_pe; pe_id++)
    {
      int node_id = pe2node.at(pe_id);
      closest_mi[node_id] = closestMI(node_id);
    }
}

// ----------------------------------------------------------------------

void NoC::makeLinks()
{
  makeWiredLinks();
  makeWirelessLinks();
  makeGRSLinks();
}

// ----------------------------------------------------------------------

void NoC::makeWiredLinks()
{
  links.clear();
  
  TLinkAttr lattr;
  lattr.total_load = 0;
  //  lattr.ncomms = 0;
  lattr.link_type = LT_WIRED;
  
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

  // links to radio-hubs
  if (radio_hubs.empty())
    return;
  
  for (TRadioHubSet::iterator rhi = radio_hubs.begin();
       rhi != radio_hubs.end(); rhi++)
    for (TNodeSet::iterator ni=rhi->second.begin();
	 ni != rhi->second.end(); ni++)
      {
	assert(rh2node.find(rhi->first) != rh2node.end());	
	int rh_node = rh2node[rhi->first];
	pair<int,int> l1(rh_node, *ni);
	pair<int,int> l2(*ni, rh_node);

	links[l1] = lattr;
	links[l2] = lattr;
      }
}

// ----------------------------------------------------------------------

void NoC::makeWirelessLinks()
{
  if (radio_hubs.empty())
    return;

  TLinkAttr lattr;

  lattr.link_type = LT_WIRELESS;
  //  lattr.ncomms = 0;
  lattr.total_load = 0;
    
  for (TRadioHubSet::iterator rh1 = radio_hubs.begin();
       rh1 != radio_hubs.end(); rh1++)
    for (TRadioHubSet::iterator rh2 = radio_hubs.begin();
       rh2 != radio_hubs.end(); rh2++)
      if (rh1 != rh2)
	{
	  assert(rh2node.find(rh1->first) != rh2node.end());
	  assert(rh2node.find(rh2->first) != rh2node.end());
	  pair<int,int> l(rh2node[rh1->first], rh2node[rh2->first]);
	  links[l] = lattr;
	}

  one_hop_wireless = clock_frequency * link_width / wireless_bandwidth;
}

// ----------------------------------------------------------------------

void NoC::makeGRSLinks()
{
  for (TGRSLinksPositionsSet::iterator l = grslinks_pos.begin();
       l != grslinks_pos.end(); l++)
    {
      assert(links.find(*l) != links.end());
      links[*l].link_type = LT_GRS;
    }
}

// ----------------------------------------------------------------------

void NoC::makePE2Node()
{
  if (node_mapping == MAP_ZIGZAG)
    makePE2NodeZigZag();
  else if (node_mapping == MAP_DIAGONAL)
    makePE2NodeDiagonal();
  else assert(false);
}

// ----------------------------------------------------------------------

void NoC::makePE2NodeZigZag()
{
  pe2node.clear();

  int pe_count = 0;
  for (int r=0; r<mesh_height; r++)
    for (int c=0; c<mesh_width; c++)
      {
	int node = coord2node(TCoordinate(r,c));
	
	if (memory_interfaces.find(node) == memory_interfaces.end())
	  {
	    pe2node[pe_count] = node;
	    pe_count++;
	  }
      }
}

// ----------------------------------------------------------------------

void NoC::makePE2NodeDiagonal()
{
  pe2node.clear();

  int pe_count = 0;
  for (int n=0; n<mesh_width*2; n++)
    {
      for (int i=0; i<=n; i++)
	{
	  int r = i;
	  int c = n - i;

	  if (r >= mesh_height || c >= mesh_width)
	    continue;

	  int node = coord2node(TCoordinate(r,c));
	  if (memory_interfaces.find(node) == memory_interfaces.end())
	    {
	      pe2node[pe_count] = node;
	      pe_count++;
	    }
	}
   }
}

// ----------------------------------------------------------------------

void NoC::makeRH2Node()
{
  int node = mesh_width * mesh_height;
  
  for (TRadioHubSet::iterator i=radio_hubs.begin();
       i != radio_hubs.end(); i++)
    {
      rh2node[i->first] = node;
      node++;
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
       << "NoC, Memory and PE" << endl
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
       << "use multicast: " << use_multicast << endl
       << "MAC/Pool operations per cycle: " << macopc << ", " << poolopc << endl
       << "Energy per bit link/router: " << epb_link << "/" << epb_router << " J" << endl
       << "Energy per bit main memory: " << epb_mmemory << " J" << endl
       << "Energy per bit local memory: " << epb_lmemory << " J" << endl
       << "Energy per operation MAC/Pool: " << epop_mac << "/" << epop_pool << " J" << endl
       << "Leakage power link/router/PE/lmem/mmem: " << leak_pwr_link << "/" << leak_pwr_router << "/" << leak_pwr_pe << "/" << leak_pwr_lmemory << "/" << leak_pwr_mmemory << " W" << endl
       << "memory interfaces: ";

  for (TNodeSet::iterator i=memory_interfaces.begin();
       i!=memory_interfaces.end(); i++)
    cout << *i << ", ";
  cout << endl;


  if (use_winoc)
    {
      cout << endl
	   << "WiNoC" << endl
	   << "==============================" << endl;

      cout << "Bandwidth: " << wireless_bandwidth/1.0E9 << " Gbps" << endl;
      cout << "One hop wireless: " << one_hop_wireless << " cycles" << endl;
      
      if (!radio_hubs.empty())
	{
	  cout << "WiNoC architecture" << endl;
	  for (TRadioHubSet::iterator i=radio_hubs.begin();
	       i != radio_hubs.end(); i++)
	    {
	      cout << "\t" << i->first << "(" << rh2node[i->first] << ") connected to ";
	      for (TNodeSet::iterator j=i->second.begin();
		   j != i->second.end(); j++)
		cout << *j << ", ";
	      cout << endl;
	    }
	}
    }

  if (use_grslinks)
    {
      cout << endl
	   << "GRS" << endl
	   << "==============================" << endl;
      
      cout << "Link width: " << grslink_width << " bits" << endl;
      if (!grslinks_pos.empty())
	{
	  cout << "GRS links: ";
	  for (TGRSLinksPositionsSet::iterator l=grslinks_pos.begin();
	       l != grslinks_pos.end(); l++)
	    cout << l->first << "->" << l->second << ", ";
	  cout << endl;
	}
    }
}

// ----------------------------------------------------------------------

void NoC::showTopology()
{
  cout << endl
       << "Topology" << endl
       << "==============================" << endl;
  
  for (int r=0; r<mesh_height; r++)
    {
      for (int c=0; c<mesh_width; c++)
	{
	  int node = coord2node(TCoordinate(r,c));
	  
	  if (memory_interfaces.find(node) != memory_interfaces.end())
	    cout << "(" << left << setw(3) << node << "MI   " << ") ";
	  else
	    cout << "(" << left << setw(3) << node << "PE" << setw(3) << node2pe(node) << ") ";
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
      //     l->second.ncomms = 0;
      l->second.comm_ids.clear();
    }
}

// ----------------------------------------------------------------------

TPath NoC::getRoutingPath(int src_node, int dst_node)
{
  if (routing == ROUTING_XY)
    return getRoutingPathXY(src_node, dst_node);
  else if (routing == ROUTING_FA)
    return getRoutingPathFA(src_node, dst_node);
  else    
    assert(true);

  return TPath();
}

// ----------------------------------------------------------------------

TPath NoC::getRoutingPathXY(int src_node, int dst_node)
{
  if (radio_hubs.empty())
    return getRoutingPathXYWired(src_node, dst_node);
  else
    return getRoutingPathXYWireless(src_node, dst_node);
}

// ----------------------------------------------------------------------

TPath NoC::getRoutingPathXYWired(int src_node, int dst_node)
{
  TPath path;

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
      int node1 = coord2node(pair<int,int>(row, coord_d.second));
      row += delta;
      int node2 = coord2node(pair<int,int>(row, coord_d.second));

      path.push_back(pair<int,int>(node1,node2));
    }    
  
  return path;
}

// ----------------------------------------------------------------------

TPath NoC::getRoutingPathXYWireless(int src_node, int dst_node)
{
  int hops_only_wired = getDistance(src_node, dst_node);
  
  if (hops_only_wired < one_hop_wireless)
    return getRoutingPathXYWired(src_node, dst_node);
        
  pair<int,int> rh_node_src = getClosestRHNode(src_node); // (attached node, rh)
  pair<int,int> rh_node_dst = getClosestRHNode(dst_node); // (attached node, rh)
  
  if (rh_node_src.second == rh_node_dst.second)
    return getRoutingPathXYWired(src_node, dst_node);
  
  int hops_wireless = getDistance(src_node, rh_node_src.first) +
    1 + // from attached node to rh to rh
    one_hop_wireless + // equivalent wired hops
    1 + // from rh to attached node to rh
    getDistance(rh_node_dst.first, dst_node);
  
  if (hops_only_wired < hops_wireless)
    return getRoutingPathXYWired(src_node, dst_node);

  
  TPath wired_path_src_to_rh = getRoutingPathXYWired(src_node, rh_node_src.first);
  TPath wired_path_rh_to_dst = getRoutingPathXYWired(rh_node_dst.first, dst_node);

  // create wireless path
  TPath wireless_path;
  pair<int,int> link_attached_node_to_rh_src(rh_node_src.first,
					     rh_node_src.second);
  pair<int,int> wireless_link(rh_node_src.second, rh_node_dst.second);
  pair<int,int> link_rh_dst_to_attached_node(rh_node_dst.second,
					     rh_node_dst.first);
  
  wireless_path.insert(wireless_path.end(),
		       wired_path_src_to_rh.begin(), wired_path_src_to_rh.end());
  wireless_path.push_back(link_attached_node_to_rh_src);
  wireless_path.push_back(wireless_link);
  wireless_path.push_back(link_rh_dst_to_attached_node);
  wireless_path.insert(wireless_path.end(),
		       wired_path_rh_to_dst.begin(), wired_path_rh_to_dst.end());
  
  return wireless_path;
}

// ----------------------------------------------------------------------

// (attached node to rh, rh node)
pair<int,int> NoC::getClosestRHNode(int node)
{
  assert(!radio_hubs.empty());

  int min_distance = INT_MAX;
  pair<int,int> return_pair;

  for (TRadioHubSet::iterator rhi = radio_hubs.begin();
       rhi != radio_hubs.end(); rhi++)
    {
      // check if node is attached to a radio hub
      if (rhi->second.find(node) != rhi->second.end())
	{
	  assert(rh2node.find(rhi->first) != rh2node.end());
	  return pair<int,int>(node, rh2node[rhi->first]);
	}

      // node is not attached to a radio hub. Search the closest node
      // attached to a radio hub
      for (TNodeSet::iterator ni = rhi->second.begin();
	   ni != rhi->second.end(); ni++)
	{
	  int distance = getDistance(node, *ni);

	  if (distance < min_distance)
	    {
	      min_distance = distance;
	      return_pair.first = *ni;
	      assert(rh2node.find(rhi->first) != rh2node.end());
	      return_pair.second = rh2node[rhi->first];
	    }
	}
    }
  
  
  return return_pair;
}

// ----------------------------------------------------------------------

TPath NoC::getRoutingPathFA(int src_node, int dst_node)
{
  TPath path;

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

void NoC::addCommunication(int src_node, int dst_node, long nbytes, int comm_id)
{
  TPath path = getRoutingPath(src_node, dst_node);

  assert(!path.empty());

  for (int i=0; i<path.size(); i++)
    {
      pair<int,int> l = path[i];
      assert(links.find(l) != links.end());
      //      links[l].ncomms++;
      if (links[l].comm_ids.find(comm_id) == links[l].comm_ids.end())
	{
	  links[l].total_load += nbytes;
	  links[l].comm_ids.insert(comm_id);
	}
    }
}

// ----------------------------------------------------------------------

double NoC::getBottleneckLinkCapacity(int src_node, int dst_node,
				      long nbytes)
{
  TPath path = getRoutingPath(src_node, dst_node);

  assert(!path.empty());

  double min_bw_utilization = DBL_MAX;

  for (int i=0; i<path.size(); i++)
    {
      TLinkAttr lattr = links.at(path[i]);

      // capacity in bytes per cycle
      /*
      double link_capacity = (lattr.link_type == LT_WIRED ||
			      lattr.link_type == LT_GRS) ? 
	link_width / 8 : wireless_bandwidth / clock_frequency / 8; 
      */
      double link_capacity;
      if (lattr.link_type == LT_WIRED)
	link_capacity = link_width / 8;
      else if (lattr.link_type == LT_GRS)
	link_capacity = grslink_width / 8;
      else if (lattr.link_type == LT_WIRELESS)
	link_capacity = wireless_bandwidth / clock_frequency / 8;
      else assert(false);
      
      double utilization = (double)nbytes / lattr.total_load;

      double bw_utilization = link_capacity * utilization;

      /*      
	cout << " link " << path[i].first << "-->" << path[i].second
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
  if (nbytes == 0)
    return 0;
  
  double link_bw = getBottleneckLinkCapacity(src_node, dst_node, nbytes);
  
  pair<int,int> distance = getDistanceWiredWireless(src_node, dst_node);
  return (distance.first * router_latency) +
    (nbytes / link_bw);
}

// ----------------------------------------------------------------------

pair<int,int> NoC::getDistanceWiredWireless(int src_node, int dst_node)
{
  int hops_only_wired = getDistance(src_node, dst_node);

  if (radio_hubs.empty())
    return pair<int,int>(hops_only_wired, 0);
  
  if (hops_only_wired < one_hop_wireless)
    return pair<int,int>(hops_only_wired, 0);
        
  pair<int,int> rh_node_src = getClosestRHNode(src_node); // (attached node, rh)
  pair<int,int> rh_node_dst = getClosestRHNode(dst_node); // (attached node, rh)

  if (rh_node_src.second == rh_node_dst.second)
    return pair<int,int>(hops_only_wired, 0);

  int src_rh_d = getDistance(src_node, rh_node_src.first);
  int rh_dst_d = getDistance(rh_node_dst.first, dst_node);
  int hops_wireless = src_rh_d +
    1 + // from attached node to rh to rh
    one_hop_wireless + // equivalent wired hops
    1 + // from rh to attached node to rh
    rh_dst_d;

  if (hops_only_wired < hops_wireless)
    return pair<int,int>(hops_only_wired, 0);
  else
    return pair<int,int>(src_rh_d + 1 + 1 + rh_dst_d, 1);
}

// ----------------------------------------------------------------------

int NoC::getDistance(int n1, int n2)
{
  pair<int,int> n1c = node2coord(n1);
  pair<int,int> n2c = node2coord(n2);

  return abs(n1c.first - n2c.first) + abs(n1c.second - n2c.second);
}
		
// ----------------------------------------------------------------------

int NoC::getNumberOfHops(int n1, int n2)
{
  pair<int,int> hops = getDistanceWiredWireless(n1, n2);
  
  return hops.first + hops.second;
}
  
// ----------------------------------------------------------------------

int NoC::closestMI(int node)
{
  vector<int> closest_list;

  int min_distance = INT_MAX;
  
  for (TNodeSet::iterator mi=memory_interfaces.begin();
       mi != memory_interfaces.end(); mi++)
    {
      int mi_node = *mi;
      int distance = getDistance(node, mi_node); // getNumberOfHops(node, mi_node);
      
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

TLatencyComponents NoC::getLatencyM2C(long nbytes, int dst_first, int dst_last,
				      bool same_data)
{
  // map communications to links
  resetLinks();

  int          comm_id = 0;
  
  for (int d=dst_first; d<=dst_last; d++)
    {
      int dst_node = pe2node.at(d);

      addCommunication(closest_mi.at(dst_node), dst_node, nbytes, comm_id);

      if ( !(same_data && use_multicast) )
	comm_id++;
    }
  
  // compute max latency
  long max_latency = -1;
  for (int d=dst_first; d<=dst_last; d++)
    {
      int dst_node = pe2node.at(d);

      long latency = getCommunicationLatency(closest_mi.at(dst_node), dst_node, nbytes);
      
      if (latency > max_latency)
	max_latency = latency;
    }
  
  // additional cycles to load data from main memory
  int mem_latency = (same_data) ? nbytes/getMainMemoryBandwidth() :
    (dst_last-dst_first+1)*(nbytes/getMainMemoryBandwidth()); 

  TLatencyComponents lc;
  lc.l_comm = max_latency;
  lc.l_mmem = mem_latency;

  return lc;
}


// ----------------------------------------------------------------------

TLatencyComponents NoC::getLatencyC2M(long nbytes, int src_first, int src_last)
{
  // map communications to links
  resetLinks();

  int          comm_id = 0;
  
  for (int s=src_first; s<=src_last; s++)
    {
      int src_node = pe2node.at(s);

      addCommunication(src_node, closest_mi.at(src_node), nbytes, comm_id);
      comm_id++;
    }
  
  // compute max latency
  long max_latency = -1;
  for (int s=src_first; s<=src_last; s++)
    {
      int src_node = pe2node.at(s);

      long latency = getCommunicationLatency(src_node, closest_mi.at(src_node), nbytes);
      
      if (latency > max_latency)
	max_latency = latency;
    }
  
  // additional cycles to write data to main memory
  int mem_latency = (src_last-src_first+1)*(nbytes/getMainMemoryBandwidth()); 

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

  int comm_id = 0;
  for (int s=src_first; s<=src_last; s++)
    {
      for (int d=dst_first; d<=dst_last; d++)
	if (s != d)
	  {
	    addCommunication(pe2node.at(s), pe2node.at(d), nbytes, comm_id);
	    if (!use_multicast)
	      comm_id++;
	  }
      comm_id++;
    }

  
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

  int comm_id = 0;
  
  // map communications to links: traffic from core to core
  for (int s=src_first; s<=src_last; s++)
    {
      for (int d=dst_first; d<=dst_last; d++)
	if (s != d)
	  {
	    addCommunication(pe2node.at(s), pe2node.at(d), nbytes_from_core, comm_id);
	    if (!use_multicast)
	      comm_id++;
	  }      
      comm_id++;
    }

  // map communications to links: traffic from memory to core
  for (int d=dst_first; d<=dst_last; d++)
    {
      int dst_node = pe2node.at(d);

      addCommunication(closest_mi.at(dst_node), dst_node, nbytes_from_memory, comm_id);

      if (!use_multicast)
	comm_id++;
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

      long latency = getCommunicationLatency(closest_mi.at(dst_node), dst_node,
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

void NoC::computeEnergyComm(double& e_wired,
			    double& e_wireless,
			    double& e_grs)
{
  e_wired = e_wireless = e_grs = 0.0;
  for (TLinks::iterator li = links.begin(); li != links.end(); li++)
    {
      TLinkAttr lattr = li->second;

      if (lattr.link_type == LT_WIRED)
	e_wired += lattr.total_load * 8 * (epb_link + epb_router);
      else if (lattr.link_type == LT_WIRELESS)
	e_wireless += lattr.total_load * 8 * epb_wireless;
      else if (lattr.link_type == LT_GRS)
	e_grs += lattr.total_load * 8 * epb_grslink;
      else assert(false);
    }
}

/* mau 
pair<double,double> NoC::computeEnergyComm(int hops_wired, int hops_wireless,
					   long nbytes)
{
  return pair<double,double>((double)nbytes * 8 * (epb_link * hops_wired + (epb_router * (hops_wired+1))),
			     (double)nbytes * 8 * epb_wireless * hops_wireless);
}
*/

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
    ec.e_mmem = computeEnergyMMem(nbytes);
  else
    ec.e_mmem = computeEnergyMMem(nbytes) * (dst_last - dst_first + 1);
    
  computeEnergyComm(ec.e_comm_wired, ec.e_comm_wireless, ec.e_comm_grs);

  return ec;
}

/* mau
TEnergyComponents NoC::getEnergyM2C(long nbytes, int dst_first, int dst_last,
				    bool same_data)
{
  TEnergyComponents ec;

  if (same_data)
    ec.e_mmem += computeEnergyMMem(nbytes);
  
  for (int d=dst_first; d<=dst_last; d++)
    {
      int s = closestMI(pe2node.at(d));

      pair<int,int> distance = getDistanceWiredWireless(s, pe2node.at(d));

      if (!same_data)
	ec.e_mmem += computeEnergyMMem(nbytes);
      
      ec.e_lmem += computeEnergyLMem(nbytes);

      pair<double,double> e_comm;
      e_comm = computeEnergyComm(distance.first, distance.second, nbytes);
      ec.e_comm_wired += e_comm.first;
      ec.e_comm_wireless += e_comm.second;
    }

  return ec;
}
*/

// ----------------------------------------------------------------------

TEnergyComponents NoC::getEnergyC2M(long nbytes_lm, long nbytes_mm,
				    int src_first, int src_last)
{
  TEnergyComponents ec;

  ec.e_mmem = computeEnergyMMem(nbytes_mm) * (src_last - src_first + 1);
  ec.e_lmem = computeEnergyLMem(nbytes_lm) * (src_last - src_first + 1);

  computeEnergyComm(ec.e_comm_wired, ec.e_comm_wireless, ec.e_comm_grs);

  return ec;
}

/* mau
TEnergyComponents NoC::getEnergyC2M(long nbytes_lm, long nbytes_mm,
				    int src_first, int src_last)
{
  TEnergyComponents ec;

  for (int s=src_first; s<=src_last; s++)
    {
      int d = closestMI(pe2node.at(s));

      pair<int,int> distance = getDistanceWiredWireless(pe2node.at(s), d);

      ec.e_mmem += computeEnergyMMem(nbytes_mm);
      
      ec.e_lmem += computeEnergyLMem(nbytes_lm);

      pair<double,double> e_comm;
      e_comm = computeEnergyComm(distance.first, distance.second, nbytes_mm);
      ec.e_comm_wired += e_comm.first;
      ec.e_comm_wireless += e_comm.second;
    }

  return ec;
}
*/

// ----------------------------------------------------------------------

TEnergyComponents NoC::getEnergyC2C(long nbytes,
				    int src_first, int src_last,
				    int dst_first, int dst_last)
{
  TEnergyComponents ec;

  ec.e_lmem = 2*computeEnergyLMem(nbytes) * // read from lmem of src core write to lmem of dst core
    (src_last - src_first) * (dst_last - dst_first + 1); // src to dst except src to src

  computeEnergyComm(ec.e_comm_wired, ec.e_comm_wireless, ec.e_comm_grs);
  
  return ec;
}

/* mau
TEnergyComponents NoC::getEnergyC2C(long nbytes,
				    int src_first, int src_last,
				    int dst_first, int dst_last)
{
  TEnergyComponents ec;

  for (int s=src_first; s<=src_last; s++)
    for (int d=dst_first; d<=dst_last; d++)
      if (s != d)
	{
	  pair<int,int> distance = getDistanceWiredWireless(pe2node.at(s), pe2node.at(d));

	  pair<double,double> e_comm;
	  e_comm = computeEnergyComm(distance.first, distance.second, nbytes);
	  ec.e_comm_wired += e_comm.first;
	  ec.e_comm_wireless += e_comm.second;

	  ec.e_lmem += 2*computeEnergyLMem(nbytes); // read from lmem of src core write to lmem of dst core
	}
  
  return ec;
}
*/

// ----------------------------------------------------------------------

TEnergyComponents NoC::getEnergyMC2C(long nbytes,
				     int src_first, int src_last,
				     int dst_first, int dst_last)
{
  TEnergyComponents ec;

  // compute energy M2C
  ec.e_mmem = computeEnergyMMem(nbytes) * (dst_last - dst_first + 1);

  // compute energy C2C
  ec.e_lmem = 2*computeEnergyLMem(nbytes) * // read from lmem of src core write to lmem of dst core
    (src_last - src_first) * (dst_last - dst_first + 1); // src to dst except src to src
  
  // compute interconnect energy
  computeEnergyComm(ec.e_comm_wired, ec.e_comm_wireless, ec.e_comm_grs);

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

  ec.e_comm_wired_leakage += ((nrouters * leak_pwr_router) +
			      (nrouters * 4 * leak_pwr_link)) * cycles / clock_frequency;
  ec.e_comm_wireless_leakage += rh2node.size() * leak_pwr_wireless * cycles / clock_frequency;

  ec.e_comm_grs_leakage += grslinks_pos.size() * leak_pwr_grslink * cycles / clock_frequency;
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

void NoC::testRouting()
{
  int src, dst;
  TPath path;
  
  do {
    cout << "src dst: ";
    cin >> src >> dst;
    path = getRoutingPathXY(src, dst);
    for (int i=0; i<path.size(); i++)
      cout << "(" << path[i].first << "," << path[i].second << ") --> ";
    cout << endl;
  } while (src != -1); 
}   

// ----------------------------------------------------------------------

void NoC::showLinksLoad()
{
  unsigned long int load_wired = 0;
  unsigned long int load_wireless = 0;
  int nwired_links = 0, nwireless_links = 0;
  
  for (TLinks::iterator li=links.begin(); li != links.end(); li++)
    {
      pair<int,int> l = li->first;
      TLinkAttr la = li->second;
      
      cout << l.first << "->" << l.second << " load: " << la.total_load
	   << " type: " << la.link_type << " idsize: " << la.comm_ids.size()
	   << endl;

      if (la.link_type == LT_WIRED)
	{
	  nwired_links++;
	  load_wired += la.total_load;
	}
      else
	{
	  nwireless_links++;
	  load_wireless += la.total_load;
	}
    }

  cout << "load_wired: " << load_wired << endl
       << "load_wireless: " << load_wireless << endl
       << "avg wired load: " << (nwired_links == 0 ? 0 : load_wired/nwired_links) << endl
       << "avg wireless load: " << (nwireless_links == 0 ? 0 : load_wireless/nwireless_links) << endl;
}

// ----------------------------------------------------------------------

void NoC::testCommunication()
{
  int   src_first, src_last, dst_first, dst_last, nbytes;
  bool  same_data;
  int   same_data_int;
  
  do {
    cout << "nbytes, src_first, src_last, dst_first, dst_last: ";
    cin >> nbytes >> src_first >> src_last >> dst_first >> dst_last;
    cout << "same data (1 yes, 0 no)? ";
    cin >> same_data_int;
    same_data = (same_data_int == 1);

    // map communications to links
    resetLinks();

    int comm_id = 0;

    for (int s=src_first; s<=src_last; s++)
      for (int d=dst_first; d<=dst_last; d++)
	{
	  int src_node = pe2node.at(s);
	  int dst_node = pe2node.at(d);
	  
	  addCommunication(src_node, dst_node, nbytes, comm_id);
	  
	  if ( !(same_data && use_multicast) )
	    comm_id++;
	}
    

    // show communications
    cout << "Links" << endl;
    showLinksLoad();
    

    // compute max latency
    cout << "Latency" << endl;
    long max_latency = -1;
    for (int d=dst_first; d<=dst_last; d++)
      {
	int dst_node = pe2node.at(d);
	
	long latency = getCommunicationLatency(closest_mi.at(dst_node), dst_node, nbytes);

	cout << closest_mi.at(dst_node) << " --> " << dst_node << ": " << latency
	     << "\t\tclosest_mi.at(" << dst_node << ") = "
	     << closest_mi.at(dst_node) << endl;
	
	if (latency > max_latency)
	  max_latency = latency;
      }

  } while (1); 
}   

// ----------------------------------------------------------------------
