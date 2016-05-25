#include "RodPair.h"
#include "MessageLogger.h"

define_enum_strings(StartZMode) = { "modulecenter", "moduleedge" };

//
//  Constructor - parse geometry config file using boost property tree & read-in Rod parameters -> use number of modules to build rod
//
RodPair::RodPair(int id, double radius, double rotation, int numModules, const PropertyTree& treeProperty) :
 minZ              (string("minZ")              ),
 maxZ              (string("maxZ")              ),
 minR              (string("minR")              ),
 maxR              (string("maxR")              ),
 maxModuleThickness(string("maxModuleThickness")),
 beamSpotCover(            "beamSpotCover"      , parsedAndChecked(), true),
 m_materialObject(MaterialObject::ROD),
 m_startZMode(             "startZMode"         , parsedOnly(), StartZMode::MODULECENTER),
 m_nModules     (numModules),
 m_outerZ       (0.0),
 m_optimalRadius(radius),
 m_rotation(     rotation)
{
  // Set the geometry config parameters
  this->myid(id);
  this->store(treeProperty);
}

//
//  Constructor - parse geometry config file using boost property tree & read-in Rod parameters -> use outerZ to build rod
//
RodPair::RodPair(int id, double radius, double rotation, double outerZ, const PropertyTree& treeProperty) :
 minZ              (string("minZ")              ),
 maxZ              (string("maxZ")              ),
 minR              (string("minR")              ),
 maxR              (string("maxR")              ),
 maxModuleThickness(string("maxModuleThickness")),
 beamSpotCover(            "beamSpotCover"      , parsedAndChecked(), true),
 m_materialObject(MaterialObject::ROD),
 m_startZMode(             "startZMode"         , parsedOnly(), StartZMode::MODULECENTER),
 m_nModules     (0),
 m_outerZ       (outerZ),
 m_optimalRadius(radius),
 m_rotation(     rotation)
{
 // Set the geometry config parameters
 this->myid(id);
 this->store(treeProperty);
}

//
// Position newly individual modules if RodPair cloned from a RodPair (i.e. rotate by respective angle and shift in R) and set rod new id
//
void RodPair::buildClone(int id, double shiftR, double rotation)
{
  // Set new id
  this->myid(id);

  // Rotate all modules -> needs to follow after translation!!!
  for (auto& m : m_zPlusModules)  { m.translateR(shiftR); m.rotateZ(rotation); }
  for (auto& m : m_zMinusModules) { m.translateR(shiftR); m.rotateZ(rotation); }
}

//
// Limit rod geometry by eta cut
//
void RodPair::cutAtEta(double eta)
{
  m_zPlusModules.erase_if([eta](BarrelModule& m)  { return fabs(m.center().Eta()) > eta; });
  m_zMinusModules.erase_if([eta](BarrelModule& m) { return fabs(m.center().Eta()) > eta; });
}

//! GeometryVisitor pattern -> rod visitable
void RodPair::accept(GeometryVisitor& v)
{
  v.visit(*this);
  for (auto& m : m_zPlusModules) { m.accept(v); }
  for (auto& m : m_zMinusModules) { m.accept(v); }
}

//! GeometryVisitor pattern -> rod visitable (const. option)
void RodPair::accept(ConstGeometryVisitor& v) const
{
  v.visit(*this);
  for (const auto& m : m_zPlusModules) { m.accept(v); }
  for (const auto& m : m_zMinusModules) { m.accept(v); }
}

//! Setup: link lambda functions to various rod related properties (use setup functions for ReadOnly Computable properties)
void RodPair::setup() {

  minZ.setup([&]() { return minget2(m_zMinusModules.begin(), m_zMinusModules.end(), &DetectorModule::minZ); }); // One needs the minZ so don't bother with scanning the zPlus vector
  maxZ.setup([&]() { return maxget2(m_zPlusModules.begin() , m_zPlusModules.end() , &DetectorModule::maxZ); });
  minR.setup([&]() { return minget2(m_zPlusModules.begin() , m_zPlusModules.end() , &DetectorModule::minR); }); // MinR and maxR can be found just by scanning the zPlus vector, since the rod pair is symmetrical in R
  maxR.setup([&]() { return maxget2(m_zPlusModules.begin() , m_zPlusModules.end() , &DetectorModule::maxR); });
  maxModuleThickness.setup([&]() { return maxget2(m_zPlusModules.begin(), m_zPlusModules.end(), &DetectorModule::thickness); });
}

