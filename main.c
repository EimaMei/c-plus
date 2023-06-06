#define STB_C_LEXER_IMPLEMENTATION
#define SI_IMPLEMENTATION

#include "sili.h"

#include "stb_c_lexer.h"

#include <assert.h>

/* shortened function names */
#define strAppend si_string_append
#define strAppendL si_string_append_len


unsigned int indent = 0;

/* 
1: found struct (keyword)
2: inside struct definition
*/
unsigned int structMode; 

siString* print_token(stb_lexer *lexer, siString* c_code) {
  switch (lexer->token) {
    case CLEX_id        : 
          if (si_strings_are_equal(lexer->string, "struct"))
            structMode++;
          
          strAppend(c_code, lexer->string); 
          break;
    case CLEX_eq        : strAppendL(c_code, "==", 2); break;
    case CLEX_noteq     : strAppendL(c_code, "!=", 2); break;
    case CLEX_lesseq    : strAppendL(c_code, "<=", 2); break;
    case CLEX_greatereq : strAppendL(c_code, ">=", 2); break;
    case CLEX_andand    : strAppendL(c_code, "&&", 2); break;
    case CLEX_oror      : strAppendL(c_code, "||", 2); break;
    case CLEX_shl       : strAppendL(c_code, "<<", 2); break;
    case CLEX_shr       : strAppendL(c_code, ">>", 2); break;
    case CLEX_plusplus  : strAppendL(c_code, "++", 2); break;
    case CLEX_minusminus: strAppendL(c_code, "--", 2); break;
    case CLEX_arrow     : strAppendL(c_code, "->", 2); break;
    case CLEX_andeq     : strAppendL(c_code, "&=", 2); break;
    case CLEX_oreq      : strAppendL(c_code, "|=", 2); break;
    case CLEX_xoreq     : strAppendL(c_code, "^=", 2); break;
    case CLEX_pluseq    : strAppendL(c_code, "+=", 2); break;
    case CLEX_minuseq   : strAppendL(c_code, "-=", 2); break;
    case CLEX_muleq     : strAppendL(c_code, "*=", 2); break;
    case CLEX_diveq     : strAppendL(c_code, "/=", 2); break;
    case CLEX_modeq     : strAppendL(c_code, "%%=", 2); break;
    case CLEX_shleq     : strAppendL(c_code, "<<=", 3); break;
    case CLEX_shreq     : strAppendL(c_code, ">>=", 3); break;
    case CLEX_eqarrow   : strAppendL(c_code, "=>", 2); break;
    case CLEX_dqstring  : 
          strAppendL(c_code, "\"", 1); 
          strAppendL(c_code, lexer->string, lexer->string_len); 
          strAppendL(c_code, "\"", 1); 
          break;
    case CLEX_sqstring  : 
          strAppendL(c_code, "\"'", 2); 
          strAppendL(c_code, lexer->string, lexer->string_len); 
          strAppendL(c_code, "\"'", 2); 
    case CLEX_charlit   : 
          strAppendL(c_code, "'", 1); 
          strAppendL(c_code, lexer->string, lexer->string_len); 
          strAppendL(c_code, "'", 1); 
    #if defined(STB__clex_int_as_double) && !defined(STB__CLEX_use_stdlib)
    case CLEX_intlit    : strAppend(c_code, "#%g", lexer->real_number); break;
    #else
    case CLEX_intlit    : 
      //strAppend(c_code, lexer->int_number); break;
    #endif
    case CLEX_floatlit  : 
      //strAppend(c_code, lexer->real_number); break;
    default:
        if (lexer->token >= 0 && lexer->token < 256) {
          switch(lexer->token) {
            case '{':
                if (structMode == 1)
                  structMode++;

                indent++;
                strAppendL(c_code, &lexer->token, 1);
                strAppendL(c_code, "\n", 1);
                break;
            
            case '}':
              if (structMode == 2)
                  structMode--;

              indent--;

              si_string_erase(c_code, si_string_len(*c_code) - 2, 2);
              strAppendL(c_code, &lexer->token, 1);
              strAppendL(c_code, "\n\n", 2);
              break;

            case ';':
                structMode = 0;
                
                if (((*c_code)[si_string_len(*c_code) - 2] == '\n'))
                  si_string_erase(c_code, si_string_len(*c_code) - 2, 2);
                else
                  si_string_erase(c_code, si_string_len(*c_code) - 1, 1);
                  

                strAppendL(c_code, &lexer->token, 1);

                if (((*c_code)[si_string_len(*c_code) - 3] == '\n')) strAppendL(c_code, "\n", 1);
                strAppendL(c_code, "\n", 1);
                break;

            case '.':
              if (si_string_back(*c_code) == ' ')
                si_string_erase(c_code, si_string_len(*c_code) - 1, 1);
              
              strAppendL(c_code, &lexer->token, 1);
              break;
            
            case ')':
              

              strAppendL(c_code, &lexer->token, 1);
              break;

            case '=':
              structMode = false;

              strAppendL(c_code, &lexer->token, 1);
              break;

            default:
              strAppendL(c_code, &lexer->token, 1);
              break;
          }
        }
        else
          printf("<<<UNKNOWN TOKEN %ld >>>\n", lexer->token);
        
        break;
  }

  int i;

  if (si_string_back(*c_code) == '\n' && indent)
    for (i = 0; i < indent * 2; i++) 
      strAppendL(c_code, " ", 1);    
  else if (si_string_back(*c_code) != '\n' && si_string_back(*c_code) != '.')
      strAppendL(c_code, " ", 1);

}

int main(int argc, char **argv) {
  siFile f = si_file_open("test.cp");
  siString text = si_file_read(f);
  si_file_close(f);

  stb_lexer lex;
  siString c_code = si_string_make("");
  stb_c_lexer_init(&lex, text, text + f.size, malloc(0x10000), 0x10000);

  while (stb_c_lexer_get_token(&lex)) {
      if (lex.token == CLEX_parse_error) {
          printf("\n<<<PARSE ERROR>>>\n");
          break;
      }

      print_token(&lex, &c_code);
  }

  f = si_file_create("output.c");
  si_file_write(&f, c_code);
  si_file_close(f);

  si_string_free(c_code);
}