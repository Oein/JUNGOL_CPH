/*
 
 ██████╗██████╗ ██╗  ██╗    ███████╗ ██████╗ ██████╗
██╔════╝██╔══██╗██║  ██║    ██╔════╝██╔═══██╗██╔══██╗
██║     ██████╔╝███████║    █████╗  ██║   ██║██████╔╝
██║     ██╔═══╝ ██╔══██║    ██╔══╝  ██║   ██║██╔══██╗
╚██████╗██║     ██║  ██║    ██║     ╚██████╔╝██║  ██║
 ╚═════╝╚═╝     ╚═╝  ╚═╝    ╚═╝      ╚═════╝ ╚═╝  ╚═╝
                                                     
             ██████╗██████╗ ██████╗
            ██╔════╝██╔══██╗██╔══██╗
            ██║     ██████╔╝██████╔╝
            ██║     ██╔═══╝ ██╔═══╝
            ╚██████╗██║     ██║
             ╚═════╝╚═╝     ╚═╝
                                                     
                                            __        __  ___
                                            |__)\ /   /  \|__ ||\ |
                                            |__) |    \__/|___|| \|
                                                            
 */



#include "./solution.hpp"
#include "json.hpp"
#include <future>
#include <fstream>
#include <iostream>
#include <filesystem>
#include <string>
#include <launch.h>
#include <chrono>
#include <vector>
#include <sys/resource.h>
#include <sys/time.h>
#include <curl/curl.h>


using namespace std;
using namespace std::chrono;
using namespace nlohmann;
using std::thread;
namespace fs = std::filesystem;
using FileEntry = fs::directory_entry;

const string cwd = fs::current_path();
const string testCasesPath = cwd + "/testCases";
const string coutFilePath = cwd + "/out.txt";
const string lastRequestedProblemPath = cwd + "/lastRequest.txt";
const string testCaseInputExtension = ".in";
string realAnswer, userAnswer;
streambuf* originalCout = cout.rdbuf();
long long testedCount = 0;
vector<FileEntry> inputFiles;

