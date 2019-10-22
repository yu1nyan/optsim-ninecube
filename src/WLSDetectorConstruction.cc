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
// $Id: WLSDetectorConstruction.cc 84718 2014-10-20 07:40:45Z gcosmo $
//
/// \file optical/wls/src/WLSDetectorConstruction.cc
/// \brief Implementation of the WLSDetectorConstruction class
//
//
#include "G4ios.hh"
#include "globals.hh"

#include "G4Box.hh"
#include "G4Tubs.hh"
#include "G4VisAttributes.hh"
#include "G4EllipticalTube.hh"

#include "G4LogicalVolume.hh"
#include "G4PVPlacement.hh"

#include "G4OpBoundaryProcess.hh"
#include "G4LogicalSkinSurface.hh"
#include "G4LogicalBorderSurface.hh"

#include "G4Material.hh"
#include "G4NistManager.hh"

#include "G4GeometryManager.hh"
#include "G4SolidStore.hh"
#include "G4LogicalVolumeStore.hh"
#include "G4PhysicalVolumeStore.hh"

#include "G4SubtractionSolid.hh"

#include "G4RunManager.hh"

#include "WLSDetectorConstruction.hh"
#include "WLSDetectorMessenger.hh"
#include "WLSMaterials.hh"
#include "WLSPhotonDetSD.hh"

#include "G4UserLimits.hh"
#include "G4PhysicalConstants.hh"
#include "G4SystemOfUnits.hh"

#include "parameter.hh"

// ....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

// length: Length of WLS fiber (cm)
// gaplength:
// mirror_reflectivity:
// cube_reflectivity: reflectivity of cube coating
WLSDetectorConstruction::WLSDetectorConstruction(double length, double gaplength, double mirror_reflectivity, double cube_reflectivity)
// : fMaterials(NULL), fLogiHole(NULL), fLogiWorld(NULL),
    : fMaterials(NULL), fLogiFiberHoleX(NULL), fLogiFiberHoleY(NULL), fLogiFiberHoleZ(NULL),
    fLogiWorld(NULL), fPhysWorld(NULL),
    // fPhysWorld(NULL), fPhysHole(NULL)
    fPhysFiberHoleX(NULL), fPhysFiberHoleY(NULL), fPhysFiberHoleZ(NULL)
{
    fDetectorMessenger = new WLSDetectorMessenger(this);

    fMirrorToggle = 0; // true;
    fMirrorPolish = 1.;
    fMPPCPolish = 1.;
    fMirrorReflectivity = mirror_reflectivity;

    // fWLSfiberZ = 100 * cm;
    fWLSfiberZ = length / 2 * cm;
    // fWLSfiberRY  = 0.5*mm;
    fWLSfiberRY = 0.50 * mm - 0.04 * mm; // phi?
    fWLSfiberOrigin = 0.0;

    // fWLSfiberl = -70 * cm;
    fWLSfiberl = (length / 2 - gaplength) * cm;

    fMPPCShape = "Square";
    fMPPCHalfL = fWLSfiberRY;
    fMPPCDist = 0.00 * mm;
    fMPPCTheta = 0.0 * deg;
    fMPPCZ = 0.05 * mm;

    fClrfiberZ = fMPPCZ + 10. * nm;
    fMirrorZ = 0.1 * mm;

    fBarLength = 1. * cm;
    fBarBase = 1. * cm;
    // fCoatingThickness = 0.3 * mm;
    fCoatingThickness = 0.1 * mm;
    fCoatingRadius = 0.01 * mm;
    fCubeReflectivity = cube_reflectivity;

    fHolePos = 2.1 * mm;
    fHoleRadius = 0.70 * mm;
    double penetration = 0.05 * mm; // value for the time being
    // fHoleLength  = 1*cm;//fBarLength;
    fHoleLength = fBarBase + fCoatingThickness * 2 + penetration * 2;
}

// ....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

WLSDetectorConstruction::~WLSDetectorConstruction()
{
    if (fDetectorMessenger)
        delete fDetectorMessenger;
    if (fMaterials)
        delete fMaterials;
}

// ....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

G4VPhysicalVolume*WLSDetectorConstruction::Construct()
{
    if (fPhysWorld)
    {
        G4GeometryManager::GetInstance()->OpenGeometry();
        G4PhysicalVolumeStore::GetInstance()->Clean();
        G4LogicalVolumeStore::GetInstance()->Clean();
        G4SolidStore::GetInstance()->Clean();
        G4LogicalSkinSurface::CleanSurfaceTable();
        G4LogicalBorderSurface::CleanSurfaceTable();
    }

    fMaterials = WLSMaterials::GetInstance();
    UpdateGeometryParameters();
    return ConstructDetector();
}

// ....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

