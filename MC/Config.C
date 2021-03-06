/*
 * AliDPG - ALICE Experiment Data Preparation Group
 * Central configuration script
 *
 */

/*****************************************************************/
/*****************************************************************/
/*****************************************************************/

#if !(defined(__CLING__)  || defined(__CINT__)) || defined(__ROOTCLING__) || defined(__ROOTCINT__)
#include "TGeant3TGeo.h"
#endif

// global variables

static Int_t   runNumber       = 0;         // run number
static Int_t   neventsConfig   = 200;       // number of events
static Int_t   magnetConfig    = 0;         // magnetic field
static Int_t   detectorConfig  = 0;         // detector
static Int_t   generatorConfig = 0;         // MC generator
static Float_t energyConfig    = 0.;        // CMS energy
static Int_t triggerConfig   = 0.;        // trigger
static Int_t   pdgConfig       = 0;         // PDG value 
static Float_t bminConfig      = 0.;        // impact parameter min
static Float_t bmaxConfig      = 20.;       // impact parameter max
static Float_t yminConfig      = -1.e6;     // rapidity min
static Float_t ymaxConfig      =  1.e6;     // rapidity max
static Float_t phiminConfig    = 0.;        // phi min
static Float_t phimaxConfig    = 360.;      // phi max
static Float_t ptminConfig     = 0.;        // pt min
static Float_t ptmaxConfig     = -1.;       // pt max
static Float_t crossingConfig  = 0.;        // 2.8e-4 // crossing angle
static Int_t   seedConfig      = 123456789; // random seed
static Int_t   uidConfig       = 1;         // unique ID
static TString processConfig   = "";        // process
static TString systemConfig    = "";        // system
static Float_t pthardminConfig = 0.;        // pt-hard min
static Float_t pthardmaxConfig = -1.;       // pt-hard max
static Float_t pttrigminConfig = 0.;        // pt-trigger min
static Float_t pttrigmaxConfig = -1.;       // pt-trigger max
static Int_t   quenchingConfig = 0;         // quenching
static Float_t qhatConfig      = 1.7;       // q-hat
static Bool_t  isGeant4        = kFALSE;    // geant4 flag
static Bool_t  purifyKine      = kTRUE;     // purifyKine flag
static Bool_t  isFluka         = kFALSE;    // fluka flag

#if ROOT_VERSION_CODE >= ROOT_VERSION(6,0,0)
#include "DetectorConfig.C"
#include "GeneratorConfig.C"
#endif

void ProcessEnvironment();
void CreateGAlice();
void GeneratorOptions();

/*****************************************************************/

