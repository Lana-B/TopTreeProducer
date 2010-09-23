#include "../interface/TopTreeProducer.h"

#include "DataFormats/HLTReco/interface/TriggerEvent.h"
#include "DataFormats/Common/interface/Handle.h"

using namespace std;
using namespace TopTree;
using namespace reco;
using namespace edm;


TopTreeProducer::TopTreeProducer(const edm::ParameterSet& iConfig)
{
	myConfig_ = iConfig.getParameter<ParameterSet>("myConfig");
	dataType_ = myConfig_.getUntrackedParameter<string>("dataType","unknown");
	cout << "dataType: " << dataType_ << endl;
	if( dataType_=="RECO" )				producersNames_ = iConfig.getParameter<ParameterSet>("producersNamesRECO");
	else if( dataType_=="AOD" )		producersNames_ = iConfig.getParameter<ParameterSet>("producersNamesAOD");
	else if( dataType_=="PATAOD" )	producersNames_ = iConfig.getParameter<ParameterSet>("producersNamesPATAOD");
	else if( dataType_=="PAT" )		producersNames_ = iConfig.getParameter<ParameterSet>("producersNamesPAT");
	else { cout << "TopTreeProducer::TopTreeProducer...   dataType is unknown...  exiting..." << endl; exit(1); }
}


TopTreeProducer::~TopTreeProducer()
{
}


