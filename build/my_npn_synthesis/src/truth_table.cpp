#include "truth_table.h"
#include <cmath>
#include <stdexcept>
#include <algorithm>

std::vector<std::string> TruthTable::generateAllTruthTables(int n_vars) {
    int num_rows = 1 << n_vars;
    int num_functions = 1 << num_rows;
    std::vector<std::string> all_tables;
    
    for (int func = 0; func < num_functions; ++func) {
        std::string tt;
        for (int row = 0; row < num_rows; ++row) {
            tt += ((func >> row) & 1) ? '1' : '0';
        }
        all_tables.push_back(tt);
    }
    
    return all_tables;
}

int TruthTable::evaluateTruthTable(const std::string& tt, const std::vector<int>& inputs) {
    int index = 0;
    int n_vars = inputs.size();
    
    for (int i = 0; i < n_vars; ++i) {
        index += inputs[i] * (1 << (n_vars - 1 - i));
    }
    
    return tt[index] - '0';
}

int TruthTable::getNumVars(const std::string& tt) {
    int length = tt.length();
    int n_vars = std::log2(length);
    
    if ((1 << n_vars) != length) {
        throw std::invalid_argument("Invalid truth table length");
    }
    
    return n_vars;
}

uint64_t TruthTable::truthTableToBinary(const std::string& tt) {
    uint64_t result = 0;
    for (size_t i = 0; i < tt.length(); ++i) {
        if (tt[i] == '1') {
            result |= (1ULL << i);
        }
    }
    return result;
}

std::string TruthTable::binaryToTruthTable(uint64_t binary, int n_vars) {
    int length = 1 << n_vars;
    std::string tt;
    
    for (int i = 0; i < length; ++i) {
        tt += ((binary >> i) & 1) ? '1' : '0';
    }
    
    return tt;
}

std::string TruthTable::getCofactor(const std::string& tt, int var_index, int value) {
    int n_vars = getNumVars(tt);
    std::string cofactor;
    
    for (int i = 0; i < (1 << n_vars); ++i) {
        int var_value = (i >> (n_vars - 1 - var_index)) & 1;
        if (var_value == value) {
            cofactor += tt[i];
        }
    }
    
    return cofactor;
}

bool TruthTable::dependsOnVar(const std::string& tt, int var_index) {
    std::string cofactor0 = getCofactor(tt, var_index, 0);
    std::string cofactor1 = getCofactor(tt, var_index, 1);
    return cofactor0 != cofactor1;
}

std::vector<int> TruthTable::getSupport(const std::string& tt) {
    int n_vars = getNumVars(tt);
    std::vector<int> support;
    
    for (int i = 0; i < n_vars; ++i) {
        if (dependsOnVar(tt, i)) {
            support.push_back(i);
        }
    }
    
    return support;
}

bool TruthTable::validateTruthTable(const std::string& tt) {
    for (char c : tt) {
        if (c != '0' && c != '1') {
            return false;
        }
    }
    int length = tt.length();
    return (length > 0) && ((length & (length - 1)) == 0);
}
