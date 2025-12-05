#include "fx_core.h"
#include <iostream>
#include <sstream>
#include <vector>
#include <unordered_map>
#include <memory>
#include <functional>
#ifndef ARDUINO
#ifndef BARE_METAL
#include <fstream>
#include <cstdio>
#ifdef _WIN32
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#undef VOID
#undef TRUE
#undef FALSE
#else
#include <dirent.h>
#include <sys/stat.h>
#include <errno.h>
#endif
#endif
#endif
#ifdef ARDUINO
#include <Arduino.h>
#include <SD.h>
#endif

using namespace std;

enum class TokKind { IMPORT, AS, VAR, FUNC, VOID, INT, FLOAT, STRING, BOOL, WHILE, FOR, IF, ELSE, ELSEIF, BREAK, RETURN, TRUE, FALSE, IDENT, INT_LITERAL, FLOAT_LITERAL, STRING_LITERAL, LPAR, RPAR, LBRACE, RBRACE, LBRACKET, RBRACKET, SEMI, COLON, COMMA, DOT, PLUS, MINUS, STAR, SLASH, PERCENT, LT, GT, LE, GE, EQEQ, NEQ, EQ, PLUSEQ, MINUSEQ, STAREQ, SLASHEQ, ANDAND, OROR, PLUSPLUS, MINUSMINUS, NOT, TRY, EXCEPT, EOF_TOK };

struct Token { TokKind kind; string text; double num; size_t pos; };

struct Lexer {
    string s; size_t i=0;
    unordered_map<string, TokKind> kw;
    Lexer(const string& t): s(t) {
        kw = {{"import",TokKind::IMPORT},{"as",TokKind::AS},{"var",TokKind::VAR},{"func",TokKind::FUNC},{"void",TokKind::VOID},{"int",TokKind::INT},{"float",TokKind::FLOAT},{"string",TokKind::STRING},{"bool",TokKind::BOOL},{"while",TokKind::WHILE},{"for",TokKind::FOR},{"if",TokKind::IF},{"else",TokKind::ELSE},{"else-if",TokKind::ELSEIF},{"break",TokKind::BREAK},{"return",TokKind::RETURN},{"true",TokKind::TRUE},{"false",TokKind::FALSE},{"try",TokKind::TRY},{"except",TokKind::EXCEPT}};
    }
    char peek(){ return i<s.size()? s[i] : '\0'; }
    char adv(){ return i<s.size()? s[i++]:'\0'; }
    void skip(){ while(true){ if(i>=s.size()) break; char c=peek(); if(c==' '||c=='\t'||c=='\r'||c=='\n'){ i++; continue; } if(c=='-' && i+1<s.size() && s[i+1]=='-'){ while(i<s.size() && s[i]!='\n') i++; continue; } break; } }
    Token make(TokKind k, string t="", double n=0){ return Token{k,t,n,i}; }
    string readString(){ char q=adv(); string b; while(i<s.size()){ char c=adv(); if(c=='\\'){ if(i<s.size()) b.push_back(adv()); continue; } if(c==q) break; b.push_back(c);} return b; }
    string readNumber(){ size_t st=i; bool dot=false; while(i<s.size()){ char c=peek(); if(isdigit(c)){ i++; continue; } if(c=='.'){ if(dot) break; dot=true; i++; continue; } break; } return s.substr(st,i-st); }
    string readIdent(){ size_t st=i; while(i<s.size()){ char c=peek(); if(isalnum(c)||c=='_'){ i++; continue; } break; } return s.substr(st,i-st); }
    vector<Token> tokens(){ vector<Token> out; while(true){ skip(); if(i>=s.size()) break; char c=peek(); if(c=='('){ out.push_back(make(TokKind::LPAR,"(")); i++; continue;} if(c==')'){ out.push_back(make(TokKind::RPAR,")")); i++; continue;} if(c=='{'){ out.push_back(make(TokKind::LBRACE,"{")); i++; continue;} if(c=='}'){ out.push_back(make(TokKind::RBRACE,"}")); i++; continue;} if(c=='['){ out.push_back(make(TokKind::LBRACKET,"[")); i++; continue;} if(c==']'){ out.push_back(make(TokKind::RBRACKET,"]")); i++; continue;} if(c==';'){ out.push_back(make(TokKind::SEMI,";")); i++; continue;} if(c==':'){ out.push_back(make(TokKind::COLON,":")); i++; continue;} if(c==','){ out.push_back(make(TokKind::COMMA,",")); i++; continue;} if(c=='.'){ out.push_back(make(TokKind::DOT,".")); i++; continue;}
            if(c=='+'||c=='-'||c=='*'||c=='/'||c=='%'||c=='<'||c=='>'||c=='='||c=='!'||c=='&'||c=='|'){
                string two = s.substr(i, min<size_t>(2, s.size()-i));
                if(two=="++"){ out.push_back(make(TokKind::PLUSPLUS,"++")); i+=2; continue; }
                if(two=="--"){ out.push_back(make(TokKind::MINUSMINUS,"--")); i+=2; continue; }
                if(two=="=="){ out.push_back(make(TokKind::EQEQ,"==")); i+=2; continue; }
                if(two=="!="){ out.push_back(make(TokKind::NEQ,"!=")); i+=2; continue; }
                if(two==">="){ out.push_back(make(TokKind::GE,">=")); i+=2; continue; }
                if(two=="<="){ out.push_back(make(TokKind::LE,"<=")); i+=2; continue; }
                if(two=="+="){ out.push_back(make(TokKind::PLUSEQ,"+=")); i+=2; continue; }
                if(two=="-="){ out.push_back(make(TokKind::MINUSEQ,"-=")); i+=2; continue; }
                if(two=="*="){ out.push_back(make(TokKind::STAREQ,"*=")); i+=2; continue; }
                if(two=="/="){ out.push_back(make(TokKind::SLASHEQ,"/=")); i+=2; continue; }
                if(two=="&&"){ out.push_back(make(TokKind::ANDAND,"&&")); i+=2; continue; }
                if(two=="||"){ out.push_back(make(TokKind::OROR,"||")); i+=2; continue; }
                char ch=adv();
                if(ch=='+') out.push_back(make(TokKind::PLUS,"+"));
                else if(ch=='-') out.push_back(make(TokKind::MINUS,"-"));
                else if(ch=='*') out.push_back(make(TokKind::STAR,"*"));
                else if(ch=='/') out.push_back(make(TokKind::SLASH,"/"));
                else if(ch=='%') out.push_back(make(TokKind::PERCENT,"%"));
                else if(ch=='<') out.push_back(make(TokKind::LT,"<"));
                else if(ch=='>') out.push_back(make(TokKind::GT,">"));
                else if(ch=='=') out.push_back(make(TokKind::EQ,"="));
                else if(ch=='!') out.push_back(make(TokKind::NOT,"!"));
                else if(ch=='&'||ch=='|') throw runtime_error(string("Unexpected character ")+ch);
                continue;
            }
            if(c=='"' || c=='\''){ string v=readString(); out.push_back(Token{TokKind::STRING_LITERAL, v, 0, i}); continue; }
            if(isdigit(c)){ string v=readNumber(); if(v.find('.')!=string::npos){ out.push_back(Token{TokKind::FLOAT_LITERAL, v, stod(v), i}); } else { out.push_back(Token{TokKind::INT_LITERAL, v, (double)stoll(v), i}); } continue; }
            if(isalpha(c)||c=='_'){ string id=readIdent(); if(id=="else" && i+3<=s.size() && s[i]=='-' && s.substr(i+1,2)=="if"){ i+=3; out.push_back(Token{TokKind::ELSEIF,"else-if",0,i}); continue; } if(id=="try"){ out.push_back(Token{TokKind::TRY,id,0,i}); continue; } if(id=="except"){ out.push_back(Token{TokKind::EXCEPT,id,0,i}); continue; } auto it=kw.find(id); if(it!=kw.end()) out.push_back(Token{it->second,id,0,i}); else out.push_back(Token{TokKind::IDENT,id,0,i}); continue; }
            throw runtime_error(string("Unexpected character ")+c);
        }
        out.push_back(Token{TokKind::EOF_TOK,"",0,i});
        return out;
    }
};

