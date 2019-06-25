#ifndef UTIL_H
#define UTIL_H

#include<string>
#include<sstream>
#include<vector>
using namespace std;

vector<string>
split(string sequence, char character) {

    stringstream stream(sequence);
    string segment;
    vector<string> splitted;

    while(getline(stream, segment, character))
       splitted.push_back(segment);

    return splitted;
}

#endif // UTIL_H
