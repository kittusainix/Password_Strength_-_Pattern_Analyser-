
#include <algorithm>
#include <cctype>
#include <cmath>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <set>
#include <sstream>
#include <string>
#include <vector>

using namespace std;

// ---------- Data ----------
const vector<string> COMMON_PASSWORDS = {
    "password", "123456", "123456789", "qwerty", "abc123", "letmein", "admin",
    "welcome", "monkey", "dragon", "iloveyou", "login", "master", "hello",
    "football", "baseball", "sunshine", "princess", "shadow", "superman",
    "passw0rd", "trustno1", "whatever", "freedom", "batman", "india", "ludhiana"};

const vector<string> KEYBOARD_ROWS = {
    "qwertyuiop", "asdfghjkl", "zxcvbnm", "1234567890", "!@#$%^&*()"};

// ---------- Result struct ----------
struct Analysis {
    int length = 0;
    bool hasLower = false, hasUpper = false, hasDigit = false, hasSymbol = false;
    int poolSize = 0;
    double entropy = 0.0;          // raw bits
    double effectiveEntropy = 0.0; // after pattern penalties
    int score = 0;                 // 0-100
    string rating;
    vector<string> patterns;       // detected weaknesses
    vector<string> suggestions;
};

// ---------- Helpers ----------
string toLower(string s) {
    transform(s.begin(), s.end(), s.begin(), [](unsigned char c) { return tolower(c); });
    return s;
}

string reverseStr(string s) {
    reverse(s.begin(), s.end());
    return s;
}

// Undo common leetspeak so "p@ssw0rd" is caught as "password"
string deLeet(const string &s) {
    string out = toLower(s);
    for (char &c : out) {
        switch (c) {
            case '@': case '4': c = 'a'; break;
            case '3': c = 'e'; break;
            case '1': case '!': case '|': c = 'i'; break;
            case '0': c = 'o'; break;
            case '$': case '5': c = 's'; break;
            case '7': case '+': c = 't'; break;
        }
    }
    return out;
}

// ---------- Pattern detectors ----------
// Each returns a penalty in bits and appends a description to `found`.

// Same character repeated 3+ times: "aaa", "1111"
double detectRepeatedChars(const string &p, vector<string> &found) {
    double penalty = 0;
    for (size_t i = 0; i < p.size();) {
        size_t j = i;
        while (j < p.size() && p[j] == p[i]) j++;
        if (j - i >= 3) {
            found.push_back("Repeated character '" + string(1, p[i]) + "' x" + to_string(j - i));
            penalty += 2.0 * (j - i);
        }
        i = j;
    }
    return penalty;
}

// Repeated block: "abcabc", "xyzxyzxyz"
double detectRepeatedBlocks(const string &p, vector<string> &found) {
    int n = p.size();
    for (int len = 2; len <= n / 2; len++) {
        if (n % len != 0) continue;
        bool ok = true;
        for (int i = len; i < n && ok; i++)
            if (p[i] != p[i - len]) ok = false;
        if (ok) {
            found.push_back("Repeating block \"" + p.substr(0, len) + "\" x" + to_string(n / len));
            return 3.0 * len + 4.0 * (n / len);
        }
    }
    return 0;
}

// Ascending/descending runs: "abcd", "4321" (run of 3 or more)
double detectSequences(const string &p, vector<string> &found) {
    double penalty = 0;
    string s = toLower(p);
    int n = s.size();
    int i = 0;
    while (i < n - 1) {
        int diff = s[i + 1] - s[i];
        if (diff == 1 || diff == -1) {
            int j = i + 1;
            while (j + 1 < n && s[j + 1] - s[j] == diff &&
                   isalnum((unsigned char)s[j]) && isalnum((unsigned char)s[j + 1]))
                j++;
            int runLen = j - i + 1;
            if (runLen >= 3 && isalnum((unsigned char)s[i])) {
                found.push_back(string(diff == 1 ? "Ascending" : "Descending") +
                                " sequence \"" + p.substr(i, runLen) + "\"");
                penalty += 3.0 * runLen;
            }
            i = j;
        } else {
            i++;
        }
    }
    return penalty;
}

