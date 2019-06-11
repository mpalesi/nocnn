#include <iostream>
#include <iomanip>
#include <cassert>
#include "estimation.h"


// ----------------------------------------------------------------------
// Compute the number of required cores for performing ntasks 
int Estimation::requiredCores(int ntasks)
{
  return (ntasks <= noc.getNumberOfCores()) ?
    ntasks : noc.getNumberOfCores();
}

// ----------------------------------------------------------------------

void Estimation::stimeLoadFeatureMap(int layer_no, TLayer& layer,
				     TLayerStat& layer_stat,
				     int nactive_cores)
{
  // Estimate time and energy to load the input feature map
  if (layer_no == 0)
    {
      // the input feature map is stored into memory      
      LatencyComponents lc = noc.getLatencyM2C(layer.input_fm.getSize(),
					       0, nactive_cores-1);
      layer_stat.addLatencyComponents(lc);
      
      EnergyComponents ec = noc.getEnergyM2C(layer.input_fm.getSize(),
					     0, nactive_cores-1, true);
      layer_stat.addEnergyComponents(ec);

      layer_stat.main_memory_traffic += layer.input_fm.getSize();
    }
  else
    {
      // The input feature map is spread over the active cores in the
      // previous layer

      if (history[layer_no-1].per_core_fm_size > noc.local_memory_size)
	{
	  // the output feature map computed in the prvious layer is
	  // stored partially into local memory and partially into
	  // main memory.
	  int per_core_fm_size_from_memory = layer.input_fm.getSize() - noc.local_memory_size; // history[layer_no-1].per_core_fm_size - noc.local_memory_size;
	  int per_core_fm_size_from_core   = noc.local_memory_size; // history[layer_no-1].per_core_fm_size; // noc.local_memory_size;
 
	  TEnergyComponents ec;

	  ec = noc.getEnergyM2C(per_core_fm_size_from_memory,
				0, nactive_cores-1, true);
	  layer_stat.addEnergyComponents(ec);
	    
	  ec = noc.getEnergyC2C(per_core_fm_size_from_core,
				0, history[layer_no-1].nactive_cores-1,
				0, nactive_cores-1);
	  layer_stat.addEnergyComponents(ec);
	  

	  TLatencyComponents lc = noc.getLatencyMC2C(per_core_fm_size_from_memory,
						     per_core_fm_size_from_core,
						     0, history[layer_no-1].nactive_cores-1,
						     0, nactive_cores-1);
	  layer_stat.addLatencyComponents(lc);
	  
	  layer_stat.main_memory_traffic += per_core_fm_size_from_memory;
	}
      else
	{
	  // the output feature map computed in the prvious layer is stored into the local memory
	  TLatencyComponents lc = noc.getLatencyC2C(history[layer_no-1].per_core_fm_size,
						    0, history[layer_no-1].nactive_cores-1,
						    0, nactive_cores-1);
	  layer_stat.addLatencyComponents(lc);
	  
	  TEnergyComponents ec = noc.getEnergyC2C(history[layer_no-1].per_core_fm_size,
						  0, history[layer_no-1].nactive_cores-1,
						  0, nactive_cores-1);
	  layer_stat.addEnergyComponents(ec);
	}
    }
}

// ----------------------------------------------------------------------

void Estimation::stimeLeakage(TLayerStat& layer_stat)
{
  TEnergyComponents ec;

  // we assume power gating of main memory during communication and
  // computation periods
  ec = noc.getEnergyMMemLeakage(layer_stat.comm_latency +
				layer_stat.comp_latency); 
  layer_stat.addEnergyComponents(ec);

  // we assume power gating of local memory whule waiting main memory
  // transfers
  ec = noc.getEnergyLMemLeakage(layer_stat.active_cores,
				layer_stat.comm_latency +
				layer_stat.comp_latency +
				layer_stat.mmem_latency); 
  layer_stat.addEnergyComponents(ec);
  
  ec = noc.getEnergyCommLeakage(noc.getNumberOfCores(),  // One router for each PE
			       layer_stat.comm_latency +
			       layer_stat.comp_latency +
			       layer_stat.mmem_latency); 
  layer_stat.addEnergyComponents(ec);
}

// ----------------------------------------------------------------------