//
//  Constructor - parse geometry config file using boost property tree & read-in Rod parameters -> use number of modules to build rod
//
RodPairStraight::RodPairStraight(int id, double radius, double rotation, double bigDelta, int bigParity, double smallDelta, int smallParity, int numModules,
                 const PropertyTree& treeProperty) :
 RodPair(id, radius, rotation, numModules, treeProperty),
 forbiddenRange(      "forbiddenRange"      , parsedOnly()),
 zOverlap(            "zOverlap"            , parsedAndChecked() , 1.),
 zError(              "zError"              , parsedAndChecked()),
 compressed(          "compressed"          , parsedOnly(), true),
 allowCompressionCuts("allowCompressionCuts", parsedOnly(), true),
 m_ringNode(          "Ring"                , parsedOnly()),
 m_smallDelta(smallDelta),
 m_smallParity(smallParity),
 m_bigDelta(bigDelta),
 m_bigParity(bigParity)
{
  // Set the geometry config parameters (id set through base class)
  this->store(treeProperty);
}

//
//  Constructor - parse geometry config file using boost property tree & read-in Rod parameters -> use outerZ to build rod
//
RodPairStraight::RodPairStraight(int id, double radius, double rotation, double bigDelta, int bigParity, double smallDelta, int smallParity, double outerZ ,
                 const PropertyTree& treeProperty) :
 RodPair(id, radius, rotation, outerZ, treeProperty),
 forbiddenRange(      "forbiddenRange"      , parsedOnly()),
 zOverlap(            "zOverlap"            , parsedAndChecked() , 1.),
 zError(              "zError"              , parsedAndChecked()),
 compressed(          "compressed"          , parsedOnly(), true),
 allowCompressionCuts("allowCompressionCuts", parsedOnly(), true),
 m_ringNode(          "Ring"                , parsedOnly()),
 m_smallDelta(smallDelta),
 m_smallParity(smallParity),
 m_bigDelta(bigDelta),
 m_bigParity(bigParity)
{
  // Set the geometry config parameters (id set through base class)
  this->store(treeProperty);
}

//
//Number of iterations allowed when positioning or compressing modules
//
const int   RodPairStraight::c_nIterations = 100;

//
// Safety factor used when compressing modules in Z
//
const float RodPairStraight::c_safetySpaceFactor = 0.1;

