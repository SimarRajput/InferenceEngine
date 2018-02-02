#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <sstream>
#include <map>

using namespace std;

// Check if argument is variable
bool IsVariable(string argument)
{
    if(isupper(argument[0]))
        return false;
    else
        return true;
}

struct Predicate
{
    bool negated;
    string predicateName;
    vector<string> arguments;

    //Constructor
    Predicate(string &query, int index)
    {
        //Negation
        char firstChar = query[0];
        if (firstChar == '~')
            negated = true;
        else
            negated = false;

        //PredicateName
        size_t indexOfLParan = query.find('(') + 1;
        size_t indexOfRParan = query.find(')');

        predicateName = query.substr(0, indexOfLParan - 1);

        //Arguments
        arguments = FindArguments(query, indexOfLParan, indexOfRParan, index);
    }

    vector<string> FindArguments(string query, size_t indexOfLParan, size_t indexOfRParan, int index)
    {
        vector<string> predicateArgument;
        string parameters = query.substr(indexOfLParan, indexOfRParan - indexOfLParan);
        stringstream paramStream(parameters);

        while (paramStream.good())
        {
            string substr;
            getline(paramStream, substr, ',');

            if(isspace(substr[0]))
                substr = substr.substr(1);

            if(IsVariable(substr))
                predicateArgument.push_back(substr + to_string(index));
            else
                predicateArgument.push_back(substr);
        }
        return predicateArgument;
    }
};

struct Sentence
{
    vector<Predicate> predicateVector;
};

class KnowledgeBase
{
public:
    int noOfQueries = 0;
    int noOfKbSentences = 0;
    vector<string> inputQueries;
    vector<Sentence> knowledgeBase;
    map<string, vector<int>> knowledgeBaseMap;

    vector<string> GetPredicateList(string fact)
    {
        vector<string> predicateList;
        stringstream predicateStream(fact);

        while (predicateStream.good())
        {
            string substr;
            getline(predicateStream, substr, '|');
            if(isspace(substr[0]))
                substr = substr.substr(1);
            predicateList.push_back(substr);
        }
        return predicateList;
    }
};

KnowledgeBase _knowledgeBase;
KnowledgeBase _mainKnowledgeBase;

// Function to negate Predicate
Predicate NegatePredicate(Predicate predicate)
{
    predicate.negated = !predicate.negated;
    if(predicate.predicateName[0] == '~')
        predicate.predicateName = predicate.predicateName.substr(1);
    else
        predicate.predicateName = "~" + predicate.predicateName;

    return predicate;
}

// Function to read input file and fill Knowledge Base structure.
void ReadFile()
{
    string arguments;
    ifstream inputFile;
    inputFile.open("input.txt");
    if (inputFile.is_open())
    {
        getline(inputFile, arguments);
        _mainKnowledgeBase.noOfQueries = stoi(arguments);

        for(int i = 0; i < _mainKnowledgeBase.noOfQueries;i++)
        {
            getline(inputFile, arguments);
            _mainKnowledgeBase.inputQueries.push_back(arguments);
        }

        getline(inputFile, arguments);
        _mainKnowledgeBase.noOfKbSentences = stoi(arguments);

        for (int i = 0; i < _mainKnowledgeBase.noOfKbSentences; i++)
        {
            getline(inputFile, arguments);
            Sentence sentence;
            vector<Predicate> predicateVector;
            vector<string> predicateList = _mainKnowledgeBase.GetPredicateList(arguments);

            for(int j = 0; j < predicateList.size(); j++)
            {
                Predicate predicate = Predicate(predicateList[j], i);
                predicateVector.push_back(predicate);

                if(_mainKnowledgeBase.knowledgeBaseMap.find(predicate.predicateName) == _mainKnowledgeBase.knowledgeBaseMap.end())
                {
                    vector<int> locationList;
                    locationList.push_back(i);
                    _mainKnowledgeBase.knowledgeBaseMap[predicate.predicateName] = locationList;
                }
                else
                {
                    vector<int> locationList = _mainKnowledgeBase.knowledgeBaseMap[predicate.predicateName];
                    locationList.push_back(i);
                    _mainKnowledgeBase.knowledgeBaseMap[predicate.predicateName] = locationList;
                }
            }
            sentence.predicateVector = predicateVector;
            _mainKnowledgeBase.knowledgeBase.push_back(sentence);
        }
        inputFile.close();
    }
}

// Check if predicate name is negated
bool IfNegatedPredicates(string string1, string string2)
{
    if (string1 == "~" + string2)
        return true;
    if (string2 == "~" + string1)
        return true;
    return false;
}

