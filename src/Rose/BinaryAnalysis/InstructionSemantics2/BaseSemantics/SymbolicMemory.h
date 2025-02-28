// Symbolic memory -- a memory state where memory is represented by an SMT function whose domain is the address space and whose
// range are the bytes stored at those addresses.
#ifndef ROSE_BinaryAnalysis_InstructionSemantics2_BaseSemantics_SymbolicMemory_H
#define ROSE_BinaryAnalysis_InstructionSemantics2_BaseSemantics_SymbolicMemory_H
#include <featureTests.h>
#ifdef ROSE_ENABLE_BINARY_ANALYSIS

#include <Rose/BinaryAnalysis/InstructionSemantics2/BaseSemantics/MemoryState.h>
#include <Rose/BinaryAnalysis/SymbolicExpr.h>

namespace Rose {
namespace BinaryAnalysis {
namespace InstructionSemantics2 {
namespace BaseSemantics {

/** Shared-ownership pointer for symbolic memory state. See @ref heap_object_shared_ownership. */
typedef boost::shared_ptr<class SymbolicMemory> SymbolicMemoryPtr;

/** Purely symbolic memory state.
 *
 *  This memory state stores memory as a symbolic expression consisting of read and/or write operations expressed
 *  symbolically. The memory state can be passed to SMT solvers and included in if-then-else symbolic expressions to represent
 *  different memory states according to different paths through a specimen. */
class SymbolicMemory: public MemoryState {
public:
    /** Base type. */
    using Super = MemoryState;

    /** Shared-ownership pointer. */
    using Ptr = SymbolicMemoryPtr;

private:
    SymbolicExpr::Ptr mem_;

protected:
    // All memory states should be heap allocated; use instance(), create(), or clone() instead.
    explicit SymbolicMemory(const SValuePtr &addrProtoval, const SValuePtr &valProtoval)
        : MemoryState(addrProtoval, valProtoval) {
        // Initially assume that addresses are 32 bits wide and values are 8 bits wide. We can change this on the first access.
        mem_ = SymbolicExpr::makeMemoryVariable(32, 8);
    }

public:
    /** Instantiate a new empty memory state on the heap. */
    static SymbolicMemoryPtr instance(const SValuePtr &addrProtoval, const SValuePtr &valProtoval) {
        return SymbolicMemoryPtr(new SymbolicMemory(addrProtoval, valProtoval));
    }

public:
    // documented in base class
    virtual MemoryStatePtr create(const SValuePtr &addrProtoval, const SValuePtr &valProtoval) const override {
        return instance(addrProtoval, valProtoval);
    }

    // documented in base class
    virtual MemoryStatePtr clone() const override {
        return SymbolicMemoryPtr(new SymbolicMemory(*this));
    }

    /** Convert pointer to a SymbolicMemory pointer.
     *
     *  Converts @p x to a SymbolicMemoryPtr and asserts that it is non-null. */
    static SymbolicMemoryPtr promote(const MemoryStatePtr &x) {
        SymbolicMemoryPtr retval = boost::dynamic_pointer_cast<SymbolicMemory>(x);
        ASSERT_not_null(retval);
        return retval;
    }

public:
    /** Property: the symbolic expression for the memory.
     *
     * @{ */
    SymbolicExpr::Ptr expression() const { return mem_; }
    void expression(const SymbolicExpr::Ptr &mem);
    /** @} */

public:
    virtual bool merge(const MemoryStatePtr &other, RiscOperators *addrOps, RiscOperators *valOps) override;

    virtual void clear() override;

    virtual SValuePtr readMemory(const SValuePtr &address, const SValuePtr &dflt,
                                 RiscOperators *addrOps, RiscOperators *valOps) override;

    virtual void writeMemory(const SValuePtr &address, const SValuePtr &value,
                             RiscOperators *addrOps, RiscOperators *valOps) override;

    virtual SValuePtr peekMemory(const SValuePtr &address, const SValuePtr &dflt,
                                 RiscOperators *addrOps, RiscOperators *valOps) override;

public:
    virtual void hash(Combinatorics::Hasher&, RiscOperators *addrOps, RiscOperators *valOps) const override;

    virtual void print(std::ostream&, Formatter&) const override;
};
    
} // namespace
} // namespace
} // namespace
} // namespace

#endif
#endif
