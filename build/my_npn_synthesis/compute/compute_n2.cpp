#include <iostream>
#include <sstream>

// my project headers
#include "../src/truth_table.h"
#include "../src/npn_classifier.h"
#include "../src/mig_synthesizer.h"
#include "../src/utils.h"
#include "../src/mig_structure.h"
int main() {
    int n_vars = 2; // 2 inputs
    Utils::logMessage("Computing NPN classes for n=2");

    auto all_tables = TruthTable::generateAllTruthTables(n_vars);
    std::cout << "Generated " << all_tables.size() << " truth tables" << std::endl;

    auto npn_classes = NPNClassifier::findNPNClasses(all_tables);
    std::cout << "Found " << npn_classes.size() << " NPN classes" << std::endl;

    std::stringstream database;

    for (size_t i = 0; i < npn_classes.size(); ++i) {
        const auto& rep = npn_classes[i];
        std::cout << "\nProcessing class " 
                  << (i + 1) << "/" << npn_classes.size() 
                  << ": " << rep << std::endl;

        auto mig = synthesizeOptimalMIG(rep);

        if (mig) {
            database << "CLASS " << rep << "\n";
            database << mig->toText();
            database << "---\n";

            std::cout << "  Size: " << mig->size() 
                      << ", Depth: " << mig->depth() << std::endl;
        }
    }

    Utils::saveToTxt(database.str(), "data/npn_2var.txt");
    Utils::logMessage("Completed n=2: " + std::to_string(npn_classes.size()) + " classes");

    return 0;
}
