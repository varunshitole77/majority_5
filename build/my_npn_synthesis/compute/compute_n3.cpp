#include <cassert>
#include <chrono>
#include <iostream>
#include <sstream>

// my project headers
#include "../src/truth_table.h"
#include "../src/npn_classifier.h"
#include "../src/mig_synthesizer.h"
#include "../src/mig_structure.h"
#include "../src/utils.h"

int main() {
    int n_vars = 3;
    Utils::logMessage("Computing NPN classes for n=3");

    auto all_tables = TruthTable::generateAllTruthTables(n_vars);
    std::cout << "Generated " << all_tables.size() << " truth tables" << std::endl;

    auto npn_classes = NPNClassifier::findNPNClasses(all_tables);
    std::cout << "Found " << npn_classes.size() << " NPN classes (expected ~14)" << std::endl;

    std::stringstream database;
    double total_time = 0.0;

    for (size_t i = 0; i < npn_classes.size(); ++i) {
        const auto& rep = npn_classes[i];
        std::cout << "\nProcessing class " 
                  << (i + 1) << "/" << npn_classes.size() 
                  << ": " << rep << std::endl;

        auto start = std::chrono::high_resolution_clock::now();
        auto mig = synthesizeOptimalMIG(rep);
        auto end = std::chrono::high_resolution_clock::now();

        double synth_time = std::chrono::duration<double>(end - start).count();
        total_time += synth_time;

        if (mig) {
            // sanity check for majority-of-3
            if (rep == "00010111") {
                std::cout << "  >>> Found MAJ-3 function!" << std::endl;
                assert(mig->size() == 1);
            }

            database << "CLASS " << rep << "\n";
            database << mig->toText();
            database << "TIME " << synth_time << "\n";
            database << "---\n";

            std::cout << "  Size: " << mig->size() 
                      << ", Depth: " << mig->depth() 
                      << ", Time: " << synth_time << "s" << std::endl;
        }
    }

    Utils::saveToTxt(database.str(), "data/npn_3var.txt");
    Utils::logMessage("Completed n=3: " + std::to_string(npn_classes.size()) +
                     " classes in " + std::to_string(total_time) + "s total");

    return 0;
}
