// === AUDIT STATUS ===
// internal:    { status: not started, auditors: [], date: YYYY-MM-DD }
// external_1:  { status: not started, auditors: [], date: YYYY-MM-DD }
// external_2:  { status: not started, auditors: [], date: YYYY-MM-DD }
// =====================

#pragma once
#include "barretenberg/common/wasm_export.hpp"
#include "barretenberg/ecc/curves/bn254/fr.hpp"
#include "barretenberg/ecc/curves/grumpkin/grumpkin.hpp"

extern "C" {

using namespace bb;
using affine_element = grumpkin::g1::affine_element;

WASM_EXPORT void pedersen_commit(fr::vec_in_buf inputs_buffer,
                                 uint32_t const* ctx_index,
                                 affine_element::out_buf output);
}
