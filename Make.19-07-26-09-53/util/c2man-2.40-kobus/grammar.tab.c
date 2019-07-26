
/*  A Bison parser, made from grammar.y
    by GNU Bison version 1.28  */

#define YYBISON 1  /* Identify Bison output.  */

#define	T_IDENTIFIER	257
#define	T_TYPEDEF_NAME	258
#define	T_AUTO	259
#define	T_EXTERN	260
#define	T_REGISTER	261
#define	T_STATIC	262
#define	T_TYPEDEF	263
#define	T_INLINE	264
#define	T_CHAR	265
#define	T_DOUBLE	266
#define	T_FLOAT	267
#define	T_INT	268
#define	T_VOID	269
#define	T_LONG	270
#define	T_SHORT	271
#define	T_SIGNED	272
#define	T_UNSIGNED	273
#define	T_ENUM	274
#define	T_STRUCT	275
#define	T_UNION	276
#define	T_CONST	277
#define	T_VOLATILE	278
#define	T_CDECL	279
#define	T_FAR	280
#define	T_HUGE	281
#define	T_INTERRUPT	282
#define	T_NEAR	283
#define	T_PASCAL	284
#define	T_BRACES	285
#define	T_BRACKETS	286
#define	T_ELLIPSIS	287
#define	T_INITIALIZER	288
#define	T_STRING_LITERAL	289
#define	T_COMMENT	290
#define	T_EOLCOMMENT	291
#define	T_BASEFILE	292

#line 69 "grammar.y"

#include "c2man.h"
#include "semantic.h"
#include "strconcat.h"
#include "strappend.h"
#include "manpage.h"
#include "enum.h"

#ifdef I_STDARG
#include <stdarg.h>
#endif
#ifdef I_VARARGS
#include <varargs.h>
#endif

int yylex();

#define YYMAXDEPTH 150

/* where are we up to scanning through an enum? */
static enum { NOENUM, KEYWORD, BRACES } enum_state = NOENUM;

/* Pointer to parameter list for the current function definition. */
static ParameterList *func_params;

/* Table of typedef names */
SymbolTable *typedef_names;

boolean first_comment;	/* are we still looking for the first comment? */
static char *body_comment = NULL;/* last comment found at start of func body */
#ifndef YYSTYPE
#define YYSTYPE int
#endif
#include <stdio.h>

#ifndef __cplusplus
#ifndef __STDC__
#define const
#endif
#endif



#define	YYFINAL		184
#define	YYFLAG		-32768
#define	YYNTBASE	46

#define YYTRANSLATE(x) ((unsigned)(x) <= 292 ? yytranslate[x] : 84)

static const char yytranslate[] = {     0,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,    43,
    44,    45,     2,    40,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,    39,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,    41,     2,    42,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     1,     3,     4,     5,     6,
     7,     8,     9,    10,    11,    12,    13,    14,    15,    16,
    17,    18,    19,    20,    21,    22,    23,    24,    25,    26,
    27,    28,    29,    30,    31,    32,    33,    34,    35,    36,
    37,    38
};

#if YYDEBUG != 0
static const short yyprhs[] = {     0,
     0,     1,     3,     5,     8,    11,    15,    18,    22,    26,
    31,    33,    35,    37,    39,    42,    46,    50,    51,    58,
    59,    65,    68,    72,    77,    79,    84,    87,    88,    90,
    93,    97,   102,   104,   107,   109,   111,   113,   115,   117,
   119,   121,   123,   125,   127,   129,   131,   133,   135,   137,
   139,   141,   143,   145,   147,   149,   151,   153,   155,   157,
   159,   161,   163,   167,   170,   173,   175,   177,   179,   184,
   187,   189,   192,   199,   205,   208,   210,   215,   218,   222,
   224,   226,   228,   231,   234,   237,   239,   241,   245,   248,
   253,   258,   261,   265,   266,   269,   272,   280,   282,   287,
   292,   297,   301,   302,   305,   307,   312,   316,   318,   321,
   323,   327,   330,   332,   337,   341,   345,   348,   349,   351,
   352
};

static const short yyrhs[] = {    -1,
    47,     0,    48,     0,    47,    48,     0,    53,    83,     0,
    36,    53,    83,     0,    50,    83,     0,    36,    50,    83,
     0,    50,    39,    83,     0,    36,    50,    39,    83,     0,
    49,     0,    36,     0,    37,     0,    38,     0,     1,    39,
     0,     6,    35,    31,     0,     6,    35,    53,     0,     0,
    57,    70,    83,    51,    55,    31,     0,     0,    70,    83,
    52,    55,    31,     0,    57,    39,     0,    57,    64,    39,
     0,     9,    57,    54,    39,     0,    70,     0,    54,    40,
    83,    70,     0,    54,    83,     0,     0,    56,     0,    56,
    33,     0,    82,    53,    83,     0,    56,    82,    53,    83,
     0,    58,     0,    57,    58,     0,    59,     0,    60,     0,
    61,     0,     5,     0,     6,     0,     7,     0,     8,     0,
    10,     0,    11,     0,    12,     0,    13,     0,    14,     0,
    16,     0,    17,     0,    18,     0,    19,     0,    15,     0,
    62,     0,    66,     0,     4,     0,    23,     0,    24,     0,
    25,     0,    28,     0,    26,     0,    27,     0,    29,     0,
    30,     0,    63,    69,    31,     0,    63,    31,     0,    63,
    69,     0,    21,     0,    22,     0,    65,     0,    64,    40,
    83,    65,     0,    64,    83,     0,    70,     0,    70,    34,
     0,    20,    69,    41,    83,    67,    42,     0,    20,    41,
    83,    67,    42,     0,    20,    69,     0,    68,     0,    67,
    40,    83,    68,     0,    67,    83,     0,    67,    40,    83,
     0,    79,     0,     3,     0,     4,     0,    69,    36,     0,
    69,    37,     0,    72,    71,     0,    71,     0,     3,     0,
    43,    70,    44,     0,    71,    32,     0,    71,    43,    74,
    44,     0,    71,    43,    77,    44,     0,    45,    73,     0,
    45,    73,    72,     0,     0,    73,    61,     0,    75,    83,
     0,    75,    40,    83,    82,    33,    82,    83,     0,    76,
     0,    75,    40,    83,    76,     0,    82,    57,    70,    82,
     0,    82,    57,    80,    82,     0,    82,    57,    82,     0,
     0,    78,    83,     0,    79,     0,    78,    40,    83,    79,
     0,    82,     3,    82,     0,    72,     0,    72,    81,     0,
    81,     0,    43,    80,    44,     0,    81,    32,     0,    32,
     0,    81,    43,    74,    44,     0,    81,    43,    44,     0,
    43,    74,    44,     0,    43,    44,     0,     0,    36,     0,
     0,    37,     0
};

#endif

#if YYDEBUG != 0
static const short yyrline[] = { 0,
   103,   104,   108,   109,   113,   117,   121,   132,   148,   159,
   175,   176,   185,   189,   193,   200,   207,   214,   223,   231,
   240,   253,   258,   263,   272,   276,   281,   289,   290,   291,
   295,   299,   306,   307,   314,   315,   316,   320,   324,   328,
   332,   336,   343,   347,   351,   355,   359,   363,   367,   371,
   375,   379,   380,   381,   392,   396,   400,   404,   408,   412,
   416,   420,   427,   432,   436,   444,   448,   455,   459,   464,
   472,   473,   477,   486,   492,   502,   506,   512,   517,   525,
   532,   533,   534,   539,   547,   556,   560,   564,   571,   577,
   585,   596,   601,   611,   615,   625,   630,   657,   661,   669,
   673,   677,   684,   688,   696,   701,   709,   717,   721,   730,
   734,   741,   747,   751,   759,   766,   776,   788,   792,   796,
   800
};
#endif


#if YYDEBUG != 0 || defined (YYERROR_VERBOSE)

static const char * const yytname[] = {   "$","error","$undefined.","T_IDENTIFIER",
"T_TYPEDEF_NAME","T_AUTO","T_EXTERN","T_REGISTER","T_STATIC","T_TYPEDEF","T_INLINE",
"T_CHAR","T_DOUBLE","T_FLOAT","T_INT","T_VOID","T_LONG","T_SHORT","T_SIGNED",
"T_UNSIGNED","T_ENUM","T_STRUCT","T_UNION","T_CONST","T_VOLATILE","T_CDECL",
"T_FAR","T_HUGE","T_INTERRUPT","T_NEAR","T_PASCAL","T_BRACES","T_BRACKETS","T_ELLIPSIS",
"T_INITIALIZER","T_STRING_LITERAL","T_COMMENT","T_EOLCOMMENT","T_BASEFILE","';'",
"','","'{'","'}'","'('","')'","'*'","program","translation_unit","external_declaration",
"linkage_specification","function_definition","@1","@2","declaration","declarator_list",
"opt_declaration_list","declaration_list","declaration_specifiers","declaration_specifier",
"storage_class","type_specifier","type_qualifier","struct_or_union_specifier",
"struct_or_union","init_declarator_list","init_declarator","enum_specifier",
"enumerator_list","enumerator","any_id","declarator","direct_declarator","pointer",
"type_qualifier_list","parameter_type_list","parameter_list","parameter_declaration",
"opt_identifier_list","identifier_list","identifier","abstract_declarator","direct_abstract_declarator",
"opt_comment","opt_eolcomment", NULL
};
#endif