G4VPhysicalVolume*WLSDetectorConstruction::ConstructDetector()
{
    /*
        http://www-jlc.kek.jp/~hoshina/geant4/Geant4Lecture2003/2-2b.html
        1 検出器の形状（ジオメトリ）……………………G4VSolidクラスファミリー: G4Box
        2 検出器の材質、可視属性、有感領域属性………G4LogicalVolumeクラス
        3 検出器の配置情報…………………………………G4VPhysicalVolumeクラスファミリー,G4PVPlacement

        G4PVPlacement　　…通常の置き方。娘LogicalVolumeを親LogicalVolumeの座標系の中の指定した位置に置く。
        G4PVReplica　　　…同じLogicalVolumeをある簡単な並べ方でたくさん置く場合に使う。
        G4PVParametrized…１つのLogicalVolumeのパラメータ（大きさ、材質、位置、回転など）を変化させつつ、たくさん置く方法。
    */
    // ----- World
    G4cerr << "\nWorld: fWorldSizeX=" << fWorldSizeX << " fWorldSizeY=" << fWorldSizeY << " fWorldSizeZ=" << fWorldSizeZ << G4endl;

    G4VSolid* solidWorld(0);
    solidWorld = new G4Box("World", fWorldSizeX, fWorldSizeY, fWorldSizeZ);
    fLogiWorld = new G4LogicalVolume(solidWorld, FindMaterial("G4_AIR"), "World");
    fPhysWorld = new G4PVPlacement(0, G4ThreeVector(), fLogiWorld, "World", 0, false, 0);

    // ----- Extrusion
    double thickness = GetBarBase() / 2 + GetCoatingThickness(); // + GetCoatingRadius();
    // G4cerr << "GetBarBase()=" << GetBarBase() << " GetBarLength()=" << GetBarLength() << G4endl;
    G4cerr << "\nExtrusion: thickness=" << thickness << G4endl;
    G4cerr << "\nScintillator cube=" << GetBarBase() / 2 << G4endl;
    G4double gap = 0.01 * mm;
    G4double sci_pitch = GetBarBase() + 2 * GetCoatingThickness() + gap;

    G4VSolid* solidExtrusion(0);
    G4VSolid* solidSciCube(0);

    solidExtrusion = new G4Box("Extrusion", thickness, thickness, thickness);
    solidSciCube = new G4Box("SciCube", GetBarBase() / 2, GetBarBase() / 2, GetBarBase() / 2);

    G4double tube_dPhi = 2. * M_PI * rad;
    // innerRadius_pmt, fOuterRadius_pmt, height_pmt, startAngle_pmt, spanningAngle_pmt
    G4VSolid* fFiberHole = new G4Tubs("fiberHoleX", 0.0 * cm, GetHoleRadius(), GetHoleLength() / 2, 0. * deg, tube_dPhi);

    G4RotationMatrix* rotMY = new G4RotationMatrix;
    G4RotationMatrix* rotMX = new G4RotationMatrix;
    rotMY->rotateY(90. * deg);
    rotMX->rotateX(90. * deg);

    // G4double fiber_pos = 2 * mm;
    G4ThreeVector xpVec(fHolePos,          0, +fHolePos);
    G4ThreeVector ypVec(0,         +fHolePos, -fHolePos);
    G4ThreeVector zpVec(-fHolePos, -fHolePos,         0);

    solidExtrusion = new G4SubtractionSolid("solSubExtrusion1", solidExtrusion, fFiberHole, rotMX, xpVec);
    solidExtrusion = new G4SubtractionSolid("solSubExtrusion2", solidExtrusion, fFiberHole, rotMY, ypVec);
    solidExtrusion = new G4SubtractionSolid("solSubExtrusion3", solidExtrusion, fFiberHole, 0,    zpVec);

    solidSciCube = new G4SubtractionSolid("solSubSciCube", solidSciCube, fFiberHole, rotMX, xpVec);
    solidSciCube = new G4SubtractionSolid("solSubSciCube", solidSciCube, fFiberHole, rotMY, ypVec);
    solidSciCube = new G4SubtractionSolid("solSubSciCube", solidSciCube, fFiberHole, 0,    zpVec);

    fLogiExtrusion = new G4LogicalVolume(solidExtrusion, FindMaterial("Coating"), "Extrusion");
    logicScintillator = new G4LogicalVolume(solidSciCube, FindMaterial("Polystyrene"), "SciCube");

    for (int i = 0; i < 3; i++)
    {
        G4double xoffset_val = (i - 1) * sci_pitch;
        G4ThreeVector xoffset(xoffset_val, 0, 0);
        for (int j = 0; j < 3; j++)
        {
            G4double yoffset_val = (j - 1) * sci_pitch;
            G4ThreeVector yoffset(0, yoffset_val, 0);
            fPhysExtrusion[i][j] = new G4PVPlacement(0, xoffset + yoffset, fLogiExtrusion, "Extrusion", fLogiWorld, false, 0);
        }
    }
    physScintillator = new G4PVPlacement(0, G4ThreeVector(), logicScintillator, "SciCube", fLogiExtrusion, false, 0);

    // ----- define surface and table of surface properties table
    /*
        G4OpBoundaryProcess クラスを用いるときには model を設定してやる必要がある。
        この model には UNIFIED model と GLISUR model の 2 種類があり、
        GLISUR model は、Gean- t3.21(Geant4 はこの改訂版) の model であるが、
        UNIFIED model はこの GLISUR model より もリアルなシミュレーションを得ようとしており、
        表面の磨きや反射剤の全ての様子を取り扱 う model である。

        https://oxon.hatenablog.com/entry/20100121/1264056313 Nagoya, Okumura
        http://wiki.opengatecollaboration.org/index.php/Users_Guide:Generating_and_tracking_optical_photons
    */
    /*
        Lambertian reflectance ( based on Kikawa-san's optical simulation )
        groundfrontpainted 反射の仕方が常にランバート反射
           : 反射の仕方が常にランバート反射
           polishとsigma_alphaの両パラメータは無視されます。
        groundbackpainted
         polishedbackpaintedと計算過程はほぼ同じですが、反射材での反射は常にランバート反射です。
         その一方で、境界面での反射や屈折にはpolishもしくはsigma_alphaが使われます。
         従って、この境界面での反射の分布は、反射材でのランバート反射と境界面での表面粗さの組み合わさったものになります。
    */
    G4OpticalSurface* TiO2Surface(0);
    TiO2Surface = new G4OpticalSurface("TiO2Surface",
                                       // glisur,ground,dielectric_metal,1); // SetModel SetFinish SetType SetPolish
                                       // unified,polished,dielectric_metal,0.98); // SetModel SetFinish SetType SetPolish
                                       unified, groundbackpainted, dielectric_metal, 0.9);         // SetModel SetFinish SetType SetPolish

    G4MaterialPropertiesTable* TiO2SurfaceProperty = new G4MaterialPropertiesTable();

    G4double p_TiO2[] = { 2.00 * eV, 3.47 * eV };
    const int nbins = sizeof(p_TiO2) / sizeof(G4double);
    // G4double refl_TiO2[] = { 0.97, 0.97 };
    // G4double refl_TiO2[] = {0.90, 0.90};
    G4double refl_TiO2[] = { fCubeReflectivity, fCubeReflectivity };
    assert(sizeof(refl_TiO2) == sizeof(p_TiO2));
    G4double effi_TiO2[] = { 0, 0 };
    assert(sizeof(effi_TiO2) == sizeof(p_TiO2));
    G4double rind_TiO2[] = { 2.2, 2.2 };
    assert(sizeof(effi_TiO2) == sizeof(p_TiO2));

    G4cout << "###Charcters of the cube ###" << G4endl;
    G4cout << ">> reflectivity of the cube = " << refl_TiO2[0] << G4endl;
    G4cout << ">> refraction index of the cube = " << rind_TiO2[0] << G4endl;
    //   G4cout << ">> refraction index of the cube = " << rind_TiO2[0] << G4endl;

    // Boundary Process
    // p210 ~ http://ftp.jaist.ac.jp/pub/Linux/Gentoo/distfiles/BookForAppliDev-4.10.1.pdf
    // reflectivity : the ratio of reflection
    // efficiency : the ratio of absorption & energy deposit on material
    // At dielectric-metal border, first determine reflect or not with reflectivity,
    // then determine deposit or not energy on material with efficiency.
    // reflectivity= 0 means no reflection
    // efficiency  = 0 means no energy deposite on material
    TiO2SurfaceProperty->AddProperty("REFLECTIVITY", p_TiO2,  refl_TiO2, nbins);
    TiO2SurfaceProperty->AddProperty("EFFICIENCY",   p_TiO2,  effi_TiO2,  nbins);
    TiO2SurfaceProperty->AddProperty("RINDEX",       p_TiO2,  rind_TiO2,  nbins);

    // SetSigmaAlpha :
    // http://wiki.opengatecollaboration.org/index.php/Users_Guide:Generating_and_tracking_optical_photons
    // This parameter defines the standard deviation of the Gaussian distribution
    //	of micro-facets around the average surface normal
    TiO2Surface->SetSigmaAlpha(parameter::sigma_alpha);
    TiO2Surface->SetMaterialPropertiesTable(TiO2SurfaceProperty);

    new G4LogicalSkinSurface("TiO2Surface", fLogiExtrusion, TiO2Surface);


    // Boundary Surface Properties scinti-hole
    G4OpticalSurface* opSurface = new G4OpticalSurface("RoughSurface",
                                                       glisur, ground, dielectric_dielectric, 0.99); // SetModel SetFinish SetType SetPolish
    // unified,polished,dielectric_dielectric,0.99); //  SetModel SetFinish SetType SetPolish
    new G4LogicalBorderSurface("surfaceHoleXOt", fPhysWorld, physScintillator, opSurface);
    new G4LogicalBorderSurface("surfaceHoleXIn", physScintillator, fPhysWorld, opSurface);

    #if 0 // test
        G4OpticalSurface* scint_air = new G4OpticalSurface("Scint_Air");
        scint_air->SetType(dielectric_dielectric);
        scint_air->SetFinish(polishedbackpainted);
        scint_air->SetModel(unified);
        scint_air->SetSigmaAlpha(0.1 * rad);

        G4double ephoton[] = { 2.0 * eV, 3.5 * eV };
        const G4int num = sizeof(ephoton) / sizeof(G4double);

        G4double SArefectivity[num] = { 0.98, 0.98 };
        G4double SAeffciency[num] = { 0.0, 0.0 };
        G4double SArindex[num] = { 1., 1, };
        G4double SAslc[num] = { 0.9, 0.9 };
        G4double SAssc[num] = { 0., 0. };
        G4double SAbsc[num] = { 0., 0. };
        G4double SAdlc[num] = { 0.1, 0.1 };
        G4MaterialPropertiesTable* scint_airProperty = new G4MaterialPropertiesTable();
        scint_airProperty->AddProperty("RINDEX", ephoton, SArindex, num);
        scint_airProperty->AddProperty("REFLECTIVITY", ephoton, SArefectivity, num);
        scint_airProperty->AddProperty("EFFICIENCY", ephoton, SAeffciency, num);
        scint_airProperty->AddProperty("SPECULARLOBECONSTANT", ephoton, SAslc, num);
        scint_airProperty->AddProperty("SPECULARSPIKECONSTANT", ephoton, SAssc, num);
        scint_airProperty->AddProperty("BACKSCATTERCONSTANT", ephoton, SAbsc, num);
        scint_airProperty->AddProperty("DIFFUSELOBECONSTANT", ephoton, SAdlc, num);
        scint_air->SetMaterialPropertiesTable(scint_airProperty);
    #endif

    // def VISUALIZE1
    G4VisAttributes* world_va = new G4VisAttributes(G4Colour(0., 0., 0.8)); // RGB
    // world_va->SetForceSolid(true);
    fLogiWorld->SetVisAttributes(world_va);
    // fLogiWorld->SetVisAttributes(G4VisAttributes::Invisible);

    G4VisAttributes* coating_va = new G4VisAttributes(G4Colour(0.2, 0.2, 0.2)); // RGB
    // coating_va->SetForceSolid(true);
    fLogiExtrusion->SetVisAttributes(coating_va);

    G4VisAttributes* sinti_va = new G4VisAttributes(G4Colour(0.8, 0.8, 0.8)); // RGB
    // sinti_va->SetForceSolid(true);
    logicScintillator->SetVisAttributes(sinti_va);

    // ----- Fiber
    ConstructFiber();

    // ----- End of Construction
    return fPhysWorld;
}

