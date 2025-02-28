
#include <algorithm>
#include <unordered_set>
#include <cassert>
#include <iterator>

#include "ClassHierarchyAnalysis.h"

namespace ct = CodeThorn;

namespace
{
    /// pseudo type to indicate that an element is not in a sequence
  struct unavailable_t {};

  template <class First, class Second>
  auto key(const std::pair<First, Second>& keyval) -> const First&
  {
    return keyval.first;
  }

  template <class... Elems>
  auto key(const std::tuple<Elems...>& keydata) -> decltype( std::get<0>(keydata) )
  {
    return std::get<0>(keydata);
  }

  auto key(ct::FunctionKeyType keydata) -> ct::FunctionKeyType
  {
    return keydata;
  }

  /// \brief  traverses two ordered associative sequences in order of their elements.
  ///         The elements in the sequences must be convertible. A merge object
  ///         is called with sequence elements in order of their keys in [aa1, zz1) and [aa2, zz2).
  /// \tparam _Iterator1 an iterator of an ordered associative container
  /// \tparam _Iterator2 an iterator of an ordered associative container
  /// \tparam BinaryOperator a merge object that provides three operator()
  ///         functions.
  ///         - void operator()(_Iterator1::value_type, unavailable_t);
  ///           called when an element is in sequence 1 but not in sequence 2.
  ///         - void operator()(unavailable_t, _Iterator2::value_type);
  ///           called when an element is in sequence 2 but not in sequence 1.
  ///         - void operator()(_Iterator1::value_type, _Iterator2::value_type);
  ///           called when an element is in both sequences.
  /// \tparam Comparator compares elements in sequences.
  ///         called using both (_Iterator1::key_type, _Iterator2::key_type)
  //          and (_Iterator2::key_type, _Iterator1::key_type).
  template <class _Iterator1, class _Iterator2, class BinaryOperator, class Comparator>
  BinaryOperator
  merge_keys( _Iterator1 aa1, _Iterator1 zz1,
              _Iterator2 aa2, _Iterator2 zz2,
              BinaryOperator binop,
              Comparator comp
            )
  {
    static constexpr unavailable_t unavail;

    while (aa1 != zz1 && aa2 != zz2)
    {
      if (comp(key(*aa1), key(*aa2)))
      {
        binop(*aa1, unavail);
        ++aa1;
      }
      else if (comp(key(*aa2), key(*aa1)))
      {
        binop(unavail, *aa2);
        ++aa2;
      }
      else
      {
        binop(*aa1, *aa2);
        ++aa1; ++aa2;
      }
    }

    while (aa1 != zz1)
    {
      binop(*aa1, unavail);
      ++aa1;
    }

    while (aa2 != zz2)
    {
      binop(unavail, *aa2);
      ++aa2;
    }

    return std::move(binop);
  }
}

namespace CodeThorn
{

void
ClassAnalysis::addInheritanceEdge(value_type& descendantEntry, ClassKeyType ancestorKey, bool virtualEdge, bool directEdge)
{
  ClassKeyType descendantKey = descendantEntry.first;
  ClassData&   descendant = descendantEntry.second;
  ClassData&   ancestor = this->at(ancestorKey);

  descendant.ancestors().emplace_back(ancestorKey,   virtualEdge, directEdge);
  ancestor.descendants().emplace_back(descendantKey, virtualEdge, directEdge);
}

void
ClassAnalysis::addInheritanceEdge(value_type& descendant, const InheritanceDesc& ancestor)
{
  addInheritanceEdge(descendant, ancestor.getClass(), ancestor.isVirtual(), ancestor.isDirect());
}

bool
ClassAnalysis::areBaseDerived(ClassKeyType ancestorKey, ClassKeyType descendantKey) const
{
  using container = ClassData::AncestorContainer;

  const container&          ancestors = this->at(descendantKey).ancestors();
  container::const_iterator zz = ancestors.end();

  return zz != std::find_if( ancestors.begin(), zz,
                             [ancestorKey](const InheritanceDesc& desc) -> bool
                             {
                               return desc.getClass() == ancestorKey;
                             }
                           );
}


namespace
{
  template <class Map>
  inline
  auto
  lookup(Map& m, const typename Map::key_type& key) -> decltype(*m.find(key))
  {
    auto pos = m.find(key);
    assert(pos != m.end());

    return *pos;
  }


  using RelationAccessor = std::vector<InheritanceDesc>& (ClassData::*)();
  using RelationAccessorConst = const std::vector<InheritanceDesc>& (ClassData::*)() const;

