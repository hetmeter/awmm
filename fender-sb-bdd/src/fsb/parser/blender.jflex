package fsb.parser;
import java_cup.runtime.Symbol;

%%
%class Lexer
%cup
%line
%column
%x COMMENT

%{

  private Symbol symbol(int sym) {
    return new Symbol(sym, yyline+1, yycolumn+1);
  }
  
  private Symbol symbol(int sym, Object val) {
    return new Symbol(sym, yyline+1, yycolumn+1, val);
  }
  
  private void error(String message) {
    System.out.println("Error at line "+(yyline+1)+", column "+(yycolumn+1)+" : "+message);
  }
%} 

Ident = [a-zA-Z] [a-zA-Z0-9_]*
IntLiteral = 0 | [1-9][0-9]*
new_line = \r|\n|\r\n;
white_space = {new_line} | [ \t\f]
comment_s = ("/*")
comment_f = ([*]+"/")
c_comment = {comment_s}([^*]|([*]+[^/*]))*{comment_f}
%%

{c_comment}	  { return  symbol(sym.C_COMMENT, new String(yytext())); }
"shared"	  { return symbol(sym.SHARED); }
"local"		  { return symbol(sym.LOCAL); }
"load"		  { return symbol(sym.LOAD); }
"store"		  { return symbol(sym.STORE); }
"put"		  { return symbol(sym.PUT); }	
"get"		  { return symbol(sym.GET); }
"rga"		  { return symbol(sym.RGA); }
"barrier"	  { return symbol(sym.BARRIER); }
"if"		  { return symbol(sym.IF); }
"goto"		  { return symbol(sym.GOTO); }
"nop"		  { return symbol(sym.NOP); }
"allocate"	  { return symbol(sym.ALLOCATE); }
"process"	  { return symbol(sym.PROCESS); }
"init"		  { return symbol(sym.INIT); }
"end_init"	  { return symbol(sym.ENDINIT); }
"true"		  { return symbol(sym.TRUE); }
"false"		  { return symbol(sym.FALSE); }
"cas"		  { return symbol(sym.CAS); }
"swap"		  { return symbol(sym.SWAP); }
"dec"		  { return symbol(sym.ATOMICDEC); }
"assert"	  { return symbol(sym.ASSERT); }
"final"		  { return symbol(sym.FINAL); }
"always"	  { return symbol(sym.ALWAYS); }
"begin_atomic" { return symbol(sym.BEGINATOMIC); }
"end_atomic"   { return symbol(sym.ENDATOMIC); }
"atomic"	  { return symbol(sym.ATOMIC); }
"non_atomic"  { return symbol(sym.NONATOMIC); }
"assume"      { return symbol(sym.ASSUME); }
"pc"		  { return symbol(sym.PC); }
"choose"		  { return symbol(sym.CHOOSE); }

{Ident}           { return symbol(sym.IDENT, yytext()); }
{IntLiteral}	  { return symbol(sym.NUMBER, new Integer(Integer.parseInt(yytext()))); }

";"		 { return symbol(sym.SEMI); }
"+"		 { return symbol(sym.PLUS); }
"-"		 { return symbol(sym.MINUS); }
"*"		 { return symbol(sym.MUL); }
"=="		 { return symbol(sym.EQUAL); }
"="		 { return symbol(sym.ASSIGN); }
"!="		 { return symbol(sym.NEQ); }
"<"		 { return symbol(sym.LESS); }
">"		 { return symbol(sym.GREATER); }
":"		 { return symbol(sym.COLON); }
","		 { return symbol(sym.COMMA); }
"["		 { return symbol(sym.LEFTBRACE); }
"]"		 { return symbol(sym.RIGHTBRACE); }
"("		 { return symbol(sym.LEFTPAREN); }
")"		 { return symbol(sym.RIGHTPAREN); }
"{"		 { return symbol(sym.LEFTCURLY); }
"}"		 { return symbol(sym.RIGHTCURLY); }
"&&"	 { return symbol(sym.AND); }
"||"	 { return symbol(sym.OR); }
"!"		 { return symbol(sym.NOT); }


{white_space}     { /* ignore */ }

.|\n              {  /* throw new Error("Illegal character <"+ yytext()+">");*/
		    error("Illegal character <"+ yytext()+">");
                  }