// ....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

#define XFIBER
#define YFIBER
#define ZFIBER

void WLSDetectorConstruction::ConstructFiber()
{
    // Boundary Surface Properties
    G4OpticalSurface* opSurfAmongWLSComps(0);
    // ----- surface among fober components
    opSurfAmongWLSComps = new G4OpticalSurface("opSurfAmongWLSComps", // Surface Name
                                               glisur, // SetModel
                                               polished, // SetFinish
                                               // ground,        // SetFinish
                                               dielectric_dielectric, // SetType
                                               0.99); // SetPolish

    G4OpticalSurface* opSurfWorldCladOt = new G4OpticalSurface("opSurfWorldCladOt", // Surface Name
                                                               glisur, // SetModel
                                                               polished, // SetFinish
                                                               // ground,        // SetFinish
                                                               dielectric_dielectric, // SetType
                                                               0.95); // SetPolish

    // ----- Determine the number of cladding layers to be built
    // G4double tube_dPhi = 2.* M_PI * rad;
    G4RotationMatrix* rotMY = new G4RotationMatrix;
    G4RotationMatrix* rotMX = new G4RotationMatrix;
    rotMY->rotateY(90. * deg);
    rotMX->rotateX(90. * deg);

    // G4double fiber_pos = 2 * mm;
    G4ThreeVector xpVec(+fHolePos,   -fWLSfiberl, +fHolePos);
    G4ThreeVector ypVec(-fWLSfiberl, +fHolePos,   -fHolePos);
    G4ThreeVector zpVec(-fHolePos,   -fHolePos,   -fWLSfiberl);
    G4cout << "#### Places of the fibers in this simulation ####" << G4endl;
    G4cout << "hHolePos: center position of the fiber << " << fHolePos << " : " << fWLSfiberl << G4endl;

    double fWLSfiberRClad2X = fWLSfiberRX + 0.02 * mm + 0.02 * mm;
    G4VSolid* solWLSfiberClad2 = new G4Tubs("fWLSFiberClad2X", 0, fWLSfiberRClad2X, fWLSfiberZ, 0.0 * rad, twopi * rad);
    G4LogicalVolume* fLogiWLSCladOtX = new G4LogicalVolume(solWLSfiberClad2, FindMaterial("FPethylene"), "LogiWLSCladOtX");
    G4LogicalVolume* fLogiWLSCladOtY = new G4LogicalVolume(solWLSfiberClad2, FindMaterial("FPethylene"), "LogiWLSCladOtY");
    G4LogicalVolume* fLogiWLSCladOtZ = new G4LogicalVolume(solWLSfiberClad2, FindMaterial("FPethylene"), "LogiWLSCladOtZ");

    G4double gap = 0.01 * mm;
    G4double sci_pitch = GetBarBase() + 2 * GetCoatingThickness() + gap;
    #ifdef XFIBER
        G4VPhysicalVolume* physWLSCladOtX[3];
        for (int i = 0; i < 3; i++)
        {
            G4double xoffset_val = (i - 1) * sci_pitch;
            G4ThreeVector xoffset(xoffset_val, 0, 0);
            physWLSCladOtX[i] = new G4PVPlacement(rotMX, xpVec + xoffset, fLogiWLSCladOtX, "WLSFiberClad2X", fLogiWorld, false, 0);
        }
    #endif
    #ifdef YFIBER
        G4VPhysicalVolume* physWLSCladOtY[3];
        for (int j = 0; j < 3; j++)
        {
            G4double yoffset_val = (j - 1) * sci_pitch;
            G4ThreeVector yoffset(0, yoffset_val, 0);
            physWLSCladOtY[j] = new G4PVPlacement(rotMY, ypVec + yoffset, fLogiWLSCladOtY, "WLSFiberClad2Y", fLogiWorld, false, 0);
        }
    #endif
    #ifdef ZFIBER
        G4VPhysicalVolume* physWLSCladOtZ[3][3];
        for (int i = 0; i < 3; i++)
        {
            G4double xoffset_val = (i - 1) * sci_pitch;
            G4ThreeVector xoffset(xoffset_val, 0, 0);
            for (int j = 0; j < 3; j++)
            {
                G4double yoffset_val = (j - 1) * sci_pitch;
                G4ThreeVector yoffset(0, yoffset_val, 0);
                physWLSCladOtZ[i][j] = new G4PVPlacement(0, zpVec + xoffset + yoffset, fLogiWLSCladOtZ, "WLSFiberClad2Z", fLogiWorld, false, 0);
            }
        }
    #endif

    #ifdef XFIBER
        for (int i = 0; i < 3; i++)
        {
            new G4LogicalBorderSurface("surfWLSCladOtXWorldOt", physWLSCladOtX[i], fPhysWorld, opSurfWorldCladOt); // clad2 -> world
            new G4LogicalBorderSurface("surfWLSCladOtXWorldIn", fPhysWorld, physWLSCladOtX[i], opSurfWorldCladOt); // world -> clad2
        }
    #endif
    #ifdef YFIBER
        for (int j = 0; j < 3; j++)
        {
            new G4LogicalBorderSurface("surfWLSCladOtYWorldOt", physWLSCladOtY[j], fPhysWorld, opSurfWorldCladOt); // clad2 -> world
            new G4LogicalBorderSurface("surfWLSCladOtYWorldIn", fPhysWorld, physWLSCladOtY[j], opSurfWorldCladOt); // world -> clad2
        }
    #endif
    #ifdef ZFIBER
        for (int i = 0; i < 3; i++)
        {
            for (int j = 0; j < 3; j++)
            {
                new G4LogicalBorderSurface("surfWLSCladOtZWorldOt", physWLSCladOtZ[i][j], fPhysWorld, opSurfWorldCladOt); // clad2 -> world
                new G4LogicalBorderSurface("surfWLSCladOtZWorldIn", fPhysWorld, physWLSCladOtZ[i][j], opSurfWorldCladOt); // world -> clad2
            }
        }
    #endif

    // -----
    double fWLSfiberRCladX = fWLSfiberRX + 0.02 * mm;
    G4VSolid* solWLSfiberClad = new G4Tubs("fWLSFiberCladX", 0, fWLSfiberRCladX, fWLSfiberZ, 0.0 * rad, twopi * rad);
    G4LogicalVolume* fLogiWLSCladInX = new G4LogicalVolume(solWLSfiberClad, FindMaterial("PMMA"), "LogiWLSCladInX");
    G4LogicalVolume* fLogiWLSCladInY = new G4LogicalVolume(solWLSfiberClad, FindMaterial("PMMA"), "LogiWLSCladInY");
    G4LogicalVolume* fLogiWLSCladInZ = new G4LogicalVolume(solWLSfiberClad, FindMaterial("PMMA"), "LogiWLSCladInZ");

    #ifdef XFIBER
        G4VPhysicalVolume* physWLSCladInX = new G4PVPlacement(0, G4ThreeVector(), fLogiWLSCladInX, "WLSFiberCladX", fLogiWLSCladOtX, false, 0);
    #endif
    #ifdef YFIBER
        G4VPhysicalVolume* physWLSCladInY = new G4PVPlacement(0, G4ThreeVector(), fLogiWLSCladInY, "WLSFiberCladY", fLogiWLSCladOtY, false, 0);
    #endif
    #ifdef ZFIBER
        G4VPhysicalVolume* physWLSCladInZ = new G4PVPlacement(0, G4ThreeVector(), fLogiWLSCladInZ, "WLSFiberCladZ", fLogiWLSCladOtZ, false, 0);
    #endif

    #ifdef XFIBER
        for (int i = 0; i < 3; i++)
        {
            new G4LogicalBorderSurface("surfWLSCladInXOt", physWLSCladInX, physWLSCladOtX[i], opSurfAmongWLSComps); // clad -> clad2
            new G4LogicalBorderSurface("surfWLSCladInXIn", physWLSCladOtX[i], physWLSCladInX, opSurfAmongWLSComps); // clad2  -> clad
        }
    #endif
    #ifdef YFIBER
        for (int j = 0; j < 3; j++)
        {
            new G4LogicalBorderSurface("surfWLSCladInYOt", physWLSCladInY, physWLSCladOtY[j], opSurfAmongWLSComps); // clad -> clad2
            new G4LogicalBorderSurface("surfWLSCladInYIn", physWLSCladOtY[j], physWLSCladInY, opSurfAmongWLSComps); // clad2  -> clad
        }
    #endif
    #ifdef ZFIBER
        for (int i = 0; i < 3; i++)
        {
            for (int j = 0; j < 3; j++)
            {
                new G4LogicalBorderSurface("surfWLSCladInZOt", physWLSCladInZ, physWLSCladOtZ[i][j], opSurfAmongWLSComps); // clad -> clad2
                new G4LogicalBorderSurface("surfWLSCladInZIn", physWLSCladOtZ[i][j], physWLSCladInZ, opSurfAmongWLSComps); // clad2  -> clad
            }
        }
    #endif

    // -----
    G4VSolid* solWLSfiber = new G4Tubs("fWLSFiberX", 0, fWLSfiberRX, fWLSfiberZ, 0.0 * rad, twopi * rad);
    G4LogicalVolume* fLogiWLSCoreX = new G4LogicalVolume(solWLSfiber, FindMaterial("Pethylene"), "LogiWLSFiberX");
    G4LogicalVolume* fLogiWLSCoreY = new G4LogicalVolume(solWLSfiber, FindMaterial("Pethylene"), "LogiWLSFiberY");
    G4LogicalVolume* fLogiWLSCoreZ = new G4LogicalVolume(solWLSfiber, FindMaterial("Pethylene"), "LogiWLSFiberZ");
    // fLogiWLSCoreX->SetUserLimits(new G4UserLimits(DBL_MAX,DBL_MAX,10*ms));
    // fLogiWLSCoreY->SetUserLimits(new G4UserLimits(DBL_MAX,DBL_MAX,10*ms));
    // fLogiWLSCoreZ->SetUserLimits(new G4UserLimits(DBL_MAX,DBL_MAX,10*ms));

    #ifdef XFIBER
        G4VPhysicalVolume* physWLSfiberX = new G4PVPlacement(0, G4ThreeVector(), fLogiWLSCoreX, "WLSFiberX", fLogiWLSCladInX, false, 0);
    #endif
    #ifdef YFIBER
        G4VPhysicalVolume* physWLSfiberY = new G4PVPlacement(0, G4ThreeVector(), fLogiWLSCoreY, "WLSFiberY", fLogiWLSCladInY, false, 0);
    #endif
    #ifdef ZFIBER
        G4VPhysicalVolume* physWLSfiberZ = new G4PVPlacement(0, G4ThreeVector(), fLogiWLSCoreZ, "WLSFiberZ", fLogiWLSCladInZ, false, 0);
    #endif

    #ifdef XFIBER
        new G4LogicalBorderSurface("surfWLSCoreXOt", physWLSfiberX, physWLSCladInX, opSurfAmongWLSComps); // fiber -> clad
        new G4LogicalBorderSurface("surfWLSCoreXIn", physWLSCladInX, physWLSfiberX, opSurfAmongWLSComps); // clad  -> fiber
    #endif
    #ifdef YFIBER
        new G4LogicalBorderSurface("surfWLSCoreYOt", physWLSfiberY, physWLSCladInY, opSurfAmongWLSComps); // fiber -> clad
        new G4LogicalBorderSurface("surfWLSCoreYIn", physWLSCladInY, physWLSfiberY, opSurfAmongWLSComps); // clad  -> fiber
    #endif
    #ifdef ZFIBER
        new G4LogicalBorderSurface("surfWLSCoreZOt", physWLSfiberZ, physWLSCladInZ, opSurfAmongWLSComps); // fiber -> clad
        new G4LogicalBorderSurface("surfWLSCoreZIn", physWLSCladInZ, physWLSfiberZ, opSurfAmongWLSComps); // clad  -> fiber
    #endif

    G4VisAttributes* vaCladOt = new G4VisAttributes(G4Colour(0.1, 0.3, 0.1)); // RGB
    G4VisAttributes* vaCladIn = new G4VisAttributes(G4Colour(0.2, 0.5, 0.2)); // RGB
    G4VisAttributes* vaWLSCore = new G4VisAttributes(G4Colour(0.4, 0.7, 0.4)); // RGB
    vaCladOt->SetForceSolid(true);
    vaCladIn->SetForceSolid(true);
    vaWLSCore->SetForceSolid(true);

    #ifdef XFIBER
        fLogiWLSCladOtX->SetVisAttributes(vaCladOt);
        fLogiWLSCladInX->SetVisAttributes(vaCladIn);
        fLogiWLSCoreX->SetVisAttributes(vaWLSCore);
    #endif
    #ifdef YFIBER
        fLogiWLSCladOtY->SetVisAttributes(vaCladOt);
        fLogiWLSCladInY->SetVisAttributes(vaCladIn);
        fLogiWLSCoreY->SetVisAttributes(vaWLSCore);
    #endif
    #ifdef ZFIBER
        fLogiWLSCladOtZ->SetVisAttributes(vaCladOt);
        fLogiWLSCladInZ->SetVisAttributes(vaCladIn);
        fLogiWLSCoreZ->SetVisAttributes(vaWLSCore);
    #endif


    // --------------------------------------------------
    // Coupling at the read-out end
    // --------------------------------------------------
    #if 0
        // Clear Fiber (Coupling Layer)
        G4VSolid* solidCouple = new G4Box("Couple", fCoupleRX, fCoupleRY, fCoupleZ);

        G4LogicalVolume* logicCouple = new G4LogicalVolume(solidCouple,
                                                           FindMaterial("G4_AIR"),
                                                           "Couple");

        new G4PVPlacement(0,
                          G4ThreeVector(0.0, 0.0, fCoupleOrigin),
                          logicCouple,
                          "Couple",
                          fLogiWorld,
                          false,
                          0);
    #endif
    // --------------------------------------------------
    // A logical layer in front of PhotonDet
    // --------------------------------------------------

    // Purpose: Preventing direct dielectric to metal contact
    #if 0
        // Check for valid placement of PhotonDet
        if (fMPPCTheta > std::atan(fMPPCDist / fMPPCHalfL))
        {
            fMPPCTheta = 0;
            fMPPCOriginX = std::sin(fMPPCTheta) * (fMPPCDist + fClrfiberZ);
            fMPPCOriginZ = -fCoupleZ + std::cos(fMPPCTheta) * (fMPPCDist + fClrfiberZ);
            G4cerr << "Invalid alignment.  Alignment Reset to 0" << G4endl;
        }

        // Clear Fiber (Coupling Layer)
        G4VSolid* solidClrfiber;

        if (fMPPCShape == "Square")
            solidClrfiber = new G4Box("ClearFiber", fClrfiberHalfL, fClrfiberHalfL, fClrfiberZ);
        else
            solidClrfiber = new G4Tubs("ClearFiber", 0., fClrfiberHalfL, fClrfiberZ, 0.0 * rad, twopi * rad);

        G4LogicalVolume* logicClrfiber = new G4LogicalVolume(solidClrfiber, FindMaterial("G4_AIR"), "ClearFiber");

        new G4PVPlacement(new G4RotationMatrix(CLHEP::HepRotationY(-fMPPCTheta)),
                          G4ThreeVector(fMPPCOriginX, 0.0, fMPPCOriginZ),
                          logicClrfiber,
                          "ClearFiber",
                          logicCouple,
                          false,
                          0);
    #endif
    // --------------------------------------------------
    // PhotonDet (Sensitive Detector)
    // --------------------------------------------------

    // Physical Construction
    // G4VSolid* solidPhotonDet;
    // if (fMPPCShape=="Square") solidPhotonDet = new G4Box("PhotonDet",fMPPCHalfL,fMPPCHalfL,fMPPCZ);
    // else                      solidPhotonDet = new G4Tubs("PhotonDet",0.,fMPPCHalfL,fMPPCZ,0.0*rad,twopi*rad);
    G4VSolid* solidPhotonDetX = new G4Box("SPhotonDetX", fMPPCHalfL, fMPPCHalfL, fMPPCZ);
    G4VSolid* solidPhotonDetY = new G4Box("SPhotonDetY", fMPPCHalfL, fMPPCHalfL, fMPPCZ);
    G4VSolid* solidPhotonDetZ = new G4Box("SPhotonDetZ", fMPPCHalfL, fMPPCHalfL, fMPPCZ);
    G4LogicalVolume* logicPhotonDetX = new G4LogicalVolume(solidPhotonDetX, FindMaterial("G4_Al"), "PhotonDetX_LV");
    G4LogicalVolume* logicPhotonDetY = new G4LogicalVolume(solidPhotonDetY, FindMaterial("G4_Al"), "PhotonDetY_LV");
    G4LogicalVolume* logicPhotonDetZ = new G4LogicalVolume(solidPhotonDetZ, FindMaterial("G4_Al"), "PhotonDetZ_LV");

    // new G4PVPlacement(0, G4ThreeVector(0.0,0.0,0.0), logicPhotonDet, "PhotonDet", logicClrfiber, false, 0);
    //	G4double fiber_pos = 2*mm;
    char pvname[32];
    for (int i = 0; i < 3; i++)
    {
        G4double xoffset_val = (i - 1) * sci_pitch;
        sprintf(pvname, "PhotonDetX%d", i);
        // new G4PVPlacement(rotMX, G4ThreeVector(xoffset_val + fiber_pos, fWLSfiberZ - fWLSfiberl, +fiber_pos), logicPhotonDetX, pvname, fLogiWorld, false, 0);
        new G4PVPlacement(rotMX, G4ThreeVector(xoffset_val + fHolePos, fWLSfiberZ - fWLSfiberl, xoffset_val + fHolePos), logicPhotonDetX, pvname, fLogiWorld, false, 0);
    }
    for (int j = 0; j < 3; j++)
    {
        G4double yoffset_val = (j - 1) * sci_pitch;
        sprintf(pvname, "PhotonDetY%d", j);
        // new G4PVPlacement(rotMY, G4ThreeVector(fWLSfiberZ - fWLSfiberl, yoffset_val + fiber_pos, -fiber_pos), logicPhotonDetY, pvname, fLogiWorld, false, 0);
        new G4PVPlacement(rotMY, G4ThreeVector(fWLSfiberZ - fWLSfiberl, yoffset_val + fHolePos, -fHolePos), logicPhotonDetY, pvname, fLogiWorld, false, 0);
    }
    for (int i = 0; i < 3; i++)
    {
        G4double xoffset_val = (i - 1) * sci_pitch;
        for (int j = 0; j < 3; j++)
        {
            G4double yoffset_val = (j - 1) * sci_pitch;
            sprintf(pvname, "PhotonDetZ%d%d", i, j);
            // new G4PVPlacement(0, G4ThreeVector(xoffset_val - fiber_pos, yoffset_val - fiber_pos, fWLSfiberZ - fWLSfiberl), logicPhotonDetZ, pvname, fLogiWorld, false, 0);
            new G4PVPlacement(0, G4ThreeVector(xoffset_val - fHolePos, yoffset_val - fHolePos, fWLSfiberZ - fWLSfiberl), logicPhotonDetZ, pvname, fLogiWorld, false, 0);
        }
    }

    G4cout << "length beween cube and MPPCs = " << fWLSfiberZ - fWLSfiberl << G4endl;

    G4VisAttributes* mppc_va = new G4VisAttributes(G4Colour(0.7, 0.7, 0.7)); // RGB
    mppc_va->SetForceSolid(true);
    logicPhotonDetX->SetVisAttributes(mppc_va);
    logicPhotonDetY->SetVisAttributes(mppc_va);
    logicPhotonDetZ->SetVisAttributes(mppc_va);

    // PhotonDet Surface Properties
    G4OpticalSurface* photonDetSurface = new G4OpticalSurface("PhotonDetSurface",
                                                              glisur,
                                                              polished,
                                                              // ground,
                                                              dielectric_metal,
                                                              fMPPCPolish);

    G4MaterialPropertiesTable* photonDetSurfProp = new G4MaterialPropertiesTable();

    G4double p_mppc[NSpectrumMPPC];
    for (int i = 0; i < NSpectrumMPPC; i++)
    {
        p_mppc[i] = parameter::photonEnergy_mppc[i];
    }
    const G4int nbins = sizeof(p_mppc) / sizeof(G4double);

    // ----- refrection parameter
    fMPPCReflectivity = 0;
    G4double refl_mppc[NSpectrumMPPC];
    for (int i = 0; i < NSpectrumMPPC; i++)
    {
        refl_mppc[i] = fMPPCReflectivity;
    }
    assert(sizeof(refl_mppc) == sizeof(p_mppc));
    // ----- efficiency parameter
    // G4double effi_mppc[] = { 1, 1 };   // original
    G4double effi_mppc[NSpectrumMPPC];
    for (int i = 0; i < NSpectrumMPPC; i++)
    {
        effi_mppc[i] = parameter::effi_mppc[i];
    }
    assert(sizeof(effi_mppc) == sizeof(p_mppc));

    photonDetSurfProp->AddProperty("REFLECTIVITY", p_mppc, refl_mppc, nbins);
    photonDetSurfProp->AddProperty("EFFICIENCY",  p_mppc, effi_mppc, nbins);
    photonDetSurface->SetMaterialPropertiesTable(photonDetSurfProp);

    new G4LogicalSkinSurface("PhotonDetSurface", logicPhotonDetX, photonDetSurface);
    new G4LogicalSkinSurface("PhotonDetSurface", logicPhotonDetY, photonDetSurface);
    new G4LogicalSkinSurface("PhotonDetSurface", logicPhotonDetZ, photonDetSurface);

    // ----- Mirror for reflection at one of the end
    /*
        memo: photons that reach to another end against MPPC
            seem to reflect and come to the MPPC side.
            put mirrors to absorbe them in the mirror side.
    */

    // Place the mirror only if the user wants the mirror
    // G4VSolid* solidMirror = new G4Box("Mirror", fMirrorRmax, fMirrorRmax, fMirrorZ);
    G4VSolid* solidMirror = new G4Box("Mirror", 5 * mm, 5 * mm, fMirrorZ);
    G4LogicalVolume* logicMirror = new G4LogicalVolume(solidMirror, FindMaterial("G4_Al"), "Mirror");

    // ----- define surface
    G4OpticalSurface* mirrorSurface = new G4OpticalSurface("MirrorSurface", glisur, ground, dielectric_metal, fMirrorPolish);
    G4MaterialPropertiesTable* mirrorSurfaceProperty = new G4MaterialPropertiesTable();

    G4double p_mirror[] = { 2.00 * eV, 3.47 * eV };
    // const G4int nbins = sizeof(p_mirror)/sizeof(G4double);

    // fMirrorReflectivity = 0.;
    G4double refl_mirror[] = { fMirrorReflectivity, fMirrorReflectivity };
    assert(sizeof(refl_mirror) == sizeof(p_mirror));
    G4double effi_mirror[] = { 0, 0 };
    assert(sizeof(effi_mirror) == sizeof(effi_mirror));

    mirrorSurfaceProperty->AddProperty("REFLECTIVITY", p_mirror, refl_mirror, nbins);
    mirrorSurfaceProperty->AddProperty("EFFICIENCY", p_mirror, effi_mirror, nbins);
    mirrorSurface->SetMaterialPropertiesTable(mirrorSurfaceProperty);
    #if 1
        //   G4double fHolePos = 2*mm;
        new G4LogicalSkinSurface("MirrorSurface", logicMirror, mirrorSurface);
        new G4PVPlacement(rotMX, G4ThreeVector(+fHolePos, -fWLSfiberZ + fWLSfiberl, +fHolePos), logicMirror, "Mirror", fLogiWorld, false, 0);
        new G4PVPlacement(rotMY, G4ThreeVector(-fWLSfiberZ + fWLSfiberl, +fHolePos, -fHolePos), logicMirror, "Mirror", fLogiWorld, false, 0);
        new G4PVPlacement(0,     G4ThreeVector(-fHolePos, -fHolePos, -fWLSfiberZ + fWLSfiberl), logicMirror, "Mirror", fLogiWorld, false, 0);
        G4cout << "Mirrors are implemented in this simulation " << G4endl;
        G4cout << ">> Reflectivity of this mirror = " << fMirrorReflectivity << G4endl;
        G4cout << ">> Efficiency of this mirror = " << effi_mirror[0] << G4endl;
    #endif
}

