#ifndef SYMBOLICEXPRESSIONPARSER_H_
#define SYMBOLICEXPRESSIONPARSER_H_

#include <string>
#include <sstream>
#include <list>
#include <regex>

using namespace std;

class ISymbolicExpression {
public:
  virtual ~ISymbolicExpression() {};
  virtual string ToString() { return ""; };
};

class Symbol : public ISymbolicExpression {
public:
    string content;
    Symbol(string content) {
      this->content = content;
    }

    string ToString() {
      return content;
    }
};

class SymbolList : public ISymbolicExpression {
  public:
    std::vector<ISymbolicExpression*> children;

    ~SymbolList() {
      for (auto child : children) {
        delete child;
      }
    }

    string ToString() {
      ostringstream output;
      output << "(";
      for (auto child : children) {
        output << child->ToString();
        if (child != children.back()) {
          output << " ";
        }
      }
      output << ")";
      return output.str();
    }
};

// https://stackoverflow.com/questions/15679672/recursive-parsing-for-lisp-like-syntax
class SymbolicExpressionParser {
  public:
    unique_ptr<SymbolList> ParseSymbolList(string input) {
      readTokens(input);
      return unique_ptr<SymbolList>(parseList());
    }
  private:
    list<string> tokens;

    void readTokens(string input) {
      regex r("[()]|[^ ()|]+|\\|[^ ]+\\|");
      smatch match;
      string::const_iterator searchStart(input.cbegin());
      while (regex_search(searchStart, input.cend(), match, r)) {
        searchStart = match.suffix().first;
        tokens.push_back(match[0].str());
      }
    }

    SymbolList* parseList() {
      tokens.pop_front();
      auto list = new SymbolList();
      while (tokens.front() != ")") {
        auto member = parseExpression();
        list->children.push_back(member);
      }
      tokens.pop_front();
      return list;
    }

    ISymbolicExpression* parseExpression() {
      if (tokens.front() == "(") {
        return parseList();
      } else {
        string symbol = tokens.front();
        tokens.pop_front();
        return new Symbol(symbol);
      }
    }
};

#endif // SYMBOLICEXPRESSIONPARSER_H_
