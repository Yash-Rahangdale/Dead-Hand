#include <iostream>
#include <fstream>
#include <ctime>
#include <cstdio>
#include <sys/stat.h>
#include <dirent.h>

using namespace std;

const char* PASSWORD = "secure123"; // this is the password
const char* AUTH_FILE = "auth_state.txt";
const char* DATA_DIR = "protected_data";

const int MAX_FAILED_ATTEMPTS = 14; // after 14 tries, it wipes
const int MAX_INACTIVE_DAYS = 14; // and if not used for 14 days

long getCurrentTime() {
    return time(NULL);
}

int daysBetween(long past, long now) {
    // calculates days, might 
    return (now - past) / (60 * 60 * 24);
}

// checks if directory is there
bool directoryExists(const char* path) {
    struct stat info;
    return (stat(path, &info) == 0 && (info.st_mode & S_IFDIR));
}

// this overwrites the file with zeros then deletes
void secureDeleteFile(string filepath) {
    fstream file(filepath.c_str(), ios::in | ios::out | ios::binary);
    if (!file) return;

    file.seekg(0, ios::end);
    long size = file.tellg();
    file.seekp(0);

    char zero = 0;
    for (long i = 0; i < size; i++) {
        file.write(&zero, 1);
    }

    file.close();
    remove(filepath.c_str());
}

void wipeDataDirectory() {
    // dead hand, like if something happens it wipes everything
    cout << "\nDEAD HAND ACTIVATED: WIPING DATA\n";

    if (!directoryExists(DATA_DIR)) {
        cout << "No protected_data directory found.\n";
        return;
    }

    DIR* dir = opendir(DATA_DIR);
    if (!dir) return;

    struct dirent* entry;
    // loop through files
    while ((entry = readdir(dir)) != NULL) {
        string filename = entry->d_name;

        if (filename == "." || filename == "..")
            continue;

        string fullPath = string(DATA_DIR) + "/" + filename;
        secureDeleteFile(fullPath);
    }

    closedir(dir);

    cout << "Data wiped successfully.\n";
}

// reads the auth file, if not there sets defaults
void readAuthState(int &failedAttempts, long &lastLogin) {
    ifstream fin(AUTH_FILE);

    if (!fin) {
        failedAttempts = 0;
        lastLogin = getCurrentTime();
        return;
    }

    fin >> failedAttempts >> lastLogin;
    fin.close();
}

// writes back to the file
void writeAuthState(int failedAttempts, long lastLogin) {
    ofstream fout(AUTH_FILE);
    fout << failedAttempts << " " << lastLogin;
    fout.close();
}

// main part
int main() {

    int failedAttempts;
    long lastLogin;

    readAuthState(failedAttempts, lastLogin);

    long now = getCurrentTime();

    // check if too inactive
    if (daysBetween(lastLogin, now) >= MAX_INACTIVE_DAYS) {
        wipeDataDirectory();
        return 0;
    }

    string input;
    cout << "Enter password: ";
    cin >> input;

    if (input == PASSWORD) {
        cout << "Login successful.\n";
        failedAttempts = 0;
        lastLogin = now;
        writeAuthState(failedAttempts, lastLogin);
    }
    else {
        cout << "Incorrect password.\n";
        failedAttempts++;
        writeAuthState(failedAttempts, lastLogin);

        // if too many fails, wipe
        if (failedAttempts >= MAX_FAILED_ATTEMPTS) {
            wipeDataDirectory();
        }
    }

    return 0;
}