//
// Build recursively individual modules
//
void RodPairStraight::build()
{
  try {

    m_materialObject.store(propertyTree());
    m_materialObject.build();

    logINFO(Form("Building %s prototype", fullid(*this).c_str()));
    check();

    // Get module length from a tree
    ReadonlyProperty<double, NoDefault> moduleLength( "length", parsedOnly());
    evaluateProperty(moduleLength);

    // Position variables
    double lastZPos       = 0.0; // Last ZPos at which module was positioned
    double newZPos        = 0.0; // New ZPos at which module is positioned
    double lastRPos       = 0.0; // Last radius at which module was positioned
    double newRPos        = 0.0; // New radius at which module is positioned
    double lastDsDistance = 0.0; // ds distance of last positioned module
    int    iMod           = 1;   // Current number of built modules

    //
    // Position first module to the centre?
    int parity = m_smallParity;

    if (m_startZMode()==StartZMode::MODULECENTER) {

      BarrelModule* mod = GeometryFactory::make<BarrelModule>(GeometryFactory::make<RectangularModule>());
      mod->myid(iMod);
      mod->store(propertyTree()); // TODO: Put it to constructor
      if (m_ringNode.count(myid())>0) mod->store(m_ringNode.at(myid())); // TODO: Put it to constructor
      mod->build();

      // Translate
      newRPos  = m_optimalRadius + (m_bigParity > 0 ? m_bigDelta : -m_bigDelta);
      newRPos += (parity > 0 ? m_smallDelta : -m_smallDelta);
      newZPos  = 0.0;

      mod->translateR(newRPos);
      mod->translateZ(newZPos);

      m_zPlusModules.push_back(mod);

      // Update info
      lastRPos = newRPos;
      lastZPos = newZPos;
      parity  *= -1;
      iMod++;
    }
    else lastZPos = -moduleLength()/2.; // Positioning algorithm starts from the centre of a previous module (here taken as virtual)

    //
    // Position other modules in positive Z

    // To avoid end-less loop
    int    targetMods = m_nModules;
    double targetZ    = m_outerZ;
    int    iIter      = 0;
    while (iMod<=targetMods || abs(newZPos)<targetZ) {

      iIter++;
      if (iIter>RodPairStraight::RodPairStraight::c_nIterations) logERROR("When positioning modules in positive Z, number of iterations exceeded allowed limit! Quitting!!!");

      BarrelModule* mod = GeometryFactory::make<BarrelModule>(GeometryFactory::make<RectangularModule>());
      mod->myid(iMod);
      mod->store(propertyTree()); // TODO: Put it to constructor
      if (m_ringNode.count(myid())>0) mod->store(m_ringNode.at(myid())); // TODO: Put it to constructor
      mod->build();

      // Translate
      newRPos  = m_optimalRadius + (m_bigParity > 0 ? m_bigDelta : -m_bigDelta);
      newRPos += (parity > 0 ? m_smallDelta : -m_smallDelta);
      newZPos  = computeNextZ(lastRPos, newRPos, lastZPos, moduleLength(), lastDsDistance, mod->dsDistance(), BuildDir::RIGHT, parity);

      mod->translateR(newRPos);
      mod->translateZ(newZPos);

      m_zPlusModules.push_back(mod);

      // Update info
      lastRPos = newRPos;
      lastZPos = newZPos;
      parity  *= -1;
      iMod++;
    }

    //
    // Build modules in negative Z
    targetMods = (m_startZMode()==StartZMode::MODULECENTER) ? m_nModules-1 : m_nModules;
    parity     = -m_smallParity;
    iMod       = 1;
    newZPos    = (m_startZMode()==StartZMode::MODULECENTER) ? 0.0 : +moduleLength()/2.;
    lastZPos   = newZPos;
    iIter      = 0;
    while (iMod<=targetMods || abs(newZPos)<targetZ) {

      iIter++;
      if (iIter>RodPairStraight::RodPairStraight::c_nIterations) logERROR("When positioning modules in negative Z, number of iterations exceeded allowed limit! Quitting!!!");

      BarrelModule* mod = GeometryFactory::make<BarrelModule>(GeometryFactory::make<RectangularModule>());
      mod->myid(iMod);
      mod->store(propertyTree()); // TODO: Put it to constructor
      if (m_ringNode.count(myid())>0) mod->store(m_ringNode.at(myid())); // TODO: Put it to constructor
      mod->build();

      // Translate & rotate (rotate after translation!)
      newRPos  = m_optimalRadius + (m_bigParity > 0 ? m_bigDelta : -m_bigDelta); // Positioning algorithm starts from the centre of a previous module (real or virtual)
      newRPos += (parity > 0 ? m_smallDelta : -m_smallDelta);
      newZPos  = computeNextZ(lastRPos, newRPos, lastZPos, moduleLength(), lastDsDistance, mod->dsDistance(), BuildDir::LEFT, parity);

      mod->translateR(newRPos);
      mod->translateZ(newZPos);

      m_zMinusModules.push_back(mod);

      // Update info
      lastRPos = newRPos;
      lastZPos = newZPos;
      parity  *= -1;
      iMod++;
    }

    //
    // Solve colisions in ZPlus
    std::map<int, double> zGuards;
    zGuards[1]      = std::numeric_limits<double>::lowest();
    zGuards[-1]     = std::numeric_limits<double>::lowest();
    parity          = m_smallParity;
    int iModule     = 1;
    int nCollisions = 0;

    logINFO("Checking for collisions in Z+ rod");

    for (auto& m : m_zPlusModules) {

      double minPhysZ = m.center().Z() - MAX(m.physicalLength(), m.length())/2;
      if (zGuards[parity] > minPhysZ) {

        double offset = zGuards[parity] - minPhysZ;
        m.translateZ(offset);
        nCollisions++;
        logINFO("  Module " + any2str(iModule) + " collides with previous " + (parity > 0 ? "outer" : "inner") + " module. Translated by " + any2str(offset) + " mm");
      }
      zGuards[parity] = m.center().Z() + MAX(m.physicalLength(), m.length())/2;
      parity = -parity;
      iModule++;
    }
    if (nCollisions>0) logWARNING("Some modules in positive Z have been translated to avoid collisions. Check info tab");

    //
    // Solve colisions in ZMinus
    zGuards[1]  = std::numeric_limits<double>::max();
    zGuards[-1] = std::numeric_limits<double>::max();
    parity      = -m_smallParity;
    iModule     = -1;
    nCollisions = 0;

    logINFO("Checking for collisions in Z- rod");

    for (auto& m : m_zMinusModules) {

      double maxPhysZ = m.center().Z() + MAX(m.physicalLength(), m.length())/2;
      if (zGuards[parity] < maxPhysZ) {

        double offset = zGuards[parity] - maxPhysZ;
        m.translateZ(offset);
        nCollisions++;
        logINFO("  Module " + any2str(iModule) + " collides with previous " + (parity > 0 ? "outer" : "inner") + " module. Translated by " + any2str(offset) + " mm");
      }
      zGuards[parity] = m.center().Z() - MAX(m.physicalLength(), m.length())/2;
      parity = -parity;
      iModule--;
    }
    if (nCollisions>0) logWARNING("Some modules in negative Z have been translated to avoid collisions. Check info tab");

    //
    // Compress modules if outerZ defined (to get maximum z position, use reverse operators)
    double curMaxZ = (m_zPlusModules.size()>1) ? MAX(m_zPlusModules.rbegin()->planarMaxZ(),(m_zPlusModules.rbegin()+1)->planarMaxZ()) : (!m_zPlusModules.empty() ? m_zPlusModules.rbegin()->planarMaxZ() : 0.);
    if (compressed() && (m_nModules==0) && (curMaxZ>m_outerZ)) compressToZ(m_outerZ);

    //
    // Rotate all modules -> needs to follow after translation!!!
    for (auto& m : m_zPlusModules)  { m.rotateZ(m_rotation); }
    for (auto& m : m_zMinusModules) { m.rotateZ(m_rotation); }
  }
  catch (PathfulException& pe) {

    pe.pushPath(fullid(*this));
    throw;
  }

  cleanup();
  builtok(true);
}

