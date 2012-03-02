grammar calc_expr;

options {
    output=AST;
    language=C;
}

start: expr EOF ;

expr: oterm (T_OR^ oterm)* ;
oterm: aterm (T_AND^ aterm)* ;
aterm: boterm (T_BITOR^ boterm)* ;
boterm: bxterm (T_BITXOR^ bxterm)* ;
bxterm: baterm (T_BITAND^ baterm)* ;
baterm: eterm ((T_EQ^|T_NE^) eterm)* ;
eterm: cterm ((T_GE^|T_LE^|T_LESS^|T_GREATER^) cterm)* ;
cterm: sterm (T_CONCAT^ sterm)* ;
sterm: bsterm ((T_SHIFT_LEFT^|T_SHIFT_RIGHT^) bsterm)* ;
bsterm: term ((T_PLUS^|T_MINUS^) term)* ;
term: factor ((T_MUL^|T_DIV^|T_MOD^) factor)* ;

factor: T_NUMBER
      | T_HEX_NUMBER 
      | T_NOT^ factor
      | T_QSTRING
      | T_ASTRING
      | '('! expr ')'!
      | '['! expr ']'!
      | invokation
      | T_MINUS^ factor 
      | '+'! factor
      | T_HASH^ factor
      | T_CB_EXPR^
      ;

invokation: T_IDENTIFIER^ (('('! paramList ')'!)|('['! paramList ']'!))? ;
paramList: expr (','! expr)* ;

T_NUMBER: '0'..'9'+ ;
T_HEX_NUMBER: '0x' ('0'..'9'|'a'..'z'|'A'..'Z')+ ;
T_NOT: 'not';
T_AND: 'and';
T_OR: 'or';
T_QSTRING: '"' QSTRING_GUTS '"';
fragment QSTRING_GUTS : ( '\\'. | ~('\\'|'"') )* ;
T_ASTRING: '\'' ASTRING_GUTS '\'';
fragment ASTRING_GUTS : ( '\\'. | ~('\\'|'\'') )* ;
T_IDENTIFIER: LETTER (LETTER|IDDIGIT)* ;
T_PLUS: '+' ;
T_MINUS: '-' ;
T_MUL: '*' ;
T_DIV: '/' ;
T_MOD: '%' ;
T_SHIFT_LEFT: '<<' ;
T_SHIFT_RIGHT: '>>' ;
T_BITOR: '|' ;
T_BITAND: '&' ;
T_BITXOR: '^' ;
T_EQ: '==' ;
T_NE: '~=' ;
T_GE: '>=' ;
T_LE: '<=' ;
T_LESS: '<' ;
T_GREATER: '>' ;
T_CONCAT: '..' ;
T_HASH: '#' ;
T_CB_EXPR: '{' CB_EXPR_GUTS '}' ;
fragment CB_EXPR_GUTS: ( T_ASTRING | T_QSTRING | (~('{'|'}'|'"'|'\'')) )* ;

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

T_WHITESPACE: (' '|'\t')+ { $channel = HIDDEN; } ;
T_NEWLINE: '\r'? '\n' ;

