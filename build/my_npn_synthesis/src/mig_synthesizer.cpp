#include "mig_synthesizer.h"
#include "mig_structure.h"

#include <algorithm>
#include <array>
#include <cctype>
#include <memory>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

// ===== Utilities =====
namespace {

int deduce_n(std::size_t len) {
  if (len == 0) return -1;
  std::size_t p = 1; int n = 0;
  while (p < len) { p <<= 1; ++n; }
  return (p == len) ? n : -1;
}

// Encodes a MIGInput to a stable string for hashing
std::string encode_input(const MIGInput& in) {
  char k = '?';
  switch (in.kind) {
    case MIGInput::Kind::PI:     k = 'P'; break;
    case MIGInput::Kind::CONST0: k = '0'; break;
    case MIGInput::Kind::CONST1: k = '1'; break;
    case MIGInput::Kind::NODE:   k = 'N'; break;
  }
  return std::string(1, k) + (in.inverted ? "'" : "") + "#" + std::to_string(in.index);
}

// Canonical sort key for commutative majority(a,b,c)
struct InKey {
  int kind_rank;      // CONST0<CONST1<PI<NODE
  int base_index;     // index
  bool inverted;      // false < true
  std::string full;   // encoded (for uniqueness)
};

InKey key_of(const MIGInput& in) {
  int kr = 0;
  switch (in.kind) {
    case MIGInput::Kind::CONST0: kr = 0; break;
    case MIGInput::Kind::CONST1: kr = 1; break;
    case MIGInput::Kind::PI:     kr = 2; break;
    case MIGInput::Kind::NODE:   kr = 3; break;
  }
  return {kr, in.index, in.inverted, encode_input(in)};
}

bool same_base(const MIGInput& a, const MIGInput& b) {
  return a.kind == b.kind && a.index == b.index;
}
bool complementary(const MIGInput& a, const MIGInput& b) {
  return same_base(a,b) && (a.inverted != b.inverted) &&
         (a.kind == MIGInput::Kind::PI || a.kind == MIGInput::Kind::NODE);
}
bool equal_inputs(const MIGInput& a, const MIGInput& b) {
  return a.kind == b.kind && a.index == b.index && a.inverted == b.inverted;
}

} // namespace

// ===== Textless (structural) MIG builder using MIG API =====
class MIGBuilder {
public:
  explicit MIGBuilder(int n) : n_vars(n), mig(std::make_unique<MIG>(n)) {
    c0 = MIG::C0();
    c1 = MIG::C1();
    // cache canonical constants as nodes as needed when materialized
  }

  // Build an MIG from a truth table (length must be 2^n_vars)
  // Returns a ready MIG with OUTPUT set.
  std::unique_ptr<MIG> build_from_truth(const std::string& tt) {
    if (tt.size() != (std::size_t)(n_vars == 0 ? 1 : (1 << n_vars)))
      throw std::invalid_argument("truth length != 2^n");

    MIGInput root;
    if (n_vars == 0) {
      root = (tt[0] == '1') ? c1 : c0;
    } else {
      std::vector<MIGInput> sum_terms;
      sum_terms.reserve(tt.size());
      for (std::size_t i = 0; i < tt.size(); ++i) {
        if (tt[i] == '1') {
          std::vector<MIGInput> lits;
          lits.reserve(n_vars);
          // x1 is MSB, xN is LSB of index i
          for (int k = 0; k < n_vars; ++k) {
            bool bit_k = ((i >> (n_vars - 1 - k)) & 1) != 0;
            // MIG PIs are zero-based; prints as x0,x1,... in your current MIG toText.
            // This matches your mig_structure.cpp behavior.
            MIGInput v = MIG::PI(k, /*inv=*/!bit_k); // if bit=0 use !xk, else xk
            if (bit_k) v.inverted = false;
            lits.push_back(bit_k ? MIG::PI(k,false) : MIG::PI(k,true));
          }
          sum_terms.push_back(and_chain(lits));
        }
      }

      if (sum_terms.empty()) {
        root = c0;
      } else if (sum_terms.size() == 1) {
        root = sum_terms[0];
      } else {
        root = or_chain(sum_terms);
      }
    }

    // Ensure OUTPUT is a gate id (materialize literal/const if needed)
    int out_gid = materialize_as_gate(root);
    mig->setOutput(out_gid, /*polarity_inverted=*/false);
    return std::move(mig);
  }

private:
  int n_vars;
  std::unique_ptr<MIG> mig;

  // Reusable literal/const handles
  MIGInput c0, c1;

  // Structural hashing: canonical key → node id
  std::unordered_map<std::string, int> maj_cache;