static const short yyr1[] = {     0,
    46,    46,    47,    47,    48,    48,    48,    48,    48,    48,
    48,    48,    48,    48,    48,    49,    49,    51,    50,    52,
    50,    53,    53,    53,    54,    54,    54,    55,    55,    55,
    56,    56,    57,    57,    58,    58,    58,    59,    59,    59,
    59,    59,    60,    60,    60,    60,    60,    60,    60,    60,
    60,    60,    60,    60,    61,    61,    61,    61,    61,    61,
    61,    61,    62,    62,    62,    63,    63,    64,    64,    64,
    65,    65,    66,    66,    66,    67,    67,    67,    67,    68,
    69,    69,    69,    69,    70,    70,    71,    71,    71,    71,
    71,    72,    72,    73,    73,    74,    74,    75,    75,    76,
    76,    76,    77,    77,    78,    78,    79,    80,    80,    80,
    81,    81,    81,    81,    81,    81,    81,    82,    82,    83,
    83
};

static const short yyr2[] = {     0,
     0,     1,     1,     2,     2,     3,     2,     3,     3,     4,
     1,     1,     1,     1,     2,     3,     3,     0,     6,     0,
     5,     2,     3,     4,     1,     4,     2,     0,     1,     2,
     3,     4,     1,     2,     1,     1,     1,     1,     1,     1,
     1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
     1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
     1,     1,     3,     2,     2,     1,     1,     1,     4,     2,
     1,     2,     6,     5,     2,     1,     4,     2,     3,     1,
     1,     1,     2,     2,     2,     1,     1,     3,     2,     4,
     4,     2,     3,     0,     2,     2,     7,     1,     4,     4,
     4,     3,     0,     2,     1,     4,     3,     1,     2,     1,
     3,     2,     1,     4,     3,     3,     2,     0,     1,     0,
     1
};

static const short yydefact[] = {     0,
     0,    87,    54,    38,    39,    40,    41,     0,    42,    43,
    44,    45,    46,    51,    47,    48,    49,    50,     0,    66,
    67,    55,    56,    57,    59,    60,    58,    61,    62,    12,
    13,    14,     0,    94,     0,     3,    11,   120,   120,     0,
    33,    35,    36,    37,    52,     0,    53,   120,    86,     0,
    15,     0,    39,     0,    81,    82,   120,    75,   120,   120,
     0,    92,     4,   121,   120,     7,     5,    22,    34,     0,
    68,   120,    64,    65,    20,    89,   118,    85,    16,    17,
     0,     0,    25,   118,    83,    84,   120,   120,     8,     6,
    88,    95,    93,     9,    23,   120,    70,    72,    18,    63,
   118,   119,     0,   120,    98,     0,   120,   105,     0,    71,
    24,   120,    27,     0,    76,    80,     0,   118,    10,     0,
   118,     0,   118,     0,    90,   120,    96,    91,   120,   104,
   118,   118,     0,   120,    74,    78,     0,    69,     0,    21,
    30,     0,   120,   118,   118,   107,   113,   118,   118,   108,
   118,   110,   102,    26,    79,    73,    19,   120,    31,    99,
     0,   106,   117,     0,     0,     0,   100,   109,   101,   112,
   118,    77,    32,   118,   116,   111,   115,     0,   120,   114,
    97,     0,     0,     0
};

static const short yydefgoto[] = {   182,
    35,    36,    37,    38,   121,   101,    39,    82,   122,   123,
    40,    41,    42,    43,    44,    45,    46,    70,    71,    47,
   114,   115,    58,    48,    49,    50,    62,   103,   104,   105,
   106,   107,   116,   151,   152,   117,   136
};

static const short yypact[] = {   169,
   -12,-32768,-32768,-32768,    -4,-32768,-32768,   542,-32768,-32768,
-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,     2,-32768,
-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,   301,
-32768,-32768,    15,-32768,   215,-32768,-32768,    57,     3,   344,
-32768,-32768,-32768,-32768,-32768,    33,-32768,     3,   -13,     9,
-32768,   459,-32768,   387,-32768,-32768,     3,    42,    76,     3,
    17,   550,-32768,-32768,     3,-32768,-32768,-32768,-32768,    87,
-32768,    29,-32768,    40,-32768,-32768,   -19,   -13,-32768,-32768,
   344,    93,-32768,     5,-32768,-32768,     3,     3,-32768,-32768,
-32768,-32768,-32768,-32768,-32768,     3,-32768,-32768,-32768,-32768,
    74,-32768,    36,    53,-32768,    37,    98,-32768,   488,    13,
-32768,     3,-32768,    45,-32768,-32768,    62,     5,-32768,    15,
    74,    71,    55,   515,-32768,     3,-32768,-32768,     3,-32768,
     5,   258,    15,     3,-32768,-32768,    72,-32768,    80,-32768,
-32768,   515,     3,     5,     5,-32768,-32768,    10,     5,     6,
     5,    30,-32768,-32768,    21,-32768,-32768,     3,-32768,-32768,
   429,-32768,-32768,    73,    75,   542,-32768,    30,-32768,-32768,
   -10,-32768,-32768,     5,-32768,-32768,-32768,    81,     3,-32768,
-32768,   131,   136,-32768
};

static const short yypgoto[] = {-32768,
-32768,   102,-32768,   109,-32768,-32768,   -20,-32768,    19,-32768,
    -8,   -33,-32768,-32768,    82,-32768,-32768,-32768,    23,-32768,
    31,    -5,   101,   -25,   -47,   -48,-32768,  -132,-32768,     7,
-32768,-32768,   -73,     4,    11,    -3,   -37
};


#define	YYLAST		595


static const short yytable[] = {    54,
    66,    67,    78,   108,    55,    56,    69,    61,     2,    60,
    75,     2,     2,    93,    72,   164,   102,     2,    76,    84,
    69,    89,    90,  -118,  -103,   102,    51,    94,    83,    77,
    52,    80,    97,   177,    99,    55,    56,   147,   178,    64,
   102,   147,    57,    81,   113,   102,    98,    69,   148,   118,
   119,    33,   148,   163,    34,   110,   102,    33,   120,    34,
    91,   170,    98,    73,   131,    64,   127,   -71,   -71,   130,
   100,   162,   171,   109,   133,    85,    86,    85,    86,   125,
   128,    64,    87,   150,   134,   -29,   135,   141,   144,    64,
   102,   145,   126,    64,   110,    65,   155,   124,    69,   150,
   132,   140,    78,   143,   -28,   159,   149,   154,    64,   102,
   157,   134,    64,   156,    88,    81,   175,   124,   176,   142,
   173,   158,    61,    64,   180,    95,    96,   146,   153,    64,
   183,   111,   112,    81,    64,   184,    63,   129,    59,   139,
   161,   181,   138,    92,   166,   167,    74,   169,   137,   172,
   160,   165,   132,     0,     0,     0,     0,   132,     0,     0,
   168,     0,     0,     0,     0,     0,     0,   166,    -1,     1,
   179,     2,     3,     4,     5,     6,     7,     8,     9,    10,
    11,    12,    13,    14,    15,    16,    17,    18,    19,    20,
    21,    22,    23,    24,    25,    26,    27,    28,    29,     0,
     0,     0,     0,     0,    30,    31,    32,     0,     0,     0,
     0,    33,     0,    34,    -2,     1,     0,     2,     3,     4,
     5,     6,     7,     8,     9,    10,    11,    12,    13,    14,
    15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
    25,    26,    27,    28,    29,     0,     0,     0,     0,     0,
    30,    31,    32,     0,     0,     0,     0,    33,     0,    34,
     2,     3,     4,    53,     6,     7,     0,     9,    10,    11,
    12,    13,    14,    15,    16,    17,    18,    19,    20,    21,
    22,    23,    24,    25,    26,    27,    28,    29,     0,   147,
     0,     0,     0,   102,     0,     0,     0,     0,     0,     0,
   148,     0,    34,     2,     3,     4,    53,     6,     7,     8,
     9,    10,    11,    12,    13,    14,    15,    16,    17,    18,
    19,    20,    21,    22,    23,    24,    25,    26,    27,    28,
    29,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     0,     0,     0,    33,     0,    34,     2,     3,     4,    53,
     6,     7,     0,     9,    10,    11,    12,    13,    14,    15,
    16,    17,    18,    19,    20,    21,    22,    23,    24,    25,
    26,    27,    28,    29,     0,     0,     0,     0,     0,     0,
     0,     0,    68,     0,     0,     0,    33,     0,    34,     2,
     3,     4,    53,     6,     7,     0,     9,    10,    11,    12,
    13,    14,    15,    16,    17,    18,    19,    20,    21,    22,
    23,    24,    25,    26,    27,    28,    29,     0,     0,     0,
     0,     0,     0,     0,     0,     0,     0,     0,     0,    33,
     0,    34,     3,     4,    53,     6,     7,     0,     9,    10,
    11,    12,    13,    14,    15,    16,    17,    18,    19,    20,
    21,    22,    23,    24,    25,    26,    27,    28,    29,     0,
     0,   174,     3,     4,    53,     6,     7,     8,     9,    10,
    11,    12,    13,    14,    15,    16,    17,    18,    19,    20,
    21,    22,    23,    24,    25,    26,    27,    28,    29,    79,
   131,     3,     4,    53,     6,     7,     0,     9,    10,    11,
    12,    13,    14,    15,    16,    17,    18,    19,    20,    21,
    22,    23,    24,    25,    26,    27,    28,    29,     3,     4,
    53,     6,     7,     8,     9,    10,    11,    12,    13,    14,
    15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
    25,    26,    27,    28,    29,     3,     4,    53,     6,     7,
     0,     9,    10,    11,    12,    13,    14,    15,    16,    17,
    18,    19,    20,    21,    22,    23,    24,    25,    26,    27,
    28,    29,    22,    23,    24,    25,    26,    27,    28,    29,
     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     0,     0,     0,     0,    34
};