  template <class AnalysisT, class Fn, class MemFnSelector, class Set>
  void traversal(AnalysisT& all, Fn fn, ClassKeyType curr, MemFnSelector selector, Set& explored)
  {
    if (!explored.insert(curr).second)
      return;

    auto& elem = lookup(all, curr);

    for (const InheritanceDesc& desc : (elem.second.*selector)())
    {
      if (desc.isDirect())
        traversal(all, fn, desc.getClass(), selector, explored);
    }

    fn(elem);
  }

  template <class AnalysisT, class Fn, class MemFnSelector>
  void traversal(AnalysisT& all, Fn fn, MemFnSelector selector)
  {
    std::unordered_set<ClassKeyType> explored;

    for (auto& elem : all)
      traversal(all, fn, elem.first, selector, explored);
  }
}


void topDownTraversal(ClassAnalysis& all, ClassAnalysisFn fn)
{
  RelationAccessor ancestors = &ClassData::ancestors;

  traversal(all, fn, ancestors);
}

void topDownTraversal(const ClassAnalysis& all, ClassAnalysisConstFn fn)
{
  RelationAccessorConst ancestors = &ClassData::ancestors;

  traversal(all, fn, ancestors);
}

void bottomUpTraversal(ClassAnalysis& all, ClassAnalysisFn fn)
{
  RelationAccessor descendants = &ClassData::descendants;

  traversal(all, fn, descendants);
}

void bottomUpTraversal(const ClassAnalysis& all, ClassAnalysisConstFn fn)
{
  RelationAccessorConst descendants = &ClassData::descendants;

  traversal(all, fn, descendants);
}

void unorderedTraversal(ClassAnalysis& all, ClassAnalysisFn fn)
{
  for (ClassAnalysis::value_type& elem : all)
    fn(elem);
}

void unorderedTraversal(const ClassAnalysis& all, ClassAnalysisConstFn fn)
{
  for (const ClassAnalysis::value_type& elem : all)
    fn(elem);
}

bool ClassData::hasVirtualFunctions() const
{
  return declaresVirtualFunctions() || inheritsVirtualFunctions();
}

bool ClassData::hasVirtualInheritance() const
{
  ClassData::AncestorContainer::const_iterator aa = ancestors().begin();
  ClassData::AncestorContainer::const_iterator zz = ancestors().end();
  auto isVirtualInheritance = [](const InheritanceDesc& desc) -> bool
                              {
                                return desc.isVirtual();
                              };

  return zz != std::find_if(aa, zz, isVirtualInheritance);
}

bool ClassData::hasVirtualTable() const
{
  return hasVirtualFunctions() || hasVirtualInheritance();
}




namespace
{
  bool inheritsVirtualFunctions(const ClassAnalysis& classes, const ClassAnalysis::value_type& entry)
  {
    const std::vector<InheritanceDesc>&          parents = entry.second.ancestors();
    std::vector<InheritanceDesc>::const_iterator aa = parents.begin();
    std::vector<InheritanceDesc>::const_iterator zz = parents.end();

    while (aa != zz && !classes.at(aa->getClass()).hasVirtualFunctions())
      ++aa;

    return aa != zz;
  }


  void integrateIndirectInheritance(ClassAnalysis& classes, ClassAnalysis::value_type& entry)
  {
    std::vector<InheritanceDesc>  tmp;

    // collect additional ancestors
    for (InheritanceDesc& parent : entry.second.ancestors())
      for (InheritanceDesc ancestor : classes.at(parent.getClass()).ancestors())
      {
        // skip virtual bases (they have already been propagated to derived)
        if (ancestor.isVirtual()) continue;

        ancestor.setDirect(false);
        tmp.push_back(ancestor);
      }

    // add additional ancestors
    for (InheritanceDesc ancestor : tmp)
    {
      classes.addInheritanceEdge(entry, ancestor);
    }
  }

  struct UniqueVirtualInheritancePredicate
  {
    bool operator()(const InheritanceDesc& lhs, const InheritanceDesc& rhs)
    {
      return lhs.getClass() == rhs.getClass();
    }
  };

  struct VirtualInheritanceComparator
  {
    bool operator()(const InheritanceDesc& lhs, const InheritanceDesc& rhs)
    {
      // only virtual inheritance is considered
      assert(lhs.isVirtual() && rhs.isVirtual());

      // at most one can be a direct base
      assert((rhs.isDirect() ^ rhs.isDirect()) <= 1);

      if (lhs.getClass() < rhs.getClass())
       return true;

      if (lhs.getClass() > rhs.getClass())
       return false;

      return lhs.isDirect();
    }
  };


