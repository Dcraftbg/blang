#include <iostream>
#include <stdio.h>
#include <string>
#include <string.h>
#include <assert.h>
#include <vector>
#include <stdarg.h>
#include <direct.h>
//#include <limits.h>
#include<windows.h>
#include "dirnameIm.cpp"
#include "isDigit.cpp"

#include<map>
#define NULLINST -1
enum constvarDefs {
	constInt,
	constStr,
};
//enum InstVarDefs {
//	VDInt,
//};
//enum InstStdFuncs {
//	printc,
//	print,
//};
enum symbols {
	NBracketOpen,
	NBracketClose,
	CBracketOpen,
	CBracketClose
};
enum IntrinsicType {
	INCLUDE,
	IF,
	PRINT,
	PRINTC,

	PLUS,
	MINUS,
	MULTIPLY,
	DIVIDE,
	EQUALS,

	INT_DEF,
	MAX_TYPE
};
std::map<std::string, int> IntrinsicNames;
enum varTypes {
	_INT,
	_CHAR,
};
enum InstructionType {
	Null,
	Word,
	Intrinsic,
	Symbol, // made for () and {}
	constDef,
	//varDef,
	//stdFunc,
};
struct location {
	std::string file;
	int linenumber;
	
	int character;
	int codeline;
	location() {
		this->file = "";
		this->linenumber = 0;
		this->character = 0;
		this->codeline = 0;
	}
	location(std::string file, int linenumber, int character, int codeline) {
		this->file = file;
		this->linenumber = linenumber;
		this->character = character;
		this->codeline = codeline;
	}
};

struct _Token{
	int Inst;
	InstructionType type;
	location loc;
	void* value;
	int operand;
	_Token() {
		this->type = Null; 
		Inst = NULLINST;
		value = nullptr;
		operand = 0;
	}
	_Token(InstructionType type, location loc) {
		this->type = type;
		this->loc = loc;
		this->Inst = NULLINST;
		operand = 0;
	}
	_Token(InstructionType type, location loc, void* value) {
		this->type = type;
		this->loc = loc;
		this->value = value;
		this->Inst = NULLINST;
		operand = 0;
	}
	_Token(InstructionType type, int inst, location loc, void* value) {
		this->type = type;
		this->loc = loc;
		this->value = value;
		this->Inst = inst;
		operand = 0;
	}
	_Token(InstructionType type, int inst, location loc) {
		this->type = type;
		this->loc = loc;
		this->value = nullptr;
		this->Inst = inst;
		operand = 0;
	}
	_Token(InstructionType type, int inst, location loc, int operand){
		this->type = type;
		this->loc = loc;
		this->value = nullptr;
		this->operand = operand;
		this->Inst = inst;
	}

};
struct Sim_eval {
	int operand;
	size_t skip;
	bool iserror;

};
void printCE(_Token token, _In_z_ _Printf_format_string_ char const* const _Format, ...) {
	fprintf(stderr,"\n%s:%d:%d: ERROR: ", token.loc.file.c_str(), token.loc.linenumber, token.loc.character);
	va_list ap;
	va_start(ap, _Format);
	vfprintf(stderr,_Format, ap);
	va_end(ap);
	//exit(1);
}
void printCNote(_Token token, _In_z_ _Printf_format_string_ char const* const _Format, ...) {
	printf("\n[NOTE]: %s:%d:%d: ", token.loc.file.c_str(), token.loc.linenumber, token.loc.character);
	va_list ap;
	va_start(ap, _Format);
	vprintf(_Format, ap);
	va_end(ap);
	//exit(1);
}
void freeTokens(const std::vector<_Token>& tokens) {
	for (size_t i = 0; i < tokens.size(); i++) {
		if (tokens[i].type == Word) {
			delete tokens[i].value;
		}
		else if (tokens[i].type == constDef && tokens[i].Inst == constStr) {
			delete tokens[i].value;
		}
	}
}
size_t fwrite_str(FILE* f, const char* str) {
	return fwrite(str, strlen(str), 1, f);
}
int64_t getTypeSize(varTypes type){
	if (type == _INT) return 4;
	if (type == _CHAR) return 1;
	return -1;
}



// Currently supporting:
/*
* enum InstructionType {
	Null,
	Word,
	Intrinsic,
	constDef,
};
*/
std::string human(const InstructionType type, bool isplural = false) {
	if (isplural) {
		if (type == Null) return "null";
		else if (type == Word) return "words";
		else if (type == Intrinsic) return "intrinsics";
		else if (type == constDef) return "constant definitions";
		else return "undefined";
	}
	else{
		if (type == Null) return "null";
		else if (type == Word) return "word";
		else if (type == Intrinsic) return "intrinsic";
		else if (type == constDef) return "const definition";
		else return "undefined";
	}

}
std::string human(const constvarDefs type, bool isplural = false) {
	if (isplural) {
		if (type == constvarDefs::constStr) return "strings";
		else if (type == constvarDefs::constInt) return "ints";
		else return "undefined";
	}
	else {
		if (type == constvarDefs::constStr) return "string";
		else if (type == constvarDefs::constInt) return "int";
		else return "undefined";
	}
}

// TODO: Implement Precompile
void Precompile(const char* path) {assert(false && "Unimplemented!\n");};

