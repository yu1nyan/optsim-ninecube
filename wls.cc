﻿//
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
// $Id: wls.cc 78066 2013-12-03 11:08:36Z gcosmo $
//
/// \file optical/wls/wls.cc
/// \brief Main program of the optical/wls example
//
// ....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

#ifndef WIN32
    #include <unistd.h>
#endif

#include <regex>
#include <iostream>
using namespace std;

#ifdef G4MULTITHREADED
    #include "G4MTRunManager.hh"
#else
    #include "G4RunManager.hh"
#endif

#include "G4UImanager.hh"

#include "Randomize.hh"

#include "WLSPhysicsList.hh"
#include "WLSDetectorConstruction.hh"

#include "WLSActionInitialization.hh"

#ifdef G4VIS_USE
    #include "G4VisExecutive.hh"
#endif

#ifdef G4UI_USE
    #include "G4UIExecutive.hh"
#endif

// argc holds the number of arguments (including the name) on the command line
// -> it is ONE when only the name is  given !!!
// argv[0] is always the name of the program
// argv[1] points to the first argument, and so on
// please
// ....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

int main(int argc, char** argv)
{
    #ifdef G4MULTITHREADED
        G4MTRunManager* runManager = new G4MTRunManager;
    #else



        G4String inpName;
        if (argc > 2)
            inpName = argv[2];
        else
            inpName = "cube";

        G4int seed = 123;
        G4double fiber_length = 200;  // cm
        G4double gap_length = fiber_length/2;
        G4double mirror_reflectivity = 0;
        G4double cube_reflectivity = 0.97;

        if (argc  > 3)
        {
            regex search_regex(R"(^(seed|fiber_length|gap_length|mirror_reflectivity|cube_reflectivity)\s*=\s*(([1-9]\d*|0)(\.\d+)?$))");
            smatch matched_words;
            for(int i=3; i<argc; i++)
            {
                string arg = argv[i];
                if(regex_match(arg, matched_words, search_regex))
                {
                    cout << matched_words[1] << ": " << matched_words[2] << endl;
                    if(matched_words[1] == "seed")
                    {
                        seed = stoi(matched_words[2]);
                    }
                    else if(matched_words[1] == "fiber_length")
                    {
                        fiber_length = stod(matched_words[2]);
                    }
                    else if(matched_words[1] == "gap_length")
                    {
                        gap_length = stod(matched_words[2]);
                    }
                    else if(matched_words[1] == "mirror_reflectivity")
                    {
                        mirror_reflectivity = stod(matched_words[2]);
                    }
                    else if(matched_words[1] == "cube_reflectivity")
                    {
                        cube_reflectivity = stod(matched_words[2]);
                    }
                    else
                    {
                        cerr << "Parameter setting error!" << endl;
                        return 1;
                    }
                }
            }
        }

        if (argv[1] != NULL)
        {
            G4cout << "input macro name is " << argv[1] << G4endl;
        }

        G4cout << "input rootfile name: " << inpName << G4endl;

        G4cout << "seed: " << seed << G4endl;
        G4cout << "fiber_length: " << fiber_length << G4endl;
        G4cout << "gap_length: " << gap_length <<G4endl;
        G4cout << "mirror_reflectivity: " << mirror_reflectivity <<G4endl;
        G4cout << "cube_reflectivity: " << cube_reflectivity << G4endl;

        // Choose the Random engine and set the seed

        G4Random::setTheEngine(new CLHEP::RanecuEngine);
        G4Random::setTheSeed(seed);

        G4RunManager* runManager = new G4RunManager;
    #endif

    G4String physName = "QGSP_BERT_HP";

    #ifndef WIN32
        G4int c = 0;
        while ((c = getopt(argc, argv, "p")) != -1)
        {
            switch (c)
            {
                case 'p':
                    physName = optarg;
                    G4cout << "Physics List used is " <<  physName << G4endl;
                    break;
                case ':': /* -p without operand */
                    fprintf(stderr, "Option -%c requires an operand\n", optopt);
                    break;
                case '?':
                    fprintf(stderr, "Unrecognised option: -%c\n", optopt);
            }
        }
    #endif

    // Set mandatory initialization classes
    //
    // Detector construction
    WLSDetectorConstruction* detector = new WLSDetectorConstruction(fiber_length, gap_length, mirror_reflectivity, cube_reflectivity);
    runManager->SetUserInitialization(detector);
    // Physics list
    runManager->SetUserInitialization(new WLSPhysicsList(physName));
    // User action initialization
    runManager->SetUserInitialization(new WLSActionInitialization(detector, inpName));

    #ifdef G4VIS_USE
        // Initialize visualization
        //
        G4VisManager* visManager = new G4VisExecutive;
        // G4VisExecutive can take a verbosity argument - see /vis/verbose guidance.
        // G4VisManager* visManager = new G4VisExecutive("Quiet");
        visManager->Initialize();
    #endif

    // Get the pointer to the User Interface manager

    G4UImanager* UImanager = G4UImanager::GetUIpointer();

    #ifndef WIN32
        G4int optmax = argc;
        if (argc > 2)
        {
            optmax = optmax - 1;
        }

        if (optind < optmax)
        {
            G4String command = "/control/execute ";
            for (; optind < optmax; optind++)
            {
                G4String macroFilename = argv[optind];
                UImanager->ApplyCommand(command + macroFilename);
            }
        }
    #else // Simple UI for Windows runs, no possibility of additional arguments
        if (argc != 1)
        {
            G4String command = "/control/execute ";
            G4String fileName = argv[1];
            UImanager->ApplyCommand(command + fileName);
        }
    #endif
    else
    {
        // Define (G)UI terminal for interactive mode
        #ifdef G4UI_USE
            G4UIExecutive* ui = new G4UIExecutive(argc, argv);
            #ifdef G4VIS_USE
                UImanager->ApplyCommand("/control/execute init.in");
                // UImanager->ApplyCommand("/control/execute vis.mac");
            #endif
            if (ui->IsGUI())
                UImanager->ApplyCommand("/control/execute gui.mac");
            ui->SessionStart();
            delete ui;
        #endif
    }

    // job termination

    #ifdef G4VIS_USE
        delete visManager;
    #endif
    delete runManager;

    return 0;
}

// ....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......