// ....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......


void WLSDetectorConstruction::ConstructSDandField()
{
    #if 0
        if (!fmppcSD.Get())
        {
            G4String mppcSDName = "WLS/PhotonDet";
            WLSPhotonDetSD* mppcSD = new WLSPhotonDetSD(mppcSDName);
            fmppcSD.Put(mppcSD);
        }
        SetSensitiveDetector("PhotonDet_LV", fmppcSD.Get(), true);
    #endif
}

// ....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void WLSDetectorConstruction::UpdateGeometryParameters()
{
    // fWLSfiberRX  = fXYRatio * fWLSfiberRY;
    fWLSfiberRX = fWLSfiberRY;

    fClad1RX = fWLSfiberRX + 0.03 * fWLSfiberRX;
    fClad1RY = fWLSfiberRY + 0.03 * fWLSfiberRY;
    fClad1Z = fWLSfiberZ;

    fClad2RX = fClad1RX + 0.03 * fWLSfiberRX;
    fClad2RY = fClad1RY + 0.03 * fWLSfiberRY;
    fClad2Z = fWLSfiberZ;

    //  fWorldSizeX = fClad2RX   + fMPPCDist + fMPPCHalfL + 20.*cm;
    //  fWorldSizeY = fClad2RY   + fMPPCDist + fMPPCHalfL + 20.*cm;
    fWorldSizeX = fWLSfiberZ + fabs(fWLSfiberl) + fMPPCDist + fMPPCHalfL + 20. * cm;
    fWorldSizeY = fWLSfiberZ + fabs(fWLSfiberl) + fMPPCDist + fMPPCHalfL + 20. * cm;
    fWorldSizeZ = fWLSfiberZ + fabs(fWLSfiberl) + fMPPCDist + fMPPCHalfL + 20. * cm;

    fCoupleRX = fWorldSizeX;
    fCoupleRY = fWorldSizeY;
    fCoupleZ = (fWorldSizeZ - fWLSfiberZ) / 2;

    fClrfiberHalfL = fMPPCHalfL;

    fMirrorRmax = fClad2RY;

    fCoupleOrigin = fWLSfiberOrigin + fWLSfiberZ + fCoupleZ;
    fMirrorOrigin = fWLSfiberOrigin - fWLSfiberZ - fMirrorZ;
    fMPPCOriginX = std::sin(fMPPCTheta) * (fMPPCDist + fClrfiberZ);
    fMPPCOriginZ = -fCoupleZ + std::cos(fMPPCTheta) * (fMPPCDist + fClrfiberZ);
}

