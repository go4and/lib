grammar calc_expr;

options {
    output=AST;
    language=C;
}

start: ( statement 
{ 
    pANTLR3_STRING str = $statement.tree->toStringTree($statement.tree);
    pANTLR3_STRING ustr = str->toUTF8(str);
    const char * result = (const char*)ustr->chars;
    printf("tree: \%s\n", result);
} )+ EOF ;

statement: expr NEWLINE -> expr
         | NEWLINE      ->
         ;

expr: oterm (OR^ oterm)* ;

oterm: aterm (AND^ aterm)* ;

aterm: boterm ('|'^ boterm)* ;

boterm: bxterm ('^'^ bxterm)* ;

bxterm: baterm ('&'^ baterm)* ;

baterm: eterm (('=='^|'~='^) eterm)* ;

eterm: cterm (('>='^|'<='^|'<'^|'>'^) cterm)* ;

cterm: sterm ('..'^ sterm)* ;

sterm: bsterm (('<<'^|'>>'^) bsterm)* ;

bsterm: term (('+'^|'-'^) term)* ;

term: factor (('*'^|'/'^|'%'^) factor)* ;

factor: NUMBER
      | HEX_NUMBER 
      | NOT^ factor
      | QSTRING
      | ASTRING
      | '('! expr ')'!
      | '['! expr ']'!
      | invokation
      | '-'^ expr 
      | '+'! expr
      | '#' expr
      ;

invokation: IDENTIFIER (('(' paramList ')')|('[' paramList ']'))? ;
paramList: expr (',' expr)* ;

NUMBER: '0'..'9'+ ;
HEX_NUMBER: '0x' ('0'..'9'|'a'..'z'|'A'..'Z')+ ;
NOT: 'not';
AND: 'and';
OR: 'or';
QSTRING: '"' QSTRING_GUTS '"';
fragment QSTRING_GUTS : ( '\\'. | ~('\\'|'"') )* ;
ASTRING: '\'' ASTRING_GUTS '\'';
fragment ASTRING_GUTS : ( '\\'. | ~('\\'|'\'') )* ;
IDENTIFIER: LETTER (LETTER|IDDIGIT)* ;

fragment
LETTER
    :  '\u0024' |
       '\u0041'..'\u005a' |
       '\u005f' |
       '\u0061'..'\u007a' |
       '\u00c0'..'\u00d6' |
       '\u00d8'..'\u00f6' |
       '\u00f8'..'\u00ff' |
       '\u0100'..'\u1fff' |
       '\u3040'..'\u318f' |
       '\u3300'..'\u337f' |
       '\u3400'..'\u3d2d' |
       '\u4e00'..'\u9fff' |
       '\uf900'..'\ufaff'
    ;

fragment
IDDIGIT
    :  '\u0030'..'\u0039' |
       '\u0660'..'\u0669' |
       '\u06f0'..'\u06f9' |
       '\u0966'..'\u096f' |
       '\u09e6'..'\u09ef' |
       '\u0a66'..'\u0a6f' |
       '\u0ae6'..'\u0aef' |
       '\u0b66'..'\u0b6f' |
       '\u0be7'..'\u0bef' |
       '\u0c66'..'\u0c6f' |
       '\u0ce6'..'\u0cef' |
       '\u0d66'..'\u0d6f' |
       '\u0e50'..'\u0e59' |
       '\u0ed0'..'\u0ed9' |
       '\u1040'..'\u1049'
   ;

WHITESPACE: (' '|'\t')+ { $channel = HIDDEN; } ;
NEWLINE: '\r'? '\n' ;

