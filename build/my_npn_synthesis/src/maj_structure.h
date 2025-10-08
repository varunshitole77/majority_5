#pragma once
#include <string>
#include <sstream>

// Minimal placeholder for a majority-5 style network.
// Everything is inline so there is NO linker dependency.
struct MAJNet {
  int n_inputs{0};

  std::string toText() const {
    std::ostringstream oss;
    oss << "MAJ-5 network with " << n_inputs << " inputs\n";
    return oss.str();
  }

  int size() const  { return 1; }  // stub
  int depth() const { return 1; }  // stub
};
