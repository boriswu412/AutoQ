#include <autoq/aut_description.hh>
#include <autoq/complex/complex.hh>
#include <autoq/symbol/concrete.hh>
#include <autoq/symbol/symbolic.hh>
#include <autoq/symbol/predicate.hh>
#include <autoq/util/util.hh>
#include <autoq/inclusion.hh>
#include <autoq/serialization/timbuk_serializer.hh>

#include "simulation/explicit_lts.hh"

#include <fstream>
#include <numeric>
#include <chrono>
#include <queue>
#include <regex>
#include <boost/dynamic_bitset.hpp>

using namespace AUTOQ;
using namespace AUTOQ::Util;
using AUTOQ::Symbol::Concrete;
using AUTOQ::Serialization::TimbukSerializer;

// using State                   = TreeAutomata::State;
// using StateSet                = TreeAutomata::StateSet;
// using StateVector             = TreeAutomata::StateVector;
// using SymbolTag                  = TreeAutomata::SymbolTag;
// using TransitionMap           = TreeAutomata::TransitionMap;

// using DiscontBinaryRelOnStates= DiscontBinaryRelation<State>;
// using StateToIndexMap         = std::unordered_map<State, size_t>;
// using StateToIndexTranslWeak  = Util::TranslatorWeak<StateToIndexMap>;
// using StateToStateMap         = std::unordered_map<State, State>;
// using StateToStateTranslWeak  = Util::TranslatorWeak<StateToStateMap>;

namespace {

  /**
   * @brief  Combine two hash values
   *
   * Values taken from
   * http://www.boost.org/doc/libs/1_64_0/boost/functional/hash/hash.hpp
   *
   * TODO: fix to be more suitable for 64b
   */
  template <class T>
  inline size_t hash_combine(size_t lhs, const T& rhs)
  {
    size_t rhs_hash = std::hash<T>{}(rhs);
    lhs ^= rhs_hash + 0x9e3779b9 + (lhs<<6) + (lhs>>2);
    return lhs;
  }

  /**
   * @brief  Hashes a range
   *
   * Inspired by
   * http://www.boost.org/doc/libs/1_64_0/boost/functional/hash/hash.hpp
   */
  template <typename It>
  size_t hash_range(It first, It last)
  {
    size_t accum = 0;
    for (; first != last; ++first) {
      accum = hash_combine(accum, *first);
    }

    return accum;
  }
} // anonymous namespace

namespace std
{
  /**
   * @brief  A hasher for vectors
   */
  template <class A>
  struct hash<std::vector<A>>
  {
    inline size_t operator()(const std::vector<A>& cont) const
    {
      return hash_range(cont.begin(), cont.end());
    }
  };
} // namespace std

template <typename Symbol>
void AUTOQ::Automata<Symbol>::remove_useless(bool only_bottom_up) {
    auto start = std::chrono::steady_clock::now();
    bool changed;
    std::vector<bool> traversed(stateNum, false);
    TransitionMap transitions_remaining = transitions;
    TransitionMap transitions_mother;

    /*******************
     * Part 1: Bottom-Up
     *******************/
    do {
        changed = false;
        transitions_mother = transitions_remaining;
        for (const auto &t : transitions_mother) {
            const auto &symbol_tag = t.first;
            for (const auto &in_out : t.second) {
                const auto &in = in_out.first;
                const auto &outs = in_out.second;
                bool input_traversed = in.empty();
                if (!input_traversed) {
                    input_traversed = true;
                    for (const auto &s : in)
                        input_traversed &= traversed[s];
                }
                if (input_traversed) {
                    for (const auto &s : outs)
                        traversed[s] = true;
                    transitions_remaining.at(symbol_tag).erase(in);
                    changed = true;
                }
            }
            if (transitions_remaining.at(symbol_tag).empty())
                transitions_remaining.erase(symbol_tag);
        }
    } while(changed);
    for (const auto &t : transitions_remaining) {
        const auto &symbol_tag = t.first;
        for (const auto &in_out : t.second)
            transitions.at(symbol_tag).erase(in_out.first);
        if (transitions.at(symbol_tag).empty())
            transitions.erase(symbol_tag);
    }

    /******************
     * Part 2: Top-Down
     ******************/
    if (!only_bottom_up) {
        traversed = std::vector<bool>(stateNum, false);
        for (const auto &s : finalStates)
            traversed[s] = true;
        transitions_remaining = transitions;
        do {
            changed = false;
            transitions_mother = transitions_remaining;
            for (const auto &t : transitions_mother) {
                const auto &symbol_tag = t.first;
                for (const auto &in_out : t.second) {
                    const auto &in = in_out.first;
                    const auto &outs = in_out.second;
                    for (const auto &s : outs) {
                        if (traversed[s]) {
                            for (const auto &v : in)
                                traversed[v] = true;
                            transitions_remaining.at(symbol_tag).at(in).erase(s);
                            changed = true;
                        }
                    }
                    if (transitions_remaining.at(symbol_tag).at(in).empty())
                        transitions_remaining.at(symbol_tag).erase(in);
                }
                if (transitions_remaining.at(symbol_tag).empty())
                    transitions_remaining.erase(symbol_tag);
            }
        } while(changed);
        for (const auto &t : transitions_remaining) {
            const auto &symbol_tag = t.first;
            for (const auto &in_out : t.second) {
                for (const auto &s : in_out.second)
                    transitions.at(symbol_tag).at(in_out.first).erase(s);
                if (transitions.at(symbol_tag).at(in_out.first).empty())
                    transitions.at(symbol_tag).erase(in_out.first);
            }
            if (transitions.at(symbol_tag).empty())
                transitions.erase(symbol_tag);
        }
    }
    /*********************
     * Part 3: Renumbering
     *********************/
    TransitionMap transitions_new;
    std::map<State, State> stateOldToNew;
    for (const auto &t : transitions) {
        for (const auto &in_out : t.second) {
            const auto &outs = in_out.second;
            StateVector sv;
            for (const auto &v : in_out.first) { // we construct the new tuple
                State newState = stateOldToNew.size();
                auto itBoolPair = stateOldToNew.insert({v, newState});
                if (!itBoolPair.second) { // if insertion didn't happened
                    const auto& it = itBoolPair.first;
                    newState = it->second;
                }
                sv.push_back(newState);
            }
            for (const auto &s : outs) {
                State newState = stateOldToNew.size();
                auto itBoolPair = stateOldToNew.insert({s, newState});
                if (!itBoolPair.second) { // if insertion didn't happened
                    const auto& it = itBoolPair.first;
                    newState = it->second;
                }
                transitions_new[t.first][sv].insert(newState);
            }
        }
    }
    transitions = transitions_new;
    StateVector finalStates_new; // TODO: Can we set the initial capacity?
    finalStates_new.reserve(finalStates.size());
    for (const auto &s : finalStates) {
        auto it = stateOldToNew.find(s);
        if (it != stateOldToNew.end()) {
            finalStates_new.push_back(it->second);
        }
        // We do not add the untouched final states here, since
        // it could severely degrade the performance (either with or without sim_reduce()).
    }
    finalStates = finalStates_new;
    stateNum = stateOldToNew.size();
    auto duration = std::chrono::steady_clock::now() - start;
    if (opLog) std::cout << __FUNCTION__ << "：" << stateNum << " states " << count_transitions() << " transitions\n";
}

