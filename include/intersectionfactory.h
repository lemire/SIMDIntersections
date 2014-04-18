
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
#include "match.h"
#include "thomaswu.h"





std::map<std::string,intersectionfunction> realinitializefactory() {
    std::map<std::string,intersectionfunction> schemes;
    schemes[ "V1" ] = V1;
    schemes[ "f2p0" ] = match_v4_f2_p0;
    schemes[ "f4p0" ] = match_v4_f4_p0;
    schemes[ "f8p0" ] = match_v4_f8_p0;

    schemes[ "branchless" ] = branchlessintersection;
    schemes[ "scalarbranchlesscached" ] = scalar_branchless_cached;
    schemes[ "scalarbranchlesscached2" ] = scalar_branchless_cached2;
    schemes[ "scalardanbranchless" ] = branchlessintersection;
    schemes[ "scalarbranchless" ] = scalar_branchless;
    schemes[ "scalarbranchlessunrolled" ] = scalar_branchless_unrolled;
    schemes[ "@hybriddan" ] =  danielshybridintersection;


    schemes[ "widevector" ] =  widevector_intersect;
     schemes[ "widevectorleo" ] =  leowidevector_intersect;

    schemes[ "natemediumdanalt" ] =  natedanalt_medium;
    schemes[ "danfar" ] = danfar_medium;
    schemes[ "danfarmov" ] = danfar_medium_mov;

    schemes[ "danfarfar" ] = danfarfar_medium;

    schemes[ "scalarnate" ] =  nate_scalar;
    schemes[ "scalarnatewg" ] =  nate_scalarwithoutgoto;

    schemes[ "scalar1sgalloping" ] =  onesidedgallopingintersection;
    schemes[ "v1" ] =  v1;
    schemes[ "v3" ] =  v3;

    schemes[ "simdgalloping" ] =  SIMDgalloping;
    schemes[ "simdgalloping2" ] =  SIMDgalloping2;

    schemes[ "hssimd" ] =  highlyscalablewordpresscom::intersect_SIMD;
    schemes[ "hssimddan" ] =  highlyscalablewordpresscom::dan_intersect_SIMD;
    /*schemes[ "thomas_scalar" ] = compute_intersection<Intersection_find_scalar>;
    schemes[ "thomas_gallop" ] =  compute_intersection<Intersection_find_gallop>;
    schemes[ "thomas_v1" ] =  compute_intersection<Intersection_find_v1>;
    schemes[ "thomas_v1_plow" ] =  compute_intersection<Intersection_find_v1_plow>;
    schemes[ "thomas_v2" ] =  compute_intersection<Intersection_find_v2>;
    schemes[ "thomas_v3" ] =  compute_intersection<Intersection_find_v3>;
    schemes[ "thomas_v3_aligned" ] =  compute_intersection<Intersection_find_v3_aligned>;
    schemes[ "thomas_simdgallop_v0" ] =  compute_intersection<Intersection_find_simdgallop_v0>;
    schemes[ "thomas_simdgallop_v1" ] =  compute_intersection<Intersection_find_simdgallop_v1>;
    schemes[ "thomas_simdgallop_v2" ] =  compute_intersection<Intersection_find_simdgallop_v2>;
    schemes[ "thomas_simdgallop_v3" ] =  compute_intersection<Intersection_find_simdgallop_v3>;
    schemes[ "thomas_v3cmpeqflagged" ] =  compute_intersection_flagged<Intersection_find_v3_cmpeq>;
    schemes[ "thomas_v3cmpeqscalarflagged" ] =  compute_intersection_flagged<Intersection_truefind_v3_cmpeq_scalar>;
    schemes[ "thomas_v3cmpeqsimd32flagged" ] =  compute_intersection_flagged<Intersection_truefind_v3_cmpeq_simd32>;
    schemes[ "thomas_v3cmpeqsimd8flagged" ] =  compute_intersection_flagged<Intersection_truefind_v3_cmpeq_simd8>;
    schemes[ "thomas_v3cmpeqbinaryflagged" ] =  compute_intersection_flagged<Intersection_truefind_v3_cmpeq_binary>;
    */
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
    schemes[ "danfar" ] =  danfar_count_medium;

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