long getMemoryUsage() {
  struct rusage usage;
  if(0 == getrusage(RUSAGE_SELF, &usage))
    return usage.ru_maxrss; // bytes
  else
    return 0;
}
size_t WriteCallback(char *contents, size_t size, size_t nmemb, void *userp)
{
    ((std::string*)userp)->append((char*)contents, size * nmemb);
    return size * nmemb;
}
string getAnswerPath(FileEntry fen) {
    return fen.path().parent_path().string() + "/" + fen.path().filename().stem().string() + ".out";
}
bool compareFileEntry(const FileEntry& a, const FileEntry& b) {
    return a.path().filename().stem() < b.path().filename().stem();
}
bool is_empty_file(ifstream& pFile) {
    return pFile.peek() == ifstream::traits_type::eof();
}
string clockByPercent(long long percent) {
    string clocks[] = {"🕐","🕑","🕒","🕓","🕔","🕕","🕖","🕗","🕘","🕙","🕚","🕛"};
    return clocks[(int)(percent / (100 / 11))];
}
long long since(steady_clock::time_point const& start) {
    return duration_cast<milliseconds>(steady_clock::now() - start).count();
}
void printInfo() {
    string temp__ = "          ";
    auto spacer = temp__.c_str();
    
    time_t t = time(nullptr);
    tm* now = localtime(&t);
    
    printf(
           "\n%sCPH for C++\n%s        by Oein\n\n Running test on %4d/%2d/%2d %2d:%2d:%2d\n\n",
           spacer,
           spacer,
           now->tm_year + 1900,
           now->tm_mon + 1,
           now->tm_mday,
           now->tm_hour,
           now->tm_min,
           now->tm_sec
           );
    
    free(now);
}
void getTestCases() {
    for (const FileEntry &entry : fs::directory_iterator(testCasesPath))
        if(entry.path().filename().extension().string() == testCaseInputExtension)
            inputFiles.push_back(entry);
    
    sort(inputFiles.begin(), inputFiles.end(), compareFileEntry);
}
void runTestcase(const FileEntry& entry) {
    string answerPath = getAnswerPath(entry);
    
    // Output
    fstream outputFile;
    outputFile.open(coutFilePath, ios::out);
    cout.rdbuf(outputFile.rdbuf());
    
    // Input
    ifstream inFile(entry.path().string());
    cin.rdbuf(inFile.rdbuf());
    
    // Eslaped Time
    steady_clock::time_point processStart = steady_clock::now();
    
    // Memory
    long startMemoryUsage = getMemoryUsage();
    
    // Threading
    future<int>* hThread = new future<int>(async(launch::async, solution));
    
    // Timeout
    bool timeouted = hThread->wait_for(milliseconds(TIMEOUT)) == future_status::timeout;
    
    // Memory
    long usedMemoryUsage = getMemoryUsage() - startMemoryUsage;
    
    // Close custom cout
    if(outputFile.is_open()) outputFile.close();
    cout.rdbuf(originalCout);
    
    delete hThread;
    
    printf(" %3lld. ",testedCount);
    
    if (timeouted) {
        printf("[❌] 시간초과.\n");
        return;
    }
    
    bool isAnswer = true;
    ifstream userAnswerStream(coutFilePath);
    
    if(!FileEntry(answerPath.c_str()).exists()) {
        if(is_empty_file(userAnswerStream)) isAnswer = 0;
    } else {
        // check two streams are the same
        ifstream realAnswerStream(answerPath);
        
        for(;1;) {
            if(!realAnswerStream || !userAnswerStream) {
                if(!realAnswerStream != !userAnswerStream)
                    isAnswer = 0;
                break;
            }
            
            realAnswerStream >> realAnswer;
            userAnswerStream >> userAnswer;
            
            if(realAnswer == userAnswer) continue;
            isAnswer = 0;
            break;
        }
    }
    
    long long tookNS = since(processStart);
    string units[] = {"bytes", "kilobytes", "megabytes", "gigabytes"};
    int unitIndex = 0;
    
    while (usedMemoryUsage >= 1024) {
        usedMemoryUsage /= 1024;
        unitIndex++;
    }
    
    
    printf(
        "[%s \t%s Took %lldms \t💾 %ld %s used\n",
        isAnswer ? "✅] 맞있습니다!" : "⚠️] 틀렸습니다.",
        clockByPercent(tookNS * 100 / TIMEOUT).c_str(),
        tookNS,
        usedMemoryUsage,
        units[unitIndex].c_str()
    );
}
void delete_dir_content(const fs::path& dir_path) {
    for (auto& path: fs::directory_iterator(dir_path)) {
        fs::remove_all(path);
    }
}
void fetchExample() {
    CURL *curl;
    CURLcode res;
    string readBuffer;
   
    curl_global_init(CURL_GLOBAL_DEFAULT);
   
    curl = curl_easy_init();
    string requestUrl = "https://api.jungol.co.kr/problem/" + to_string(PROBLEM);
    curl_easy_setopt(curl, CURLOPT_URL, requestUrl.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
    res = curl_easy_perform(curl);
    curl_easy_cleanup(curl);
    
    vector<uint8_t> vec(readBuffer.begin(), readBuffer.end());
    json j = json::from_bson(vec);
    long exampleIndex = 1;
    for(auto example : j["data"]["example"]) {
        ofstream inputFile(testCasesPath + "/" + to_string(exampleIndex) + ".in");
        inputFile << example["input"].get<string>();
        inputFile.close();
        
        ofstream outputFile(testCasesPath + "/" + to_string(exampleIndex) + ".out");
        outputFile << example["output"].get<string>();
        outputFile.close();
        
        exampleIndex++;
    }
}
void ensureExample() {
    FileEntry lastRequestExampleProblemFile(lastRequestedProblemPath);
    if(!lastRequestExampleProblemFile.exists()) {
        ofstream lastRequestExampleProblemStream(lastRequestedProblemPath);
        lastRequestExampleProblemStream << "-1";
        lastRequestExampleProblemStream.close();
    }
    
    ifstream lastRequestExampleProblemStream(lastRequestedProblemPath);
    string lastRequestExampleProblem;
    lastRequestExampleProblemStream >> lastRequestExampleProblem;
    lastRequestExampleProblemStream.close();
    if(lastRequestExampleProblem != to_string(PROBLEM)) {
        delete_dir_content(fs::path(testCasesPath));
        printf("[!] Fetch example data for problem %d\n\n",PROBLEM);
        fetchExample();
        
        ofstream lastRequestExampleProblemStream(lastRequestedProblemPath);
        lastRequestExampleProblemStream << PROBLEM;
        lastRequestExampleProblemStream.close();
    }
}
int main() {
    printInfo();
    ensureExample();
    getTestCases();
     
    for(const FileEntry & entry : inputFiles) {
        ++testedCount;
        runTestcase(entry);
    }
    
    printf("\n\n");
}
