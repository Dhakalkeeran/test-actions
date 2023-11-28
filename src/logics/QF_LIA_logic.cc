#include "logic.h"
#include "QF_LIA_logic.h"
#include "glog/logging.h"
#include "program.h"
#include <numeric>
#include "smtstringhelpers.h"

string QF_LIA_logic::SMT_LOGIC_NAME() { return "QF_LIA"; }

void QF_LIA_logic::processTheoryStatements(list<TheoryStatement*> statements) {
    this->statements = statements;

    for (auto statement : statements) {
        statement->traverseNestedTerms([&](ITheoryTerm* term) {
            if (auto s = dynamic_cast<SymbolicTerm*>(term)) {
                this->symbolicTerms[s->id] = s;
            }
        });
    }
}

void QF_LIA_logic::getDeclarationStatements(std::ostringstream &output) {
    for (const auto p : this->symbolicTerms) {
        auto term = p.second;
        output << SMT::DeclareConst(term->name, "Int");
    }
}

void QF_LIA_logic::getAssertionStatements(std::ostringstream &output) {
    for (const auto statement : statements) {
        if (statement->symbolicTerm->name == "sum")  {
            list<string> elements;
            for (const auto element : statement->leftElements) {
                elements.push_back(toString(element));
            }
            auto sumOfElements = SMT::Expr("+", elements, true);

            // HACK: SMT-LIB doesn't have a != operator
            string operation = statement->operation->name;
            if (operation == "!=") {
                operation = "distinct";
            }

            auto sumStatement = SMT::Expr(operation, {sumOfElements, SMT::ToString(statement->rightTerm)});

            auto assertion = SMT::Assert(SMT::Expr("=", {SMT::Var(statement->statementAtom), sumStatement}));
            output << assertion;
        }
        else if (statement->symbolicTerm->name == "dom") {
            auto singleElement = statement->leftElements.front();
            auto domainExpression = dynamic_cast<ExpressionTerm*>(singleElement->terms.front());
            NumericTerm* lowerBound = dynamic_cast<NumericTerm*>(domainExpression->children.front());
            NumericTerm* upperBound = dynamic_cast<NumericTerm*>(domainExpression->children.back());

            auto assertion = SMT::Assert(SMT::Expr("<=", {
                    SMT::ToString(lowerBound),
                    SMT::ToString(statement->rightTerm),
                    SMT::ToString(upperBound)
                }));
            output << assertion;
        }
        else {
            LOG(FATAL) << "The " << statement->symbolicTerm->name << " statement is not supported with the QF_LIA logic.";
        }
    }
}

list<SymbolicTerm*> QF_LIA_logic::getConstraintVariables() {
    list<SymbolicTerm*> variables;
    for (auto pair : symbolicTerms) {
        variables.push_back(pair.second);
    }
    return variables;
}

string QF_LIA_logic::toString(TheoryAtomElement* element) {
    if (element->literals.empty()) {
        list<string> terms;
        for (auto term : element->terms) {
            terms.push_back(SMT::ToString(term));
        }

        return SMT::Expr("+", terms, true);
    }

    LOG(FATAL) << "Not yet supported";
}