std::vector<_Token>::iterator getLEnd(std::vector<_Token>& tokens, size_t offset=0) {
	size_t i2 = offset;
	int from = tokens[offset].loc.codeline;
	auto i = tokens.begin() + offset;
	for (; i->loc.codeline == from; i++) {
		//printf("I's location: %d and from: %d\n", i->loc.codeline, from);
		
	}
	if ((i - (tokens.begin() + offset)) == 2) return i;
	auto debug = (i - (tokens.begin() + offset));
	i++;
	return i;
}
std::vector<_Token> Tokenize(const char* path) {
	FILE* f = 0;
	fopen_s(&f,path, "rb");
	//assert(f != NULL && "Error: Undefined behaviour occured whilst reading file!\n");
	if (f == NULL) {
		fprintf(stderr, "ERROR: could not open file %s...\n",path);
		exit(1);
	}
	char c = 0;
	std::string word;
	std::vector<_Token> out;
	std::vector<size_t>    nbracket_stack;
	int linenum = 1;
	int character = 0;
	int codeline = 0;
	while (c != EOF) {
		c = fgetc(f);
		STARTMALOOPTOKENIZE:
		if (c == '/'){			
			if (fgetc(f) == '/') {
				while (c != '\n' && c != EOF) {
					c = fgetc(f);
	
				}
			
				if (c == '\n') {

					linenum++;
					character = 0;

					if (word.length() > 0) {


						c = '\n';
						goto STARTMALOOPTOKENIZE;
					}
				}
				else {

					if (word.length() > 0) {

						c = EOF;
					}
				}
			}
			else {
				fseek(f, -1, SEEK_CUR);
				//word.insert('/');
				word+= '/';
				//assert(false, "Unimplemented");
			}	
			c = 0;
		}
		else if (c == ' ') {
		IFCSPACE:
			if (IntrinsicNames.find(word) != IntrinsicNames.end()) {
				out.push_back(_Token(InstructionType::Intrinsic, IntrinsicNames[word], location(path, linenum, character, codeline)));
			}
			else if (mis_number(word)) {
				out.push_back(_Token(constDef, constInt, location(path, linenum, character,codeline), stoi(word)));
			}
			else if (word == "(") {
				nbracket_stack.push_back(out.size());
				out.push_back(_Token(Symbol, NBracketOpen, location(path, linenum, character, codeline)));
			}
			else if (word == ")") {
				size_t oloc = nbracket_stack.back();
				nbracket_stack.pop_back();
				out[oloc].operand = out.size();
				out.push_back(_Token(Symbol, NBracketClose, location(path, linenum, character, codeline),oloc));
			}
			else{
				if (word.length() > 0) {
					//printf("wat da fak 2\n");
					//printf("Pushing word... \"%s\"\n", word.c_str());
					std::string* val = new std::string();
					*val = word;
					out.push_back(_Token(Word, location(path, linenum, character, codeline), val));
				}
			}
			word.clear();

		}
		else if (c == '\t' || c == '\r') {

		}
		else if (c == '\n') {
		NEWLINEHANDLE:
			linenum++;
			character = 0;
		}
		else if (c == '"') {
			char temp = 0;
			std::string strlit;
			//if(temp != '"' && temp != '\n' && temp != EOF) strlit += temp;
			while (temp != '"' && temp != '\n' && temp != EOF) {
				temp = fgetc(f);
				if (temp == '\\') {
					char tnc = fgetc(f);
					if (tnc == 'n') {
						strlit += '\n';
					}
					else if (tnc == 't') {
						strlit += '\t';
					}
					else if (tnc == 'b') {
						strlit += '\b';
					}
					else if (tnc == 'r') {
						strlit += '\r';
					}
					else {
						strlit += tnc;
					}
				}
				else if(temp != '"') {
					strlit += temp;
				}
			}
			if (temp == EOF || temp == '\n') {
				fprintf(stderr, "%s:%d:%d: ERROR: string literal opened but never closed!\n", path,linenum,character);
				freeTokens(out);
				exit(1);
			}
			auto p = new std::string();
			*p = strlit;
			out.push_back(_Token(constDef, constStr, location(path, linenum, character, codeline), p));
			linenum++;
			character = 0;
			if (word.length() > 0) {
				goto IFCSPACE;
			}
		}
		else if (c == '\'') {
			char temp = fgetc(f);
			char lastchar = 0;
			if (temp == '\\') {
				character += 3;
				char ntemp = fgetc(f);
				if (ntemp == 'n') {
					if (fgetc(f) != '\'') {
						fprintf(stderr, "%s:%d:%d: Error: Undefined character sequence\n", path, linenum, character);
						exit(1);
					}
					out.push_back(_Token(constDef,constInt, location(path, linenum, character,codeline), '\n'));
				}
				else if (ntemp == 't') {
					if (fgetc(f) != '\'') {
						fprintf(stderr, "%s:%d:%d: Error: Undefined character sequence\n", path, linenum, character);
						exit(1);
					}
					out.push_back(_Token(constDef, constInt, location(path, linenum, character, codeline), '\t'));
				}
				else if (ntemp == 'r') {
					if (fgetc(f) != '\'') {
						fprintf(stderr, "%s:%d:%d: Error: Undefined character sequence\n", path, linenum, character);
						exit(1);
					}
					out.push_back(_Token(constDef, constInt, location(path, linenum, character, codeline), '\r'));
				}
				else if (ntemp == '\\') {
					if (fgetc(f) != '\'') {
						fprintf(stderr, "%s:%d:%d: Error: Undefined character sequence\n", path, linenum, character);
						exit(1);
					}
					out.push_back(_Token(constDef, constInt, location(path, linenum, character, codeline), '\\'));
				}
				else if (ntemp == '\'') {
					if (fgetc(f) != '\'') {
						fprintf(stderr, "%s:%d:%d: Error: Undefined character sequence\n", path, linenum, character);
						exit(1);
					}
					out.push_back(_Token(constDef, constInt, location(path, linenum, character,codeline), '\''));
				}
				else {
					fprintf(stderr, "%s:%d:%d: Error: Undefined character sequence\n", path, linenum, character);
					exit(1);
				}
			}
			else {
				if (fgetc(f) != '\'') {
					fprintf(stderr, "%s:%d:%d: Error: Undefined character sequence\n", path, linenum, character);
					exit(1);
				}
				else {
					character += 2;
					out.push_back(_Token(constDef, constInt, location(path, linenum, character,codeline), temp));
				}
			}


			if (word.length() > 0) {
				goto IFCSPACE;
			}
		}
		else if (c == ';') {
			codeline++;
			if (word.length() > 0) {
				goto IFCSPACE;
			}
		}
		else{
			word.push_back(c);
		}
		character++;

	}
	ENDTOKENIZE:
	return out;
};
template<typename MapFrom, typename MapTo> 
MapFrom findFirstByValueInMap(std::map<MapFrom, MapTo> m, MapTo val) {
	for (auto iter = m.begin(); iter != m.end(); iter++) {
		if (val == iter->second) {
			return iter->first;
		}
	}
	/*for (std::map<MapFrom, MapTo>:: iter = List.begin(); iter != List.end(); iter++) {
		cout << (*iter).first << " is " << (*iter).second << endl;
	}*/
}

struct SimVar {
	//std::string name;
	varTypes type;
	void* value;
	int operand;
	SimVar(void* value, varTypes type=_INT) {
		//this->name = name;
		this->value = value;
		this->operand = -1;
		this->type = type;
	}
	SimVar(int operand, varTypes type=_INT) {
		//this->name = name;
		this->operand = operand;
		this->value = nullptr;
		this->type = type;
	}
	SimVar(varTypes type=_INT) {
		//this->name = name;
		this->operand = -1;
		this->value = nullptr;
		this->type = type;
	}
	//SimVar() {
	//	//this->name = "";
	//	this->operand = -1;
	//	this->value = nullptr;
	//}
};
struct ComVar {
	varTypes             type;
	uint64_t     stack_offset;
};
//std::string          name;
struct Instruction {
	_Token token; // original token

};
struct Program {
	std::vector<_Token> tokens;

};

