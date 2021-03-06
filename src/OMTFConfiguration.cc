#include <iostream>

#include "UserCode/OMTFSimulation/interface/OMTFConfiguration.h"
#include "UserCode/OMTFSimulation/interface/XMLConfigReader.h"

#include "FWCore/ParameterSet/interface/ParameterSet.h"

unsigned int OMTFConfiguration::nLayers;
unsigned int OMTFConfiguration::nHitsPerLayer;
unsigned int OMTFConfiguration::nRefLayers;
unsigned int OMTFConfiguration::nPdfAddrBits;
unsigned int OMTFConfiguration::nPdfValBits;
unsigned int OMTFConfiguration::nPhiBits;
unsigned int OMTFConfiguration::nPhiBins;
unsigned int OMTFConfiguration::nRefHits;
unsigned int OMTFConfiguration::nTestRefHits;

std::map<int,int> OMTFConfiguration::hwToLogicLayer;
std::map<int,int> OMTFConfiguration::logicToHwLayer;
std::map<int,int> OMTFConfiguration::logicToLogic;
std::vector<int> OMTFConfiguration::refToLogicNumber;
std::set<int> OMTFConfiguration::bendingLayers;
std::vector<std::vector<int> > OMTFConfiguration::processorPhiVsRefLayer;
OMTFConfiguration::vector3D_A OMTFConfiguration::connections;
std::vector<std::vector<std::vector<std::pair<int,int> > > >OMTFConfiguration::regionPhisVsRefLayerVsProcessor;

std::vector<std::vector<RefHitDef> >OMTFConfiguration::refHitsDefs;


OMTFConfiguration::vector4D OMTFConfiguration::measurements4D;
OMTFConfiguration::vector4D OMTFConfiguration::measurements4Dref;

///////////////////////////////////////////////
///////////////////////////////////////////////
RefHitDef::RefHitDef(unsigned int aInput, 
		     int aPhiMin, int aPhiMax,
		     unsigned int aRegion,
		     unsigned int aRefLayer):
  iInput(aInput),
  iRegion(aRegion),
  iRefLayer(aRefLayer),
  range(std::pair<int,int>(aPhiMin,aPhiMax)){}
///////////////////////////////////////////////
///////////////////////////////////////////////
bool RefHitDef::fitsRange(int iPhi) const{

  return iPhi>=range.first && 
         iPhi<range.second;

}
///////////////////////////////////////////////
///////////////////////////////////////////////
OMTFConfiguration::OMTFConfiguration(const edm::ParameterSet & theConfig){

  if ( !theConfig.exists("configXMLFile") ) return;
  std::string fName = theConfig.getParameter<std::string>("configXMLFile");

  XMLConfigReader myReader;
  myReader.setConfigFile(fName);
  configure(&myReader);

  ///Vector of all inpouts (14)
  std::vector<int> aLayer1D(14,0);

  ///Vector of all layers 
  OMTFConfiguration::vector2D aLayer2D;
  aLayer2D.assign(OMTFConfiguration::nLayers,aLayer1D);

  ///Vector of all logic cones
  OMTFConfiguration::vector3D aLayer3D;
  aLayer3D.assign(6,aLayer2D);

  ///Vector of all processors
  measurements4D.assign(6,aLayer3D);
  measurements4Dref.assign(6,aLayer3D);


}
///////////////////////////////////////////////
///////////////////////////////////////////////
void OMTFConfiguration::configure(XMLConfigReader *aReader){

 aReader->readConfig(this);

}
///////////////////////////////////////////////
///////////////////////////////////////////////
std::ostream & operator << (std::ostream &out, const OMTFConfiguration & aConfig){


  out<<"nLayers: "<<aConfig.nLayers
     <<" nHitsPerLayer: "<<aConfig.nHitsPerLayer
     <<" nRefLayers: "<<aConfig.nRefLayers
     <<" nPdfAddrBits: "<<aConfig.nPdfAddrBits
     <<" nPdfValBits: "<<aConfig.nPdfValBits
     <<std::endl;

  for(unsigned int iProcessor = 0;iProcessor<6; ++iProcessor){
    out<<"Processor: "<<iProcessor;
    for(unsigned int iRefLayer=0;iRefLayer<aConfig.nRefLayers;++iRefLayer){
      out<<" "<<aConfig.processorPhiVsRefLayer[iProcessor][iRefLayer];
    }
    out<<std::endl;
  }
  
  return out;

}
///////////////////////////////////////////////
///////////////////////////////////////////////
bool OMTFConfiguration::isInRegionRange(int iPhiStart,
				unsigned int coneSize,
				int iPhi){

  if(iPhi<0) iPhi+=OMTFConfiguration::nPhiBins;
  if(iPhiStart<0) iPhiStart+=OMTFConfiguration::nPhiBins;

  if(iPhiStart+(int)coneSize<(int)OMTFConfiguration::nPhiBins){
    return iPhiStart<=iPhi && iPhiStart+(int)coneSize>iPhi;
  }
  else if(iPhi>(int)OMTFConfiguration::nPhiBins/2){
    return iPhiStart<=iPhi;
  }
  else if(iPhi<(int)OMTFConfiguration::nPhiBins/2){
    return iPhi<iPhiStart+(int)coneSize-(int)OMTFConfiguration::nPhiBins;
  }
  return false;
}
///////////////////////////////////////////////
///////////////////////////////////////////////
unsigned int OMTFConfiguration::getRegionNumber(unsigned int iProcessor,
					  unsigned int iRefLayer,
					  int iPhi){

  if(iPhi>=(int)OMTFConfiguration::nPhiBins) return 99;

  unsigned int logicRegionSize = 10/360.0*OMTFConfiguration::nPhiBins;
  

  unsigned int iRegion = 0;
  int iPhiStart = OMTFConfiguration::processorPhiVsRefLayer[iProcessor][iRefLayer];
  
  ///FIX ME 2Pi wrapping  
  while(!OMTFConfiguration::isInRegionRange(iPhiStart,logicRegionSize,iPhi) && iRegion<6){
    ++iRegion;
    iPhiStart+=logicRegionSize;    
  }

  if(iRegion>5) iRegion = 99;
  return iRegion;
}
///////////////////////////////////////////////
///////////////////////////////////////////////
unsigned int OMTFConfiguration::getRegionNumberFromMap(unsigned int iProcessor,
						       unsigned int iRefLayer,
						       int iPhi){
  for(unsigned int iRegion=0;iRegion<6;++iRegion){
    if(iPhi>=OMTFConfiguration::regionPhisVsRefLayerVsProcessor[iProcessor][iRefLayer][iRegion].first &&
       iPhi<=OMTFConfiguration::regionPhisVsRefLayerVsProcessor[iProcessor][iRefLayer][iRegion].second)
      return iRegion;
  }

  return 99;
}
///////////////////////////////////////////////
///////////////////////////////////////////////
int OMTFConfiguration::globalPhiStart(unsigned int iProcessor){

  return *std::min_element(OMTFConfiguration::processorPhiVsRefLayer[iProcessor].begin(),
			   OMTFConfiguration::processorPhiVsRefLayer[iProcessor].end());

}
///////////////////////////////////////////////
///////////////////////////////////////////////
