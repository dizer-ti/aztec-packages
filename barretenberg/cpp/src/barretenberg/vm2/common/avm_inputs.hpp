// NOTE: names are in camel-case because they matter to messagepack.
// DO NOT use camel-case outside of these structures.
#pragma once

#include <cstdint>
#include <ostream>
#include <vector>

#include "barretenberg/common/utils.hpp"
#include "barretenberg/crypto/merkle_tree/indexed_tree/indexed_leaf.hpp"
#include "barretenberg/crypto/merkle_tree/response.hpp"
#include "barretenberg/serialize/msgpack.hpp"
#include "barretenberg/world_state/world_state.hpp"

#include "barretenberg/vm2/common/aztec_types.hpp"
#include "barretenberg/vm2/common/field.hpp"
#include "barretenberg/world_state/types.hpp"
#include "msgpack/adaptor/define_decl.hpp"

namespace bb::avm2 {

////////////////////////////////////////////////////////////////////////////
// Public Inputs
////////////////////////////////////////////////////////////////////////////

struct GlobalVariables {
    FF blockNumber;

    bool operator==(const GlobalVariables& other) const = default;

    MSGPACK_FIELDS(blockNumber);
};

struct AppendOnlyTreeSnapshot {
    FF root;
    uint64_t nextAvailableLeafIndex;

    std::size_t hash() const noexcept { return utils::hash_as_tuple(root, nextAvailableLeafIndex); }
    bool operator==(const AppendOnlyTreeSnapshot& other) const = default;
    friend std::ostream& operator<<(std::ostream& os, const AppendOnlyTreeSnapshot& obj)
    {
        os << "root: " << obj.root << ", nextAvailableLeafIndex: " << obj.nextAvailableLeafIndex;
        return os;
    }

    MSGPACK_FIELDS(root, nextAvailableLeafIndex);
};

struct TreeSnapshots {
    AppendOnlyTreeSnapshot l1ToL2MessageTree;
    AppendOnlyTreeSnapshot noteHashTree;
    AppendOnlyTreeSnapshot nullifierTree;
    AppendOnlyTreeSnapshot publicDataTree;

    bool operator==(const TreeSnapshots& other) const = default;

    MSGPACK_FIELDS(l1ToL2MessageTree, noteHashTree, nullifierTree, publicDataTree);
};

struct PublicInputs {
    GlobalVariables globalVariables;
    TreeSnapshots startTreeSnapshots;
    bool reverted;

    static PublicInputs from(const std::vector<uint8_t>& data);
    // TODO: implement three following methods.
    std::vector<std::vector<FF>> to_columns() const { return { { static_cast<uint8_t>(reverted) } }; }
    // Flatten public input columns as a single vector.
    static std::vector<FF> columns_to_flat(std::vector<std::vector<FF>> columns) { return columns[0]; }

    // Reverse direction as the above but needs to templated as recursive verifier needs it with a circuit type.
    template <typename FF_> static std::vector<std::vector<FF_>> flat_to_columns(const std::vector<FF_>& input)
    {
        return { input };
    }

    bool operator==(const PublicInputs& other) const = default;

    MSGPACK_FIELDS(globalVariables, startTreeSnapshots, reverted);
};

////////////////////////////////////////////////////////////////////////////
// Hints (contracts)
////////////////////////////////////////////////////////////////////////////
struct PublicKeysHint {
    AffinePoint masterNullifierPublicKey;
    AffinePoint masterIncomingViewingPublicKey;
    AffinePoint masterOutgoingViewingPublicKey;
    AffinePoint masterTaggingPublicKey;

    bool operator==(const PublicKeysHint& other) const = default;

    MSGPACK_FIELDS(masterNullifierPublicKey,
                   masterIncomingViewingPublicKey,
                   masterOutgoingViewingPublicKey,
                   masterTaggingPublicKey);
};

struct ContractInstanceHint {
    AztecAddress address;
    FF salt;
    AztecAddress deployer;
    ContractClassId currentContractClassId;
    ContractClassId originalContractClassId;
    FF initializationHash;
    PublicKeysHint publicKeys;

    bool operator==(const ContractInstanceHint& other) const = default;

    MSGPACK_FIELDS(
        address, salt, deployer, currentContractClassId, originalContractClassId, initializationHash, publicKeys);
};

struct ContractClassHint {
    FF classId;
    FF artifactHash;
    FF privateFunctionsRoot;
    std::vector<uint8_t> packedBytecode;

