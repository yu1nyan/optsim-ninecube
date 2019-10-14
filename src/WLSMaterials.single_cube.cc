//
// ********************************************************************
// * License and Disclaimer                                           *
// *                                                                  *
// * The  Geant4 software  is  copyright of the Copyright Holders  of *
// * the Geant4 Collaboration.  It is provided  under  the terms  and *
// * conditions of the Geant4 Software License,  included in the file *
// * LICENSE and available at  http://cern.ch/geant4/license .  These *
// * include a list of copyright holders.                             *
// *                                                                  *
// * Neither the authors of this software system, nor their employing *
// * institutes,nor the agencies providing financial support for this *
// * work  make  any representation or  warranty, express or implied, *
// * regarding  this  software system or assume any liability for its *
// * use.  Please see the license in the file  LICENSE  and URL above *
// * for the full disclaimer and the limitation of liability.         *
// *                                                                  *
// * This  code  implementation is the result of  the  scientific and *
// * technical work of the GEANT4 collaboration.                      *
// * By using,  copying,  modifying or  distributing the software (or *
// * any work based  on the software)  you  agree  to acknowledge its *
// * use  in  resulting  scientific  publications,  and indicate your *
// * acceptance of all terms of the Geant4 Software license.          *
// ********************************************************************
//
// $Id: WLSMaterials.cc 82854 2014-07-14 09:08:25Z gcosmo $
//
/// \file optical/wls/src/WLSMaterials.cc
/// \brief Implementation of the WLSMaterials class
//
//
#include "WLSMaterials.hh"

#include "G4SystemOfUnits.hh"

WLSMaterials* WLSMaterials::fInstance = 0;

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

