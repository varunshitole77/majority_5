#pragma once
#include <vector>
#include <string>
#include <sstream>

// ---------------- MIGInput ----------------
struct MIGInput {
  enum class Kind { PI, CONST0, CONST1, NODE };
  Kind kind{};
  int index{-1};
  bool inverted{false};
};

// ---------------- MIGGate ----------------
struct MIGGate {
  int id{0};
  MIGInput a, b, c;

  static int maj(int x, int y, int z);
  std::string toText(int n_vars) const;
};

// ---------------- MIG ----------------
class MIG {
public:
  explicit MIG(int vars);

  // Input creators
  static MIGInput PI(int i, bool inv=false);
  static MIGInput C0();
  static MIGInput C1();
  static MIGInput NODE(int id, bool inv=false);

  // Add majority gate
  int addGate(const MIGInput& a, const MIGInput& b, const MIGInput& c);

  // Set the output
  void setOutput(int gate_id, bool polarity_inverted);

  // Stats
  int size() const;
  int depth() const;

  // Evaluate
  int evaluate(const std::vector<int>& input_values) const;
  std::string getTruthTable() const;

  // Check against truth table
  bool validate(const std::string& tt) const;

  // Print textual network
  std::string toText() const;

private:
  int n_vars{0};
  std::vector<MIGGate> gates;
  int output_gate{0};
  bool output_inverted{false};

  int evalInput(const MIGInput& in,
                const std::vector<int>& inputs,
                std::vector<int>& memo) const;
};
