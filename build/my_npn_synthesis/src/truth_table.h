#pragma once
#include <vector>
#include <string>
#include <cstdint>

class TruthTable {
public:
    // Generate all truth tables for n_vars variables
    static std::vector<std::string> generateAllTruthTables(int n_vars);

    // Evaluate the truth table with given inputs
    static int evaluateTruthTable(const std::string& tt, const std::vector<int>& inputs);

    // Number of variables in a truth table
    static int getNumVars(const std::string& tt);

    // Convert truth table string to binary representation
    static uint64_t truthTableToBinary(const std::string& tt);

    // Convert binary representation to truth table string
    static std::string binaryToTruthTable(uint64_t binary, int n_vars);

    // Get cofactor with respect to variable
    static std::string getCofactor(const std::string& tt, int var_index, int value);

    // Check if truth table depends on a variable
    static bool dependsOnVar(const std::string& tt, int var_index);

    // Get support variables
    static std::vector<int> getSupport(const std::string& tt);

    // Validate truth table
    static bool validateTruthTable(const std::string& tt);
};
