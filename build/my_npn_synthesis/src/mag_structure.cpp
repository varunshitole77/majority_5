#include "maj_structure.h"
#include <cassert>

static std::string inputToStr(const MAJInput& t) {
  std::stringstream ss;
  switch (t.kind) {
    case MAJInput::Kind::PI:     ss << (t.inverted ? "¬" : "") << "x" << (t.index+1); break;
    case MAJInput::Kind::CONST0: ss << "0"; break;
    case MAJInput::Kind::CONST1: ss << "1"; break;
    case MAJInput::Kind::NODE:   ss << (t.inverted ? "¬" : "") << "n" << t.index; break;
  }
  return ss.str();
}

std::string MAJGate::toText(int /*n_vars*/) const {
  std::stringstream ss;
  ss << "MAJ" << k << "(";
  for (int i=0;i<k;i++) {
    if (i) ss << ", ";
    ss << inputToStr(in[i]);
  }
  ss << ")";
  if (inverted) ss << " (inv)";
  return ss.str();
}

int MAJNet::evalInput(const MAJInput& t, const std::vector<int>& pi_vals) const {
  int v=0;
  switch (t.kind) {
    case MAJInput::Kind::PI:     v = pi_vals[t.index]; break;
    case MAJInput::Kind::CONST0: v = 0; break;
    case MAJInput::Kind::CONST1: v = 1; break;
    case MAJInput::Kind::NODE:   v = evalNode(t.index, pi_vals); break;
  }
  return t.inverted ? (1 - v) : v;
}

int MAJNet::evalNode(int node_id, const std::vector<int>& pi_vals) const {
  const auto& g = gates[node_id];
  if (g.k == 3) {
    int a = evalInput(g.in[0], pi_vals);
    int b = evalInput(g.in[1], pi_vals);
    int c = evalInput(g.in[2], pi_vals);
    int out = MAJGate::maj3(a,b,c);
    return g.inverted ? (1 - out) : out;
  } else {
    assert(g.k == 5);
    int a = evalInput(g.in[0], pi_vals);
    int b = evalInput(g.in[1], pi_vals);
    int c = evalInput(g.in[2], pi_vals);
    int d = evalInput(g.in[3], pi_vals);
    int e = evalInput(g.in[4], pi_vals);
    int out = MAJGate::maj5(a,b,c,d,e);
    return g.inverted ? (1 - out) : out;
  }
}

std::string MAJNet::getTruthTable() const {
  std::string tt; tt.reserve(1u<<n_vars);
  int rows = 1<<n_vars;
  for (int m=0;m<rows;m++){
    std::vector<int> pi(n_vars,0);
    for (int i=0;i<n_vars;i++) pi[n_vars-1-i] = (m>>i)&1; // MSB=x1…LSB=xn
    int v = evalNode(output_gate, pi);
    tt.push_back( (output_inverted ? (v? '0':'1') : (v? '1':'0')) );
  }
  return tt;
}

std::string MAJNet::toText() const {
  std::stringstream ss;
  for (int i=0;i<(int)gates.size();++i){
    ss << "n" << i << " = " << gates[i].toText(n_vars) << "\n";
  }
  ss << "Output: n" << output_gate << (output_inverted? " (inv)":"") << "\n";
  return ss.str();
}
