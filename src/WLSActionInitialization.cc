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
// $Id: WLSActionInitialization.cc 68058 2013-03-13 14:47:43Z gcosmo $
//
/// \file optical/wls/src/WLSActionInitialization.cc
/// \brief Implementation of the WLSActionInitialization class

#include "WLSActionInitialization.hh"
#include "WLSDetectorConstruction.hh"

#include "WLSPrimaryGeneratorAction.hh"

#include "WLSRunAction.hh"
#include "WLSEventAction.hh"
#include "WLSTrackingAction.hh"
#include "WLSSteppingAction.hh"
#include "WLSStackingAction.hh"
#include "WLSSteppingVerbose.hh"
#include "G4GeneralParticleSource.hh"

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

WLSActionInitialization::WLSActionInitialization(WLSDetectorConstruction* det, G4String name)
 : G4VUserActionInitialization(), fDetector(det), fName(name)
{
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

WLSActionInitialization::~WLSActionInitialization()
{
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void WLSActionInitialization::BuildForMaster() const
{
  SetUserAction(new WLSRunAction(fName));
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void WLSActionInitialization::Build() const
{
  	//SetUserAction(new WLSPrimaryGeneratorAction(fDetector)); // original
	WLSPrimaryGeneratorAction *primaryGenarator = new WLSPrimaryGeneratorAction(fDetector);
  	SetUserAction(primaryGenarator);
	
  	WLSRunAction* runAction = new WLSRunAction(fName);

	WLSStackingAction* stacking = new WLSStackingAction();
  	//WLSEventAction* eventAction = new WLSEventAction(runAction); // original
  	WLSEventAction* eventAction = new WLSEventAction(runAction,primaryGenarator,stacking);

  	SetUserAction(runAction);
  	SetUserAction(eventAction);

  	SetUserAction(new WLSTrackingAction()); 
  	//SetUserAction(new WLSSteppingAction(fDetector)); // original
  	SetUserAction(new WLSSteppingAction(fDetector,eventAction));
  	//SetUserAction(new WLSStackingAction()); // original
   SetUserAction(stacking);
}  

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

G4VSteppingVerbose* WLSActionInitialization::InitializeSteppingVerbose() const
{
  return new WLSSteppingVerbose();
}
