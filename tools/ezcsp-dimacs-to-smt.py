# class token:
# gType = "undefined"
# value
#    def __init__(self, gType, value):
#        self.gType = gType
#        self.value = value
import sys
import re

CSP_VAR = "CSP_VAR"
EZSMT_VAR = "EZSMT_VAR"
CSP_EXPRESSION = "CSP_EXPRESSION"

INTEGER_TYPE = "int"
REAL_TYPE = "real"
MIXED_TYPE = "mixed"

BOOL_OPERATOR = "BOOL_OPERATOR"
BOOL_COMPARE = "BOOL_COMPARE"

DIGIT = "DIGIT"
DISTINCT = "DISTINCT"
DOMAIN = "DOMAIN"
VARDOMAIN = "VARDOMAIN"
FUNCTION = "FUNCTION"
INT_COMPARE = "INT_COMPARE"
INT_OPERATOR = "INT_OPERATOR"
IDENTIFIER = "IDENTIFIER"
PAREN = "PAREN"
END_PAREN = "END_PAREN"
COMMA = "COMMA"

GLOBAL_OPERATOR = "GLOBAL_OPERATOR"
SUM_OPERATOR = "SUM_OPERATOR"
ALL_DIFFERENT_OPERATOR = "ALL_DIFFERENT_OPERATOR"

def domain(scanner, token):
    return DOMAIN, token


def vardomain(scanner, token):
    return VARDOMAIN, token

def bool_operator_and(scanner, token):
    return BOOL_OPERATOR, "and"


def bool_operator_or(scanner, token):
    return BOOL_OPERATOR, "or"


def bool_operator_xor(scanner, token):
    return BOOL_OPERATOR, "xor"


'''
Bool x Bool --> Bool
'''


def bool_compare_eq(scanner, token):
    return BOOL_COMPARE, "="


def bool_compare_impl_r(scanner,token):
    return BOOL_COMPARE, "=>"
    
'''    
def bool_compare_impl_l(scanner,token):
    return BOOL_COMPARE, ""
'''

# Any others here?

'''
Int x Int --> Bool
'''

def int_compare_neq(scanner, token):
    return INT_COMPARE, "distinct"

def int_compare_eq(scanner, token):
    return INT_COMPARE, "="

def int_compare_lt(scanner, token):
    return INT_COMPARE, "<"


def int_compare_gt(scanner, token):
    return INT_COMPARE, ">"


def int_compare_leq(scanner, token):
    return INT_COMPARE, "<="


def int_compare_geq(scanner, token):
    return INT_COMPARE, ">="


'''
INTEGER COMPARISONS TO IMPLEMENT
def int_compare_neq(scanner, token): return INT_COMPARE
'''

'''
Int x Int --> Int
'''


def int_operator_add(scanner, token):
    return INT_OPERATOR, "+"


def int_operator_sub(scanner, token):
    return INT_OPERATOR, "-"


def int_operator_mult(scanner, token):
    return INT_OPERATOR, "*"


def int_operator_div(scanner, token):
    return INT_OPERATOR, "/"


def int_operator_max(scanner, token):
    return INT_OPERATOR, "max"


def int_operator_min(scanner, token):
    return INT_OPERATOR, "min"


def global_constraint_operator_sum(scanner, token):
    return GLOBAL_OPERATOR, SUM_OPERATOR


def global_constraint_operator_all_different(scanner, token):
    return GLOBAL_OPERATOR, ALL_DIFFERENT_OPERATOR

'''
INTEGER OPERATORS TO ADD
def int_operator_abs(scanner, token): 
def int_operator_mod(scanner, token): 
def int_operator_pow(scanner, token): 
'''

def csp_var(scanner, token):
    return CSP_VAR, token


def ezsmt_var(scanner, token):
    return EZSMT_VAR, token


def csp_required_expression(scanner, token):
    return CSP_EXPRESSION, token


def identifier(scanner, token):
    return IDENTIFIER, token


#def function(scanner, token):
#    print(token)
#    return FUNCTION, token


def digit(scanner, token):
    return DIGIT, token


def paren(scanner, token):
    return PAREN, token


def end_paren(scanner, token):
    return END_PAREN, token


def comma(scanner, token):
    return None  #return COMMA #Token unnecessary

function_scanner = re.Scanner([
    (r"\(|\{", paren),
    (r"\)|\}", end_paren),
    (r",", comma),
    (r"[a-zA-Z0-9_]+", identifier),
])

function_scanner = re.Scanner([
    (r"[-]*\d+\.*\d*", digit),
    (r"[a-zA-Z0-9_]+", identifier),
    (r"\(|\{", paren),
    (r"\)|\}", end_paren),
    (r",", comma),
])

