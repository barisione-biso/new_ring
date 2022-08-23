
#include <fstream>
#include <bits/stdc++.h>
#include <ratio>
#include <chrono>
#include <ctime>
#include <cstring>
#include <string>
#include <iostream>
#include <map>
#include <cstdlib>
#include <libgen.h>

using namespace std;

void remove_extension(std::string& file_name){
    int position=file_name.find_last_of(".");
    //store the characters after the '.' from the file_name string
    file_name = file_name.substr(0, position - 1);
}

int main(int argc, char** argv)
{
    
    if (argc < 3) {
        cout << "Usage: " << argv[0] << " <results-file> <query-types-file>" << endl;
        exit(1);
    }

    string system_full_path(argv[1]);
    string path(dirname(argv[0]));
    string system(system_full_path.substr(system_full_path.find_last_of("/\\") + 1));  // name of the file (in the local directory) to process
    std::cout << "Generating input files for overleaf plot (SIGMOD21). Working with :" << system << std::endl;
    remove_extension(system);
    uint64_t nFiles = 0, total_size = 0, total_tuples = 0;

    vector<pair<string,string>> S;

    std::ifstream ifs_results(argv[1]);
    std::ifstream ifs_query_t(argv[2]);

    uint64_t qTime, number, results;
    string query_t;
    char semicolon;
    map<string, vector<float>> M;


    for (uint64_t j = 0; j < 850; j++) {
        ifs_results >> number >> semicolon >> results >> semicolon >> qTime;
	//cout << number << semicolon << results << semicolon << qTime << endl;
        ifs_query_t >> query_t;
        query_t.pop_back();
	query_t.pop_back();
	query_t.pop_back();
	query_t.pop_back();
        
	M[query_t].push_back((float)qTime/1000000000);
    }


    map<string, vector<float>>::iterator it1;



    for (it1 = M.begin(); it1 != M.end(); it1++) {
      
      {
            std::ofstream ofs(path+"/output/"+it1->first+"/"+system);
            ofs << "\\begin{filecontents}{"+it1->first+"-"+system+".dat}" << endl;
            for (uint64_t j = 0; j < (it1->second).size(); j++)
                ofs << (it1->second)[j] << endl;
            ofs << "\\end{filecontents}" << endl;
      }
    }


    return 0;
}

