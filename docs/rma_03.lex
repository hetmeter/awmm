

/* Blocks */
programBlock						{{initializationBlock}} {{processDeclaration}*} {EOF!}
initializationBlock 				{BEGINIT_KEYWORD} {processInitialization*} {ENDINIT_KEYWORD}
processInitialization				{PROCESS_KEYWORD} {INT} {COLON} {variableInitializationStatement*}
variableInitializationStatement		{storeStatement} | {assignStatement}
processDeclaration					{PROCESS_KEYWORD} {INT} {COLON} {statement+}
statement							{label?} ( {storeStatement} |	{loadStatement} |	{ifElseBlock} |	{gotoStatement} |	{nopStatement} |	{assignStatement} |	{putStatement} |	{getStatement} |	{flushStatement} )
label								{INT} {COLON}
storeStatement						{STORE_KEYWORD} {ID} {EQUALS} {integerTerm} {SEMICOLON}
loadStatement						{LOAD_KEYWORD} {ID} {EQUALS} {ID} {SEMICOLON}
ifElseBlock							{IF_KEYWORD} {LEFTPARENTHESIS} {booleanTerm} {RIGHTPARENTHESIS} {statement*} ( {ELSE_KEYWORD} {statement*} {)?}
gotoStatement						{GOTO_KEYWORD} {INT} {SEMICOLON}
nopStatement						{NOP_KEYWORD} {SEMICOLON}
assignStatement						{ID} {EXTENDED_ASSIGNMENT} {integerTerm} {SEMICOLON}
putStatement						{PUT_KEYWORD} {LEFTPARENTHESIS} {ATOMICITY} {COMMA} {ATOMICITY} {RIGHTPARENTHESIS} {LEFTPARENTHESIS} {ID} {COMMA} {INT} {COMMA} {ID} {RIGHTPARENTHESIS} {SEMICOLON}
getStatement						{GET_KEYWORD} {LEFTPARENTHESIS} {ATOMICITY} {COMMA} {ATOMICITY} {RIGHTPARENTHESIS} {LEFTPARENTHESIS} {ID} {COMMA} {INT} {RIGHTPARENTHESIS} {SEMICOLON}
flushStatement						{FLUSH_KEYWORD} {LEFTPARENTHESIS} {INT} {RIGHTPARENTHESIS} {SEMICOLON}
integerTerm							( {INT} |	{ID} |	{LEFTPARENTHESIS} {integerTerm} {RIGHTPARENTHESIS} |	{integerTerm} {binaryIntegerOperator} {integerTerm} )
binaryIntegerOperator				( {PLUS} |	{MINUS} |	{ASTERISK} |	{SLASH} |	{MOD} )
booleanTerm							( {BOOL} |	{LEFTPARENTHESIS} {booleanTerm} {RIGHTPARENTHESIS} |	{unaryBooleanOperator} {booleanTerm} |	{booleanTerm} {binaryBooleanOperator} {booleanTerm} |	{integerTerm} {binaryIntegerComparisonOperator} {integerTerm} )
unaryBooleanOperator				( {NOT} )
binaryBooleanOperator				( {OR} |	{AND} )
binaryIntegerComparisonOperator		( {EQUALS} |	{GREATER} |	{LESS} |	{GREATER_EQUALS} |	{LESS_EQUALS} |	{NOT_EQUALS} )