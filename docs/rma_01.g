grammar rma_01;

options { language = Cpp; }

programBlock
	:	initializationBlock
		processDeclaration*
		EOF!
	;

initializationBlock 
	:	BEGINIT_KEYWORD
		processInitialization*
		ENDINIT_KEYWORD
	;
	
processInitialization
	:	PROCESS_KEYWORD INT COLON
		variableInitializationStatement*
	;
	
variableInitializationStatement
	:	( storeStatement
	|	assignStatement )
	;
	
processDeclaration
	:	PROCESS_KEYWORD INT COLON
		statement+
	;
	
statement
	:	label?
		( storeStatement
	|	loadStatement
	|	ifElseBlock
	|	gotoStatement
	|	nopStatement
	|	assignStatement
	|	putStatement
	|	getStatement
	|	flushStatement )
	;
	
label
	:	INT COLON
	;
	
storeStatement
	:	STORE_KEYWORD ID EQUALS integerTerm SEMICOLON
	;
	
loadStatement
	:	LOAD_KEYWORD ID EQUALS ID SEMICOLON
	;
	
ifElseBlock
	:	IF_KEYWORD LEFTPARENTHESIS booleanTerm RIGHTPARENTHESIS
		statement*
		( ELSE_KEYWORD
		statement* )?
	;
	
gotoStatement
	:	GOTO_KEYWORD INT SEMICOLON
	;
	
nopStatement
	:	NOP_KEYWORD SEMICOLON
	;

assignStatement
	:	ID EXTENDED_ASSIGNMENT integerTerm SEMICOLON
	;
	
putStatement
	:	PUT_KEYWORD LEFTPARENTHESIS ATOMICITY COMMA ATOMICITY RIGHTPARENTHESIS LEFTPARENTHESIS ID COMMA INT COMMA ID RIGHTPARENTHESIS SEMICOLON
	;
	
getStatement
	:	GET_KEYWORD LEFTPARENTHESIS ATOMICITY COMMA ATOMICITY RIGHTPARENTHESIS LEFTPARENTHESIS ID COMMA INT RIGHTPARENTHESIS SEMICOLON
	;
	
flushStatement
	:	FLUSH_KEYWORD LEFTPARENTHESIS INT RIGHTPARENTHESIS SEMICOLON
	;
		
integerTerm
	:	( INT
	|	ID
	|	LEFTPARENTHESIS integerTerm RIGHTPARENTHESIS
	|	integerTerm binaryIntegerOperator integerTerm )
	;

binaryIntegerOperator
	:	( PLUS
	|	MINUS
	|	ASTERISK
	|	SLASH
	|	MOD )
	;
		
booleanTerm
	:	( BOOL
	|	LEFTPARENTHESIS booleanTerm RIGHTPARENTHESIS
	|	unaryBooleanOperator booleanTerm
	|	booleanTerm binaryBooleanOperator booleanTerm
	|	integerTerm binaryIntegerComparisonOperator integerTerm )
	;
	
unaryBooleanOperator
	:	( NOT )
	;
		
binaryBooleanOperator
	:	( OR
	|	AND )
	;
		
binaryIntegerComparisonOperator
	:	( EQUALS
	|	GREATER
	|	LESS
	|	GREATER_EQUALS
	|	LESS_EQUALS
	|	NOT_EQUALS )
	;
	
ATOMICITY
	:	( 'a' | 'n' )
	;
	
EXTENDED_ASSIGNMENT
	:	( EQUALS
	|	'=(' ATOMICITY ')' )
	;

BOOL
	:	( 'true' | 'false' )
	;

ID
	:	('a'..'z'|'A'..'Z'|'_') ('a'..'z'|'A'..'Z'|'0'..'9'|'_')*
	;

INT
	:'0'..'9'+
	;

COMMENT
    :   '//' ~('\n'|'\r')* '\r'? '\n' {$channel=HIDDEN;}
    |   '/*' ( options {greedy=false;} : . )* '*/' {$channel=HIDDEN;}
    ;

WS  :   ( ' '
        | '\t'
        | '\r'
        | '\n'
        ) {$channel=HIDDEN;}
    ;

BEGINIT_KEYWORD
	:	'beginit'
	;
	
ENDINIT_KEYWORD
	:	'endinit'
	;
	
PROCESS_KEYWORD
	:	'process'
	;
		
STORE_KEYWORD
	:	'store'
	;
	
LOAD_KEYWORD
	:	'load'
	;
	
IF_KEYWORD
	:	'if'
	;
		
ELSE_KEYWORD
	:	'else'
	;
		
GOTO_KEYWORD
	:	'goto'
	;
		
NOP_KEYWORD
	:	'nop'
	;
		
PUT_KEYWORD
	:	'put'
	;
		
GET_KEYWORD
	:	'get'
	;
		
FLUSH_KEYWORD
	:	'flush'
	;
	
LEFTPARENTHESIS
	:	'('
	;
	
RIGHTPARENTHESIS
	:	')'
	;
	
SEMICOLON
	:	';'
	;

EQUALS	:	'='
	;

NOT	:	'!'
	;
	
NOT_EQUALS
	:	'!='
	;

LESS	:	'<'
	;

LESS_EQUALS
	:	'<='
	;
		
GREATER	:	'>'
	;
	
GREATER_EQUALS
	:	'>='
	;
	
AND	:	'&'
	;
	
OR	:	'|'
	;
	
COLON	:	':'
	;
	
PLUS	:	'+'
	;
	
MINUS	:	'-'
	;
	
ASTERISK:	'*'
	;
	
SLASH	:	'/'
	;
	
MOD	:	'%'
	;
	
COMMA	:	','
	;