scanner = re.Scanner([
    #('["].*?["]',string),
    #(r"true|false", boolean),
    (r"cspdomain", domain),
    (r"cspvardomain", vardomain),
	
	# To be implemented:
	#(r"_distinct", distinct),

    # Integer Comparisons
    (r"ezcsp__eq", int_compare_eq),
    (r"ezcsp__lt", int_compare_lt),
    (r"ezcsp__leq", int_compare_leq),
    (r"ezcsp__gt", int_compare_gt),
    (r"ezcsp__geq", int_compare_geq),
    (r"ezcsp__neq", int_compare_neq),

    # Integer Operators
    (r"ezcsp__pl", int_operator_add),
    (r"ezcsp__mn", int_operator_sub),
    (r"ezcsp__tm", int_operator_mult),
    (r"ezcsp__dv", int_operator_div),
    # To be implemented
    #(r"ezcsp_mod",int_operator_mod),
    (r"ezcsp__max", int_operator_max), 
    (r"ezcsp__min", int_operator_min),
    #(r"ezcsp__abs", int_operator_abs),
    #(r"ezcsp__pow", int_operator_pow),
    #(r"ezcsp__exp", int_operator_exp),
    #(r"ezcsp__sin", int_operator_sin),
    #(r"ezcsp__cos", int_operator_cos),
    #(r"ezcsp__tan", int_operator_tan),
    
    (r"ezcsp__sum", global_constraint_operator_sum),
    (r"all_different", global_constraint_operator_all_different),


    # Boolean Comparison
    (r"_beq", bool_compare_eq),
    (r"ezcsp__impl_r", bool_compare_impl_r),

    # Boolean Operators
    (r"ezcsp__and", bool_operator_and),
    (r"ezcsp__or", bool_operator_or),
    (r"ezcsp__xor", bool_operator_xor),

    (r"[-]*\d+\.*\d*", digit),
    (r"required\(",csp_required_expression),
    (r"cspvar", csp_var),
    (r"ezsmtvar", ezsmt_var),
    #                       Integer       Constant           Variable       _  supremum  infimum
    #(r"_*[a-z][A-Za-z0-9]*\(([0-9]+|_*[a-z][A-Za-z0-9]*|_*[A-Z][A-Za-z0-9]*|_|#supremum|#infimum)(,([0-9]+|_*[a-z][A-Za-z0-9]*|_*[A-Z][A-Za-z0-9]*|_|#supremum|#infimum))*\)", function),
    #(r"(?:[^,()]+((?:\((?>[^()]+|\((?<open>)|\)(?<-open>))*\)))*)+", identifier),
    (r"[a-zA-Z0-9_]+\([a-zA-Z0-9_,]+\)", identifier),
    (r"\(|\{", paren),
    (r"\)|\}", end_paren),
    (r",", comma),
    #    (r";", end_statement),
    (r"[a-zA-Z0-9_]+", identifier),

    (r"\s+", None),
])

text = ""

# Retrieved from: http://stackoverflow.com/questions/8297424/use-z3-and-smt-lib-to-get-a-maximum-of-two-values
# Tested Independently.
smt_max_function = "(define-fun max ((x Int) (y Int)) Int\n\
  (ite (< x y) y x))\n"

smt_min_function = "(define-fun min ((x Int) (y Int)) Int\n\
  (ite (< x y) x y))\n"

smt_real_max_function = "(define-fun max ((x Real) (y Real)) Real\n\
  (ite (< x y) y x))\n"

smt_real_min_function = "(define-fun min ((x Real) (y Real)) Real\n\
  (ite (< x y) x y))\n"  


def smt_header(logic="LIA"):
    if logic != "AUFLIRA" and logic != "AUFNIRA":
    	print("\
(set-option :interactive-mode true) \n\
(set-option :produce-models true) \n\
(set-option :produce-assignments true) \n\
(set-option :print-success false) \n\n\
;Quantifier Free Linear Arithmetic \n\
(set-logic QF_{0})".format(logic))
    elif logic == "AUFLIRA":
    	print("\
(set-option :interactive-mode true) \n\
(set-option :produce-models true) \n\
(set-option :produce-assignments true) \n\
(set-option :print-success false) \n\n\
; \n\
(set-logic AUFLIRA)")
    else:
    	print("\
(set-option :interactive-mode true) \n\
(set-option :produce-models true) \n\
(set-option :produce-assignments true) \n\
(set-option :print-success false) \n\n\
; \n\
(set-logic AUFNIRA)")

	
    
    print("; --- END HEADER ---\n")
    
    print("; --- Including necessary SMT functions ---\n")
    if logic != "LRA" and logic != "AUFLIRA" and logic !="NRA":
        print(smt_max_function)
        print(smt_min_function)
    else:
        print(smt_real_max_function)
        print(smt_real_min_function)        
    print("; --- END INCLUDES ---\n")