static const short yycheck[] = {     8,
    38,    39,    50,    77,     3,     4,    40,    33,     3,    30,
    48,     3,     3,    62,    40,   148,    36,     3,    32,    57,
    54,    59,    60,     3,    44,    36,    39,    65,    54,    43,
    35,    52,    70,    44,    72,     3,     4,    32,   171,    37,
    36,    32,    41,    52,    82,    36,    34,    81,    43,    87,
    88,    43,    43,    44,    45,    81,    36,    43,    96,    45,
    44,    32,    34,    31,     3,    37,   104,    39,    40,   107,
    31,   145,    43,    77,   112,    36,    37,    36,    37,    44,
    44,    37,    41,   132,    40,    31,    42,    33,   126,    37,
    36,   129,    40,    37,   120,    39,   134,   101,   132,   148,
   109,    31,   150,   124,    31,   143,   132,   133,    37,    36,
    31,    40,    37,    42,    39,   124,    44,   121,    44,   123,
   158,   142,   148,    37,    44,    39,    40,   131,   132,    37,
     0,    39,    40,   142,    37,     0,    35,    40,    30,   121,
   144,   179,   120,    62,   148,   149,    46,   151,   118,   155,
   144,   148,   161,    -1,    -1,    -1,    -1,   166,    -1,    -1,
   150,    -1,    -1,    -1,    -1,    -1,    -1,   171,     0,     1,
   174,     3,     4,     5,     6,     7,     8,     9,    10,    11,
    12,    13,    14,    15,    16,    17,    18,    19,    20,    21,
    22,    23,    24,    25,    26,    27,    28,    29,    30,    -1,
    -1,    -1,    -1,    -1,    36,    37,    38,    -1,    -1,    -1,
    -1,    43,    -1,    45,     0,     1,    -1,     3,     4,     5,
     6,     7,     8,     9,    10,    11,    12,    13,    14,    15,
    16,    17,    18,    19,    20,    21,    22,    23,    24,    25,
    26,    27,    28,    29,    30,    -1,    -1,    -1,    -1,    -1,
    36,    37,    38,    -1,    -1,    -1,    -1,    43,    -1,    45,
     3,     4,     5,     6,     7,     8,    -1,    10,    11,    12,
    13,    14,    15,    16,    17,    18,    19,    20,    21,    22,
    23,    24,    25,    26,    27,    28,    29,    30,    -1,    32,
    -1,    -1,    -1,    36,    -1,    -1,    -1,    -1,    -1,    -1,
    43,    -1,    45,     3,     4,     5,     6,     7,     8,     9,
    10,    11,    12,    13,    14,    15,    16,    17,    18,    19,
    20,    21,    22,    23,    24,    25,    26,    27,    28,    29,
    30,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    -1,    -1,    -1,    43,    -1,    45,     3,     4,     5,     6,
     7,     8,    -1,    10,    11,    12,    13,    14,    15,    16,
    17,    18,    19,    20,    21,    22,    23,    24,    25,    26,
    27,    28,    29,    30,    -1,    -1,    -1,    -1,    -1,    -1,
    -1,    -1,    39,    -1,    -1,    -1,    43,    -1,    45,     3,
     4,     5,     6,     7,     8,    -1,    10,    11,    12,    13,
    14,    15,    16,    17,    18,    19,    20,    21,    22,    23,
    24,    25,    26,    27,    28,    29,    30,    -1,    -1,    -1,
    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    43,
    -1,    45,     4,     5,     6,     7,     8,    -1,    10,    11,
    12,    13,    14,    15,    16,    17,    18,    19,    20,    21,
    22,    23,    24,    25,    26,    27,    28,    29,    30,    -1,
    -1,    33,     4,     5,     6,     7,     8,     9,    10,    11,
    12,    13,    14,    15,    16,    17,    18,    19,    20,    21,
    22,    23,    24,    25,    26,    27,    28,    29,    30,    31,
     3,     4,     5,     6,     7,     8,    -1,    10,    11,    12,
    13,    14,    15,    16,    17,    18,    19,    20,    21,    22,
    23,    24,    25,    26,    27,    28,    29,    30,     4,     5,
     6,     7,     8,     9,    10,    11,    12,    13,    14,    15,
    16,    17,    18,    19,    20,    21,    22,    23,    24,    25,
    26,    27,    28,    29,    30,     4,     5,     6,     7,     8,
    -1,    10,    11,    12,    13,    14,    15,    16,    17,    18,
    19,    20,    21,    22,    23,    24,    25,    26,    27,    28,
    29,    30,    23,    24,    25,    26,    27,    28,    29,    30,
    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    -1,    -1,    -1,    -1,    45
};
/* -*-C-*-  Note some compilers choke on comments on `#line' lines.  */
#line 3 "/usr/share/bison.simple"
/* This file comes from bison-1.28.  */

/* Skeleton output parser for bison,
   Copyright (C) 1984, 1989, 1990 Free Software Foundation, Inc.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2, or (at your option)
   any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.  */

/* As a special exception, when this file is copied by Bison into a
   Bison output file, you may use that output file without restriction.
   This special exception was added by the Free Software Foundation
   in version 1.24 of Bison.  */

/* This is the parser code that is written into each bison parser
  when the %semantic_parser declaration is not specified in the grammar.
  It was written by Richard Stallman by simplifying the hairy parser
  used when %semantic_parser is specified.  */

#ifndef YYSTACK_USE_ALLOCA
#ifdef alloca
#define YYSTACK_USE_ALLOCA
#else /* alloca not defined */
#ifdef __GNUC__
#define YYSTACK_USE_ALLOCA
#define alloca __builtin_alloca
#else /* not GNU C.  */
#if (!defined (__STDC__) && defined (sparc)) || defined (__sparc__) || defined (__sparc) || defined (__sgi) || (defined (__sun) && defined (__i386))
#define YYSTACK_USE_ALLOCA
#include <alloca.h>
#else /* not sparc */
/* We think this test detects Watcom and Microsoft C.  */
/* This used to test MSDOS, but that is a bad idea
   since that symbol is in the user namespace.  */
#if (defined (_MSDOS) || defined (_MSDOS_)) && !defined (__TURBOC__)
#if 0 /* No need for malloc.h, which pollutes the namespace;
	 instead, just don't use alloca.  */
#include <malloc.h>
#endif
#else /* not MSDOS, or __TURBOC__ */
#if defined(_AIX)
/* I don't know what this was needed for, but it pollutes the namespace.
   So I turned it off.   rms, 2 May 1997.  */
/* #include <malloc.h>  */
 #pragma alloca
#define YYSTACK_USE_ALLOCA
#else /* not MSDOS, or __TURBOC__, or _AIX */
#if 0
#ifdef __hpux /* haible@ilog.fr says this works for HPUX 9.05 and up,
		 and on HPUX 10.  Eventually we can turn this on.  */
#define YYSTACK_USE_ALLOCA
#define alloca __builtin_alloca
#endif /* __hpux */
#endif
#endif /* not _AIX */
#endif /* not MSDOS, or __TURBOC__ */
#endif /* not sparc */
#endif /* not GNU C */
#endif /* alloca not defined */
#endif /* YYSTACK_USE_ALLOCA not defined */

#ifdef YYSTACK_USE_ALLOCA
#define YYSTACK_ALLOC alloca
#else
#define YYSTACK_ALLOC malloc
#endif

/* Note: there must be only one dollar sign in this file.
   It is replaced by the list of actions, each action
   as one case of the switch.  */

#define yyerrok		(yyerrstatus = 0)
#define yyclearin	(yychar = YYEMPTY)
#define YYEMPTY		-2
#define YYEOF		0
#define YYACCEPT	goto yyacceptlab
#define YYABORT 	goto yyabortlab
#define YYERROR		goto yyerrlab1
/* Like YYERROR except do call yyerror.
   This remains here temporarily to ease the
   transition to the new meaning of YYERROR, for GCC.
   Once GCC version 2 has supplanted version 1, this can go.  */
#define YYFAIL		goto yyerrlab
#define YYRECOVERING()  (!!yyerrstatus)
#define YYBACKUP(token, value) \
do								\
  if (yychar == YYEMPTY && yylen == 1)				\
    { yychar = (token), yylval = (value);			\
      yychar1 = YYTRANSLATE (yychar);				\
      YYPOPSTACK;						\
      goto yybackup;						\
    }								\
  else								\
    { yyerror ("syntax error: cannot back up"); YYERROR; }	\
while (0)

#define YYTERROR	1
#define YYERRCODE	256

#ifndef YYPURE
#define YYLEX		yylex()
#endif

