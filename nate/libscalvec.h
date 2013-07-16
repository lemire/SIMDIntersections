#pragma once

#ifdef __cplusplus
extern "C" {
#endif

    size_t match_scalar(const uint32_t *A, const size_t lenA,
                        const uint32_t *B, const size_t lenB,
                        uint32_t *out);
       

    
    size_t match_scalvec_v4_f1(const uint32_t *rare, size_t lenRare,
                               const uint32_t *freq, size_t lenFreq,
                               uint32_t *matchOut);


    size_t match_scalvec_v4_f2(const uint32_t *rare, size_t lenRare,
                               const uint32_t *freq, size_t lenFreq,
                               uint32_t *matchOut);


    size_t match_scalvec_v4_f3(const uint32_t *rare, size_t lenRare,
                               const uint32_t *freq, size_t lenFreq,
                               uint32_t *matchOut);

    size_t match_scalvec_v4_f4(const uint32_t *rare, size_t lenRare,
                               const uint32_t *freq, size_t lenFreq,
                               uint32_t *matchOut);

    size_t match_scalvec_v4_f5(const uint32_t *rare, size_t lenRare,
                               const uint32_t *freq, size_t lenFreq,
                               uint32_t *matchOut);

    size_t match_scalvec_v4_f6(const uint32_t *rare, size_t lenRare,
                               const uint32_t *freq, size_t lenFreq,
                               uint32_t *matchOut);

    size_t match_scalvec_v4_f7(const uint32_t *rare, size_t lenRare,
                               const uint32_t *freq, size_t lenFreq,
                               uint32_t *matchOut);

    size_t match_scalvec_v4_f8(const uint32_t *rare, size_t lenRare,
                               const uint32_t *freq, size_t lenFreq,
                               uint32_t *matchOut);

    size_t match_scalvec_v4_f9(const uint32_t *rare, size_t lenRare,
                               const uint32_t *freq, size_t lenFreq,
                               uint32_t *matchOut);

    size_t match_scalvec_v4_f10(const uint32_t *rare, size_t lenRare,
                                const uint32_t *freq, size_t lenFreq,
                                uint32_t *matchOut);

    size_t match_scalvec_v4_f11(const uint32_t *rare, size_t lenRare,
                                const uint32_t *freq, size_t lenFreq,
                                uint32_t *matchOut);

    size_t match_scalvec_v4_f12(const uint32_t *rare, size_t lenRare,
                                const uint32_t *freq, size_t lenFreq,
                                uint32_t *matchOut);

#ifdef __cplusplus
} // extern "C"
#endif