// Check if predicates name are equal
bool IfEqualPredicates(string string1, string string2)
{
    if (string1 == "~" + string2)
        return true;
    if (string2 == "~" + string1)
        return true;
    if (string1 == string2)
        return true;
    return false;
}

// Substitution from Constant to Variable
bool DoConstantToVariableSubs(string argument1, string argument2, map<string, string> &substitutionMap)
{
    if (substitutionMap.find(argument2) != substitutionMap.end())
    {
        if (IsVariable(substitutionMap[argument2]))
        {
            substitutionMap[argument2] = argument1;
            return true;
        }
        if (substitutionMap[argument2] != argument1)
        {
            return false;
        }
        return true;
    }
    else
    {
        substitutionMap[argument2] = argument1;
        return true;
    }
}

// Substitution from Variable To Constant
bool DoVariableToConstantSub(string argument1, string argument2, map<string, string> &substitutionMap)
{
    if (substitutionMap.find(argument1) != substitutionMap.end())
    {
        if (IsVariable(substitutionMap[argument1]))
        {
            substitutionMap[argument1] = argument2;
            return true;
        }
        if (substitutionMap[argument1] != argument2)
        {
            return false;
        }
        return true;
    }
    else
    {
        substitutionMap[argument1] = argument2;
        return true;
    }
}

// Substitution from Variable to Variable
bool DoVariableToVariableSubs(string argument1, string argument2, map<string, string> &substitutionMap)
{
    if (substitutionMap.find(argument1) != substitutionMap.end())
    {
        if (substitutionMap.find(argument2) != substitutionMap.end())
        {
            if (substitutionMap[argument1] == substitutionMap[argument2])
            {
                return true;
            }
            return false;
        }
        substitutionMap[argument2] = substitutionMap[argument1];
        return true;
    }
    else
    {
        if (substitutionMap.find(argument2) != substitutionMap.end())
        {
            substitutionMap[argument1] = substitutionMap[argument2];
            return true;
        }
        substitutionMap[argument1] = argument2;
        substitutionMap[argument2] = argument2;
        return true;
    }
}

// Substitution from Constant to Constant
bool DoConstantToConstantSubs(string argument1, string argument2, map<string, string> &substitutionMap)
{
    if (argument1 == argument2)
    {
        substitutionMap[argument1] = argument2;
        return true;
    }
    return false;
}

// Do Substitution
bool DoSubstitution(Predicate predicate, Sentence sentence, map<string, string> &substitutionMap)
{
    vector<bool> cToCBoolList;
    for (int i = 0; i < sentence.predicateVector.size(); i++)
    {
        if (IfNegatedPredicates(predicate.predicateName, sentence.predicateVector[i].predicateName))
        {
            cToCBoolList.push_back(true);
            for (int j = 0; j < predicate.arguments.size(); j++)
            {
                bool ifSubs = true;
                // Check for Variable
                if (IsVariable(predicate.arguments[j]))
                {
                    // Update Substitution Map for variables i.e Variable to Constant and Variable to Variable
                    if (IsVariable(sentence.predicateVector[i].arguments[j]))
                    {
                        ifSubs = DoVariableToVariableSubs(predicate.arguments[j], sentence.predicateVector[i].arguments[j], substitutionMap);
                    }
                    else
                    {
                        ifSubs = DoVariableToConstantSub(predicate.arguments[j], sentence.predicateVector[i].arguments[j], substitutionMap);
                    }
                }
                else
                {
                    // Update Substitution Map for constants i.e Constant to Constant and Constant to Variable
                    if (IsVariable(sentence.predicateVector[i].arguments[j]))
                    {
                        ifSubs = DoConstantToVariableSubs(predicate.arguments[j], sentence.predicateVector[i].arguments[j], substitutionMap);
                    }
                    else
                    {
                        ifSubs = DoConstantToConstantSubs(predicate.arguments[j], sentence.predicateVector[i].arguments[j], substitutionMap);
                    }
                }

                if(!ifSubs)
                cToCBoolList.back() = ifSubs;
            }
        }
    }

    if(cToCBoolList.size() > 0)
    {
        for(int i = 0; i < cToCBoolList.size(); i++)
        {
            if(cToCBoolList[i] == true)
            return true;
        }
    }
    else
        return true;

    
    return false;
}

// Function to Unify Sentences
map<string, string> Unify(Sentence querySentence, Sentence unifySentence, map<string, string> substitutionMap, int index)
{
    for (int i = 0; i < querySentence.predicateVector.size(); ++i)
    {
        // Check if Resolved or not after substitution
        bool ifResolved = DoSubstitution(querySentence.predicateVector[i], unifySentence, substitutionMap);
        if (!ifResolved)
        {
            substitutionMap.clear();
            break;
        }
    }
    return substitutionMap;
}