// Keyboard walks: "qwerty", "asdf", "ytrewq"
double detectKeyboardWalks(const string &p, vector<string> &found) {
    double penalty = 0;
    string s = toLower(p);
    set<string> reported;
    for (const string &row : KEYBOARD_ROWS) {
        string rev = reverseStr(row);
        for (const string &r : {row, rev}) {
            for (size_t len = r.size(); len >= 4; len--) {
                for (size_t st = 0; st + len <= r.size(); st++) {
                    string chunk = r.substr(st, len);
                    if (s.find(chunk) != string::npos) {
                        // only report the longest overlapping hits once
                        bool covered = false;
                        for (auto &x : reported)
                            if (x.find(chunk) != string::npos) covered = true;
                        if (!covered) {
                            reported.insert(chunk);
                            found.push_back("Keyboard pattern \"" + chunk + "\"");
                            penalty += 2.5 * len;
                        }
                    }
                }
            }
        }
    }
    return penalty;
}

// Dictionary words, including leetspeak variants
double detectCommonWords(const string &p, vector<string> &found) {
    double penalty = 0;
    string plain = toLower(p), leet = deLeet(p);
    for (const string &w : COMMON_PASSWORDS) {
        if (plain == w) {
            found.push_back("Exact match with a very common password (\"" + w + "\")");
            penalty += 60;
        } else if (plain.find(w) != string::npos) {
            found.push_back("Contains common word \"" + w + "\"");
            penalty += 3.0 * w.size();
        } else if (leet.find(w) != string::npos) {
            found.push_back("Leetspeak variant of common word \"" + w + "\"");
            penalty += 6.0 * w.size();
        }
    }
    return penalty;
}

// Years 1950-2039 and date-like numbers
double detectYears(const string &p, vector<string> &found) {
    double penalty = 0;
    for (size_t i = 0; i + 4 <= p.size(); i++) {
        string chunk = p.substr(i, 4);
        if (all_of(chunk.begin(), chunk.end(), ::isdigit)) {
            int y = stoi(chunk);
            if (y >= 1950 && y <= 2039) {
                found.push_back("Looks like a year (" + chunk + ")");
                penalty += 6;
            }
        }
    }
    return penalty;
}

// Single-class passwords: all letters, all digits
double detectLowVariety(const string &p, vector<string> &found) {
    bool allDigits = all_of(p.begin(), p.end(), ::isdigit);
    bool allAlpha = all_of(p.begin(), p.end(), ::isalpha);
    if (allDigits) { found.push_back("Only digits"); return 8; }
    if (allAlpha)  { found.push_back("Only letters"); return 4; }
    return 0;
}

// Pattern like Word + digits at end ("Summer2024", "India@123")
double detectTrailingDigits(const string &p, vector<string> &found) {
    size_t i = p.size();
    while (i > 0 && (isdigit((unsigned char)p[i - 1]) || p[i - 1] == '!' || p[i - 1] == '@')) i--;
    size_t tail = p.size() - i;
    if (i >= 3 && tail >= 1 && tail <= 4 && isalpha((unsigned char)p[0])) {
        found.push_back("Predictable \"word + digits/symbol\" structure");
        return 5;
    }
    return 0;
}

// ---------- Core analyser ----------
Analysis analyse(const string &p) {
    Analysis a;
    a.length = p.size();
    if (p.empty()) { a.rating = "Empty"; return a; }

    for (unsigned char c : p) {
        if (islower(c)) a.hasLower = true;
        else if (isupper(c)) a.hasUpper = true;
        else if (isdigit(c)) a.hasDigit = true;
        else a.hasSymbol = true;
    }
    a.poolSize = (a.hasLower ? 26 : 0) + (a.hasUpper ? 26 : 0) +
                 (a.hasDigit ? 10 : 0) + (a.hasSymbol ? 32 : 0);
    a.entropy = a.length * log2((double)a.poolSize);

    // Pattern penalties
    double penalty = 0;
    penalty += detectRepeatedChars(p, a.patterns);
    penalty += detectRepeatedBlocks(p, a.patterns);
    penalty += detectSequences(p, a.patterns);
    penalty += detectKeyboardWalks(p, a.patterns);
    penalty += detectCommonWords(p, a.patterns);
    penalty += detectYears(p, a.patterns);
    penalty += detectLowVariety(p, a.patterns);
    penalty += detectTrailingDigits(p, a.patterns);

    a.effectiveEntropy = max(0.0, a.entropy - penalty);

    // Map effective entropy to 0-100 (~80 bits = full marks)
    a.score = (int)min(100.0, a.effectiveEntropy / 80.0 * 100.0);
    if (a.length < 8) a.score = min(a.score, 25);  // hard cap for short passwords

    if (a.score < 20)      a.rating = "Very Weak";
    else if (a.score < 40) a.rating = "Weak";
    else if (a.score < 60) a.rating = "Fair";
    else if (a.score < 80) a.rating = "Strong";
    else                   a.rating = "Very Strong";

    // Suggestions
    if (a.length < 12) a.suggestions.push_back("Use at least 12 characters (longer is better).");
    if (!a.hasUpper)   a.suggestions.push_back("Add uppercase letters.");
    if (!a.hasLower)   a.suggestions.push_back("Add lowercase letters.");
    if (!a.hasDigit)   a.suggestions.push_back("Add digits.");
    if (!a.hasSymbol)  a.suggestions.push_back("Add symbols such as ! @ # $ %.");
    if (!a.patterns.empty())
        a.suggestions.push_back("Avoid the detected patterns; they are the first things attackers try.");
    if (a.suggestions.empty())
        a.suggestions.push_back("Looks good! Consider a passphrase of 4+ random words for easy recall.");
    return a;
}