def gather_dimacs_bools(lines):
    dimacs_bools = set()
    for line in lines:
        for bool_to_declare in line:
            bool_to_declare = bool_to_declare.strip("-")
            dimacs_bools.add(bool_to_declare)

    return dimacs_bools


def assert_cnf(lines):
    for line in lines:
        cnf = list()
        for bool_to_declare in line:
            if bool_to_declare[0] == "-":
                cnf.append("(not |%s|)" % bool_to_declare[1:])
            else:
                cnf.append("|%s|" % bool_to_declare)
        if len(cnf) == 1:
            print("(assert %s)" % cnf[0])
        else:
            print("(assert (or %s))" % " ".join(cnf))


def assert_statements(statements_to_assert):
    for statement in statements_to_assert:
        print("(assert (= |{0}| {1}))".format(statement[0], statement[1]))


def get_statement(tokens):
    for idx, token in enumerate(tokens, 0):
        try:
            if ((token[0] == INT_COMPARE or token[0] == BOOL_COMPARE or token[0] == INT_OPERATOR
                    or token[0] == BOOL_OPERATOR) and tokens[idx + 1][0] == PAREN):
                tokens[idx], tokens[idx + 1] = tokens[idx + 1], tokens[idx]
        except IndexError:
            print("Scanner found a syntax error at: %s" % declared_bool)
            raise
    statement_list = list()
    for token in tokens:
        if token[0] == IDENTIFIER:
            statement_list.append("|%s|" % token[1])
        else:
            statement_list.append(token[1])

    return " ".join(statement_list)


def declare_bools(dimacs_bools):
    for bool_to_declare in dimacs_bools:
        print("(declare-fun |%s| () Bool)" % bool_to_declare)
 

func_dict = dict()
atom_dict = dict()

'''
Necessary for the sum global constraint. Keep track of all predicates and their arity.
'''
def maintain_cspvars(id):
    func_tokens, remainder = function_scanner.scan(id)
    
    parameter_counter = 0
    paren_counter = -1 
    
    if not remainder == '':
        raise Exception("Scanner found a syntax error at: {0}" % id)
    elif not func_tokens[0][0] == IDENTIFIER:
        raise Exception("CSP Variable is not recognized: {0}" % id)
    for func_token in func_tokens[1:]:
        if func_token[0] == PAREN:
            paren_counter += 1
        elif func_token[0] == END_PAREN:
            paren_counter -= 1
        elif paren_counter == 0:
            parameter_counter += 1 
            
    if not func_tokens[0][1] in func_dict:
        func_dict[func_tokens[0][1]] = dict()
    if not parameter_counter in func_dict[func_tokens[0][1]]:
        func_dict[func_tokens[0][1]][parameter_counter] = list()
    
    func_dict[func_tokens[0][1]][parameter_counter].append(id)
    
def maintain_atoms(id):
    atom_tokens, remainder = function_scanner.scan(id)
    
    parameter_counter = 0
    paren_counter = -1 
    
    if not remainder == '':
        raise Exception("Scanner found a syntax error at: {0}" % id)
    
    for atom_token in atom_tokens[1:]:
        if atom_token[0] == PAREN:
            paren_counter += 1
        elif atom_token[0] == END_PAREN:
            paren_counter -= 1
        elif paren_counter == 0:
            parameter_counter += 1 
            
    if not atom_tokens[0][1] in atom_dict:
        atom_dict[atom_tokens[0][1]] = dict()
    if not parameter_counter in atom_dict[atom_tokens[0][1]]:
        atom_dict[atom_tokens[0][1]][parameter_counter] = list()
    
    atom_dict[atom_tokens[0][1]][parameter_counter].append(id)
    
def declare_ezsmt_var(tokens):
    assert(tokens[0][0] == EZSMT_VAR)
    assert(tokens[1][0] == PAREN)
    assert(tokens[3][0] == IDENTIFIER)
    
    if tokens[4][0] == END_PAREN:
        if tokens[2][1] == REAL_TYPE:
            declare_real(tokens[3][1])
        elif tokens[2][1] == INTEGER_TYPE:
            declare_int(tokens[3][1])
        else:
            raise Exception("The declared ezsmt variable is an unsupported type. Supported types are: 'real' and 'int'")
    else:
        if tokens[2][1] == REAL_TYPE:
            declare_bounded_real(tokens[3][1], tokens[4][1], tokens[5][1])
        elif tokens[2][1] == INTEGER_TYPE:
            declare_bounded_int(tokens[3][1], tokens[4][1], tokens[5][1])
        else:
            raise Exception("The declared ezsmt variable is an unsupported type. Supported types are: 'real' and 'int'")
    
      
        