void Estimation::stimeConv(int layer_no, TLayer& layer,
			   TLayerStat& layer_stat)
{
  // Initialize stat structure
  layer_stat.reset();
  
  int nactive_cores = requiredCores(layer.filter.nf);

  // Estimate time and energy to load the input feature map
  stimeLoadFeatureMap(layer_no, layer, layer_stat, nactive_cores);

    
  // Estimate time and energy to load the filters
  int per_core_filters_size = layer.filter.getSizeAll() / nactive_cores;

  TLatencyComponents lc = noc.getLatencyM2C(per_core_filters_size,
					    0, nactive_cores-1);
  layer_stat.addLatencyComponents(lc);

  TEnergyComponents ec;
  ec = noc.getEnergyM2C(per_core_filters_size,
			0, nactive_cores-1, false);
  layer_stat.addEnergyComponents(ec);

  layer_stat.main_memory_traffic += layer.filter.getSizeSingle()*layer.filter.nf;

  // Estimate time and energy to compute the output feature map
  double per_core_filters = (double)layer.filter.nf / nactive_cores;
  long nmac_per_core = (int)(per_core_filters *
		       (layer.filter.w * layer.filter.h * layer.input_fm.ch) *
		       (layer.output_fm.w * layer.output_fm.h));
  
  lc = noc.getLatencyMAC(nmac_per_core);
  layer_stat.addLatencyComponents(lc);
  
  ec = noc.getEnergyMAC(nmac_per_core * nactive_cores, layer.filter.bits);
  layer_stat.addEnergyComponents(ec);

  layer_stat.active_cores += nactive_cores;
  layer_stat.ops_per_core += nmac_per_core;

  // Compute leakage energy component (must be called at the end when
  // timing statistics are updated)
  stimeLeakage(layer_stat);
    
  // Update history
  THistorySample h;
  h.setSample(nactive_cores, layer.output_fm.getSize() / nactive_cores);
  history.push_back(h);
}

// ----------------------------------------------------------------------

void Estimation::stimeFC(int layer_no, TLayer& layer,
			 TLayerStat& layer_stat)
{
  // Initialize stat structure
  layer_stat.reset();
  
  int nactive_cores = requiredCores(layer.fc.n);

  
  // Estimate time and energy to load the input feature map
  stimeLoadFeatureMap(layer_no, layer, layer_stat, nactive_cores);

  
  // Estimate time and energy to load the weights
  int weights_size_per_neuron = layer.input_fm.w *
    layer.input_fm.h *
    layer.input_fm.ch * layer.fc.bits / 8;
  double neurons_per_core = (double)layer.fc.n / nactive_cores;
  int weights_size_per_core = (int)(weights_size_per_neuron * neurons_per_core);
					   
  TLatencyComponents lc = noc.getLatencyM2C(weights_size_per_core,
					    0, nactive_cores-1);
  layer_stat.addLatencyComponents(lc);

  TEnergyComponents ec;
  ec = noc.getEnergyM2C(weights_size_per_core,
			0, nactive_cores-1, false);
  layer_stat.addEnergyComponents(ec);

  layer_stat.main_memory_traffic += weights_size_per_core * nactive_cores;

  // Estimate time and energy to compute the output feature map
  int nmac_per_core = (int)(neurons_per_core *
			    (layer.input_fm.w * layer.input_fm.h * layer.input_fm.ch));
  
  lc = noc.getLatencyMAC(nmac_per_core);
  layer_stat.addLatencyComponents(lc);
  
  ec = noc.getEnergyMAC(nmac_per_core * nactive_cores, layer.fc.bits);
  layer_stat.addEnergyComponents(ec);

  layer_stat.active_cores += nactive_cores;
  layer_stat.ops_per_core += nmac_per_core;

  // Compute leakage energy component (must be called at the end when
  // timing statistics are updated)
  stimeLeakage(layer_stat);

  
  // Update history
  THistorySample h;
  h.setSample(nactive_cores, layer.output_fm.getSize() / nactive_cores);
  history.push_back(h);
}

// ----------------------------------------------------------------------

void Estimation::stimePool(int layer_no, TLayer& layer,
			   TLayerStat& layer_stat)
{
  // Initialize stat structure
  layer_stat.reset();
  
  int nactive_cores = requiredCores(layer.input_fm.ch);
  
  // Estimate time and energy to load the input feature map
  stimeLoadFeatureMap(layer_no, layer, layer_stat, nactive_cores);

  // Estimate time and energy to compute the output feature map
  int per_core_channels = layer.input_fm.ch / nactive_cores;
  long npool_per_core = per_core_channels *
    (layer.pool.w * layer.pool.h) * (layer.output_fm.w * layer.output_fm.h);

  TLatencyComponents lc = noc.getLatencyPool(npool_per_core);
  layer_stat.addLatencyComponents(lc);

  TEnergyComponents ec;
  ec = noc.getEnergyPool(npool_per_core * nactive_cores, layer.pool.bits);
  layer_stat.addEnergyComponents(ec);

  layer_stat.active_cores += nactive_cores;
  layer_stat.ops_per_core += npool_per_core;

  // Compute leakage energy component (must be called at the end when
  // timing statistics are updated)
  stimeLeakage(layer_stat);
  
  // Update history
  THistorySample h;
  h.setSample(nactive_cores, layer.output_fm.getSize() / nactive_cores);
  history.push_back(h);
}

