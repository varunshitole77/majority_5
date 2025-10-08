#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <stdexcept>

// your project headers
#include "utils.h"
#include "truth_table.h"
#include "npn_classifier.h"
#include "mig_structure.h"
#include "mig_synthesizer.h"
#include "maj_structure.h"  // <-- now exists and has inline methods

static void save_string(const std::string& path, const std::string& s) {
  std::ofstream f(path);
  if (!f) throw std::runtime_error("cannot open " + path);
  f << s;
}

static int required_nvars_from_tt(const std::string& tt) {
  if (tt.empty()) return 0;
  int len = static_cast<int>(tt.size());
  int n = 0; while ((1 << n) < len) ++n;
  if ((1 << n) != len) throw std::runtime_error("truth table length is not power-of-two");
  return n;
}

int main(int argc, char** argv) {
  try {
    // Minimal CLI:
    //   ./main --function 0101... (binary)   OR
    //   ./main --compute N
    if (argc < 2) {
      std::cerr << "Usage:\n"
                << "  " << argv[0] << " --function <binary_tt>\n"
                << "  " << argv[0] << " --compute <n>\n";
      return 1;
    }

    const std::string mode = argv[1];

    if (mode == "--function") {
      if (argc < 3) throw std::runtime_error("--function needs a binary truth table string");
      const std::string tt = argv[2];
      const int n_vars = required_nvars_from_tt(tt);

      // 1) Try MAJ5/MAJk fast-path (placeholder)
      if (auto maj_net = synthesizeMAJ5_orFallback(tt, n_vars, /*force_maj5=*/false)) {
        std::ostringstream oss;
        oss << "Synthesized MAJ network:\n"
            << "Size: " << maj_net->size() << ", Depth: " << maj_net->depth() << "\n"
            << maj_net->toText() << "\n";
        save_string("output.txt", oss.str());
        std::cout << "Saved synthesized MAJ network to output.txt\n";
        return 0;
      }

      // 2) Fall back to MIG/Mockturtle
      if (auto mig = synthesizeOptimalMIG(tt)) {
        std::ostringstream oss;
        oss << "Synthesized MIG (from Mockturtle Akers):\n";
        oss << "Size: " << mig->size() << ", Depth: " << mig->depth() << "\n";
        oss << mig->toText() << "\n";
        save_string("output.txt", oss.str());
        std::cout << "Saved synthesized MIG to output.txt\n";
        return 0;
      } else {
        std::cerr << "Synthesis failed (Mockturtle disabled or error)\n";
        return 2;
      }

    } else if (mode == "--compute") {
      if (argc < 3) throw std::runtime_error("--compute needs n");
      const int n = std::stoi(argv[2]);
      std::cerr << "[info] --compute flow unchanged; call synthesizeMAJ5_orFallback/synthesizeOptimalMIG as needed.\n";
      (void)n;
      return 0;

    } else {
      throw std::runtime_error("unknown mode: " + mode);
    }

  } catch (const std::exception& e) {
    std::cerr << "Error: " << e.what() << "\n";
    return 3;
  }
}
