#include <iostream>

#include "FWCore/MessageLogger/interface/MessageLogger.h"

#include "DataFormats/L1GlobalMuonTrigger/interface/L1MuRegionalCand.h"

#include "UserCode/OMTFSimulation/plugins/OMTFProducer.h"
#include "UserCode/OMTFSimulation/interface/OMTFProcessor.h"
#include "UserCode/OMTFSimulation/interface/OMTFinputMaker.h"
#include "UserCode/OMTFSimulation/interface/OMTFinput.h"
#include "UserCode/OMTFSimulation/interface/OMTFSorter.h"
#include "UserCode/OMTFSimulation/interface/OMTFConfiguration.h"
#include "UserCode/OMTFSimulation/interface/OMTFConfigMaker.h"
#include "UserCode/OMTFSimulation/interface/XMLConfigWriter.h"

using namespace L1TMuon;

OMTFProducer::OMTFProducer(const edm::ParameterSet& cfg):
  theConfig(cfg),
  trigPrimSrc(cfg.getParameter<edm::InputTag>("TriggerPrimitiveSrc")){

  produces<std::vector<L1MuRegionalCand> >("OMTF");

  inputToken = consumes<TriggerPrimitiveCollection>(trigPrimSrc);
  
  if(!theConfig.exists("omtf")){
    edm::LogError("OMTFProducer")<<"omtf configuration not found in cfg.py";
  }
  
  myInputMaker = new OMTFinputMaker();
  mySorter = new OMTFSorter();
  
  myWriter = new XMLConfigWriter();
  std::string fName = "OMTF_Events";
  myWriter->initialiseXMLDocument(fName);

  dumpResultToXML = theConfig.getParameter<bool>("dumpResultToXML");
  dumpGPToXML = theConfig.getParameter<bool>("dumpGPToXML");
  makeConnectionsMaps = theConfig.getParameter<bool>("makeConnectionsMaps");

  myOMTFConfig = 0;
}
/////////////////////////////////////////////////////
/////////////////////////////////////////////////////
OMTFProducer::~OMTFProducer(){

  delete myOMTFConfig;
  delete myOMTFConfigMaker;
  delete myOMTF;

}
/////////////////////////////////////////////////////
/////////////////////////////////////////////////////
void OMTFProducer::beginJob(){

  if(theConfig.exists("omtf")){
    myOMTFConfig = new OMTFConfiguration(theConfig.getParameter<edm::ParameterSet>("omtf"));
    myOMTFConfigMaker = new OMTFConfigMaker(theConfig.getParameter<edm::ParameterSet>("omtf"));
    myOMTF = new OMTFProcessor(theConfig.getParameter<edm::ParameterSet>("omtf"));
  }
}
/////////////////////////////////////////////////////
/////////////////////////////////////////////////////  
void OMTFProducer::endJob(){

  if(dumpResultToXML && !dumpGPToXML && !makeConnectionsMaps){
    std::string fName = "TestEvents.xml";
    myWriter->finaliseXMLDocument(fName);
  }

  if(dumpGPToXML && !dumpResultToXML && !makeConnectionsMaps){
    std::string fName = "OMTF";
    myWriter->initialiseXMLDocument(fName);
    const std::map<Key,GoldenPattern*> & myGPmap = myOMTF->getPatterns();
    for(auto itGP: myGPmap){
      //if(itGP.first.thePtCode==9 && itGP.first.theCharge==1) std::cout<<*itGP.second<<std::endl;     
      std::cout<<*itGP.second<<std::endl;     
      myWriter->writeGPData(*itGP.second);
    }
    fName = "GPs.xml";
    myWriter->finaliseXMLDocument(fName);
    ///Write merged GPs.//////////////////////////////////
    ///2x merging
    int charge = -1;
    int iPtMin = 6;
    Key aKey(1, iPtMin, charge);    

    fName = "OMTF";
    myWriter->initialiseXMLDocument(fName);
    while(myGPmap.find(aKey)!=myGPmap.end()){
      
      GoldenPattern *aGP1 = myGPmap.find(aKey)->second;
      GoldenPattern *aGP2 = aGP1;

      ++aKey.thePtCode;
      if(aKey.thePtCode<=31 && myGPmap.find(aKey)!=myGPmap.end()) aGP2 =  myGPmap.find(aKey)->second;      

      myWriter->writeGPData(*aGP1,*aGP2);

      aKey.theCharge = 1;
      if(aKey.thePtCode<=31 && myGPmap.find(aKey)!=myGPmap.end()) aGP2 = myGPmap.find(aKey)->second;
      --aKey.thePtCode;
      if(myGPmap.find(aKey)!=myGPmap.end()) aGP1 = myGPmap.find(aKey)->second;
      myWriter->writeGPData(*aGP1,*aGP2);

      ++aKey.thePtCode;
      ++aKey.thePtCode;
      aKey.theCharge = -1;
      
    }   
    fName = "GPs_2x.xml";
    myWriter->finaliseXMLDocument(fName);  
    //////////////////////////////////////////////
    ///4x merging
    fName = "OMTF";
    myWriter->initialiseXMLDocument(fName);

    GoldenPattern *dummy = new GoldenPattern(Key(0,0,0));
    dummy->reset();

    aKey = Key(1, iPtMin, charge);    
    while(myGPmap.find(aKey)!=myGPmap.end()){

    GoldenPattern *aGP1 = myGPmap.find(aKey)->second;
    GoldenPattern *aGP2 = aGP1;
    GoldenPattern *aGP3 = aGP1;
    GoldenPattern *aGP4 = aGP1;

    ++aKey.thePtCode;
    if(aKey.thePtCode<=31 && myGPmap.find(aKey)!=myGPmap.end()) aGP2 =  myGPmap.find(aKey)->second;
    
    if(aKey.thePtCode>19){
      ++aKey.thePtCode;
      if(aKey.thePtCode<=31 && myGPmap.find(aKey)!=myGPmap.end()) aGP3 =  myGPmap.find(aKey)->second;
      
      ++aKey.thePtCode;
      if(aKey.thePtCode<=31 && myGPmap.find(aKey)!=myGPmap.end()) aGP4 =  myGPmap.find(aKey)->second;
    }
    else{
      aGP3 = dummy;
      aGP4 = dummy;
    }
    ++aKey.thePtCode;

    myWriter->writeGPData(*aGP1,*aGP2, *aGP3, *aGP4);
    }   
    ///
    aKey = Key(1, iPtMin,1);    
    while(myGPmap.find(aKey)!=myGPmap.end()){

    GoldenPattern *aGP1 = myGPmap.find(aKey)->second;
    GoldenPattern *aGP2 = aGP1;
    GoldenPattern *aGP3 = aGP1;
    GoldenPattern *aGP4 = aGP1;

    ++aKey.thePtCode;
    if(aKey.thePtCode<=31 && myGPmap.find(aKey)!=myGPmap.end()) aGP2 =  myGPmap.find(aKey)->second;
    
    if(aKey.thePtCode>19){
      ++aKey.thePtCode;
      if(aKey.thePtCode<=31 && myGPmap.find(aKey)!=myGPmap.end()) aGP3 =  myGPmap.find(aKey)->second;
      
      ++aKey.thePtCode;
      if(aKey.thePtCode<=31 && myGPmap.find(aKey)!=myGPmap.end()) aGP4 =  myGPmap.find(aKey)->second;
    }
    else{
      aGP3 = dummy;
      aGP4 = dummy;
    }
    ++aKey.thePtCode;

    myWriter->writeGPData(*aGP1,*aGP2, *aGP3, *aGP4);
    }   

    fName = "GPs_4x.xml";
    myWriter->finaliseXMLDocument(fName); 
  }

  if(makeConnectionsMaps && !dumpGPToXML && !dumpResultToXML){
    std::string fName = "Connections.xml";  
    myWriter->writeConnectionsData(OMTFConfiguration::measurements4D);
    myWriter->finaliseXMLDocument(fName);
    myOMTFConfigMaker->printPhiMap(std::cout);
    myOMTFConfigMaker->printConnections(std::cout,0,0);
  }
}
/////////////////////////////////////////////////////
/////////////////////////////////////////////////////  
void OMTFProducer::produce(edm::Event& iEvent, const edm::EventSetup& evSetup){

  myInputMaker->initialize(evSetup);

  edm::Handle<TriggerPrimitiveCollection> trigPrimitives;
  iEvent.getByToken(inputToken, trigPrimitives);

  ///Filter digis by dropping digis from selected (by cfg.py) subsystems
  const L1TMuon::TriggerPrimitiveCollection filteredDigis = filterDigis(*trigPrimitives);

  std::auto_ptr<std::vector<L1MuRegionalCand> > myCands(new std::vector<L1MuRegionalCand>);

  if(dumpResultToXML) aTopElement = myWriter->writeEventHeader(iEvent.id().event());

  ///Loop over all processors, each covering 60 deg in phi
  for(unsigned int iProcessor=0;iProcessor<6;++iProcessor){
    
    edm::LogInfo("OMTF ROOTReader")<<"iProcessor: "<<iProcessor;
    
    const OMTFinput *myInput = myInputMaker->buildInputForProcessor(filteredDigis,iProcessor);
       
    ///Input data with phi ranges shifted for each processor, so it fits 10 bits range
    const OMTFinput myShiftedInput =  myOMTF->shiftInput(iProcessor,*myInput);	
       
    /////////////////////////
    
    ///Results for each GP in each logic region of given processor
    const std::vector<OMTFProcessor::resultsMap> & myResults = myOMTF->processInput(iProcessor,myShiftedInput);

    ///At the moment allow up to two, opposite charge, candidates per processor.    
    L1MuRegionalCand myOTFCandidatePlus = mySorter->sortProcessor(myResults,1);
    //L1MuRegionalCand myOTFCandidateMinus = mySorter->sortProcessor(myResults,-1);

    ////Switch from internal processor n bit scale to global one
    int procOffset = OMTFConfiguration::globalPhiStart(iProcessor);
    int lowScaleEnd = pow(2,OMTFConfiguration::nPhiBits-1);

    if(procOffset<0) procOffset+=OMTFConfiguration::nPhiBins;

    float phiValue = (myOTFCandidatePlus.phiValue()+OMTFConfiguration::globalPhiStart(iProcessor)+lowScaleEnd)/OMTFConfiguration::nPhiBins*2*M_PI;
    if(phiValue>M_PI) phiValue-=2*M_PI;
    myOTFCandidatePlus.setPhiValue(phiValue);

    //phiValue = (myOTFCandidateMinus.phiValue()+OMTFConfiguration::globalPhiStart(iProcessor)+lowScaleEnd)/OMTFConfiguration::nPhiBins*2*M_PI;
    //if(phiValue>M_PI) phiValue-=2*M_PI;
    //myOTFCandidateMinus.setPhiValue(phiValue);
    //////////////////
    if(myOTFCandidatePlus.pt_packed()) myCands->push_back(myOTFCandidatePlus); 
    //if(myOTFCandidateMinus.pt_packed()) myCands->push_back(myOTFCandidateMinus); 

    ///////////////DEBUG
    if(myOTFCandidatePlus.pt_packed()>20 && myOTFCandidatePlus.bx()%100>3){
      std::cout<<"pt OMTF: "<<myOTFCandidatePlus.pt_packed()<<std::endl;
      std::cout<<"ref layer OMTF: "<<myOTFCandidatePlus.bx()%1000<<std::endl;
      std::cout<<myShiftedInput<<std::endl;
    }
    ////////////////////
   
    ///Write to XML
    if(dumpResultToXML){
      xercesc::DOMElement * aProcElement = myWriter->writeEventData(aTopElement,iProcessor,myShiftedInput);
      for(unsigned int iRefHit=0;iRefHit<OMTFConfiguration::nTestRefHits;++iRefHit){
	///Dump only regions, where a candidate was found
	InternalObj myCand = mySorter->sortRefHitResults(myResults[iRefHit],1);
	if(myCand.pt){
	  myWriter->writeCandidateData(aProcElement,iRefHit,myCand);
	  for(auto & itKey: myResults[iRefHit]) myWriter->writeResultsData(aProcElement, 
									   iRefHit,
									   itKey.first,itKey.second);    
	}
      }
    }
  }
  iEvent.put(myCands, "OMTF");
}
/////////////////////////////////////////////////////
/////////////////////////////////////////////////////  
const L1TMuon::TriggerPrimitiveCollection OMTFProducer::filterDigis(const L1TMuon::TriggerPrimitiveCollection & vDigi){

  if(!theConfig.getParameter<bool>("dropRPCPrimitives") &&
     !theConfig.getParameter<bool>("dropDTPrimitives") &&
     !theConfig.getParameter<bool>("dropCSCPrimitives")) return vDigi;
  
  L1TMuon::TriggerPrimitiveCollection filteredDigis;
  for(auto it:vDigi){
    switch (it.subsystem()) {
    case L1TMuon::TriggerPrimitive::kRPC: {
      if(!theConfig.getParameter<bool>("dropRPCPrimitives")) filteredDigis.push_back(it);
      break;
    }
    case L1TMuon::TriggerPrimitive::kDT: {
      if(!theConfig.getParameter<bool>("dropDTPrimitives")) filteredDigis.push_back(it);
      break;
    }
    case L1TMuon::TriggerPrimitive::kCSC: {
      if(!theConfig.getParameter<bool>("dropCSCPrimitives")) filteredDigis.push_back(it);
      break;
    }
    case L1TMuon::TriggerPrimitive::kNSubsystems: {break;} 
    }
  }
  return filteredDigis;
}
/////////////////////////////////////////////////////
/////////////////////////////////////////////////////  