// ....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

G4RotationMatrix
WLSDetectorConstruction::StringToRotationMatrix(G4String rotation)
{
    // We apply successive rotations OF THE OBJECT around the FIXED
    // axes of the parent's local coordinates; rotations are applied
    // left-to-right (rotation="r1,r2,r3" => r1 then r2 then r3).

    G4RotationMatrix rot;

    unsigned int place = 0;

    while (place < rotation.size())
    {
        G4double angle;
        char* p;

        const G4String tmpstring = rotation.substr(place + 1);

        angle = strtod(tmpstring.c_str(), &p) * deg;

        if (!p || (*p != (char) ',' && *p != (char) '\0'))
        {
            G4cerr << "Invalid rotation specification: " <<
                rotation.c_str() << G4endl;
            return rot;
        }

        G4RotationMatrix thisRotation;

        switch (rotation.substr(place, 1).c_str()[0]) {
            case 'X': case 'x':
                thisRotation = G4RotationMatrix(CLHEP::HepRotationX(angle));
                break;
            case 'Y': case 'y':
                thisRotation = G4RotationMatrix(CLHEP::HepRotationY(angle));
                break;
            case 'Z': case 'z':
                thisRotation = G4RotationMatrix(CLHEP::HepRotationZ(angle));
                break;
            default:
                G4cerr << " Invalid rotation specification: " <<
                    rotation << G4endl;
                return rot;
        }

        rot = thisRotation * rot;
        place = rotation.find(',', place);
        if (place > rotation.size())
            break;
        ++place;
    }

    return rot;
}