// ------------ method called once each job just before starting event loop  ------------
void TopTreeProducer::beginJob()
{

	// Load Config parameters	
	verbosity = myConfig_.getUntrackedParameter<int>("verbosity", 0);
	rootFileName_ = myConfig_.getUntrackedParameter<string>("RootFileName","noname.root");
	doHLT8E29 = myConfig_.getUntrackedParameter<bool>("doHLT8E29",false);
	doHLT = myConfig_.getUntrackedParameter<bool>("doHLT",false);
	doMC = myConfig_.getUntrackedParameter<bool>("doMC",false);
	doPDFInfo = myConfig_.getUntrackedParameter<bool>("doPDFInfo",false);
	doPrimaryVertex = myConfig_.getUntrackedParameter<bool>("doPrimaryVertex",false);
	runGeneralTracks = myConfig_.getUntrackedParameter<bool>("runGeneralTracks",false);
	doCaloJet = myConfig_.getUntrackedParameter<bool>("doCaloJet",false);
	doCaloJetStudy = myConfig_.getUntrackedParameter<bool>("doCaloJetStudy",false);
	doGenJet = myConfig_.getUntrackedParameter<bool>("doGenJet",false);
	doGenJetStudy = myConfig_.getUntrackedParameter<bool>("doGenJetStudy",false);
	doPFJet = myConfig_.getUntrackedParameter<bool>("doPFJet",false);
	doPFJetStudy = myConfig_.getUntrackedParameter<bool>("doPFJetStudy",false);
	doJPTJet = myConfig_.getUntrackedParameter<bool>("doJPTJet",false);
	doJPTJetStudy = myConfig_.getUntrackedParameter<bool>("doJPTJetStudy",false);
	doMuon = myConfig_.getUntrackedParameter<bool>("doMuon",false);
	doCosmicMuon = myConfig_.getUntrackedParameter<bool>("doCosmicMuon",false);
	doElectron = myConfig_.getUntrackedParameter<bool>("doElectron",false);	
	doCaloMET = myConfig_.getUntrackedParameter<bool>("doCaloMET",false);
	doPFMET = myConfig_.getUntrackedParameter<bool>("doPFMET",false);
	doTCMET = myConfig_.getUntrackedParameter<bool>("doTCMET",false);
	doMHT = myConfig_.getUntrackedParameter<bool>("doMHT",false);
	drawMCTree = myConfig_.getUntrackedParameter<bool>("drawMCTree",false);
	doGenEvent = myConfig_.getUntrackedParameter<bool>("doGenEvent",false);
	doNPGenEvent = myConfig_.getUntrackedParameter<bool>("doNPGenEvent",false);
	doSpinCorrGen = myConfig_.getUntrackedParameter<bool>("doSpinCorrGen",false);
	doSemiLepEvent = myConfig_.getUntrackedParameter<bool>("doSemiLepEvent",false);
	vector<string> defaultVec;
	vector<string> defaultVecCM;
	vGenJetProducer = producersNames_.getUntrackedParameter<vector<string> >("vgenJetProducer",defaultVec);
	vCaloJetProducer = producersNames_.getUntrackedParameter<vector<string> >("vcaloJetProducer",defaultVec);
	vPFJetProducer = producersNames_.getUntrackedParameter<vector<string> >("vpfJetProducer",defaultVec);
	vJPTJetProducer = producersNames_.getUntrackedParameter<vector<string> >("vJPTJetProducer",defaultVec);
	vCosmicMuonProducer = producersNames_.getUntrackedParameter<vector<string> >("vcosmicMuonProducer",defaultVecCM);

	for(unsigned int s=0;s<vGenJetProducer.size();s++){
		TClonesArray* a;
		vgenJets.push_back(a);
	}

	for(unsigned int s=0;s<vCaloJetProducer.size();s++){
		TClonesArray* a;
		vcaloJets.push_back(a);
	}

	for(unsigned int s=0;s<vPFJetProducer.size();s++){
		TClonesArray* a;
		vpfJets.push_back(a);
	}

	for(unsigned int s=0;s<vJPTJetProducer.size();s++){
		TClonesArray* a;
		vjptJets.push_back(a);
	}

	for(unsigned int s=0;s<vCosmicMuonProducer.size();s++){
		TClonesArray* a;
		vcosmicMuons.push_back(a); 

		vector<TClonesArray*> trackVector;
                for (unsigned int t=0; t<3; t++)
		  trackVector.push_back(a);

		vcosmicMuonTracks.push_back(trackVector);
	}

	nTotEvt_ = 0;
	
	// initialize root output file
	rootFile_ = new TFile(rootFileName_.c_str(), "recreate");
	rootFile_->cd();
	if(verbosity>0) cout << "New RootFile " << rootFileName_.c_str() << " is created" << endl;

	runInfos_ = new TRootRun();
	runTree_ = new TTree("runTree", "Global Run Infos");
	runTree_->Branch ("runInfos", "TopTree::TRootRun", &runInfos_);
	if(verbosity>0) cout << "RunTree is created" << endl;

	rootEvent = 0;
	eventTree_ = new TTree("eventTree", "Event Infos");
	eventTree_->Branch ("Event", "TopTree::TRootEvent", &rootEvent);
	if(verbosity>0) cout << "EventTree is created" << endl;

	if(doHLT8E29 || doHLT)
	{
		if(verbosity>0) cout << "HLT info will be added to rootuple" << endl;
		hltAnalyzer_ = new HLTAnalyzer(producersNames_, myConfig_);
		hltAnalyzer_->setVerbosity(verbosity);
	}

	if(doMC)
	{
		if(verbosity>0) cout << "MC Particles info will be added to rootuple" << endl;
		mcParticles = new TClonesArray("TopTree::TRootMCParticle", 1000);
		eventTree_->Branch ("MCParticles", "TClonesArray", &mcParticles);
	}

	if(doCaloJet)
	{
		if(verbosity>0) cout << "CaloJets info will be added to rootuple" << endl;
		caloJets = new TClonesArray("TopTree::TRootCaloJet", 1000);
		eventTree_->Branch ("CaloJets", "TClonesArray", &caloJets);
	}
	
	if(doCaloJetStudy)
	{
		if(verbosity>0) cout << "CaloJets info will be added to rootuple (for CaloJetStudy)" << endl;
		for(unsigned int s=0;s<vCaloJetProducer.size();s++)
		{
			vcaloJets[s] = new TClonesArray("TopTree::TRootCaloJet", 1000);
			char name[100];
			sprintf(name,"CaloJets_%s",vCaloJetProducer[s].c_str());
			eventTree_->Branch (name, "TClonesArray", &vcaloJets[s]);
		}
	}

	if(doGenJet)
	{
		if(verbosity>0) cout << "GenJets info will be added to rootuple" << endl;
		genJets = new TClonesArray("TopTree::TRootGenJet", 1000);
		eventTree_->Branch ("GenJets", "TClonesArray", &genJets);
	}

	if(doGenJetStudy)
	{
		if(verbosity>0) cout << "GenJets info will be added to rootuple (for GenJetStudy)" << endl;
		for(unsigned int s=0; s<vGenJetProducer.size(); s++)
		{
			vgenJets[s] = new TClonesArray("TopTree::TRootGenJet", 1000);
			char name[100];
			sprintf(name,"GenJets_%s",vGenJetProducer[s].c_str());
			eventTree_->Branch (name, "TClonesArray", &vgenJets[s]);
		}
	}

	if(doPFJet)
	{
		if(verbosity>0) cout << "PFJets info will be added to rootuple" << endl;
		pfJets = new TClonesArray("TopTree::TRootPFJet", 1000);
		eventTree_->Branch ("PFJets", "TClonesArray", &pfJets);
	}
	
	if(doPFJetStudy)
	{
		if(verbosity>0) cout << "PFJets info will be added to rootuple (for PFJetStudy)" << endl;
		for(unsigned int s=0;s<vPFJetProducer.size();s++)
		{
			vpfJets[s] = new TClonesArray("TopTree::TRootPFJet", 1000);
			char name[100];
			sprintf(name,"PFJets_%s",vPFJetProducer[s].c_str());
			eventTree_->Branch (name, "TClonesArray", &vpfJets[s]);
		}
	}

	if(doJPTJet)
	{
		if(verbosity>0) cout << "JPTJets info will be added to rootuple" << endl;
		jptJets = new TClonesArray("TopTree::TRootJPTJet", 1000);
		eventTree_->Branch ("JPTJets", "TClonesArray", &jptJets);
	}

	if(doJPTJetStudy)
	{
		if(verbosity>0) cout << "JPT Jets info will be added to rootuple (for JPTJetStudy)" << endl;
		for(unsigned int s=0;s<vJPTJetProducer.size();s++)
		{
			vjptJets[s] = new TClonesArray("TopTree::TRootJPTJet", 1000);
			char name[100];
			sprintf(name,"JPTJets_%s",vJPTJetProducer[s].c_str());
			eventTree_->Branch (name, "TClonesArray", &vjptJets[s]);
		}
	}
	
	if(doGenEvent)
	{
		if(verbosity>0) cout << "GenEvent info will be added to rootuple" << endl;
		genEvent = new TClonesArray("TopTree::TRootGenEvent", 1000);
		eventTree_->Branch ("GenEvent", "TClonesArray", &genEvent);
	}

	if(doNPGenEvent)
	{
		if(verbosity>0) cout << "NPGenEvent info will be added to rootuple" << endl;
		NPgenEvent = new TClonesArray("TopTree::TRootNPGenEvent", 1000);
		eventTree_->Branch ("NPGenEvent", "TClonesArray", &NPgenEvent);
	}

	if(doSpinCorrGen)
	{
		if(verbosity>0) cout << "SpinCorrelation Gen info will be added to rootuple" << endl;
		spinCorrGen = new TClonesArray("TopTree::TRootSpinCorrGen", 1000);
		eventTree_->Branch ("SpinCorrGen", "TClonesArray", &spinCorrGen);
	}
    
	if(doSemiLepEvent)
	{
		if(verbosity>0) cout << "SemiLepEvent info will be added to rootuple" << endl;
		semiLepEvent = new TClonesArray("TopTree::TRootSemiLepEvent", 1000);
		eventTree_->Branch ("SemiLepEvent", "TClonesArray", &semiLepEvent);
	}

	if(doMuon)
	{
		if(verbosity>0) cout << "Muons info will be added to rootuple" << endl;
		muons = new TClonesArray("TopTree::TRootMuon", 1000);
		eventTree_->Branch ("Muons", "TClonesArray", &muons);
	}

	if(doCosmicMuon)
	{
		if(verbosity>0) cout << "Cosmic Muons info will be added to rootuple" << endl;
	     
		for(unsigned int s=0;s<vCosmicMuonProducer.size();s++){
			vcosmicMuons[s] = new TClonesArray("TopTree::TRootCosmicMuon", 1000);
			char name[100];
			sprintf(name,"CosmicMuons_%s",vCosmicMuonProducer[s].c_str());
			eventTree_->Branch (name, "TClonesArray", &vcosmicMuons[s]);

			// put the track(gl,sta,tr) branches for this muoncollection

		        vcosmicMuonTracks[s][0] = new TClonesArray("TopTree::TRootTrack",1000);
			//char name[100];
			sprintf(name,"CosmicMuonGlobalTracks_%s",vCosmicMuonProducer[s].c_str());
			eventTree_->Branch (name, "TClonesArray", &vcosmicMuonTracks[s][0]);

			vcosmicMuonTracks[s][1] = new TClonesArray("TopTree::TRootTrack",1000);
			//char name[100];
			sprintf(name,"CosmicMuonTrackerTracks_%s",vCosmicMuonProducer[s].c_str());
			eventTree_->Branch (name, "TClonesArray", &vcosmicMuonTracks[s][1]);

			vcosmicMuonTracks[s][2] = new TClonesArray("TopTree::TRootTrack",1000);
			//char name[100];
			sprintf(name,"CosmicMuonStandAloneTracks_%s",vCosmicMuonProducer[s].c_str());
			eventTree_->Branch (name, "TClonesArray", &vcosmicMuonTracks[s][2]);

			//}
		} 
	}
	
	if(doElectron)
	{
		if(verbosity>0) cout << "Electrons info will be added to rootuple" << endl;
		electrons = new TClonesArray("TopTree::TRootElectron", 1000);
		eventTree_->Branch ("Electrons", "TClonesArray", &electrons);
	}

	if(doCaloMET)
	{
		if(verbosity>0) cout << "CaloMET info will be added to rootuple" << endl;
		CALOmet = new TClonesArray("TopTree::TRootCaloMET", 1000);
		eventTree_->Branch ("CaloMET", "TClonesArray", &CALOmet);
	}

	if(doPFMET)
	{
		if(verbosity>0) cout << "ParticleFlowMET info will be added to rootuple" << endl;
		PFmet = new TClonesArray("TopTree::TRootPFMET", 1000);
		eventTree_->Branch ("PFMET", "TClonesArray", &PFmet);
	}

	if(doTCMET)
	{
		if(verbosity>0) cout << "Track Corrected MET info will be added to rootuple" << endl;
		TCmet = new TClonesArray("TopTree::TRootMET", 1000);
		eventTree_->Branch ("TCMET", "TClonesArray", &TCmet);
	}
	
	if(doMHT && (dataType_ == "PAT" || dataType_ == "PATAOD"))
	{
		if(verbosity>0) cout << "MHT info will be added to rootuple" << endl;
		mht = new TClonesArray("TopTree::TRootMHT", 1000);
		eventTree_->Branch ("MHT", "TClonesArray", &mht);
	}
	
	if(doPrimaryVertex)
	{
		if(verbosity>0) cout << "Primary Vertex info will be added to rootuple" << endl;
		primaryVertex = new TClonesArray("TopTree::TRootVertex", 1000);
		eventTree_->Branch ("PrimaryVertex", "TClonesArray", &primaryVertex);
	}

}