void
Config()
{

  /* initialise */
#if ROOT_VERSION_CODE >= ROOT_VERSION(6,0,0)
  // in root5 the ROOT_VERSION_CODE is defined only in ACLic mode
#else  
  gROOT->LoadMacro("$ALIDPG_ROOT/MC/DetectorConfig.C");
  gROOT->LoadMacro("$ALIDPG_ROOT/MC/GeneratorConfig.C");
#endif
  ProcessEnvironment();

  /* verbose */
  printf(">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>\n");
  printf(">>>>>       run number: %d \n", runNumber);
  printf(">>>>> number of events: %d \n", neventsConfig);
  printf(">>>>>   magnetic field: %s \n", MagnetName[magnetConfig]);
  printf(">>>>>         detector: %s \n", DetectorName[detectorConfig]);
  printf(">>>>>     MC generator: %s \n", GeneratorName[generatorConfig]);
  printf(">>>>>          process: %s \n", processConfig.Data());
  printf(">>>>>           system: %s \n", systemConfig.Data());
  printf(">>>>>       CMS energy: %f \n", energyConfig);
  printf(">>>>>          trigger: %s \n", TriggerName[triggerConfig]);
  printf(">>>>>              PDG: %d \n", pdgConfig);
  printf(">>>>>            b-min: %f \n", bminConfig);
  printf(">>>>>            b-max: %f \n", bmaxConfig);
  printf(">>>>>            y-min: %f \n", yminConfig);
  printf(">>>>>            y-max: %f \n", ymaxConfig);
  printf(">>>>>   phi-min (deg.): %f \n", phiminConfig);
  printf(">>>>>   phi-max (deg.): %f \n", phimaxConfig);
  printf(">>>>>           pt-min: %f \n", ptminConfig);
  printf(">>>>>           pt-max: %f \n", ptmaxConfig);
  printf(">>>>>      pt-hard min: %f \n", pthardminConfig);
  printf(">>>>>      pt-hard max: %f \n", pthardmaxConfig);
  printf(">>>>>   pt-trigger min: %f \n", pttrigminConfig);
  printf(">>>>>   pt-trigger max: %f \n", pttrigmaxConfig);
  printf(">>>>>        quenching: %d \n", quenchingConfig);
  printf(">>>>>            q-hat: %f \n", qhatConfig);
  printf(">>>>>   crossing angle: %f \n", crossingConfig);
  printf(">>>>>      random seed: %d \n", seedConfig);
  printf(">>>>>           geant4: %d \n", isGeant4);
  printf(">>>>>       purifyKine: %d \n", purifyKine);
  printf(">>>>>            fluka: %d \n", isFluka);
  printf(">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>\n");

#if ROOT_VERSION_CODE >= ROOT_VERSION(6,0,0)
  // in root5 the ROOT_VERSION_CODE is defined only in ACLic mode
#else  
  LoadLibraries();
#endif

  /* Check that it is not set both Geant4 and Fluka */
  if (isGeant4 && isFluka) {
     printf(">>>>> You cannot have in your parameters both Geant4 and Fluka set!: isGeant4 = %d, isFluka = %d \n", (Int_t)isGeant4, (Int_t)isFluka);
     abort();
  }

  /* setup geant3 */
  if (!isGeant4 && !isFluka) new TGeant3TGeo("C++ Interface to Geant3");

  /* create galice.root */
  CreateGAlice();

  /* configure detector */
  DetectorConfig(detectorConfig);

  /* configure Geant4 if requested */
  if (isGeant4) {
    Int_t error;
    TString geant4config_macro = "$ALIDPG_ROOT/MC/Geant4Config.C";
    if (gROOT->LoadMacro(Form("%s/Geant4Config.C", gSystem->pwd()), &error, kTRUE) == 0) {
      printf(">>>>> Geant4Config.C macro detected in CWD, using that one \n");
      geant4config_macro = Form("%s/Geant4Config.C", gSystem->pwd());
    }
    gROOT->LoadMacro(geant4config_macro.Data());
    gROOT->ProcessLine("Geant4Config();");
  }

  /* configure Fluka if requested */
  if (isFluka) {
     gSystem->Load("libfluka.so");
     new TFluka("C++ Interface to Fluka", 0/*verbositylevel*/);
     TFluka* fluka = (TFluka*) gMC; 
     fluka->SetLowEnergyNeutronTransport(1);
  }

  /* configure MC generator */
  GeneratorConfig(generatorConfig);
  GeneratorOptions();

  if (!purifyKine) gAlice->GetMCApp()->PurifyLimits(80., 80.);
}

/*****************************************************************/