//Program _parse_program(std::vector<_Token> tokens) {
//	
//}
Sim_eval sim_evaluate(std::vector<_Token>& tokens, std::map<std::string, SimVar> varStack) {
	//std::vector<int> stack;
	std::vector<int> stack;
	size_t i = 0;
	for (; i < tokens.size(); i++) {
		if (tokens[i].type == constDef) {
			stack.push_back(tokens[i].operand);
		}
		else if (tokens[i].type == Word) {
			auto loc = varStack.find(*((std::string*)tokens[i].value));
			if (loc == varStack.end()) {
				printCE(tokens[i], "Error: unidentified symbol \"%s\"", (*((std::string*)tokens[i].value)).c_str());
				return {- 1,i, true};
				//exit(1);
			}
			else {
				stack.push_back(loc->second.operand);
				//tokens[i].operand = loc->second.operand;
			}
		}
		else if (tokens[i].type == Intrinsic) {
			// TODO:
			// fix cases like 5 - 7 + 3 (it turns to 5 - 10)
			if (tokens[i].Inst == IntrinsicType::PLUS) {
				//std::vector<_Token>::iterator snapshotEnd   = tokens.end();
				std::vector<_Token>::iterator snapshotEnd   = tokens.end();
				std::vector<_Token>::iterator snapshotStart = tokens.begin() + i + 1;
				std::vector<_Token> snapshot(snapshotStart, snapshotEnd);
				//std::pair<int,bool> t = sim_evaluate(snapshot, varStack);
				Sim_eval t = sim_evaluate(snapshot, varStack);
				if (t.iserror) return { -1, t.skip, true }; //return std::pair<int, bool>(-1, true);
				//stack.push_back(t.operand);
				int v = stack.back();
				stack.pop_back();
				stack.push_back(t.operand + v);
				i += t.skip;
			}
			// TODO:
			// fix cases like 5 - 7 + 3 (it turns to 5 - 10)
			else if (tokens[i].Inst == IntrinsicType::MINUS) {
				//std::vector<_Token>::iterator snapshotEnd   = tokens.end();
				std::vector<_Token>::iterator snapshotEnd = tokens.end();
				std::vector<_Token>::iterator snapshotStart = tokens.begin() + i + 1;
				std::vector<_Token> snapshot(snapshotStart, snapshotEnd);
				//std::pair<int,bool> t = sim_evaluate(snapshot, varStack);
				Sim_eval t = sim_evaluate(snapshot, varStack);
				if (t.iserror) return { -1, t.skip, true }; //return std::pair<int, bool>(-1, true);
				//stack.push_back(t.operand);
				int v = stack.back();
				stack.pop_back();
				stack.push_back(t.operand - v);
				i += t.skip;
			}
			else if (tokens[i].Inst == IntrinsicType::MULTIPLY) {
				if (tokens[i+1].type == InstructionType::Symbol) {
					// TODO: throw an error message to indicate that the symbol was incorrect
					if (tokens[i + 1].Inst != NBracketOpen) return { -1, i,true };
					auto snapshotStart = tokens.begin() + i + 1;
					auto snapshotEnd   = tokens.begin() + tokens[i+1].operand;
					std::vector<_Token> snapshot(snapshotStart, snapshotEnd);
					Sim_eval t = sim_evaluate(snapshot, varStack);
					if (t.iserror) return { -1, t.skip, true }; 
					int v = stack.back();
					stack.pop_back();
					stack.push_back(v * t.operand);
					i += t.skip;
				}
				else if (tokens[i + 1].type == InstructionType::constDef && tokens[i+1].Inst == constInt) {
					int v = stack.back();
					stack.pop_back();
					stack.push_back(v * tokens[i + 1].operand);
					i++; 
				}
				else if (tokens[i + 1].type == InstructionType::Word) {
					auto vv = varStack.find(*((std::string*)tokens[i + 1].value));
					if (vv == varStack.end()) {
						printCE(tokens[i + 1], "Error: symbol '%s' is not defined\n", (* ((std::string*)tokens[i + 1].value)).c_str());
						return { -1, i, true };
					}
					int v = stack.back();
					stack.pop_back();
					// TODO: check for the vv type etc.
					stack.push_back(v * vv->second.operand);
					i++;
				}
				else {
					printCE(tokens[i+1], "Error: invalid intrinsic taken when evaluating value in mulitply!");
					return { -1, i, true };
				}
			}
			else if (tokens[i].Inst == IntrinsicType::DIVIDE) {
				if (tokens[i + 1].type == InstructionType::Symbol) {
					// TODO: throw an error message to indicate that the symbol was incorrect
					if (tokens[i + 1].Inst != NBracketOpen) return { -1, i,true };
					auto snapshotStart = tokens.begin() + i + 1;
					auto snapshotEnd = tokens.begin() + tokens[i + 1].operand;
					std::vector<_Token> snapshot(snapshotStart, snapshotEnd);
					Sim_eval t = sim_evaluate(snapshot, varStack);
					if (t.iserror) return { -1, t.skip, true };
					int v = stack.back();
					stack.pop_back();
					stack.push_back(v / t.operand);
					i += t.skip;
				}
				else if (tokens[i + 1].type == InstructionType::constDef && tokens[i + 1].Inst == constInt) {
					int v = stack.back();
					stack.pop_back();
					stack.push_back(v / tokens[i + 1].operand);
					i++;
				}
				else if (tokens[i + 1].type == InstructionType::Word) {
					auto vv = varStack.find(*((std::string*)tokens[i + 1].value));
					if (vv == varStack.end()) {
						printCE(tokens[i + 1], "Error: symbol '%s' is not defined\n", (* ((std::string*)tokens[i + 1].value)).c_str());
						return { -1, i, true };
					}
					int v = stack.back();
					stack.pop_back();
					// TODO: check for the vv type etc.
					stack.push_back(v / vv->second.operand);
					i++;
				}
				else {
					printCE(tokens[i], "Error: invalid intrinsic taken when evaluating value in mulitply!");
					return { -1, i, true };
				}
			}
			else {
				printCE(tokens[i], "Error: invalid intrinsic taken when evaluating value, (Found: \"%s\")", findFirstByValueInMap<std::string, int>(IntrinsicNames,tokens[i].Inst).c_str());
				//return std::pair<int, bool>(-1, true);
				return { -1, i, true };
			}
		}
		else if (tokens[i].type == Symbol) {
			//Literally do nothing for right now since () don't affect any part of the code
		}
		else {
			printCE(tokens[i], "Error: unidentified token when evaluating value, (Found type: \"%s\")", human(tokens[i].type).c_str());
			//exit(1);
			return { -1, i, true };
		}
	}
	if (stack.size() > 1) {
		printCE(tokens[tokens.size() - 1], "Error: Too much information was put onto the output stack!");
		//exit(1);
		return { -1, i, true };
	}
	else if (stack.size() == 0) {
		printCE(tokens[tokens.size() - 1], "Error: Too little information was put onto the output stack!");
		//exit(1);
		return { -1, i, true };
	}
	return { stack[0], i,false };
}
void simulate(std::vector<_Token>& tokens) {
	
	//std::vector<SimVar> varStack;
	std::map<std::string, SimVar> varStack;
	for (size_t i = 0; i < tokens.size(); i++) {
		switch (tokens[i].type) {
		case Intrinsic:
			switch (tokens[i].Inst) {
			case IntrinsicType::PRINTC: {
				if (tokens[i+1].type == constDef && tokens[i + 1].Inst == constInt) {
					printf("%c", tokens[i + 1].operand);
				}
				else if(tokens[i + 1].type == Word) {
					//int val = 0;
					//bool hasFoundVal = false;
					//varStack[*((std::string*)tokens[i + 1].value)]
					auto loc = varStack.find(*((std::string*)tokens[i + 1].value));
					if (loc == varStack.end()) {
						fprintf(stderr, "\n%s:%d:%d: ERROR: Undefined word \"%s\"\n", tokens[i].loc.file.c_str(), tokens[i].loc.linenumber, tokens[i].loc.character, (char*)tokens[i].value);
						freeTokens(tokens);
						exit(1);
					}
					else {
						//printf("%c", loc->second.operand);
						putchar(loc->second.operand);
					}
					/*for (size_t z = 0; z < varStack.size(); z++) {
						if (varStack[z].name == *((std::string*)tokens[i + 1].value)) {
							val = varStack[z].operand;
							hasFoundVal = true;
						}
					}*/
					
					/*if (!hasFoundVal) {
						fprintf(stderr, "\n%s:%d:%d: ERROR: Undefined word \"%s\"\n", tokens[i].loc.file.c_str(), tokens[i].loc.linenumber, tokens[i].loc.character,(char*)tokens[i].value);
						freeTokens(tokens);
						exit(1);
					}*/
					//printf("%c", val);
				}
				else {
					freeTokens(tokens);
					printCE(tokens[i], "Error: Unsupported type passed to printc intrinsic\nExpected: \"%s\" but got \"%s\"\n", human(Word).c_str(), human(tokens[i + 1].type).c_str());
					exit(1);
				}
				i++;
			}
					   break;
			case IntrinsicType::PRINT: {
				if (tokens[i+1].type == constDef && tokens[i + 1].Inst == constInt) {
					printf("%d", tokens[i + 1].operand);
				}
				else if (tokens[i + 1].type== Word) {
					//int val = 0;
					//bool hasFoundVal = false;
					//varStack[*((std::string*)tokens[i + 1].value)]
					auto loc = varStack.find(*((std::string*)tokens[i + 1].value));
					if (loc == varStack.end()) {
						fprintf(stderr, "\n%s:%d:%d: ERROR: Undefined word \"%s\"\n", tokens[i].loc.file.c_str(), tokens[i].loc.linenumber, tokens[i].loc.character, (char*)tokens[i].value);
						freeTokens(tokens);
						exit(1);
					}
					else {
						printf("%d", loc->second.operand);
						//putchar(loc->second.operand);
					}
					/*for (size_t z = 0; z < varStack.size(); z++) {
						if (varStack[z].name == *((std::string*)tokens[i + 1].value)) {
							val = varStack[z].operand;
							hasFoundVal = true;
						}
					}*/

					/*if (!hasFoundVal) {
						fprintf(stderr, "\n%s:%d:%d: ERROR: Undefined word \"%s\"\n", tokens[i].loc.file.c_str(), tokens[i].loc.linenumber, tokens[i].loc.character,(char*)tokens[i].value);
						freeTokens(tokens);
						exit(1);
					}*/
					//printf("%c", val);
				}
				else {
					freeTokens(tokens);
					printCE(tokens[i], "Error: Unsupported type passed to print intrinsic\nExpected: \"%s\" but got \"%s\"\n",human(Word).c_str(), human(tokens[i+1].type).c_str());
					exit(1);
				}
				/*if (tokens[i + 1].Inst == constInt) {
					printf("%d", tokens[i + 1].operand);
				}*/
				i++;
			}
					  break;
			case IntrinsicType::INT_DEF: {
				if (tokens[i + 1].type == Word) {
					if (varStack.find(*((std::string*)tokens[i + 1].value)) != varStack.end()) {
						printCE(tokens[i], "Error: Symbol \"%s\" already defined!", (* ((std::string*)tokens[i + 1].value)).c_str());
						freeTokens(tokens);
						exit(1);
					}
					else {
						varStack[*((std::string*)tokens[i + 1].value)] = SimVar(_INT);
					}
					//varStack.push_back(SimVar(*((std::string*)tokens[i + 1].value)));
				}
				else {
					freeTokens(tokens);
					printCE(tokens[i], "Invalid variable definition. Expected type \"WORD\" but got type: \"%s\"", human(tokens[i + 1].type).c_str());
					exit(1);
				}
				i++;
			}		  break;
			case IntrinsicType::EQUALS: {
				if (tokens[i - 1 ].type == Word) {
					if (varStack.find(*((std::string*)tokens[i - 1].value)) != varStack.end()) {
						SimVar v = varStack[*((std::string*)tokens[i - 1].value)];
							if (v.type == _INT) {
								auto snapshotStart = tokens.begin() + i + 1;
								auto snapshotEnd   = getLEnd(tokens, i);
								std::vector<_Token> snapshot(snapshotStart, snapshotEnd);
								Sim_eval result = sim_evaluate(snapshot, varStack);
								if (result.iserror) { freeTokens(tokens); exit(1); }
								varStack[*((std::string*)tokens[i - 1].value)].operand = result.operand;
								i += result.skip;
								/*
								if (tokens[i + 1].Inst == constInt) {
									varStack[*((std::string*)tokens[i - 1].value)].operand = tokens[i + 1].operand;
								}
								else if (tokens[i + 1].type == Word) {
									auto loc = varStack.find(*(std::string*)tokens[i + 1].value);
									if (loc == varStack.end()) {
										//TODO: throw some sort of message here.
										exit(1);
									}
									varStack[*((std::string*)tokens[i - 1].value)].operand = loc->second.operand;
								}
								else {
									freeTokens(tokens);
										printCE(tokens[i + 1],
											"Error: Unidentified type for the '=' intrinsic\nExpected: \"%s\" or \"%s\" but found \"%s\"",
											human(Word).c_str(),
											human(constDef).c_str(),
											human(tokens[i + 1].type).c_str());
									exit(1);
								}
								*/
							}
					}
					else {
						// TODO: generate some sort of error
						freeTokens(tokens);
						exit(1);
					}
				}
			}         break;
			case IntrinsicType::INCLUDE: {
				//if (tokens[i + 1].type != constDef) {
				//	printCE(tokens[i + 1], "Error: Wrong include intrinsic operand type, Expected \"%s\" but found \"%s\"", human(constDef).c_str(), human(tokens[i + 1].type).c_str());
				//	freeTokens(tokens);
				//	exit(1);
				//}
				//if (tokens[i + 1].Inst != constStr) {
				//	printCE(tokens[i + 1], "Error: Wrong include intrinsic operand variable type, Expected: \"%s\" but found \"%s\"", human(constStr).c_str(), human((constvarDefs)tokens[i + 1].Inst).c_str());
				//	freeTokens(tokens);
				//	exit(1);
				//}
				//std::vector<_Token> includeTokens = Tokenize(((std::string*)tokens[i + 1].value)->c_str());
				////tokens.resize(tokens.size() + includeTokens.size());
				//
				//tokens.insert(tokens.end(), includeTokens.begin(), includeTokens.end());
				i++;
			}         break;
			default:
				//fprintf(stderr, "%s:%d:%d: ERROR: Unhandled intrinsic: %s", Intrinsic)
				//assert(IntrinsicNames.size() == 3 && "Unhandled intrinsics in simulate");
				//freeTokens(tokens);
				//assert(false && "Unimplemented\n");
				fprintf(stderr, "%s:%d:%d: ERROR: Unhandled intrinsic \"%s\"\n", tokens[i].loc.file.c_str(), tokens[i].loc.linenumber, tokens[i].loc.character,
					findFirstByValueInMap<std::string, int>(IntrinsicNames, tokens[i].Inst).c_str()
				);
				freeTokens(tokens);
				exit(1);
				assert(false && "Unimplemented\n");
			}
			break;
		default:
			if (tokens[i].type == Word) {
				auto vv = varStack.find((*((std::string*)tokens[i].value)));
				if (vv == varStack.end()) {
					printCE(tokens[i], "Error: Undefined word: \"%s\"", (*((std::string*)tokens[i].value)).c_str());
					freeTokens(tokens);
					exit(1);
				}

				if (tokens[i + 1].type == InstructionType::Intrinsic) {
					if (tokens[i + 1].Inst == IntrinsicType::EQUALS) {}
					else {
						printCE(tokens[i], "Error: undefined symbol usage \"%s\"", vv->first.c_str());
						freeTokens(tokens);
						exit(1);
					}
				}
				// TODO: this should only work if there is I
			}
			else {
				printCE(tokens[i], "Error: Undefined token of type \"%s\", with instance: %d", human(tokens[i].type).c_str(), tokens[i].Inst);
				freeTokens(tokens);
				exit(1);
			}
			//printCE(tokens[i], "Token: %d, Instance: %d", tokens[i].type, tokens[i].Inst);
		}
	}
	freeTokens(tokens);
}
enum ComEIdentifier : uint8_t
{
	RBX,
	RCX,
	RDX
};
ComEIdentifier nextComE(ComEIdentifier& e) {
	ComEIdentifier prev = e;
	if (e == RDX) {
		e = RBX;
		return prev;
	}
	e = (ComEIdentifier)((uint8_t)e + 1);
	return prev;
}
ComEIdentifier backComE(ComEIdentifier& e) {
	ComEIdentifier prev = e;
	if (e == RBX) {
		e = RDX;
		return prev;
	}
	e = (ComEIdentifier)((uint8_t)e - 1);
	return e;
}
const char* toStrComE(ComEIdentifier e, size_t size=8) {
	if (size == 8) {
		if (e == RBX)      return "rbx";
		else if (e == RCX) return "rcx";
		else if (e == RDX) return "rdx";
	}
	else if(size == 4) {
		if (e == RBX)      return "ebx";
		else if (e == RCX) return "ecx";
		else if (e == RDX) return "edx";
	}
	else if (size == 2) {
		if (e == RBX)      return "bx";
		else if (e == RCX) return "cx";
		else if (e == RDX) return "dx";
	}
	else if (size == 1) {
		if (e == RBX)      return "bl";
		else if (e == RCX) return "cl";
		else if (e == RDX) return "dl";
	}
	return "UNHANDLED";
}
void writeComE(ComEIdentifier& e, int val, FILE* f) {
	fprintf(f, "   mov %s, %d\n",toStrComE(e),val);
}
struct ComRet {
	size_t skip;
	bool iserror;
	ComVar var = ComVar();
	int64_t  operand = 0;
	ComEIdentifier id;
	bool isconstant; // basically meaning that we have the result on the stack
	// For variables with higher than 8 sizes like structs, we might need an identifier to specifically say that they are in fact 
	ComRet(size_t skip, bool iserror, ComEIdentifier id, ComVar var, size_t operand, bool isconstant=false)     : skip(skip), iserror(iserror), var(var), id(id), isconstant(isconstant), operand(operand) {}
	ComRet(size_t skip, bool iserror, ComEIdentifier id, size_t operand, bool isconstant=true)                : skip(skip), iserror(iserror), operand(operand),id(id),isconstant(isconstant) {}
	ComRet(size_t skip, bool iserror, ComEIdentifier id)                                                       : skip(skip), iserror(iserror), id(id), isconstant(false) {}
};
struct EvalStackIden{
	bool isconstant;
	//size_t offset;
	ComVar  var;
	int64_t operand;
	EvalStackIden(bool isconstan, int operan=0,ComVar va=ComVar()) : isconstant(isconstan), operand(operan), var(va){}
};
const char* getVarTypeName(ComVar& var) {
	switch (getTypeSize(var.type)) {
	case 1:
		return "byte";
	case 2:
		return "word";
	case 4:
		return "dword";
	case 8:
		return "qword";
	default:
		return "byte";
	}
}
ComRet com_evaluate(std::vector<_Token>& tokens, std::map<std::string, ComVar>& varStack, size_t stack_size, FILE* f, ComEIdentifier id=RBX) {
	std::vector <EvalStackIden> stack;
	size_t i = 0;
	for (; i < tokens.size(); i++) {
		if (tokens[i].type == constDef) {
			//stack.push_back({1,,tokens[i].operand});
			stack.push_back(EvalStackIden(1, tokens[i].operand));
		}
		else if (tokens[i].type == Word) {
			auto loc = varStack.find(*((std::string*)tokens[i].value));
			if (loc == varStack.end()) {
				printCE(tokens[i], "Error: unidentified symbol \"%s\"", (*((std::string*)tokens[i].value)).c_str());
				return ComRet(i, true, id);
			}
			// A = 4;
			// A + 5;
			// ^ mov RBX, A
			// add RBX, 5
			if (id == RDX) {
				// this should never happen but I'll do it anyway
				fprintf(stderr, "Error: ID handling failed, now allocating on the stack\n");
				/*fprintf(f, "   sub rsp, %llu\n", getTypeSize(loc->second.type));
				fprintf(f, "   mov %s[rsp+%llu], %s[rsp+%llu]\n", getVarTypeName(loc->second), getTypeSize(loc->second.type), getVarTypeName(loc->second), stack_size - loc->second.stack_offset);*/
				//stack.push_back(EvalStackIden(1, -1, loc->second));
				
				stack.push_back(EvalStackIden(false,0,loc->second));
			}
			else {

				//fprintf(f, "   mov %s, %s[rsp+%llu]\n", toStrComE(id,getTypeSize(loc->second.type)), getVarTypeName(loc->second), stack_size - loc->second.stack_offset);
				//nextComE(id);
				// 0 not in register
				// 1 in register
				stack.push_back(EvalStackIden(false, 0, loc->second));
			}
		}
		else if (tokens[i].type == Intrinsic) {
			if (tokens[i].Inst == IntrinsicType::PLUS) {
				auto snapshotStart = tokens.begin() + i + 1;
				auto snapshotEnd = tokens.end();
				//tokens.begin() + tokens[i + 1].operand; // Tf was I thinking
				std::vector<_Token> snapshot(snapshotStart, snapshotEnd);
				ComRet r = com_evaluate(snapshot, varStack, stack_size, f, id);
				id = r.id;
				if (r.iserror) return ComRet(i, true, id);
				// A + (B + (C+D)
				EvalStackIden v1 = stack.back();
				stack.pop_back();
				if (v1.isconstant) {
					if (r.isconstant) {
						stack.push_back(EvalStackIden(true, v1.operand + r.operand));
					}
					else {
						// TODO: we lose some information when we push variables onto the stack (specifically their size)
						if (r.operand) fprintf(f, "   add %s, %lld\n", toStrComE(id), v1.operand);
						else {
							fprintf(f, "   mov %s, %s[rsp+%llu]\n", toStrComE(id, getTypeSize(r.var.type)), getVarTypeName(r.var), stack_size - r.var.stack_offset);
							fprintf(f, "   add %s, %lld\n", toStrComE(id), v1.operand);
						}
						backComE(id);
						stack.push_back(EvalStackIden(false, 1));
						
					}
				}
				else {
					if (r.isconstant) {
						if(v1.operand) fprintf(f, "   add %s, %lld\n", toStrComE(id), r.operand);
						else {
							fprintf(f, "   mov %s, %s[rsp+%llu]\n", toStrComE(id, getTypeSize(v1.var.type)), getVarTypeName(v1.var), stack_size - v1.var.stack_offset);
							fprintf(f, "   add %s, %lld\n", toStrComE(id), r.operand);
						}
						stack.push_back(EvalStackIden(false));
					}
					else {
						ComEIdentifier v1Id = ComEIdentifier::RBX;
						ComEIdentifier rId = ComEIdentifier::RBX;
						if (v1.operand) v1Id = backComE(id);
						else {
							fprintf(f, "   mov %s, %s[rsp+%llu]\n", toStrComE(id, getTypeSize(v1.var.type)), getVarTypeName(v1.var),stack_size-v1.var.stack_offset); 
							v1Id = id;
							nextComE(id);
						}
						if (r.operand) rId = backComE(id);
						else {
							fprintf(f, "   mov %s, %s[rsp+%llu]\n", toStrComE(id, getTypeSize(r.var.type)), getVarTypeName(r.var), stack_size - r.var.stack_offset);
							rId = id;
							nextComE(id);
						}
						/*ComEIdentifier rId = backComE(id);
						ComEIdentifier v1Id = backComE(id);*/
						fprintf(f, "   add %s, %s\n", toStrComE(v1Id), toStrComE(rId));
						stack.push_back(EvalStackIden(false, 1));
					}

				}
				i += r.skip;
			}
			// TODO: implement minus!
			else if (tokens[i].Inst == IntrinsicType::MINUS) {
				auto snapshotStart = tokens.begin() + i + 1;
				auto snapshotEnd = tokens.end();
				//tokens.begin() + tokens[i + 1].operand; // Tf was I thinking
				std::vector<_Token> snapshot(snapshotStart, snapshotEnd);
				ComRet r = com_evaluate(snapshot, varStack, stack_size, f, id);
				id = r.id;
				if (r.iserror) return ComRet(i, true, id);
				// A + (B + (C+D)
				EvalStackIden v1 = stack.back();
				stack.pop_back();
				if (v1.isconstant) {
					if (r.isconstant) {
						stack.push_back(EvalStackIden(true, v1.operand - r.operand));
					}
					else {
						// TODO: we lose some information when we push variables onto the stack (specifically their size)
						fprintf(f, "   sub %s, %lld\n", toStrComE(id), v1.operand);
						backComE(id);
						stack.push_back(EvalStackIden(false));
					}
				}
				else {
					if (r.isconstant) {
						fprintf(f, "   sub %s, %lld\n", toStrComE(id), r.operand);
						stack.push_back(EvalStackIden(false));
					}
					else {
						ComEIdentifier rId = backComE(id);
						ComEIdentifier v1Id = backComE(id);
						fprintf(f, "   sub %s, %s\n", toStrComE(v1Id), toStrComE(rId));
						stack.push_back(EvalStackIden(false));
					}

				}
				i += r.skip;
			}
			else {
				printCE(tokens[i], "Error: invalid intrinsic taken when evaluating value, (Found: \"%s\")", findFirstByValueInMap<std::string, int>(IntrinsicNames, tokens[i].Inst).c_str());
				return ComRet(i, true,id);
			}
			
		}
		else {
			printCE(tokens[i], "Error: unidentified token when evaluating value, (Found type: \"%s\")", human(tokens[i].type).c_str());
			//exit(1);
			return ComRet(i, true, id);
		}
	}
	// TODO: handle returns
	if (stack.size() > 1 || stack.size() == 0) {
		fprintf(stderr, "Error: too few or too little data on the stack...\n");
		return ComRet(i, true, id); 
	}
	if (stack.back().isconstant) return ComRet(i, false, id, stack.back().operand);
	else { 
		if(stack.back().operand) return ComRet(i, false, id, stack.back().operand,false); 
		else { return ComRet(i, false, id, stack.back().var, stack.back().operand); }
	}
	// This should be unreachable!
	return ComRet(i, false, id);
}
// TODO: possibly remove
//ComRet com_evaluate2(std::vector<_Token>& tokens, std::map<std::string, ComVar>& varStack, size_t stack_size, FILE* f) {
//	ComEIdentifier id = RBX;
//
//	size_t i = 0;
//	for (; i < tokens.size(); i++) {
//		if (tokens[i].type == constDef) {
//			writeComE(id, tokens[i].operand, f);
//			nextComE(id);
//		}
//		else if (tokens[i].type == Word) {
//			auto loc = varStack.find(*((std::string*)tokens[i].value));
//			if (loc == varStack.end()) {
//				printCE(tokens[i], "Error: unidentified symbol \"%s\"", (*((std::string*)tokens[i].value)).c_str());
//				return { i, true };
//			}
//			if (getTypeSize(loc->second.type) <= 8) {
//				fprintf(f, "   mov %s, dword[rsp+%d]\n", toStrComE(id), stack_size-loc->second.stack_offset);
//				nextComE(id);
//			}
//			else {
//				fprintf(stderr, "Error handling for variables with higher sizes than 8 is not yet implemented");
//				return { i,true };
//			}
//		}
//	}
//	return { i, false };
//}
size_t includeFiles(std::vector<_Token>& tokens) {
	size_t i = 0;
	for (; i < tokens.size(); i++) {
		if (tokens[i].type == Intrinsic && tokens[i].Inst == INCLUDE) {
			if (tokens[i + 1].type != constDef) {
				printCE(tokens[i + 1], "Error: Wrong include intrinsic operand type, Expected \"%s\" but found \"%s\"", human(constDef).c_str(), human(tokens[i + 1].type).c_str());
				freeTokens(tokens);
				exit(1);
			}
			if (tokens[i + 1].Inst != constStr) {
				printCE(tokens[i + 1], "Error: Wrong include intrinsic operand variable type, Expected: \"%s\" but found \"%s\"", human(constStr).c_str(), human((constvarDefs)tokens[i + 1].Inst).c_str());
				freeTokens(tokens);
				exit(1);
			}
			std::vector<_Token> includeTokens = Tokenize(((std::string*)tokens[i + 1].value)->c_str());
			char oldWD[FILENAME_MAX] = { 0 };
			if (_getcwd(oldWD, sizeof(oldWD)) == nullptr) {
				printCE(tokens[i + 1], "Error: could not access currect directory");
				freeTokens(tokens);
				freeTokens(includeTokens);
				exit(1);
			}
			if (_chdir(dirname((char*)((std::string*)tokens[i + 1].value)->c_str()))) {
				printCNote(tokens[i + 1], "Could not access directory %s", dirname((char*)((std::string*)tokens[i + 1].value)->c_str()));
				//fprintf(stderr, "Error: could not access chdir :(...\n");
				//exit(1);
			}
			//tokens.resize(tokens.size() + includeTokens.size());
			if (_chdir(oldWD)) {
				printCE(tokens[i + 1], "Error: could not roll back directory");
				freeTokens(tokens);
				freeTokens(includeTokens);
				exit(1);
			}
			tokens.insert(tokens.begin()+i+2, includeTokens.begin(), includeTokens.end());
			i += includeFiles(includeTokens) + 1;
		}
	}
	return i;
}
void compile(std::vector<_Token>& tokens, std::string path) {
	FILE* out = nullptr;
	fopen_s(&out, path.c_str(), "w");
	if (out == nullptr) {
		fprintf(stderr, "Error: could not open path \"%s\"", path.c_str());
		freeTokens(tokens);
		exit(1);
	}
	std::map<std::string,ComVar> varStack;
	uint64_t stack_size = 0;
	fwrite_str(out,"BITS 64\n");
	fwrite_str(out, "global _main\n");
	// Temporarily link to LibC
	fwrite_str(out, "   extern _printf\n");
	fwrite_str(out, "   extern _putchar\n");
	fwrite_str(out, "section .text\n");
	fwrite_str(out, "_main:\n");
	for(size_t i = 0; i < tokens.size(); i++){
		switch (tokens[i].type) {
		case Intrinsic: {
			switch (tokens[i].Inst) {
			case IntrinsicType::EQUALS: {
				if (tokens[i - 1].type == Word) {
					if (varStack.find(*((std::string*)tokens[i - 1].value)) != varStack.end()) {
						ComVar v = varStack[*((std::string*)tokens[i - 1].value)];
						if (v.type == _INT) {
							auto snapshotStart = tokens.begin() + i + 1;
							auto snapshotEnd = getLEnd(tokens, i);
							std::vector<_Token> snapshot(snapshotStart, snapshotEnd);
							ComRet result = com_evaluate(snapshot, varStack, stack_size, out);
							if (result.iserror) { freeTokens(tokens); exit(1); }
							if (result.isconstant) fprintf(out, "   mov %s[rsp+%llu], %I64u\n", getVarTypeName(v), (stack_size - v.stack_offset), result.operand);
							else {
								if(result.operand) fprintf(out, "   mov %s[rsp+%llu], %s\n", getVarTypeName(v), stack_size - v.stack_offset, toStrComE(result.id, getTypeSize(v.type)));
								else {
									// hold the phone.. wtf.. why does this get called?!?
									// TODO: fix bug with type size missmatching
									// for example:
									// char A = 'A';
									//int B = A; // This would cause an error
									fprintf(out, "   mov %s, %s[rsp+%llu]\n",
										toStrComE(RBX, getTypeSize(result.var.type)),
										getVarTypeName(result.var),
										stack_size - result.var.stack_offset
										);
									fprintf(out, "   mov %s[rsp+%llu], %s\n",
										getVarTypeName(v),
										stack_size - v.stack_offset,
										toStrComE(RBX, getTypeSize(result.var.type))
									);
									//fprintf(out, "   mov %s[rsp+%llu], %s[rsp+%llu]\n", getVarTypeName(v), stack_size - v.stack_offset, getVarTypeName(v), stack_size-result.var.stack_offset);
								}
							}
							i += result.skip;
							//Sim_eval result = sim_evaluate(snapshot, varStack);
							/*if (result.iserror) exit(1);
							varStack[*((std::string*)tokens[i - 1].value)].operand = result.operand;
							i += result.skip;*/
							/*
							if (tokens[i + 1].Inst == constInt) {
								varStack[*((std::string*)tokens[i - 1].value)].operand = tokens[i + 1].operand;
							}
							else if (tokens[i + 1].type == Word) {
								auto loc = varStack.find(*(std::string*)tokens[i + 1].value);
								if (loc == varStack.end()) {
									//TODO: throw some sort of message here.
									exit(1);
								}
								varStack[*((std::string*)tokens[i - 1].value)].operand = loc->second.operand;
							}
							else {
								freeTokens(tokens);
									printCE(tokens[i + 1],
										"Error: Unidentified type for the '=' intrinsic\nExpected: \"%s\" or \"%s\" but found \"%s\"",
										human(Word).c_str(),
										human(constDef).c_str(),
										human(tokens[i + 1].type).c_str());
								exit(1);
							}
							*/
						}
					}
					else {
						// todo: generate some sort of error
						freeTokens(tokens);
						exit(1);
					}
				}
			} break;
			case IntrinsicType::PRINTC: {
				if (tokens[i + 1].type == constDef && tokens[i + 1].Inst == constInt) {
					fprintf(out, "   push %d\n", tokens[i + 1].operand);
					fprintf(out, "   call _putchar\n");
					fprintf(out, "   pop rax\n");
				}
				else if (tokens[i + 1].type == Word) {
					auto loc = varStack.find(*((std::string*)tokens[i + 1].value));
					if (loc == varStack.end()) {
						fprintf(stderr, "\n%s:%d:%d: ERROR: Undefined word \"%s\"\n", tokens[i].loc.file.c_str(), tokens[i].loc.linenumber, tokens[i].loc.character, (char*)tokens[i].value);
						freeTokens(tokens);
						exit(1);
					}
					else {
						if (getTypeSize(loc->second.type) == 1) {
							fprintf(out, "   xor rax, rax\n");
							fprintf(out, "   mov al, byte[rsp+%zu]\n", stack_size - loc->second.stack_offset);
							fprintf(out, "   push rax\n");
							fprintf(out, "   call _putchar\n");
							fprintf(out, "   pop  rax\n");
						}
						else if (getTypeSize(loc->second.type) == 4) {
							fprintf(out, "   xor rax, rax\n");
							fprintf(out, "   mov eax, dword[rsp+%zu]\n", stack_size - loc->second.stack_offset);
							fprintf(out, "   push rax\n");
							fprintf(out, "   call _putchar\n");
							fprintf(out, "   pop  rax\n");
						}
						else if (getTypeSize(loc->second.type) == 8) {
							fprintf(out, "   xor rax, rax\n");
							fprintf(out, "   mov rax, dword[rsp+%zu]\n", stack_size - loc->second.stack_offset);
							fprintf(out, "   push rax\n");
							fprintf(out, "   call _putchar\n");
							fprintf(out, "   pop  rax\n");
						}
						//printf("%d", loc->second.operand);
						//putchar(loc->second.operand);
					}
					/*for (size_t z = 0; z < varStack.size(); z++) {
						if (varStack[z].name == *((std::string*)tokens[i + 1].value)) {
							val = varStack[z].operand;
							hasFoundVal = true;
						}
					}*/

					/*if (!hasFoundVal) {
						fprintf(stderr, "\n%s:%d:%d: ERROR: Undefined word \"%s\"\n", tokens[i].loc.file.c_str(), tokens[i].loc.linenumber, tokens[i].loc.character,(char*)tokens[i].value);
						freeTokens(tokens);
						exit(1);
					}*/
					//printf("%c", val);
				}
				else {
					freeTokens(tokens);
					printCE(tokens[i], "Error: Unsupported type passed to print intrinsic\nExpected: \"%s\" but got \"%s\"\n", human(Word).c_str(), human(tokens[i + 1].type).c_str());
					exit(1);
				}
				/*if (tokens[i + 1].Inst == constInt) {
					printf("%d", tokens[i + 1].operand);
				}*/
				i++;
			} break;
			case IntrinsicType::INT_DEF: {
				if (tokens[i + 1].type != Word) {
					///printCE(tokens[i+1])
					freeTokens(tokens);
					printCE(tokens[i], "Invalid variable definition. Expected type \"WORD\" but got type: \"%s\"", human(tokens[i + 1].type).c_str());
					exit(1);
				}
				if (varStack.find(*((std::string*)tokens[i + 1].value)) != varStack.end()) {
					printCE(tokens[i], "Error: Symbol \"%s\" already defined!", (*((std::string*)tokens[i + 1].value)).c_str());
					freeTokens(tokens);
					exit(1);
				}
				fprintf(out, "   sub rsp, %d\n", 4);
				varStack[*((std::string*)tokens[i + 1].value)] = { varTypes::_INT,stack_size };
				stack_size += 4;
				i++;
			} break;
			default: 
			{
				fprintf(stderr, "%s:%d:%d: ERROR: Unhandled intrinsic \"%s\"\n", tokens[i].loc.file.c_str(), tokens[i].loc.linenumber, tokens[i].loc.character,
					findFirstByValueInMap<std::string, int>(IntrinsicNames, tokens[i].Inst).c_str()
				);
				freeTokens(tokens);
				exit(1);
			}
			}
		} break;
		default :{
			if (tokens[i].type == Word) {
				auto vv = varStack.find((*((std::string*)tokens[i].value)));
				if (vv == varStack.end()) {
					printCE(tokens[i], "Error: Undefined word: \"%s\"", (*((std::string*)tokens[i].value)).c_str());
					freeTokens(tokens);
					exit(1);
				}

				if (tokens[i + 1].type == InstructionType::Intrinsic) {
					if (tokens[i + 1].Inst == IntrinsicType::EQUALS) {}
					else {
						printCE(tokens[i], "Error: undefined symbol usage \"%s\"", vv->first.c_str());
						freeTokens(tokens);
						exit(1);
					}
				}
				// TODO: this should only work if there is I
			}
			else {
				printCE(tokens[i], "Error: Undefined token of type \"%s\", with instance: %d", human(tokens[i].type).c_str(), tokens[i].Inst);
				freeTokens(tokens);
				exit(1);
			}
		}
		}
	}
	fwrite_str(out, "   xor rax, rax\n");
	fwrite_str(out, "section .data\n");
	fwrite_str(out, "   intformt: db \"%d\",0\n");
	fwrite_str(out, "   chrformt: db \"%c\",0\n");
	fclose(out);
	/*
	    ofile.write("   intformt: db \"%d\",0\n")
        ofile.write("   chrformt: db \"%c\",0\n")
	*/
}
int main(int argc, char** argv) {
	assert(argc>2 && "Error: argc = 0! Provide more arguments\n");
	static_assert(IntrinsicType::MAX_TYPE == 10, "Unhandled intrinsics\n");
	IntrinsicNames["include"] = IntrinsicType::INCLUDE;
	IntrinsicNames["if"]	  = IntrinsicType::IF;
	IntrinsicNames["print"]   = IntrinsicType::PRINT;
	IntrinsicNames["printc"]  = IntrinsicType::PRINTC;
	IntrinsicNames["+"]		  = IntrinsicType::PLUS;
	IntrinsicNames["-"]       = IntrinsicType::MINUS;
	IntrinsicNames["*"]       = IntrinsicType::MULTIPLY;
	IntrinsicNames["/"]       = IntrinsicType::DIVIDE;
	IntrinsicNames["="]       = IntrinsicType::EQUALS;
	IntrinsicNames["int"]     = IntrinsicType::INT_DEF;
	char* path = argv[2];
	if (_chdir(dirname(path))) {
		fprintf(stderr, "Error: could not access chdir :(...\n");
		exit(1);
	}
	//fprintf(stderr, "Set cwd to: %s\n", dirname(path));
	if (!strcmp(argv[1], "com")) {
		std::vector<_Token> tokenized = Tokenize(basename(path));
		includeFiles(tokenized);
		//assert(false && "Unimplemented!\n");
		std::string opath = basename(path);
		opath = opath.substr(0, opath.length() - 3) + ".asm";
		compile(tokenized, opath);
	}
	if (!strcmp(argv[1], "sim")) {
		std::vector<_Token> tokenized = Tokenize(basename(path));
		includeFiles(tokenized);
		//Program p = _parse_program(tokenized);
		/*for (int i = 0; i < tokenized.size(); i++) {
			printf("T: %d\n",tokenized[i].type);
		}*/
		simulate(tokenized);

	}
}