template <typename Symbol>
void AUTOQ::Automata<Symbol>::omega_multiplication(int rotation) {
    TransitionMap transitions_new;
    for (const auto &t_old : transitions) {
        const SymbolTag &symbol_tag = t_old.first;
        if (symbol_tag.is_leaf()) {
            SymbolTag s = symbol_tag;
            /************************** rotation **************************/
            s.symbol().omega_multiplication(rotation);
            transitions_new[s] = t_old.second;
        } else {
            assert(symbol_tag.tag().size() <= 1);
            transitions_new.insert(t_old);
        }
    }
    transitions = transitions_new;
    // DO NOT reduce here.
    if (opLog) std::cout << __FUNCTION__ << "：" << stateNum << " states " << count_transitions() << " transitions\n";
}

template <typename Symbol>
void AUTOQ::Automata<Symbol>::divide_by_the_square_root_of_two() {
    std::vector<SymbolTag> to_be_removed;
    TransitionMap to_be_inserted;
    for (const auto &t : transitions) {
        const SymbolTag &symbol_tag = t.first;
        if (symbol_tag.is_leaf()) {
            to_be_removed.push_back(symbol_tag);
            SymbolTag s = symbol_tag;
            s.symbol().divide_by_the_square_root_of_two();
            to_be_inserted[s] = t.second;
        }
    }
    for (const auto &t : to_be_removed)
        transitions.erase(t);
    for (const auto &t : to_be_inserted)
        transitions.insert(t);
    // fraction_simplification();
    if (opLog) std::cout << __FUNCTION__ << "：" << stateNum << " states " << count_transitions() << " transitions\n";
}

template <typename Symbol>
void AUTOQ::Automata<Symbol>::branch_restriction(int k, bool positive_has_value) {
    auto start = std::chrono::steady_clock::now();
    State num_of_states = stateNum;
    if (stateNum > std::numeric_limits<State>::max() / 2)
        throw std::overflow_error("[ERROR] The number of states is too large.");
    stateNum *= 2;

    TransitionMap transitions_copy = transitions;
    for (const auto &t : transitions_copy) {
        const SymbolTag &symbol_tag = t.first;
        if (symbol_tag.is_internal()) { // x_i + determinized number
            auto &in_outs_dest = transitions.at(symbol_tag);
            for (const auto &in_out : t.second) {
                StateVector in;
                assert(in_out.first.size() == 2);
                for (unsigned i=0; i<in_out.first.size(); i++)
                    in.push_back(in_out.first[i] + num_of_states);
                StateSet out;
                for (const auto &n : in_out.second)
                    out.insert(n + num_of_states);
                in_outs_dest.insert(make_pair(in, out)); // duplicate this internal transition
            }
        } else { // (a,b,c,d,k)
            assert(symbol_tag.is_leaf());
            for (const auto &in_out : t.second) {
                assert(in_out.first.empty());
                for (const auto &n : in_out.second) { // Note we do not change k.
                    SymbolTag s = symbol_tag;
                    s.symbol().back_to_zero();
                    transitions[s][{}].insert(n + num_of_states); // duplicate this leaf transition
                }
            }
        }
    }

    transitions_copy = transitions;
    for (const auto &t : transitions_copy) {
        const SymbolTag &symbol_tag = t.first;
        if (symbol_tag.is_internal() && symbol_tag.symbol().qubit() == k) { // x_i + determinized number
            auto &in_outs_dest = transitions.at(symbol_tag);
            for (const auto &in_out : t.second) {
                assert(in_out.first.size() == 2);
                StateVector sv = in_out.first;
                if (in_out.first[0] < num_of_states && in_out.first[1] < num_of_states) {
                    if (positive_has_value) {
                        sv[0] = in_out.first[0] + num_of_states;
                    } else {
                        sv[1] = in_out.first[1] + num_of_states;
                    }
                    in_outs_dest.erase(in_out.first);
                    in_outs_dest.insert(make_pair(sv, in_out.second));
                }
            }
        }
    }
    remove_useless(); // otherwise, will out of memory
    auto end = std::chrono::steady_clock::now();
    branch_rest_time += end - start;
    if (opLog) std::cout << __FUNCTION__ << "：" << stateNum << " states " << count_transitions() << " transitions\n";
}

template <typename Symbol>
void AUTOQ::Automata<Symbol>::semi_determinize() {
    if (isTopdownDeterministic) return;
    TransitionMap transitions_copy = transitions;
    for (const auto &t : transitions_copy) {
        const SymbolTag &symbol_tag = t.first;
        if (symbol_tag.is_internal()) { // x_i not determinized yet
            assert(!symbol_tag.is_tagged()); // not determinized yet
            transitions.erase(symbol_tag); // modify
            int counter = 0;
            SymbolTag new_symbol;
            new_symbol.symbol() = symbol_tag.symbol();
            for (const auto &in_out : t.second) {
                new_symbol.tag().push_back(counter++);
                std::map<StateVector, StateSet> value;
                value.insert(in_out);
                transitions.insert(std::make_pair(new_symbol, value)); // modify
                new_symbol.tag().pop_back();
            }
        }
    }
    // DO NOT reduce here.
    if (opLog) std::cout << __FUNCTION__ << "：" << stateNum << " states " << count_transitions() << " transitions\n";
}

template <typename Symbol>
void AUTOQ::Automata<Symbol>::semi_undeterminize() {
    if (isTopdownDeterministic) return;
    TransitionMap transitions_copy = transitions;
    for (const auto &t : transitions_copy) {
        const SymbolTag &symbol_tag = t.first;
        if (symbol_tag.is_internal()) { // pick all determinized x_i's
            assert(symbol_tag.is_tagged()); // determinized
            transitions.erase(symbol_tag); // modify
            for (const auto &in_out : t.second) {
                for (const auto &v : in_out.second)
                    transitions[symbol_tag.symbol()][in_out.first].insert(v); // modify
            }
        }
    }
    this->reduce();
    if (opLog) std::cout << __FUNCTION__ << "：" << stateNum << " states " << count_transitions() << " transitions\n";
}

#define construct_product_state_id(a, b, i) \
    State i; \
    if (overflow) { \
        auto it = stateOldToNew.find(std::make_pair(a, b)); \
        if (it == stateOldToNew.end()) { \
            i = stateOldToNew.size(); \
            stateOldToNew[std::make_pair(a, b)] = i; \
        } \
        else i = it->second; \
    } else i = a * o.stateNum + b;