//
// Helper method calculating module optimal Z position taking into account small/bigDelta, beam spot sizes etc. -> to fully cover eta region
//
double RodPairStraight::computeNextZ(double lastRPos, double newRPos, double lastZPos, double moduleLength, double lastDsDistance, double newDsDistance, BuildDir direction, int parity)
{
  // Update maximum, minimum radius using dsDistance values (Transform given Z positions from central Z to its edge equivalent -> one calculates optimal new Z pos related to edge coverage in eta)
  double lastZ = (direction==BuildDir::RIGHT ?  lastZPos + moduleLength/2. : lastZPos - moduleLength/2.);
  double newZ  = lastZ;
  double newR  = (parity > 0 ? (newRPos  - newDsDistance/2. ) : (newRPos  + newDsDistance/2. ));
  double lastR = (parity > 0 ? (lastRPos - lastDsDistance/2.) : (lastRPos + lastDsDistance/2.));

  // Cover beam spot
  double dz = (beamSpotCover()) ? zError() : 0.0;

  // Building in positive Z direction
  if (direction == BuildDir::RIGHT) {

    // For +smallDelta a worse case is +dz, for -smallDelta -dz
    double originZ = parity > 0 ? dz : -dz;

    // Take worse from 2 effects: beam spot size or required overlap in Z
    double newZorigin  = (newZ - zOverlap()) * newR/lastR;
    double newZshifted = (newZ - originZ) * newR/lastR + originZ;
    if (beamSpotCover()) newZ = MIN(newZorigin, newZshifted);
    else                 newZ = newZorigin;

    // Transfor back to module central position
    newZ = newZ + moduleLength/2.;
  }

  // Building in negative Z direction
  else {

    // For +smallDelta a worse case is -dz, for -smallDelta +dz
    double originZ = parity > 0 ? -dz : dz;

    // Take worse from 2 effects: beam spot size or required overlap in Z
    double newZorigin  = (newZ + zOverlap()) * newR/lastR;
    double newZshifted = (newZ - originZ) * newR/lastR + originZ;
    if (beamSpotCover()) newZ = MAX(newZorigin, newZshifted);
    else                 newZ = newZorigin;

    // Transfor back to module central position
    newZ = newZ - moduleLength/2.;
  }

  // Return centre agaion
  return newZ;
}