// ....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void WLSDetectorConstruction::SetPhotonDetGeometry(G4String shape)
// Set the Geometry of the PhotonDet detector
// Pre:  shape must be either "Circle" and "Square"
{
    if (shape == "Circle" || shape == "Square")
        fMPPCShape = shape;
    G4RunManager::GetRunManager()->ReinitializeGeometry();
}

// ....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void WLSDetectorConstruction::SetNumberOfCladding(G4int num)
// Set the number of claddings
// Pre: 0 <= num <= 2
{
    fNumOfCladLayers = num;
    G4RunManager::GetRunManager()->ReinitializeGeometry();
}

// ....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void WLSDetectorConstruction::SetWLSLength(G4double length)
// Set the TOTAL length of the WLS fiber
{
    fWLSfiberZ = length;
    G4RunManager::GetRunManager()->ReinitializeGeometry();
}

// ....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void WLSDetectorConstruction::SetWLSRadius(G4double radius)
// Set the Y radius of WLS fiber
{
    fWLSfiberRY = radius;
    G4RunManager::GetRunManager()->ReinitializeGeometry();
}

// ....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void WLSDetectorConstruction::SetClad1Radius(G4double radius)
// Set the Y radius of Cladding 1
{
    fClad1RY = radius;
    G4RunManager::GetRunManager()->ReinitializeGeometry();
}

