#ifndef UTREEXO_RAMFOREST_H
#define UTREEXO_RAMFOREST_H

#include "accumulator.h"
#include <fstream>
#include <unordered_map>

namespace utreexo {

class BatchProof;

class RamForest : public Accumulator
{
private:
    struct LeafHasher {
        size_t operator()(const Hash& hash) const;
    };

    // A vector of hashes for each row.
    std::vector<std::vector<Hash>> m_data;

    // A map from leaf hashes to their positions.
    // This is needed for proving that leaves are included in the accumulator.
    std::unordered_map<Hash, uint64_t, LeafHasher> m_posmap;

    // RamForests implementation of Accumulator::Node.
    class Node;

    // NodePool for RamForest::Nodes
    NodePool<Node>* m_nodepool;

    // Path to the file in which the forest is stored.
    std::string m_file_path;
    std::fstream m_file;

    bool Restore();

    /* Return the hash at a position */
    const Hash& Read(uint64_t pos) const;

    /* Swap the hashes of ranges (from, from+range) and (to, to+range). */
    void SwapRange(uint64_t from, uint64_t to, uint64_t range);

    NodePtr<Accumulator::Node> SwapSubTrees(uint64_t from, uint64_t to) override;
    NodePtr<Accumulator::Node> MergeRoot(uint64_t parent_pos, Hash parent_hash) override;
    NodePtr<Accumulator::Node> NewLeaf(const Leaf& leaf) override;
    void FinalizeRemove(uint64_t next_num_leaves) override;

public:
    RamForest(uint64_t num_leaves, int max_nodes);
    RamForest(const std::string& file, int max_nodes);
    ~RamForest();

    bool Prove(BatchProof& proof, const std::vector<Hash>& target_hashes) const override;
    bool Verify(const BatchProof& proof, const std::vector<Hash>& target_hashes) override;
    bool Add(const std::vector<Leaf>& leaves) override;

    /** Save the forest to file. */
    bool Commit();

    Hash GetLeaf(uint64_t pos) const;
};

};     // namespace utreexo
#endif // UTREEXO_RAMFOREST_H