#ifdef YYPURE
#ifdef YYLSP_NEEDED
#ifdef YYLEX_PARAM
#define YYLEX		yylex(&yylval, &yylloc, YYLEX_PARAM)
#else
#define YYLEX		yylex(&yylval, &yylloc)
#endif
#else /* not YYLSP_NEEDED */
#ifdef YYLEX_PARAM
#define YYLEX		yylex(&yylval, YYLEX_PARAM)
#else
#define YYLEX		yylex(&yylval)
#endif
#endif /* not YYLSP_NEEDED */
#endif

/* If nonreentrant, generate the variables here */

#ifndef YYPURE

int	yychar;			/*  the lookahead symbol		*/
YYSTYPE	yylval;			/*  the semantic value of the		*/
				/*  lookahead symbol			*/

#ifdef YYLSP_NEEDED
YYLTYPE yylloc;			/*  location data for the lookahead	*/
				/*  symbol				*/
#endif

int yynerrs;			/*  number of parse errors so far       */
#endif  /* not YYPURE */

#if YYDEBUG != 0
int yydebug;			/*  nonzero means print parse trace	*/
/* Since this is uninitialized, it does not stop multiple parsers
   from coexisting.  */
#endif

/*  YYINITDEPTH indicates the initial size of the parser's stacks	*/

#ifndef	YYINITDEPTH
#define YYINITDEPTH 200
#endif

/*  YYMAXDEPTH is the maximum size the stacks can grow to
    (effective only if the built-in stack extension method is used).  */

#if YYMAXDEPTH == 0
#undef YYMAXDEPTH
#endif

#ifndef YYMAXDEPTH
#define YYMAXDEPTH 10000
#endif

/* Define __yy_memcpy.  Note that the size argument
   should be passed with type unsigned int, because that is what the non-GCC
   definitions require.  With GCC, __builtin_memcpy takes an arg
   of type size_t, but it can handle unsigned int.  */

#if __GNUC__ > 1		/* GNU C and GNU C++ define this.  */
#define __yy_memcpy(TO,FROM,COUNT)	__builtin_memcpy(TO,FROM,COUNT)
#else				/* not GNU C or C++ */
#ifndef __cplusplus

/* This is the most reliable way to avoid incompatibilities
   in available built-in functions on various systems.  */
static void
__yy_memcpy (to, from, count)
     char *to;
     char *from;
     unsigned int count;
{
  register char *f = from;
  register char *t = to;
  register int i = count;

  while (i-- > 0)
    *t++ = *f++;
}

#else /* __cplusplus */

/* This is the most reliable way to avoid incompatibilities
   in available built-in functions on various systems.  */
static void
__yy_memcpy (char *to, char *from, unsigned int count)
{
  register char *t = to;
  register char *f = from;
  register int i = count;

  while (i-- > 0)
    *t++ = *f++;
}

#endif
#endif

#line 217 "/usr/share/bison.simple"

/* The user can define YYPARSE_PARAM as the name of an argument to be passed
   into yyparse.  The argument should have type void *.
   It should actually point to an object.
   Grammar actions can access the variable by casting it
   to the proper pointer type.  */

#ifdef YYPARSE_PARAM
#ifdef __cplusplus
#define YYPARSE_PARAM_ARG void *YYPARSE_PARAM
#define YYPARSE_PARAM_DECL
#else /* not __cplusplus */
#define YYPARSE_PARAM_ARG YYPARSE_PARAM
#define YYPARSE_PARAM_DECL void *YYPARSE_PARAM;
#endif /* not __cplusplus */
#else /* not YYPARSE_PARAM */
#define YYPARSE_PARAM_ARG
#define YYPARSE_PARAM_DECL
#endif /* not YYPARSE_PARAM */

/* Prevent warning if -Wstrict-prototypes.  */
#ifdef __GNUC__
#ifdef YYPARSE_PARAM
int yyparse (void *);
#else
int yyparse (void);
#endif
#endif

