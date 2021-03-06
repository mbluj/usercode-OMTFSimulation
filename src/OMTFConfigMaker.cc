#include <iostream>

#include "UserCode/OMTFSimulation/interface/OMTFConfigMaker.h"
#include "UserCode/OMTFSimulation/interface/OMTFConfiguration.h"
#include "UserCode/OMTFSimulation/interface/GoldenPattern.h"
#include "UserCode/OMTFSimulation/interface/XMLConfigReader.h"
#include "UserCode/OMTFSimulation/interface/OMTFinput.h"
#include "UserCode/OMTFSimulation/interface/OMTFResult.h"

#include "FWCore/ParameterSet/interface/ParameterSet.h"

///////////////////////////////////////////////
///////////////////////////////////////////////
OMTFConfigMaker::OMTFConfigMaker(const edm::ParameterSet & theConfig){ 

  std::vector<int> minRefPhi1D(6,2*OMTFConfiguration::nPhiBins);
  minRefPhi2D.assign(OMTFConfiguration::nRefLayers,minRefPhi1D);

}
///////////////////////////////////////////////
///////////////////////////////////////////////
OMTFConfigMaker::~OMTFConfigMaker(){ }
///////////////////////////////////////////////
///////////////////////////////////////////////
void OMTFConfigMaker::fillPhiMaps(unsigned int iProcessor,
				  const OMTFinput & aInput){

  ////Find starting iPhi for each processor and each referecne layer    
  for(unsigned int iRefLayer=0;iRefLayer<OMTFConfiguration::nRefLayers;++iRefLayer){
    const OMTFinput::vector1D & refLayerHits = aInput.getLayerData(OMTFConfiguration::refToLogicNumber[iRefLayer]);	
    if(!refLayerHits.size()) continue;
    for(auto itRefHit: refLayerHits){	
      int phiRef = itRefHit;
      if(phiRef>=(int)OMTFConfiguration::nPhiBins) continue;
      if(phiRef<0) phiRef+=OMTFConfiguration::nPhiBins;
      if(minRefPhi2D[iRefLayer][iProcessor]<0) minRefPhi2D[iRefLayer][iProcessor]+=OMTFConfiguration::nPhiBins;

      if(iProcessor==0 || iProcessor==5){
	phiRef+=OMTFConfiguration::nPhiBins/4;
	minRefPhi2D[iRefLayer][iProcessor]+=OMTFConfiguration::nPhiBins/4;

	if(phiRef>=(int)OMTFConfiguration::nPhiBins) phiRef-=OMTFConfiguration::nPhiBins;
	if(minRefPhi2D[iRefLayer][iProcessor]>=(int)OMTFConfiguration::nPhiBins)minRefPhi2D[iRefLayer][iProcessor]-=OMTFConfiguration::nPhiBins;

      }
      
      if(phiRef<minRefPhi2D[iRefLayer][iProcessor]) minRefPhi2D[iRefLayer][iProcessor] = phiRef;
      
      if( (iProcessor==0 || iProcessor==5) && 
	 minRefPhi2D[iRefLayer][iProcessor]<(int)OMTFConfiguration::nPhiBins) minRefPhi2D[iRefLayer][iProcessor]-=OMTFConfiguration::nPhiBins/4;
      if(minRefPhi2D[iRefLayer][iProcessor]<0) minRefPhi2D[iRefLayer][iProcessor]+=OMTFConfiguration::nPhiBins;	
          
      if(minRefPhi2D[iRefLayer][iProcessor]>(int)OMTFConfiguration::nPhiBins/2) minRefPhi2D[iRefLayer][iProcessor]-=OMTFConfiguration::nPhiBins;
    }
  }
}
///////////////////////////////////////////////
///////////////////////////////////////////////
void OMTFConfigMaker::printPhiMap(std::ostream & out){
  
  for(unsigned int iRefLayer=0;iRefLayer<OMTFConfiguration::nRefLayers;++iRefLayer){
    for(unsigned int iProcessor=0;iProcessor<6;++iProcessor){
      out<<"          "<<minRefPhi2D[iRefLayer][iProcessor]<<"\t";
    }
    out<<std::endl;
  }
  out<<std::endl; 
}
///////////////////////////////////////////////
///////////////////////////////////////////////
void OMTFConfigMaker::makeConnetionsMap(unsigned int iProcessor,
					const OMTFinput & aInput){

  fillPhiMaps(iProcessor,aInput);

  for(unsigned int iRefLayer=0;iRefLayer<OMTFConfiguration::nRefLayers;++iRefLayer){
    const OMTFinput::vector1D & refLayerHits = aInput.getLayerData(OMTFConfiguration::refToLogicNumber[iRefLayer]);	
    if(!refLayerHits.size()) continue;
    //////////////////////
    for(unsigned int iLayer=0;iLayer<OMTFConfiguration::nLayers;++iLayer){
      const OMTFinput::vector1D & layerHits = aInput.getLayerData(iLayer);
      if(!layerHits.size()) continue;
      for(unsigned int iInput=0;iInput<refLayerHits.size();++iInput){	
	  int phiRef = refLayerHits[iInput];
	  unsigned int iRegion = OMTFConfiguration::getRegionNumber(iProcessor,iRefLayer,phiRef);
	  if(iRegion>5) continue;
	  fillInputRange(iProcessor,iRegion,aInput);
	  fillInputRange(iProcessor,iRegion,iRefLayer,iInput);
      }
    }      
  }
} 
///////////////////////////////////////////////
///////////////////////////////////////////////
void OMTFConfigMaker::fillInputRange(unsigned int iProcessor,
				   unsigned int iRegion,
				   const OMTFinput & aInput){

  for(unsigned int iLogicLayer=0;iLogicLayer<OMTFConfiguration::nLayers;++iLogicLayer){
    for(unsigned int iHit=0;iHit<14;++iHit){
      bool isHit = aInput.getLayerData(iLogicLayer)[iHit]<(int)OMTFConfiguration::nPhiBins;
      OMTFConfiguration::measurements4D[iProcessor][iRegion][iLogicLayer][iHit]+=isHit;
    }
  }
}
///////////////////////////////////////////////
///////////////////////////////////////////////
void OMTFConfigMaker::fillInputRange(unsigned int iProcessor,
				   unsigned int iRegion,
				   unsigned int iRefLayer,
				   unsigned int iHit){

      ++OMTFConfiguration::measurements4Dref[iProcessor][iRegion][iRefLayer][iHit]; 

}
///////////////////////////////////////////////
///////////////////////////////////////////////
void OMTFConfigMaker::printConnections(std::ostream & out,
				       unsigned int iProcessor,
				       unsigned int iRegion){

  out<<"iProcessor: "<<iProcessor
     <<" iRegion: "<<iRegion
     <<std::endl;

  for(unsigned int iLogicLayer=0;iLogicLayer<OMTFConfiguration::nLayers;++iLogicLayer){
    out<<"Logic layer: "<<iLogicLayer<<" Hits: ";
    for(unsigned int iHit=0;iHit<14;++iHit){
      out<<OMTFConfiguration::measurements4Dref[iProcessor][iRegion][iLogicLayer][iHit]<<"\t";
    }
    out<<std::endl;
  }
}
/////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////
