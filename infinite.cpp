#include <bits/stdc++.h>
#include <windows.h>
using namespace std;

struct Suffix {
    string form;
    int cls;
};

struct SplitResult {
    string bare;
    int cls;
    string matchedSuffix;
};

SplitResult splitWord(const string& word, const vector<Suffix>& suf) {
    for (auto x : suf) {
        if (word.size() > x.form.size() &&
            word.substr(word.size() - x.form.size()) == x.form) {
            string bare = word.substr(0, word.size() - x.form.size());
            return {bare, x.cls, x.form};
        }
    }
    return {word, -1, ""};
}

int main() {
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);

    string word;
    cin >> word;

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

    sort(suf.begin(), suf.end(), [](const Suffix& a, const Suffix& b) {
        return a.form.size() > b.form.size();
    });

    auto res = splitWord(word, suf);

    string lemma = res.bare + u8"х";
    cout << lemma << "\n" << res.cls << "\n";

    return 0;
}