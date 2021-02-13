#ifndef UTREEXO_ACCUMULATOR_H
#define UTREEXO_ACCUMULATOR_H

#include <array>
#include <nodepool.h>
#include <state.h>
#include <utility>
#include <vector>

namespace utreexo {
using Hash = std::array<uint8_t, 32>;
using Leaf = std::pair<Hash, bool>;

/** Provides an interface for a hash based dynamic accumulator. */
class Accumulator
{
public:
    /** BatchProof represents a proof for multiple leaves. */
    class BatchProof
    {
    private:
        // The positions of the leaves that are being proven.
        std::vector<uint64_t> targets;

        // The proof hashes for the targets.
        std::vector<Hash> proof;

        /*bool Verify(ForestState state, const std::vector<Hash>& roots, const std::vector<std::shared_ptr<Leaf>>& targetHashes) const;*/

    public:
        BatchProof(std::vector<uint64_t> targets, std::vector<Hash> proof)
            : targets(targets), proof(proof) {}
        BatchProof() {}

        const std::vector<uint64_t>& GetTargets() const { return targets; }

        void Serialize(std::vector<uint8_t>& bytes) const;
        bool Unserialize(const std::vector<uint8_t>& bytes);

        bool operator==(const BatchProof& other);

        void Print();
    };

    Accumulator(ForestState& state) : m_state(state) { this->m_roots.reserve(64); }

    virtual ~Accumulator() {}

    /** 
     * Try to prove the provided targets. 
     * Return true on success and false on failure.
     */
    virtual bool Prove(BatchProof& proof, const std::vector<Hash>& target_hashes) const = 0;

    /**
     * TODO: Verify a proof.
     * The internal state of the accumulator might be mutated but the roots will not.
     * Return whether or not the proof proved the targetHashes.
     */
    /*bool Verify(const BatchProof& proof, const std::vector<Hash>& targetHashes);*/

    /** Modify the accumulator by adding leaves and removing targets. */
    bool Modify(const std::vector<Leaf>& new_leaves,
                const std::vector<uint64_t>& targets);

    /** Return the root hashes (roots of taller trees first) */
    void Roots(std::vector<Hash>& roots) const;

    void PrintRoots() const;

protected:
    /*
     * Node represents a node in the accumulator forest.
     * This is used to create an abstraction on top of a accumulator implementation,
     * because it might not use a pointer based tree datastructure but the verification and modification
     * algorithms are quite nicely expressed using one.
     */
    class Node
    {
    public:
        // The forest state in which this node was created.
        ForestState m_forest_state;

        // A pointer to the parent node.
        // This is useful if you want to rehash a path from the bottom up.
        NodePtr<Accumulator::Node> m_parent;

        // The position of the node in the forest.
        uint64_t m_position;

        virtual ~Node() {}

        /*
         * Return the hash of the node.
         * This does not compute the hash only returns a previously computed hash.
         */
        virtual const Hash& GetHash() const = 0;

        /* Recompute the hash from children nodes and return it. */
        virtual void ReHash() = 0;

        /*
         * Return the parent of the node.
         * A return value of nullptr does *not* always indicate that a tree top was reached. 
         */
        virtual NodePtr<Accumulator::Node> Parent() const { return m_parent; }

        virtual void NodePoolDestroy() { m_parent = nullptr; }
    };

    // The state of the forest.
    ForestState& m_state;

    // The roots of the accumulator.
    std::vector<NodePtr<Accumulator::Node>> m_roots;

    /*
     * Swap two subtrees in the forest.
     * Return the nodes that need to be rehashed.
     */
    virtual NodePtr<Accumulator::Node> SwapSubTrees(uint64_t from, uint64_t to) = 0;

    // MergeRoot and NewLeaf only have the desired effect if called correctly.
    // newLeaf should be called to allocate a new leaf.
    // After calling newLeaf, mergeRoot should be called for every consecutive least significant bit that is set to 1.

    /* Return the result of the latest merge. */
    virtual NodePtr<Accumulator::Node> MergeRoot(uint64_t parent_pos, Hash parent_hash) = 0;
    /* Allocate a new leaf and assign it the given hash */
    virtual NodePtr<Accumulator::Node> NewLeaf(const Leaf& leaf) = 0;

    /* Free memory or select new roots. */
    virtual void FinalizeRemove(const ForestState next_state) = 0;

    /* Add new leaves to the accumulator. */
    virtual void Add(const std::vector<Leaf>& leaves);
    /* Remove target leaves from the accumulator. */
    bool Remove(const std::vector<uint64_t>& targets);

    /* Compute the parent hash from to children. */
    static void ParentHash(Hash& parent, const Hash& left, const Hash& right);
};

};     // namespace utreexo
#endif // UTREEXO_ACCUMULATOR_H
