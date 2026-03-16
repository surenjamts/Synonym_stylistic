#include <iostream>
#include <fstream>
#include <vector>
#include <string>
using namespace std;

int main() {
    ifstream fin("suffix.txt");
    ofstream fout("output.txt");
    string str = u8"сонгожээ";

    
    
    vector<string> suffixes;
    string suf;

    while (fin >> suf) {
        suffixes.push_back(suf);
    }
    for (int i = 0; i < (int)suffixes.size(); i++) {
        string cur = suffixes[i];

        if (str.size() >= cur.size() &&
            str.substr(str.size() - cur.size()) == cur) {
            str.erase(str.size() - cur.size());
            break;
        }
    }

    fout << str + u8"х" << '\n';
    return 0;
}