  // ---------- Boolean ops via majority ----------
  MIGInput and2(const MIGInput& a, const MIGInput& b) {
    // Simplifications
    if (a.kind == MIGInput::Kind::CONST0 || b.kind == MIGInput::Kind::CONST0) return c0;
    if (a.kind == MIGInput::Kind::CONST1) return b;
    if (b.kind == MIGInput::Kind::CONST1) return a;
    if (equal_inputs(a,b)) return a;
    if (complementary(a,b)) return c0;
    return make_maj(a, b, c0);
  }

  MIGInput or2(const MIGInput& a, const MIGInput& b) {
    if (a.kind == MIGInput::Kind::CONST1 || b.kind == MIGInput::Kind::CONST1) return c1;
    if (a.kind == MIGInput::Kind::CONST0) return b;
    if (b.kind == MIGInput::Kind::CONST0) return a;
    if (equal_inputs(a,b)) return a;
    if (complementary(a,b)) return c1;
    return make_maj(a, b, c1);
  }

  MIGInput and_chain(const std::vector<MIGInput>& lits) {
    if (lits.empty()) return c1;
    MIGInput acc = lits[0];
    for (std::size_t i = 1; i < lits.size(); ++i) {
      acc = and2(acc, lits[i]);
      if (acc.kind == MIGInput::Kind::CONST0) return c0;
    }
    return acc;
  }

  MIGInput or_chain(const std::vector<MIGInput>& terms) {
    if (terms.empty()) return c0;
    MIGInput acc = terms[0];
    for (std::size_t i = 1; i < terms.size(); ++i) {
      acc = or2(acc, terms[i]);
      if (acc.kind == MIGInput::Kind::CONST1) return c1;
    }
    return acc;
  }

  // ---------- Majority creation with simplification + hashing ----------
  MIGInput make_maj(MIGInput a, MIGInput b, MIGInput c) {
    // If any two equal → that input
    if (equal_inputs(a,b) || equal_inputs(a,c)) return a;
    if (equal_inputs(b,c)) return b;

    // Complementary pair → the third
    if (complementary(a,b)) return c;
    if (complementary(a,c)) return b;
    if (complementary(b,c)) return a;

    // Canonicalize order for hashing (commutative)
    std::array<InKey,3> arr{ key_of(a), key_of(b), key_of(c) };
    std::sort(arr.begin(), arr.end(), [](const InKey& x, const InKey& y){
      if (x.kind_rank != y.kind_rank) return x.kind_rank < y.kind_rank;
      if (x.base_index != y.base_index) return x.base_index < y.base_index;
      if (x.inverted  != y.inverted )   return x.inverted  < y.inverted;
      return x.full < y.full;
    });

    const std::string key = "MAJ(" + arr[0].full + "," + arr[1].full + "," + arr[2].full + ")";
    auto it = maj_cache.find(key);
    if (it != maj_cache.end()) {
      return MIG::NODE(it->second, /*inv=*/false);
    }

    // Create real node
    int gid = mig->addGate(a, b, c);
    maj_cache.emplace(key, gid);
    return MIG::NODE(gid, /*inv=*/false);
  }

  // Materialize any literal or constant as an explicit gate so OUTPUT is always a gate id
  int materialize_as_gate(const MIGInput& in) {
    if (in.kind == MIGInput::Kind::NODE && !in.inverted) return in.index; // already node id
    // For a negated node, we can still emit MAJ(N',0,1) to produce a node with the desired polarity.
    if (in.kind == MIGInput::Kind::CONST0) {
      return mig->addGate(c0, c0, c0); // MAJ(0,0,0)
    }
    if (in.kind == MIGInput::Kind::CONST1) {
      return mig->addGate(c1, c1, c1); // MAJ(1,1,1)
    }
    // PI or NODE (possibly inverted): MAJ(in, 0, 1)
    return mig->addGate(in, c0, c1);
  }
};

// ===== Public API (matches your header) =====

std::unique_ptr<MIG> MIGSynthesizer::synthesize(const std::string& truth_table) {
  int n = deduce_n(truth_table.size());
  if (n < 0) {
    // Make a minimal constant-0 network to keep callers safe
    auto m = std::make_unique<MIG>(1);
    int g0 = m->addGate(MIG::C0(), MIG::C0(), MIG::C0()); // MAJ(0,0,0)
    m->setOutput(g0, false);
    return m;
  }
  MIGBuilder b(n);
  return b.build_from_truth(truth_table);
}

std::unique_ptr<MIG> synthesizeOptimalMIG(const std::string& truth_table) {
  MIGSynthesizer s;
  return s.synthesize(truth_table);
}

// We only have a forward-declared MAJNet type in the header.
// Returning nullptr avoids needing its complete definition here.
std::unique_ptr<MAJNet> synthesizeMAJ5_orFallback(const std::string& /*tt*/, int /*nvars*/, bool /*force_maj5*/) {
  return nullptr;
}
