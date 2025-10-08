// compute_n5.cpp
// Pure Majority-5 synthesis with NPN classification for 5-input Boolean functions.
//
// Output format:
//   CLASS <bits>
//   G0 = MAJ5(...)
//   ...
//   OUTPUT = Gk
//   TIME <seconds>
//   ---
//
// Writes results into data/npn_5var.txt
//
// Compatible with older kitty (set_bit only, no reset_bit).

#include <algorithm>
#include <array>
#include <chrono>
#include <cstdint>
#include <exception>
#include <fstream>
#include <iostream>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include <kitty/dynamic_truth_table.hpp>
#include <kitty/npn.hpp>
#include <kitty/print.hpp>

// ───────────────────────────── Config ─────────────────────────────
static constexpr int      N_VARS         = 5;
static constexpr uint64_t SAMPLE_COUNT   = 100000;   // random samples
static constexpr uint32_t MAX_SYNTH      = 50;       // reps to synthesize
static constexpr uint32_t PROGRESS_EVERY = 10000;    // log interval

// ────────────────────────── MAJ5 Structures ───────────────────────
struct MAJ5Input {
  enum class Kind { PI, CONST0, CONST1, NODE } kind{Kind::PI};
  int  index{0};
  bool inv{false};

  static MAJ5Input PI(int i, bool inv=false)    { return {Kind::PI,     i, inv}; }
  static MAJ5Input NODE(int id, bool inv=false) { return {Kind::NODE,   id, inv}; }
  static MAJ5Input C0()                         { return {Kind::CONST0, 0, false}; }
  static MAJ5Input C1()                         { return {Kind::CONST1, 1, false}; }
};

struct MAJ5Net {
  struct Gate { MAJ5Input a,b,c,d,e; };
  int num_pis{5};
  std::vector<Gate> gates;
  int output_gate{-1};
  bool output_inv{false};
  std::unordered_map<std::string,int> cache;

  static std::string sig2str(const MAJ5Input& s){
    std::string n;
    switch(s.kind){
      case MAJ5Input::Kind::PI: n="x"+std::to_string(s.index); break;
      case MAJ5Input::Kind::NODE: n="G"+std::to_string(s.index); break;
      case MAJ5Input::Kind::CONST0: n="0"; break;
      case MAJ5Input::Kind::CONST1: n="1"; break;
    }
    if(s.inv) n += "'";
    return n;
  }

  std::string to_text() const {
    std::string out;
    for(size_t i=0;i<gates.size();i++){
      const auto& g=gates[i];
      out += "G"+std::to_string(i)+" = MAJ5("+
             sig2str(g.a)+", "+sig2str(g.b)+", "+
             sig2str(g.c)+", "+sig2str(g.d)+", "+
             sig2str(g.e)+")\n";
    }
    out += "OUTPUT = ";
    if(output_gate>=0) out += "G"+std::to_string(output_gate); else out += "?";
    if(output_inv) out += "'";
    out += "\n";
    return out;
  }

  std::string keyPart(const MAJ5Input& s){
    return std::to_string((int)s.kind)+":"+std::to_string(s.index)+":"+std::to_string((int)s.inv);
  }

  int create_maj5(MAJ5Input a,MAJ5Input b,MAJ5Input c,MAJ5Input d,MAJ5Input e){
    std::array<MAJ5Input,5> v{a,b,c,d,e};
    std::sort(v.begin(),v.end(),[&](const MAJ5Input& x,const MAJ5Input& y){
      if(x.kind!=y.kind) return (int)x.kind<(int)y.kind;
      if(x.index!=y.index) return x.index<y.index;
      return (int)x.inv<(int)y.inv;
    });
    std::string key="M5("+keyPart(v[0])+","+keyPart(v[1])+","+keyPart(v[2])+","+keyPart(v[3])+","+keyPart(v[4])+")";
    if(auto it=cache.find(key);it!=cache.end()) return it->second;
    gates.push_back({v[0],v[1],v[2],v[3],v[4]});
    int id=(int)gates.size()-1;
    cache.emplace(key,id);
    return id;
  }

  void set_output_node(int node_id,bool inv=false){ output_gate=node_id; output_inv=inv; }
};

static MAJ5Input maj5_and2(MAJ5Net& net, MAJ5Input a, MAJ5Input b){
  return MAJ5Input::NODE(net.create_maj5(a,b,a,b,MAJ5Input::C0()));
}
static MAJ5Input maj5_or2(MAJ5Net& net, MAJ5Input a, MAJ5Input b){
  return MAJ5Input::NODE(net.create_maj5(MAJ5Input::C0(),MAJ5Input::C1(),MAJ5Input::C1(),a,b));
}
static MAJ5Input chain_and(MAJ5Net& net,std::vector<MAJ5Input> v){
  if(v.empty()) return MAJ5Input::C1();
  while(v.size()>1){
    std::vector<MAJ5Input> nxt;
    for(size_t i=0;i+1<v.size();i+=2) nxt.push_back(maj5_and2(net,v[i],v[i+1]));
    if(v.size()&1) nxt.push_back(v.back());
    v.swap(nxt);
  }
  return v[0];
}
static MAJ5Input chain_or(MAJ5Net& net,std::vector<MAJ5Input> v){
  if(v.empty()) return MAJ5Input::C0();
  while(v.size()>1){
    std::vector<MAJ5Input> nxt;
    for(size_t i=0;i+1<v.size();i+=2) nxt.push_back(maj5_or2(net,v[i],v[i+1]));
    if(v.size()&1) nxt.push_back(v.back());
    v.swap(nxt);
  }
  return v[0];
}