def declare_int(id):
    maintain_cspvars(id)
    
    real_declaration = ("(declare-fun |{id}| () Int)\n")
    values = {'id': id}
    print(real_declaration.format(**values))    
    
    
def declare_bounded_int(id, lower_bound, uppper_bound):
    maintain_cspvars(id)
    
    int_declaration = ("(declare-fun |{id}| () Int)\n"
                       "(assert (<= {lower_bound} |{id}|))\n"
                       "(assert (>= {upper_bound} |{id}|))\n")
    if lower_bound[0]=='-':
    	values = {'id': id, 'lower_bound': "(- "+lower_bound[1:]+")", 'upper_bound': uppper_bound}
    else:
	values = {'id': id, 'lower_bound': lower_bound, 'upper_bound': uppper_bound}
    print(int_declaration.format(**values))    
    
    
def declare_real(id):
    maintain_cspvars(id)
    
    real_declaration = ("(declare-fun |{id}| () Real)\n")
    values = {'id': id}
    print(real_declaration.format(**values))
    
 
 
def declare_bounded_real(id, lower_bound, uppper_bound):
    maintain_cspvars(id)
    
    real_declaration = ("(declare-fun |{id}| () Real)\n"
                       "(assert (<= {lower_bound} |{id}|))\n"
                       "(assert (>= {upper_bound} |{id}|))\n")
    if lower_bound[0]=='-':
    	values = {'id': id, 'lower_bound': "(- "+lower_bound[1:]+")", 'upper_bound': uppper_bound}
    else:
	values = {'id': id, 'lower_bound': lower_bound, 'upper_bound': uppper_bound}
    print(real_declaration.format(**values))


