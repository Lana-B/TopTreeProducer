#include <iomanip>
#include "../interface/TRootMuon.h"
#include "../interface/TRootElectron.h"
#include "../interface/TRootJet.h"
#include "../interface/TRootCaloJet.h"
#include "../interface/TRootPFJet.h"
#include "../interface/TRootMET.h"
#include "../interface/TRootGenEvent.h"
#include "../interface/TRootEvent.h"
#include "../interface/TRootRun.h"
#include "../interface/TRootParticle.h"
#include "../interface/TRootMCParticle.h"
#include "../interface/TRootVertex.h"

#include <TFile.h>
#include <TH1.h>
#include <TH2.h>
#include <TCanvas.h>
#include <TBranch.h>
#include <TTree.h>

using namespace TopTree;

int main(){
        int verbosity                 = 1;
	//0 muet
	//1 Main Info
	//2
	//3 
	//4 Info for each event
	//5 Debug

	bool realData						= true;
	bool doHLT                    = false;
	bool doMC                     = false;
	bool doCaloJet                = true;
	bool doMuon                   = true;
	bool doElectron               = false;
	bool doMET                    = true;
	bool doGenEvent               = false;

	TFile* f = TFile::Open("dcap://maite.iihe.ac.be//pnfs/iihe/cms/store/user/echabert/TopTree.root");
	TTree* runTree = (TTree*) f->Get("runTree");
	TTree* eventTree = (TTree*) f->Get("eventTree");

	TBranch* run_br = (TBranch *) runTree->GetBranch("runInfos");
	TRootRun* runInfos = 0;
	run_br->SetAddress(&runInfos);
	
	TBranch* event_br = (TBranch *) eventTree->GetBranch("Event");
	TRootEvent* event = 0;
	event_br->SetAddress(&event);
	
	//Declartion of Branches and TClonesArray
	TBranch* mcParticles_br;
	TClonesArray* mcParticles;
	TBranch* caloJets_br;
	TClonesArray* caloJets;
	TBranch* muons_br;
	TClonesArray* muons;
	TBranch* electrons_br;
	TClonesArray* electrons;
	TBranch* genEvents_br;
	TClonesArray* genEvents;
	TBranch* mets_br;
	TClonesArray* mets;

	if(doMC)
	{
		mcParticles_br = (TBranch *) eventTree->GetBranch("MCParticles");
		mcParticles = new TClonesArray("TopTree::TRootParticle", 0);
		mcParticles_br->SetAddress(&mcParticles);
	}

	if(doCaloJet)
	{
		caloJets_br = (TBranch *) eventTree->GetBranch("CaloJets");
		caloJets = new TClonesArray("TopTree::TRootCaloJet", 0);
		caloJets_br->SetAddress(&caloJets);
	}

	
	if(doMuon)
	{
		muons_br = (TBranch *) eventTree->GetBranch("Muons");
		muons = new TClonesArray("TopTree::TRootMuon", 0);
		muons_br->SetAddress(&muons);
	}
		
	if(doElectron)
	{

		electrons_br = (TBranch *) eventTree->GetBranch("Electrons");
		electrons = new TClonesArray("TopTree::TRootElectron", 0);
		electrons_br->SetAddress(&electrons);
	}
		
        if(doMET)
	{
		mets_br = (TBranch *) eventTree->GetBranch("MET");
		mets = new TClonesArray("TopTree::TRootMET", 0);
		mets_br->SetAddress(&mets);
	}
        
	if(doGenEvent)
	{
		genEvents_br = (TBranch *) eventTree->GetBranch("GenEvent");
		genEvents = new TClonesArray("TopTree::TRootGenEvent", 0);
		genEvents_br->SetAddress(&genEvents);
	}
		
		
	// HLT Run Summary
	if (doHLT)
	{
		cout << "runTree->GetEntries()="<<runTree->GetEntries()<<endl;
		runTree->GetEvent(0);
		cout << dec << endl;
		cout << "HLT-Report " << "---------- Event  Summary ------------\n";
		cout << "HLT-Report"
				<< " Events total = " << runInfos->nHLTEvents()
				<< "  wasrun = " << runInfos->nHLTWasRun()
				<< "  passed = " << runInfos->nHLTAccept()
				<< "  errors = " << runInfos->nHLTErrors()
				<< "\n";

		cout << endl;
		cout << "HLT-Report " << "---------- HLTrig Summary ------------\n";
		cout << "HLT-Report   HLT Bit#     WasRun     Passed     Errors  Name\n";

		for (unsigned int i=0; i!=runInfos->nHLTPaths(); ++i)
		{
			printf("HLT-Report %10u %10u %10u %10u  ", i, runInfos->hltWasRun(i), runInfos->hltAccept(i), runInfos->hltErrors(i));
			cout << runInfos->hltNames(i) << endl;
		}

		cout << endl;
	}



        //Declaration of histograms
        TH1F h_PtJets("PtCaloJets","Pt of caloJets",50,0,500); 
	//

	if(realData)
	{
		if(verbosity > 3) cout << "Reading in JSON file " << endl;

		vector< vector<int> > runLumiInfo;
		string inputJSON;

		ifstream myfile ("JSON_test.txt");
		if (myfile.is_open())
		{
			getline (myfile,inputJSON); // Only the first line is needed
			myfile.close();
		}
	
		vector<string> splittedInputJSON;
		size_t begin = 2, end = 2;

		while(end < inputJSON.size())
		{
			end = inputJSON.find("]], \"",begin);
			string splitted = inputJSON.substr(begin, end - begin + 1);
			begin = end + 5;
		
			size_t tempEnd = splitted.find("\": [[", 0);
			string runNr = splitted.substr(0, tempEnd);
			stringstream ss(runNr);
			int runNumber = 0;
			ss >> runNumber;
		
			string remain = splitted.substr(tempEnd + 4, splitted.size() - ( tempEnd + 3 ) );
			size_t tempEnd2 = remain.find("]", 0);
			size_t tempBegin2 = 0;

			while(tempEnd2 < remain.size())
			{
				string lumiInfo = remain.substr(tempBegin2 + 1, tempEnd2 - tempBegin2 - 1);
				tempBegin2 = tempEnd2 + 3;
				tempEnd2 = remain.find("]", tempBegin2);
			
				// parse lumiInfo string
				size_t tempBegin3 = lumiInfo.find(", ",0);
				string minLS = lumiInfo.substr(0,tempBegin3);
				string maxLS = lumiInfo.substr(tempBegin3 + 2, lumiInfo.size());
				int minLumiSection = 0;
				int maxLumiSection = 0;
				stringstream ssMin(minLS);		
				stringstream ssMax(maxLS);
				ssMin >> minLumiSection;
				ssMax >> maxLumiSection;
		
				vector<int> tempInfo;
				tempInfo.push_back(runNumber);
				tempInfo.push_back(minLumiSection);
				tempInfo.push_back(maxLumiSection);
				runLumiInfo.push_back(tempInfo);
			}
		}
	}

	unsigned int nEvents = (int)eventTree->GetEntries();
	
	for(unsigned int ievt=0; ievt<nEvents; ievt++)
	{
		eventTree->GetEvent(ievt);
		if(verbosity>3) cout <<"event "<< ievt <<endl;
		if(verbosity>3) cout<<"event->nb()="<<event->nb()<<endl;

		bool goodEvent = true;
		if(realData)
		{
			goodEvent = false;
			for(unsigned int k=0; k<runLumiInfo.size(); k++)
			{
				if(event->runId() == runLumiInfo[k][0] && event->lumiBlockId() >= runLumiInfo[k][1] && event->lumiBlockId() <= runLumiInfo[k][2])
					goodEvent = true;
			}
		}
		
		if(goodEvent == false) continue;

		if (doHLT)
		{
			for(unsigned int ipath=36; ipath<40; ipath++) cout << "   " << runInfos->hltNames(ipath) << " decision=" << event->trigHLT(ipath) <<endl;
		}

		//access to muons
		if(doMuon){
	  	  if(verbosity>4) cout<<"Access to muons"<<endl;
                  if(verbosity>3) cout<<"Nof muons: "<<muons->GetEntriesFast()<<endl;
		  TRootMuon* muon;
		  for(int i=0;i<muons->GetEntriesFast();i++){
		    muon = (TRootMuon*) muons->At(i);
		  }
		}
		
		//access to electrons
		if(doElectron){
		  if(verbosity>4) cout<<"Access to electrons"<<endl;
                  if(verbosity>3) cout<<"Nof electrons: "<<electrons->GetEntriesFast()<<endl;
		  TRootElectron* electron;
		  for(int i=0;i<electrons->GetEntriesFast();i++){
		    electron = (TRootElectron*) electrons->At(i);
		  }
	        } 
		
		//access to calojets
		if(doCaloJet){
		  if(verbosity>4) cout<<"Access to caloJets"<<endl;
        if(verbosity>3) cout<<"Nof caloJets: "<<caloJets->GetEntriesFast()<<endl;
		  TRootCaloJet* caloJet;
		  for(int i=0;i<caloJets->GetEntriesFast();i++){
		    caloJet = (TRootCaloJet*) caloJets->At(i);
		    h_PtJets.Fill(caloJet->Pt());
		  }
      }
		
		//access to GenEvent
		if(doGenEvent){
		  if(verbosity>4) cout<<"Access to GenEvent"<<endl;
		  if(genEvents->GetEntriesFast()>0){
		    TRootGenEvent* genEvent = (TRootGenEvent*) genEvents->At(0);
		  }
		  else if(verbosity>0) cout<<" No access to GenEvent in this entry"<<endl;
		}
		
		//access to MET
		if(doMET){
		  if(verbosity>4) cout<<"Access to MET"<<endl;
		  if(mets->GetEntriesFast()>0){
		    TRootMET* met = (TRootMET*) mets->At(0);
		  }
		  else if(verbosity>0) cout<<" No access to MET in this entry"<<endl;
		}



	} // end of loop over evts

	

      if(verbosity>1) cout<<"Writting histograms in the root-file ... "<<endl;
      
      TFile* fout = new TFile("MacroTreeOutput.root","RECREATE");
      fout->cd();
      //Write histograms
      h_PtJets.Write();
      fout->Close();

      if(verbosity>1) cout<<"End of the Macro"<<endl;
     
      return(0);
}