void
ProcessEnvironment()
{

  // run number
  if (gSystem->Getenv("CONFIG_RUN"))
    runNumber = atoi(gSystem->Getenv("CONFIG_RUN"));

  // number of events configuration
  neventsConfig = 200;
  if (gSystem->Getenv("CONFIG_NEVENTS"))
    neventsConfig = atoi(gSystem->Getenv("CONFIG_NEVENTS"));

  // magnetic field configuration
  magnetConfig = kMagnetDefault;
  if (gSystem->Getenv("CONFIG_MAGNET")) {
    Bool_t valid = kFALSE;
    for (Int_t imag = 0; imag < kNMagnets; imag++)
      if (strcmp(gSystem->Getenv("CONFIG_MAGNET"), MagnetName[imag]) == 0) {
	magnetConfig = imag;
	valid = kTRUE;
	break;
      }
    if (!valid) {
      printf(">>>>> Unknown magnetic field configuration: %s \n", gSystem->Getenv("CONFIG_MAGNET"));
      abort();
    }
  }
	
  // detector configuration
  detectorConfig = kDetectorDefault;
  if (gSystem->Getenv("CONFIG_DETECTOR")) {
    Bool_t valid = kFALSE;
    for (Int_t idet = 0; idet < kNDetectors; idet++)
      if (strcmp(gSystem->Getenv("CONFIG_DETECTOR"), DetectorName[idet]) == 0) {
	detectorConfig = idet;
	valid = kTRUE;
	break;
      }
    if (!valid) {
      printf(">>>>> Unknown detector configuration: %s \n", gSystem->Getenv("CONFIG_DETECTOR"));
      abort();
    }
  }
	
  // generator configuration
  generatorConfig = kGeneratorDefault;
  if (gSystem->Getenv("CONFIG_GENERATOR")) {
    Bool_t valid = kFALSE;
    for (Int_t igen = 0; igen < kNGenerators; igen++)
      if (strcmp(gSystem->Getenv("CONFIG_GENERATOR"), GeneratorName[igen]) == 0) {
	generatorConfig = igen;
	valid = kTRUE;
	break;
      }
    // check PWG tag
    if (!valid) {
      TString str = gSystem->Getenv("CONFIG_GENERATOR");
      if (str.Contains(":")) {
	printf(">>>>> PWG custom MC generator configuration: %s \n", gSystem->Getenv("CONFIG_GENERATOR"));
	generatorConfig = kGeneratorPWG;	  
	valid = kTRUE;
      }
    }
    // unknown generator
    if (!valid) {
      printf(">>>>> Unknown MC generator configuration: %s \n", gSystem->Getenv("CONFIG_GENERATOR"));
      abort();
    }
  }
  
  // process configuration
  processConfig = "";
  if (gSystem->Getenv("CONFIG_PROCESS")) {
    processConfig = gSystem->Getenv("CONFIG_PROCESS");
  }
  
  // system configuration
  systemConfig = "";
  if (gSystem->Getenv("CONFIG_SYSTEM")) {
    systemConfig = gSystem->Getenv("CONFIG_SYSTEM");
  }
  
  // energy configuration
  energyConfig = 0.;
  if (gSystem->Getenv("CONFIG_ENERGY"))
    energyConfig = atoi(gSystem->Getenv("CONFIG_ENERGY"));
  if (energyConfig <= 0) {
    printf(">>>>> Invalid CMS energy: %f \n", energyConfig);
    abort();
  }

  // trigger configuration
  triggerConfig = kTriggerDefault;
  if (gSystem->Getenv("CONFIG_TRIGGER")) {
    Bool_t valid = kFALSE;
    for (Int_t itrg = 0; itrg < kNTriggers; itrg++)
      if (strcmp(gSystem->Getenv("CONFIG_TRIGGER"), TriggerName[itrg]) == 0) {
	triggerConfig = itrg;
	valid = kTRUE;
	break;
      }
    if (!valid) {
      printf(">>>>> Unknown trigger configuration: %s \n", gSystem->Getenv("CONFIG_TRIGGER"));
      abort();
    }
  }

  // PDG value for single particles
  if (gSystem->Getenv("CONFIG_PDG"))
    pdgConfig = atoi(gSystem->Getenv("CONFIG_PDG"));

  // impact parameter configuration
  bminConfig = 0.;
  if (gSystem->Getenv("CONFIG_BMIN"))
    bminConfig = atof(gSystem->Getenv("CONFIG_BMIN"));
  if (bminConfig < 0) {
    printf(">>>>> Invalid min impact parameter: %f \n", bminConfig);
    abort();
  }
  bmaxConfig = 20.;
  if (gSystem->Getenv("CONFIG_BMAX"))
    bmaxConfig = atof(gSystem->Getenv("CONFIG_BMAX"));
  if (bmaxConfig <= bminConfig) {
    printf(">>>>> Invalid max impact parameter: %f \n", bmaxConfig);
    abort();
  }

  // rapidity, phi, pT configuration
  yminConfig = -1.e6;
  if (gSystem->Getenv("CONFIG_YMIN"))
    yminConfig = atof(gSystem->Getenv("CONFIG_YMIN"));
  ymaxConfig = 1.e6;
  if (gSystem->Getenv("CONFIG_YMAX"))
    ymaxConfig = atof(gSystem->Getenv("CONFIG_YMAX"));
  if (ymaxConfig <= yminConfig) {
    printf(">>>>> Invalid max rapidity: %f \n", ymaxConfig);
    abort();
  }
  phiminConfig = 0.;
  if (gSystem->Getenv("CONFIG_PHIMIN"))
    phiminConfig = atof(gSystem->Getenv("CONFIG_PHIMIN"));
  phimaxConfig = 360.;
  if (gSystem->Getenv("CONFIG_PHIMAX"))
    phimaxConfig = atof(gSystem->Getenv("CONFIG_PHIMAX"));
  if (phimaxConfig <= phiminConfig) {
    printf(">>>>> Invalid max phi: %f \n", phimaxConfig);
    abort();
  }
  if (phimaxConfig <= 2.*TMath::Pi()) {
    printf(">>>>> WARNING: phi is expected to be in degree\n");
    printf(">>>>> WARNING: max phi = %f suspected to be in rad. \n", phimaxConfig);
  }
  ptminConfig = 0.;
  if (gSystem->Getenv("CONFIG_PTMIN"))
    ptminConfig = atof(gSystem->Getenv("CONFIG_PTMIN"));
  ptmaxConfig = -1.;
  if (gSystem->Getenv("CONFIG_PTMAX"))
    ptmaxConfig = atof(gSystem->Getenv("CONFIG_PTMAX"));
  if (ptmaxConfig != -1 && ptmaxConfig <= ptminConfig) {
    printf(">>>>> Invalid max pt: %f \n", ptmaxConfig);
    abort();
  }
  
  // pt-hard, pt-trigger and quenching configuration
  pthardminConfig = 0.;
  if (gSystem->Getenv("CONFIG_PTHARDMIN"))
    pthardminConfig = atof(gSystem->Getenv("CONFIG_PTHARDMIN"));
  pthardmaxConfig = -1.;
  if (gSystem->Getenv("CONFIG_PTHARDMAX"))
    pthardmaxConfig = atof(gSystem->Getenv("CONFIG_PTHARDMAX"));
  if (pthardmaxConfig != -1 && pthardmaxConfig <= pthardminConfig) {
    printf(">>>>> Invalid max pt-hard: %f \n", pthardmaxConfig);
    abort();
  }
  pttrigminConfig = 0.;
  if (gSystem->Getenv("CONFIG_PTTRIGMIN"))
    pttrigminConfig = atof(gSystem->Getenv("CONFIG_PTTRIGMIN"));
  pttrigmaxConfig = -1.;
  if (gSystem->Getenv("CONFIG_PTTRIGMAX"))
    pttrigmaxConfig = atof(gSystem->Getenv("CONFIG_PTTRIGMAX"));
  if (pttrigmaxConfig != -1 && pttrigmaxConfig <= pttrigminConfig) {
    printf(">>>>> Invalid max pt-trigger: %f \n", pttrigmaxConfig);
    abort();
  }
  quenchingConfig = 0;
  if (gSystem->Getenv("CONFIG_QUENCHING"))
    quenchingConfig = atoi(gSystem->Getenv("CONFIG_QUENCHING"));
  qhatConfig = 1.7;
  if (gSystem->Getenv("CONFIG_QHAT"))
    qhatConfig = atof(gSystem->Getenv("CONFIG_QHAT"));
  
  // seed configuration
  seedConfig = TDatime().Get();
  if (gSystem->Getenv("CONFIG_SEED"))
    seedConfig = atoi(gSystem->Getenv("CONFIG_SEED"));
  
  // unique ID configuration
  uidConfig = 1;
  if (gSystem->Getenv("CONFIG_UID"))
    uidConfig = atoi(gSystem->Getenv("CONFIG_UID"));
  
  // Geant4 configuration
  isGeant4 = kFALSE;
  if (gSystem->Getenv("CONFIG_GEANT4"))
    isGeant4 = kTRUE;

  // PurifyKine OFF
  purifyKine = kTRUE;
  if (gSystem->Getenv("CONFIG_PURIFYKINEOFF"))
    purifyKine = kFALSE;

  // Fluka configuration
  isFluka = kFALSE;
  if (gSystem->Getenv("CONFIG_FLUKA"))
    isFluka = kTRUE;

}