struct Expr { virtual ~Expr(){} }; using ExprPtr=shared_ptr<Expr>;
struct IntExpr:Expr{ long long v; IntExpr(long long x):v(x){} };
struct FloatExpr:Expr{ double v; FloatExpr(double x):v(x){} };
struct StringExpr:Expr{ string v; StringExpr(string x):v(move(x)){} };
struct BoolExpr:Expr{ bool v; BoolExpr(bool x):v(x){} };
struct IdentExpr:Expr{ string name; IdentExpr(string n):name(move(n)){} };
struct UnaryExpr:Expr{ string op; ExprPtr expr; UnaryExpr(string o, ExprPtr e):op(move(o)),expr(move(e)){} };
struct BinaryExpr:Expr{ string op; ExprPtr l,r; BinaryExpr(string o, ExprPtr a, ExprPtr b):op(move(o)),l(move(a)),r(move(b)){} };
struct AssignExpr:Expr{ string op; ExprPtr left,right; AssignExpr(string o, ExprPtr l, ExprPtr r):op(move(o)),left(move(l)),right(move(r)){} };
struct MemberExpr:Expr{ ExprPtr obj; string prop; MemberExpr(ExprPtr o,string p):obj(move(o)),prop(move(p)){} };
struct CallExpr:Expr{ ExprPtr callee; vector<ExprPtr> args; CallExpr(ExprPtr c, vector<ExprPtr> a):callee(move(c)),args(move(a)){} };
struct UpdateExpr:Expr{ string op; ExprPtr arg; bool prefix; UpdateExpr(string o, ExprPtr a, bool p):op(move(o)),arg(move(a)),prefix(p){} };
struct ListExpr:Expr{ vector<ExprPtr> items; ListExpr(vector<ExprPtr> v):items(move(v)){} };
struct DictExpr:Expr{ vector<pair<string,ExprPtr>> items; DictExpr(vector<pair<string,ExprPtr>> v):items(move(v)){} };
struct IndexExpr:Expr{ ExprPtr obj; ExprPtr index; IndexExpr(ExprPtr o, ExprPtr i):obj(move(o)), index(move(i)){} };

struct Stmt { virtual ~Stmt(){} }; using StmtPtr=shared_ptr<Stmt>;
struct VarDeclStmt:Stmt{ string typ,name; ExprPtr value; VarDeclStmt(string t,string n,ExprPtr v):typ(move(t)),name(move(n)),value(move(v)){} };
struct ExprStmt:Stmt{ ExprPtr expr; ExprStmt(ExprPtr e):expr(move(e)){} };
struct WhileStmt:Stmt{ ExprPtr cond; vector<StmtPtr> body; WhileStmt(ExprPtr c, vector<StmtPtr> b):cond(move(c)),body(move(b)){} };
struct ForStmt:Stmt{ ExprPtr init,cond,post; vector<StmtPtr> body; ForStmt(ExprPtr i,ExprPtr c,ExprPtr p,vector<StmtPtr> b):init(move(i)),cond(move(c)),post(move(p)),body(move(b)){} };
struct IfStmt:Stmt{ ExprPtr cond; vector<StmtPtr> thenBody; vector<pair<ExprPtr, vector<StmtPtr>>> elifs; vector<StmtPtr> elseBody; IfStmt(ExprPtr c, vector<StmtPtr> t, vector<pair<ExprPtr, vector<StmtPtr>>> e, vector<StmtPtr> eb):cond(move(c)),thenBody(move(t)),elifs(move(e)),elseBody(move(eb)){} };
struct BreakStmt:Stmt{};
struct ReturnStmt:Stmt{ ExprPtr value; ReturnStmt(ExprPtr v):value(move(v)){} };
struct TryStmt:Stmt{ vector<StmtPtr> tryBody; vector<StmtPtr> exceptBody; TryStmt(vector<StmtPtr> t, vector<StmtPtr> e):tryBody(move(t)), exceptBody(move(e)){} };

struct FuncDecl { string ret,name; vector<pair<string,string>> params; vector<StmtPtr> body; };
struct ImportDecl { string name, version, alias; string path; string varName; };
struct Program { vector<ImportDecl> imports; vector<VarDeclStmt> globals; unordered_map<string, FuncDecl> funcs; };
struct ImportStmtS:Stmt{ ImportDecl imp; ImportStmtS(ImportDecl d):imp(move(d)){} };

