// === AUDIT STATUS ===
// internal:    { status: not started, auditors: [], date: YYYY-MM-DD }
// external_1:  { status: not started, auditors: [], date: YYYY-MM-DD }
// external_2:  { status: not started, auditors: [], date: YYYY-MM-DD }
// =====================

#pragma once
#include "barretenberg/common/constexpr_utils.hpp"

#include "barretenberg/honk/proof_system/logderivative_library.hpp"
#include "ecc_lookup_relation.hpp"

namespace bb {

/**
 * @brief Expression for ECCVM lookup tables.
 * @details We use log-derivative lookup tables for the following case:
 * Table writes: ECCVMPointTable columns: we define Straus point table:
 * { {0, -15[P]}, {1, -13[P]}, ..., {15, 15[P]} }
 * write source: { precompute_round, precompute_tx, precompute_ty }
 * Table reads: ECCVMMSM columns. Each row adds up to 4 points into MSM accumulator
 * read source: { msm_slice1, msm_x1, msm_y1 }, ..., { msm_slice4, msm_x4, msm_y4 }
 * @param evals transformed to `evals + C(in(X)...)*scaling_factor`
 * @param in an std::array containing the fully extended Accumulator edges.
 * @param parameters contains beta, gamma, and public_input_delta, ....
 * @param scaling_factor optional term to scale the evaluation before adding to evals.
 */
template <typename FF>
template <typename ContainerOverSubrelations, typename AllEntities, typename Parameters>
void ECCVMLookupRelationImpl<FF>::accumulate(ContainerOverSubrelations& accumulator,
                                             const AllEntities& in,
                                             const Parameters& params,
                                             const FF& scaling_factor)
{
    accumulate_logderivative_lookup_subrelation_contributions<FF, ECCVMLookupRelationImpl<FF>>(
        accumulator, in, params, scaling_factor);
}
} // namespace bb
