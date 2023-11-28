#ifndef LOGIC_H_
#define LOGIC_H_


#include <sstream>
#include "program.h"

class ILogic {
public:
  virtual ~ILogic() {};
  virtual string SMT_LOGIC_NAME() = 0;
  virtual void processTheoryStatements(list<TheoryStatement*> statements) = 0;

  virtual void getDeclarationStatements(std::ostringstream &output) = 0;
  virtual void getAssertionStatements(std::ostringstream &output) = 0;
  virtual list<SymbolicTerm*> getConstraintVariables() = 0;
};

#endif // LOGIC_H_
