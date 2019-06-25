#ifndef CNF_H
#define CNF_H

#include<iostream>
#include<fstream>
#include<vector>
#include<cmath>
#include<stdlib.h>
#include<time.h>
#include <limits.h>
#include "util.h"
#include <unordered_map>
#include <set>
using namespace std;
using bptr = bool*;

class CNF {
private:

    class var {
    public:
        var(bptr &varptr, bool flag): varptr(&varptr), flag(flag) {};
        bool value() { return flag == **varptr; }
        bptr* varptr;
        bool flag;
    };

    class clause {
    private:
        bool last_eval, changed;
    public:
        clause(): last_eval(false), changed(true) {};
        vector<var> variables;
        bool evaluate();
    };

    struct satisfy_rec {
        set<clause*> t, f;
    };

    clause clause_from_string(string row);
    int num_variables, num_clauses;

public:
    bool FALSE = false, TRUE = true;
    vector<bptr> variables;
    vector<clause> clauses;
    explicit CNF(const string& filename);
    bool evaluate();
    int count_satisfied();

    // Heuristics
    void local_search();
    void frequence_based(bool(*cmp)(int,int));

    // Metaheuristics
    bool simulated_annealing(double,double);
    void tabu_search(int,int);
};

// ------------------------------------------------------------------------------------------------------- CLASS METHODS
CNF::clause CNF::clause_from_string(string row) {
    clause c;
    vector<string> r = split(row, ' ');
    for (int i=0; i<r.size()-1; i++) {
        int variable = stoi(r[i]);
        c.variables.emplace_back(variables[abs(variable)-1], (variable>0));
    }
    return c;
}

bool CNF::clause::evaluate() {
    last_eval = false;
    for (auto& variable : variables) {
        if (variable.value()) {
            last_eval = true;
            break;
        }
    }
    return last_eval;
}

CNF::CNF(const string& filename) {

    ifstream is(filename);
    string row;
    srand(time(nullptr));

    while(getline(is, row))
    {
        // Character 'c' denotes a flag for a comment line
        if (row[0]=='c') {
            continue;
        // Character 'p' denotes a flag for a parameter line
        } else if (row[0]=='p') {
            vector<string> r = split(row, ' ');
            num_variables = stoi(r[2]);
            num_clauses = stoi(r[3]);

            for (int i=0; i<num_variables; i++) {
//                variables.push_back(&FALSE);
                variables.push_back(((double)rand()/(RAND_MAX)>0.5) ? &TRUE: &FALSE);
            }
            continue;
        // Character '%' denotes an EOF flag
        } else if (row[0] == '%') {
            break;
        }
        clauses.push_back(clause_from_string(row));
    }

    is.close();
}

bool CNF::evaluate() {
    for (auto& it : clauses)
        if (!it.evaluate())
            return false;
    return true;
}

int CNF::count_satisfied() {
    int satisfied = 0;
    for (auto& it : clauses)
        if (it.evaluate())
            satisfied++;
    return satisfied;
}

// ---------------------------------------------------------------------------------------------------------- HEURISTICS
void
CNF::local_search() {

    int best = count_satisfied();
    bool changed = true;

    while (changed) {
        changed = false;

        for (int i=0; i<variables.size(); i++) {
            variables[i] = (variables[i]==&TRUE) ? &FALSE : &TRUE;
            int current_result = count_satisfied();

            if (current_result > best) {
                best = current_result;
                changed = true;
            } else {
                variables[i] = (variables[i]==&TRUE) ? &FALSE : &TRUE;
            }
        }
    }
}

void
CNF::frequence_based(bool(*cmp)(int p1, int p2)) {

    vector<bool> satisfied(clauses.size(), false);
    unordered_map<bptr*, satisfy_rec> presence;

    // Map count of clauses which are satisfied for true and false assignments for each variable
    for (int i=0; i<clauses.size(); i++) {
        for (auto &it : clauses[i].variables) {
            unordered_map<bptr*,satisfy_rec>::const_iterator key = presence.find(it.varptr);

            if (key == presence.end())
                presence[it.varptr] = satisfy_rec();

            if (it.flag)
                presence[it.varptr].t.insert(&clauses[i]);
            else
                presence[it.varptr].f.insert(&clauses[i]);

            if (it.value())
                satisfied[i] = true;
        }
    }

    // Variable initialization for the code block below
    bptr* max_var = nullptr;
    int max_count = (cmp(0,1) ? INT_MAX : INT_MIN);
    bool value = false;
    int best = count_satisfied(), current = 0;

    while (true) {

        // Choose variable that is present in the most clauses and value which satisfies most clauses
        for (auto& it : presence) {
            int t = it.second.t.size(), f = it.second.f.size();
            if (cmp(f,t) && cmp(f,max_count)) {
                max_var = it.first;
                max_count = f;
                value = false;
            } else if (cmp(t,f) && cmp(t,max_count)) {
                max_var = it.first;
                max_count = t;
                value = true;
            }
        }
        *max_var = value ? &TRUE : &FALSE;
        current = count_satisfied();

        // If new swap does not any new clause or even makes the result worse then revert the move and stop search
        if (cmp(current,best)) {
            best = current;
        } else {
            *max_var = value ? &TRUE : &FALSE;
            break;
        }
        if (best == clauses.size()) break;
    }
}

