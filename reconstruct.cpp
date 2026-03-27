#include <bits/stdc++.h>
#include <windows.h>
using namespace std;

struct Suffix {
    string form;
    int cls;
};

vector<string> splitWords(const string& s) {
    vector<string> words;
    string cur;
    for (char c : s) {
        if (c == ' ') {
            if (!cur.empty()) {
                words.push_back(cur);
                cur.clear();
            }
        } else {
            cur += c;
        }
    }
    if (!cur.empty()) words.push_back(cur);
    return words;
}

string joinWords(const vector<string>& words) {
    string res;
    for (int i = 0; i < (int)words.size(); i++) {
        if (i) res += " ";
        res += words[i];
    }
    return res;
}

bool endsWith(const string& s, const string& suf) {
    if (s.size() < suf.size()) return false;
    return s.substr(s.size() - suf.size()) == suf;
}

string removeLastH(const string& s) {
    string h = u8"х";
    if (endsWith(s, h)) {
        return s.substr(0, s.size() - h.size());
    }
    return s;
}

vector<string> utf8Chars(const string& s) {
    vector<string> res;
    for (size_t i = 0; i < s.size();) {
        unsigned char c = (unsigned char)s[i];
        int len = 1;
        if ((c & 0x80) == 0x00) len = 1;
        else if ((c & 0xE0) == 0xC0) len = 2;
        else if ((c & 0xF0) == 0xE0) len = 3;
        else if ((c & 0xF8) == 0xF0) len = 4;
        res.push_back(s.substr(i, len));
        i += len;
    }
    return res;
}

bool isVowel(const string& ch) {
    return ch == u8"а" || ch == u8"э" || ch == u8"о" || ch == u8"ө" ||
           ch == u8"у" || ch == u8"ү" || ch == u8"ы" || ch == u8"и" ||
           ch == u8"А" || ch == u8"Э" || ch == u8"О" || ch == u8"Ө" ||
           ch == u8"У" || ch == u8"Ү" || ch == u8"Ы" || ch == u8"И";
}

string toLowerMongolianVowel(const string& ch) {
    if (ch == u8"А") return u8"а";
    if (ch == u8"Э") return u8"э";
    if (ch == u8"О") return u8"о";
    if (ch == u8"Ө") return u8"ө";
    if (ch == u8"У") return u8"у";
    if (ch == u8"Ү") return u8"ү";
    if (ch == u8"Ы") return u8"ы";
    if (ch == u8"И") return u8"и";
    return ch;
}

string findLastVowel(const string& s) {
    vector<string> chars = utf8Chars(s);
    for (int i = (int)chars.size() - 1; i >= 0; i--) {
        if (isVowel(chars[i])) {
            return toLowerMongolianVowel(chars[i]);
        }
    }
    return "";
}

string chooseVariant4(const vector<string>& forms, const string& stem) {
    if (forms.empty()) return "";
    if (forms.size() != 4) return forms[0];

    string v = findLastVowel(stem);

    if (v == u8"а") return forms[0];
    if (v == u8"э") return forms[1];
    if (v == u8"о" || v == u8"у") return forms[2];
    if (v == u8"ө" || v == u8"ү") return forms[3];

    return forms[0];
}

string chooseVariant2(const vector<string>& forms, const string& stem) {
    if (forms.empty()) return "";
    if (forms.size() != 2) return forms[0];

    string v = findLastVowel(stem);

    if (v == u8"ө" || v == u8"ү" || v == u8"э") return forms[1];
    return forms[0];
}

int main() {
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);

    string line;
    string idLine;

    getline(cin, line);
    getline(cin, idLine);

    if (line.empty() || idLine.empty()) {
        cout << "\n";
        return 0;
    }

    int suf_id = stoi(idLine);

    vector<Suffix> suf = {
        {"аад", 1}, {"ээд", 1}, {"оод", 1}, {"өөд", 1},
        {"саар", 2}, {"сээр", 2}, {"соор", 2}, {"сөөр", 2},
        {"цгаа", 3}, {"цгээ", 3}, {"цгоо", 3}, {"цгөө", 3},
        {"я", 4}, {"е", 4}, {"ё", 4},
        {"сугай", 5}, {"сэгэй", 5}, {"согой", 5}, {"сөгэй", 5},
        {"аач", 7}, {"ээч", 7}, {"ооч", 7}, {"өөч", 7},
        {"аарай", 8}, {"ээрэй", 8},
        {"уузай", 9}, {"үүзэй", 9},
        {"г", 10},
        {"аасай", 11}, {"ээсэй", 11}, {"оосой", 11}, {"өөсэй", 11},
        {"в", 12},
        {"лаа", 13}, {"лээ", 13}, {"лоо", 13}, {"лөө", 13},
        {"жээ", 14},
        {"чээ", 15},
        {"на", 16}, {"нэ", 16}, {"но", 16}, {"нө", 16},
        {"аар", 17}, {"ээр", 17}, {"оор", 17}, {"өөр", 17},
        {"тал", 18}, {"тэл", 18}, {"тол", 18}, {"төл", 18},
        {"маар", 19}, {"мээр", 19}, {"моор", 19}, {"мөөр", 19},
        {"сан", 20}, {"сэн", 20}, {"сон", 20}, {"сөн", 20},
        {"аа", 21}, {"ээ", 21}, {"оо", 21}, {"өө", 21},
        {"даг", 22}, {"дэг", 22}, {"дог", 22}, {"дөг", 22},
        {"ж", 23}, {"ч", 23},
        {"н", 25},
        {"вч", 26},
        {"магц", 28}, {"мэгц", 28}, {"могц", 28}, {"мөгц", 28},
        {"нгуут", 29}, {"нгүүт", 29},
        {"вал", 30}, {"вэл", 30}, {"бол", 30}, {"бөл", 30},
        {"ваас", 30}, {"вээс", 30}, {"баас", 30}, {"бээс", 30},
        {"нгаа", 31}, {"нгээ", 31}, {"нгоо", 31}, {"нгөө", 31},
        {"хад", 32}, {"хэд", 32}, {"ход", 32}, {"хөд", 32}
    };

    vector<vector<string>> byClass(40);
    for (auto x : suf) {
        byClass[x.cls].push_back(x.form);
    }

    vector<string> words = splitWords(line);
    if (words.empty()) {
        cout << "\n";
        return 0;
    }

    string last = words.back();
    string stem = removeLastH(last);

    vector<string> forms = byClass[suf_id];
    if (forms.empty()) {
        cout << line << "\n";
        return 0;
    }

    string chosen;
    if ((int)forms.size() == 1) {
        chosen = forms[0];
    } else if ((int)forms.size() == 2) {
        chosen = chooseVariant2(forms, stem);
    } else if ((int)forms.size() == 4) {
        chosen = chooseVariant4(forms, stem);
    } else {
        chosen = forms[0];
    }

    words.back() = stem + chosen;
    cout << joinWords(words) << "\n";
    return 0;
}