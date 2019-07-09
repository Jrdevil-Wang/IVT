#include <fstream>
#include <iostream>
#include <map>
#include <vector>

#include "common.h"

using namespace std;
vector<long> parse_parts(string s) {
    vector<long> res;
    size_t found = s.find(" x ");
    while (found != string::npos) {
        size_t left = s.rfind("[", found) + 1;
        string ns = s.substr(left, found - left);
        long n = stoi(ns);
        res.push_back(n);
        found = s.find(" x ", found + 1);
    }
    return res;
}
vector<long> parse_pos(string s) {
    vector<long> res;
    size_t found = s.find("inrange");
    if (found == string::npos)
        return res;
    size_t p1r = s.find(",", found);
    size_t p1l = s.rfind("i32 ", p1r) + 4;
    long p1 = stoi(s.substr(p1l, p1r - p1l));
    size_t p2r = s.find(")", p1r);
    size_t p2l = s.rfind("i32 ", p2r) + 4;
    long p2 = stoi(s.substr(p2l, p2r - p2l));
    res.push_back(p1);
    res.push_back(p2);
    return res;
}
// calculate number of pointers in vtable (t) and position of the pointer in vtable (p)
// based on short version of vptr symbol (s) and full version of vptr symbol (f)
void cal_position(string s, string f, long * t, long * p) {
    size_t found = f.find(s);
    if (found == string::npos)
        return;
    size_t pos_l = f.rfind("{", found);
    size_t pos_r = f.rfind("}", found);
    string nums = f.substr(pos_l, pos_r - pos_l);
    vector<long> parts = parse_parts(nums);
    *t = 0;
    for (long l : parts)
        *t += l;
    vector<long> pos = parse_pos(f.substr(found));
    *p = 0;
    for (long i = 0; i < pos[0]; i++)
        *p += parts[i];
    *p += pos[1];
}
// main process
int main(int argc, char** argv) {
    // handle input error
    if (argc <= 1) {
        cerr << "Usage: <" << argv[0] << "> config.ivt\n";
        return -1;
    }
    // open ivt specification
    ifstream fin(argv[1]);
    if (!fin.is_open()) {
        cerr << "open ivt configure file <" << argv[1] << ">\n";
        return -1;
    }
    // read the ivt specification
    map<string, long > vptr_num;            // map from short vptr symbol to number of pointers in vtable
    vector< pair<string, long> > vptr_pos;  // map from short vptr symbol to position of the pointer in vtable
    while (!fin.eof()) {
        string line;
        getline(fin, line);
        if (line.empty())
            continue;
        if (line[0] == '#')
            continue;
        string vptr_f = trim(line);
        string vptr_s = name2vptr(line);
        vptr_s = vptr_s.substr(1, vptr_s.length() - 2);
        long total = 0, pos = 0;
        cal_position(vptr_s, vptr_f, &total, &pos);
        vptr_num.insert(pair<string, long >(vptr_s, total));
        vptr_pos.push_back(pair<string, long>(vptr_s, pos));
    }
    fin.close();
    // generate config.cpp
    for (pair<string, long> p : vptr_num)
        cout << "extern \"C\" long " << p.first << "[" << p.second << "];" << endl;
    cout << endl;
    cout << "long * g__ivt_space[" << vptr_pos.size() << "] = {" << endl;
    for (pair<string, long> p : vptr_pos)
        cout << "    &" << p.first << "[" << p.second << "]," << endl;
    cout << "};" << endl;
    cout << "long g__ivt = (long)(g__ivt_space);" << endl;
    return 0;
}