WLSMaterials::WLSMaterials()
{
  fNistMan = G4NistManager::Instance();
  fNistMan->SetVerbose(2);
  CreateMaterials();
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

WLSMaterials::~WLSMaterials()
{
  delete    fPMMA;
  delete    fPethylene;
  delete    fFPethylene;
  delete    fPolystyrene;
  delete    fSilicone;
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

WLSMaterials* WLSMaterials::GetInstance()
{
  	if (fInstance==0) {
      fInstance = new WLSMaterials();
  	}
  	return fInstance;
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

G4Material* WLSMaterials::GetMaterial(const G4String material)
{
  	G4Material* mat =  fNistMan->FindOrBuildMaterial(material);

  	if (!mat) mat = G4Material::GetMaterial(material);
  	if (!mat) {
     	std::ostringstream o;
     	o << "Material " << material << " not found!";
     	G4Exception("WLSMaterials::GetMaterial","",
                 FatalException,o.str().c_str());
  	}
  	return mat;
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void WLSMaterials::CreateMaterials()
{
  	G4double density;
  	G4int ncomponents;
  	G4double fractionmass;
  	std::vector<G4int> natoms;
  	std::vector<G4double> fractionMass;
  	std::vector<G4String> elements;

  	// Materials Definitions

  	// Vacuum
  	fNistMan->FindOrBuildMaterial("G4_Galactic");

  	// Air
  	fAir = fNistMan->FindOrBuildMaterial("G4_AIR");

  	// ----- WLSfiber PMMA
  	elements.push_back("C"); natoms.push_back(5);
  	elements.push_back("H"); natoms.push_back(8);
  	elements.push_back("O"); natoms.push_back(2);
  	density = 1.190*g/cm3;

  	fPMMA = fNistMan-> ConstructNewMaterial("PMMA", elements, natoms, density);
  	elements.clear();
  	natoms.clear();

  	// ----- Cladding (polyethylene)
  	elements.push_back("C"); natoms.push_back(2);
  	elements.push_back("H"); natoms.push_back(4);
  	density = 1.050*g/cm3;

  	fPethylene = fNistMan-> ConstructNewMaterial("Pethylene", elements, natoms, density);
  	elements.clear();
  	natoms.clear();

  	// ----- Double Cladding (fluorinated polyethylene)
  	elements.push_back("C"); natoms.push_back(2);
  	elements.push_back("H"); natoms.push_back(4);
  	density = 1.430*g/cm3;

  	fFPethylene = fNistMan-> ConstructNewMaterial("FPethylene", elements, natoms, density);
  	elements.clear();
  	natoms.clear();

  	// Polystyrene
  	elements.push_back("C");     natoms.push_back(8);
  	elements.push_back("H");     natoms.push_back(8);
  	density = 1.050*g/cm3;

  	fPolystyrene = fNistMan->ConstructNewMaterial("Polystyrene", elements, natoms, density);
  	elements.clear();
  	natoms.clear();

  	// Silicone (Template for Optical Grease)
  	elements.push_back("C");     natoms.push_back(2);
  	elements.push_back("H");     natoms.push_back(6);
  	density = 1.060*g/cm3;

  	fSilicone = fNistMan-> ConstructNewMaterial("Silicone", elements, natoms, density);
  	elements.clear();
  	natoms.clear();

  	//--------------------------------------------------
  	// Aluminium
  	//--------------------------------------------------
  	fNistMan->FindOrBuildMaterial("G4_Al");

  	//--------------------------------------------------
  	// TiO2
  	//--------------------------------------------------
  	elements.push_back("Ti");     natoms.push_back(1);
  	elements.push_back("O");      natoms.push_back(2);
  	density     = 4.26*g/cm3;
  	G4Material* TiO2 = fNistMan-> ConstructNewMaterial("TiO2", elements, natoms, density);

  	elements.clear();
  	natoms.clear();

  	//--------------------------------------------------
  	// Scintillator Coating - 15% TiO2 and 85% polystyrene by weight.
  	//--------------------------------------------------
  	density = 1.52*g/cm3;

  	fCoating = new G4Material("Coating", density, ncomponents=2);
  	fCoating->AddMaterial(TiO2,         fractionmass = 15*perCent);
  	fCoating->AddMaterial(fPolystyrene, fractionmass = 85*perCent);

  	//
  	// ------------ Generate & Add Material Properties Table ------------
  	//
	// http://wiki.opengatecollaboration.org/index.php/Users_Guide:Generating_and_tracking_optical_photons
	// 1Å = 0.1 nm
	// 2.00eV ~ 6200Å ~ 620 nm
   // 3.47eV ~ 3573Å ~ 357 nm
   G4double photonEnergy[] = {
	2.00*eV,2.03*eV,2.06*eV,2.09*eV,2.12*eV,2.15*eV,2.18*eV,2.21*eV,2.24*eV,2.27*eV, // 620nm -- 550 nm
   2.30*eV,2.33*eV,2.36*eV,2.39*eV,2.42*eV,2.45*eV,2.48*eV,2.51*eV,2.54*eV,2.57*eV, // 540nm -- 480 nm 
   2.60*eV,2.63*eV,2.66*eV,2.69*eV,2.72*eV,2.75*eV,2.78*eV,2.81*eV,2.84*eV,2.87*eV, // 485nm -- 430 nm
   2.90*eV,2.93*eV,2.96*eV,2.99*eV,3.02*eV,3.05*eV,3.08*eV,3.11*eV,3.14*eV,3.17*eV, // 428nm -- 390 nm
   3.20*eV,3.23*eV,3.26*eV,3.29*eV,3.32*eV,3.35*eV,3.38*eV,3.41*eV,3.44*eV,3.47*eV};// 395nm -- 357 nm

  	const G4int nEntries = sizeof(photonEnergy)/sizeof(G4double);

  	//--------------------------------------------------
  	// Air
  	//--------------------------------------------------
  	G4double refractiveIndex[] = { 
	1.00, 1.00, 1.00, 1.00, 1.00, 1.00, 1.00, 1.00, 1.00, 1.00,
   1.00, 1.00, 1.00, 1.00, 1.00, 1.00, 1.00, 1.00, 1.00, 1.00,
   1.00, 1.00, 1.00, 1.00, 1.00, 1.00, 1.00, 1.00, 1.00, 1.00,
   1.00, 1.00, 1.00, 1.00, 1.00, 1.00, 1.00, 1.00, 1.00, 1.00,
   1.00, 1.00, 1.00, 1.00, 1.00, 1.00, 1.00, 1.00, 1.00, 1.00};
  	assert(sizeof(refractiveIndex) == sizeof(photonEnergy));

  	G4MaterialPropertiesTable* mpt = new G4MaterialPropertiesTable();
  	mpt->AddProperty("RINDEX", photonEnergy, refractiveIndex, nEntries);
  	fAir->SetMaterialPropertiesTable(mpt);

   // 
	// http://kuraraypsf.jp/pdf/all.pdf
	// https://indico.cern.ch/event/143675/contributions/164201/
  	// ----- Pethylene: Core information
  	G4double refractiveIndexWLSfiber[] = {
   1.59, 1.59, 1.59, 1.59, 1.59, 1.59, 1.59, 1.59, 1.59, 1.59, 
	1.59, 1.59, 1.59, 1.59, 1.59, 1.59, 1.59, 1.59, 1.59, 1.59,
   1.59, 1.59, 1.59, 1.59, 1.59, 1.59, 1.59, 1.59, 1.59, 1.59, 
	1.59, 1.59, 1.59, 1.59, 1.59, 1.59, 1.59, 1.59, 1.59, 1.59,
   1.59, 1.59, 1.59, 1.59, 1.59, 1.59, 1.59, 1.59, 1.59, 1.59};
  	assert(sizeof(refractiveIndexWLSfiber) == sizeof(photonEnergy));
/* //  default
  	G4double absWLSfiber[] = {
   5.40*m,5.40*m,5.40*m,5.40*m,5.40*m,5.40*m,5.40*m,5.40*m,5.40*m,5.40*m, 
   5.40*m,5.40*m,5.40*m,5.40*m,5.40*m,5.40*m,5.40*m,5.40*m,5.40*m,5.40*m,
   5.40*m,5.40*m,5.40*m,5.40*m,5.40*m,5.40*m,5.40*m,5.40*m,5.40*m,1.10*m, 
   1.10*m,1.10*m,1.10*m,1.10*m,1.10*m,1.10*m,1.0*mm,1.0*mm,1.0*mm,1.0*mm,
   1.0*mm,1.0*mm,1.0*mm,1.0*mm,1.0*mm,1.0*mm,1.0*mm,1.0*mm,1.0*mm,1.0*mm};
*/
   G4double absWLSfiber[] = {
   4.50*m,4.50*m,4.50*m,4.50*m,4.50*m,4.50*m,4.50*m,4.50*m,4.50*m,4.50*m, 
   4.50*m,4.50*m,4.50*m,4.50*m,4.50*m,4.50*m,4.50*m,4.50*m,4.50*m,4.50*m,
   4.50*m,4.50*m,4.50*m,4.50*m,4.50*m,4.50*m,4.50*m,4.50*m,4.50*m,2.80*m, // 485nm -- 430nm
   2.40*m,2.10*m,1.90*m,1.70*m,1.40*m,1.20*m,0.80*m,0.50*m,0.30*m,0.10*m,
   0.10*m,0.10*m,0.05*m,0.05*m,0.01*m,0.01*m,0.01*m,0.01*m,0.01*m,0.01*m};// 395nm -- 
  	assert(sizeof(absWLSfiber) == sizeof(photonEnergy));
/*
  	G4double emissionFib[] = {
   0.05, 0.10, 0.30, 0.50, 0.75, 1.00, 1.50, 1.85, 2.30, 2.75, // 620nm -- 550 nm 
   3.25, 3.80, 4.50, 5.20, 6.00, 7.00, 8.50, 9.50, 11.1, 12.4, // 540   -- 480 
   12.9, 13.0, 12.8, 12.3, 11.1, 11.0, 12.0, 11.0, 17.0, 16.9, // 485nm -- 430nm
   15.0, 9.00, 2.50, 1.00, 0.05, 0.00, 0.00, 0.00, 0.00, 0.00, // 428nm --
   0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00};// 395nm -- 
*/
   G4double emissionFib[] = {
   0.00, 0.00, 0.00, 0.02, 0.05, 0.09, 0.11, 0.14, 0.17, 0.20, // 620nm -- 550 nm 
   0.30, 0.35, 0.40, 0.45, 0.50, 0.60, 0.67, 0.80, 0.75, 0.75, // 540   -- 480 
   0.80, 0.85, 1.00, 0.92, 0.75, 0.62, 0.45, 0.20, 0.00, 0.00, // 485nm -- 430nm
   0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, // 428nm --
   0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00};// 395nm -- 
   assert(sizeof(emissionFib) == sizeof(photonEnergy));

  	// Add entries into properties table
  	G4MaterialPropertiesTable* mptWLSfiber = new G4MaterialPropertiesTable();
  	mptWLSfiber->AddProperty("RINDEX",photonEnergy,refractiveIndexWLSfiber,nEntries);
  	//mptWLSfiber->AddProperty("ABSLENGTH",photonEnergy,absWLSfiber,nEntries);
  	mptWLSfiber->AddProperty("WLSABSLENGTH",photonEnergy,absWLSfiber,nEntries);
  	mptWLSfiber->AddProperty("WLSCOMPONENT",photonEnergy,emissionFib,nEntries);
  	mptWLSfiber->AddConstProperty("WLSTIMECONSTANT", 1*ns);



  	// ----- PMMA (inner claddig) : http://kuraraypsf.jp/pdf/all.pdf
  	G4double refractiveIndexClad1[] = { 
	1.49, 1.49, 1.49, 1.49, 1.49, 1.49, 1.49, 1.49, 1.49, 1.49, 
	1.49, 1.49, 1.49, 1.49, 1.49, 1.49, 1.49, 1.49, 1.49, 1.49,
   1.49, 1.49, 1.49, 1.49, 1.49, 1.49, 1.49, 1.49, 1.49, 1.49, 
	1.49, 1.49, 1.49, 1.49, 1.49, 1.49, 1.49, 1.49, 1.49, 1.49,
   1.49, 1.49, 1.49, 1.49, 1.49, 1.49, 1.49, 1.49, 1.49, 1.49};
  	assert(sizeof(refractiveIndexClad1) == sizeof(photonEnergy));
/*
  	G4double absClad[] = {
	20.0*m,20.0*m,20.0*m,20.0*m,20.0*m,20.0*m,20.0*m,20.0*m,20.0*m,20.0*m, 
	20.0*m,20.0*m,20.0*m,20.0*m,20.0*m,20.0*m,20.0*m,20.0*m,20.0*m,20.0*m,
   20.0*m,20.0*m,20.0*m,20.0*m,20.0*m,20.0*m,20.0*m,20.0*m,20.0*m,20.0*m, 
	20.0*m,20.0*m,20.0*m,20.0*m,20.0*m,20.0*m,20.0*m,20.0*m,20.0*m,20.0*m,
   20.0*m,20.0*m,20.0*m,20.0*m,20.0*m,20.0*m,20.0*m,20.0*m,20.0*m,20.0*m};
*/
   G4double absClad[] = {
   4.50*m,4.50*m,4.50*m,4.50*m,4.50*m,4.50*m,4.50*m,4.50*m,4.50*m,4.50*m,
   4.50*m,4.50*m,4.50*m,4.50*m,4.50*m,4.50*m,4.50*m,4.50*m,4.50*m,4.50*m,
   4.50*m,4.50*m,4.50*m,4.50*m,4.50*m,4.50*m,4.50*m,4.50*m,4.50*m,4.50*m, // 485nm -- 430nm
   4.50*m,4.50*m,4.50*m,4.50*m,4.50*m,4.50*m,4.50*m,4.50*m,4.50*m,4.50*m, // 428nm
   4.50*m,4.50*m,4.50*m,4.50*m,4.50*m,4.50*m,4.50*m,4.40*m,4.20*m,4.00*m};// 395nm -- 357nm
   assert(sizeof(absClad) == sizeof(photonEnergy));
  	// Add entries into properties table
  	G4MaterialPropertiesTable* mptClad1 = new G4MaterialPropertiesTable();
  	mptClad1->AddProperty("RINDEX",photonEnergy,refractiveIndexClad1,nEntries);
  	mptClad1->AddProperty("ABSLENGTH",photonEnergy,absClad,nEntries);

	
  	// ----- Fluorinated Polyethylene (outer cladding(FP)) : http://kuraraypsf.jp/pdf/all.pdf
   G4double refractiveIndexClad2[] = { 
	1.42, 1.42, 1.42, 1.42, 1.42, 1.42, 1.42, 1.42, 1.42, 1.42, 
	1.42, 1.42, 1.42, 1.42, 1.42, 1.42, 1.42, 1.42, 1.42, 1.42,
   1.42, 1.42, 1.42, 1.42, 1.42, 1.42, 1.42, 1.42, 1.42, 1.42, 
	1.42, 1.42, 1.42, 1.42, 1.42, 1.42, 1.42, 1.42, 1.42, 1.42,
   1.42, 1.42, 1.42, 1.42, 1.42, 1.42, 1.42, 1.42, 1.42, 1.42};
   assert(sizeof(refractiveIndexClad2) == sizeof(photonEnergy));
  	// Add entries into properties table
  	G4MaterialPropertiesTable* mptClad2 = new G4MaterialPropertiesTable();
  	mptClad2->AddProperty("RINDEX",photonEnergy,refractiveIndexClad2,nEntries);
  	mptClad2->AddProperty("ABSLENGTH",photonEnergy,absClad,nEntries);

		
	// ----- set table
	#if 0 // original 
   fPMMA      ->SetMaterialPropertiesTable(mptWLSfiber);
   fPethylene ->SetMaterialPropertiesTable(mptClad1);
   fFPethylene->SetMaterialPropertiesTable(mptClad2);
	#else // CAUTION: this is coreect, check used materials
   fPethylene ->SetMaterialPropertiesTable(mptWLSfiber); // fPethylene is "core"
   fPMMA      ->SetMaterialPropertiesTable(mptClad1);    // fPMMA inner cladding
   fFPethylene->SetMaterialPropertiesTable(mptClad2);    // fFPethylene outer cladding
	#endif


  //--------------------------------------------------
  // Silicone
  //--------------------------------------------------
   G4double refractiveIndexSilicone[] =
   { 1.46, 1.46, 1.46, 1.46, 1.46, 1.46, 1.46, 1.46, 1.46, 1.46,
     1.46, 1.46, 1.46, 1.46, 1.46, 1.46, 1.46, 1.46, 1.46, 1.46,
     1.46, 1.46, 1.46, 1.46, 1.46, 1.46, 1.46, 1.46, 1.46, 1.46,
     1.46, 1.46, 1.46, 1.46, 1.46, 1.46, 1.46, 1.46, 1.46, 1.46,
     1.46, 1.46, 1.46, 1.46, 1.46, 1.46, 1.46, 1.46, 1.46, 1.46};

   assert(sizeof(refractiveIndexSilicone) == sizeof(photonEnergy));

  // Add entries into properties table
  G4MaterialPropertiesTable* mptSilicone = new G4MaterialPropertiesTable();
  mptSilicone->AddProperty("RINDEX",photonEnergy,refractiveIndexSilicone,nEntries);
  mptSilicone->AddProperty("ABSLENGTH",photonEnergy,absClad,nEntries);

  fSilicone->SetMaterialPropertiesTable(mptSilicone);


  	// ----- Polystyrene / scintillator
  	G4double refractiveIndexPS[] = {
   1.58, 1.58, 1.58, 1.58, 1.58, 1.58, 1.58, 1.58, 1.58, 1.58, 
   1.58, 1.58, 1.58, 1.58, 1.58, 1.58, 1.58, 1.58, 1.58, 1.58,
   1.58, 1.58, 1.58, 1.58, 1.58, 1.58, 1.58, 1.58, 1.58, 1.58, 
   1.58, 1.58, 1.58, 1.58, 1.58, 1.58, 1.58, 1.58, 1.58, 1.58,
   1.58, 1.58, 1.58, 1.58, 1.58, 1.58, 1.58, 1.58, 1.58, 1.58};
  	assert(sizeof(refractiveIndexPS) == sizeof(photonEnergy));
#if 0 // default
 	G4double absPS[] = {
   2.*cm,2.*cm,2.*cm,2.*cm,2.*cm,2.*cm,2.*cm,2.*cm,2.*cm,2.*cm,
   2.*cm,2.*cm,2.*cm,2.*cm,2.*cm,2.*cm,2.*cm,2.*cm,2.*cm,2.*cm,
   2.*cm,2.*cm,2.*cm,2.*cm,2.*cm,2.*cm,2.*cm,2.*cm,2.*cm,2.*cm,
   2.*cm,2.*cm,2.*cm,2.*cm,2.*cm,2.*cm,2.*cm,2.*cm,2.*cm,2.*cm,
   2.*cm,2.*cm,2.*cm,2.*cm,2.*cm,2.*cm,2.*cm,2.*cm,2.*cm,2.*cm};
#endif
#if 1
   G4double absPS[] = {
   4.50*m,4.50*m,4.50*m,4.50*m,4.50*m,4.50*m,4.50*m,4.50*m,4.50*m,4.50*m,
   4.50*m,4.50*m,4.50*m,4.50*m,4.50*m,4.50*m,4.50*m,4.50*m,4.50*m,4.50*m,
   4.00*m,4.00*m,3.50*m,3.50*m,3.50*m,3.30*m,3.10*m,2.90*m,2.70*m,2.50*m, // 485nm -- 430nm
   2.20*m,2.00*m,1.80*m,1.70*m,1.60*m,1.50*m,1.40*m,1.30*m,1.10*m,1.00*m, // 428nm
   0.90*m,0.80*m,0.70*m,0.60*m,0.50*m,0.40*m,0.30*m,0.20*m,0.10*m,0.05*m};
#endif
  	assert(sizeof(absPS) == sizeof(photonEnergy));
#if 0 // default
  	G4double scintilFast[] = {
   0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, // 620nm -- 550 nm
	0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, // 540   -- 480 
   0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, // 485nm -- 430nm
	1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, // 428nm -- // default
	//0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, // 428nm -- // test -> large
   1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0};// 395nm -- 
   //0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0};// 395nm -- // test -> small
#endif
#if 1
   G4double scintilFast[] = {
   0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, // 620nm -- 550 nm
   0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, // 540   -- 480 
   0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.05, 0.10, 0.10, 0.15, // 485nm -- 430nm
   0.20, 0.25, 0.30, 0.35, 0.40, 0.50, 0.60, 0.70, 0.80, 0.90, // 428nm --
   0.90, 1.00, 0.90, 0.70, 0.50, 0.30, 0.20, 0.10, 0.00, 0.00};// 395nm -- 
#endif
   assert(sizeof(scintilFast) == sizeof(photonEnergy));

  	// Add entries into properties table
  	G4MaterialPropertiesTable* mptPolystyrene = new G4MaterialPropertiesTable();
  	mptPolystyrene->AddProperty("RINDEX",photonEnergy,refractiveIndexPS,nEntries);
  	mptPolystyrene->AddProperty("ABSLENGTH",photonEnergy,absPS,nEntries);
  	mptPolystyrene->AddProperty("FASTCOMPONENT",photonEnergy, scintilFast,nEntries);

   mptPolystyrene->AddConstProperty("EFFICIENCY",1);
  	mptPolystyrene->AddConstProperty("SCINTILLATIONYIELD",20./keV);
  	//mptPolystyrene->AddConstProperty("SCINTILLATIONYIELD",50./keV);
  	mptPolystyrene->AddConstProperty("RESOLUTIONSCALE",1.0);
  	mptPolystyrene->AddConstProperty("FASTTIMECONSTANT",1.*ns);
  	mptPolystyrene->AddConstProperty("SLOWTIMECONSTANT",2.*ns);
  	fPolystyrene->SetMaterialPropertiesTable(mptPolystyrene);

  	// Set the Birks Constant for the Polystyrene scintillator
  	fPolystyrene->GetIonisation()->SetBirksConstant(0.126*mm/MeV);
}