template <typename Symbol>
AUTOQ::Automata<Symbol> AUTOQ::Automata<Symbol>::binary_operation(const Automata<Symbol> &o, bool add) {
    auto start = std::chrono::steady_clock::now();
    AUTOQ::Automata<Symbol> result;
    result.name = name;
    result.qubitNum = qubitNum;
    result.isTopdownDeterministic = isTopdownDeterministic; // IMPORTANT: Avoid missing copying new fields afterwards.

    std::map<std::pair<State, State>, State> stateOldToNew; // used only if overflow := true;
    bool overflow = (stateNum > std::numeric_limits<State>::max() / o.stateNum);
    if (!overflow)
        result.finalStates.reserve(finalStates.size() * o.finalStates.size()); // TODO: Can we set the initial capacity?
    else
        throw std::overflow_error("[ERROR] The number of states after multiplication is too large.");

    for (const auto &fs1 : finalStates)
        for (const auto &fs2 : o.finalStates) {
            construct_product_state_id(fs1, fs2, i);
            result.finalStates.push_back(i);
        }

    // We assume here transitions are ordered by symbols.
    // x_i are placed in the beginning, and leaves are placed in the end.
    // This traversal method is due to efficiency.
    std::vector<bool> previous_level_states2(stateNum * o.stateNum);
    std::vector<bool> previous_level_states(stateNum * o.stateNum);
    for (auto s : result.finalStates)
        previous_level_states[s] = true;
    std::vector<bool> next_level_states;
    auto it = transitions.begin();
    auto it2 = o.transitions.begin();
    for (; it != transitions.end(); it++) { // iterate over all internal transitions of T1
        if (it->first.is_leaf()) break; // internal
        if (it->first < it2->first) continue;
        while (it2 != o.transitions.end() && it->first > it2->first)
            it2++;
        if (it2 == o.transitions.end()) break;
        if (it->first < it2->first) continue;
        assert(it->first == it2->first); // Ensure T1's and T2's current transitions have the same symbol now.
        // Update previous_level_states.
        if (it != transitions.begin() && it->first.symbol().qubit() != std::prev(it)->first.symbol().qubit()) { // T1 changes level.
            previous_level_states = previous_level_states2;
            previous_level_states2 = std::vector<bool>(stateNum * o.stateNum);
        }
        // Update next_level_states.
        if (it == transitions.begin() || it->first.symbol().qubit() != std::prev(it)->first.symbol().qubit()) { // T1 goes into the new level.
            next_level_states = std::vector<bool>(stateNum * o.stateNum);
            auto it3 = it; // it3 indicates the next level of it.
            while (it3 != transitions.end() && it3->first.is_internal() && it3->first.symbol().qubit() == it->first.symbol().qubit())
                it3++;
            if (it3 == transitions.end()) {} // T1 has no leaf transitions?
            else if (it3->first.is_leaf()) { // The next level of T1 is leaf transitions.
                auto it4 = it2; // Initially it2 has the same symbol as it.
                while (it4 != o.transitions.end() && it4->first.is_internal())
                    it4++;
                auto it4i = it4;
                // We hope it4 currently points to the first leaf transition.
                // If it4 points to o.transitions.end(), then the following loop will not be executed.
                for (; it3 != transitions.end(); it3++) { // iterate over all leaf transitions of T1
                    assert(it3->first.is_leaf());
                    for (it4 = it4i; it4 != o.transitions.end(); it4++) { // iterate over all leaf transitions of T2
                        assert(it4->first.is_leaf());
                        for (const auto &s1 : it3->second.begin()->second) { // iterate over all output states of it3
                            for (const auto &s2 : it4->second.begin()->second) { // iterate over all output states of it4
                                construct_product_state_id(s1, s2, i);
                                next_level_states[i] = true; // collect all output state products of the next level
                            }
                        }
                    }
                }
            } else { // The next level of T1 is still internal transitions.
                int current_level = static_cast<int>(it3->first.symbol().qubit());
                auto it4 = it2; // Initially it2 has the same symbol as it.
                while (it4 != o.transitions.end() && it4->first.is_internal() && it4->first.symbol().qubit() == current_level)
                    it4++;
                // We hope it4 currently points to T2's first transition of the next level.
                // If it4 points to o.transitions.end(), then the following loop will not be executed.
                for (; it3->first.is_internal() && it3->first.symbol().qubit() == current_level; it3++) {
                    if (it3->first < it4->first) continue;
                    while (it4 != o.transitions.end() && it3->first > it4->first)
                        it4++;
                    if (it4 == o.transitions.end()) break;
                    if (it3->first < it4->first) continue;
                    assert(it3->first == it4->first);
                    // Ensure T1's and T2's current transitions have the same symbol now.
                    for (auto itt = it3->second.begin(); itt != it3->second.end(); itt++) { // all input-output pairs of it3
                        for (auto itt2 = it4->second.begin(); itt2 != it4->second.end(); itt2++) { // all input-output pairs of it4
                            for (const auto &s1 : itt->second) { // all output states of it3
                                for (const auto &s2 : itt2->second) { // all output states of it4
                                    construct_product_state_id(s1, s2, i);
                                    next_level_states[i] = true; // collect all output state products of the next level
                                }
                            }
                        }
                    }
                }
            }
        }
        std::map<StateVector, StateSet> m;
        for (auto itt = it->second.begin(); itt != it->second.end(); itt++) { // iterate over all input-output pairs of it
            for (auto itt2 = it2->second.begin(); itt2 != it2->second.end(); itt2++) { // iterate over all input-output pairs of it2
                StateVector sv;
                StateSet ss;
                construct_product_state_id(itt->first[0], itt2->first[0], i);
                if (!next_level_states[i]) continue;
                sv.push_back(i); // construct product of T1's and T2's left input states
                construct_product_state_id(itt->first[1], itt2->first[1], j);
                if (!next_level_states[j]) continue;
                sv.push_back(j); // construct product of T1's and T2's right input states
                for (const auto &s1 : itt->second) { // all output states of itt
                    for (const auto &s2 : itt2->second) { // all output states of itt2
                        construct_product_state_id(s1, s2, i);
                        if (previous_level_states[i])
                            ss.insert(i);
                    }
                }
                if (ss.size() > 0) {
                    previous_level_states2[sv[0]] = true;
                    previous_level_states2[sv[1]] = true;
                    m.insert(std::make_pair(sv, ss));
                }
            }
        }
        result.transitions.insert(make_pair(it->first, m));
        assert(m.begin()->first.size() == 2);
    }
    previous_level_states = previous_level_states2;
    // We now advance it2 to T2's first leaf transition.
    while (it2 != o.transitions.end() && !it2->first.is_leaf())
        it2++;
    for (; it != transitions.end(); it++) {
        assert(it->first.is_leaf());
        for (auto it2t = it2; it2t != o.transitions.end(); it2t++) { // it2 as the new begin point.
            assert(it2t->first.is_leaf());
            Symbol symbol;
            if (add) symbol = it->first.symbol() + it2t->first.symbol();
            else symbol = it->first.symbol() - it2t->first.symbol();
            for (const auto &s1 : it->second.begin()->second)
                for (const auto &s2 : it2t->second.begin()->second) {
                    construct_product_state_id(s1, s2, i);
                    if (previous_level_states[i]) {
                        assert(it->first.tag() == it2t->first.tag()); // untagged
                        result.transitions[{symbol, it->first.tag()}][{}].insert(i);
                    }
                }
        }
    }
    if (overflow)
        result.stateNum = stateOldToNew.size();
    else
        result.stateNum = stateNum * o.stateNum;
    result.remove_useless(true); // otherwise, will out of memory
    // Round several approximately equal floating points to the same value!
    #if COMPLEX != 1
        result.fraction_simplification();
    #endif
    auto end = std::chrono::steady_clock::now();
    binop_time += end - start;
    if (opLog) std::cout << __FUNCTION__ << "：" << stateNum << " states " << count_transitions() << " transitions\n";
    return result;
}
template <typename Symbol>
AUTOQ::Automata<Symbol> Automata<Symbol>::operator+(const Automata &o) {
    return binary_operation(o, true);
}
template <typename Symbol>
AUTOQ::Automata<Symbol> Automata<Symbol>::operator-(const Automata &o) {
    return binary_operation(o, false);
}

template <>
AUTOQ::TreeAutomata AUTOQ::TreeAutomata::uniform(int n) {
    TreeAutomata aut;
    aut.name = "Uniform";
    aut.qubitNum = n;
    int pow_of_two = 1;
    State state_counter = 0;
    for (int level=1; level<=n; level++) {
        for (int i=0; i<pow_of_two; i++) {
            if (level < n)
                aut.transitions[{level}][{state_counter*2+1, state_counter*2+2}] = {state_counter};
            else
                aut.transitions[{level}][{pow_of_two*2-1, pow_of_two*2-1}].insert(state_counter);
            state_counter++;
        }
        pow_of_two *= 2;
    }
    aut.transitions[Concrete(Complex::Complex::One().divide_by_the_square_root_of_two(n))][{}] = {pow_of_two-1};
    aut.finalStates.push_back(0);
    aut.stateNum = pow_of_two;

    // aut.minimize();
    // aut.isTopdownDeterministic = true;
    return aut;
}

