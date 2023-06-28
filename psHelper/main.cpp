#include <cstdio>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <array>
#include <filesystem>

using namespace std;
namespace fs = std::filesystem;

const string cwd = fs::current_path();

int PROBLEM = 1000;

string exec(string command,bool printLogs = false) {
   char buffer[128];
   string result = "";

   // Open pipe to file
   FILE* pipe = popen(command.c_str(), "r");
   if (!pipe) {
      return "popen failed!";
   }

   // read till end of process:
   while (!feof(pipe)) {

      // use buffer to read and add to result
       if (fgets(buffer, 128, pipe) != NULL) {
           if(printLogs) cout << buffer;
           else result += buffer;
       }
   }

   pclose(pipe);
   return result;
}

void build() {
    cout << "[🛠️] Building...\n\n";
    string cmd = "cd \"" + cwd + "\"; xcodebuild -quiet -json -target cpp -scheme cpp CONFIGURATION_BUILD_DIR=./build";
    exec(cmd);
    cout << "\n[✅] Build done!\n\n";
}

void run() {
    cout << "[🏃] Running...\n\n";
    string cmd = "cd \"" + cwd + "\"; cd cpp; echo \"" + to_string(PROBLEM) + "\\n\" | ../build/cpp";
    exec(cmd, true);
}

void runCommand(string cmd) {
    build();
    if(cmd.size() <= 2) {
        cout << "[📝] Testing Problem " << PROBLEM << "...\n";
        return run();
    }
    int parsedInt = 0;
    for(int i = 2;i < cmd.size();i++) {
        parsedInt = parsedInt * 10 + cmd[i] - '0';
    }
    PROBLEM = parsedInt;
    cout << "[📝] Testing Problem " << PROBLEM << "...\n";
    run();
}

int main() {
    ios_base::sync_with_stdio(false);
    cin.tie(0);
    cout.tie(0);
    
    while (true) {
        cout << " $ ";
        string cmd;
        getline(cin, cmd);
        if(cmd.size() == 0) continue;
        switch (cmd[0]) {
            case 'e':
                return 0;
            case 'E':
                return 0;
            case 'b':
                build();
                break;
            case 'B':
                build();
                break;
            case 'r':
                runCommand(cmd);
                break;
            case 'R':
                runCommand(cmd);
                break;
            default:
                break;
        }
    }
}