    bool operator==(const ContractClassHint& other) const = default;

    MSGPACK_FIELDS(classId, artifactHash, privateFunctionsRoot, packedBytecode);
};

struct BytecodeCommitmentHint {
    FF classId;
    FF commitment;

    bool operator==(const BytecodeCommitmentHint& other) const = default;

    MSGPACK_FIELDS(classId, commitment);
};

////////////////////////////////////////////////////////////////////////////
// Hints (merkle db)
////////////////////////////////////////////////////////////////////////////
struct GetSiblingPathHint {
    AppendOnlyTreeSnapshot hintKey;
    // params
    world_state::MerkleTreeId treeId;
    uint64_t index;
    // return
    std::vector<FF> path;

    bool operator==(const GetSiblingPathHint& other) const = default;

    MSGPACK_FIELDS(hintKey, treeId, index, path);
};

struct GetPreviousValueIndexHint {
    AppendOnlyTreeSnapshot hintKey;
    // params
    world_state::MerkleTreeId treeId;
    FF value;
    // return
    uint64_t index;
    bool alreadyPresent;

    bool operator==(const GetPreviousValueIndexHint& other) const = default;

    MSGPACK_FIELDS(hintKey, treeId, value, index, alreadyPresent);
};

template <typename LeafPreimage_> struct GetLeafPreimageHint {
    AppendOnlyTreeSnapshot hintKey;
    // params (tree id will be implicit)
    uint64_t index;
    // return
    LeafPreimage_ leafPreimage;

    bool operator==(const GetLeafPreimageHint<LeafPreimage_>& other) const = default;

    MSGPACK_FIELDS(hintKey, index, leafPreimage);
};

struct GetLeafValueHint {
    AppendOnlyTreeSnapshot hintKey;
    // params
    world_state::MerkleTreeId treeId;
    uint64_t index;
    // return
    FF value;

    bool operator==(const GetLeafValueHint& other) const = default;

    MSGPACK_FIELDS(hintKey, treeId, index, value);
};

template <typename Leaf> struct SequentialInsertHint {
    AppendOnlyTreeSnapshot hintKey;
    // params
    world_state::MerkleTreeId treeId;
    Leaf leaf;
    // return
    crypto::merkle_tree::LeafUpdateWitnessData<Leaf> lowLeavesWitnessData;
    crypto::merkle_tree::LeafUpdateWitnessData<Leaf> insertionWitnessData;
    // evolved state
    AppendOnlyTreeSnapshot stateAfter;

    bool operator==(const SequentialInsertHint<Leaf>& other) const = default;

    MSGPACK_FIELDS(hintKey, treeId, leaf, lowLeavesWitnessData, insertionWitnessData, stateAfter);
};

// Hint for MerkleTreeDB.appendLeaves.
// Note: only supported for NOTE_HASH_TREE and L1_TO_L2_MESSAGE_TREE.
struct AppendLeavesHint {
    AppendOnlyTreeSnapshot hintKey;
    AppendOnlyTreeSnapshot stateAfter;
    // params
    world_state::MerkleTreeId treeId;
    std::vector<FF> leaves;

    bool operator==(const AppendLeavesHint& other) const = default;

    MSGPACK_FIELDS(hintKey, stateAfter, treeId, leaves);
};

struct CheckpointActionNoStateChangeHint {
    // key
    uint32_t actionCounter;
    // current checkpoint evolution
    uint32_t oldCheckpointId;
    uint32_t newCheckpointId;

    bool operator==(const CheckpointActionNoStateChangeHint& other) const = default;

    MSGPACK_FIELDS(actionCounter, oldCheckpointId, newCheckpointId);
};

using CreateCheckpointHint = CheckpointActionNoStateChangeHint;
using CommitCheckpointHint = CheckpointActionNoStateChangeHint;

struct RevertCheckpointHint {
    // key
    uint32_t actionCounter;
    // current checkpoint evolution
    uint32_t oldCheckpointId;
    uint32_t newCheckpointId;
    // state evolution
    TreeSnapshots stateBefore;
    TreeSnapshots stateAfter;

    bool operator==(const RevertCheckpointHint& other) const = default;

