#include<iostream>
#include<stdlib.h>
#include<time.h>
#include"cnf.h"
#include"util.h"
using namespace std;

int main() {
    CNF cnf = CNF("./data/g125.18.cnf");
    cout << "Clauses satisfied: " << cnf.count_satisfied() << endl;

//    cnf.frequence_based([](int p1, int p2){ return p1>p2; });
//    cout << "Clauses satisfied: " << cnf.count_satisfied() << endl;

//    cnf.tabu_search(60, 20);
//    cout << "Clauses satisfied: " << cnf.count_satisfied() << endl;

    cnf.local_search();
    cout << cnf.count_satisfied() << endl;

//    cnf.simulated_annealing(25000, 0.0008);
//    cout << "Clauses satisfied: " << cnf.count_satisfied() << endl;

    cout << "Number of clauses: " << cnf.clauses.size() << endl;
    return 0;
}