static MAJ5Net synthesize_maj5_from_tt_5(const kitty::dynamic_truth_table& tt){
  MAJ5Net net; net.num_pis=5;
  std::vector<MAJ5Input> products;
  for(uint32_t m=0;m<32;m++){
    if(!kitty::get_bit(tt,m)) continue;
    std::vector<MAJ5Input> lits;
    for(uint32_t j=0;j<5;j++){
      bool bit=((m>>j)&1u)!=0u;
      lits.push_back(MAJ5Input::PI((int)j,!bit));
    }
    products.push_back(chain_and(net,std::move(lits)));
  }
  if(products.empty()){ 
    int g=net.create_maj5(MAJ5Input::C0(),MAJ5Input::C0(),MAJ5Input::C0(),MAJ5Input::C0(),MAJ5Input::C0()); 
    net.set_output_node(g); 
    return net; 
  }
  MAJ5Input root=(products.size()==1)?products[0]:chain_or(net,std::move(products));
  int out=(root.kind==MAJ5Input::Kind::NODE)?root.index:net.create_maj5(root,root,root,root,root);
  net.set_output_node(out);
  return net;
}

// ─────────────────────────── Helpers ─────────────────────────────
static std::string hex_to_32_bits(const std::string& hex_in){
  std::string hex=hex_in;
  if(hex.size()>=2&&hex[0]=='0'&&(hex[1]=='X'||hex[1]=='x')) hex=hex.substr(2);
  if(hex.size()<8) hex=std::string(8-hex.size(),'0')+hex;
  else if(hex.size()>8) hex=hex.substr(hex.size()-8);
  auto nibble_bits=[](char c)->std::array<char,4>{
    int v;
    if(c>='0'&&c<='9') v=c-'0';
    else if(c>='a'&&c<='f') v=10+(c-'a');
    else if(c>='A'&&c<='F') v=10+(c-'A');
    else v=0;
    return { char(((v>>3)&1)+'0'),
             char(((v>>2)&1)+'0'),
             char(((v>>1)&1)+'0'),
             char((v&1)+'0') };
  };
  std::string bits; bits.reserve(32);
  for(char c:hex){ auto nb=nibble_bits(c); bits.push_back(nb[0]); bits.push_back(nb[1]); bits.push_back(nb[2]); bits.push_back(nb[3]); }
  return bits;
}

static void set_tt_from_bits(kitty::dynamic_truth_table& tt, const std::string& bits32){
  for(uint32_t m=0; m<32; m++){
    char b = bits32[31 - m]; // MSB-left → LSB-first
    if(b == '1') kitty::set_bit(tt, m); // only set 1s
  }
}

static std::string class_bits_from_tt(const kitty::dynamic_truth_table& tt){
  std::string s; s.resize(32);
  for(uint32_t m=0;m<32;m++) s[31-m]=kitty::get_bit(tt,m)?'1':'0';
  return s;
}

// ─────────────────────────── Main ─────────────────────────────
int main(){
  try{
    kitty::dynamic_truth_table tt(N_VARS);
    std::unordered_set<std::string> reps_hex;
    reps_hex.reserve(SAMPLE_COUNT/2);

    for(uint64_t i=0;i<SAMPLE_COUNT;i++){
      kitty::create_random(tt);
      auto canon=kitty::exact_npn_canonization(tt);
      reps_hex.insert(kitty::to_hex(std::get<0>(canon)));
      if(((i+1)%PROGRESS_EVERY)==0)
        std::cerr<<"[NPN] processed "<<(i+1)<<"/"<<SAMPLE_COUNT
                 <<" | reps so far: "<<reps_hex.size()<<"\n";
    }

    std::vector<std::string> reps_sorted(reps_hex.begin(),reps_hex.end());
    std::sort(reps_sorted.begin(),reps_sorted.end());

    std::ofstream ofs("data/npn_5var.txt");
    if(!ofs){ std::cerr<<"ERROR opening data/npn_5var.txt\n"; return 1; }

    uint32_t printed=0;
    for(const auto& hex:reps_sorted){
      if(printed>=MAX_SYNTH) break;
      std::string bits32=hex_to_32_bits(hex);
      kitty::dynamic_truth_table tt5(N_VARS);
      set_tt_from_bits(tt5,bits32);

      auto s0=std::chrono::steady_clock::now();
      auto net=synthesize_maj5_from_tt_5(tt5);
      auto s1=std::chrono::steady_clock::now();
      double secs=std::chrono::duration<double>(s1-s0).count();

      std::string class_bits=class_bits_from_tt(tt5);

      ofs<<"CLASS "<<class_bits<<"\n"<<net.to_text();
      ofs<<"TIME "<<secs<<"\n---\n";

      printed++;
    }
    std::cerr<<"Completed n=5, wrote "<<printed<<" classes\n";
    return 0;
  }catch(const std::exception& e){
    std::cerr<<"FATAL: "<<e.what()<<"\n"; return 1;
  }
}