// ------------ method called once each job just after ending the event loop  ------------
void TopTreeProducer::endJob()
{

	// Trigger Summary Tables
	if(doHLT8E29 || doHLT)
	{	
		cout << "Trigger Summary Tables" << endl;
		hltAnalyzer_->copySummary(runInfos_);
		hltAnalyzer_->printStats();
	}

	runTree_->Fill();
	
	std::cout << "Total number of events: " << nTotEvt_ << std::endl;
	std::cout << "Closing rootfile " << rootFile_->GetName() << std::endl;
	rootFile_->Write();
	rootFile_->Close();

}


// ------------ method called to for each event  ------------
void TopTreeProducer::analyze(const edm::Event& iEvent, const edm::EventSetup& iSetup)
{
	rootFile_->cd();
	nTotEvt_++;
	if( (verbosity>1) || (verbosity>0 && nTotEvt_%10==0 && nTotEvt_<=100)  || (verbosity>0 && nTotEvt_%100==0 && nTotEvt_>100) )
		cout << endl << endl 
			<< "####### TopTreeProducer - Cumulated Events " << nTotEvt_
			<< " - Run " << iEvent.id().run() 
			<< " - Event " << iEvent.id().event() 
			<< " #######" << endl;

	// Global Event Infos
	rootEvent = new TRootEvent();
	rootEvent->setNb(nTotEvt_);
	rootEvent->setEventId(iEvent.id().event());
	rootEvent->setRunId(iEvent.id().run());
	rootEvent->setLumiBlockId(iEvent.luminosityBlock());

	// we need to store some triggerFilter info to be able to emulate triggers on older data

	std::map<std::string, std::vector<double> > triggerFilters;

	// get Trigger summary from Event
	edm::Handle<trigger::TriggerEvent> summary;
	edm::InputTag summaryTag_("hltTriggerSummaryAOD","","HLT");
	iEvent.getByLabel(summaryTag_,summary);
	
	for (unsigned int i=0; i<summary->sizeFilters(); i++) {
	  //cout << i << " -> " << summary->filterTag(i).label() << endl;

	  // get all trigger objects corresponding to this module.
	  // loop through them and see how many objects match the selection
	  const trigger::Keys& KEYS (summary->filterKeys(i));
	  const int n1(KEYS.size());
	    
	  for (int i=0; i!=n1; ++i) {
	    const trigger::TriggerObject& triggerObject( summary-> 
							 getObjects().at(KEYS[i]) );
	    //cout << "pt " << triggerObject.pt() << endl;
	    
	    triggerFilters[string(summary->filterTag(i).label())].push_back(triggerObject.pt());
	    
	  }
	
	}

	rootEvent->setTriggerFilters(triggerFilters);

	if(runGeneralTracks) // Calculate and fill number of tracks and number of high purity tracks
	{
		// get GeneralTracks collection
		edm::Handle<reco::TrackCollection> tkRef;
		iEvent.getByLabel("generalTracks",tkRef);    
		const reco::TrackCollection* tkColl = tkRef.product();

		if(verbosity>1) std::cout << "Total Number of Tracks " << tkColl->size() << endl;
		rootEvent->setNTracks(tkColl->size());

		int numhighpurity=0;
		reco::TrackBase::TrackQuality _trackQuality = reco::TrackBase::qualityByName("highPurity");

		reco::TrackCollection::const_iterator itk = tkColl->begin();
		reco::TrackCollection::const_iterator itk_e = tkColl->end();
		for(;itk!=itk_e;++itk)
		{
			if(verbosity>1) std::cout << "HighPurity?  " << itk->quality(_trackQuality) << std::endl;
			if(itk->quality(_trackQuality)) numhighpurity++;
		}

		if(verbosity>1) std::cout << "Total Number of HighPurityTracks " << numhighpurity << endl;
		rootEvent->setNHighPurityTracks(numhighpurity);
	}


	// Trigger
	rootEvent->setGlobalHLT(true);
	rootEvent->setGlobalHLT8E29(true);
	if(doHLT8E29 || doHLT)
	{
		if(verbosity>1) std::cout << endl << "Get TriggerResults..." << std::endl;
		if (nTotEvt_==1) hltAnalyzer_->init(iEvent, rootEvent);
		hltAnalyzer_->process(iEvent, rootEvent);
	}

	// MC Info
	if(doMC)
	{
		if(verbosity>1) cout << endl << "Analysing MC info..." << endl;
		MCAnalyzer* myMCAnalyzer = new MCAnalyzer(myConfig_, producersNames_);
		myMCAnalyzer->SetVerbosity(verbosity);
		if (drawMCTree) myMCAnalyzer->DrawMCTree(iEvent, iSetup, myConfig_, producersNames_);
		if ( dataType_=="RECO" && doPDFInfo ) myMCAnalyzer->PDFInfo(iEvent, rootEvent);
		myMCAnalyzer->ProcessMCParticle(iEvent, mcParticles);
		delete myMCAnalyzer;
	}

	// Get Primary Vertices
	if(doPrimaryVertex)
	{
		if(verbosity>1) cout << endl << "Analysing primary vertices collection..." << endl;
		VertexAnalyzer* myVertexAnalyzer = new VertexAnalyzer(producersNames_, verbosity);
		myVertexAnalyzer->Process(iEvent, primaryVertex);
		delete myVertexAnalyzer;
	}

	// CaloJet
	if(doCaloJet)
	{
		if(verbosity>1) cout << endl << "Analysing CaloJets collection..." << endl;
		CaloJetAnalyzer* myCaloJetAnalyzer = new CaloJetAnalyzer(producersNames_, myConfig_, verbosity);
		myCaloJetAnalyzer->Process(iEvent, caloJets);
		delete myCaloJetAnalyzer;
	}

	if(doCaloJetStudy)
	{
		if(verbosity>1) cout << endl << "Analysing Calojets collection (for JetStudy)..." << endl;
		for(unsigned int s=0;s<vCaloJetProducer.size();s++){
			CaloJetAnalyzer* myCaloJetAnalyzer = new CaloJetAnalyzer(producersNames_, s, myConfig_, verbosity);
			myCaloJetAnalyzer->Process(iEvent, vcaloJets[s]);
			delete myCaloJetAnalyzer;
		}
	}

	// GenJet
	if(doGenJet)
	{
		if(verbosity>1) cout << endl << "Analysing GenJets collection..." << endl;
		GenJetAnalyzer* myGenJetAnalyzer = new GenJetAnalyzer(producersNames_, myConfig_, verbosity);
		myGenJetAnalyzer->Process(iEvent, genJets);
		delete myGenJetAnalyzer;
	}

	if(doGenJetStudy)
	{
		if(verbosity>1) cout << endl << "Analysing GenJets collection (for GenJetStudy)..." << endl;
		for(unsigned int s=0; s<vGenJetProducer.size(); s++)
		{
			GenJetAnalyzer* myGenJetAnalyzer = new GenJetAnalyzer(producersNames_, s, myConfig_, verbosity);
			myGenJetAnalyzer->Process(iEvent, vgenJets[s]);
			delete myGenJetAnalyzer;
		}
	}

	// PFJet
	if(doPFJet)
	{
		if(verbosity>1) cout << endl << "Analysing PFJets collection..." << endl;
		PFJetAnalyzer* myPFJetAnalyzer = new PFJetAnalyzer(producersNames_, myConfig_, verbosity);
		myPFJetAnalyzer->Process(iEvent, pfJets);
		delete myPFJetAnalyzer;
	}

	if(doPFJetStudy)
	{
		if(verbosity>1) cout << endl << "Analysing PFjets collection (for JetStudy)..." << endl;
		for(unsigned int s=0;s<vPFJetProducer.size();s++){
			PFJetAnalyzer* myPFJetAnalyzer = new PFJetAnalyzer(producersNames_, s,  myConfig_, verbosity);
			myPFJetAnalyzer->Process(iEvent, vpfJets[s]);
			delete myPFJetAnalyzer;
		}
	}

	// JPT Jets

	if(doJPTJet)
	{
		if(verbosity>1) cout << endl << "Analysing JPTJets collection..." << endl;
		JPTJetAnalyzer* myJPTJetAnalyzer = new JPTJetAnalyzer(producersNames_, myConfig_, verbosity);
		myJPTJetAnalyzer->Process(iEvent, jptJets);
		delete myJPTJetAnalyzer;
	}

	if(doJPTJetStudy)
	{
		if(verbosity>1) cout << endl << "Analysing JPT jets collection (for JetStudy)..." << endl;
		for(unsigned int s=0;s<vJPTJetProducer.size();s++){
			JPTJetAnalyzer* myJPTJetAnalyzer = new JPTJetAnalyzer(producersNames_, s,  myConfig_, verbosity);
			myJPTJetAnalyzer->Process(iEvent, vjptJets[s]);
			delete myJPTJetAnalyzer;
		}
	}

	// GenEvent
	if(doGenEvent)
	{
		if(verbosity>1) cout << endl << "Analysing GenEvent collection..." << endl;
		GenEventAnalyzer* myGenEventAnalyzer = new GenEventAnalyzer(producersNames_, myConfig_, verbosity);
		myGenEventAnalyzer->Process(iEvent, genEvent);
		delete myGenEventAnalyzer;
	}

	// NPGenEvent
	if(doNPGenEvent)
	{
		if(verbosity>1) cout << endl << "Analysing NPGenEvent collection..." << endl;
		NPGenEventAnalyzer* myNPGenEventAnalyzer = new NPGenEventAnalyzer(producersNames_, myConfig_, verbosity);
		if(verbosity>1) cout << endl << "Analysing NPGenEvent collection..." << endl;
		myNPGenEventAnalyzer->Process(iEvent, NPgenEvent);
		if(verbosity>1) cout << endl << "Analysing NPGenEvent collection..." << endl;
		delete myNPGenEventAnalyzer;
	}

	// SpinCorrelation Gen
	if(doSpinCorrGen)
	{
		if(verbosity>1) cout << endl << "Analysing SpinCorrGen collection..." << endl;
		SpinCorrGenAnalyzer* mySpinCorrGenAnalyzer = new SpinCorrGenAnalyzer(producersNames_, myConfig_, verbosity);
		mySpinCorrGenAnalyzer->Process(iEvent, spinCorrGen);
		delete mySpinCorrGenAnalyzer;
	}

	//SemiLepEvent
	if(doSemiLepEvent)
	{
		if(verbosity>1) cout << endl << "Analysing SemiLepEvent collection..." << endl;
		SemiLepEventAnalyzer* mySemiLepEventAnalyzer = new SemiLepEventAnalyzer(producersNames_, myConfig_, verbosity);
		mySemiLepEventAnalyzer->Process(iEvent, semiLepEvent, caloJets, muons);
		// FIXME: add possibility to use PFjets in the SemiLepEventAnalyzer
		delete mySemiLepEventAnalyzer;
	}
	
	// Muons
	if(doMuon)
	{
		if(verbosity>1) cout << endl << "Analysing muons collection..." << endl;
		MuonAnalyzer* myMuonAnalyzer = new MuonAnalyzer(producersNames_, myConfig_, verbosity);
		myMuonAnalyzer->Process(iEvent, muons);
		delete myMuonAnalyzer;
	}

	// Cosmic Muons
	if(doCosmicMuon)
	{
	  if(verbosity>1) cout << endl << "Analysing muons collection (for cosmics)..." << endl;
     	  
	  for(unsigned int s=0;s<vCosmicMuonProducer.size();s++){
	    CosmicMuonAnalyzer *myCosmicMuonAnalyzer = new CosmicMuonAnalyzer(producersNames_, s,  myConfig_, verbosity);
	    myCosmicMuonAnalyzer->Process(iEvent, vcosmicMuons[s],vcosmicMuonTracks[s]);
	    delete myCosmicMuonAnalyzer;
	  }

	}
	

	// Lazy Tools to calculate Cluster shape variables
	EcalClusterLazyTools* lazyTools = 0;
	if( (dataType_=="RECO" || dataType_=="AOD" || dataType_=="PATAOD") && ( doElectron )  )
	{
		if(verbosity>1) cout << endl << "Loading egamma LazyTools..." << endl;
		edm::InputTag reducedBarrelEcalRecHitCollection_ = producersNames_.getParameter<edm::InputTag>("reducedBarrelEcalRecHitCollection");
		edm::InputTag reducedEndcapEcalRecHitCollection_ = producersNames_.getParameter<edm::InputTag>("reducedEndcapEcalRecHitCollection");
		// FIXME - Test availability of reducedEcalRecHits...
		lazyTools = new EcalClusterLazyTools( iEvent, iSetup, reducedBarrelEcalRecHitCollection_, reducedEndcapEcalRecHitCollection_ );
	}

	// Electrons
	if(doElectron)
	{
		if(verbosity>1) cout << endl << "Analysing electrons collection..." << endl;
		ElectronAnalyzer* myElectronAnalyzer = new ElectronAnalyzer(producersNames_, myConfig_, verbosity);
		myElectronAnalyzer->Process(iEvent, electrons, *lazyTools, iSetup);
		delete myElectronAnalyzer;
	}

	// MET 
	if(doCaloMET)
	{
		if(verbosity>1) cout << endl << "Analysing Calorimeter Missing Et..." << endl;
		CaloMETAnalyzer* myMETAnalyzer = new CaloMETAnalyzer(producersNames_, myConfig_, verbosity);
		myMETAnalyzer->Process(iEvent, CALOmet);
		delete myMETAnalyzer;
	}

	if(doPFMET)
	{
		if(verbosity>1) cout << endl << "Analysing ParticleFlow Missing Et..." << endl;
		PFMETAnalyzer* myMETAnalyzer = new PFMETAnalyzer(producersNames_, myConfig_, verbosity);
		myMETAnalyzer->Process(iEvent, PFmet);
		delete myMETAnalyzer;
	}

	if(doTCMET)
	{
		if(verbosity>1) cout << endl << "Analysing Track Corrected Missing Et..." << endl;
		TCMETAnalyzer* myMETAnalyzer = new TCMETAnalyzer(producersNames_, myConfig_, verbosity);
		myMETAnalyzer->Process(iEvent, TCmet);
		delete myMETAnalyzer;
	}
	
	// MHT 
	if(doMHT && (dataType_ == "PAT" || dataType_ == "PATAOD"))
	{
		if(verbosity>1) cout << endl << "Analysing MHT..." << endl;
		MHTAnalyzer* myMHTAnalyzer = new MHTAnalyzer(producersNames_, myConfig_, verbosity);
		myMHTAnalyzer->Process(iEvent, mht);
		delete myMHTAnalyzer;
	}
	
	// Associate recoParticles to mcParticles
	if(doMC)
	{
		MCAssociator* myMCAssociator = new MCAssociator(producersNames_, verbosity);
		myMCAssociator->init(iEvent, mcParticles);
		if(doCaloJet) myMCAssociator->process(caloJets);
		if(doPFJet) myMCAssociator->process(pfJets);
		// FIXME: do MC association if multiple jets are used (with jetStudy stuff)
		if(doMuon) myMCAssociator->process(muons);
		if(doElectron) myMCAssociator->process(electrons);
		if(doCaloMET) myMCAssociator->process(CALOmet);
		//if(verbosity>2 && doJet) myMCAssociator->printParticleAssociation(jets);
		//if(verbosity>2 && doMuon) myMCAssociator->printParticleAssociation(muons);
		//if(verbosity>2 && doElectron) myMCAssociator->printParticleAssociation(electrons);
		//if(verbosity>2 && doPhoton) myMCAssociator->printParticleAssociation(photons);
		//if(verbosity>2 && doCaloMET) myMCAssociator->printParticleAssociation(CALOmet);
		delete myMCAssociator;
	}


	if(verbosity>1) cout << endl << "Filling rootuple..." << endl;
	eventTree_->Fill();
	if(verbosity>1) cout << endl << "Deleting objects..." << endl;
	delete rootEvent;
	if(doMC) (*mcParticles).Delete();
	if(doCaloJet) (*caloJets).Delete();
	if(doCaloJetStudy)
	{
		for(unsigned int s=0;s<vCaloJetProducer.size();s++)
		{
			(*vcaloJets[s]).Delete();
		}
	}
	if(doGenJet) (*genJets).Delete();
	if(doGenJetStudy)
	{
		for(unsigned int s=0;s<vGenJetProducer.size();s++)
		{
			(*vgenJets[s]).Delete();
		}		
	}
	if(doPFJet) (*pfJets).Delete();
	if(doPFJetStudy){
		for(unsigned int s=0;s<vPFJetProducer.size();s++){
			(*vpfJets[s]).Delete();
		}
	}
	if(doJPTJet) (*jptJets).Delete();
	if(doJPTJetStudy){
		for(unsigned int s=0;s<vJPTJetProducer.size();s++){
			(*vjptJets[s]).Delete();
		}
	}
	if(doMuon) (*muons).Delete();
       	if(doCosmicMuon) {
	
	  for(unsigned int s=0;s<vCosmicMuonProducer.size();s++){
	    (*vcosmicMuons[s]).Delete();
	  }

	}

	if(doElectron) (*electrons).Delete();
	if(doCaloMET) (*CALOmet).Delete();
	if(doPFMET) (*PFmet).Delete();
	if(doTCMET) (*TCmet).Delete();
	if(doMHT && (dataType_ == "PAT" || dataType_ == "PATAOD")) (*mht).Delete();
	if(doGenEvent) (*genEvent).Delete();
	if(doNPGenEvent) (*NPgenEvent).Delete();
	if(doSpinCorrGen) (*spinCorrGen).Delete();
	if(doSemiLepEvent) (*semiLepEvent).Delete();
	if(doPrimaryVertex) (*primaryVertex).Delete();
	if(verbosity>0) cout << endl;

}