//
// Helper method compressing modules in Z direction (symmetrically from minus/plus Z) such as they fit into -outerZ, +outerZ region
//
void RodPairStraight::compressToZ(double zLimit) {

  if (m_zPlusModules.empty()) return;

  logINFO("Layer slated for compression");

  double findMaxZModule = std::numeric_limits<double>::min();
  double findMinZModule = std::numeric_limits<double>::max();

  for (auto& m : m_zPlusModules)  if (m.planarMaxZ()>findMaxZModule) findMaxZModule = m.planarMaxZ();
  for (auto& m : m_zMinusModules) if (m.planarMinZ()<findMinZModule) findMinZModule = m.planarMinZ();

  double Deltap =  fabs(zLimit) - findMaxZModule;
  double Deltam = -fabs(zLimit) - findMinZModule;

  logINFO("Rod length cannot exceed " + any2str(zLimit));
  logINFO("  Z+ rod will be compressed by " + any2str(fabs(Deltap)));
  logINFO("  Z- rod will be compressed by " + any2str(fabs(Deltam)));

  // Cut modules beyond allowed maximum Z: outerZ
  if (allowCompressionCuts()) {

    logINFO("Compression algorithm is allowed to cut modules which fall out entirely from the maximum z line");

    //
    // Cut out modules in positive Z
    int zPlusOrigSize = m_zPlusModules.size();
    for (auto it = m_zPlusModules.begin(); it != m_zPlusModules.end();) {

      if (it->planarMinZ()>zLimit) it = m_zPlusModules.erase(it);
      else ++it;
    }
    logINFO("  " + any2str(zPlusOrigSize - m_zPlusModules.size()) + " modules were cut from the Z+ rod");

    if (zPlusOrigSize - m_zPlusModules.size()) {

      findMaxZModule = std::numeric_limits<double>::min();
      for (auto& m : m_zPlusModules)  if (m.planarMaxZ()>findMaxZModule) findMaxZModule = m.planarMaxZ();

      Deltap = fabs(zLimit) - findMaxZModule;  // we have to use findMaxZModule instead of checking only the last module as there might be inversions at high Z
      logINFO("  Z+ rod now exceeding by " + any2str(fabs(Deltap)));
    }

    //
    // Cut out modules in negative Z
    int zMinusOrigSize = m_zMinusModules.size();
    for (auto it = m_zMinusModules.begin(); it != m_zMinusModules.end();) {

      if (it->planarMaxZ()<-zLimit) it = m_zMinusModules.erase(it);
      else ++it;
    }
    logINFO("  " + any2str(zMinusOrigSize - m_zMinusModules.size()) + " modules were cut from the Z- rod");

    if (zMinusOrigSize - m_zMinusModules.size()) {

      findMinZModule = std::numeric_limits<double>::max();
      for (auto& m : m_zMinusModules) if (m.planarMinZ()<findMinZModule) findMinZModule = m.planarMinZ();

      Deltam = -fabs(zLimit) - findMinZModule;
      logINFO("  Z- rod now exceeding by " + any2str(fabs(Deltam)));
    }
  }

  //
  // Do compression in positive Z
  logINFO("Iterative compression of Z+ rod");
  int iIter=0;

  for (iIter = 0; fabs(Deltap)>c_safetySpaceFactor && iIter<RodPairStraight::c_nIterations; iIter++) {

    double maxCentreZ = std::numeric_limits<double>::min();
    for (auto& m : m_zPlusModules)  if (m.center().Z()>maxCentreZ) maxCentreZ = m.center().Z();

    // Relative fraction: offset to central Z position of uppermost module in a rod
    double deltap = Deltap/maxCentreZ;
    int    parity = m_smallParity;

    std::map<int, double> zGuards;
    zGuards[1]  = std::numeric_limits<double>::lowest();
    zGuards[-1] = std::numeric_limits<double>::lowest();

    for (auto it = m_zPlusModules.begin(); it<m_zPlusModules.end(); ++it, parity = -parity) {

      // Correction shift for each module
      double translation = deltap*it->center().Z();

      // If first module put to the centre -> avoid any shift
      if (it==m_zPlusModules.begin() && m_startZMode()==StartZMode::MODULECENTER) translation = 0.0;

      // Calculate that new lower Z position of current module so that it doesn't collide with the upper Z position of preceding module
      double minPhysZ    = MIN(it->planarMinZ(), it->center().Z() - it->physicalLength()/2);
      if (minPhysZ + translation<zGuards[parity]) translation = zGuards[parity] - minPhysZ;

      // Shift the module
      it->translateZ(translation);

      // Calculate safe boundary (new upper Z position) to avoid collision with the next module
      double maxPhysZ = MAX(it->planarMaxZ(), it->center().Z() + it->physicalLength()/2);
      zGuards[parity] = maxPhysZ;
    }

    double newMaxZ = std::numeric_limits<double>::min();
    for (auto& m : m_zPlusModules)  if (m.planarMaxZ()>newMaxZ) newMaxZ = m.planarMaxZ();

    Deltap =  fabs(zLimit) - newMaxZ;
  }
  if (iIter==RodPairStraight::c_nIterations) {
    logINFO(   "Iterative compression didn't terminate after " + any2str(RodPairStraight::c_nIterations) + " iterations. Z+ rod still exceeds by " + any2str(fabs(Deltap)));
    logWARNING("Failed to compress Z+ rod. Still exceeding by " + any2str(fabs(Deltap)) + ". Check info tab.");
  }
  else logINFO("Z+ rod successfully compressed after " + any2str(iIter) + " iterations. Rod now only exceeds by " + any2str(fabs(Deltap)) + " mm.");

  //
  // Do compression in negative Z
  logINFO("Iterative compression of Z- rod");
  iIter=0;

  for (iIter = 0; fabs(Deltam) > c_safetySpaceFactor && iIter < RodPairStraight::c_nIterations; iIter++) {

    double minCentreZ = std::numeric_limits<double>::max();
    for (auto& m : m_zMinusModules)  if (m.center().Z()<minCentreZ) minCentreZ = m.center().Z();

    // Relative fraction: offset to central Z position of lowermost module in a rod
    double deltam = Deltam/minCentreZ; // this can be optimized
    int    parity = -m_smallParity;

    std::map<int, double> zGuards;
    zGuards[1] = std::numeric_limits<double>::max();
    zGuards[-1] = std::numeric_limits<double>::max();

    for (auto it=m_zMinusModules.begin(); it<m_zMinusModules.end(); ++it, parity = -parity) {

      // Correction shift for each module
      double translation = deltam*it->center().Z();

      // Calculate that new upper Z position of current module so that it doesn't collide with the lower Z position of preceding module
      double maxPhysZ = MAX(it->planarMaxZ(), it->center().Z() + it->physicalLength()/2);
      if (maxPhysZ + translation > zGuards[parity]) translation = zGuards[parity] - maxPhysZ;

      // Shift the module
      it->translateZ(translation);

      // Calculate safe boundary (new lower Z position) to avoid collision with the next module
      double minPhysZ = MIN(it->planarMinZ(), it->center().Z() - it->physicalLength()/2);
      zGuards[parity] = minPhysZ;
    }

    double newMinZ = std::numeric_limits<double>::max();
    for (auto& m : m_zMinusModules)  if (m.planarMinZ()<newMinZ) newMinZ = m.planarMinZ();

    Deltam = -fabs(zLimit) - newMinZ;
  }
  if (iIter==RodPairStraight::c_nIterations) {
    logINFO("Iterative compression didn't terminate after " + any2str(RodPairStraight::c_nIterations) + " iterations. Z- rod still exceeds by " + any2str(fabs(Deltam)));
    logWARNING("Failed to compress Z- rod. Still exceeding by " + any2str(fabs(Deltam)) + ". Check info tab.");
  }
  else logINFO("Z- rod successfully compressed after " + any2str(iIter) + " iterations. Rod now only exceeds by " + any2str(fabs(Deltam)) + " mm.");

}

