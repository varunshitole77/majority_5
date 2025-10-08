#include <chrono>
#include <iostream>
#include <map>
#include <sstream>
#include <vector>

// my project headers
#include "../src/truth_table.h"
#include "../src/npn_classifier.h"
#include "../src/mig_synthesizer.h"
#include "../src/mig_structure.h" 
#include "../src/utils.h"

void computeN4(int max_classes = -1) {
    Utils::Timer timer("compute_n4");

    int n_vars = 4;
    Utils::logMessage("Computing NPN classes for n=" + std::to_string(n_vars));

    auto all_tables = TruthTable::generateAllTruthTables(n_vars);
    std::cout << "Generated " << all_tables.size() << " truth tables" << std::endl;

    auto npn_classes = NPNClassifier::findNPNClasses(all_tables);
    std::cout << "Found " << npn_classes.size() << " NPN classes" << std::endl;

    if (max_classes > 0 && max_classes < (int)npn_classes.size()) {
        npn_classes.resize(max_classes);
        std::cout << "Processing first " << max_classes << " classes only" << std::endl;
    }

    std::stringstream database;
    double total_time = 0;
    int success_count = 0, fail_count = 0;
    std::map<int, int> size_hist;

    auto global_start = std::chrono::high_resolution_clock::now();

    for (size_t i = 0; i < npn_classes.size(); ++i) {
        const auto& rep = npn_classes[i];

        if (i % 10 == 0) {
            auto now = std::chrono::high_resolution_clock::now();
            double elapsed = std::chrono::duration<double>(now - global_start).count();
            std::cout << "[Progress] " << (i + 1) << "/" << npn_classes.size()
                      << " (" << (100.0 * (i + 1) / npn_classes.size()) << "%)"
                      << " | Elapsed: " << elapsed << "s" << std::endl;
        }

        std::cout << "\n[" << (i + 1) << "/" << npn_classes.size() 
                  << "] Processing: " << rep << std::endl;

        auto start = std::chrono::high_resolution_clock::now();
        auto mig = synthesizeOptimalMIG(rep);
        auto end = std::chrono::high_resolution_clock::now();

        double synth_time = std::chrono::duration<double>(end - start).count();
        total_time += synth_time;

        if (mig) {
            int sz = mig->size();
            size_hist[sz]++;
            success_count++;

            database << "CLASS " << rep << "\n";
            database << mig->toText();
            database << "TIME " << synth_time << "\n";
            database << "---\n";

            std::cout << "  ✓ Size: " << sz 
                      << ", Depth: " << mig->depth() 
                      << ", Time: " << synth_time << "s" << std::endl;
        } else {
            fail_count++;
            std::cout << "  ✗ Failed to synthesize" << std::endl;
        }
    }

    std::cout << "\n" << std::string(40, '=') << std::endl;
    std::cout << "SYNTHESIS COMPLETE\n";
    std::cout << "Success: " << success_count 
              << ", Fail: " << fail_count 
              << ", Total time: " << total_time << "s" << std::endl;
    std::cout << "Average/class: " << total_time / npn_classes.size() << "s" << std::endl;

    std::cout << "\nSize distribution:" << std::endl;
    for (const auto& [sz, count] : size_hist) {
        std::cout << "  " << sz << " gates: " << count << " functions" << std::endl;
    }

    Utils::saveToTxt(database.str(), "data/npn_" + std::to_string(n_vars) + "var.txt");
    Utils::logMessage("Completed n=4 with " + std::to_string(success_count) + " classes");
}

int main(int argc, char* argv[]) {
    if (argc > 1) {
        int max_classes = std::stoi(argv[1]);
        computeN4(max_classes);
    } else {
        computeN4();
    }
    return 0;
}
