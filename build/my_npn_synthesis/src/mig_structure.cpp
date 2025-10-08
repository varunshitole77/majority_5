#include "mig_structure.h"
#include <algorithm>
#include <stdexcept>

// MIGGate majority
int MIGGate::maj(int x, int y, int z) {
  return (x + y + z >= 2) ? 1 : 0;
}

std::string MIGGate::toText(int n_vars) const {
  std::ostringstream oss;
  oss << "G" << id << " = MAJ(";
  auto inputToStr = [&](const MIGInput& in) {
    std::ostringstream s;
    if (in.kind == MIGInput::Kind::PI) s << "x" << in.index;
    else if (in.kind == MIGInput::Kind::CONST0) s << "0";
    else if (in.kind == MIGInput::Kind::CONST1) s << "1";
    else if (in.kind == MIGInput::Kind::NODE) s << "G" << in.index;
    if (in.inverted) s << "'";
    return s.str();
  };
  oss << inputToStr(a) << "," << inputToStr(b) << "," << inputToStr(c) << ")";
  return oss.str();
}

// MIG
MIG::MIG(int vars) : n_vars(vars) {}

MIGInput MIG::PI(int i, bool inv) { return {MIGInput::Kind::PI, i, inv}; }
MIGInput MIG::C0() { return {MIGInput::Kind::CONST0, 0, false}; }
MIGInput MIG::C1() { return {MIGInput::Kind::CONST1, 1, false}; }
MIGInput MIG::NODE(int id, bool inv) { return {MIGInput::Kind::NODE, id, inv}; }

int MIG::addGate(const MIGInput& a, const MIGInput& b, const MIGInput& c) {
  int id = gates.size();
  gates.push_back({id, a, b, c});
  return id;
}

void MIG::setOutput(int gate_id, bool polarity_inverted) {
  output_gate = gate_id;
  output_inverted = polarity_inverted;
}

int MIG::size() const { return gates.size(); }
int MIG::depth() const { return size(); } // naive depth = number of gates

int MIG::evalInput(const MIGInput& in,
                   const std::vector<int>& inputs,
                   std::vector<int>& memo) const {
  int val = 0;
  switch (in.kind) {
    case MIGInput::Kind::PI: val = inputs[in.index]; break;
    case MIGInput::Kind::CONST0: val = 0; break;
    case MIGInput::Kind::CONST1: val = 1; break;
    case MIGInput::Kind::NODE:
      if (memo[in.index] == -1) {
        const auto& g = gates[in.index];
        int av = evalInput(g.a, inputs, memo);
        int bv = evalInput(g.b, inputs, memo);
        int cv = evalInput(g.c, inputs, memo);
        memo[in.index] = MIGGate::maj(av,bv,cv);
      }
      val = memo[in.index];
      break;
  }
  if (in.inverted) val ^= 1;
  return val;
}

int MIG::evaluate(const std::vector<int>& input_values) const {
  std::vector<int> memo(gates.size(), -1);
  int out = evalInput(MIG::NODE(output_gate), input_values, memo);
  return output_inverted ? (out ^ 1) : out;
}

std::string MIG::getTruthTable() const {
  int rows = 1 << n_vars;
  std::string tt(rows, '0');
  for (int i = 0; i < rows; i++) {
    std::vector<int> inputs(n_vars);
    for (int j = 0; j < n_vars; j++)
      inputs[j] = (i >> j) & 1;
    tt[i] = evaluate(inputs) ? '1' : '0';
  }
  return tt;
}

bool MIG::validate(const std::string& tt) const {
  return getTruthTable() == tt;
}

std::string MIG::toText() const {
  std::ostringstream oss;
  for (auto& g : gates) {
    oss << g.toText(n_vars) << "\n";
  }
  oss << "OUTPUT = G" << output_gate;
  if (output_inverted) oss << "'";
  oss << "\n";
  return oss.str();
}