int
yyparse(YYPARSE_PARAM_ARG)
     YYPARSE_PARAM_DECL
{
  register int yystate;
  register int yyn;
  register short *yyssp;
  register YYSTYPE *yyvsp;
  int yyerrstatus;	/*  number of tokens to shift before error messages enabled */
  int yychar1 = 0;		/*  lookahead token as an internal (translated) token number */

  short	yyssa[YYINITDEPTH];	/*  the state stack			*/
  YYSTYPE yyvsa[YYINITDEPTH];	/*  the semantic value stack		*/

  short *yyss = yyssa;		/*  refer to the stacks thru separate pointers */
  YYSTYPE *yyvs = yyvsa;	/*  to allow yyoverflow to reallocate them elsewhere */

#ifdef YYLSP_NEEDED
  YYLTYPE yylsa[YYINITDEPTH];	/*  the location stack			*/
  YYLTYPE *yyls = yylsa;
  YYLTYPE *yylsp;

#define YYPOPSTACK   (yyvsp--, yyssp--, yylsp--)
#else
#define YYPOPSTACK   (yyvsp--, yyssp--)
#endif

  int yystacksize = YYINITDEPTH;
  int yyfree_stacks = 0;

#ifdef YYPURE
  int yychar;
  YYSTYPE yylval;
  int yynerrs;
#ifdef YYLSP_NEEDED
  YYLTYPE yylloc;
#endif
#endif

  YYSTYPE yyval;		/*  the variable used to return		*/
				/*  semantic values from the action	*/
				/*  routines				*/

  int yylen;

#if YYDEBUG != 0
  if (yydebug)
    fprintf(stderr, "Starting parse\n");
#endif

  yystate = 0;
  yyerrstatus = 0;
  yynerrs = 0;
  yychar = YYEMPTY;		/* Cause a token to be read.  */

  /* Initialize stack pointers.
     Waste one element of value and location stack
     so that they stay on the same level as the state stack.
     The wasted elements are never initialized.  */

  yyssp = yyss - 1;
  yyvsp = yyvs;
#ifdef YYLSP_NEEDED
  yylsp = yyls;
#endif

/* Push a new state, which is found in  yystate  .  */
/* In all cases, when you get here, the value and location stacks
   have just been pushed. so pushing a state here evens the stacks.  */
yynewstate:

  *++yyssp = yystate;

  if (yyssp >= yyss + yystacksize - 1)
    {
      /* Give user a chance to reallocate the stack */
      /* Use copies of these so that the &'s don't force the real ones into memory. */
      YYSTYPE *yyvs1 = yyvs;
      short *yyss1 = yyss;
#ifdef YYLSP_NEEDED
      YYLTYPE *yyls1 = yyls;
#endif

      /* Get the current used size of the three stacks, in elements.  */
      int size = yyssp - yyss + 1;

#ifdef yyoverflow
      /* Each stack pointer address is followed by the size of
	 the data in use in that stack, in bytes.  */
#ifdef YYLSP_NEEDED
      /* This used to be a conditional around just the two extra args,
	 but that might be undefined if yyoverflow is a macro.  */
      yyoverflow("parser stack overflow",
		 &yyss1, size * sizeof (*yyssp),
		 &yyvs1, size * sizeof (*yyvsp),
		 &yyls1, size * sizeof (*yylsp),
		 &yystacksize);
#else
      yyoverflow("parser stack overflow",
		 &yyss1, size * sizeof (*yyssp),
		 &yyvs1, size * sizeof (*yyvsp),
		 &yystacksize);
#endif

      yyss = yyss1; yyvs = yyvs1;
#ifdef YYLSP_NEEDED
      yyls = yyls1;
#endif
#else /* no yyoverflow */
      /* Extend the stack our own way.  */
      if (yystacksize >= YYMAXDEPTH)
	{
	  yyerror("parser stack overflow");
	  if (yyfree_stacks)
	    {
	      free (yyss);
	      free (yyvs);
#ifdef YYLSP_NEEDED
	      free (yyls);
#endif
	    }
	  return 2;
	}
      yystacksize *= 2;
      if (yystacksize > YYMAXDEPTH)
	yystacksize = YYMAXDEPTH;
#ifndef YYSTACK_USE_ALLOCA
      yyfree_stacks = 1;
#endif
      yyss = (short *) YYSTACK_ALLOC (yystacksize * sizeof (*yyssp));
      __yy_memcpy ((char *)yyss, (char *)yyss1,
		   size * (unsigned int) sizeof (*yyssp));
      yyvs = (YYSTYPE *) YYSTACK_ALLOC (yystacksize * sizeof (*yyvsp));
      __yy_memcpy ((char *)yyvs, (char *)yyvs1,
		   size * (unsigned int) sizeof (*yyvsp));
#ifdef YYLSP_NEEDED
      yyls = (YYLTYPE *) YYSTACK_ALLOC (yystacksize * sizeof (*yylsp));
      __yy_memcpy ((char *)yyls, (char *)yyls1,
		   size * (unsigned int) sizeof (*yylsp));
#endif
#endif /* no yyoverflow */

      yyssp = yyss + size - 1;
      yyvsp = yyvs + size - 1;
#ifdef YYLSP_NEEDED
      yylsp = yyls + size - 1;
#endif

#if YYDEBUG != 0
      if (yydebug)
	fprintf(stderr, "Stack size increased to %d\n", yystacksize);
#endif

      if (yyssp >= yyss + yystacksize - 1)
	YYABORT;
    }

#if YYDEBUG != 0
  if (yydebug)
    fprintf(stderr, "Entering state %d\n", yystate);
#endif

  goto yybackup;
 yybackup:

/* Do appropriate processing given the current state.  */
/* Read a lookahead token if we need one and don't already have one.  */
/* yyresume: */

  /* First try to decide what to do without reference to lookahead token.  */

  yyn = yypact[yystate];
  if (yyn == YYFLAG)
    goto yydefault;

  /* Not known => get a lookahead token if don't already have one.  */

  /* yychar is either YYEMPTY or YYEOF
     or a valid token in external form.  */

  if (yychar == YYEMPTY)
    {
#if YYDEBUG != 0
      if (yydebug)
	fprintf(stderr, "Reading a token: ");
#endif
      yychar = YYLEX;
    }

  /* Convert token to internal form (in yychar1) for indexing tables with */

  if (yychar <= 0)		/* This means end of input. */
    {
      yychar1 = 0;
      yychar = YYEOF;		/* Don't call YYLEX any more */

#if YYDEBUG != 0
      if (yydebug)
	fprintf(stderr, "Now at end of input.\n");
#endif
    }
  else
    {
      yychar1 = YYTRANSLATE(yychar);

#if YYDEBUG != 0
      if (yydebug)
	{
	  fprintf (stderr, "Next token is %d (%s", yychar, yytname[yychar1]);
	  /* Give the individual parser a way to print the precise meaning
	     of a token, for further debugging info.  */
#ifdef YYPRINT
	  YYPRINT (stderr, yychar, yylval);
#endif
	  fprintf (stderr, ")\n");
	}
#endif
    }

  yyn += yychar1;
  if (yyn < 0 || yyn > YYLAST || yycheck[yyn] != yychar1)
    goto yydefault;

  yyn = yytable[yyn];

  /* yyn is what to do for this token type in this state.
     Negative => reduce, -yyn is rule number.
     Positive => shift, yyn is new state.
       New state is final state => don't bother to shift,
       just return success.
     0, or most negative number => error.  */

  if (yyn < 0)
    {
      if (yyn == YYFLAG)
	goto yyerrlab;
      yyn = -yyn;
      goto yyreduce;
    }
  else if (yyn == 0)
    goto yyerrlab;

  if (yyn == YYFINAL)
    YYACCEPT;

  /* Shift the lookahead token.  */

#if YYDEBUG != 0
  if (yydebug)
    fprintf(stderr, "Shifting token %d (%s), ", yychar, yytname[yychar1]);
#endif

  /* Discard the token being shifted unless it is eof.  */
  if (yychar != YYEOF)
    yychar = YYEMPTY;

  *++yyvsp = yylval;
#ifdef YYLSP_NEEDED
  *++yylsp = yylloc;
#endif

  /* count tokens shifted since error; after three, turn off error status.  */
  if (yyerrstatus) yyerrstatus--;

  yystate = yyn;
  goto yynewstate;

/* Do the default action for the current state.  */
yydefault:

  yyn = yydefact[yystate];
  if (yyn == 0)
    goto yyerrlab;

/* Do a reduction.  yyn is the number of a rule to reduce with.  */
yyreduce:
  yylen = yyr2[yyn];
  if (yylen > 0)
    yyval = yyvsp[1-yylen]; /* implement default value of the action */

#if YYDEBUG != 0
  if (yydebug)
    {
      int i;

      fprintf (stderr, "Reducing via rule %d (line %d), ",
	       yyn, yyrline[yyn]);

      /* Print the symbols being reduced, and their result.  */
      for (i = yyprhs[yyn]; yyrhs[i] > 0; i++)
	fprintf (stderr, "%s ", yytname[yyrhs[i]]);
      fprintf (stderr, " -> %s\n", yytname[yyr1[yyn]]);
    }
#endif


  switch (yyn) {

case 5:
#line 114 "grammar.y"
{
	    remember_declarations(NULL, &yyvsp[-1].declaration.decl_spec, &yyvsp[-1].declaration.decl_list, yyvsp[0].text);
	;
    break;}
case 6:
#line 118 "grammar.y"
{
	    remember_declarations(yyvsp[-2].text, &yyvsp[-1].declaration.decl_spec, &yyvsp[-1].declaration.decl_list, yyvsp[0].text);
	;
    break;}
case 7:
#line 122 "grammar.y"
{
	    if (look_at_body_start && body_comment) {	
				/* Use the body comment */
	      new_manual_page(body_comment,&yyvsp[-1].parameter.decl_spec,yyvsp[-1].parameter.declarator);
	      body_comment = NULL; /* Prevent it being free'ed */
	    } else {
	      free_declarator(yyvsp[-1].parameter.declarator);
	      free_decl_spec(&yyvsp[-1].parameter.decl_spec);
	    }
	;
    break;}
case 8:
#line 133 "grammar.y"
{
	    if (body_start_only) {
	      if (body_comment) {
		new_manual_page(body_comment,&yyvsp[-1].parameter.decl_spec,yyvsp[-1].parameter.declarator);
		body_comment = NULL; /* Prevent it being free'ed */
	      } else {
		free_declarator(yyvsp[-1].parameter.declarator);
		free_decl_spec(&yyvsp[-1].parameter.decl_spec);
		safe_free(yyvsp[-2].text);
	      }      
	    } else {
	      new_manual_page(yyvsp[-2].text,&yyvsp[-1].parameter.decl_spec,yyvsp[-1].parameter.declarator);
	    }
	    safe_free(yyvsp[0].text);
	;
    break;}
case 9:
#line 149 "grammar.y"
{
	    if (look_at_body_start && body_comment) {
	      new_manual_page(body_comment,&yyvsp[-2].parameter.decl_spec,yyvsp[-2].parameter.declarator);
	      body_comment = NULL; /* Prevent it being free'ed */
	    } else {
	      free_declarator(yyvsp[-2].parameter.declarator);
	      free_decl_spec(&yyvsp[-2].parameter.decl_spec);
	    }
	    safe_free(yyvsp[0].text);
	;
    break;}
case 10:
#line 160 "grammar.y"
{
	    if (body_start_only) {
	      if (body_comment) {
		new_manual_page(body_comment,&yyvsp[-2].parameter.decl_spec,yyvsp[-2].parameter.declarator);
		body_comment = NULL; /* Prevent it being free'ed */
	      } else {
		free_declarator(yyvsp[-2].parameter.declarator);
		free_decl_spec(&yyvsp[-2].parameter.decl_spec);
		safe_free(yyvsp[-3].text);
	      }      
	    } else {
	      new_manual_page(yyvsp[-3].text,&yyvsp[-2].parameter.decl_spec,yyvsp[-2].parameter.declarator);
	    }
	    safe_free(yyvsp[0].text);
	;
    break;}
case 12:
#line 177 "grammar.y"
{
	    if (inbasefile && first_comment)
	    {
		remember_terse(yyvsp[0].text);
		first_comment = FALSE;
	    }
	    free(yyvsp[0].text);
	;
    break;}
case 13:
#line 186 "grammar.y"
{
	    free(yyvsp[0].text);
	;
    break;}
case 14:
#line 190 "grammar.y"
{
	    inbasefile = yyvsp[0].boolean;
	;
    break;}
case 15:
#line 194 "grammar.y"
{
	    yyerrok;
	;
    break;}
case 16:
#line 201 "grammar.y"
{
	    /* Provide an empty action here so bison will not complain about
	     * incompatible types in the default action it normally would
	     * have generated.
	     */
	;
    break;}
case 17:
#line 208 "grammar.y"
{
	    /* empty */
	;
    break;}
case 18:
#line 215 "grammar.y"
{
	    if (yyvsp[-1].declarator->type != DECL_FUNCTION) {
		yyerror("syntax error");
		YYERROR;
	    }
	    func_params = &(yyvsp[-1].declarator->head->params);
            if (yyvsp[0].text)	comment_last_parameter(&yyvsp[-1].declarator->head->params, yyvsp[0].text);
	;
    break;}
case 19:
#line 224 "grammar.y"
{
	    func_params = NULL;
	    yyvsp[-4].declarator->type = DECL_FUNCDEF;

	    yyval.parameter.decl_spec = yyvsp[-5].decl_spec;
	    yyval.parameter.declarator = yyvsp[-4].declarator;
	;
    break;}
case 20:
#line 232 "grammar.y"
{
	    if (yyvsp[-1].declarator->type != DECL_FUNCTION) {
		yyerror("syntax error");
		YYERROR;
	    }
	    func_params = &(yyvsp[-1].declarator->head->params);
            if (yyvsp[0].text)	comment_last_parameter(&yyvsp[-1].declarator->head->params, yyvsp[0].text);
	;
    break;}
case 21:
#line 241 "grammar.y"
{
	    DeclSpec	decl_spec;

	    func_params = NULL;
	    yyvsp[-4].declarator->type = DECL_FUNCDEF;

	    new_decl_spec(&yyval.parameter.decl_spec, "int", DS_NONE);
	    yyval.parameter.declarator = yyvsp[-4].declarator;
	;
    break;}
case 22:
#line 254 "grammar.y"
{
	    yyval.declaration.decl_spec = yyvsp[-1].decl_spec;
	    yyval.declaration.decl_list.first = NULL;
	;
    break;}
case 23:
#line 259 "grammar.y"
{
	    yyval.declaration.decl_spec = yyvsp[-2].decl_spec;
	    yyval.declaration.decl_list = yyvsp[-1].decl_list;
	;
    break;}
case 24:
#line 264 "grammar.y"
{
	    new_typedef_symbols(&yyvsp[-2].decl_spec,&yyvsp[-1].decl_list);
	    yyval.declaration.decl_spec = yyvsp[-2].decl_spec;
	    yyval.declaration.decl_list = yyvsp[-1].decl_list;
	;
    break;}
case 25:
#line 273 "grammar.y"
{
	    new_decl_list(&yyval.decl_list, yyvsp[0].declarator);
	;
    break;}
case 26:
#line 277 "grammar.y"
{
            if (yyvsp[-1].text)	comment_last_decl(&yyvsp[-3].decl_list, yyvsp[-1].text);
            add_decl_list(&yyval.decl_list, &yyvsp[-3].decl_list, yyvsp[0].declarator);
        ;
    break;}
case 27:
#line 282 "grammar.y"
{
            yyval.decl_list = yyvsp[-1].decl_list;
            if (yyvsp[0].text)	comment_last_decl(&yyvsp[-1].decl_list, yyvsp[0].text);
        ;
    break;}
case 31:
#line 296 "grammar.y"
{
	    set_param_types(func_params, &yyvsp[-1].declaration.decl_spec, &yyvsp[-1].declaration.decl_list, yyvsp[-2].text, yyvsp[0].text);
	;
    break;}
case 32:
#line 300 "grammar.y"
{
	    set_param_types(func_params, &yyvsp[-1].declaration.decl_spec, &yyvsp[-1].declaration.decl_list, yyvsp[-2].text, yyvsp[0].text);
	;
    break;}
case 34:
#line 308 "grammar.y"
{
	    join_decl_specs(&yyval.decl_spec, &yyvsp[-1].decl_spec, &yyvsp[0].decl_spec);
	;
    break;}
case 38:
#line 321 "grammar.y"
{
	    new_decl_spec(&yyval.decl_spec, "auto", DS_NONE);
	;
    break;}
case 39:
#line 325 "grammar.y"
{
	    new_decl_spec(&yyval.decl_spec, "extern", DS_EXTERN);
	;
    break;}
case 40:
#line 329 "grammar.y"
{
	    new_decl_spec(&yyval.decl_spec, "register", DS_NONE);
	;
    break;}
case 41:
#line 333 "grammar.y"
{
	    new_decl_spec(&yyval.decl_spec, "static", DS_STATIC);
	;
    break;}
case 42:
#line 337 "grammar.y"
{
	    new_decl_spec(&yyval.decl_spec, "inline", DS_INLINE);
	;
    break;}
case 43:
#line 344 "grammar.y"
{
	    new_decl_spec(&yyval.decl_spec, "char", DS_CHAR);
	;
    break;}
case 44:
#line 348 "grammar.y"
{
	    new_decl_spec(&yyval.decl_spec, "double", DS_NONE);
	;
    break;}
case 45:
#line 352 "grammar.y"
{
	    new_decl_spec(&yyval.decl_spec, "float", DS_FLOAT);
	;
    break;}
case 46:
#line 356 "grammar.y"
{
	    new_decl_spec(&yyval.decl_spec, "int", DS_NONE);
	;
    break;}
case 47:
#line 360 "grammar.y"
{
	    new_decl_spec(&yyval.decl_spec, "long", DS_NONE);
	;
    break;}
case 48:
#line 364 "grammar.y"
{
	    new_decl_spec(&yyval.decl_spec, "short", DS_SHORT);
	;
    break;}
case 49:
#line 368 "grammar.y"
{
	    new_decl_spec(&yyval.decl_spec, "signed", DS_NONE);
	;
    break;}
case 50:
#line 372 "grammar.y"
{
	    new_decl_spec(&yyval.decl_spec, "unsigned", DS_NONE);
	;
    break;}
case 51:
#line 376 "grammar.y"
{
	    new_decl_spec(&yyval.decl_spec, "void", DS_NONE);
	;
    break;}
case 54:
#line 382 "grammar.y"
{
	    Symbol *s = find_symbol(typedef_names, yyvsp[0].text);
	   
	    new_enum_decl_spec(&yyval.decl_spec, yyvsp[0].text, s->flags,
		s->valtype == SYMVAL_ENUM ? s->value.enum_list
					  : (EnumeratorList *)NULL);
	;
    break;}
case 55:
#line 393 "grammar.y"
{
	    new_decl_spec(&yyval.decl_spec, "const", DS_NONE);
	;
    break;}
case 56:
#line 397 "grammar.y"
{
	    new_decl_spec(&yyval.decl_spec, "volatile", DS_NONE);
	;
    break;}
case 57:
#line 401 "grammar.y"
{
	    new_decl_spec(&yyval.decl_spec, "cdecl", DS_NONE);
	;
    break;}
case 58:
#line 405 "grammar.y"
{
	    new_decl_spec(&yyval.decl_spec, "interrupt", DS_NONE);
	;
    break;}
case 59:
#line 409 "grammar.y"
{
	    new_decl_spec(&yyval.decl_spec, "far", DS_NONE);
	;
    break;}
case 60:
#line 413 "grammar.y"
{
	    new_decl_spec(&yyval.decl_spec, "huge", DS_NONE);
	;
    break;}
case 61:
#line 417 "grammar.y"
{
	    new_decl_spec(&yyval.decl_spec, "near", DS_NONE);
	;
    break;}
case 62:
#line 421 "grammar.y"
{
	    new_decl_spec(&yyval.decl_spec, "pascal", DS_NONE);
	;
    break;}
case 63:
#line 428 "grammar.y"
{
	    dyn_decl_spec(&yyval.decl_spec, strconcat(yyvsp[-2].text, " ",yyvsp[-1].text," {}",NULLCP), DS_NONE);
	    free(yyvsp[-1].text);
	;
    break;}
case 64:
#line 433 "grammar.y"
{
	    dyn_decl_spec(&yyval.decl_spec, strconcat(yyvsp[-1].text," {}",NULLCP), DS_NONE);
	;
    break;}
case 65:
#line 437 "grammar.y"
{
	    dyn_decl_spec(&yyval.decl_spec, strconcat(yyvsp[-1].text, " ",yyvsp[0].text,NULLCP), DS_NONE);
	    free(yyvsp[0].text);
	;
    break;}
case 66:
#line 445 "grammar.y"
{
	    yyval.text = "struct";
	;
    break;}
case 67:
#line 449 "grammar.y"
{
	    yyval.text = "union";
	;
    break;}
case 68:
#line 456 "grammar.y"
{
	    new_decl_list(&yyval.decl_list, yyvsp[0].declarator);
	;
    break;}
case 69:
#line 460 "grammar.y"
{
            if (yyvsp[-1].text)	comment_last_decl(&yyvsp[-3].decl_list, yyvsp[-1].text);
            add_decl_list(&yyval.decl_list, &yyvsp[-3].decl_list, yyvsp[0].declarator);
        ;
    break;}
case 70:
#line 465 "grammar.y"
{
            yyval.decl_list = yyvsp[-1].decl_list;
            if (yyvsp[0].text)	comment_last_decl(&yyvsp[-1].decl_list, yyvsp[0].text);
        ;
    break;}
case 73:
#line 478 "grammar.y"
{
	    add_enum_symbol(yyvsp[-4].text, yyvsp[-1].enum_list);
	    new_enum_decl_spec(&yyval.decl_spec, strconcat("enum ",yyvsp[-4].text," {}",NULLCP),
		   DS_NONE, yyvsp[-1].enum_list);
	    free(yyvsp[-4].text);
	    safe_free(yyvsp[-2].text);
	    enum_state = NOENUM;
	;
    break;}
case 74:
#line 487 "grammar.y"
{
	    new_enum_decl_spec(&yyval.decl_spec, strduplicate("enum {}"), DS_NONE, yyvsp[-1].enum_list);
	    safe_free(yyvsp[-2].text);
	    enum_state = NOENUM;
	;
    break;}
case 75:
#line 493 "grammar.y"
{
	    new_enum_decl_spec(&yyval.decl_spec, strconcat("enum ",yyvsp[0].text,NULLCP), DS_NONE,
	    	find_enum_symbol(yyvsp[0].text));
	    free(yyvsp[0].text);
	    enum_state = NOENUM;
	;
    break;}
case 76:
#line 503 "grammar.y"
{
	    yyval.enum_list = new_enumerator_list(&yyvsp[0].enumerator);
	;
    break;}
case 77:
#line 507 "grammar.y"
{
	    yyval.enum_list = yyvsp[-3].enum_list;
	    if (yyvsp[-1].text)	comment_last_enumerator(yyval.enum_list, yyvsp[-1].text);
	    add_enumerator_list(yyval.enum_list, &yyvsp[0].enumerator);
	;
    break;}
case 78:
#line 513 "grammar.y"
{
	    yyval.enum_list = yyvsp[-1].enum_list;
	    if (yyvsp[0].text)	comment_last_enumerator(yyval.enum_list, yyvsp[0].text);
	;
    break;}
case 79:
#line 518 "grammar.y"
{
	    yyval.enum_list = yyvsp[-2].enum_list;
	    if (yyvsp[0].text)     comment_last_enumerator(yyval.enum_list, yyvsp[0].text);
	;
    break;}
case 80:
#line 526 "grammar.y"
{
	    new_enumerator(&yyval.enumerator,yyvsp[0].identifier.name,yyvsp[0].identifier.comment_before,yyvsp[0].identifier.comment_after);
	;
    break;}
case 83:
#line 535 "grammar.y"
{
	    yyval.text = yyvsp[-1].text;
	    free(yyvsp[0].text);
	;
    break;}
case 84:
#line 540 "grammar.y"
{
	    yyval.text = yyvsp[-1].text;
	    free(yyvsp[0].text);
	;
    break;}
case 85:
#line 548 "grammar.y"
{
	    char *newtext = strappend(yyvsp[-1].text,yyvsp[0].declarator->text,NULLCP);
	    free(yyvsp[0].declarator->text);
	    yyval.declarator = yyvsp[0].declarator;
	    yyval.declarator->text = newtext;
	    if (yyval.declarator->type == DECL_SIMPLE)
		yyval.declarator->type = DECL_COMPOUND;
	;
    break;}
case 87:
#line 561 "grammar.y"
{
	    yyval.declarator = new_declarator(yyvsp[0].text, strduplicate(yyvsp[0].text));
	;
    break;}
case 88:
#line 565 "grammar.y"
{
	    char *newtext = strconcat("(",yyvsp[-1].declarator->text,")",NULLCP);
	    free(yyvsp[-1].declarator->text);
	    yyval.declarator = yyvsp[-1].declarator;
	    yyval.declarator->text = newtext;
	;
    break;}
case 89:
#line 572 "grammar.y"
{
	    yyval.declarator = yyvsp[-1].declarator;
	    yyval.declarator->text = strappend(yyvsp[-1].declarator->text,yyvsp[0].text,NULLCP);
	    free(yyvsp[0].text);
	;
    break;}
case 90:
#line 578 "grammar.y"
{
	    yyval.declarator = new_declarator(strduplicate("%s()"), strduplicate(yyvsp[-3].declarator->name));
	    yyval.declarator->params = yyvsp[-1].param_list;
	    yyval.declarator->func_stack = yyvsp[-3].declarator;
	    yyval.declarator->head = (yyvsp[-3].declarator->func_stack == NULL) ? yyval.declarator : yyvsp[-3].declarator->head;
	    yyval.declarator->type = (yyvsp[-3].declarator->type == DECL_SIMPLE) ? DECL_FUNCTION : yyvsp[-3].declarator->type;
	;
    break;}
case 91:
#line 586 "grammar.y"
{
	    yyval.declarator = new_declarator(strduplicate("%s()"), strduplicate(yyvsp[-3].declarator->name));
	    yyval.declarator->params = yyvsp[-1].param_list;
	    yyval.declarator->func_stack = yyvsp[-3].declarator;
	    yyval.declarator->head = (yyvsp[-3].declarator->func_stack == NULL) ? yyval.declarator : yyvsp[-3].declarator->head;
	    yyval.declarator->type = (yyvsp[-3].declarator->type == DECL_SIMPLE) ? DECL_FUNCTION : yyvsp[-3].declarator->type;
	;
    break;}
case 92:
#line 597 "grammar.y"
{
	    yyval.text = strconcat("*",yyvsp[0].text, NULLCP);
	    safe_free(yyvsp[0].text);
	;
    break;}
case 93:
#line 602 "grammar.y"
{
	    yyval.text = yyvsp[-1].text ? strconcat("*",yyvsp[-1].text, yyvsp[0].text, NULLCP)
		    : strconcat("*", yyvsp[0].text, NULLCP);
	    safe_free(yyvsp[-1].text);
	    free(yyvsp[0].text);
	;
    break;}
case 94:
#line 612 "grammar.y"
{
	    yyval.text = NULL;
	;
    break;}
case 95:
#line 616 "grammar.y"
{
	    yyval.text = yyvsp[-1].text ? strconcat(yyvsp[-1].text," ",yyvsp[0].decl_spec.text," ",NULLCP)
		    : strconcat(yyvsp[0].decl_spec.text," ",NULLCP);
	    safe_free(yyvsp[-1].text);
	    free_decl_spec(&yyvsp[0].decl_spec);
	;
    break;}
case 96:
#line 626 "grammar.y"
{
	    yyval.param_list = yyvsp[-1].param_list;
	    if (yyvsp[0].text)	comment_last_parameter(&yyvsp[-1].param_list, yyvsp[0].text);
	;
    break;}
case 97:
#line 632 "grammar.y"
{
	    Identifier ellipsis;

	    if (yyvsp[-4].text)	comment_last_parameter(&yyvsp[-6].param_list, yyvsp[-4].text);
	    ellipsis.name = strduplicate("...");

	    if (yyvsp[-3].text && yyvsp[-1].text && yyvsp[0].text)
	    {
		yyerror("ellipsis parameter has multiple comments");
		free(yyvsp[0].text);
		free(yyvsp[-1].text);
		free(yyvsp[-3].text);
		ellipsis.comment_before = ellipsis.comment_after = NULL;
	    }
	    else
	    {
		ellipsis.comment_before = yyvsp[-3].text;
		ellipsis.comment_after = yyvsp[-1].text ? yyvsp[-1].text : yyvsp[0].text;
	    }

	    add_ident_list(&yyval.param_list, &yyvsp[-6].param_list, &ellipsis);
	;
    break;}
case 98:
#line 658 "grammar.y"
{
	    new_param_list(&yyval.param_list, &yyvsp[0].parameter);
	;
    break;}
case 99:
#line 662 "grammar.y"
{
	    if (yyvsp[-1].text)	comment_last_parameter(&yyvsp[-3].param_list, yyvsp[-1].text);
	    add_param_list(&yyval.param_list, &yyvsp[-3].param_list, &yyvsp[0].parameter);
	;
    break;}
case 100:
#line 670 "grammar.y"
{
	    new_parameter(&yyval.parameter, &yyvsp[-2].decl_spec, yyvsp[-1].declarator, yyvsp[-3].text, yyvsp[0].text);
	;
    break;}
case 101:
#line 674 "grammar.y"
{
	    new_parameter(&yyval.parameter, &yyvsp[-2].decl_spec, yyvsp[-1].declarator, yyvsp[-3].text, yyvsp[0].text);
	;
    break;}
case 102:
#line 678 "grammar.y"
{
	    new_parameter(&yyval.parameter, &yyvsp[-1].decl_spec, (Declarator *)NULL, yyvsp[-2].text, yyvsp[0].text);
	;
    break;}
case 103:
#line 685 "grammar.y"
{
	    new_ident_list(&yyval.param_list);
	;
    break;}
case 104:
#line 689 "grammar.y"
{
	    yyval.param_list = yyvsp[-1].param_list;
	    if (yyvsp[0].text)	comment_last_parameter(&yyvsp[-1].param_list, yyvsp[0].text);
	;
    break;}
case 105:
#line 697 "grammar.y"
{
	    new_ident_list(&yyval.param_list);
	    add_ident_list(&yyval.param_list, &yyval.param_list, &yyvsp[0].identifier);
	;
    break;}
case 106:
#line 702 "grammar.y"
{
	    if (yyvsp[-1].text)	comment_last_parameter(&yyvsp[-3].param_list, yyvsp[-1].text);
	    add_ident_list(&yyval.param_list, &yyvsp[-3].param_list, &yyvsp[0].identifier);
	;
    break;}
case 107:
#line 710 "grammar.y"
{
	    yyval.identifier.comment_before = yyvsp[-2].text;
	    yyval.identifier.comment_after = yyvsp[0].text;
	    yyval.identifier.name = yyvsp[-1].text;
	;
    break;}
case 108:
#line 718 "grammar.y"
{
	    yyval.declarator = new_declarator(yyvsp[0].text, NULLCP);
	;
    break;}
case 109:
#line 722 "grammar.y"
{
	    char *newtext = strappend(yyvsp[-1].text,yyvsp[0].declarator->text,NULLCP);
	    free(yyvsp[0].declarator->text);
	    yyval.declarator = yyvsp[0].declarator;
	    yyval.declarator->text = newtext;
	    if (yyval.declarator->type == DECL_SIMPLE)
		yyval.declarator->type = DECL_COMPOUND;
	;
    break;}
case 111:
#line 735 "grammar.y"
{
	    char *newtext = strconcat("(",yyvsp[-1].declarator->text,")",NULLCP);
	    free(yyvsp[-1].declarator->text);
	    yyval.declarator = yyvsp[-1].declarator;
	    yyval.declarator->text = newtext;
	;
    break;}
case 112:
#line 742 "grammar.y"
{
	    yyval.declarator = yyvsp[-1].declarator;
	    yyval.declarator->text = strappend(yyvsp[-1].declarator->text,yyvsp[0].text,NULLCP);
	    free(yyvsp[0].text);
	;
    break;}
case 113:
#line 748 "grammar.y"
{
	    yyval.declarator = new_declarator(yyvsp[0].text, NULLCP);
	;
    break;}
case 114:
#line 752 "grammar.y"
{
	    yyval.declarator = new_declarator(strduplicate("%s()"), NULLCP);
	    yyval.declarator->params = yyvsp[-1].param_list;
	    yyval.declarator->func_stack = yyvsp[-3].declarator;
	    yyval.declarator->head = (yyvsp[-3].declarator->func_stack == NULL) ? yyval.declarator : yyvsp[-3].declarator->head;
	    yyval.declarator->type = (yyvsp[-3].declarator->type == DECL_SIMPLE) ? DECL_FUNCTION : yyvsp[-3].declarator->type;
	;
    break;}
case 115:
#line 760 "grammar.y"
{
	    yyval.declarator = new_declarator(strduplicate("%s()"), NULLCP);
	    yyval.declarator->func_stack = yyvsp[-2].declarator;
	    yyval.declarator->head = (yyvsp[-2].declarator->func_stack == NULL) ? yyval.declarator : yyvsp[-2].declarator->head;
	    yyval.declarator->type = (yyvsp[-2].declarator->type == DECL_SIMPLE) ? DECL_FUNCTION : yyvsp[-2].declarator->type;
	;
    break;}
case 116:
#line 767 "grammar.y"
{
	    Declarator *d;
	    
	    d = new_declarator(NULL, NULL);
	    yyval.declarator = new_declarator(strduplicate("%s()"), NULLCP);
	    yyval.declarator->params = yyvsp[-1].param_list;
	    yyval.declarator->func_stack = d;
	    yyval.declarator->head = yyval.declarator;
	;
    break;}
case 117:
#line 777 "grammar.y"
{
	    Declarator *d;
	    
	    d = new_declarator(NULL, NULL);
	    yyval.declarator = new_declarator(strduplicate("%s()"), NULLCP);
	    yyval.declarator->func_stack = d;
	    yyval.declarator->head = yyval.declarator;
	;
    break;}
case 118:
#line 789 "grammar.y"
{
	    yyval.text = NULL;
	;
    break;}
case 120:
#line 797 "grammar.y"
{
	    yyval.text = NULL;
	;
    break;}
}
   /* the action file gets copied in in place of this dollarsign */
#line 543 "/usr/share/bison.simple"

  yyvsp -= yylen;
  yyssp -= yylen;
#ifdef YYLSP_NEEDED
  yylsp -= yylen;
#endif

#if YYDEBUG != 0
  if (yydebug)
    {
      short *ssp1 = yyss - 1;
      fprintf (stderr, "state stack now");
      while (ssp1 != yyssp)
	fprintf (stderr, " %d", *++ssp1);
      fprintf (stderr, "\n");
    }
#endif

  *++yyvsp = yyval;

#ifdef YYLSP_NEEDED
  yylsp++;
  if (yylen == 0)
    {
      yylsp->first_line = yylloc.first_line;
      yylsp->first_column = yylloc.first_column;
      yylsp->last_line = (yylsp-1)->last_line;
      yylsp->last_column = (yylsp-1)->last_column;
      yylsp->text = 0;
    }
  else
    {
      yylsp->last_line = (yylsp+yylen-1)->last_line;
      yylsp->last_column = (yylsp+yylen-1)->last_column;
    }
#endif

  /* Now "shift" the result of the reduction.
     Determine what state that goes to,
     based on the state we popped back to
     and the rule number reduced by.  */

  yyn = yyr1[yyn];

  yystate = yypgoto[yyn - YYNTBASE] + *yyssp;
  if (yystate >= 0 && yystate <= YYLAST && yycheck[yystate] == *yyssp)
    yystate = yytable[yystate];
  else
    yystate = yydefgoto[yyn - YYNTBASE];

  goto yynewstate;

yyerrlab:   /* here on detecting error */

  if (! yyerrstatus)
    /* If not already recovering from an error, report this error.  */
    {
      ++yynerrs;

#ifdef YYERROR_VERBOSE
      yyn = yypact[yystate];

      if (yyn > YYFLAG && yyn < YYLAST)
	{
	  int size = 0;
	  char *msg;
	  int x, count;

	  count = 0;
	  /* Start X at -yyn if nec to avoid negative indexes in yycheck.  */
	  for (x = (yyn < 0 ? -yyn : 0);
	       x < (sizeof(yytname) / sizeof(char *)); x++)
	    if (yycheck[x + yyn] == x)
	      size += strlen(yytname[x]) + 15, count++;
	  msg = (char *) malloc(size + 15);
	  if (msg != 0)
	    {
	      strcpy(msg, "parse error");

	      if (count < 5)
		{
		  count = 0;
		  for (x = (yyn < 0 ? -yyn : 0);
		       x < (sizeof(yytname) / sizeof(char *)); x++)
		    if (yycheck[x + yyn] == x)
		      {
			strcat(msg, count == 0 ? ", expecting `" : " or `");
			strcat(msg, yytname[x]);
			strcat(msg, "'");
			count++;
		      }
		}
	      yyerror(msg);
	      free(msg);
	    }
	  else
	    yyerror ("parse error; also virtual memory exceeded");
	}
      else
#endif /* YYERROR_VERBOSE */
	yyerror("parse error");
    }

  goto yyerrlab1;
yyerrlab1:   /* here on error raised explicitly by an action */

  if (yyerrstatus == 3)
    {
      /* if just tried and failed to reuse lookahead token after an error, discard it.  */

      /* return failure if at end of input */
      if (yychar == YYEOF)
	YYABORT;

#if YYDEBUG != 0
      if (yydebug)
	fprintf(stderr, "Discarding token %d (%s).\n", yychar, yytname[yychar1]);
#endif

      yychar = YYEMPTY;
    }

  /* Else will try to reuse lookahead token
     after shifting the error token.  */

  yyerrstatus = 3;		/* Each real token shifted decrements this */

  goto yyerrhandle;

yyerrdefault:  /* current state does not do anything special for the error token. */

#if 0
  /* This is wrong; only states that explicitly want error tokens
     should shift them.  */
  yyn = yydefact[yystate];  /* If its default is to accept any token, ok.  Otherwise pop it.*/
  if (yyn) goto yydefault;
#endif

yyerrpop:   /* pop the current state because it cannot handle the error token */

  if (yyssp == yyss) YYABORT;
  yyvsp--;
  yystate = *--yyssp;
#ifdef YYLSP_NEEDED
  yylsp--;
#endif

#if YYDEBUG != 0
  if (yydebug)
    {
      short *ssp1 = yyss - 1;
      fprintf (stderr, "Error: state stack now");
      while (ssp1 != yyssp)
	fprintf (stderr, " %d", *++ssp1);
      fprintf (stderr, "\n");
    }
#endif

yyerrhandle:

  yyn = yypact[yystate];
  if (yyn == YYFLAG)
    goto yyerrdefault;

  yyn += YYTERROR;
  if (yyn < 0 || yyn > YYLAST || yycheck[yyn] != YYTERROR)
    goto yyerrdefault;

  yyn = yytable[yyn];
  if (yyn < 0)
    {
      if (yyn == YYFLAG)
	goto yyerrpop;
      yyn = -yyn;
      goto yyreduce;
    }
  else if (yyn == 0)
    goto yyerrpop;

  if (yyn == YYFINAL)
    YYACCEPT;

#if YYDEBUG != 0
  if (yydebug)
    fprintf(stderr, "Shifting error token, ");
#endif

  *++yyvsp = yylval;
#ifdef YYLSP_NEEDED
  *++yylsp = yylloc;
#endif

  yystate = yyn;
  goto yynewstate;

 yyacceptlab:
  /* YYACCEPT comes here.  */
  if (yyfree_stacks)
    {
      free (yyss);
      free (yyvs);
#ifdef YYLSP_NEEDED
      free (yyls);
#endif
    }
  return 0;

 yyabortlab:
  /* YYABORT comes here.  */
  if (yyfree_stacks)
    {
      free (yyss);
      free (yyvs);
#ifdef YYLSP_NEEDED
      free (yyls);
#endif
    }
  return 1;
}
#line 803 "grammar.y"