template <>
AUTOQ::TreeAutomata AUTOQ::TreeAutomata::basis(int n) {
    TreeAutomata aut;
    aut.name = "Classical";
    aut.qubitNum = n;

    for (int level=1; level<=n; level++) {
        if (level >= 2)
            aut.transitions[{level}][{2*level - 1, 2*level - 1}] = {2*level - 3};
        aut.transitions[{level}][{2*level - 1, 2*level}] = {2*level - 2};
        aut.transitions[{level}][{2*level, 2*level - 1}] = {2*level - 2};
    }
    aut.transitions[Concrete(Complex::Complex::One())][{}] = {2*n};
    aut.transitions[Concrete(Complex::Complex::Zero())][{}] = {2*n - 1};
    aut.finalStates.push_back(0);
    aut.stateNum = 2*n + 1;

    // aut.minimize();
    return aut;
}

template <>
AUTOQ::TreeAutomata AUTOQ::TreeAutomata::random(int n) {
    TreeAutomata aut;
    aut.name = "Random";
    aut.qubitNum = n;
    int pow_of_two = 1;
    State state_counter = 0;
    for (int level=1; level<=n; level++) {
        for (int i=0; i<pow_of_two; i++) {
            aut.transitions[{level}][{state_counter*2+1, state_counter*2+2}] = {state_counter};
            state_counter++;
        }
        pow_of_two *= 2;
    }
    for (State i=state_counter; i<=state_counter*2; i++) {
        aut.transitions[Concrete(Complex::Complex::Rand())][{}].insert(i);
    }
    aut.finalStates.push_back(0);
    aut.stateNum = state_counter*2 + 1;

    // aut.minimize();
    // aut.isTopdownDeterministic = true;
    return aut;
}

template <>
AUTOQ::TreeAutomata AUTOQ::TreeAutomata::zero(int n) {
    /* Example of n = 6:
        Final States 0
        Transitions
        [1](2, 1) -> 0
        [2](3, 3) -> 1
        [2](4, 3) -> 2
        [3](5, 5) -> 3
        [3](6, 5) -> 4
        [4](7, 7) -> 5
        [4](8, 7) -> 6
        [5](9, 9) -> 7
        [5](10, 9) -> 8
        [6](11, 11) -> 9
        [6](12, 11) -> 10
        [0,0,0,0,0] -> 11
        [1,0,0,0,0] -> 12
    */
    TreeAutomata aut;
    aut.name = "Zero";
    aut.qubitNum = n;
    aut.finalStates.push_back(0);
    aut.transitions[{1}][{2,1}] = {0};
    for (int level=2; level<=n; level++) {
        aut.transitions[{level}][{level*2-1, level*2-1}] = {level*2-3};
        aut.transitions[{level}][{level*2, level*2-1}] = {level*2-2};
    }
    aut.transitions[Concrete(Complex::Complex::Zero())][{}].insert(n*2-1);
    aut.transitions[Concrete(Complex::Complex::One())][{}].insert(n*2);
    aut.stateNum = n*2 + 1;

    // aut.minimize();
    // aut.isTopdownDeterministic = true;
    return aut;
}

template <>
AUTOQ::TreeAutomata AUTOQ::TreeAutomata::basis_zero_one_zero(int n) {
    TreeAutomata aut;
    assert(n >= 2);
    aut.name = "Classical_Zero_One_Zero";
    aut.qubitNum = n + (n+1) + (n>=3) * (n-1);

    for (int level=1; level<=n; level++) {
        if (level >= 2)
            aut.transitions[{level}][{2*level - 1, 2*level - 1}] = {2*level - 3};
        aut.transitions[{level}][{2*level - 1, 2*level}] = {2*level - 2};
        aut.transitions[{level}][{2*level, 2*level - 1}] = {2*level - 2};
    }
    for (int level=1; level<=n; level++) {
        aut.transitions[{n + level}][{2*n + 2*level-1, 2*n + 2*level-1}] = {2*n + 2*level-3};
        aut.transitions[{n + level}][{2*n + 2*level, 2*n + 2*level-1}] = {2*n + 2*level-2};
    }
    aut.transitions[{n + (n+1)}][{2*n + 2*(n+1)-1, 2*n + 2*(n+1)-1}] = {2*n + 2*(n+1)-3};
    aut.transitions[{n + (n+1)}][{2*n + 2*(n+1)-1, 2*n + 2*(n+1)}] = {2*n + 2*(n+1)-2};
    if (n >= 3) {
        for (int level=n+2; level<=2*n; level++) {
            aut.transitions[{n + level}][{2*n + 2*level-1, 2*n + 2*level-1}] = {2*n + 2*level-3};
            aut.transitions[{n + level}][{2*n + 2*level, 2*n + 2*level-1}] = {2*n + 2*level-2};
        }
        aut.transitions[Concrete(Complex::Complex::One())][{}] = {6*n};
        aut.transitions[Concrete(Complex::Complex::Zero())][{}] = {6*n - 1};
        aut.stateNum = 6*n + 1;
    } else {
        assert(n == 2);
        aut.transitions[Concrete(Complex::Complex::One())][{}] = {4*n + 2};
        aut.transitions[Concrete(Complex::Complex::Zero())][{}] = {4*n + 1};
        aut.stateNum = 4*n + 3;
    }
	aut.finalStates.push_back(0);
    return aut;
}

template <>
AUTOQ::TreeAutomata AUTOQ::TreeAutomata::zero_zero_one_zero(int n) {
    TreeAutomata aut;
    assert(n >= 2);
    aut.name = "Zero_Zero_One_Zero";
    aut.qubitNum = n + (n+1) + (n>=3) * (n-1);

    aut.transitions[{1}][{2,1}] = {0};
    for (int level=2; level<=n; level++) {
        aut.transitions[{level}][{level*2-1, level*2-1}] = {level*2-3};
        aut.transitions[{level}][{level*2, level*2-1}] = {level*2-2};
    }
    for (int level=1; level<=n; level++) {
        aut.transitions[{n + level}][{2*n + 2*level-1, 2*n + 2*level-1}] = {2*n + 2*level-3};
        aut.transitions[{n + level}][{2*n + 2*level, 2*n + 2*level-1}] = {2*n + 2*level-2};
    }
    aut.transitions[{n + (n+1)}][{2*n + 2*(n+1)-1, 2*n + 2*(n+1)-1}] = {2*n + 2*(n+1)-3};
    aut.transitions[{n + (n+1)}][{2*n + 2*(n+1)-1, 2*n + 2*(n+1)}] = {2*n + 2*(n+1)-2};
    if (n >= 3) {
        for (int level=n+2; level<=2*n; level++) {
            aut.transitions[{n + level}][{2*n + 2*level-1, 2*n + 2*level-1}] = {2*n + 2*level-3};
            aut.transitions[{n + level}][{2*n + 2*level, 2*n + 2*level-1}] = {2*n + 2*level-2};
        }
        aut.transitions[Concrete(Complex::Complex::One())][{}] = {6*n};
        aut.transitions[Concrete(Complex::Complex::Zero())][{}] = {6*n - 1};
        aut.stateNum = 6*n + 1;
    } else {
        assert(n == 2);
        aut.transitions[Concrete(Complex::Complex::One())][{}] = {4*n + 2};
        aut.transitions[Concrete(Complex::Complex::Zero())][{}] = {4*n + 1};
        aut.stateNum = 4*n + 3;
    }
	aut.finalStates.push_back(0);
    // aut.isTopdownDeterministic = true;
    return aut;
}