// ....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void WLSDetectorConstruction::SetClad2Radius(G4double radius)
// Set the Y radius of Cladding 2
{
    fClad2RY = radius;
    G4RunManager::GetRunManager()->ReinitializeGeometry();
}

// ....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void WLSDetectorConstruction::SetPhotonDetHalfLength(G4double halfL)
// Set the half length of the PhotonDet detector
// The half length will be the radius if PhotonDet is circular
{
    fMPPCHalfL = halfL;
    G4RunManager::GetRunManager()->ReinitializeGeometry();
}

// ....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void WLSDetectorConstruction::SetGap(G4double gap)
// Set the distance between fiber end and PhotonDet
{
    fMPPCDist = gap;
    G4RunManager::GetRunManager()->ReinitializeGeometry();
}

// ....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void WLSDetectorConstruction::SetPhotonDetAlignment(G4double theta)
// Set the Aligment of PhotonDet with respect to the z axis
// If theta is 0 deg, then the detector is perfectly aligned
// PhotonDet will be deviated by theta from z axis
// facing towards the center of the fiber
{
    fMPPCTheta = theta;
    G4RunManager::GetRunManager()->ReinitializeGeometry();
}

// ....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void WLSDetectorConstruction::SetSurfaceRoughness(G4double roughness)
// Set the Surface Roughness between Cladding 1 and WLS fiber
// Pre: 0 < roughness <= 1
{
    fSurfaceRoughness = roughness;
    G4RunManager::GetRunManager()->ReinitializeGeometry();
}