#if ROOT_VERSION_CODE >= ROOT_VERSION(6,0,0)
  // in root5 the ROOT_VERSION_CODE is defined only in ACLic mode
#else
LoadLibraries()
{

  // get generator string 
  TString genstr = gSystem->Getenv("CONFIG_GENERATOR");
  // check if needs Phojet/Dpmjet
  Bool_t isDpmjet = kFALSE;
  if (genstr.Contains("dpmjet", TString::kIgnoreCase) || genstr.Contains("phojet", TString::kIgnoreCase)) {
    isDpmjet = kTRUE;
    printf(">>>>> Phojet/Dpmjet libraries receipt \n");
  }

  gSystem->Load("liblhapdf");
  gSystem->Load("libEGPythia6");
  // Phojet/DPMjet with PYTHIA 6.2.14
  if (isDpmjet) {
    gSystem->Load("libpythia6");
  }
  else { 
    gSystem->Load("libpythia6_4_25");
  }
  gSystem->Load("libAliPythia6");
  // hack to make Phojet/DPMjet work
  if (isDpmjet) {
    gSystem->Load("libDPMJET");
    gSystem->Load("libTDPMjet");    
  } 
  if (!isFluka)  gSystem->Load("libgeant321");

}
#endif


/*****************************************************************/

void
CreateGAlice() 
{
  //=======================================================================
  //  Create the output file
   
  AliRunLoader* rl=0x0;

  cout<<"Config.C: Creating Run Loader ..."<<endl;
  rl = AliRunLoader::Open("galice.root",
			  AliConfig::GetDefaultEventFolderName(),
			  "recreate");
  if (!rl) {
    gAlice->Fatal("Config.C","Can not instatiate the Run Loader");
    return;
  }
  rl->SetCompressionLevel(2);
  rl->SetNumberOfEventsPerFile(1000);
  gAlice->SetRunLoader(rl);
  // gAlice->SetGeometryFromFile("geometry.root");
  // gAlice->SetGeometryFromCDB();
  rl->CdGAFile();
}