template <>
AUTOQ::TreeAutomata AUTOQ::TreeAutomata::zero_one_zero(int n) {
    TreeAutomata aut;
    assert(n >= 2);
    aut.name = "Zero_One_Zero";
    aut.qubitNum = (n+1) + (n>=3) * (n-1);

    aut.transitions[{1}][{2,1}] = {0};
    for (int level=2; level<=n; level++) {
        aut.transitions[{level}][{level*2-1, level*2-1}] = {level*2-3};
        aut.transitions[{level}][{level*2, level*2-1}] = {level*2-2};
    }
    aut.transitions[{n+1}][{2*(n+1)-1, 2*(n+1)-1}] = {2*n-1};
    aut.transitions[{n+1}][{2*(n+1)-1, 2*(n+1)}] = {2*n};
    if (n >= 3) {
        for (int level=n+2; level<=2*n; level++) {
            aut.transitions[{level}][{2*level-1, 2*level-1}] = {2*(level-1)-1};
            aut.transitions[{level}][{2*level, 2*level-1}] = {2*(level-1)};
        }
        aut.transitions[Concrete(Complex::Complex::One())][{}] = {4*n};
        aut.transitions[Concrete(Complex::Complex::Zero())][{}] = {4*n - 1};
        aut.stateNum = 4*n + 1;
    } else {
        assert(n == 2);
        aut.transitions[Concrete(Complex::Complex::One())][{}] = {2*n + 2};
        aut.transitions[Concrete(Complex::Complex::Zero())][{}] = {2*n + 1};
        aut.stateNum = 2*n + 3;
    }
	aut.finalStates.push_back(0);
    // aut.isTopdownDeterministic = true;
    return aut;
}

template <typename Symbol>
void AUTOQ::Automata<Symbol>::swap_forward(const int k) {
    if (isTopdownDeterministic) return;
    for (unsigned next_k=k+1; next_k<=qubitNum; next_k++) {
        std::map<State, std::vector<std::pair<SymbolTag, StateVector>>> svsv;
        for (const auto &t : transitions) {
            const auto &symbol_tag = t.first;
            const auto &in_outs = t.second;
            if (symbol_tag.is_internal() && symbol_tag.symbol().qubit() == next_k) {
                assert(symbol_tag.tag().size() <= 1);
                for (const auto &in_out : in_outs) {
                    for (const auto &s : in_out.second)
                        svsv[s].push_back(make_pair(symbol_tag, in_out.first));
                }
            }
        }
        std::vector<SymbolTag> to_be_removed2;
        TransitionMap to_be_removed, to_be_inserted;
        for (const auto &t : transitions) {
            const SymbolTag &symbol_tag = t.first;
            if (symbol_tag.is_internal() && symbol_tag.symbol().qubit() == k) {
                for (const auto &in_out : t.second) {
                    assert(in_out.first.size() == 2);
                    for (const auto &ssv1 : svsv[in_out.first[0]]) {
                        for (const auto &ssv2 : svsv[in_out.first[1]]) {
                            to_be_removed[ssv1.first][ssv1.second].insert(in_out.first[0]);
                            to_be_removed[ssv2.first][ssv2.second].insert(in_out.first[1]);
                            if (to_be_inserted[symbol_tag][{ssv1.second[0], ssv2.second[0]}].empty()) {
                                if (transitions[symbol_tag][{ssv1.second[0], ssv2.second[0]}].empty())
                                    to_be_inserted[symbol_tag][{ssv1.second[0], ssv2.second[0]}].insert(stateNum++);
                                else
                                    to_be_inserted[symbol_tag][{ssv1.second[0], ssv2.second[0]}].insert(*(transitions[symbol_tag][{ssv1.second[0], ssv2.second[0]}].begin()));
                            }
                            if (to_be_inserted[symbol_tag][{ssv1.second[1], ssv2.second[1]}].empty()) {
                                if (transitions[symbol_tag][{ssv1.second[1], ssv2.second[1]}].empty())
                                    to_be_inserted[symbol_tag][{ssv1.second[1], ssv2.second[1]}].insert(stateNum++);
                                else
                                    to_be_inserted[symbol_tag][{ssv1.second[1], ssv2.second[1]}].insert(*(transitions[symbol_tag][{ssv1.second[1], ssv2.second[1]}].begin()));
                            }
                            for (const auto &s : in_out.second)
                                to_be_inserted[{Symbol(next_k), {ssv1.first.tag(0), ssv2.first.tag(0)}}][{*(to_be_inserted[symbol_tag][{ssv1.second[0], ssv2.second[0]}].begin()), *(to_be_inserted[symbol_tag][{ssv1.second[1], ssv2.second[1]}].begin())}].insert(s);
                        }
                    }
                }
                to_be_removed2.push_back(symbol_tag);
            }
        }
        for (const auto &v : to_be_removed2)
            transitions.erase(v);
        for (const auto &t : to_be_removed) {
            const SymbolTag &symbol_tag = t.first;
            for (const auto &in_out : t.second) {
                for (const auto &s : in_out.second)
                    transitions[symbol_tag][in_out.first].erase(s);
                if (transitions[symbol_tag][in_out.first].empty())
                    transitions[symbol_tag].erase(in_out.first);
                if (transitions[symbol_tag].empty())
                    transitions.erase(symbol_tag);
            }
        }
        for (const auto &t : to_be_inserted) {
            const SymbolTag &symbol_tag = t.first;
            for (const auto &in_out : t.second) {
                for (const auto &s : in_out.second) {
                    transitions[symbol_tag][in_out.first].insert(s);
                }
            }
        }
        // DO NOT reduce here.
    }
    if (opLog) std::cout << __FUNCTION__ << "：" << stateNum << " states " << count_transitions() << " transitions\n";
}

