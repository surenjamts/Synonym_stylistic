#include <bits/stdc++.h>
using namespace std;

static inline string trim_ascii(string s) {
    auto is_space = [](unsigned char c){
        return c==' ' || c=='\t' || c=='\r' || c=='\n';
    };
    while (!s.empty() && is_space((unsigned char)s.front())) s.erase(s.begin());
    while (!s.empty() && is_space((unsigned char)s.back())) s.pop_back();
    return s;
}

static inline vector<string> split_by_comma(const string& line) {
    vector<string> out;
    string cur;
    for (char ch : line) {
        if (ch == ',') {
            out.push_back(trim_ascii(cur));
            cur.clear();
        } else {
            cur.push_back(ch);
        }
    }
    out.push_back(trim_ascii(cur));
    // remove empties
    out.erase(remove_if(out.begin(), out.end(), [](const string& x){ return x.empty(); }), out.end());
    return out;
}

static inline string json_escape(const string& s) {
    string o;
    o.reserve(s.size() + 8);
    for (unsigned char c : s) {
        if (c == '\\') o += "\\\\";
        else if (c == '"') o += "\\\"";
        else if (c == '\n') o += "\\n";
        else if (c == '\r') o += "\\r";
        else if (c == '\t') o += "\\t";
        else o.push_back((char)c);
    }
    return o;
}

int main(int argc, char** argv) {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    string inPath = (argc >= 2 ? argv[1] : "part1.txt");
    string outPath = (argc >= 3 ? argv[2] : "synonyms.json");

    ifstream fin(inPath, ios::binary);
    if (!fin) {
        cerr << "Cannot open input: " << inPath << "\n";
        return 1;
    }

    // Build synonym groups
    vector<vector<string>> groups;
    string line;
    while (getline(fin, line)) {
        line = trim_ascii(line);
        if (line.empty()) continue;
        if (!line.empty() && line[0] == '-') line.erase(line.begin()); // remove leading '-'
        line = trim_ascii(line);
        if (line.empty()) continue;

        auto words = split_by_comma(line);
        if (words.size() >= 2) groups.push_back(words);
    }

    // Build mapping word -> set of synonyms
    unordered_map<string, unordered_set<string>> mp;
    mp.reserve(200000);

    for (auto& g : groups) {
        for (size_t i = 0; i < g.size(); i++) {
            for (size_t j = 0; j < g.size(); j++) {
                if (i == j) continue;
                mp[g[i]].insert(g[j]);
            }
        }
    }

    // Write JSON (UTF-8)
    ofstream fout(outPath, ios::binary);
    if (!fout) {
        cerr << "Cannot open output: " << outPath << "\n";
        return 1;
    }

    fout << "{\n";
    bool firstKey = true;
    // stable-ish output: sort keys
    vector<string> keys;
    keys.reserve(mp.size());
    for (auto& kv : mp) keys.push_back(kv.first);
    sort(keys.begin(), keys.end());

    for (auto& k : keys) {
        auto& setv = mp[k];
        vector<string> vals(setv.begin(), setv.end());
        sort(vals.begin(), vals.end());

        if (!firstKey) fout << ",\n";
        firstKey = false;

        fout << "  \"" << json_escape(k) << "\": [";
        for (size_t i = 0; i < vals.size(); i++) {
            if (i) fout << ",";
            fout << "\"" << json_escape(vals[i]) << "\"";
        }
        fout << "]";
    }
    fout << "\n}\n";

    cerr << "Wrote " << outPath << " with " << keys.size() << " keys\n";
    return 0;
}