struct Parser {
    vector<Token> ts; size_t i=0;
    Parser(vector<Token> t): ts(move(t)){}
    Token& peek(){ return ts[i]; }
    bool match(TokKind k){ if(peek().kind==k){ i++; return true; } return false; }
    const char* name(TokKind k){ switch(k){ case TokKind::IMPORT: return "IMPORT"; case TokKind::AS: return "AS"; case TokKind::VAR: return "VAR"; case TokKind::FUNC: return "FUNC"; case TokKind::VOID: return "VOID"; case TokKind::INT: return "INT"; case TokKind::FLOAT: return "FLOAT"; case TokKind::STRING: return "STRING"; case TokKind::BOOL: return "BOOL"; case TokKind::WHILE: return "WHILE"; case TokKind::FOR: return "FOR"; case TokKind::IF: return "IF"; case TokKind::ELSE: return "ELSE"; case TokKind::ELSEIF: return "ELSEIF"; case TokKind::BREAK: return "BREAK"; case TokKind::RETURN: return "RETURN"; case TokKind::TRUE: return "TRUE"; case TokKind::FALSE: return "FALSE"; case TokKind::IDENT: return "IDENT"; case TokKind::INT_LITERAL: return "INT_LITERAL"; case TokKind::FLOAT_LITERAL: return "FLOAT_LITERAL"; case TokKind::STRING_LITERAL: return "STRING_LITERAL"; case TokKind::LPAR: return "("; case TokKind::RPAR: return ")"; case TokKind::LBRACE: return "{"; case TokKind::RBRACE: return "}"; case TokKind::LBRACKET: return "["; case TokKind::RBRACKET: return "]"; case TokKind::SEMI: return ";"; case TokKind::COLON: return ":"; case TokKind::COMMA: return ","; case TokKind::DOT: return "."; case TokKind::PLUS: return "+"; case TokKind::MINUS: return "-"; case TokKind::STAR: return "*"; case TokKind::SLASH: return "/"; case TokKind::PERCENT: return "%"; case TokKind::LT: return "<"; case TokKind::GT: return ">"; case TokKind::LE: return "<="; case TokKind::GE: return ">="; case TokKind::EQEQ: return "=="; case TokKind::NEQ: return "!="; case TokKind::EQ: return "="; case TokKind::PLUSEQ: return "+="; case TokKind::MINUSEQ: return "-="; case TokKind::STAREQ: return "*="; case TokKind::SLASHEQ: return "/="; case TokKind::ANDAND: return "&&"; case TokKind::OROR: return "||"; case TokKind::PLUSPLUS: return "++"; case TokKind::MINUSMINUS: return "--"; case TokKind::NOT: return "!"; case TokKind::TRY: return "try"; case TokKind::EXCEPT: return "except"; case TokKind::EOF_TOK: return "EOF"; default: return "?"; } }
    Token expect(TokKind k){ Token &t=peek(); if(t.kind!=k){ std::ostringstream oss; oss<<"Expected token "<<name(k)<<" but saw "<<name(t.kind); throw runtime_error(oss.str()); } i++; return t; }
    string parseType(){ TokKind k=peek().kind; if(k==TokKind::VOID||k==TokKind::INT||k==TokKind::FLOAT||k==TokKind::STRING||k==TokKind::BOOL){ string s; if(k==TokKind::VOID) s="void"; else if(k==TokKind::INT) s="int"; else if(k==TokKind::FLOAT) s="float"; else if(k==TokKind::STRING) s="string"; else s="bool"; i++; return s;} throw runtime_error("Type expected"); }
Program parse(){ Program p; while(peek().kind!=TokKind::EOF_TOK){ if(peek().kind==TokKind::IMPORT){ p.imports.push_back(parseImport()); } else if(peek().kind==TokKind::VAR){ p.globals.push_back(parseVarDecl()); } else if(peek().kind==TokKind::FUNC){ auto f=parseFuncDecl(); p.funcs[f.name]=move(f); } else { throw runtime_error("Unexpected token"); } } return p; }
ImportDecl parseImport(){ expect(TokKind::IMPORT); ImportDecl imp; if(peek().kind==TokKind::STRING_LITERAL){ imp.path=expect(TokKind::STRING_LITERAL).text; expect(TokKind::AS); imp.alias=expect(TokKind::IDENT).text; return imp; } string ident=expect(TokKind::IDENT).text; if(match(TokKind::COLON)){ string ver; while(peek().kind==TokKind::INT_LITERAL || peek().kind==TokKind::FLOAT_LITERAL || peek().kind==TokKind::DOT){ if(peek().kind==TokKind::INT_LITERAL){ ver+=expect(TokKind::INT_LITERAL).text; continue; } if(peek().kind==TokKind::FLOAT_LITERAL){ ver+=expect(TokKind::FLOAT_LITERAL).text; continue; } if(peek().kind==TokKind::DOT){ ver+="."; expect(TokKind::DOT); continue; } break; } imp.name=ident; imp.version=ver; expect(TokKind::AS); imp.alias=expect(TokKind::IDENT).text; return imp; } else { imp.varName=ident; expect(TokKind::AS); imp.alias=expect(TokKind::IDENT).text; return imp; } }
    VarDeclStmt parseVarDecl(){ expect(TokKind::VAR); string typ=parseType(); string name=expect(TokKind::IDENT).text; expect(TokKind::EQ); auto v=parseExpression(); expect(TokKind::SEMI); return VarDeclStmt(typ,name,v); }
    FuncDecl parseFuncDecl(){ expect(TokKind::FUNC); string ret=parseType(); string name=expect(TokKind::IDENT).text; expect(TokKind::LPAR); vector<pair<string,string>> params; if(peek().kind!=TokKind::RPAR){ while(true){ string pt=parseType(); string pn=expect(TokKind::IDENT).text; params.push_back({pt,pn}); if(match(TokKind::COMMA)) continue; else break; } } expect(TokKind::RPAR); auto body=parseBlock(); FuncDecl f; f.ret=ret; f.name=name; f.params=params; f.body=move(body); return f; }
    vector<StmtPtr> parseBlock(){ expect(TokKind::LBRACE); vector<StmtPtr> items; while(peek().kind!=TokKind::RBRACE){ items.push_back(parseStatement()); } expect(TokKind::RBRACE); return items; }
    StmtPtr parseTry(){ expect(TokKind::TRY); auto t=parseBlock(); expect(TokKind::EXCEPT); auto e=parseBlock(); return make_shared<TryStmt>(t,e); }
    StmtPtr parseStatement(){ TokKind k=peek().kind; if(k==TokKind::IMPORT){ auto d=parseImport(); return make_shared<ImportStmtS>(d); } if(k==TokKind::VAR) return make_shared<VarDeclStmt>(parseVarDecl()); if(k==TokKind::WHILE) return parseWhile(); if(k==TokKind::FOR) return parseFor(); if(k==TokKind::IF) return parseIf(); if(k==TokKind::TRY) return parseTry(); if(k==TokKind::BREAK){ expect(TokKind::BREAK); expect(TokKind::SEMI); return make_shared<BreakStmt>(); } if(k==TokKind::RETURN){ expect(TokKind::RETURN); ExprPtr v; if(peek().kind!=TokKind::SEMI) v=parseExpression(); expect(TokKind::SEMI); return make_shared<ReturnStmt>(v); } auto e=parseExpression(); expect(TokKind::SEMI); return make_shared<ExprStmt>(e); }
    StmtPtr parseWhile(){ expect(TokKind::WHILE); expect(TokKind::LPAR); auto c=parseExpression(); expect(TokKind::RPAR); auto b=parseBlock(); return make_shared<WhileStmt>(c,b); }
    StmtPtr parseFor(){ expect(TokKind::FOR); expect(TokKind::LPAR); ExprPtr init; if(peek().kind!=TokKind::SEMI) init=parseExpression(); expect(TokKind::SEMI); ExprPtr cond; if(peek().kind!=TokKind::SEMI) cond=parseExpression(); expect(TokKind::SEMI); ExprPtr post; if(peek().kind!=TokKind::RPAR) post=parseExpression(); expect(TokKind::RPAR); auto b=parseBlock(); return make_shared<ForStmt>(init,cond,post,b); }
    StmtPtr parseIf(){ expect(TokKind::IF); expect(TokKind::LPAR); auto c=parseExpression(); expect(TokKind::RPAR); vector<StmtPtr> thenBody; if(peek().kind==TokKind::LBRACE){ thenBody = parseBlock(); } else { thenBody = { parseStatement() }; }
        vector<pair<ExprPtr, vector<StmtPtr>>> elifs; while(peek().kind==TokKind::ELSEIF){ expect(TokKind::ELSEIF); expect(TokKind::LPAR); auto ec=parseExpression(); expect(TokKind::RPAR); vector<StmtPtr> eb; if(peek().kind==TokKind::LBRACE){ eb = parseBlock(); } else { eb = { parseStatement() }; } elifs.push_back({ec,eb}); }
        vector<StmtPtr> eb; if(peek().kind==TokKind::ELSE){ expect(TokKind::ELSE); if(peek().kind==TokKind::LBRACE){ eb = parseBlock(); } else { eb = { parseStatement() }; } }
        return make_shared<IfStmt>(c,thenBody,elifs,eb);
    }
    ExprPtr parseExpression(){ return parseAssignment(); }
    ExprPtr parseAssignment(){ auto e=parseLogicOr(); TokKind k=peek().kind; if(k==TokKind::EQ||k==TokKind::PLUSEQ||k==TokKind::MINUSEQ||k==TokKind::STAREQ||k==TokKind::SLASHEQ){ string op=expect(k).text; auto r=parseAssignment(); return make_shared<AssignExpr>(op,e,r); } return e; }
    ExprPtr parseLogicOr(){ auto e=parseLogicAnd(); while(peek().kind==TokKind::OROR){ expect(TokKind::OROR); auto r=parseLogicAnd(); e=make_shared<BinaryExpr>("||",e,r);} return e; }
    ExprPtr parseLogicAnd(){ auto e=parseEquality(); while(peek().kind==TokKind::ANDAND){ expect(TokKind::ANDAND); auto r=parseEquality(); e=make_shared<BinaryExpr>("&&",e,r);} return e; }
    ExprPtr parseEquality(){ auto e=parseComparison(); while(peek().kind==TokKind::EQEQ||peek().kind==TokKind::NEQ){ TokKind k=peek().kind; string op=expect(k).text; auto r=parseComparison(); e=make_shared<BinaryExpr>(op,e,r);} return e; }
    ExprPtr parseComparison(){ auto e=parseTerm(); while(peek().kind==TokKind::LT||peek().kind==TokKind::GT||peek().kind==TokKind::LE||peek().kind==TokKind::GE){ TokKind k=peek().kind; string op=expect(k).text; auto r=parseTerm(); e=make_shared<BinaryExpr>(op,e,r);} return e; }
    ExprPtr parseTerm(){ auto e=parseFactor(); while(peek().kind==TokKind::PLUS||peek().kind==TokKind::MINUS){ TokKind k=peek().kind; string op=expect(k).text; auto r=parseFactor(); e=make_shared<BinaryExpr>(op,e,r);} return e; }
    ExprPtr parseFactor(){ auto e=parseUnary(); while(peek().kind==TokKind::STAR||peek().kind==TokKind::SLASH||peek().kind==TokKind::PERCENT){ TokKind k=peek().kind; string op=expect(k).text; auto r=parseUnary(); e=make_shared<BinaryExpr>(op,e,r);} return e; }
    ExprPtr parseUnary(){ TokKind k=peek().kind; if(k==TokKind::MINUS||k==TokKind::PLUS||k==TokKind::PLUSPLUS||k==TokKind::MINUSMINUS||k==TokKind::NOT){ string op=expect(k).text; if(op=="++"||op=="--"){ auto t=parseUnary(); return make_shared<UpdateExpr>(op,t,true);} auto r=parseUnary(); return make_shared<UnaryExpr>(op,r);} return parseCall(); }
    ExprPtr parseCall(){ auto e=parsePrimary(); while(true){ if(peek().kind==TokKind::LPAR){ vector<ExprPtr> args; expect(TokKind::LPAR); if(peek().kind!=TokKind::RPAR){ while(true){ args.push_back(parseExpression()); if(match(TokKind::COMMA)) continue; else break; } } expect(TokKind::RPAR); e=make_shared<CallExpr>(e,args); continue; } if(peek().kind==TokKind::DOT){ expect(TokKind::DOT); string name=expect(TokKind::IDENT).text; e=make_shared<MemberExpr>(e,name); continue; } if(peek().kind==TokKind::LBRACKET){ expect(TokKind::LBRACKET); auto idx=parseExpression(); expect(TokKind::RBRACKET); e=make_shared<IndexExpr>(e, idx); continue; } if(peek().kind==TokKind::PLUSPLUS||peek().kind==TokKind::MINUSMINUS){ string op=expect(peek().kind).text; e=make_shared<UpdateExpr>(op,e,false); continue; } break; } return e; }
    ExprPtr parsePrimary(){ Token &t=peek(); if(t.kind==TokKind::INT_LITERAL){ long long v=(long long)t.num; expect(TokKind::INT_LITERAL); return make_shared<IntExpr>(v);} if(t.kind==TokKind::FLOAT_LITERAL){ double v=t.num; expect(TokKind::FLOAT_LITERAL); return make_shared<FloatExpr>(v);} if(t.kind==TokKind::STRING_LITERAL){ string v=t.text; expect(TokKind::STRING_LITERAL); return make_shared<StringExpr>(v);} if(t.kind==TokKind::TRUE){ expect(TokKind::TRUE); return make_shared<BoolExpr>(true);} if(t.kind==TokKind::FALSE){ expect(TokKind::FALSE); return make_shared<BoolExpr>(false);} if(t.kind==TokKind::IDENT){ string n=t.text; expect(TokKind::IDENT); return make_shared<IdentExpr>(n);} if(t.kind==TokKind::LPAR){ expect(TokKind::LPAR); auto e=parseExpression(); expect(TokKind::RPAR); return e;} if(t.kind==TokKind::LBRACKET){ expect(TokKind::LBRACKET); vector<ExprPtr> items; if(peek().kind!=TokKind::RBRACKET){ while(true){ items.push_back(parseExpression()); if(match(TokKind::COMMA)) continue; else break; } } expect(TokKind::RBRACKET); return make_shared<ListExpr>(items);} if(t.kind==TokKind::LBRACE){ expect(TokKind::LBRACE); vector<pair<string,ExprPtr>> items; if(peek().kind!=TokKind::RBRACE){ while(true){ string key; if(peek().kind==TokKind::STRING_LITERAL){ key=expect(TokKind::STRING_LITERAL).text; } else { key=expect(TokKind::IDENT).text; } expect(TokKind::COLON); auto val=parseExpression(); items.push_back({key,val}); if(match(TokKind::COMMA)) continue; else break; } } expect(TokKind::RBRACE); return make_shared<DictExpr>(items);} throw runtime_error("Expression expected"); }
};