template <typename Symbol>
void AUTOQ::Automata<Symbol>::swap_backward(const int k) {
    if (isTopdownDeterministic) return;
    for (int next_k=qubitNum; next_k>k; next_k--) {
      std::map<State, std::vector<std::pair<SymbolTag, StateVector>>> svsv;
        for (const auto &t : transitions) {
            const auto &symbol_tag = t.first;
            const auto &in_outs = t.second;
            if (symbol_tag.is_internal() && symbol_tag.symbol().qubit() == k) {
                assert(symbol_tag.tag().size() == 1);
                for (const auto &in_out : in_outs) {
                    for (const auto &s : in_out.second)
                        svsv[s].push_back(make_pair(symbol_tag, in_out.first));
                }
            }
        }
        std::vector<SymbolTag> to_be_removed2;
        TransitionMap to_be_removed, to_be_inserted;
        for (const auto &t : transitions) {
            const SymbolTag &symbol_tag = t.first;
            if (symbol_tag.is_internal() && symbol_tag.symbol().qubit() == next_k) {
                assert(symbol_tag.tag().size() == 2);
                for (const auto &in_out : t.second) {
                    assert(in_out.first.size() == 2);
                    for (const auto &ssv1 : svsv[in_out.first[0]]) {
                        for (const auto &ssv2 : svsv[in_out.first[1]]) {
                            if (ssv1.first == ssv2.first) {
                                to_be_removed[ssv1.first][ssv1.second].insert(in_out.first[0]);
                                to_be_removed[ssv2.first][ssv2.second].insert(in_out.first[1]);
                                SymbolTag t1 = {symbol_tag.symbol(), {symbol_tag.tag(0)}};
                                if (to_be_inserted[t1][{ssv1.second[0], ssv2.second[0]}].empty()) {
                                    if (transitions[t1][{ssv1.second[0], ssv2.second[0]}].empty())
                                        to_be_inserted[t1][{ssv1.second[0], ssv2.second[0]}].insert(stateNum++);
                                    else
                                        to_be_inserted[t1][{ssv1.second[0], ssv2.second[0]}].insert(*(transitions[t1][{ssv1.second[0], ssv2.second[0]}].begin()));
                                }
                                SymbolTag t2 = {symbol_tag.symbol(), {symbol_tag.tag(1)}};
                                if (to_be_inserted[t2][{ssv1.second[1], ssv2.second[1]}].empty()) {
                                    if (transitions[t2][{ssv1.second[1], ssv2.second[1]}].empty())
                                        to_be_inserted[t2][{ssv1.second[1], ssv2.second[1]}].insert(stateNum++);
                                    else
                                        to_be_inserted[t2][{ssv1.second[1], ssv2.second[1]}].insert(*(transitions[t2][{ssv1.second[1], ssv2.second[1]}].begin()));
                                }
                                assert(k == ssv1.first.symbol().qubit());
                                for (const auto &s : in_out.second)
                                    to_be_inserted[ssv1.first][{*(to_be_inserted[t1][{ssv1.second[0], ssv2.second[0]}].begin()), *(to_be_inserted[t2][{ssv1.second[1], ssv2.second[1]}].begin())}].insert(s);
                            }
                        }
                    }
                }
                to_be_removed2.push_back(symbol_tag);
            }
        }
        for (const auto &v : to_be_removed2)
            transitions.erase(v);
        for (const auto &t : to_be_removed) {
            const SymbolTag &symbol_tag = t.first;
            for (const auto &in_out : t.second) {
                for (const auto &s : in_out.second)
                    transitions[symbol_tag][in_out.first].erase(s);
                if (transitions[symbol_tag][in_out.first].empty())
                    transitions[symbol_tag].erase(in_out.first);
                if (transitions[symbol_tag].empty())
                    transitions.erase(symbol_tag);
            }
        }
        for (const auto &t : to_be_inserted) {
            const SymbolTag &symbol_tag = t.first;
            for (const auto &in_out : t.second) {
                for (const auto &s : in_out.second) {
                    transitions[symbol_tag][in_out.first].insert(s);
                }
            }
        }
        // DO NOT reduce here.
    }
    if (opLog) std::cout << __FUNCTION__ << "：" << stateNum << " states " << count_transitions() << " transitions\n";
}

template <typename Symbol>
void AUTOQ::Automata<Symbol>::value_restriction(int k, bool branch) {
    auto start = std::chrono::steady_clock::now();
    swap_forward(k);
    TransitionMap to_be_inserted;
    std::vector<SymbolTag> to_be_removed;
    for (const auto &t : transitions) {
        const SymbolTag &symbol_tag = t.first;
        if (symbol_tag.is_internal() && symbol_tag.symbol().qubit() == k) {
            to_be_removed.push_back(symbol_tag);
            for (const auto &in_out : t.second) {
                assert(in_out.first.size() == 2);
                for (const auto &s : in_out.second) {
                    if (branch)
                        to_be_inserted[symbol_tag][{in_out.first[1], in_out.first[1]}].insert(s);
                    else
                        to_be_inserted[symbol_tag][{in_out.first[0], in_out.first[0]}].insert(s);
                }
            }
        }
    }
    for (const auto &t : to_be_removed) {
        transitions.erase(t);
    }
    for (const auto &t : to_be_inserted) {
        transitions[t.first] = t.second;
    }
    swap_backward(k);
    this->reduce();
    auto end = std::chrono::steady_clock::now();
    value_rest_time += end - start;
    if (opLog) std::cout << __FUNCTION__ << "：" << stateNum << " states " << count_transitions() << " transitions\n";
}

template <typename Symbol>
void AUTOQ::Automata<Symbol>::fraction_simplification() {
    std::vector<SymbolTag> to_be_removed;
    TransitionMap to_be_inserted;
    for (const auto &t : transitions) {
        const SymbolTag &s = t.first;
        if (s.is_leaf()) {
            SymbolTag symbol_tag = s;
            symbol_tag.symbol().fraction_simplification();
            if (t.first != symbol_tag) {
                to_be_removed.push_back(t.first);
                for (const auto &in_out : t.second) {
                    for (const auto &s : in_out.second) {
                        to_be_inserted[symbol_tag][in_out.first].insert(s);
                    }
                }
            }
        }
    }
    for (const auto &t : to_be_removed) transitions.erase(t);
    for (const auto &t : to_be_inserted) {
        for (const auto &kv : t.second)
            for (const auto &s : kv.second)
                transitions[t.first][kv.first].insert(s);
    }
    // remove_useless();
    // reduce();
    if (opLog) std::cout << __FUNCTION__ << "：" << stateNum << " states " << count_transitions() << " transitions\n";
}

/**************** Equivalence Checking ****************/
// namespace
// { // anonymous namespace
  std::string gpath_to_VATA = "";

  /** returns the path to AUTOQ executable */
  const std::string& get_vata_path()
  {
    // is it cached?
    if (!gpath_to_VATA.empty()) return gpath_to_VATA;

    // not cached, get it from ENV
    const char* path = std::getenv("VATA_PATH");
    if (nullptr == path) {
      throw std::runtime_error("[ERROR] The environment variable VATA_PATH is not found!");
    }

    gpath_to_VATA = path;
    return gpath_to_VATA;
  }

  const int MAX_ARG_STRLEN = 131070; // in fact is 131072 on the Internet

  /** checks inclusion of two TAs */
  template <typename Symbol>
  bool AUTOQ::Automata<Symbol>::check_inclusion(const std::string& lhsPath, const std::string& rhsPath)
  {
    std::string aux;
    AUTOQ::Util::ShellCmd(get_vata_path() + " incl " + lhsPath + " " + rhsPath, aux);
    return (aux == "1\n");
  }

  template <typename Symbol>
  bool AUTOQ::Automata<Symbol>::check_inclusion(const Automata<Symbol>& lhsPath, const std::string& rhsPath)
  {
    std::string aux;
    std::string aut1 = TimbukSerializer::Serialize(lhsPath);
    assert(aut1.length() <= MAX_ARG_STRLEN);
    AUTOQ::Util::ShellCmd(get_vata_path() + " incl2 '" + aut1 + "' " + rhsPath, aux);
    return (aux == "1\n");
  }

  template <typename Symbol>
  bool AUTOQ::Automata<Symbol>::check_inclusion(const std::string& lhsPath, const Automata<Symbol>& rhsPath)
  {
    std::string aux;
    std::string aut2 = TimbukSerializer::Serialize(rhsPath);
    assert(aut2.length() <= MAX_ARG_STRLEN);
    AUTOQ::Util::ShellCmd(get_vata_path() + " incl3 " + lhsPath + " '" + aut2 + "'", aux);
    return (aux == "1\n");
  }

  template <typename Symbol>
  bool AUTOQ::Automata<Symbol>::check_inclusion(const Automata<Symbol>& lhsPath, const Automata<Symbol>& rhsPath)
  {
    std::string input = TimbukSerializer::Serialize(lhsPath);
    std::vector<std::string> args{get_vata_path(), "incl4"};
    int length = input.length();
    for (int i=0; i<length; i+=MAX_ARG_STRLEN) {
        args.push_back(input.substr(i, MAX_ARG_STRLEN));
    }
    input = TimbukSerializer::Serialize(rhsPath);
    length = input.length();    
    for (int i=0; i<length; i+=MAX_ARG_STRLEN) {
        args.push_back(input.substr(i, MAX_ARG_STRLEN));
    }
    std::string aux;
    if (!AUTOQ::Util::ShellCmd(args, aux)) {
        throw std::runtime_error("[ERROR] Failed to execute VATA.");
    }
    return (aux == "1\n");
  }

  /** checks language equivalence of two TAs */
  template <typename Symbol>
  bool AUTOQ::Automata<Symbol>::check_equal(const Automata<Symbol>& lhsPath, const Automata<Symbol>& rhsPath)
  {
    return check_inclusion(lhsPath, rhsPath) && check_inclusion(rhsPath, lhsPath);
  }

  template <>
  bool AUTOQ::TreeAutomata::check_equal_aut(
      AUTOQ::TreeAutomata lhs,
      AUTOQ::TreeAutomata rhs)
  {
	return check_equal(lhs, rhs);
  }
