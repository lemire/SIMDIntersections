
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

std::map<std::string,std::string> hhh = {
    {   "schlegel","sfds"},
    {   "danschlegel","sfds"}
};

std::map<std::string,cardinalityintersectionfunction> schemes = {
	{	"danhybrid",danielshybridintersectioncardinality},
	{	"widevector", widevector_cardinality_intersect},
	{	"danscalar", intersectioncardinality},
	{	"galloping",frogintersectioncardinality},
	{	"1sgalloping", onesidedgallopingintersectioncardinality},
	{	"textbook", classicalintersectioncardinality},
	{	"textbook2", highlyscalablewordpresscom::cardinality_intersect_scalar},
	{	"hssimd", highlyscalablewordpresscom::cardinality_intersect_SIMD},
	{	"danhssimd", highlyscalablewordpresscom::opti2_cardinality_intersect_SIMD},
	{	"natemedium", nate_count_medium},
	{	"natedanmedium", natedan_count_medium},
	{   "natedanaltmedium", natedanalt_count_medium},
    {   "danfarmedium", danfar_count_medium},
    {   "danfarfinemedium", danfarfine_count_medium}
};




std::map<std::string,cardinalityintersectionfunctionpart> partschemes = {
	{	"schlegel",partitioned::cardinality_intersect_partitioned},
	{	"danschlegel", partitioned::faster_cardinality_intersect_partitioned}
};


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


#endif /* INTERSECTIONFACTORY_H_ */