// ------------------------------------------------------------------------------------------------------ METAHEURISTICS
void
CNF::tabu_search(int restriction, int runs) {

    vector<int> restricted(variables.size(), 0);
    int best = count_satisfied();
    int r = runs;
    vector<bool> best_values(variables.size());

    // Creates a snapshot of the initial (best) result
    for (int i=0; i<best_values.size(); i++)
        best_values[i] = (variables[i] == &TRUE);

    while (r > 0) {
        cout << best << endl;
        // Variable declaration for the tabu search to take place taking into account the aspiration criteria
        int index = -1, tabu_index = -1;
        int neighbourhood_best = INT_MIN, tabu_nb = INT_MIN;
        int current = count_satisfied(), temp;

        for (int i=0; i<variables.size(); i++) {
            // Make move
            variables[i] = (variables[i]==&TRUE) ? &FALSE : &TRUE;
            temp = count_satisfied();

            // Finding maximum result in the node neighborhood
            if (restricted[i] == 0) {
                if (temp > neighbourhood_best) {
                    index = i; neighbourhood_best = temp;
                }
            // Aspiration criteria
            } else {
                if (temp > tabu_nb) {
                    tabu_index = i; tabu_nb = temp;
                }
                restricted[i]--;
            }
            // Revert move
            variables[i] = (variables[i]==&TRUE) ? &FALSE : &TRUE;
        }
        if (current > neighbourhood_best) { // All of the neighborhood solutions
            if (tabu_nb > best) {
                // Aspiration criteria met
                best = tabu_nb;
                restricted[tabu_index] = restriction;
                variables[tabu_index] = (variables[tabu_index]==&TRUE) ? &FALSE : &TRUE;
                r = runs;

                // Store the best result found
                for (int i=0; i<best_values.size(); i++)
                    best_values[i] = (variables[i] == &TRUE);

            } else {
                // Copy the best result to the instance
                for (int i=0; i<best_values.size(); i++)
                    variables[i] = (best_values[i]) ? &TRUE : &FALSE;
                return;
            }
        }
        else {
            restricted[index] = restriction;
            variables[index] = (variables[index]==&TRUE) ? &FALSE : &TRUE;

            if (neighbourhood_best > best) {
                best = neighbourhood_best; r = runs;

                // Store the best result found
                for (int i=0; i<best_values.size(); i++)
                    best_values[i] = (variables[i] == &TRUE);
            }
        }
        r--;
    }
    // Copy the best result to the instance
    for (int i=0; i<best_values.size(); i++)
        variables[i] = (best_values[i]) ? &TRUE : &FALSE;
}

bool
CNF::simulated_annealing(double temp, double cooling_rate) {

    srand(time(nullptr));
    int best_result = count_satisfied();
    int current_result = best_result;

    while (temp >= 1) {
        cout << "Current temperature:" << temp << endl;
        int i = rand()%num_variables;
        variables[i] = (variables[i]==&TRUE) ? &FALSE : &TRUE;
        int neighbour_result = count_satisfied();

        if (neighbour_result >= current_result) {
            current_result = neighbour_result;
        } else {
            double prob = 1./(exp((neighbour_result-current_result)/temp));
            double random = ((double) rand() / (RAND_MAX));

            if (random >= prob) {
                current_result = neighbour_result;
            }
        }

        if (neighbour_result != current_result) {
            variables[i] = (variables[i]==&TRUE) ? &FALSE : &TRUE;
        }
        best_result = max(best_result, current_result);

        if (best_result == num_clauses) {
            return true;
        }
        temp *= (1-cooling_rate);
    }

    return (best_result == num_clauses);
}

#endif // CNF_H