// } // anonymous namespace
/******************************************************/

// template <typename Symbol>
// void AUTOQ::Automata<Symbol>::sim_reduce()
// {
//   using State = typename Automata<Symbol>::State;
//   using DiscontBinaryRelOnStates = typename Util::DiscontBinaryRelation<State>;
//   using StateToStateMap = typename std::unordered_map<State, State>;

//   DiscontBinaryRelOnStates sim = compute_down_sim(*this);

//   // TODO: this is probably not optimal, we could probably get the mapping of
//   // states for collapsing in a faster way
//   sim.RestrictToSymmetric();       // sim is now an equivalence relation

//   StateToStateMap collapseMap;
//   sim.GetQuotientProjection(collapseMap);

//   // Automata old = *this;
//   reindex_aut_states(*this, collapseMap);

//   // if (!check_equal_aut(*this, old)) {
//   //   AUTOQ_DEBUG("wrong simulation result!");
//   //   AUTOQ_DEBUG("old: " + old.ToString());
//   //   AUTOQ_DEBUG("new: " + this->ToString());
//   //   AUTOQ_DEBUG("simulation: " + sim.ToString());
//   // }
// }

template <typename Symbol>
void AUTOQ::Automata<Symbol>::execute(const char *filename) {
    std::ifstream qasm(filename);
    const std::regex digit("\\d+");
    const std::regex_iterator<std::string::iterator> END;
    if (!qasm.is_open()) throw std::runtime_error("[ERROR] Failed to open file " + std::string(filename) + ".");
    std::string line;
    while (getline(qasm, line)) {
        if (line.find("OPENQASM") == 0 || line.find("include ") == 0|| line.find("//") == 0) continue;
        if (line.find("qreg ") == 0) {
            std::regex_iterator<std::string::iterator> it(line.begin(), line.end(), digit);
            while (it != END) {
                if (atoi(it->str().c_str()) != static_cast<int>(qubitNum))
                    throw std::runtime_error("[ERROR] The number of qubits in the automaton does not match the number of qubits in the circuit.");
                ++it;
            }
        } else if (line.find("x ") == 0) {
            std::smatch match_pieces;
            if (std::regex_search(line, match_pieces, digit))
                X(1 + atoi(match_pieces[0].str().c_str()));
        } else if (line.find("y ") == 0) {
            std::smatch match_pieces;
            if (std::regex_search(line, match_pieces, digit))
                Y(1 + atoi(match_pieces[0].str().c_str()));
        } else if (line.find("z ") == 0) {
            std::smatch match_pieces;
            if (std::regex_search(line, match_pieces, digit))
                Z(1 + atoi(match_pieces[0].str().c_str()));
        } else if (line.find("h ") == 0) {
            std::smatch match_pieces;
            if (std::regex_search(line, match_pieces, digit))
                H(1 + atoi(match_pieces[0].str().c_str()));
        } else if (line.find("s ") == 0) {
            std::smatch match_pieces;
            if (std::regex_search(line, match_pieces, digit))
                S(1 + atoi(match_pieces[0].str().c_str()));
        } else if (line.find("sdg ") == 0) {
            std::smatch match_pieces;
            if (std::regex_search(line, match_pieces, digit))
                Sdg(1 + atoi(match_pieces[0].str().c_str()));
        } else if (line.find("t ") == 0) {
            std::smatch match_pieces;
            if (std::regex_search(line, match_pieces, digit))
                T(1 + atoi(match_pieces[0].str().c_str()));
        } else if (line.find("tdg ") == 0) {
            std::smatch match_pieces;
            if (std::regex_search(line, match_pieces, digit))
                Tdg(1 + atoi(match_pieces[0].str().c_str()));
        } else if (line.find("rx(pi/2) ") == 0 || line.find("rx(pi / 2)") == 0) {
            std::smatch match_pieces;
            if (std::regex_search(line, match_pieces, digit))
                Rx(1 + atoi(match_pieces[0].str().c_str()));
        } else if (line.find("ry(pi/2) ") == 0 || line.find("ry(pi / 2)") == 0) {
            std::smatch match_pieces;
            if (std::regex_search(line, match_pieces, digit))
                Ry(1 + atoi(match_pieces[0].str().c_str()));
        } else if (line.find("cx ") == 0 || line.find("CX ") == 0 ) {
            std::regex_iterator<std::string::iterator> it(line.begin(), line.end(), digit);
            std::vector<int> pos;
            while (it != END) {
                pos.push_back(1 + atoi(it->str().c_str()));
                ++it;
            }
            CNOT(pos[0], pos[1]);
        } else if (line.find("cz ") == 0) {
            std::regex_iterator<std::string::iterator> it(line.begin(), line.end(), digit);
            std::vector<int> pos;
            while (it != END) {
                pos.push_back(1 + atoi(it->str().c_str()));
                ++it;
            }
            CZ(pos[0], pos[1]);
        } else if (line.find("ccx ") == 0) {
            std::regex_iterator<std::string::iterator> it(line.begin(), line.end(), digit);
            std::vector<int> pos;
            while (it != END) {
                pos.push_back(1 + atoi(it->str().c_str()));
                ++it;
            }
            Toffoli(pos[0], pos[1], pos[2]);
        } else if (line.find("swap ") == 0) {
            std::regex_iterator<std::string::iterator> it(line.begin(), line.end(), digit);
            std::vector<int> pos;
            while (it != END) {
                pos.push_back(1 + atoi(it->str().c_str()));
                ++it;
            }
            swap(pos[0], pos[1]);
        } else if (line.length() > 0)
            throw std::runtime_error("[ERROR] unsupported gate: " + line + ".");
    }
    qasm.close();
}