struct Value { enum Type { NONE, INT, FLOAT, STRING, BOOL, LIST, DICT } type = NONE; long long i = 0; double d = 0.0; string s; bool b = false; vector<shared_ptr<Value>> list; unordered_map<string, shared_ptr<Value>> dict; };

struct Env{
    shared_ptr<Env> parent; unordered_map<string, Value> vals;
    Env(shared_ptr<Env> p=nullptr):parent(p){}
    void define(const string& n, const Value& val){ vals[n]=val; }
    void set(const string& n, const Value& val){ auto it=vals.find(n); if(it!=vals.end()){ it->second=val; return;} if(parent){ parent->set(n,val); return;} throw runtime_error("Undefined variable"); }
    Value get(const string& n){ auto it=vals.find(n); if(it!=vals.end()) return it->second; if(parent) return parent->get(n); throw runtime_error(string("Undefined variable: ")+n); }
};

struct BreakSignal{};
struct ReturnSignal{ Value v; };

struct Interpreter;
struct BuiltinModule{ unordered_map<string, function<Value(vector<Value>)>> fns; };
struct Module{ bool builtin; BuiltinModule bi; shared_ptr<Interpreter> interp; };

struct Interpreter{
    Program prog; shared_ptr<Env> globals=make_shared<Env>(); unordered_map<string, FuncDecl> functions; unordered_map<string, Module> modules; ReadFileFn reader; PrintFn printer; DrawRectFn drawRectCb=nullptr; DrawTextFn drawTextCb=nullptr; TouchReadFn touchReadCb=nullptr;
    Interpreter(Program p, ReadFileFn r, PrintFn pr):prog(move(p)), reader(r), printer(pr){}
    static bool truthy(const Value& x){ if(x.type==Value::BOOL) return x.b; if(x.type==Value::INT) return x.i!=0; if(x.type==Value::FLOAT) return x.d!=0.0; if(x.type==Value::STRING) return !x.s.empty(); return false; }
    static Value make_int(long long x){ Value v; v.type=Value::INT; v.i=x; return v; }
    static Value make_float(double x){ Value v; v.type=Value::FLOAT; v.d=x; return v; }
    static Value make_str(const string& x){ Value v; v.type=Value::STRING; v.s=x; return v; }
    static Value make_bool(bool x){ Value v; v.type=Value::BOOL; v.b=x; return v; }
    static double num_to_double(const Value& a){ if(a.type==Value::FLOAT) return a.d; if(a.type==Value::INT) return (double)a.i; throw runtime_error("Numeric expected"); }
    static long long num_to_int(const Value& a){ if(a.type==Value::INT) return a.i; if(a.type==Value::FLOAT) return (long long)a.d; throw runtime_error("Numeric expected"); }
    static string to_string_value(const Value& a){ if(a.type==Value::STRING) return a.s; if(a.type==Value::INT) return to_string(a.i); if(a.type==Value::FLOAT){ ostringstream oss; oss<<a.d; return oss.str(); } if(a.type==Value::BOOL) return a.b?"true":"false"; if(a.type==Value::LIST){ string out="["; for(size_t i=0;i<a.list.size();++i){ out+=to_string_value(*a.list[i]); if(i+1<a.list.size()) out+=", "; } out+="]"; return out; } if(a.type==Value::DICT){ string out="{"; bool first=true; for(auto &kv: a.dict){ if(!first) out+=", "; first=false; out+=kv.first+": "+to_string_value(*kv.second); } out+="}"; return out; } return string(); }
    static bool equal_values(const Value& a, const Value& b){ if(a.type==Value::STRING && b.type==Value::STRING) return a.s==b.s; if((a.type==Value::INT||a.type==Value::FLOAT) && (b.type==Value::INT||b.type==Value::FLOAT)) return num_to_double(a)==num_to_double(b); if(a.type==Value::BOOL && b.type==Value::BOOL) return a.b==b.b; return false; }
    void eval(){ for(auto &imp: prog.imports) handle_import(imp); for(auto &g: prog.globals){ Value v=eval_expr(g.value, globals); globals->define(g.name, v);} functions=prog.funcs; }

