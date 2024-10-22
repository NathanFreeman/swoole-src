/* Generated by re2c 3.0 */
#line 1 "ext/pdo_sqlite/sqlite_sql_parser.re"
/*
  +----------------------------------------------------------------------+
  | Copyright (c) The PHP Group                                          |
  +----------------------------------------------------------------------+
  | This source file is subject to version 3.01 of the PHP license,      |
  | that is bundled with this package in the file LICENSE, and is        |
  | available through the world-wide-web at the following url:           |
  | https://www.php.net/license/3_01.txt                                 |
  | If you did not receive a copy of the PHP license and are unable to   |
  | obtain it through the world-wide-web, please send a note to          |
  | license@php.net so we can mail you a copy immediately.               |
  +----------------------------------------------------------------------+
  | Author: Matteo Beccati <mbeccati@php.net>                            |
  +----------------------------------------------------------------------+
*/

#include "php_swoole.h"
#if defined(SW_USE_SQLITE) && PHP_VERSION_ID >= 80400

#include "php.h"
#include "ext/pdo/php_pdo_driver.h"
#include "ext/pdo/pdo_sql_parser.h"

int pdo_sqlite_scanner(pdo_scanner_t *s)
{
	const char *cursor = s->cur;

	s->tok = cursor;
	#line 35 "ext/pdo_sqlite/sqlite_sql_parser.re"


	
#line 35 "ext/pdo_sqlite/sqlite_sql_parser.c"
{
	YYCTYPE yych;
	unsigned int yyaccept = 0;
	if ((YYLIMIT - YYCURSOR) < 2) YYFILL(2);
	yych = *YYCURSOR;
	switch (yych) {
		case 0x00: goto yy1;
		case '"': goto yy4;
		case '\'': goto yy6;
		case '-': goto yy7;
		case '/': goto yy8;
		case ':': goto yy9;
		case '?': goto yy10;
		case '[': goto yy12;
		case '`': goto yy13;
		default: goto yy2;
	}
yy1:
	YYCURSOR = YYMARKER;
	switch (yyaccept) {
		case 0: goto yy5;
		case 1: goto yy17;
		case 2: goto yy21;
		case 3: goto yy33;
		default: goto yy37;
	}
yy2:
	++YYCURSOR;
	if (YYLIMIT <= YYCURSOR) YYFILL(1);
	yych = *YYCURSOR;
	switch (yych) {
		case 0x00:
		case '"':
		case '\'':
		case '-':
		case '/':
		case ':':
		case '?':
		case '[':
		case '`': goto yy3;
		default: goto yy2;
	}
yy3:
#line 47 "ext/pdo_sqlite/sqlite_sql_parser.re"
	{ RET(PDO_PARSER_TEXT); }
#line 81 "ext/pdo_sqlite/sqlite_sql_parser.c"
yy4:
	yyaccept = 0;
	yych = *(YYMARKER = ++YYCURSOR);
	if (yych >= 0x01) goto yy15;
yy5:
#line 45 "ext/pdo_sqlite/sqlite_sql_parser.re"
	{ SKIP_ONE(PDO_PARSER_TEXT); }
#line 89 "ext/pdo_sqlite/sqlite_sql_parser.c"
yy6:
	yyaccept = 0;
	yych = *(YYMARKER = ++YYCURSOR);
	if (yych <= 0x00) goto yy5;
	goto yy19;
yy7:
	yych = *++YYCURSOR;
	switch (yych) {
		case '-': goto yy22;
		default: goto yy5;
	}
yy8:
	yych = *++YYCURSOR;
	switch (yych) {
		case '*': goto yy24;
		default: goto yy5;
	}
yy9:
	yych = *++YYCURSOR;
	switch (yych) {
		case '0':
		case '1':
		case '2':
		case '3':
		case '4':
		case '5':
		case '6':
		case '7':
		case '8':
		case '9':
		case 'A':
		case 'B':
		case 'C':
		case 'D':
		case 'E':
		case 'F':
		case 'G':
		case 'H':
		case 'I':
		case 'J':
		case 'K':
		case 'L':
		case 'M':
		case 'N':
		case 'O':
		case 'P':
		case 'Q':
		case 'R':
		case 'S':
		case 'T':
		case 'U':
		case 'V':
		case 'W':
		case 'X':
		case 'Y':
		case 'Z':
		case '_':
		case 'a':
		case 'b':
		case 'c':
		case 'd':
		case 'e':
		case 'f':
		case 'g':
		case 'h':
		case 'i':
		case 'j':
		case 'k':
		case 'l':
		case 'm':
		case 'n':
		case 'o':
		case 'p':
		case 'q':
		case 'r':
		case 's':
		case 't':
		case 'u':
		case 'v':
		case 'w':
		case 'x':
		case 'y':
		case 'z': goto yy25;
		case ':': goto yy27;
		default: goto yy5;
	}
yy10:
	yych = *++YYCURSOR;
	switch (yych) {
		case '?': goto yy29;
		default: goto yy11;
	}
yy11:
#line 44 "ext/pdo_sqlite/sqlite_sql_parser.re"
	{ RET(PDO_PARSER_BIND_POS); }
#line 185 "ext/pdo_sqlite/sqlite_sql_parser.c"
yy12:
	yyaccept = 0;
	yych = *(YYMARKER = ++YYCURSOR);
	if (yych <= 0x00) goto yy5;
	goto yy31;
yy13:
	yyaccept = 0;
	yych = *(YYMARKER = ++YYCURSOR);
	if (yych <= 0x00) goto yy5;
	goto yy35;
yy14:
	++YYCURSOR;
	if (YYLIMIT <= YYCURSOR) YYFILL(1);
	yych = *YYCURSOR;
yy15:
	switch (yych) {
		case 0x00: goto yy1;
		case '"': goto yy16;
		default: goto yy14;
	}
yy16:
	yyaccept = 1;
	YYMARKER = ++YYCURSOR;
	if (YYLIMIT <= YYCURSOR) YYFILL(1);
	yych = *YYCURSOR;
	switch (yych) {
		case 0x00: goto yy17;
		case '"': goto yy16;
		default: goto yy14;
	}
yy17:
#line 38 "ext/pdo_sqlite/sqlite_sql_parser.re"
	{ RET(PDO_PARSER_TEXT); }
#line 219 "ext/pdo_sqlite/sqlite_sql_parser.c"
yy18:
	++YYCURSOR;
	if (YYLIMIT <= YYCURSOR) YYFILL(1);
	yych = *YYCURSOR;
yy19:
	switch (yych) {
		case 0x00: goto yy1;
		case '\'': goto yy20;
		default: goto yy18;
	}
yy20:
	yyaccept = 2;
	YYMARKER = ++YYCURSOR;
	if (YYLIMIT <= YYCURSOR) YYFILL(1);
	yych = *YYCURSOR;
	switch (yych) {
		case 0x00: goto yy21;
		case '\'': goto yy20;
		default: goto yy18;
	}
yy21:
#line 39 "ext/pdo_sqlite/sqlite_sql_parser.re"
	{ RET(PDO_PARSER_TEXT); }
#line 243 "ext/pdo_sqlite/sqlite_sql_parser.c"
yy22:
	++YYCURSOR;
	if (YYLIMIT <= YYCURSOR) YYFILL(1);
	yych = *YYCURSOR;
	switch (yych) {
		case '\n': goto yy23;
		default: goto yy22;
	}
yy23:
#line 46 "ext/pdo_sqlite/sqlite_sql_parser.re"
	{ RET(PDO_PARSER_TEXT); }
#line 255 "ext/pdo_sqlite/sqlite_sql_parser.c"
yy24:
	++YYCURSOR;
	if (YYLIMIT <= YYCURSOR) YYFILL(1);
	yych = *YYCURSOR;
	switch (yych) {
		case '*': goto yy38;
		default: goto yy24;
	}
yy25:
	++YYCURSOR;
	if (YYLIMIT <= YYCURSOR) YYFILL(1);
	yych = *YYCURSOR;
	switch (yych) {
		case '0':
		case '1':
		case '2':
		case '3':
		case '4':
		case '5':
		case '6':
		case '7':
		case '8':
		case '9':
		case 'A':
		case 'B':
		case 'C':
		case 'D':
		case 'E':
		case 'F':
		case 'G':
		case 'H':
		case 'I':
		case 'J':
		case 'K':
		case 'L':
		case 'M':
		case 'N':
		case 'O':
		case 'P':
		case 'Q':
		case 'R':
		case 'S':
		case 'T':
		case 'U':
		case 'V':
		case 'W':
		case 'X':
		case 'Y':
		case 'Z':
		case '_':
		case 'a':
		case 'b':
		case 'c':
		case 'd':
		case 'e':
		case 'f':
		case 'g':
		case 'h':
		case 'i':
		case 'j':
		case 'k':
		case 'l':
		case 'm':
		case 'n':
		case 'o':
		case 'p':
		case 'q':
		case 'r':
		case 's':
		case 't':
		case 'u':
		case 'v':
		case 'w':
		case 'x':
		case 'y':
		case 'z': goto yy25;
		default: goto yy26;
	}
yy26:
#line 43 "ext/pdo_sqlite/sqlite_sql_parser.re"
	{ RET(PDO_PARSER_BIND); }
#line 337 "ext/pdo_sqlite/sqlite_sql_parser.c"
yy27:
	++YYCURSOR;
	if (YYLIMIT <= YYCURSOR) YYFILL(1);
	yych = *YYCURSOR;
	switch (yych) {
		case ':': goto yy27;
		default: goto yy28;
	}
yy28:
#line 42 "ext/pdo_sqlite/sqlite_sql_parser.re"
	{ RET(PDO_PARSER_TEXT); }
#line 349 "ext/pdo_sqlite/sqlite_sql_parser.c"
yy29:
	++YYCURSOR;
	if (YYLIMIT <= YYCURSOR) YYFILL(1);
	yych = *YYCURSOR;
	switch (yych) {
		case '?': goto yy29;
		default: goto yy28;
	}
yy30:
	++YYCURSOR;
	if (YYLIMIT <= YYCURSOR) YYFILL(1);
	yych = *YYCURSOR;
yy31:
	switch (yych) {
		case 0x00: goto yy1;
		case ']': goto yy32;
		default: goto yy30;
	}
yy32:
	yyaccept = 3;
	YYMARKER = ++YYCURSOR;
	if (YYLIMIT <= YYCURSOR) YYFILL(1);
	yych = *YYCURSOR;
	switch (yych) {
		case 0x00: goto yy33;
		case ']': goto yy32;
		default: goto yy30;
	}
yy33:
#line 41 "ext/pdo_sqlite/sqlite_sql_parser.re"
	{ RET(PDO_PARSER_TEXT); }
#line 381 "ext/pdo_sqlite/sqlite_sql_parser.c"
yy34:
	++YYCURSOR;
	if (YYLIMIT <= YYCURSOR) YYFILL(1);
	yych = *YYCURSOR;
yy35:
	switch (yych) {
		case 0x00: goto yy1;
		case '`': goto yy36;
		default: goto yy34;
	}
yy36:
	yyaccept = 4;
	YYMARKER = ++YYCURSOR;
	if (YYLIMIT <= YYCURSOR) YYFILL(1);
	yych = *YYCURSOR;
	switch (yych) {
		case 0x00: goto yy37;
		case '`': goto yy36;
		default: goto yy34;
	}
yy37:
#line 40 "ext/pdo_sqlite/sqlite_sql_parser.re"
	{ RET(PDO_PARSER_TEXT); }
#line 405 "ext/pdo_sqlite/sqlite_sql_parser.c"
yy38:
	++YYCURSOR;
	if (YYLIMIT <= YYCURSOR) YYFILL(1);
	yych = *YYCURSOR;
	switch (yych) {
		case '*': goto yy38;
		case '/': goto yy39;
		default: goto yy24;
	}
yy39:
	++YYCURSOR;
	goto yy23;
}
#line 48 "ext/pdo_sqlite/sqlite_sql_parser.re"

}
#endif