// std::string exec(const char* cmd) {
//     std::array<char, 128> buffer;
//     std::string result;
//     std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(cmd, "r"), pclose);
//     if (!pipe) {
//         throw std::runtime_error("popen() failed!");
//     }
//     while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
//         result += buffer.data();
//     }
//     return result;
// }
#include <chrono>
using namespace std;
std::string toString2(std::chrono::steady_clock::duration tp) {
    using namespace std;
    using namespace std::chrono;
    nanoseconds ns = duration_cast<nanoseconds>(tp);
    typedef duration<int, ratio<86400>> days;
    std::stringstream ss;
    char fill = ss.fill();
    ss.fill('0');
    auto d = duration_cast<days>(ns);
    ns -= d;
    auto h = duration_cast<hours>(ns);
    ns -= h;
    auto m = duration_cast<minutes>(ns);
    ns -= m;
    auto s = duration_cast<seconds>(ns);
    ns -= s;
    auto ms = duration_cast<milliseconds>(ns);
    // auto s = duration<float, std::ratio<1, 1>>(ns);
    if (d.count() > 0 || h.count() > 0)
        ss << d.count() << 'd' << h.count() << 'h' << m.count() << 'm' << s.count() << 's';
    else if (m.count() == 0 && s.count() < 10) {
        ss << s.count() << '.' << ms.count() / 100 << "s";
    } else {
        if (m.count() > 0) ss << m.count() << 'm';
        ss << s.count() << 's';// << " & ";
    }
    ss.fill(fill);
    return ss.str();
}
bool AUTOQ::check_validity(Constraint C, const PredicateAutomata::Symbol &ps, const SymbolicAutomata::Symbol &te) {
    std::string str(ps);
    auto regToExpr = C.to_exprs(te);
    for (const auto &kv : regToExpr) // example: z3 <(echo '(declare-fun x () Int)(declare-fun z () Int)(assert (= z (+ x 3)))(check-sat)')
        str = std::regex_replace(str, std::regex(kv.first), kv.second);
    // std::cout << std::string(C) + "(assert (not " + str + "))(check-sat)\n";
    std::string smt_input = "bash -c \"z3 <(echo '" + std::string(C) + "(assert (not " + str + "))(check-sat)')\"";
    // auto startSim = chrono::steady_clock::now();
    ShellCmd(smt_input, str);
    // std::cout << smt_input << "\n";
    // std::cout << str << "\n";
    // auto durationSim = chrono::steady_clock::now() - startSim;
    // std::cout << toString2(durationSim) << "\n";
    if (str == "unsat\n") return true;
    else if (str == "sat\n") return false;
    else {
        std::cout << smt_input << "\n";
        std::cout << str << "-\n";
        throw std::runtime_error("[ERROR] The solver Z3 did not correctly return SAT or UNSAT.\nIt's probably because the specification automaton is NOT a predicate automaton.");
    }
}
bool AUTOQ::is_spec_satisfied(const Constraint &C, const SymbolicAutomata &Ae, const PredicateAutomata &As) {
    using State = SymbolicAutomata::State;
    using StateSet = SymbolicAutomata::StateSet;
    using StateVector = SymbolicAutomata::StateVector;
    StateSet As_finalStates(As.finalStates.begin(), As.finalStates.end());
    std::vector<std::pair<State, StateSet>> processed; // ← ∅;
    std::queue<std::pair<State, StateSet>> worklist;
    for (const auto &te : Ae.transitions) {
        if (te.first.is_leaf()) {
            for (const auto &qe: te.second.at({})) {
                StateSet Us;
                for (const auto &ps : As.transitions) {
                    if (ps.first.is_leaf()) {
                        if (check_validity(C, ps.first.symbol(), te.first.symbol())) { // C ⇒ ps(te)
                            for (const auto &us: ps.second.at({})) {
                                Us.insert(us);
                            }
                        }
                    }
                } // compute Us above!
                worklist.push({qe, Us});
            }
        }
    } // antichainize Worklist
    while (!worklist.empty()) {
        auto qeUs = worklist.front();
        worklist.pop();
        if (std::find(processed.begin(), processed.end(), qeUs) == processed.end()) { // not found
            processed.push_back(qeUs);
            for (const auto &te : Ae.transitions) {
                if (te.first.is_internal()) {
                    const auto &alpha = te.first;
                    auto qeUs1 = qeUs;
                    for (auto qeUs2 : processed) {
                        bool flip = false;
                        do {
                            // Assume Ae and As have the same internal symbols!
                            StateSet Hs;
                            for (const auto &in_out : As.transitions.at({AUTOQ::Symbol::Predicate(alpha.symbol().complex.at(Complex::Complex::One()).at("1")), {}})) {
                                assert(in_out.first.size() == 2);
                                if (qeUs1.second.find(in_out.first[0]) != qeUs1.second.end()
                                    && qeUs2.second.find(in_out.first[1]) != qeUs2.second.end()) {
                                        Hs.insert(in_out.second.begin(), in_out.second.end());
                                    }
                            } // compute Hs above!
                            StateVector output;
                            std::set_intersection(Hs.begin(), Hs.end(), As_finalStates.begin(), As_finalStates.end(), std::back_inserter(output));
                            // output.resize(it - output.begin());
                            bool Hs_Rs_no_intersection = output.empty();
                            // check the above boolean value!
                            auto it = te.second.find({qeUs1.first, qeUs2.first});
                            if (it != te.second.end()) {
                                for (const auto &q : it->second) { // He
                                    auto qHs = std::make_pair(q, Hs);
                                    if (std::find(processed.begin(), processed.end(), qHs) == processed.end()) { // not found
                                        if (std::find(Ae.finalStates.begin(), Ae.finalStates.end(), q) != Ae.finalStates.end()
                                            && Hs_Rs_no_intersection) {
                                            return false;
                                        }
                                        worklist.push(qHs);
                                        // antichainize Worklist and Processed
                                    }
                                }
                            }
                            // perform swap
                            std::swap(qeUs1, qeUs2);
                            flip = !flip;
                        } while (flip);
                    }
                }
            }
        }
    }
    return true;
}

template <typename Symbol>
std::vector<std::vector<std::string>> AUTOQ::Automata<Symbol>::print(const std::map<typename AUTOQ::Automata<Symbol>::State, typename AUTOQ::Automata<Symbol>::Symbol> &leafSymbolMap, int qubit, typename AUTOQ::Automata<Symbol>::State state) {
    if (qubit == static_cast<int>(qubitNum + 1)) {
        std::stringstream ss;
        ss << leafSymbolMap.at(state);
        return {{ss.str()}};
    }
    std::vector<std::vector<std::string>> ans;
    for (const auto &in_outs : transitions.at({qubit})) {
        if (in_outs.second.find(state) != in_outs.second.end()) {
            auto v1 = print(leafSymbolMap, qubit + 1, in_outs.first.at(0));
            auto v2 = print(leafSymbolMap, qubit + 1, in_outs.first.at(1));
            for (const auto &s1 : v1) {
                for (const auto &s2 : v2) {
                    auto v = s1;
                    v.insert(v.end(), s2.begin(), s2.end());
                    ans.push_back(v);
                }
            }
        }
    }
    return ans;
}

template <typename Symbol>
void AUTOQ::Automata<Symbol>::print_language() {
    std::map<typename AUTOQ::Automata<Symbol>::State, typename AUTOQ::Automata<Symbol>::Symbol> leafSymbolMap;
    for (const auto &t : transitions) { // construct the map from state to leaf symbol
        if (t.first.is_leaf()) {
            for (const auto &s : t.second.at({})) {
                leafSymbolMap[s] = t.first.symbol(); // assume each state only maps to one leaf symbol
            }
        }
    }
    for (const auto &s : finalStates) {
        auto v = print(leafSymbolMap, 1, s);
        for (const auto &s : v) {
            std::map<std::string, int> count;
            for (unsigned i=0; i<s.size(); i++)
                count[s[i]]++;
            auto ptr = std::max_element(count.begin(), count.end(), [](const auto &x, const auto &y) {
                return x.second < y.second;
            });
            for (unsigned i=0; i<s.size(); i++) {
                if (s[i] != (ptr->first))
                    std::cout << boost::dynamic_bitset(qubitNum, i) << ":" << s[i] << " ";
            }
            std::cout << "*:" << (ptr->first) << std::endl;
        }
    }
}

// https://bytefreaks.net/programming-2/c/c-undefined-reference-to-templated-class-function
template struct AUTOQ::Automata<AUTOQ::Symbol::Concrete>;
template struct AUTOQ::Automata<AUTOQ::Symbol::Symbolic>;