
#ifndef INTERSECTIONFACTORY_H_
#define INTERSECTIONFACTORY_H_

#include "common.h"
#include "intersection.h"
#include "partitionedintersection.h"
#include "hscalableintersection.h"
#include "gallopingintersection.h"
#include "binarysearchintersection.h"
#include "hybridintersection.h"
#include "mediumintersection.h"
#include "widevectorintersection.h"
#include "branchless.h"

#ifdef __INTEL_COMPILER
/**
 * Intel does not really support C++11 so we have to do something ugly like this:
 */



std::map<std::string,intersectionfunction> realinitializefactory() {
    std::map<std::string,intersectionfunction> schemes;
    schemes[ "@hybriddan" ] =  danielshybridintersection;

    schemes[ "widevector" ] =  widevector_intersect;
     schemes[ "widevectorleo" ] =  leowidevector_intersect;

    schemes[ "natemediumdanalt" ] =  natedanalt_medium;
    schemes[ "natemediumdanfar" ] = danfar_medium;

    schemes[ "scalarnate" ] =  nate_scalar;
    schemes[ "scalar1sgalloping" ] =  onesidedgallopingintersection;

    schemes[ "hssimd" ] =  highlyscalablewordpresscom::intersect_SIMD;
    schemes[ "hssimddan" ] =  highlyscalablewordpresscom::dan_intersect_SIMD;

    return schemes;
}


std::map<std::string,cardinalityintersectionfunction> initializefactory() {
    std::map<std::string,cardinalityintersectionfunction> schemes;
    schemes[ "@hybriddan" ] =  danielshybridintersectioncardinality;

    schemes[ "widevector" ] =  widevector_cardinality_intersect;
    schemes[ "widevectorleo" ] =  leowidevector_cardinality_intersect;

    schemes[ "scalargalloping" ] =  frogintersectioncardinality;
    schemes[ "scalar1sgalloping" ] =  onesidedgallopingintersectioncardinality;
    schemes[ "scalarnate" ] =  nate_count_scalar;

    schemes[ "hssimd" ] =  highlyscalablewordpresscom::cardinality_intersect_SIMD;
    schemes[ "hssimddan" ] =  highlyscalablewordpresscom::dan_cardinality_intersect_SIMD;

    schemes[ "natemedium" ] =  nate_count_medium;
    schemes[ "natemediumdan" ] =  natedan_count_medium;
    schemes[ "natemediumdanalt" ] =  natedanalt_count_medium;
    schemes[ "natemediumdanfar" ] =  danfar_count_medium;
    schemes[ "natemediumfarfine" ] =  danfarfine_count_medium;
    return schemes;
}

std::set<std::string> initializebuggy() {
    std::set<std::string> schemes;
    schemes.insert("widevectorleo");//makes some assumptions
    return schemes;
}

std::map<std::string,cardinalityintersectionfunction> schemes = initializefactory();
std::map<std::string,intersectionfunction> realschemes = realinitializefactory();

std::set<std::string> buggyschemes = initializebuggy();


std::map<std::string,cardinalityintersectionfunctionpart> initializefactorypart() {
    std::map<std::string,cardinalityintersectionfunctionpart> partschemes;
    partschemes[ "schlegel" ] = partitioned::cardinality_intersect_partitioned;
    partschemes[ "danschlegel" ] = partitioned::faster_cardinality_intersect_partitioned;
    return partschemes;
}


std::map<std::string,cardinalityintersectionfunctionpart> partschemes = initializefactorypart();

#else

/**
 * This is the proper way to do it:
 */


std::map<std::string,intersectionfunction> realschemes = {
    {"branchless",branchlessintersection},
    {"scalarbranchless", scalar_branchless},
    {"scalarbranchlessunrolled", scalar_branchless_unrolled},
    {   "@hybriddan",danielshybridintersection},

    {   "scalar1sgalloping", onesidedgallopingintersection},
    {   "scalarnate", nate_scalar},

    {   "widevector", widevector_intersect},
    {   "widevector2", widevector2_intersect},

    {   "widevectorleo",     leowidevector_intersect},

    {   "natemediumdanalt", natedanalt_medium},
    {   "natemediumdanfar", danfar_medium},


    {   "hssimd", highlyscalablewordpresscom::intersect_SIMD},
    {   "hssimddan", highlyscalablewordpresscom::dan_intersect_SIMD},
};


std::map<std::string,cardinalityintersectionfunction> schemes = {
	{	"@hybriddan",danielshybridintersectioncardinality},

	{	"widevector", widevector_cardinality_intersect},
    {   "widevectorleo", leowidevector_cardinality_intersect},

	{	"scalargalloping",frogintersectioncardinality},
	{	"scalar1sgalloping", onesidedgallopingintersectioncardinality},
    {   "scalarnate", nate_count_scalar},

	{	"hssimd", highlyscalablewordpresscom::cardinality_intersect_SIMD},
	{	"hssimddan", highlyscalablewordpresscom::dan_cardinality_intersect_SIMD},

	{	"natemedium", nate_count_medium},
	{	"natemediumdan", natedan_count_medium},
	{   "natemediumdanalt", natedanalt_count_medium},
        {   "natemediumdanfar", danfar_count_medium},
        {   "natemediumdanfinefar", danfarfine_count_medium}
};

std::set<std::string> buggyschemes = {"widevectorleo"};

std::map<std::string,cardinalityintersectionfunctionpart> partschemes = {
	{	"schlegel",partitioned::cardinality_intersect_partitioned},
	{	"danschlegel", partitioned::faster_cardinality_intersect_partitioned}
};

#endif

/**
 * Convenience function
 */
std::vector<std::string> allNames() {
    std::vector < std::string > ans;
    for (auto i = schemes.begin(); i != schemes.end(); ++i) {
        ans.push_back(i->first);
    }
    for (auto i = partschemes.begin(); i != partschemes.end(); ++i) {
        ans.push_back(i->first);
    }
    return ans;
}
/**
 * Convenience function
 */
std::vector<std::string> allRealNames() {
    std::vector < std::string > ans;
    for (auto i = realschemes.begin(); i != realschemes.end(); ++i) {
        ans.push_back(i->first);
    }
    return ans;
}


#endif /* INTERSECTIONFACTORY_H_ */
