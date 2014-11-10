import FWCore.ParameterSet.Config as cms
process = cms.Process("MakePatterns")
import os
import sys

process.load("FWCore.MessageLogger.MessageLogger_cfi")
process.MessageLogger = cms.Service("MessageLogger",
       suppressInfo       = cms.untracked.vstring('AfterSource', 'PostModule'),
       destinations   = cms.untracked.vstring(
                                             'detailedInfo'
                                             ,'critical'
                                             ,'cout'
                    ),
       categories = cms.untracked.vstring(
                                        'CondDBESSource'
                                        ,'EventSetupDependency'
                                        ,'Geometry'
                                        ,'MuonGeom'
                                        ,'GetManyWithoutRegistration'
                                        ,'GetByLabelWithoutRegistration'
                                        ,'Alignment'
                                        ,'SiStripBackPlaneCorrectionDepESProducer'
                                        ,'SiStripLorentzAngleDepESProducer'
                                        ,'SiStripQualityESProducer'
                                        ,'TRACKER'
                                        ,'HCAL'
        ),
       critical       = cms.untracked.PSet(
                        threshold = cms.untracked.string('ERROR') 
        ),
       detailedInfo   = cms.untracked.PSet(
                      threshold  = cms.untracked.string('INFO'), 
                      CondDBESSource  = cms.untracked.PSet (limit = cms.untracked.int32(0) ), 
                      EventSetupDependency  = cms.untracked.PSet (limit = cms.untracked.int32(0) ), 
                      Geometry  = cms.untracked.PSet (limit = cms.untracked.int32(0) ), 
                      MuonGeom  = cms.untracked.PSet (limit = cms.untracked.int32(0) ), 
                      Alignment  = cms.untracked.PSet (limit = cms.untracked.int32(0) ), 
                      GetManyWithoutRegistration  = cms.untracked.PSet (limit = cms.untracked.int32(0) ), 
                      GetByLabelWithoutRegistration  = cms.untracked.PSet (limit = cms.untracked.int32(0) ) 

       ),
       cout   = cms.untracked.PSet(
                threshold  = cms.untracked.string('INFO'), 
                CondDBESSource  = cms.untracked.PSet (limit = cms.untracked.int32(0) ), 
                EventSetupDependency  = cms.untracked.PSet (limit = cms.untracked.int32(0) ), 
                Geometry  = cms.untracked.PSet (limit = cms.untracked.int32(0) ), 
                MuonGeom  = cms.untracked.PSet (limit = cms.untracked.int32(0) ), 
                Alignment  = cms.untracked.PSet (limit = cms.untracked.int32(0) ), 
                GetManyWithoutRegistration  = cms.untracked.PSet (limit = cms.untracked.int32(0) ), 
                GetByLabelWithoutRegistration  = cms.untracked.PSet (limit = cms.untracked.int32(0) ) 
       ),
)
#Uncomment it to switch off messages
#process.MessageLogger = cms.Service("MessageLogger")
process.MessageLogger.cout = cms.untracked.PSet(INFO = cms.untracked.PSet(
        reportEvery = cms.untracked.int32(1) # every 100th only
     ))



process.source = cms.Source(
    'PoolSource',
    fileNames = cms.untracked.vstring('file:/home/akalinow/scratch/CMS/OverlapTrackFinder/Crab/Test/Ipt_16m_720_FullEta_Test_v2/crab_0_141028_111133/res/SingleMu_16_m_1_1_hvk.root',
                                      'file:/home/akalinow/scratch/CMS/OverlapTrackFinder/Crab/Test/Ipt_16m_720_FullEta_Test_v2/crab_0_141028_111133/res/SingleMu_16_m_2_1_SZ6.root'
                                      )
    )

process.maxEvents = cms.untracked.PSet( input = cms.untracked.int32(100))

###PostLS1 geometry used
process.load('Configuration.Geometry.GeometryExtendedPostLS1Reco_cff')
process.load('Configuration.Geometry.GeometryExtendedPostLS1_cff')
############################
process.load('Configuration.StandardSequences.FrontierConditions_GlobalTag_cff')
from Configuration.AlCa.GlobalTag import GlobalTag
process.GlobalTag = GlobalTag(process.GlobalTag, 'auto:run2_mc', '')

path = os.environ['CMSSW_BASE']+"/src/UserCode/OMTFSimulation/data/"


process.load('L1Trigger.L1TMuon.L1TMuonTriggerPrimitiveProducer_cfi')

process.omtfEmulator = cms.EDProducer("OMTFProducer",
                                      TriggerPrimitiveSrc = cms.InputTag('L1TMuonTriggerPrimitives'),
                                      dumpResultToXML = cms.bool(True),                                     
                                      dumpGPToXML = cms.bool(False),                                     
                                      makeConnectionsMaps = cms.bool(False),                                      
                                      omtf = cms.PSet(
        configXMLFile = cms.string(path+"hwToLogicLayer.xml"),
        patternsXMLFiles = cms.vstring(path+"Patterns_chPlus.xml",path+"Patterns_chMinus.xml"),
        )
                                      )


process.L1TMuonSeq = cms.Sequence( process.L1TMuonTriggerPrimitives +
                                   process.omtfEmulator )

process.L1TMuonPath = cms.Path(process.L1TMuonSeq)

process.schedule = cms.Schedule(process.L1TMuonPath)