  void copyVirtualInhertanceToDerived(ClassAnalysis& classes, ClassAnalysis::value_type& entry)
  {
    using iterator = std::vector<InheritanceDesc>::iterator;

    std::vector<InheritanceDesc> tmp;

    // collect addition ancestors
    for (InheritanceDesc& parent : entry.second.ancestors())
    {
      if (parent.isVirtual())
        tmp.push_back(parent);

      for (InheritanceDesc ancestor : classes.at(parent.getClass()).ancestors())
      {
        if (!ancestor.isVirtual()) continue;

        ancestor.setDirect(false);
        tmp.push_back(ancestor);
      }
    }

    std::sort(tmp.begin(), tmp.end(), VirtualInheritanceComparator{});

    iterator zz = std::unique(tmp.begin(), tmp.end(), UniqueVirtualInheritancePredicate{});

    // add additional ancestors
    std::for_each( tmp.begin(), zz,
                   [&classes, &entry](const InheritanceDesc& ancestor) -> void
                   {
                     if (!ancestor.isDirect())
                       classes.addInheritanceEdge(entry, ancestor);
                   }
                 );
  }

  void computeVirtualBaseInitializationOrder(ClassAnalysis& classes, ClassAnalysis::value_type& entry)
  {
    std::set<ClassKeyType>     alreadySeen;
    std::vector<ClassKeyType>& initorder = entry.second.virtualBaseClassOrder();

    // traverse direct ancestors and collect their initialization orders
    for (InheritanceDesc& parentDesc : entry.second.ancestors())
    {
      if (!parentDesc.isDirect()) continue;

      ClassKeyType                     parent = parentDesc.getClass();
      const std::vector<ClassKeyType>& parentInit = classes.at(parent).virtualBaseClassOrder();

      // add the virtual bases that had not been seen before
      for (ClassKeyType vbase : parentInit)
        if (alreadySeen.insert(vbase).second)
          initorder.push_back(vbase);

      if (parentDesc.isVirtual() && alreadySeen.insert(parent).second)
        initorder.push_back(parent);
    }
  }

  void analyzeClassRelationships(ClassAnalysis& all)
  {
    logTrace() << all.size() << std::endl;

    auto propagateVirtualInheritance =
           [&all](ClassAnalysis::value_type& rep) -> void
           {
             copyVirtualInhertanceToDerived(all, rep);
           };
    topDownTraversal(all, propagateVirtualInheritance);

    auto computeVirtualBaseClassInitializationOrder =
           [&all](ClassAnalysis::value_type& rep) -> void
           {
             computeVirtualBaseInitializationOrder(all, rep);
           };
    topDownTraversal(all, computeVirtualBaseClassInitializationOrder);

    auto flattenInheritance =
           [&all](ClassAnalysis::value_type& rep) -> void
           {
             integrateIndirectInheritance(all, rep);
           };
    topDownTraversal(all, flattenInheritance);

    auto analyzeInheritedVirtualMethod =
           [&all](ClassAnalysis::value_type& rep) -> void
           {
             rep.second.inheritsVirtualFunctions(inheritsVirtualFunctions(all, rep));
           };
    topDownTraversal(all, analyzeInheritedVirtualMethod);
  }


  void inheritanceRelations(ClassAnalysis& classes)
  {
    for (ClassAnalysis::value_type& elem : classes)
    {
      inheritanceEdges( elem.first,
                        [&elem, &classes](ClassKeyType child, ClassKeyType parent, bool virt) -> void
                        {
                          assert(elem.first == child);

                          classes.addInheritanceEdge(elem, parent, virt, true /* direct ancestor */);
                        }
                      );
    }
  }

  // returns true iff lhs < rhs
  struct VFNameTypeOrder
  {
    bool operator()(FunctionKeyType lhs, FunctionKeyType rhs) const
    {
      static constexpr bool firstDecisiveComparison = false;

      int res = 0;

      firstDecisiveComparison
      || (res = rcb.compareNames(lhs, rhs))
      || (res = rcb.compareTypes(lhs, rhs))
      ;

      return res < 0;
    }

    template <class TupleWithFunctionKeyType>
    bool operator()(const TupleWithFunctionKeyType& lhs, const TupleWithFunctionKeyType& rhs) const
    {
      return (*this)(std::get<0>(lhs), std::get<0>(rhs));
    }

