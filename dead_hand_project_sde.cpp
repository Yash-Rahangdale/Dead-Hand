#include <iostream>
#include <fstream>
#include <filesystem>
#include <ctime>
#include <cstring>


using namespace std;
namespace fs = std::filesystem;

const string PASSWORD = "secure123";
const string AUTH_FILE = "auth_state.txt";
const string DATA_DIR = "protected_data";

const int MAX_FAILED_ATTEMPTS = 14;
const int MAX_INACTIVE_DAYS = 14;

/* ----------------- Utility ----------------- */

long getCurrentTime() {
    return time(nullptr);
}

int daysBetween(long past, long now) {
    return (now - past) / (60 * 60 * 24);
}

/* ---------------- Secure Wipe ---------------- */

void secureDeleteFile(const fs::path& file) {
    fstream f(file, ios::in | ios::out | ios::binary);
    if (!f) return;

    auto size = fs::file_size(file);
    char zero = 0;

    for (uintmax_t i = 0; i < size; i++) {
        f.write(&zero, 1);
    }

    f.close();
    fs::remove(file);
}

void wipeDataDirectory() {
    cout << "\n⚠ DEAD HAND ACTIVATED: WIPING DATA ⚠\n";

    for (auto& entry : fs::recursive_directory_iterator(DATA_DIR)) {
        if (fs::is_regular_file(entry)) {
            secureDeleteFile(entry.path());
        }
    }

    cout << "✅ Data wiped successfully.\n";
}

/* ---------------- Auth State ---------------- */

void readAuthState(int &failedAttempts, long &lastLogin) {
    ifstream fin(AUTH_FILE);
    if (!fin) {
        failedAttempts = 0;
        lastLogin = getCurrentTime();
        return;
    }
    fin >> failedAttempts >> lastLogin;
}

void writeAuthState(int failedAttempts, long lastLogin) {
    ofstream fout(AUTH_FILE);
    fout << failedAttempts << " " << lastLogin;
}

/* ---------------- Main Logic ---------------- */

int main() {
    int failedAttempts;
    long lastLogin;

    readAuthState(failedAttempts, lastLogin);

    long now = getCurrentTime();

    if (daysBetween(lastLogin, now) >= MAX_INACTIVE_DAYS) {
        wipeDataDirectory();
        return 0;
    }

    string input;
    cout << "Enter password: ";
    cin >> input;

    if (input == PASSWORD) {
        cout << "✅ Login successful.\n";
        failedAttempts = 0;
        lastLogin = now;
        writeAuthState(failedAttempts, lastLogin);
    } else {
        cout << "❌ Incorrect password.\n";
        failedAttempts++;
        writeAuthState(failedAttempts, lastLogin);

        if (failedAttempts >= MAX_FAILED_ATTEMPTS) {
            wipeDataDirectory();
        }
    }

    return 0;
}
