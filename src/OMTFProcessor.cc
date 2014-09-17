#include <iostream>

#include "interface/OMTFProcessor.h"
#include "interface/OMTFConfiguration.h"
#include "interface/GoldenPattern.h"
#include "interface/XMLConfigReader.h"
#include "interface/OMTFinput.h"
#include "interface/OMTFResult.h"

#include "FWCore/ParameterSet/interface/ParameterSet.h"

///////////////////////////////////////////////
///////////////////////////////////////////////
OMTFProcessor::OMTFProcessor(const edm::ParameterSet & theConfig){

  if ( !theConfig.exists("patternsXMLFiles") ) return;
  std::vector<std::string> fileNames = theConfig.getParameter<std::vector<std::string> >("patternsXMLFiles");

  XMLConfigReader myReader;
  for(auto it: fileNames){
   myReader.setPatternsFile(it);
   configure(&myReader);
  }
}
///////////////////////////////////////////////
///////////////////////////////////////////////
OMTFProcessor::~OMTFProcessor(){

   for(auto it: theGPs) delete it.second;

}
///////////////////////////////////////////////
///////////////////////////////////////////////
bool OMTFProcessor::configure(XMLConfigReader *aReader){

  const std::vector<GoldenPattern *> & aGPs = aReader->readPatterns();
  for(auto it: aGPs){    
    if(!addGP(it)) return false;
  }

  return true;
}
///////////////////////////////////////////////
///////////////////////////////////////////////
bool OMTFProcessor::addGP(GoldenPattern *aGP){

  if(theGPs.find(aGP->key())!=theGPs.end()) return false;
  else theGPs[aGP->key()] = new GoldenPattern(*aGP);

  myResults[aGP->key()] = OMTFResult(); 

  return true;
}
///////////////////////////////////////////////
///////////////////////////////////////////////
const std::map<Key,GoldenPattern*> & OMTFProcessor::getPatterns() const{

  return theGPs;

}
///////////////////////////////////////////////
///////////////////////////////////////////////
OMTFProcessor::resultsMap OMTFProcessor::processInput(const OMTFinput & aInput){

  for(auto & itKey: myResults) itKey.second.clear();

  for(unsigned int iRefLayer=0;iRefLayer<OMTFConfiguration::nRefLayers;++iRefLayer){
    const OMTFinput::vector1D & refLayerHits = aInput.getLayerData(OMTFConfiguration::refToLogicNumber[iRefLayer]);	
    if(!refLayerHits.size()) continue;
    for(unsigned int iLayer=0;iLayer<OMTFConfiguration::nLayers;++iLayer){
      const OMTFinput::vector1D & layerHits = aInput.getLayerData(iLayer);
      if(!layerHits.size()) continue;
      for(auto itRefHit: refLayerHits){	
	for(auto itGP: theGPs){	  
	  int phiRef = itRefHit;
	  if(phiRef>=(int)OMTFConfiguration::nPhiBins) continue;
	  if(OMTFConfiguration::bendingLayers.count(iLayer)) phiRef = 0;

	  GoldenPattern::layerResult aLayerResult = itGP.second->process1Layer1RefLayer(iRefLayer,iLayer,
											phiRef,
											layerHits);
	  myResults[itGP.second->key()].addResult(iRefLayer,iLayer,aLayerResult.first,phiRef);	 
	}
      }      
    }
  }

  for(auto & itKey: myResults) itKey.second.finalise();

  return myResults;
}   
///////////////////////////////////////////////
///////////////////////////////////////////////