    void load_module_from_path(const string& p, const string& alias){
        string text = reader ? reader(p) : string();
        if(text.empty()) throw runtime_error(string("Module file not found: ")+p);
        Lexer lx(text); auto toks=lx.tokens(); Parser ps(toks); Program mp=ps.parse();
        auto child=make_shared<Interpreter>(mp, reader, printer);
        child->drawRectCb=drawRectCb;
        child->drawTextCb=drawTextCb;
        child->touchReadCb=touchReadCb;
        child->eval();
        Module m; m.builtin=false; m.interp=child;
        modules[alias]=m;
    }

    static string trim_ws(const string& s){ size_t a=0; while(a<s.size() && (s[a]==' '||s[a]=='\t'||s[a]=='\r'||s[a]=='\n')) a++; size_t b=s.size(); while(b>a && (s[b-1]==' '||s[b-1]=='\t'||s[b-1]=='\r'||s[b-1]=='\n')) b--; return s.substr(a,b-a); }
    static Value make_dict(){ Value v; v.type=Value::DICT; return v; }
    static Value make_list(){ Value v; v.type=Value::LIST; return v; }
    static void dict_set(Value& d, const string& k, const Value& v){ d.dict[k]=make_shared<Value>(v); }
    static Value parse_config_text(const string& text){ Value root=make_dict(); string cur; string line; for(size_t i=0;i<=text.size();++i){ char c = (i<text.size()? text[i]:'\n'); if(c=='\n'){ string t=trim_ws(line); line.clear(); if(t.empty()) continue; if(t.size()>2 && t.front()=='[' && t.back()==']'){ cur = t.substr(1,t.size()-2); if(root.dict.find(cur)==root.dict.end()){ Value sec=make_dict(); dict_set(root, cur, sec); } continue; } size_t eq = t.find('='); if(eq!=string::npos){ string key = trim_ws(t.substr(0,eq)); string val = trim_ws(t.substr(eq+1)); if(cur.empty()){ if(root.dict.find("root")==root.dict.end()){ Value sec=make_dict(); dict_set(root, "root", sec); } Value sec = *root.dict["root"]; dict_set(sec, key, Interpreter::make_str(val)); dict_set(root, "root", sec); } else { Value sec = *root.dict[cur]; dict_set(sec, key, Interpreter::make_str(val)); dict_set(root, cur, sec); } } continue; } else { line.push_back(c); } } return root; }
    struct JsonParser{ string s; size_t i=0; JsonParser(string t):s(move(t)){} char peek(){ return i<s.size()? s[i]:'\0'; } char adv(){ return i<s.size()? s[i++]:'\0'; } void skip(){ while(i<s.size()){ char c=peek(); if(c==' '||c=='\t'||c=='\r'||c=='\n') { i++; continue; } break; } } Value parse(){ skip(); char c=peek(); if(c=='{') return parseObj(); if(c=='[') return parseArr(); if(c=='"') return parseStr(); if(c=='-'||isdigit(c)) return parseNum(); if(s.compare(i,4,"true")==0){ i+=4; return Interpreter::make_bool(true);} if(s.compare(i,5,"false")==0){ i+=5; return Interpreter::make_bool(false);} if(s.compare(i,4,"null")==0){ i+=4; Value v; return v;} Value v; return v; } Value parseObj(){ Value v=make_dict(); expect('{'); skip(); if(peek()=='}'){ adv(); return v; } while(true){ Value keyv = parseStr(); skip(); expect(':'); Value val = parse(); dict_set(v, keyv.s, val); skip(); if(peek()==','){ adv(); skip(); continue; } expect('}'); break; } return v; } Value parseArr(){ Value v=make_list(); expect('['); skip(); if(peek()==']'){ adv(); return v; } while(true){ Value it = parse(); v.list.push_back(make_shared<Value>(it)); skip(); if(peek()==','){ adv(); skip(); continue; } expect(']'); break; } return v; } Value parseStr(){ expect('"'); string out; while(i<s.size()){ char c=adv(); if(c=='\\'){ if(i<s.size()){ char e=adv(); out.push_back(e); } continue; } if(c=='"') break; out.push_back(c);} return Interpreter::make_str(out); } Value parseNum(){ size_t st=i; bool dot=false; while(i<s.size()){ char c=peek(); if(isdigit(c)){ i++; continue; } if(c=='.'||c=='e'||c=='E'){ dot=true; i++; continue; } if(c=='+'||c=='-'){ if(i>st && (s[i-1]=='e'||s[i-1]=='E')){ i++; continue; } break; } break; } string t=s.substr(st,i-st); if(dot) return Interpreter::make_float(strtod(t.c_str(), nullptr)); return Interpreter::make_int(stoll(t)); } void expect(char ch){ char c=adv(); if(c!=ch) throw runtime_error("Invalid JSON"); } };
    void handle_import(const ImportDecl& imp){
        if(!imp.path.empty()){ load_module_from_path(imp.path, imp.alias); return; }
        string path = string("libs/")+imp.name+"/"+imp.version+"/entry.fx";
        string text = reader ? reader(path) : string();
        if(!text.empty()){
            Lexer lx(text); auto toks=lx.tokens(); Parser ps(toks); Program mp=ps.parse(); auto child=make_shared<Interpreter>(mp, reader, printer); child->drawRectCb=drawRectCb; child->drawTextCb=drawTextCb; child->eval(); Module m; m.builtin=false; m.interp=child; modules[imp.alias]=m; return;
        }
        if(imp.name=="IO"){
            BuiltinModule bi;
            bi.fns["print"]= [this](vector<Value> args)->Value{ string out; for(size_t i=0;i<args.size();++i){ out += to_string_value(args[i]); if(i+1<args.size()) out+=' '; } if(printer) printer(out+"\n"); return Value{}; };
            bi.fns["readFile"]= [this](vector<Value> args)->Value{
                string p = to_string_value(args[0]);
                string data;
                #if defined(ARDUINO)
                File f = SD.open(p.c_str(), FILE_READ);
                if(f){ while(f.available()){ data.push_back((char)f.read()); } f.close(); }
                #elif defined(BARE_METAL)
                data = reader ? reader(p) : string();
                #else
                ifstream in(p, ios::binary);
                if(in.good()){ ostringstream ss; ss<<in.rdbuf(); data=ss.str(); }
                #endif
                return make_str(data);
            };
            bi.fns["writeFile"]= [this](vector<Value> args)->Value{
                string p = to_string_value(args[0]);
                string d = to_string_value(args[1]);
                bool ok=false;
                #ifdef ARDUINO
                SD.remove(p.c_str());
                File f = SD.open(p.c_str(), FILE_WRITE);
                if(f){ f.write((const uint8_t*)d.data(), d.size()); f.close(); ok=true; }
                #elif defined(BARE_METAL)
                ok=false;
                #else
                ofstream out(p, ios::binary|ios::trunc);
                if(out.good()){ out.write(d.data(), (streamsize)d.size()); out.close(); ok=true; }
                #endif
                return make_bool(ok);
            };
            bi.fns["remove"]= [this](vector<Value> args)->Value{
                string p = to_string_value(args[0]);
                bool ok=false;
                #ifdef ARDUINO
                ok = SD.remove(p.c_str());
                #elif defined(BARE_METAL)
                ok=false;
                #else
                ok = std::remove(p.c_str())==0;
                #endif
                return make_bool(ok);
            };
            bi.fns["mkdir"]= [this](vector<Value> args)->Value{
                string p = to_string_value(args[0]);
                bool ok=false;
                #ifdef ARDUINO
                ok = SD.mkdir(p.c_str());
                #elif defined(BARE_METAL)
                ok=false;
                #else
                #ifdef _WIN32
                string q=p; for(size_t i=0;i<q.size();++i) if(q[i]=='/') q[i]='\\'; ok = CreateDirectoryA(q.c_str(), NULL) || GetLastError()==ERROR_ALREADY_EXISTS;
                #else
                ok = false;
                #endif
                #endif
                return make_bool(ok);
            };
            bi.fns["readdir"]= [this](vector<Value> args)->Value{
                string p = to_string_value(args[0]);
                string out;
                #ifdef ARDUINO
                File dir = SD.open(p.c_str());
                if(dir && dir.isDirectory()){
                    while(true){ File entry = dir.openNextFile(); if(!entry) break; out += string(entry.name()); out += "\n"; entry.close(); }
                    dir.close();
                }
                #elif defined(BARE_METAL)
                out = reader ? reader(p) : string();
                #else
                #ifdef _WIN32
                string pat=p; for(size_t i=0;i<pat.size();++i) if(pat[i]=='/') pat[i]='\\'; if(!pat.empty() && pat.back()!='\\') pat += "\\"; pat += "*";
                WIN32_FIND_DATAA ffd; HANDLE h = FindFirstFileA(pat.c_str(), &ffd);
                if(h!=INVALID_HANDLE_VALUE){ do{ string name = ffd.cFileName; if(name!="." && name!="..") { out += name; out += "\n"; } } while(FindNextFileA(h, &ffd)); FindClose(h); }
                #endif
                #endif
                if(!out.empty() && out.back()=='\n') out.pop_back();
                return make_str(out);
            };
            modules[imp.alias]=Module{true,bi,nullptr}; return;
        }
        if(imp.name=="Arduino"){
            BuiltinModule bi;
            bi.fns["pinMode"]= [](vector<Value> args)->Value{
                #ifdef ARDUINO
                long long pin = Interpreter::num_to_int(args[0]);
                string mode = Interpreter::to_string_value(args[1]);
                int m = INPUT;
                if(mode=="OUTPUT") m=OUTPUT; else if(mode=="INPUT_PULLUP") m=INPUT_PULLUP; else m=INPUT;
                ::pinMode((uint8_t)pin, m);
                #endif
                return Value{};
            };
            bi.fns["digitalWrite"]= [](vector<Value> args)->Value{
                #ifdef ARDUINO
                long long pin = Interpreter::num_to_int(args[0]);
                bool v = args[1].type==Value::BOOL ? args[1].b : Interpreter::num_to_int(args[1])!=0;
                ::digitalWrite((uint8_t)pin, v?HIGH:LOW);
                #endif
                return Value{};
            };
            bi.fns["digitalRead"]= [](vector<Value> args)->Value{
                #ifdef ARDUINO
                long long pin = Interpreter::num_to_int(args[0]);
                int r = ::digitalRead((uint8_t)pin);
                return Interpreter::make_int(r);
                #else
                return Value{};
                #endif
            };
            modules[imp.alias]=Module{true,bi,nullptr}; return;
        }
        if(imp.name=="Time"){
            BuiltinModule bi;
            bi.fns["millis"]= [](vector<Value>)->Value{
                #ifdef ARDUINO
                return Interpreter::make_int((long long)::millis());
                #else
                return Interpreter::make_int(0);
                #endif
            };
            bi.fns["delay"]= [](vector<Value> args)->Value{
                #ifdef ARDUINO
                ::delay((unsigned long)Interpreter::num_to_int(args[0]));
                #endif
                return Value{};
            };
            modules[imp.alias]=Module{true,bi,nullptr}; return;
        }
        if(imp.name=="Display"){
            BuiltinModule bi;
            bi.fns["drawRect"]= [this](vector<Value> args)->Value{
                if(!drawRectCb) throw runtime_error("Display not configured");
                int x = (int)num_to_int(args[0]);
                int y = (int)num_to_int(args[1]);
                int w = (int)num_to_int(args[2]);
                int h = (int)num_to_int(args[3]);
                unsigned short color = (unsigned short)num_to_int(args[4]);
                bool fill = args.size()>5 ? (args[5].type==Value::BOOL ? args[5].b : num_to_int(args[5])!=0) : false;
                drawRectCb(x,y,w,h,color,fill);
                return Value{};
            };
            bi.fns["drawText"]= [this](vector<Value> args)->Value{
                if(!drawTextCb) throw runtime_error("Display not configured");
                int x = (int)num_to_int(args[0]);
                int y = (int)num_to_int(args[1]);
                string text = to_string_value(args[2]);
                unsigned short color = (unsigned short)num_to_int(args[3]);
                int size = args.size()>4 ? (int)num_to_int(args[4]) : 1;
                drawTextCb(x,y,text,color,size);
                return Value{};
            };
            modules[imp.alias]=Module{true,bi,nullptr}; return;
        }
        if(imp.name=="Touch"){
            BuiltinModule bi;
            bi.fns["read"]= [this](vector<Value>)->Value{
                int x=0,y=0; bool pressed=false; Value v; v.type=Value::DICT; if(touchReadCb && touchReadCb(&x,&y,&pressed)){ v.dict["x"]=make_shared<Value>(make_int(x)); v.dict["y"]=make_shared<Value>(make_int(y)); v.dict["pressed"]=make_shared<Value>(make_bool(pressed)); } else { v.dict["x"]=make_shared<Value>(make_int(0)); v.dict["y"]=make_shared<Value>(make_int(0)); v.dict["pressed"]=make_shared<Value>(make_bool(false)); } return v; };
            modules[imp.alias]=Module{true,bi,nullptr}; return;
        }
        if(imp.name=="String"){
            BuiltinModule bi;
            bi.fns["len"]= [](vector<Value> args)->Value{ string s = Interpreter::to_string_value(args[0]); return Interpreter::make_int((long long)s.size()); };
            bi.fns["substr"]= [](vector<Value> args)->Value{ string s = Interpreter::to_string_value(args[0]); long long a = Interpreter::num_to_int(args[1]); long long b = Interpreter::num_to_int(args[2]); if(a<0) a=0; if(b<0) b=0; if((size_t)a > s.size()) a = (long long)s.size(); size_t len = (size_t)b; if(a+len > s.size()) len = s.size() - (size_t)a; return Interpreter::make_str(s.substr((size_t)a, len)); };
            bi.fns["splitLines"]= [](vector<Value> args)->Value{ string s = Interpreter::to_string_value(args[0]); Value out; out.type=Value::LIST; size_t start=0; while(true){ size_t pos = s.find('\n', start); if(pos==string::npos){ string part = s.substr(start); out.list.push_back(make_shared<Value>(Interpreter::make_str(part))); break; } string part = s.substr(start, pos-start); out.list.push_back(make_shared<Value>(Interpreter::make_str(part))); start = pos+1; } return out; };
            modules[imp.alias]=Module{true,bi,nullptr}; return;
        }
        if(imp.name=="List"){
            BuiltinModule bi;
            bi.fns["len"]= [](vector<Value> args)->Value{ const Value &lst = args[0]; if(lst.type==Value::LIST) return Interpreter::make_int((long long)lst.list.size()); return Interpreter::make_int(0); };
            
            bi.fns["append"]= [this](vector<Value> args)->Value{
                Value &lst = args[0];
                
                if(lst.type!=Value::LIST) throw runtime_error("1st argument must be type list");
                lst.list.push_back(make_shared<Value>(args[1]));
                return Value{};
            };

            bi.fns["append"]= [this](vector<Value> args)->Value{
                Value &lst = args[0];
                Value &lst2nd = args[0];

                if(lst.type!=Value::LIST || lst2nd.type!=Value::LIST) throw runtime_error("1st and 2nd arguments must be type list");
                for(auto &v : lst2nd.list) lst.list.push_back(v);
                return Value{};
            };

            bi.fns["remove"]= [this](vector<Value> args)->Value{
                Value &lst = args[0];
                Value &index = args[0];

                if(lst.type!=Value::LIST || index.type!=Value::INT) throw runtime_error("1st and 2nd arguments must be type list and int");
                int x = (int)num_to_int(index);
                if(x<0 || x>=(int)lst.list.size()) throw runtime_error("Index out of range");
                lst.list.erase(lst.list.begin()+x);
                return Value{};
            };
            
            modules[imp.alias]=Module{true,bi,nullptr}; return;
        }
        if(imp.name=="Config"){
            BuiltinModule bi;
            bi.fns["load"]= [this](vector<Value> args)->Value{ string p = to_string_value(args[0]); string t = reader? reader(p): string(); return parse_config_text(t); };
            bi.fns["parse"]= [this](vector<Value> args)->Value{ string t = to_string_value(args[0]); return parse_config_text(t); };
            modules[imp.alias]=Module{true,bi,nullptr}; return;
        }
        if(imp.name=="JSON"){
            BuiltinModule bi;
            bi.fns["load"]= [this](vector<Value> args)->Value{ string p = to_string_value(args[0]); string t = reader? reader(p): string(); JsonParser jp(t); return jp.parse(); };
            bi.fns["parse"]= [this](vector<Value> args)->Value{ string t = to_string_value(args[0]); JsonParser jp(t); return jp.parse(); };
            modules[imp.alias]=Module{true,bi,nullptr}; return;
        }
        throw runtime_error("Module not found");
    }
    Value call_function(const string& name, vector<Value> args){ auto it=functions.find(name); if(it==functions.end()) throw runtime_error("Undefined function"); auto &fn=it->second; auto env=make_shared<Env>(globals); for(size_t i=0;i<fn.params.size();++i){ Value v; if(i<args.size()) v=args[i]; env->define(fn.params[i].second, v); } try{ exec_block(fn.body, env); } catch(ReturnSignal &rs){ return rs.v; } return Value{}; }
    void exec_block(const vector<StmtPtr>& body, shared_ptr<Env> env){ for(auto &st: body){ exec_stmt(st, env); } }
    void exec_stmt(const StmtPtr& st, shared_ptr<Env> env){
        if(auto im = dynamic_pointer_cast<ImportStmtS>(st)){
            ImportDecl d = im->imp;
            if(!d.path.empty()){ load_module_from_path(d.path, d.alias); return; }
            if(!d.varName.empty()){ Value v = env->get(d.varName); string p = to_string_value(v); load_module_from_path(p, d.alias); return; }
            handle_import(d); return;
        }
        if(auto ts = dynamic_pointer_cast<TryStmt>(st)){
            try{ exec_block(ts->tryBody, make_shared<Env>(env)); }
            catch(...){ exec_block(ts->exceptBody, make_shared<Env>(env)); }
            return;
        }
        if(auto v = dynamic_pointer_cast<VarDeclStmt>(st)){ Value val=eval_expr(v->value, env); env->define(v->name, val); return; }
        if(auto e = dynamic_pointer_cast<ExprStmt>(st)){ eval_expr(e->expr, env); return; }
        if(auto w = dynamic_pointer_cast<WhileStmt>(st)){ while(truthy(eval_expr(w->cond, env))){ try{ exec_block(w->body, make_shared<Env>(env)); } catch(BreakSignal&){ break; } } return; }
        if(auto f = dynamic_pointer_cast<ForStmt>(st)){ auto local=make_shared<Env>(env); if(f->init) eval_expr(f->init, local); while(true){ if(f->cond && !truthy(eval_expr(f->cond, local))) break; try{ exec_block(f->body, make_shared<Env>(local)); } catch(BreakSignal&){ break; } if(f->post){ if(dynamic_pointer_cast<IdentExpr>(f->post)){ auto id=static_pointer_cast<IdentExpr>(f->post)->name; Value cur=local->get(id); long long nv=num_to_int(cur)+1; local->set(id, make_int(nv)); } else eval_expr(f->post, local); } } return; }
        if(auto i = dynamic_pointer_cast<IfStmt>(st)){ if(truthy(eval_expr(i->cond, env))){ exec_block(i->thenBody, make_shared<Env>(env)); return; } for(auto &el: i->elifs){ if(truthy(eval_expr(el.first, env))){ exec_block(el.second, make_shared<Env>(env)); return; } } if(!i->elseBody.empty()) exec_block(i->elseBody, make_shared<Env>(env)); return; }
        if(dynamic_pointer_cast<BreakStmt>(st)){ throw BreakSignal{}; }
        if(auto r = dynamic_pointer_cast<ReturnStmt>(st)){ Value v; if(r->value) v=eval_expr(r->value, env); throw ReturnSignal{v}; }
        throw runtime_error("Unknown statement");
    }
    Value eval_expr(const ExprPtr& e, shared_ptr<Env> env){ if(auto p=dynamic_pointer_cast<IntExpr>(e)) return make_int(p->v); if(auto p=dynamic_pointer_cast<FloatExpr>(e)) return make_float(p->v); if(auto p=dynamic_pointer_cast<StringExpr>(e)) return make_str(p->v); if(auto p=dynamic_pointer_cast<BoolExpr>(e)) return make_bool(p->v); if(auto p=dynamic_pointer_cast<IdentExpr>(e)) return env->get(p->name); if(auto p=dynamic_pointer_cast<UnaryExpr>(e)){ Value v=eval_expr(p->expr, env); if(p->op=="-"){ if(v.type==Value::FLOAT) return make_float(-v.d); if(v.type==Value::INT) return make_int(-v.i); throw runtime_error("Numeric expected"); } if(p->op=="+"){ return v; } if(p->op=="!"){ return make_bool(!truthy(v)); } } if(auto p=dynamic_pointer_cast<BinaryExpr>(e)){ Value lv=eval_expr(p->l, env); Value rv=eval_expr(p->r, env); string op=p->op; if(op=="+"){ if(lv.type==Value::STRING || rv.type==Value::STRING){ string a=to_string_value(lv); string b=to_string_value(rv); return make_str(a+b);} double a=num_to_double(lv), b=num_to_double(rv); return make_float(a+b);} if(op=="-"){ double a=num_to_double(lv), b=num_to_double(rv); return make_float(a-b);} if(op=="*"){ double a=num_to_double(lv), b=num_to_double(rv); return make_float(a*b);} if(op=="/"){ double a=num_to_double(lv), b=num_to_double(rv); return make_float(a/b);} if(op=="%"){ long long a=num_to_int(lv), b=num_to_int(rv); return make_int(a%b);} if(op=="=="){ return make_bool(equal_values(lv,rv));} if(op=="!="){ return make_bool(!equal_values(lv,rv));} if(op=="<"){ return make_bool(num_to_double(lv)<num_to_double(rv));} if(op==">"){ return make_bool(num_to_double(lv)>num_to_double(rv));} if(op=="<="){ return make_bool(num_to_double(lv)<=num_to_double(rv));} if(op==">="){ return make_bool(num_to_double(lv)>=num_to_double(rv));} if(op=="&&"){ return make_bool(truthy(lv) && truthy(rv));} if(op=="||"){ return make_bool(truthy(lv) || truthy(rv));} }
        if(auto p=dynamic_pointer_cast<AssignExpr>(e)){
            if(auto id=dynamic_pointer_cast<IdentExpr>(p->left)){
                string n=id->name; if(p->op=="="){ Value v=eval_expr(p->right, env); env->set(n,v); return v;} Value base=env->get(n); Value delta=eval_expr(p->right, env); if(p->op=="+="){ double a=num_to_double(base), b=num_to_double(delta); env->set(n, make_float(a+b)); return env->get(n);} if(p->op=="-="){ double a=num_to_double(base), b=num_to_double(delta); env->set(n, make_float(a-b)); return env->get(n);} if(p->op=="*="){ double a=num_to_double(base), b=num_to_double(delta); env->set(n, make_float(a*b)); return env->get(n);} if(p->op=="/="){ double a=num_to_double(base), b=num_to_double(delta); env->set(n, make_float(a/b)); return env->get(n);} throw runtime_error("Unsupported assignment");
            }
            if(auto idx=dynamic_pointer_cast<IndexExpr>(p->left)){
                if(auto id=dynamic_pointer_cast<IdentExpr>(idx->obj)){
                    string n=id->name; Value arr=env->get(n); Value pos=eval_expr(idx->index, env); Value val=eval_expr(p->right, env); if(arr.type==Value::LIST){ size_t i=(size_t)num_to_int(pos); if(i<arr.list.size()){ arr.list[i]=make_shared<Value>(val); env->set(n, arr); return val; } }
                }
                Value v; return v;
            }
            if(auto mem=dynamic_pointer_cast<MemberExpr>(p->left)){
                if(auto id=dynamic_pointer_cast<IdentExpr>(mem->obj)){
                    string n=id->name; Value obj=env->get(n); Value val=eval_expr(p->right, env); if(obj.type==Value::DICT){ obj.dict[mem->prop]=make_shared<Value>(val); env->set(n, obj); return val; }
                }
                Value v; return v;
            }
            Value v; return v;
        }
        if(auto p=dynamic_pointer_cast<UpdateExpr>(e)){ auto id=dynamic_pointer_cast<IdentExpr>(p->arg); if(!id) throw runtime_error("Update on non-ident"); string n=id->name; Value cur=env->get(n); long long nv=num_to_int(cur) + (p->op=="++"?1:-1); Value nw=make_int(nv); if(p->prefix){ env->set(n,nw); return nw; } else { env->set(n,nw); return cur; } }
        if(auto p=dynamic_pointer_cast<MemberExpr>(e)){ Value obj=eval_expr(p->obj, env); if(obj.type==Value::DICT){ auto it=obj.dict.find(p->prop); if(it!=obj.dict.end()) return *it->second; Value v; return v; } Value v; return v; }
        if(auto p=dynamic_pointer_cast<IndexExpr>(e)){ Value obj=eval_expr(p->obj, env); Value ix=eval_expr(p->index, env); if(obj.type==Value::LIST){ size_t i=(size_t)num_to_int(ix); if(i<obj.list.size()) return *obj.list[i]; } Value v; return v; }
        if(auto p=dynamic_pointer_cast<CallExpr>(e)){
            if(auto m = dynamic_pointer_cast<MemberExpr>(p->callee)){
                if(auto id = dynamic_pointer_cast<IdentExpr>(m->obj)){
                    string alias=id->name; auto it=modules.find(alias); if(it==modules.end()) throw runtime_error("Unknown module alias"); auto &mod=it->second; vector<Value> args; for(auto &a: p->args) args.push_back(eval_expr(a, env)); if(mod.builtin){ auto jt=mod.bi.fns.find(m->prop); if(jt==mod.bi.fns.end()) throw runtime_error("Unknown function"); return jt->second(args); } else { return mod.interp->call_function(m->prop, args); }
                }
                throw runtime_error("Unsupported call");
            }
            if(auto id = dynamic_pointer_cast<IdentExpr>(p->callee)){
                vector<Value> args; for(auto &a: p->args) args.push_back(eval_expr(a, env)); return call_function(id->name, args);
            }
            throw runtime_error("Unsupported call");
        }
        if(auto p=dynamic_pointer_cast<ListExpr>(e)){ Value v; v.type=Value::LIST; for(auto &it: p->items){ v.list.push_back(make_shared<Value>(eval_expr(it, env))); } return v; }
        if(auto p=dynamic_pointer_cast<DictExpr>(e)){ Value v; v.type=Value::DICT; for(auto &kv: p->items){ v.dict[kv.first]=make_shared<Value>(eval_expr(kv.second, env)); } return v; }
        throw runtime_error("Unknown expression");
    }
};

