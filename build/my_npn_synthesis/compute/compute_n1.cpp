#include <iostream>
#include <sstream>

// my project headers
#include "../src/truth_table.h"
#include "../src/npn_classifier.h"
#include "../src/mig_synthesizer.h"
#include "../src/utils.h"
#include "../src/mig_structure.h"
int main() {
    int n_vars = 1; // starting small with 1 variable
    Utils::logMessage("Computing NPN classes for n=1");

    // generate all possible truth tables for 1 input
    auto all_tables = TruthTable::generateAllTruthTables(n_vars);
    std::cout << "Generated " << all_tables.size() << " truth tables" << std::endl;

    // classify them into NPN equivalence classes
    auto npn_classes = NPNClassifier::findNPNClasses(all_tables);
    std::cout << "Found " << npn_classes.size() << " NPN classes" << std::endl;

    std::stringstream database;

    // loop through each representative and synthesize MIGs
    for (size_t i = 0; i < npn_classes.size(); ++i) {
        const auto& rep = npn_classes[i];
        std::cout << "\nProcessing class " 
                  << (i + 1) << "/" << npn_classes.size() 
                  << ": " << rep << std::endl;

        auto mig = synthesizeOptimalMIG(rep);

        if (mig) {
            // dump MIG into text format
            database << "CLASS " << rep << "\n";
            database << mig->toText();
            database << "---\n";

            // also print to console
            std::cout << "  Size: " << mig->size() 
                      << ", Depth: " << mig->depth() << std::endl;
        }
    }

    // save results to file
    Utils::saveToTxt(database.str(), "data/npn_1var.txt");
    Utils::logMessage("Completed n=1: " + std::to_string(npn_classes.size()) + " classes");

    return 0;
}
