#include <iostream>
#include <algorithm>
#include <strstream>

#include "FWCore/MessageLogger/interface/MessageLogger.h"
#include "FWCore/ParameterSet/interface/ParameterSet.h"

#include "UserCode/OMTFSimulation/interface/OMTFProcessor.h"
#include "UserCode/OMTFSimulation/interface/OMTFConfiguration.h"
#include "UserCode/OMTFSimulation/interface/GoldenPattern.h"
#include "UserCode/OMTFSimulation/interface/XMLConfigReader.h"
#include "UserCode/OMTFSimulation/interface/OMTFinput.h"
#include "UserCode/OMTFSimulation/interface/OMTFResult.h"

///////////////////////////////////////////////
///////////////////////////////////////////////
OMTFProcessor::OMTFProcessor(const edm::ParameterSet & theConfig){

myResults.assign(6,OMTFProcessor::resultsMap());

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

  ///For making small patterns file for compilation
  //if(aGP->key().thePtCode>20 ||
  //   aGP->key().thePtCode<16) return true;
  ///////

  if(theGPs.find(aGP->key())!=theGPs.end()) return false;
  else theGPs[aGP->key()] = new GoldenPattern(*aGP);

  for(auto & itRegion: myResults) itRegion[aGP->key()] = OMTFResult(); 

  return true;
}
///////////////////////////////////////////////
///////////////////////////////////////////////
const std::map<Key,GoldenPattern*> & OMTFProcessor::getPatterns() const{

  return theGPs;

}
///////////////////////////////////////////////
///////////////////////////////////////////////
const std::vector<OMTFProcessor::resultsMap> & OMTFProcessor::processInput(unsigned int iProcessor,
									     const OMTFinput & aInput){

  for(auto & itRegion: myResults) for(auto & itKey: itRegion) itKey.second.clear();

  //////////////////////////////////////
  //////////////////////////////////////  
  std::bitset<80> refHitsBits = aInput.getRefHits(iProcessor);
  if(refHitsBits.none()) return myResults;
  
  //if(refHitsBits.count()>4) std::cout<<"ref hits: "<<refHitsBits.to_string()<<std::endl;
 
  for(unsigned int iLayer=0;iLayer<OMTFConfiguration::nLayers;++iLayer){
    const OMTFinput::vector1D & layerHits = aInput.getLayerData(iLayer);
    if(!layerHits.size()) continue;
    ///Number of reference hits to be checked. 
    ///Value read from XML configuration
    unsigned int nTestedRefHits = OMTFConfiguration::nTestRefHits;
    for(unsigned int iRefHit=0;iRefHit<80;++iRefHit){
      if(!refHitsBits[iRefHit]) continue;
      if(nTestedRefHits--==0) break;
      const RefHitDef & aRefHitDef = OMTFConfiguration::refHitsDefs[iProcessor][iRefHit];
      int phiRef = aInput.getLayerData(OMTFConfiguration::refToLogicNumber[aRefHitDef.iRefLayer])[aRefHitDef.iInput]; 
      unsigned int iRegion = aRefHitDef.iRegion;
      if(OMTFConfiguration::bendingLayers.count(iLayer)) phiRef = 0;
      const OMTFinput::vector1D restrictedLayerHits = restrictInput(iProcessor, iRegion, iLayer,layerHits);
      for(auto itGP: theGPs){
	GoldenPattern::layerResult aLayerResult = itGP.second->process1Layer1RefLayer(aRefHitDef.iRefLayer,iLayer,
										      phiRef,
										      restrictedLayerHits);
	myResults[iRegion][itGP.second->key()].addResult(aRefHitDef.iRefLayer,iLayer,aLayerResult.first,phiRef);	 
      }
    }
  }  
  //////////////////////////////////////
  //////////////////////////////////////
  
  /*
  for(unsigned int iRefLayer=0;iRefLayer<OMTFConfiguration::nRefLayers;++iRefLayer){
    const OMTFinput::vector1D & refLayerHits = aInput.getLayerData(OMTFConfiguration::refToLogicNumber[iRefLayer]);	
    if(!refLayerHits.size()) continue;
    //////////////////////
    for(unsigned int iLayer=0;iLayer<OMTFConfiguration::nLayers;++iLayer){
      const OMTFinput::vector1D & layerHits = aInput.getLayerData(iLayer);
      if(!layerHits.size()) continue;
      for(unsigned int iInput=0;iInput<refLayerHits.size();++iInput){	
	  int phiRef = refLayerHits[iInput];
	  unsigned int iRegion = OMTFConfiguration::getRegionNumberFromMap(iProcessor,iRefLayer,phiRef);
	  if(iRegion>5) continue;
	  if(phiRef>=(int)OMTFConfiguration::nPhiBins) continue;
	  if(OMTFConfiguration::bendingLayers.count(iLayer)) phiRef = 0;
	  const OMTFinput::vector1D restrictedLayerHits = restrictInput(iProcessor, iRegion, iLayer,layerHits);
	  for(auto itGP: theGPs){
	    GoldenPattern::layerResult aLayerResult = itGP.second->process1Layer1RefLayer(iRefLayer,iLayer,
											  phiRef,
											  restrictedLayerHits);
	    myResults[iRegion][itGP.second->key()].addResult(iRefLayer,iLayer,aLayerResult.first,phiRef);	 
	  }
      }      
    }
  }
  */

  for(auto & itRegion: myResults) for(auto & itKey: itRegion) itKey.second.finalise();


  //#ifndef NDEBUG
  std::ostringstream myStr;
  myStr<<"iProcessor: "<<iProcessor<<std::endl;
  myStr<<"Input: ------------"<<std::endl;
  myStr<<aInput<<std::endl;
  for(auto itRegion: myResults){ 
    for(auto itKey: itRegion){      
      myStr<<itKey.first<<std::endl;
      myStr<<itKey.second<<std::endl;
    }
  }
  //LogDebug("OMTF processor")<<myStr.str();
  //edm::LogInfo("OMTF processor")<<myStr.str();
  //#endif
  
  return myResults;
}   
////////////////////////////////////////////
////////////////////////////////////////////
OMTFinput OMTFProcessor::shiftInput(unsigned int iProcessor,
				    const OMTFinput & aInput){

  int minPhi =  OMTFConfiguration::globalPhiStart(iProcessor);

  ///OMTFConfiguration::nPhiBins/2 to shift the minPhi to 0-nBins scale,
  if(minPhi<0) minPhi+=OMTFConfiguration::nPhiBins;

  OMTFinput myCopy = aInput;
  myCopy.shiftMyPhi(minPhi);
  
  return myCopy;
}
////////////////////////////////////////////
////////////////////////////////////////////
OMTFinput::vector1D OMTFProcessor::restrictInput(unsigned int iProcessor,
						 unsigned int iRegion,
						 unsigned int iLayer,
						 const OMTFinput::vector1D & layerHits){

  OMTFinput::vector1D myHits = layerHits;
  unsigned int iStart = OMTFConfiguration::connections[iProcessor][iRegion][iLayer].first;
  unsigned int iEnd = iStart + OMTFConfiguration::connections[iProcessor][iRegion][iLayer].second -1;

  for(unsigned int iHit=0;iHit<14;++iHit){
    if(iHit<iStart || iHit>iEnd) myHits[iHit] = OMTFConfiguration::nPhiBins;    
  }
  return myHits;
}
////////////////////////////////////////////
////////////////////////////////////////////