    const RoseCompatibilityBridge& rcb;
  };

/*
  VirtualFunctionContainer
  extractVirtualFunctionsFromClass(const RoseCompatibilityBridge& rcb, ClassKeyType clkey)
  {
    using BasicVirtualFunctions = std::vector<std::tuple<FunctionId, bool> >;

    VirtualFunctionContainer res;
    BasicVirtualFunctions    funcs = rcb.getVirtualFunctions(clkey);

    res.reserve(funcs.size());
    std::sort(funcs.begin(), funcs.end(), VFNameTypeOrder{ rcb });

    //~ std::move(funcs.begin(), funcs.end(), std::back_inserter(virtualFuncs));
    for (const std::tuple<FunctionId, bool>& func : funcs)
      res.emplace_back(func);

    return res;
  }
*/
}

struct ComputeVFunctionRelation
{
  void operator()(FunctionKeyType drv, FunctionKeyType bas) const
  {
    using ReturnTypeRelation = RoseCompatibilityBridge::ReturnTypeRelation;

    const ReturnTypeRelation rel = rcb.haveSameOrCovariantReturn(classes, bas, drv);
    ROSE_ASSERT(rel != RoseCompatibilityBridge::unrelated);

    if (rel == RoseCompatibilityBridge::unrelated)
    {
      logError() << "oops, covariant return assertion failed: "
                 << rcb.nameOf(bas) << " <> " << rcb.nameOf(drv)
                 << std::endl;
      return;
    }

    const bool               covariant = rel == RoseCompatibilityBridge::covariant;

    vfunAnalysis.at(drv).overridden().emplace_back(bas, covariant);
    vfunAnalysis.at(bas).overriders().emplace_back(drv, covariant);
  }

  void operator()(FunctionKeyType, unavailable_t) const {}

  void operator()(unavailable_t, FunctionKeyType) const {}

  // data members
  const RoseCompatibilityBridge& rcb;
  const ClassAnalysis&           classes;
  VirtualFunctionAnalysis&       vfunAnalysis;
  ClassKeyType                   ancestor;
  ClassKeyType                   descendant;
};

using SortedVirtualMemberFunctions = std::unordered_map<ClassKeyType, ClassData::VirtualFunctionContainer>;

void computeOverriders( const RoseCompatibilityBridge& rcb,
                        const ClassAnalysis& classes,
                        VirtualFunctionAnalysis& vfunAnalysis,
                        const ClassAnalysis::value_type& entry,
                        bool normalizedSignature,
                        SortedVirtualMemberFunctions& sortedVFunMap
                      )
{
  using VirtualFunctionContainer = ClassData::VirtualFunctionContainer;

  // create a new entry
  VirtualFunctionContainer& vfunSorted = sortedVFunMap[entry.first];

  ROSE_ASSERT(vfunSorted.empty());

  vfunSorted = entry.second.virtualFunctions();
  std::sort(vfunSorted.begin(), vfunSorted.end(), VFNameTypeOrder{ rcb });

  std::for_each( vfunSorted.begin(), vfunSorted.end(),
                 [&vfunAnalysis, &entry, &rcb](FunctionKeyType id) -> void
                 {
                   vfunAnalysis.emplace(id, VirtualFunctionDesc{entry.first, rcb.isPureVirtual(id)});
                 }
               );

  for (const InheritanceDesc& parentDesc : entry.second.ancestors())
  {
    VirtualFunctionContainer& parentVFunSorted = sortedVFunMap.at(parentDesc.getClass());

    merge_keys( vfunSorted.begin(), vfunSorted.end(),
                parentVFunSorted.begin(), parentVFunSorted.end(),
                ComputeVFunctionRelation{rcb, classes, vfunAnalysis, entry.first, parentDesc.getClass()},
                VFNameTypeOrder{rcb}
              );
  }
}

VirtualFunctionAnalysis
analyzeVirtualFunctions(const RoseCompatibilityBridge& rcb, const ClassAnalysis& all, bool normalizedSignature)
{
  VirtualFunctionAnalysis      res;
  SortedVirtualMemberFunctions tmpSorted;

  topDownTraversal( all,
                    [&rcb, &all, &res, &tmpSorted, &normalizedSignature]
                    (const ClassAnalysis::value_type& clrep) -> void
                    {
                      computeOverriders(rcb, all, res, clrep, normalizedSignature, tmpSorted);
                    }
                  );

  return res;
}

VirtualFunctionAnalysis
analyzeVirtualFunctions(const ClassAnalysis& all, bool normalizedSignature)
{
  return analyzeVirtualFunctions(RoseCompatibilityBridge{}, all, normalizedSignature);
}


AnalysesTuple
analyzeClassesAndCasts(const RoseCompatibilityBridge& rcb, ASTRootType n)
{
  ClassAnalysis classes;
  CastAnalysis  casts;

  rcb.extractFromProject(classes, casts, n);
  inheritanceRelations(classes);
  analyzeClassRelationships(classes);

  return AnalysesTuple{std::move(classes), std::move(casts)};
}

ClassAnalysis
analyzeClasses(const RoseCompatibilityBridge& rcb, ASTRootType n)
{
  return std::move(analyzeClassesAndCasts(rcb, n).classAnalysis());
}

ClassAnalysis
analyzeClasses(ASTRootType n)
{
  return analyzeClasses(RoseCompatibilityBridge{}, n);
}

}