////
////
//template<typename Iterator> pair<vector<double>, vector<double>> StraightRodPair::computeZListPair(Iterator begin, Iterator end, double startZ, int recursionCounter) {
//  bool fixedStartZ = true;
//  vector<double> zPlusList = computeZList(begin, end, startZ, BuildDir::RIGHT, smallParity(), fixedStartZ);
//  vector<double> zMinusList = computeZList(begin, end, startZ, BuildDir::LEFT, -smallParity(), !fixedStartZ);
//
//  double zUnbalance = (zPlusList.back()+(*(end-1))->length()/2) + (zMinusList.back()-(*(end-1))->length()/2); // balancing uneven pos/neg strings
//
//  if (++recursionCounter == 100) { // this stops infinite recursion if the balancing doesn't converge
//    std::ostringstream tempSS;
//    tempSS << "Balanced module placement in rod pair at avg build radius " << (maxBuildRadius()+minBuildRadius())/2. << " didn't converge!! Layer is skewed";
//    tempSS << "Unbalance is " << zUnbalance << " mm";
//    logWARNING(tempSS);
//
//    return std::make_pair(zPlusList, zMinusList);
//  }
//
//  if (fabs(zUnbalance) > 0.1) { // 0.1 mm unbalance is tolerated
//    return computeZListPair(begin, end,
//                            startZ-zUnbalance/2, // countering the unbalance by displacing the startZ (by half the inverse unbalance, to improve convergence)
//                            recursionCounter);
//  } else {
//    std::ostringstream tempSS;
//    tempSS << "Balanced module placement in rod pair at avg build radius " << (maxBuildRadius()+minBuildRadius())/2. << " converged after " << recursionCounter << " step(s).\n"
//           << "   Residual Z unbalance is " << zUnbalance << ".\n"
//           << "   Positive string has " << zPlusList.size() << " modules, negative string has " << zMinusList.size() << " modules.\n"
//           << "   Z+ rod starts at " << zPlusList.front() << ", Z- rod starts at " << zMinusList.front() << ".";
//    logINFO(tempSS);
//    return std::make_pair(zPlusList, zMinusList);
//  }
//}