struct FxInterpreterHandle { shared_ptr<Interpreter> interp; };

static Program parse_program_text(const string& text){ Lexer lx(text); auto toks=lx.tokens(); Parser ps(toks); return ps.parse(); }

FxInterpreterHandle* fx_create(ReadFileFn readFile, PrintFn print){ auto h=new FxInterpreterHandle(); Program empty; h->interp=make_shared<Interpreter>(empty, readFile, print); return h; }
void fx_load(FxInterpreterHandle* h, const string& mainPath){ try{ string text = h->interp->reader ? h->interp->reader(mainPath) : string(); Program prog = parse_program_text(text); h->interp->prog=prog; h->interp->eval(); } catch(exception& e){ h->interp->printer ? h->interp->printer(string("ERROR: ")+e.what()+"\n") : void(); h->interp->globals->define("__error__", Interpreter::make_str(e.what())); } }
void fx_call_setup(FxInterpreterHandle* h){ try{ if(h->interp->functions.count("setup")) h->interp->call_function("setup",{}); } catch(exception& e){ h->interp->globals->define("__error__", Interpreter::make_str(e.what())); if(h->interp->printer) h->interp->printer(string("ERROR: ")+e.what()+"\n"); } }
void fx_call_loop(FxInterpreterHandle* h, float dt){ try{ if(h->interp->functions.count("loop")){ vector<Value> a{ Interpreter::make_float(dt)}; h->interp->call_function("loop", a); } } catch(exception& e){ h->interp->globals->define("__error__", Interpreter::make_str(e.what())); if(h->interp->printer) h->interp->printer(string("ERROR: ")+e.what()+"\n"); } }
void fx_set_display(FxInterpreterHandle* h, DrawRectFn drawRect, DrawTextFn drawText){ h->interp->drawRectCb=drawRect; h->interp->drawTextCb=drawText; }
void fx_set_touch(FxInterpreterHandle* h, TouchReadFn touchRead){ h->interp->touchReadCb=touchRead; }
const char* fx_last_error(FxInterpreterHandle* h){ try{ Value v = h->interp->globals->get("__error__"); string s = Interpreter::to_string_value(v); static string last; last = s; return last.c_str(); } catch(...){ return ""; } }
void fx_clear_error(FxInterpreterHandle* h){ try{ h->interp->globals->set("__error__", Value{}); } catch(...){ /* ignore */ } }
void fx_destroy(FxInterpreterHandle* h){ delete h; }