def global_constraint_to_smt(smt_expression, tokens):
    if tokens[1][1] == SUM_OPERATOR:
        sum_tokens = tokens[3:-2]
        prefix = "list("
	# Two cases
        # Case 1: Expect list(csp_id,arity),ezcsp_INT_COMPARE,csp_id 
        # Case 2: Expect list(csp_id1(csp_id2(s)),arity),ezcsp_INT_COMPARE,csp_id3   where csp_id1() is declared as csp variable of the spicified arity whose parameter starts with csp_id2(s)
        # Case 3: Expect list(csp_id1(csp_id2(s)), num),ezcsp_INT_COMPARE,csp_id3   where csp_id1(csp_id2(s)) is a regular atom. The summation sums up the num-th parameter of csp_id1(csp_id2(s)).

        # We start with case 1
	if sum_tokens[0][1].startswith(prefix) and sum_tokens[0][0] == IDENTIFIER and sum_tokens[1][0] == INT_COMPARE and (sum_tokens[2][0] == IDENTIFIER or sum_tokens[2][0] == DIGIT):
           
            predicate_id, predicate_arity = sum_tokens[0][1][len(prefix):-1].split(',')
            predicate_arity = int(predicate_arity)
        
            # Ensure predicates exist in the program, otherwise the statement cannot satisfy
            if predicate_id in func_dict and predicate_arity in func_dict[predicate_id] and len(func_dict[predicate_id][predicate_arity]) > 0:        
                smt_expression.extend(["(",sum_tokens[1][1]," (+ "])
            
                for predicate in func_dict[predicate_id][predicate_arity]:
                    smt_expression.extend([" |",predicate,"| "])
                smt_expression.append(")")
            
                if sum_tokens[2][0] == IDENTIFIER:
                    smt_expression.extend([" |",sum_tokens[2][1],"| "])
                elif sum_tokens[2][0] == DIGIT:
                    if sum_tokens[2][1][0] == "-": 
                        smt_expression.extend([" (- ", sum_tokens[2][1][1:], ")"])
                    else:
                        smt_expression.extend([" ", sum_tokens[2][1]])
            
                smt_expression.append(")")
            else:
                smt_expression.append(" false")

	#case 2	
	# create another function dict for ground atoms.
	elif sum_tokens[0][1]=='list' and sum_tokens[0][0] == IDENTIFIER and sum_tokens[1][1]=='(' and sum_tokens[2][0] == IDENTIFIER and sum_tokens[3][0] == DIGIT and len(sum_tokens[2][1].split(',')) <= int(sum_tokens[3][1]) and sum_tokens[4][1] == ')' and sum_tokens[5][0] == INT_COMPARE and (sum_tokens[6][0] == IDENTIFIER or sum_tokens[6][0] == DIGIT):

            predicate_id= sum_tokens[2][1].split('(')[0]
            predicate_startswith =  sum_tokens[2][1].split('(')[1][:-1]
	    predicate_arity = sum_tokens[3][1]
            predicate_arity = int(predicate_arity)
	    para_num = int(sum_tokens[3][1])

            # Ensure predicates exist in the program, otherwise the statement cannot satisfy
            if predicate_id in func_dict: 
	    #case 2:
 		if predicate_arity in func_dict[predicate_id] and len(func_dict[predicate_id][predicate_arity]) > 0:        
                    #Ensure that predicates starting with given csp_id exist
                    predicates_exist = False
		    for predicate in func_dict[predicate_id][predicate_arity]:
		        if len(predicate.split('(')[1]) > len(predicate_startswith):
		            if predicate.split('(')[1].startswith(predicate_startswith):
			        predicates_exist = True
			        break
		    if predicates_exist:
		        smt_expression.extend(["(",sum_tokens[5][1]," (+ "])
            
                        for predicate in func_dict[predicate_id][predicate_arity]:
		            if len(predicate.split('(')[1]) > len(predicate_startswith):
		                if predicate.split('(')[1].startswith(predicate_startswith):
				    smt_expression.extend([" |",predicate,"| "])
                        smt_expression.append(")")
            
                        if sum_tokens[6][0] == IDENTIFIER:
                            smt_expression.extend([" |",sum_tokens[6][1],"| "])
                        elif sum_tokens[6][0] == DIGIT:
                            if sum_tokens[6][1][0] == "-": 
                                smt_expression.extend([" (- ", sum_tokens[6][1][1:], ")"])
                            else:
                                smt_expression.extend([" ", sum_tokens[6][1]])
            
                        smt_expression.append(")")
	  	    else:
		        smt_expression.append(" false")
	  	else:
		    smt_expression.append(" false")
            else:
	        #case 3
                #Ensure that predicates starting with given csp_id exist
                predicates_count = 0
	        for predicate_arity in atom_dict[predicate_id].iterkeys():
	            if predicate_arity >= para_num:
		        for predicate in atom_dict[predicate_id][predicate_arity]:
		            if len(predicate.split('(')[1]) > len(predicate_startswith):
		                if predicate.split('(')[1].startswith(predicate_startswith):
			            predicates_count += 1
			            if predicates_count >1:
					break
	        if predicates_count > 0:
		    smt_expression.extend(["(",sum_tokens[5][1]])
		    if predicates_count >1:
		        smt_expression.extend([" (+ "])
	            for predicate_arity in atom_dict[predicate_id].iterkeys():
                        for predicate in atom_dict[predicate_id][predicate_arity]:
			    if len(predicate.split('(')[1]) > len(predicate_startswith) and  predicate.split('(')[1].startswith(predicate_startswith):
				    #parse out arguments of the atom 
				    temptokens = predicate[len(predicate_id)+1:-1].split(',')
				    temptokens2 = []
				    tempstr = ''
				    pcounter =0
				    for token in temptokens:
				        pcounter += token.count('(')
				        pcounter -= token.count(')')
					if pcounter == 0:
					    if len(tempstr) > 0:
						tempstr += ','
					    tempstr += token
					    temptokens2.append(tempstr)
					    tempstr = ''
					else:
					    if len(tempstr) > 0:
						tempstr += ','
					    tempstr += token 
				    #print('\n\n\n')
				    #print(temptokens)
				    #print(temptokens2)
				    smt_expression.extend([" |", temptokens2[para_num-1] ,"| "])
                    if predicates_count >1:
			smt_expression.append(")")
            
                    if sum_tokens[6][0] == IDENTIFIER:
                        smt_expression.extend([" |",sum_tokens[6][1],"| "])
                    elif sum_tokens[6][0] == DIGIT:
                        if sum_tokens[6][1][0] == "-": 
                            smt_expression.extend([" (- ", sum_tokens[6][1][1:], ")"])
                        else:
                            smt_expression.extend([" ", sum_tokens[6][1]])
            
                    smt_expression.append(")")
	        else:
		    smt_expression.append(" false")
	else:
            raise Exception("Does not conform to ezcsp__sum specifications")


    elif tokens[1][1] == ALL_DIFFERENT_OPERATOR:
        different_token = tokens[3]
        prefix = "list("
        
        if not different_token[1].startswith(prefix) or different_token[0] != IDENTIFIER:
            raise Exception("Does not conform to all_different specifications")
        
        predicate_id, predicate_arity = different_token[1][len(prefix):-1].split(',')
        predicate_arity = int(predicate_arity)
        
        # Ensure predicates exist in the program, otherwise the statement cannot satisfy
        if predicate_id in func_dict and predicate_arity in func_dict[predicate_id] and len(func_dict[predicate_id][predicate_arity]) > 0:  
            smt_expression.append("(distinct ")
            for predicate in func_dict[predicate_id][predicate_arity]:
                    smt_expression.extend([" |",predicate,"| "])
            smt_expression.append(")")
        else:
            #Not sure if this can happen / is the correct method
            smt_expression.append(" true")

