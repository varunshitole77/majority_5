#pragma once
#include <memory>
#include <string>
#include "mig_structure.h"   // full MIG definition

// forward declare
struct MAJNet;

class MIGSynthesizer {
public:
  std::unique_ptr<MIG> synthesize(const std::string& truth_table);
};

std::unique_ptr<MIG> synthesizeOptimalMIG(const std::string& truth_table);
std::unique_ptr<MAJNet> synthesizeMAJ5_orFallback(const std::string& tt, int nvars, bool force_maj5=false);