// ....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void WLSDetectorConstruction::SetMirrorPolish(G4double polish)
// Set the Polish of the mirror, polish of 1 is a perfect mirror surface
// Pre: 0 < polish <= 1
{
    fMirrorPolish = polish;
    G4RunManager::GetRunManager()->ReinitializeGeometry();
}

// ....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void WLSDetectorConstruction::SetMirrorReflectivity(G4double reflectivity)
// Set the Reflectivity of the mirror, reflectivity of 1 is a perfect mirror
// Pre: 0 < reflectivity <= 1
{
    fMirrorReflectivity = reflectivity;
    G4RunManager::GetRunManager()->ReinitializeGeometry();
}

// ....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void WLSDetectorConstruction::SetPhotonDetPolish(G4double polish)
// Set the Polish of the PhotonDet, polish of 1 is a perfect mirror surface
// Pre: 0 < polish <= 1
{
    fMPPCPolish = polish;
    G4RunManager::GetRunManager()->ReinitializeGeometry();
}

// ....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void WLSDetectorConstruction::SetPhotonDetReflectivity(G4double reflectivity)
// Set the Reflectivity of the PhotonDet, reflectivity of 1 is a perfect mirror
// Pre: 0 < reflectivity <= 1
{
    fMPPCReflectivity = reflectivity;
    G4RunManager::GetRunManager()->ReinitializeGeometry();
}

// ....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void WLSDetectorConstruction::SetMirror(G4bool flag)
// Toggle to place the mirror or not at one end (-z end) of the fiber
// True means place the mirror, false means otherwise
{
    fMirrorToggle = flag;
    G4RunManager::GetRunManager()->ReinitializeGeometry();
}

// ....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void WLSDetectorConstruction::SetXYRatio(G4double r)
// Set the ratio of the x and y radius of the ellipse (x/y)
// a ratio of 1 would produce a circle
{
    fXYRatio = r;
    G4RunManager::GetRunManager()->ReinitializeGeometry();
}

// ....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void WLSDetectorConstruction::SetBarLength(G4double length)
// Set the length of the scintillator bar
{
    fBarLength = length;
    G4RunManager::GetRunManager()->ReinitializeGeometry();
}

// ....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void WLSDetectorConstruction::SetBarBase(G4double side)
// Set the side of the scintillator bar
{
    fBarBase = side;
    G4RunManager::GetRunManager()->ReinitializeGeometry();
}

// ....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void WLSDetectorConstruction::SetHoleRadius(G4double radius)
// Set the radius of the fiber hole
{
    fHoleRadius = radius;
    G4RunManager::GetRunManager()->ReinitializeGeometry();
}

// ....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void WLSDetectorConstruction::SetCoatingThickness(G4double thick)
// Set thickness of the coating on the bars
{
    fCoatingThickness = thick;
    G4RunManager::GetRunManager()->ReinitializeGeometry();
}

// ....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void WLSDetectorConstruction::SetCoatingRadius(G4double radius)
// Set inner radius of the corner bar coating
{
    fCoatingRadius = radius;
    G4RunManager::GetRunManager()->ReinitializeGeometry();
}

// ....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

G4double WLSDetectorConstruction::GetWLSFiberLength()
{
    return fWLSfiberZ;
}

// ....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

G4double WLSDetectorConstruction::GetBarLength()
{
    return fBarLength;
}

// ....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

G4double WLSDetectorConstruction::GetBarBase()
{
    return fBarBase;
}

// ....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

G4double WLSDetectorConstruction::GetHoleRadius()
{
    return fHoleRadius;
}

// ....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

G4double WLSDetectorConstruction::GetHoleLength()
{
    return fHoleLength;
}

// ....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

G4double WLSDetectorConstruction::GetFiberRadius()
{
    return GetWLSFiberRMax();
}

// ....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

G4double WLSDetectorConstruction::GetCoatingThickness()
{
    return fCoatingThickness;
}

// ....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

G4double WLSDetectorConstruction::GetCoatingRadius()
{
    return fCoatingRadius;
}

// ....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

G4double WLSDetectorConstruction::GetWLSFiberEnd()
{
    return fWLSfiberOrigin + fWLSfiberZ;
}

// ....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

G4double WLSDetectorConstruction::GetWLSFiberRMax()
{
    if (fNumOfCladLayers == 2)
        return fClad2RY;
    if (fNumOfCladLayers == 1)
        return fClad1RY;
    return fWLSfiberRY;
}

// ....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......


// Return True if the fiber construction is ideal
G4bool WLSDetectorConstruction::IsPerfectFiber()
{
    return fSurfaceRoughness == 1. && fXYRatio == 1. &&
           (!fMirrorToggle    ||
            (fMirrorPolish    == 1. && fMirrorReflectivity == 1.));
}

// ....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

G4Material*WLSDetectorConstruction::FindMaterial(G4String name)
{
    G4Material* material = G4Material::GetMaterial(name, true);
    return material;
}
