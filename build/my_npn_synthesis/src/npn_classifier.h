#pragma once
#include <string>
#include <vector>
#include <set>

class NPNClassifier {
public:
    static std::string applyNegationPattern(const std::string& tt, 
                                           const std::vector<int>& neg_inputs, 
                                           int neg_output);

    static std::string applyPermutation(const std::string& tt, 
                                       const std::vector<int>& perm);

    static std::string getNPNRepresentative(const std::string& tt);

    static bool areNPNEquivalent(const std::string& tt1, const std::string& tt2);

    static std::vector<std::string> findNPNClasses(const std::vector<std::string>& truth_tables);

    static bool checkSymmetry(const std::string& tt, int var1, int var2);

    static std::vector<std::pair<int, int>> getSymmetricVariables(const std::string& tt);
};