#ifdef MSDOS
#include "lex_yy.c"
#else
#ifdef VMS
#include "lexyy.c"
#else
#include "lex.yy.c"
#endif /* !VMS   */
#endif /* !MSDOS */

#ifdef I_STDARG
void yyerror(const char *format, ...)
#else
void yyerror(va_alist)
    va_dcl
#endif
{
#ifndef I_STDARG
    const char *format;
#endif
    va_list args;

    output_error();

#ifdef I_STDARG
    va_start(args, format);
#else
    va_start(args);
    format = va_arg(args, char *);
#endif

    vfprintf(stderr, format, args);
    va_end(args);
    putc('.',stderr);
    putc('\n',stderr);
}

void
parse_file (start_file)
const char *start_file;
{
    const char *s;
#ifdef FLEX_SCANNER
    static boolean restart = FALSE;
#endif

    cur_file = start_file ? strduplicate(start_file) : NULL;

    if (basefile && strlen(basefile) > 2) {
	s = basefile + strlen(basefile) - 2;
	if (strcmp(s, ".l") == 0 || strcmp(s, ".y") == 0)
	    BEGIN LEXYACC;
    }

    typedef_names = create_symbol_table();
    enum_table = create_symbol_table();

    line_num = 1;
    ly_count = 0;
    first_comment = group_together && !terse_specified;

    /* flex needs a yyrestart before every file but the first */
#ifdef FLEX_SCANNER
    if (restart)	yyrestart(yyin);
    restart = TRUE;
#endif

    yyparse();

    destroy_symbol_table(enum_table);
    destroy_symbol_table(typedef_names);

    safe_free(cur_file);
}