// Function to check if Predicates are equal
bool IfPredicatesEqual(Predicate predicate1, Predicate predicate2, map<string, string> substitutionMap)
{
    bool ifNegatedPredicates = IfNegatedPredicates(predicate1.predicateName, predicate2.predicateName);
    if (ifNegatedPredicates && (predicate1.arguments.size() == predicate2.arguments.size()))
    {
        for (int i = 0; i < predicate1.arguments.size(); i++)
        {
            bool ifArgumentsEqual = false;

            if (predicate1.arguments[i] == predicate2.arguments[i]) {
                ifArgumentsEqual = true;
            }

            if (substitutionMap.find(predicate1.arguments[i]) != substitutionMap.end() &&
                predicate2.arguments[i] == substitutionMap[predicate1.arguments[i]])
            {
                ifArgumentsEqual = true;
            }

            if (substitutionMap.find(predicate2.arguments[i]) != substitutionMap.end() &&
                predicate1.arguments[i] == substitutionMap[predicate2.arguments[i]])
            {
                ifArgumentsEqual = true;
            }

            if (substitutionMap.find(predicate2.arguments[i]) != substitutionMap.end() &&
                substitutionMap.find(predicate1.arguments[i]) != substitutionMap.end())
            {
                if (substitutionMap[predicate1.arguments[i]] == substitutionMap[predicate2.arguments[i]])
                {
                    ifArgumentsEqual = true;
                }
            }

            if(!ifArgumentsEqual)
                return false;
        }
        return true;
    }
    return false;
}

// Function to remove resoluted sentences
Sentence RemoveResolutedPredicates(Sentence sentence, map<string, string> substitutionMap)
{
	vector<int> removeLocation;
    bool ifResoluted = false;

	int predicateVectorSize = (int)sentence.predicateVector.size();

	for (int i = 0; i < predicateVectorSize; i++)
	{
		for (int j = i + 1; j < predicateVectorSize; j++)
		{
			if (i != j && IfPredicatesEqual(sentence.predicateVector[i], sentence.predicateVector[j], substitutionMap))
			{
				sentence.predicateVector.erase(sentence.predicateVector.begin() + j);
				sentence.predicateVector.erase(sentence.predicateVector.begin() + i);
				predicateVectorSize -= 2;
                ifResoluted = true;
			}
		}

        if(ifResoluted)
        {
            i = -1;
            ifResoluted = false;
        }
	}

    return sentence;
}

// Function to merge sentences and get new sentence
Sentence MergeSentences(Sentence querySentence, Sentence unifySentence, map<string, string> substitutionMap)
{
    Sentence newSentence;

    // Insert values from map to query predicates
    for (int i = 0; i < querySentence.predicateVector.size(); i++)
    {
        for (int j = 0; j < querySentence.predicateVector[i].arguments.size(); j++)
        {
            string argument = querySentence.predicateVector[i].arguments[j];
            if (substitutionMap.find(argument) != substitutionMap.end())
            {
                querySentence.predicateVector[i].arguments[j] = substitutionMap[argument];
            }
        }
    }

    // Insert values from map to unify predicates
    for (int i = 0; i < unifySentence.predicateVector.size(); i++)
    {
        for (int j = 0; j < unifySentence.predicateVector[i].arguments.size(); j++)
        {
            string argument = unifySentence.predicateVector[i].arguments[j];
            if (substitutionMap.find(argument) != substitutionMap.end())
            {
                unifySentence.predicateVector[i].arguments[j] = substitutionMap[argument];
            }
        }
    }

    // Add predicates of unify and query sentences to New Sentence
    for (int i = 0; i < querySentence.predicateVector.size(); i++) {
        newSentence.predicateVector.push_back(querySentence.predicateVector[i]);
    }

    for (int j = 0; j < unifySentence.predicateVector.size(); j++) {
        newSentence.predicateVector.push_back(unifySentence.predicateVector[j]);
    }

    newSentence = RemoveResolutedPredicates(newSentence, substitutionMap);

    return newSentence;
}

void Print(Sentence sentence)
{
    for(int i = 0; i < sentence.predicateVector.size(); i++)
    {
        cout<< sentence.predicateVector[i].predicateName;
        cout<<"(";
        for(int j = 0; j<sentence.predicateVector[i].arguments.size(); j++ )
        {
            cout << sentence.predicateVector[i].arguments[j] + ",";
        }
        cout<<")";
        cout<< " | ";
    }
}