// ---------- Output ----------
string bar(int score) {
    int filled = score / 5;  // 20 segments
    return "[" + string(filled, '#') + string(20 - filled, '-') + "] " + to_string(score) + "/100";
}

string crackTime(double bits) {
    // Assume 10 billion guesses/second (offline GPU attack on a fast hash)
    double seconds = pow(2.0, bits) / 1e10;
    if (seconds < 1) return "instantly";
    const struct { double s; const char *name; } units[] = {
        {31557600.0 * 1e9, "billion+ years"}, {31557600.0 * 1e6, "million years"},
        {31557600.0 * 1e3, "thousand years"}, {31557600.0, "years"},
        {86400.0, "days"}, {3600.0, "hours"}, {60.0, "minutes"}, {1.0, "seconds"}};
    for (auto &u : units)
        if (seconds >= u.s) {
            ostringstream os;
            os << fixed << setprecision(1) << seconds / u.s << " " << u.name;
            return os.str();
        }
    return "instantly";
}

void printReport(const string &pw, const Analysis &a, bool showPassword) {
    cout << "\n==================== REPORT ====================\n";
    cout << "Password : " << (showPassword ? pw : string(pw.size(), '*')) << "\n";
    cout << "Length   : " << a.length << "\n";
    cout << "Classes  : "
         << (a.hasLower ? "lower " : "") << (a.hasUpper ? "UPPER " : "")
         << (a.hasDigit ? "digits " : "") << (a.hasSymbol ? "symbols" : "") << "\n";
    cout << fixed << setprecision(1);
    cout << "Entropy  : " << a.entropy << " bits (raw), "
         << a.effectiveEntropy << " bits (after penalties)\n";
    cout << "Strength : " << a.rating << "  " << bar(a.score) << "\n";
    cout << "Crack est: " << crackTime(a.effectiveEntropy)
         << "  (offline attack, 10^10 guesses/sec)\n";

    cout << "\nPatterns detected:\n";
    if (a.patterns.empty()) cout << "  (none)\n";
    for (auto &p : a.patterns) cout << "  - " << p << "\n";

    cout << "\nSuggestions:\n";
    for (auto &s : a.suggestions) cout << "  * " << s << "\n";
    cout << "================================================\n";
}

void batchMode(const string &file) {
    ifstream in(file);
    if (!in) { cout << "Could not open file: " << file << "\n"; return; }
    string line;
    cout << "\n" << left << setw(24) << "Password" << setw(14) << "Rating"
         << setw(8) << "Score" << "Patterns\n" << string(60, '-') << "\n";
    int total = 0, count = 0;
    while (getline(in, line)) {
        if (line.empty()) continue;
        Analysis a = analyse(line);
        cout << left << setw(24) << line.substr(0, 22) << setw(14) << a.rating
             << setw(8) << a.score << a.patterns.size() << "\n";
        total += a.score; count++;
    }
    if (count) cout << "\nAnalysed " << count << " passwords, average score "
                    << total / count << "/100\n";
}

// ---------- Main ----------
int main() {
    cout << "=== Password Strength & Pattern Analyser ===\n";
    cout << "NOTE: everything runs locally; nothing is stored or sent anywhere.\n";
    int choice;
    do {
        cout << "\n1. Analyse a password\n2. Analyse passwords from a file (one per line)\n"
                "3. Exit\nChoice: ";
        if (!(cin >> choice)) break;
        cin.ignore(10000, '\n');
        if (choice == 1) {
            string pw;
            cout << "Enter password: ";
            getline(cin, pw);
            char show;
            cout << "Show password in report? (y/n): ";
            cin >> show;
            cin.ignore(10000, '\n');
            printReport(pw, analyse(pw), tolower(show) == 'y');
        } else if (choice == 2) {
            string f;
            cout << "File name: ";
            getline(cin, f);
            batchMode(f);
        }
    } while (choice != 3);
    cout << "Goodbye!\n";
    return 0;
}