    MSGPACK_FIELDS(actionCounter, oldCheckpointId, newCheckpointId, stateBefore, stateAfter);
};

////////////////////////////////////////////////////////////////////////////
// Hints (other)
////////////////////////////////////////////////////////////////////////////

// The reason we need EnqueuedCall hints at all (and cannot just use the public inputs) is
// because they don't have the actual calldata, just the hash of it.
struct EnqueuedCallHint {
    AztecAddress msgSender;
    AztecAddress contractAddress;
    std::vector<FF> calldata;
    bool isStaticCall;

    bool operator==(const EnqueuedCallHint& other) const = default;

    MSGPACK_FIELDS(msgSender, contractAddress, calldata, isStaticCall);
};

struct AccumulatedData {
    // TODO: add as needed.
    std::vector<FF> noteHashes;
    std::vector<FF> nullifiers;

    bool operator==(const AccumulatedData& other) const = default;

    MSGPACK_FIELDS(noteHashes, nullifiers);
};

// We are currently using this structure as the input to TX simulation.
// That's why I'm not calling it TxHint. We can reconsider if the inner types seem to dirty.
struct Tx {
    std::string hash;
    GlobalVariables globalVariables;
    AccumulatedData nonRevertibleAccumulatedData;
    AccumulatedData revertibleAccumulatedData;
    std::vector<EnqueuedCallHint> setupEnqueuedCalls;
    std::vector<EnqueuedCallHint> appLogicEnqueuedCalls;
    std::optional<EnqueuedCallHint> teardownEnqueuedCall;

    bool operator==(const Tx& other) const = default;

    MSGPACK_FIELDS(hash,
                   globalVariables,
                   nonRevertibleAccumulatedData,
                   revertibleAccumulatedData,
                   setupEnqueuedCalls,
                   appLogicEnqueuedCalls,
                   teardownEnqueuedCall);
};

struct ExecutionHints {
    Tx tx;
    // Contracts.
    std::vector<ContractInstanceHint> contractInstances;
    std::vector<ContractClassHint> contractClasses;
    std::vector<BytecodeCommitmentHint> bytecodeCommitments;
    // Merkle DB.
    TreeSnapshots startingTreeRoots;
    std::vector<GetSiblingPathHint> getSiblingPathHints;
    std::vector<GetPreviousValueIndexHint> getPreviousValueIndexHints;
    std::vector<GetLeafPreimageHint<crypto::merkle_tree::IndexedLeaf<crypto::merkle_tree::PublicDataLeafValue>>>
        getLeafPreimageHintsPublicDataTree;
    std::vector<GetLeafPreimageHint<crypto::merkle_tree::IndexedLeaf<crypto::merkle_tree::NullifierLeafValue>>>
        getLeafPreimageHintsNullifierTree;
    std::vector<GetLeafValueHint> getLeafValueHints;
    std::vector<SequentialInsertHint<crypto::merkle_tree::PublicDataLeafValue>> sequentialInsertHintsPublicDataTree;
    std::vector<SequentialInsertHint<crypto::merkle_tree::NullifierLeafValue>> sequentialInsertHintsNullifierTree;
    std::vector<AppendLeavesHint> appendLeavesHints;
    std::vector<CreateCheckpointHint> createCheckpointHints;
    std::vector<CommitCheckpointHint> commitCheckpointHints;
    std::vector<RevertCheckpointHint> revertCheckpointHints;

    bool operator==(const ExecutionHints& other) const = default;

    MSGPACK_FIELDS(tx,
                   contractInstances,
                   contractClasses,
                   bytecodeCommitments,
                   startingTreeRoots,
                   getSiblingPathHints,
                   getPreviousValueIndexHints,
                   getLeafPreimageHintsPublicDataTree,
                   getLeafPreimageHintsNullifierTree,
                   getLeafValueHints,
                   sequentialInsertHintsPublicDataTree,
                   sequentialInsertHintsNullifierTree,
                   appendLeavesHints,
                   createCheckpointHints,
                   commitCheckpointHints,
                   revertCheckpointHints);
};

////////////////////////////////////////////////////////////////////////////
// AVM Inputs
////////////////////////////////////////////////////////////////////////////
struct AvmProvingInputs {
    PublicInputs publicInputs;
    ExecutionHints hints;

    static AvmProvingInputs from(const std::vector<uint8_t>& data);
    bool operator==(const AvmProvingInputs& other) const = default;

    MSGPACK_FIELDS(publicInputs, hints);
};

} // namespace bb::avm2
