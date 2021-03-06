#ifndef OMTF_OMTFSorter_H
#define OMTF_OMTFSorter_H

#include <tuple>

#include "UserCode/OMTFSimulation/interface/OMTFResult.h"
#include "UserCode/OMTFSimulation/interface/OMTFProcessor.h"

#include "UserCode/L1RpcTriggerAnalysis/interface/L1Obj.h"

class OMTFSorter{

 public:

  ///Sort all processor results. 
  ///First for each region cone find a best candidate using sortRegionResults() 
  ///Then select best candidate amongs found for each logic region
  L1Obj sortProcessorResults(const std::vector<OMTFProcessor::resultsMap> & procResults);


  ///Sort results for signle logic region.
  ///Select candidate with highed number of hit layers
  ///Then select a candidate with largest likelihood value
  L1Obj sortRegionResults(const OMTFProcessor::resultsMap & aResultsMap);

 private:

  ///Find a candidate with best parameters for given GoldenPattern
  ///Sorting is made amongs candidates with different reference layers
  ///The output tuple contains (nHitsMax, pdfValMax, refPhi) 
  std::tuple<unsigned int,unsigned int, int> sortSingleResult(const OMTFResult & aResult);

};

#endif
