#ifndef MODULESTOBUNDLESCONNECTOR_HH
#define MODULESTOBUNDLESCONNECTOR_HH

#include <global_constants.hh>
#include "global_funcs.hh"
#include "Cabling/PhiPosition.hh"
#include "Cabling/DTC.hh"


class ModulesToBundlesConnector : public GeometryVisitor {
public:
  std::map<int, Bundle*> getBundles() { return bundles_; }
  std::map<int, Bundle*> getNegBundles() { return negBundles_; }

  void visit(Barrel& b);
  void visit(Layer& l);
  void visit(RodPair& r);     
  void visit(BarrelModule& m);

  void visit(Endcap& e);
  void visit(Disk& d);
  void visit(Ring& r);
  void visit(EndcapModule& m);

  void postVisit();

private:
  bool computeBarrelFlatPartRodCablingSide(const double rodPhi, const double phiSegmentWidth);

  Category computeBundleType(const bool isBarrel, const std::string subDetectorName, const int layerDiskNumber, const int ringNumber = 0) const;
  void buildBundle(DetectorModule& m, std::map<int, Bundle*>& bundles, std::map<int, Bundle*>& negBundles, const Category& bundleType, const bool isBarrel, const std::string subDetectorName, const int layerDiskNumber, const PhiPosition& modulePhiPosition, const bool isPositiveCablingSide, const int totalNumFlatRings = 0, const bool isTiltedPart = false, const bool isExtraFlatPart = false);
  int computeBundleTypeIndex(const bool isBarrel, const Category& bundleType, const int totalNumFlatRings = 0, const bool isTilted = false, const bool isExtraFlatPart = false) const;
  int computeBundleId(const bool isBarrel, const bool isPositiveCablingSide, const int layerDiskNumber, const int phiRef, const int bundleTypeIndex) const;
  Bundle* createAndStoreBundle(std::map<int, Bundle*>& bundles, std::map<int, Bundle*>& negBundles, const int bundleId, const Category& bundleType, const std::string subDetectorName, const int layerDiskNumber, const PhiPosition& modulePhiPosition, const bool isPositiveCablingSide, const bool isTiltedPart = false);
  void connectModuleToBundle(DetectorModule& m, Bundle* bundle) const;

  void staggerModules(std::map<int, Bundle*>& bundles);
  void checkModulesToBundlesCabling(const std::map<int, Bundle*>& bundles) const;

  std::map<int, Bundle*> bundles_;
  std::map<int, Bundle*> negBundles_;

  bool isBarrel_;
  std::string barrelName_;
  int layerNumber_;
  int numRods_;
  int totalNumFlatRings_;
  double rodPhi_; 

  std::string endcapName_;
  int diskNumber_;
  int ringNumber_;
  int numModulesInRing_;

  Category bundleType_;
  bool side_;
};


#endif  // MODULESTOBUNDLESCONNECTOR_HH