// Function to add sentence and mapping for query sentence
void AddSentenceAndMapping(Sentence sentence)
{
    _knowledgeBase.knowledgeBase.push_back(sentence);
    _knowledgeBase.noOfKbSentences += 1;
    int newSentenceLocation = (int)_knowledgeBase.knowledgeBase.size() - 1;

    for(int i = 0; i < sentence.predicateVector.size(); i++)
    {
        if(_knowledgeBase.knowledgeBaseMap.find(sentence.predicateVector[i].predicateName) != _knowledgeBase.knowledgeBaseMap.end())
        {
            vector<int> locationList = _knowledgeBase.knowledgeBaseMap[sentence.predicateVector[i].predicateName];
            locationList.push_back(newSentenceLocation);

            _knowledgeBase.knowledgeBaseMap[sentence.predicateVector[i].predicateName] = locationList;
        }
        else
        {
            vector<int> locationList;
            locationList.push_back(newSentenceLocation);
            _knowledgeBase.knowledgeBaseMap[sentence.predicateVector[i].predicateName] = locationList;
        }
    }
}

// Run Resolution Algorithm
bool RunResolutionAlgorithm(Sentence querySentence, map<int, int> sentenceUsed)
{
//    Print(querySentence);
//    cout<< endl;

    // Initialize Substitution Map
    map<string, string> substitutionMap;
    
    if (querySentence.predicateVector.size() == 0)
        return true;

    // Loop for every Predicate in Query Sentence
    for(int i = 0; i < querySentence.predicateVector.size(); i++)
    {
        Predicate predicate = querySentence.predicateVector[i];
        predicate = NegatePredicate(predicate);

        // Get location of query predicate from Knowledge Map
        vector<int> locationList = _knowledgeBase.knowledgeBaseMap[predicate.predicateName];

        if(locationList.size() != 0)
        {
            Sentence finalSentence;
            for(int j = 0; j < locationList.size(); j++)
            {
                sentenceUsed[locationList[j]] = sentenceUsed[locationList[j]] + 1;

                // If sentence is use for more than 100 times loop out and backtrack
                if (sentenceUsed[locationList[j]] > 5)
                    continue;

                // Unify Sentence
                map<string,string> newsubstitutionMap = Unify(querySentence, _knowledgeBase.knowledgeBase[locationList[j]], substitutionMap, i);

                if (newsubstitutionMap.size() != 0)
                {
                    // Merge Sentences and find new sentence to be resolved
                    Sentence newSentence = MergeSentences(querySentence, _knowledgeBase.knowledgeBase[locationList[j]], newsubstitutionMap);

                    if (newSentence.predicateVector.size() == 0)
                        return true;

                    // Run resolution on new sentence
                    if(RunResolutionAlgorithm(newSentence, sentenceUsed))
                       return true;
                }
            }
        }
        else
        {
            return false;
        }
    }
    return false;
}

// Function to write output file.
void WriteOutputFile(vector<bool> finalResult)
{
    ofstream file;
    file.open("output.txt");

    for (int i = 0; i < finalResult.size(); i++)
    {
        if(finalResult[i])
            file << "TRUE" << endl;
        else
            file << "FALSE" << endl;
    }
    file.close();
}

int main(int argc, const char * argv[])
{
    // Global Main Knowledge Base Which will not be changed
    _mainKnowledgeBase = KnowledgeBase();

    // Global Knowledge Base Intialization
    _knowledgeBase = KnowledgeBase();

    // Read File, fill KB and initialize Maps
    ReadFile();

    vector<bool> finalResult;

    for(int i = 0; i < _mainKnowledgeBase.inputQueries.size(); i++)
    {
        _knowledgeBase = _mainKnowledgeBase;
        Sentence sentence;
        map<int, int> sentenceUsed;

        vector<string> predicateVector = _knowledgeBase.GetPredicateList(_knowledgeBase.inputQueries[i]);

        for(int j = 0; j < predicateVector.size(); j++)
        {
            Predicate predicate = Predicate(predicateVector[j], 0);
            predicate  = NegatePredicate(predicate);
            sentence.predicateVector.push_back(predicate);
        }
        
        // Initiialize Sentence Used for Every Resolution
        for (int h = 0; h < _knowledgeBase.knowledgeBase.size(); h++)
        {
            sentenceUsed[h] = 0;
        }

        AddSentenceAndMapping(sentence);
        // Run Resolution Algorithm
        bool result = RunResolutionAlgorithm(sentence, sentenceUsed);
		//cout << "--------------------------------------------------------------------" << endl << endl;
        finalResult.push_back(result);
    }
    WriteOutputFile(finalResult);
    return 0;
}