/*****************************************************************/

void
GeneratorOptions()
{
  //======================//
  //    Set MC options    //
  //======================//
  
  //
  gMC->SetProcess("DCAY",1);
  gMC->SetProcess("PAIR",1);
  gMC->SetProcess("COMP",1);
  gMC->SetProcess("PHOT",1);
  gMC->SetProcess("PFIS",0);
  gMC->SetProcess("DRAY",0);
  gMC->SetProcess("ANNI",1);
  gMC->SetProcess("BREM",1);
  gMC->SetProcess("MUNU",1);
  gMC->SetProcess("CKOV",1);
  gMC->SetProcess("HADR",1);
  gMC->SetProcess("LOSS",2);
  gMC->SetProcess("MULS",1);
  gMC->SetProcess("RAYL",1);
  
  Float_t cut = 1.e-3;        // 1MeV cut by default
  Float_t tofmax = 1.e10;
  
  gMC->SetCut("CUTGAM", cut);
  gMC->SetCut("CUTELE", cut);
  gMC->SetCut("CUTNEU", cut);
  gMC->SetCut("CUTHAD", cut);
  gMC->SetCut("CUTMUO", cut);
  gMC->SetCut("BCUTE",  cut); 
  gMC->SetCut("BCUTM",  cut); 
  gMC->SetCut("DCUTE",  cut); 
  gMC->SetCut("DCUTM",  cut); 
  gMC->SetCut("PPCUTM", cut);
  gMC->SetCut("TOFMAX", tofmax); 
  //
  //======================//
  // Set External decayer //
  //======================//
  if(!gMC->GetDecayer()){
    cout<<"Create and set external decayer..."<<endl;
    TVirtualMCDecayer* decayer = new AliDecayerPythia();
    decayer->SetForceDecay(kAll);
    decayer->Init();
    gMC->SetExternalDecayer(decayer);
  }
  else{
    cout<<"External decayer was set already ... nothing to do."<<endl;
  }
  //
}
