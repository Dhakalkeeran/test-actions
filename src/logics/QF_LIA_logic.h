#ifndef QF_LIA_logic_H_
#define QF_LIA_logic_H_


#include <sstream>
#include "program.h"
#include "logic.h"

class QF_LIA_logic : public ILogic {
public:
    string SMT_LOGIC_NAME() override;
    void processTheoryStatements(list<TheoryStatement*> statements) override;
    void getDeclarationStatements(std::ostringstream &output) override;
    void getAssertionStatements(std::ostringstream &output) override;
    list<SymbolicTerm*> getConstraintVariables() override;

private:
    map<int, SymbolicTerm*> symbolicTerms;
    list<TheoryStatement *> statements;

    string toString(TheoryAtomElement* elements);
};

#endif // QF_LIA_logic_H_
