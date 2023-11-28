#include "solver/smtprocess.h"
#include "symbolicexpressionparser.h"
#include "smtstringhelpers.h"

namespace bp = boost::process;
using namespace chrono;

SMTProcess::SMTProcess(SMTSolverCommand type) {
  solverOption = type;

  string solverCommand;
  if (type == CVC4)
    solverCommand =
        "cvc4 --lang smt --output-lang smt --incremental";
  else if (type == CVC5)
    solverCommand =
        "cvc5 --lang smt --output-lang smt --incremental";
  else if (type == Z3)
    solverCommand = "z3 -smt2 -in";
  else if (type == YICES)
    solverCommand = "yices-smt2 --incremental";

  VLOG(3) << "Starting child process for solver: " << solverCommand;
  process = bp::child(solverCommand, bp::std_out > output, bp::std_in < input);
}

SMTProcess::SMTProcess(string customSolverCommand) {
  VLOG(3) << "Starting child process for solver: " << customSolverCommand;
  process = bp::child(customSolverCommand, bp::std_out > output, bp::std_in < input);
}

void SMTProcess::Send(string body) {
  if (VLOG_IS_ON(3)) {
    // Use cout because body might exceed the VLOG() max line length
    cout << "Sending to SMT process: " << body << endl;
  }
  input << body << endl;
}

unique_ptr<SolverResult> SMTProcess::CheckSatAndGetAssignments(list<Atom*> &atoms, list<SymbolicTerm*> &constraintVariables, list<MinimizationStatement*> &minimizations, Param &params) {
  auto result = unique_ptr<SolverResult>(new SolverResult());

  auto solveStart = high_resolution_clock::now();
  Send("(check-sat)");
  auto solveEnd = high_resolution_clock::now();
  result->solveDuration = duration_cast<milliseconds>(solveEnd - solveStart);

  string satResult;
  std::getline(output, satResult);
  VLOG(3) << "Read check sat result: " << satResult;
  if (satResult != "unsat" && satResult != "sat") {
    LOG(FATAL) << "Got unexpected result from SMT solver: " << satResult;
  }

  if (satResult == "unsat") {
    result->isSatisfiable = false;
    return result;
  }

  result->isSatisfiable = true;

  auto getValuesStart = high_resolution_clock::now();

  // Atoms
  list<string> atomNames;
  transform(atoms.begin(), atoms.end(), std::back_inserter(atomNames), [](Atom* a) { return a->getSmtName(); });
  auto rawAtomAssignments = getRawAssignments(atomNames);
  string valueString;

  if (params.debug_file.length() > 0){
    system(("rm -f " + params.debug_file).c_str());
    for (auto atom_name: atomNames){
      if (rawAtomAssignments[atom_name] == "true"){
        valueString = ":- not " + atom_name + ".";
      }
      else{
        valueString = ":- " + atom_name + ".";
      }
      system(("echo \"" + valueString + "\" >> " + params.debug_file).c_str());
    }
  }

  for (Atom* atom : atoms) {
    string atomName = atom->getSmtName();
    if (rawAtomAssignments.find(atomName) != rawAtomAssignments.end()) {
      result->atomAssignments[atom] = rawAtomAssignments[atomName] == "true" ? true : false;
    }
  }

  // Constraint variables
  list<string> variableNames;
  transform(constraintVariables.begin(), constraintVariables.end(), std::back_inserter(variableNames), [](SymbolicTerm* t) { return t->name; });
  auto rawVariableAssignments = getRawAssignments(variableNames);
  if (params.debug_file.length() > 0){
    for (auto var_name: variableNames){
      valueString = "&sum {" + var_name + "} = " + rawVariableAssignments[var_name] + ".";
      system(("echo \"" + valueString + "\" >> " + params.debug_file).c_str());
    }
  }
  for (SymbolicTerm* variable : constraintVariables) {
    if (rawVariableAssignments.find(variable->name) != rawVariableAssignments.end()) {
      result->constraintVariableAssignments[variable] = rawVariableAssignments[variable->name];
    }
  }

  // Minimization statements
  list<string> minimizationAtomNames;
  transform(minimizations.begin(), minimizations.end(), std::back_inserter(minimizationAtomNames), [](MinimizationStatement* m) { return m->getAtomName(); });
  auto rawMinimizationAssignments = getRawAssignments(minimizationAtomNames);
  for (auto minimization : minimizations) {
    string atomName = minimization->getAtomName();
    if (rawMinimizationAssignments.find(atomName) != rawMinimizationAssignments.end()) {
      result->minimizationAtomAssignments[minimization] = rawMinimizationAssignments[atomName];
    }
  }

  auto getValuesEnd = high_resolution_clock::now();
  result->getValuesDuration = duration_cast<milliseconds>(getValuesEnd - getValuesStart);

  return result;
}

map<string, string> SMTProcess::getRawAssignments(list<string> &variableNames) {
  if (variableNames.empty()) {
    return map<string, string>();
  }

  stringstream getValueStatement;
  getValueStatement << "(get-value (";
  for (string variableName : variableNames) {
    getValueStatement << SMT::VarUnsafe(variableName) << " ";
  }
  getValueStatement << "))";

  Send(getValueStatement.str());

  // Concat multi-line output to single line
  stringstream singleLineStream;
  string line;
  while (std::getline(output, line)) {
    VLOG(3) << "Read line from solver: " << line;
    singleLineStream << line;

    // This is a hacky workaround to handle z3's multiline output
    if (line[line.length() - 2] == ')' && line[line.length() - 1] == ')') {
      break;
    }
  }
  string assignmentsLine = singleLineStream.str();
  map<string,string> rawAssignments;

  SymbolicExpressionParser parser;
  auto assignmentsList = parser.ParseSymbolList(assignmentsLine);
  // cout << "to string: " << assignmentsList->ToString() << endl;
  for (auto a : assignmentsList->children) {
    auto assignment = dynamic_cast<SymbolList*>(a);
    auto variable = dynamic_cast<Symbol*>(assignment->children.front());
    auto value = dynamic_cast<ISymbolicExpression*>(assignment->children.back());

    VLOG(3) << "Parsed assignment: " << SMT::Unescape(variable->content) << "=" << value->ToString();
    rawAssignments[SMT::Unescape(variable->content)] = value->ToString();
  }

  return rawAssignments;
}