def regular_constraint_to_smt(smt_expression, tokens):
    paren_counter = 0
    for elem in tokens[1:-1]:
        if elem[0] == INT_COMPARE or elem[0] == INT_OPERATOR or elem[0] == BOOL_COMPARE or elem[0] == BOOL_OPERATOR:
            smt_expression.extend( ["(",elem[1]])
	    paren_counter += 1
        elif elem[0] == IDENTIFIER:
            smt_expression.extend([" |",elem[1],"| "])
        elif elem[0] == DIGIT:
            if elem[1][0] == "-": 
                smt_expression.extend([" (- ", elem[1][1:], ")"])
            else:
                smt_expression.extend([" ", elem[1]])
        elif elem[0] == END_PAREN:
            smt_expression.append(")")
	    paren_counter -= 1

    while True:
	if paren_counter > 0:
	    paren_counter -= 1
	    smt_expression.append(")")
	else:
	    break 
    


def csp_expression_to_smt(declared_bool, tokens):
    smt_expression = list("(assert (=> |" + declared_bool + "| ")

    try:
        if tokens[1][0] == GLOBAL_OPERATOR:
            global_constraint_to_smt(smt_expression, tokens)
        else:
            regular_constraint_to_smt(smt_expression, tokens)
    except:
        print("Error Statement: {}".format(declared_bool))
        raise

    smt_expression.append("))")
    print("".join(smt_expression))
        


def check_sat():
	print("\n; Check satisfiability")
	print("(check-sat)")
	print("; Comment if unsat occurs.")
	print("(get-model)")


SUPPORTED_LOGICS = {"linear-integer" : "LIA", "difference-logic" : "IDL", "linear-real" : "LRA", "r" : "LRA", "fd" :"LIA" , "nonlinear-integer" : "NIA","nonlinear-real" : "NRA", "mixed":"AUFLIRA"}