// ----------------------------------------------------------------------

bool Estimation::stime(int layer_no, TLayer& layer, TLayerStat& layer_stat)
{
  switch (layer.ltype)
    {
    case LT_CONV:
      stimeConv(layer_no, layer, layer_stat);
      return true;
      
    case LT_FC:
      stimeFC(layer_no, layer, layer_stat);
      return true;
      
    case LT_AVG_POOL:
    case LT_MAX_POOL:
      stimePool(layer_no, layer, layer_stat);
      return true;
    }
  
  cerr << "Unrecognized layer type for layer '" << layer.lname << "'" << endl;

  return false;
}

// ----------------------------------------------------------------------

bool Estimation::stime(TGlobalStats& stats)
{
  stats.reset();
  history.clear();
  
  for (int l=0; l<cnn.layers.size(); l++)
    {
      TLayerStat ls;

      if (!stime(l, cnn.layers[l], ls))
	return false;

      stats.addLayerStat(ls);
    }

  return true;
}

// ----------------------------------------------------------------------

void Estimation::hline(int n, char c)
{
  for (int i=0; i<n; i++)
    cout << c;
  cout << endl;
}

#define DOUBLE_FORMAT(x) std::scientific <<std::setprecision(2)<<(x)
void Estimation::showStats(TGlobalStats& stats)
{
  hline(165, '-');

  cout << setw(16) << "Layer"
       << setw(10) << "Type"
       << setw(30) << "Latency (cycles)"
       << setw(80) << "Energy (Joule)"
       << setw(12) << "Mem traffic"
       << setw(8)  << "Cores"
       << setw(9) << "OPs"
       << endl;
  
  cout << setw(16+10) << ""
       << setw(10) << "Comm"
       << setw(10) << "Comp"
       << setw(10) << "MMem"
       << setw(10) << "Comm"
       << setw(10) << "CommLeak"
       << setw(10) << "Comp"
       << setw(10) << "CompLeak"
       << setw(10) << "LMem"
       << setw(10) << "LMemLeak"
       << setw(10) << "MMem"
       << setw(10) << "MMemLeak"
       << setw(12) << "(bytes)"
       << setw(8)  << ""
       << setw(9)  << "per core"
       << endl;
  
  hline(165, '=');
  
  for (int l=0; l<cnn.layers.size(); l++)
    {
      cout << setw(16) << cnn.layers[l].lname
	   << setw(10) << cnn.ltype2str(cnn.layers[l].ltype)
	   << setw(10) << stats.layer_stats[l].comm_latency
	   << setw(10) << stats.layer_stats[l].comp_latency
	   << setw(10) << stats.layer_stats[l].mmem_latency
	   << setw(10) << DOUBLE_FORMAT(stats.layer_stats[l].comm_energy)
	   << setw(10) << DOUBLE_FORMAT(stats.layer_stats[l].comm_energy_leakage)
	   << setw(10) << DOUBLE_FORMAT(stats.layer_stats[l].comp_energy)
	   << setw(10) << DOUBLE_FORMAT(stats.layer_stats[l].comp_energy_leakage)
	   << setw(10) << DOUBLE_FORMAT(stats.layer_stats[l].lmem_energy)
	   << setw(10) << DOUBLE_FORMAT(stats.layer_stats[l].lmem_energy_leakage)
	   << setw(10) << DOUBLE_FORMAT(stats.layer_stats[l].mmem_energy)
	   << setw(10) << DOUBLE_FORMAT(stats.layer_stats[l].mmem_energy_leakage)
	   << setw(12) << stats.layer_stats[l].main_memory_traffic
	   << setw(8)  << stats.layer_stats[l].active_cores
	   << setw(9)  << stats.layer_stats[l].ops_per_core
	   << endl;
    }

  hline(165, '-');

  cout << setw(16+10) << "TOTAL"
       << setw(10) << stats.total_comm_latency
       << setw(10) << stats.total_comp_latency
       << setw(10) << stats.total_mmem_latency
       << setw(10) << DOUBLE_FORMAT(stats.total_comm_energy)
       << setw(10) << DOUBLE_FORMAT(stats.total_comm_energy_leakage)
       << setw(10) << DOUBLE_FORMAT(stats.total_comp_energy)
       << setw(10) << DOUBLE_FORMAT(stats.total_comp_energy_leakage)
       << setw(10) << DOUBLE_FORMAT(stats.total_lmem_energy)
       << setw(10) << DOUBLE_FORMAT(stats.total_lmem_energy_leakage)
       << setw(10) << DOUBLE_FORMAT(stats.total_mmem_energy)
       << setw(10) << DOUBLE_FORMAT(stats.total_mmem_energy_leakage)
       << setw(12) << stats.total_main_memory_traffic
       << endl;

  hline(165, '-');
}
