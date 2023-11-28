#include "defines.h"
#include "program.h"
#include "read.h"
#include "api.h"
#include <algorithm>
#include <catch2/catch_test_macros.hpp>
#include <fstream>

string writeTempFile(string content) {
  const string TEMP_FILE_NAME = "test-temp.lp";

  remove(TEMP_FILE_NAME.c_str());

  ofstream fileStream(TEMP_FILE_NAME);
  fileStream << content << endl;
  fileStream.close();

  return TEMP_FILE_NAME;
}

TEST_CASE("read correctly parses aspif format", "[read]") {
  Program *program = new Program();
  Api *api = new Api(program);
  Param *params = new Param();
  Read read(program, api, params);

  SECTION("should ignore comments") {
    string grounded = "asp 1 0 0\n"
                      "10 anything\n"
                      "0";
    read.read(writeTempFile(grounded));

    CHECK(program->atoms.size() == 0);
    CHECK(program->rules.size() == 0);
  }

  SECTION("parsing a fact") {
    string grounded = "asp 1 0 0\n"
                      "1 0 1 1 0 0\n"
                      "0";
    read.read(writeTempFile(grounded));

    SECTION("should add atom") {
      REQUIRE(program->number_of_atoms == 1);
      REQUIRE(program->atoms[0]->original_id == 1);
    }

    SECTION("should add rule") {
      auto rule = (BasicRule *)program->rules.front();

      REQUIRE(program->number_of_rules == 1);
      REQUIRE(rule->posBodyCount == 0);
      REQUIRE(rule->negBodyCount == 0);
      REQUIRE(rule->type == BASICRULE);
      REQUIRE(rule->head->original_id == 1);
      REQUIRE(rule->head == program->atoms[0]);
    }
  }

  SECTION("parsing a disjunctive rule") {
    string grounded = "asp 1 0 0\n"
                      "1 0 1 1 0 1 2\n"
                      "0";
    read.read(writeTempFile(grounded));

    auto atoms = program->atoms;

    SECTION("should add all atoms") {
      CHECK(program->number_of_atoms == 2);
      CHECK(atoms.size() == 2);
    }

    SECTION(
        "should add atoms in the order in which they appear in the program") {
      CHECK(atoms[0]->original_id == 1);
      CHECK(atoms[1]->original_id == 2);
    }

    SECTION("should assign internal ids") {
      CHECK(atoms[0]->id == 1);
      CHECK(atoms[1]->id == 2);
    }
  }

  SECTION("parsing a disjunctive rule with output lines") {
    string grounded = "asp 1 0 0\n"
                      "1 0 1 1 0 1 2\n"
                      "4 1 y 1 1\n"
                      "4 1 x 1 2\n"
                      "0";
    read.read(writeTempFile(grounded));

    SECTION("should assign names to atoms") {
      auto atoms = program->atoms;

      CHECK(atoms.size() == 2);
      CHECK(string(atoms[0]->name) == "y");
      CHECK(string(atoms[1]->name) == "x");
    }
  }

  // SECTION("should not allow disjunctive rules with multiple head atoms")
  // {
  //     string grounded =
  //         "asp 1 0 0\n"
  //         "1 0 2 1 2 0 1 3\n"
  //         "4 1 y 1 1\n"
  //         "4 1 x 1 2\n"
  //         "4 1 z 1 3\n"
  //         "0";
  //     REQUIRE_THROWS(read.read(writeTempFile(grounded)));
  // }

  SECTION("parsing a choice rule with no body") {
    string grounded = "asp 1 0 0\n"
                      "1 1 1 1 0 0\n"
                      "4 1 x 1 1\n"
                      "0";
    read.read(writeTempFile(grounded));

    SECTION("should create a ChoiceRule with no body") {
      auto atom = program->atoms[0];
      CHECK(atom->original_id == 1);

      auto rule = (ChoiceRule *)program->rules.front();
      CHECK(rule->head == atom);
      CHECK(rule->posBodyCount == 0);
      CHECK(rule->negBodyCount == 0);
      CHECK(rule->pbody == NULL);
      CHECK(rule->nbody == NULL);
      CHECK(rule->nend == NULL);
      CHECK(rule->pend == NULL);
      CHECK(rule->end == NULL);
    }
  }

  SECTION("parsing a choice rule with a body") {
    string grounded = "asp 1 0 0\n"
                      "1 1 1 3 0 2 -2 1\n"
                      "4 1 y 1 1\n"
                      "4 1 z 1 2\n"
                      "4 1 x 1 3\n"
                      "0";
    read.read(writeTempFile(grounded));

    SECTION("should create a ChoiceRule with a body") {
      auto xAtom = program->atoms[0];
      auto zAtom = program->atoms[1];
      auto yAtom = program->atoms[2];

      auto rule = (ChoiceRule *)program->rules.front();
      CHECK(rule->head == xAtom);
      CHECK(rule->pbody[0] == yAtom);
      CHECK(rule->nbody[0] == zAtom);
      CHECK(rule->posBodyCount == 1);
      CHECK(rule->negBodyCount == 1);
    }
  }

  SECTION("parsing a constraint rule") {
    string grounded = "asp 1 0 0\n"
                      "1 0 0 0 1 1\n"
                      "4 1 y 1 1\n"
                      "0";
    read.read(writeTempFile(grounded));

    SECTION("should populate false_atom") {
      auto false_atom = program->atoms[0];
      CHECK(false_atom->original_id == 0);
      CHECK(string(false_atom->name) == NEVER_ATOM);
      CHECK(false_atom->computeFalse == true);

      // TODO There shouldn't be a separate computeTrue and computeFalse
      CHECK(false_atom->computeTrue == false);
      CHECK(false_atom->computeTrue0 == false);

      CHECK(program->false_atom == program->atoms[0]);
    }

    SECTION("should create a basic rule with false_atom as head") {
      auto rule = (BasicRule *)program->rules.front();
      CHECK(rule->head == program->false_atom);
      CHECK(rule->type == BASICRULE);
      CHECK(rule->negBodyCount == 0);
      CHECK(rule->posBodyCount == 1);
      CHECK(program->atoms[1]->original_id == 1);
      CHECK(rule->posbody()[0] == program->atoms[1]);
    }
  }

  SECTION("parsing a weight rule") {
    string grounded = "asp 1 0 0\n"
                      // z :- { x=1, y=1} >= 1
                      "1 0 1 3 1 1 2 1 1 2 4\n"
                      "4 1 x 1 1\n"
                      "4 1 y 1 2\n"
                      "4 1 z 1 3\n"
                      "0";
    read.read(writeTempFile(grounded));

    auto rule = (WeightRule *)program->rules.front();

    auto zAtom = program->atoms[0];
    auto xAtom = program->atoms[1];
    auto yAtom = program->atoms[2];
    CHECK(xAtom->original_id == 1);
    CHECK(yAtom->original_id == 2);
    CHECK(zAtom->original_id == 3);

    SECTION("should set head") { CHECK(rule->head == zAtom); }

    SECTION("should set body with weights") {
      CHECK(rule->posBodyCount == 2);
      CHECK(rule->negBodyCount == 0);

      CHECK(rule->body[0] == xAtom);
      CHECK(rule->aux[0].weight == 1);

      CHECK(rule->body[1] == yAtom);
      CHECK(rule->aux[1].weight == 4);
    }
  }

  SECTION("parsing simple theory terms") {
    string grounded = "asp 1 0 0\n"
                      "9 0 1 5\n" // Numeric
                      "9 1 2 10 helloworld\n" // Symbolic
                      "0";
    read.read(writeTempFile(grounded));

    SECTION("should parse numeric term") {
      auto term = dynamic_cast<NumericTerm*>(program->theoryTerms[1]);
      REQUIRE(term != nullptr);
      CHECK(term->id == 1);
      CHECK(term->value == 5);
    }

    SECTION("should parse symbolic term") {
      auto term = dynamic_cast<SymbolicTerm*>(program->theoryTerms[2]);
      REQUIRE(term != nullptr);
      CHECK(term->id == 2);
      CHECK(term->name == "helloworld");
    }
  }

  SECTION("parsing theory tuple terms") {
    string grounded = "asp 1 0 0\n"
                      "9 0 1 5\n" // 5
                      "9 0 2 10\n" // 10
                      "9 0 3 15\n" // 15
                      "9 2 4 -1 3 1 2 3\n" // (5, 10, 15)
                      "9 2 5 -2 3 1 2 3\n" // {5, 10, 15}
                      "9 2 6 -3 3 1 2 3\n" // [5, 10, 15]
                      "0";
    read.read(writeTempFile(grounded));

    SECTION("should parse tuple term in parenthesis") {
      auto term = dynamic_cast<TupleTerm*>(program->theoryTerms[4]);
      REQUIRE(term != nullptr);
      CHECK(term->id == 4);
      CHECK(term->type == PARENTHESES);
      auto childTerms = vector<ITheoryTerm*>(term->children.begin(), term->children.end());
      CHECK(childTerms[0] == program->theoryTerms[1]);
      CHECK(childTerms[1] == program->theoryTerms[2]);
      CHECK(childTerms[2] == program->theoryTerms[3]);
    }

    SECTION("should parse tuple term in braces") {
      auto term = dynamic_cast<TupleTerm*>(program->theoryTerms[5]);
      REQUIRE(term != nullptr);
      CHECK(term->id == 5);
      CHECK(term->type == BRACES);
      auto childTerms = vector<ITheoryTerm*>(term->children.begin(), term->children.end());
      CHECK(childTerms[0] == program->theoryTerms[1]);
      CHECK(childTerms[1] == program->theoryTerms[2]);
      CHECK(childTerms[2] == program->theoryTerms[3]);
    }

    SECTION("should parse tuple term in brackets") {
      auto term = dynamic_cast<TupleTerm*>(program->theoryTerms[6]);
      REQUIRE(term != nullptr);
      CHECK(term->id == 6);
      CHECK(term->type == BRACKETS);
      auto childTerms = vector<ITheoryTerm*>(term->children.begin(), term->children.end());
      CHECK(childTerms[0] == program->theoryTerms[1]);
      CHECK(childTerms[1] == program->theoryTerms[2]);
      CHECK(childTerms[2] == program->theoryTerms[3]);
    }
  }

  SECTION("parsing theory compound terms") {
    string grounded = "asp 1 0 0\n"
                      "9 0 1 5\n" // 5
                      "9 1 2 1 x\n" // x
                      "9 1 3 1 +\n" // +
                      "9 2 4 3 2 1 2\n" // 5 + x
                      "0";
    read.read(writeTempFile(grounded));

    SECTION("should parse binary operation") {
      auto term = dynamic_cast<ExpressionTerm*>(program->theoryTerms[4]);
      REQUIRE(term != nullptr);
      CHECK(term->id == 4);
      CHECK(term->operation == program->theoryTerms[3]);
      auto childTerms = vector<ITheoryTerm*>(term->children.begin(), term->children.end());
      CHECK(childTerms[0] == program->theoryTerms[1]);
      CHECK(childTerms[1] == program->theoryTerms[2]);
    }
  }

  SECTION("Theory atom elements") {
    // TODO test theory atom elements
  }

  SECTION("Theory statements") {
    // TODO test theory statements
  }
}