if __name__ == '__main__':
    if len(sys.argv) <= 1 or sys.argv[1] == '-h' or sys.argv[1] == '--help':
        print("Run example: python %s {cmodels-completion-output} [nonlinear|linear-integer|difference-logic|linear-real|nonlinear-integer|nonlinear-real|mixed]" % sys.argv[0])
        exit()
        #raise Exception("Invalid commandline arguments")
    #if len(sys.argv) >= 3:
    #    sys.stdout = open(sys.argv[2], 'w')

    fileName = sys.argv[1]

    with open(fileName) as f:
        lines = [line.replace('"','').split(" ")[:-1] for line in f.readlines()[1:]]
    f.closed
    
    #for line in lines:
    #    print(line)
    
    logic = None
    
    # Get all the bools in the clausified dimacs file
    dimacs_bools = gather_dimacs_bools(lines)
    
    cspdomain = None
    intList = []
    
    # Get the logic. If it is not specified in the arguments, 
    # we get it from the dimacs file according to token cspdomain().
    #If the logic is non-linear, set logic according to atom cspdomain() 
    if len(sys.argv) > 2 and sys.argv[2] != "nonlinear":
        if sys.argv[2] in SUPPORTED_LOGICS.keys():
	    if SUPPORTED_LOGICS[sys.argv[2]] == "LIA" or SUPPORTED_LOGICS[sys.argv[2]] == "IDL" or SUPPORTED_LOGICS[sys.argv[2]] == "NIA":
		logic = INTEGER_TYPE
	    elif SUPPORTED_LOGICS[sys.argv[2]] == "AUFLIRA":
	    	logic = MIXED_TYPE
	    else:
            	logic = REAL_TYPE
	else:	
	    raise Exception("Invalid commandline arguments: Logic not supported")
    else:  
        for dimacs_bool in dimacs_bools:
            tokens, remainder = scanner.scan(dimacs_bool)
            if remainder == '' and tokens[0][0] == DOMAIN:
                assert tokens[2][0] == IDENTIFIER
		cspdomain = tokens[2][1] 
                if SUPPORTED_LOGICS[cspdomain] == "LIA" or SUPPORTED_LOGICS[cspdomain] == "IDL":
                    logic = INTEGER_TYPE
            	elif SUPPORTED_LOGICS[cspdomain] == "AUFLIRA":
                    logic = MIXED_TYPE
                else:
                    logic = REAL_TYPE
                break
	# If we are in the mixed domains, then get the list of variables to
        # be declared as integers.
	if cspdomain == 'mixed':
            for dimacs_bool in dimacs_bools:
                tokens, remainder = scanner.scan(dimacs_bool)
                if remainder == '' and tokens[0][0] == VARDOMAIN:
                    assert tokens[2][0] == IDENTIFIER
		    if tokens[3][1] =='real':
                        continue
		    elif tokens[3][1] == 'int':
			intList.append(tokens[2][1])
		    else:
			raise Exception("Invalid cspvardomain: Please choose from either \"int\" or \"real\" ")
        

    # Writes the SMT header.
    if len(sys.argv) == 2:
	if cspdomain == None:
	    cspdomain = "fd"
        if cspdomain in SUPPORTED_LOGICS.keys():
	    smt_header(SUPPORTED_LOGICS[cspdomain])
		
        else:
	    raise Exception("Invalid cspdomain: Logic not supported")
    elif sys.argv[2] != "nonlinear" and sys.argv[2] in SUPPORTED_LOGICS.keys():
        smt_header(SUPPORTED_LOGICS[sys.argv[2]])
    elif sys.argv[2] == "nonlinear":
	if logic == INTEGER_TYPE:
		smt_header("NIA")
	elif logic == MIXED_TYPE:
		smt_header("AUFNIRA")
	elif logic == REAL_TYPE:
		smt_header("NRA")
    else:
        raise Exception("Invalid commandline arguments: Logic not supported")


    # Declare each variable from dimacs
    declare_bools(dimacs_bools)
    
    # Assert Conjuctive Normal Form from each line of dimacs
    assert_cnf(lines)

    int_lower_bound = None
    int_upper_bound = None
    int_ids_to_declare = set()
    bool_ids_to_declare = set()
    statements_to_assert = set()
		
    for dimacs_bool in dimacs_bools:
    	maintain_atoms(dimacs_bool)
    
    #print('\n\n\n')
    #print(atom_dict)
    #print('\n\n\n')
    for dimacs_bool in dimacs_bools:
        
	tokens, remainder = scanner.scan(dimacs_bool)
        #print(tokens)
        #print(remainder)

        if not remainder == '':
            raise Exception("Scanner found a syntax error at: {0}" % dimacs_bool)

	if tokens[0][0] == CSP_VAR:
            tempstr = ""
	    pcounter = 0
	    icounter = 0
	    lbound =0 
	    ubound =0
	    pnum =0
	    for (a,b) in tokens:
	        if a == 'PAREN':
	            pcounter += 1
                    pnum =0
		    if pcounter > 1:
		        tempstr = tempstr + b
	        elif a == 'END_PAREN':
		    pcounter -= 1
                    pnum  = 1
		    if pcounter >= 1:
		        tempstr = tempstr + b
	        elif a == 'DIGIT' and pcounter == 1 and icounter ==0:
		    icounter += 1
		    lbound = b
	        elif a == 'DIGIT' and pcounter == 1 and icounter != 0:
		    icounter +=1
		    ubound = b
	        elif a =='IDENTIFIER'  and pcounter >= 1:
		    if pnum ==0:
		        tempstr = tempstr + b
		    else:
		        tempstr = tempstr + ',' + b
                    pnum  = 1
	        elif a == 'DIGIT' and pcounter > 1:
		    if pnum ==0:
		        tempstr = tempstr + b
		    else:
		        tempstr = tempstr + ',' + b
                    pnum  = 1
	    
	    if icounter != 0 and icounter != 2:
		raise Exception("Invalid declaration of cspvar")


    	    if logic == REAL_TYPE:
		if icounter ==0:
                    declare_real(tempstr)
		else:
                    declare_bounded_real(tempstr, lbound, ubound)
	    elif logic == MIXED_TYPE: 
		# Level ranking variables must be declared over integers.
	        if tokens[2][1][:2] == 'lr' and tokens[2][1][2:].isdigit():
               	    declare_bounded_int(tokens[2][1], tokens[3][1], tokens[4][1])
		# if the variable is specified as over integers 	
		elif tokens[2][1] in intList:
		    if icounter ==0:
			declare_int(tempstr)
		    else:
			declare_bounded_int(tempstr, lbound, ubound)
		else:
		    if icounter ==0:
                	declare_real(tempstr)
		    else:
                	declare_bounded_real(tempstr, lbound, ubound)
            else:
		if icounter ==0:
		    declare_int(tempstr)
		else:
		    declare_bounded_int(tempstr, lbound, ubound)
        elif tokens[0][0] == EZSMT_VAR:
            declare_ezsmt_var(tokens)
        elif tokens[0][0] == DOMAIN:
            assert tokens[2][0] == IDENTIFIER
    	    if logic != MIXED_TYPE and logic != REAL_TYPE:
		cspdomain = tokens[2][1] 
                logic = SUPPORTED_LOGICS[tokens[2][1]]

            #print("domain", tokens[2][1])
            # Do something with cspdomain
            #print("domain " + tokens[2][1])
            # Do nothing
            continue
        
    for dimacs_bool in dimacs_bools:
        tokens, remainder = scanner.scan(dimacs_bool)
        #print(dimacs_bool)
	#print(tokens)
        #print(remainder)

        if not remainder == '':
            raise Exception("Scanner found a syntax error at: {0}" % dimacs_bool)
        elif tokens[0][0] == CSP_EXPRESSION and tokens[1][0] == 'INT_COMPARE':
	    #Handle nested parens in variable names
	    newtokens = []
	    plist = [0]
	    tempstr = ""
	    temptpl = (0,0)
	    pnum =0

	    for (a,b) in tokens:
            	if a == 'PAREN':
		    if newtokens[-1][0] == 'IDENTIFIER':
			plist.append(1)
		    else:
			plist.append(0)
		    newtokens.append((a,b))
            	elif a == 'END_PAREN':
		    if plist.pop() == 0:
		        newtokens.append((a,b))
		    else:
			pnum =0
			tempstr = ")"
			while True:
			    temptpl = newtokens.pop()
			    if temptpl[0] ==   'PAREN':
				tempstr = newtokens.pop()[1] + "(" + tempstr
				newtokens.append(('IDENTIFIER', tempstr))
				break
			    else:
				if pnum ==0:
				    tempstr = temptpl[1] + tempstr
				    pnum = 1
				else:
				    tempstr = temptpl[1] + "," + tempstr
		else: 
		    newtokens.append((a,b))
	    if len(plist) != 0:
		raise Exception("Unmatched Parenthesis")

	    csp_expression_to_smt(dimacs_bool, newtokens)
	    
        elif tokens[0][0] == CSP_EXPRESSION:
	    #Handle nested parens in variable names
	    newtokens = []
	    plist = [0]
	    tempstr = ""
	    temptpl = (0,0)
	    pnum =0

	    for (a,b) in tokens:
            	if a == 'PAREN':
		    if newtokens[-1][0] == 'IDENTIFIER' and newtokens[-1][1] != 'list':
			plist.append(1)
		    else:
			plist.append(0)
		    newtokens.append((a,b))
            	elif a == 'END_PAREN':
		    if plist.pop() == 0:
		        newtokens.append((a,b))
		    else:
			pnum =0
			tempstr = ")"
			while True:
			    temptpl = newtokens.pop()
			    if temptpl[0] ==   'PAREN':
				tempstr = newtokens.pop()[1] + "(" + tempstr
				newtokens.append(('IDENTIFIER', tempstr))
				break
			    else:
				if pnum ==0:
				    tempstr = temptpl[1] + tempstr
				    pnum = 1
				else:
				    tempstr = temptpl[1] + "," + tempstr
		else: 
		    newtokens.append((a,b))
	    if len(plist) != 0:
		raise Exception("Unmatched Parenthesis")
            csp_expression_to_smt(dimacs_bool, newtokens)
        elif tokens[0][0] == INT_COMPARE or tokens[0][0] == BOOL_COMPARE:
            # Get all items that must be declared
            # Booleans can be declared as is
            # Integers eventually need to be defined to be within the specified domain
            ids_to_declare_list = list()
            ids_to_declare = None
            
            for token in tokens[::-1]:
                if token[0] == END_PAREN:
                    ids_to_declare_list.append(set())
                elif token[0] == PAREN:
                    # Investigate: Could this raise problems?
                    if not ids_to_declare:
                        #print("A id could not be declared in: %s" % bool)
                        #raise
                        ids_to_declare = ids_to_declare_list.pop()
                    else:
                        ids_to_declare_list.pop()
                elif token[0] == BOOL_COMPARE or token[0] == BOOL_OPERATOR:
                    bool_ids_to_declare.update(ids_to_declare)
                    ids_to_declare = None
                elif token[0] == INT_COMPARE or token[0] == INT_OPERATOR:
                    int_ids_to_declare.update(ids_to_declare)
                    ids_to_declare = None
                elif token[0] == IDENTIFIER:
                    ids_to_declare_list[len(ids_to_declare_list) - 1].add(token[1])
                #elif token[0] == FUNCTION:
                #    print("FUNCTION:" + token[1])

            # Transform statement for appropriate prefix SMT format
            statements_to_assert.add((dimacs_bool, get_statement(tokens)))

    # Don't redeclare already declared variables (precautionary measure)
    bool_ids_to_declare.difference_update(dimacs_bools)
    
    declare_bools(bool_ids_to_declare)

    # Must fix assert statements to assert equality of |statement| and generated statement    
    assert_statements(statements_to_assert)

    #print(func_dict)

    check_sat()