////
////void TiltedRodPair::buildModules(BarrelModules& modules, const RodTemplate& rodTemplate, const vector<TiltedModuleSpecs>& tmspecs, BuildDir direction) {
////  auto it = rodTemplate.begin();
////  int side = (direction == BuildDir::RIGHT ? 1 : -1);
////  if (tmspecs.empty()) return;
////  int i = (direction == BuildDir::LEFT && fabs(tmspecs[0].z) < 0.5); // this skips the first module if we're going left (i.e. neg rod) and z=0 because it means the pos rod has already got a module there
////  for (; i < tmspecs.size(); i++, ++it) {
////    BarrelModule* mod = GeometryFactory::make<BarrelModule>(**it);
////    mod->myid(i+1);
////    mod->side(side);
////    mod->tilt(side * tmspecs[i].gamma);
////    mod->translateR(tmspecs[i].r);
////    mod->translateZ(side * tmspecs[i].z);
////    modules.push_back(mod);
////  }
////}
////
////
////void TiltedRodPair::build(const RodTemplate& rodTemplate, const std::vector<TiltedModuleSpecs>& tmspecs) {
////  materialObject_.store(propertyTree());
////  materialObject_.build();
////
////  try {
////    logINFO(Form("Building %s", fullid(*this).c_str()));
////    check();
////    buildModules(zPlusModules_, rodTemplate, tmspecs, BuildDir::RIGHT);
////    buildModules(zMinusModules_, rodTemplate, tmspecs, BuildDir::LEFT);
////
////  } catch (PathfulException& pe) { pe.pushPath(fullid(*this)); throw; }
////  cleanup();
////  builtok(true);
////}
//
//
//