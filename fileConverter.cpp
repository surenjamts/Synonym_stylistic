#include <bits/stdc++.h>
#include <windows.h>
using namespace std;
#define pb push_back
unsigned int i;
int main() {
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);

    ifstream fin("C:/Users/acer/Desktop/Final_year/image/part1.txt");
    ofstream fout("syno.txt");
    string line;
    fout << "{\n";
    vector < string > vc, synos;
    while (getline(fin, line)) {
        if( line.size() == 0 ) continue;
        if( line[0] == '-' ){
            line = line.substr(1);
            if( synos.size() == 0 ) continue;
            fout << "   \"" << synos[0] << "\":[";
            for( i = 1; i < synos.size(); i ++ ){
                fout << "\"" << synos[i] << "\"";
                if( i+1 < synos.size() ){
                    fout << ",";
                }
            }
            fout << "],\n";
            synos.clear();
        }
        string temp = "";
        for(char c : line ){
            if( c == ' ' ){
                if (!temp.empty()) {
                    synos.pb(temp);
                    temp = "";
                }
                temp = "";
                continue;
            }
            temp += c;
        }
        if( temp.size() > 0 ){
            if (!temp.empty()) {
                synos.pb(temp);
                temp = "";
            }
        }
    }
    fout << "}\n";
}