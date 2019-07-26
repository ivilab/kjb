
/**
 * Basic tokenizer, without multiword support
 */

%{

#include <iostream>
#include <vector>
#include <string>

/** Start of the input buffer */
static char * bufferBegin;
/** End of the input buffer */
static char * bufferEnd;

/** Output token vector */
static std::vector<std::string> * tokens;

/** Transfers from input buffer into the lex buffer */
static int splitterTransfer(char * buffer, int size);

/** Finds if this string ends with an apostrophe construction, e.g. 've */
static int hasApostropheBlock(const std::string & s);

static std::string toLower(const std::string & s);

#undef YY_INPUT
#define YY_INPUT(buffer, count, size) (count = splitterTransfer(buffer, size))

%}

%option noyywrap
%option prefix="splitter"

whitespace [ \t\n\r]+
digit      [0-9]
letter     ([a-z])|([A-Z])
upper      [A-Z]
sign       "-"|"\+"
signnum    {sign}{digit}+(("."|","|"/")?{digit}+)*
decnum     "."{digit}+
num        {signnum}|{decnum}
date       {digit}+"/"{digit}+"/"{digit}+ 
time       {digit}+(":"{digit}+)+
letters    {letter}+
blocknum   {digit}+(("."|","|"/")?{digit}+)*
block      {blocknum}|{letters}
word       ("'")?{block}("."?("_"|"-"|"'"|"/")?{block})*
punc       ("\'\'")|("\`\`")|("\-""\-"+)|("\.""\."+)
paren      ("-LRB-")|("-RRB-")|("-LCB-")|("-RRB-")
sgml       "<"([^><])+">"
docstart   "-DOCSTART-"
htmlcode   "\&"{upper}+";"

%%

{whitespace} {
}

{num} {
  // this recognizes special numbers that 
  // start with a dot (decimals) or with a sign (signed)
  tokens->push_back(splittertext);
  return 1;
}

{word} {
  //
  // The word includes alphas, regular numbers (not num,
  //   see above), alphanums, and acronyms
  // IMPORTANT NOTE: the grammar does not separate inner
  //   dashes and dots! Hence the detection of abbreviations
  //   simply has to verify the dot following the word. 
  //

  // split apostrophes followed by letters, e.g. 's, 'm, 're, 've, n't
  std::string buffer = splittertext;
  int block = -1;
  if(buffer.size() > 3 && 
     toLower(buffer.substr(buffer.size() - 3, 3)) == "n't"){
    tokens->push_back(buffer.substr(0, buffer.size() - 3));
    tokens->push_back(buffer.substr(buffer.size() - 3, 3));
  } else if((block = hasApostropheBlock(buffer)) > 0){
    tokens->push_back(buffer.substr(0, block));
    tokens->push_back(buffer.substr(block, buffer.size() - block));
  } else{
    tokens->push_back(splittertext);
  }

  return 1;
}

{date} {
  tokens->push_back(splittertext);
  return 1;
}

{time} {
  tokens->push_back(splittertext);
  return 1;
}

{punc} {
  tokens->push_back(splittertext);
  return 1;
}

{paren} {
  tokens->push_back(splittertext);
  return 1;
}

{sgml} {
  tokens->push_back(splittertext);
  return 1;
}

{docstart} {
  tokens->push_back(splittertext);
  return 1;
}

{htmlcode} {
  tokens->push_back(splittertext);
  return 1;
}

. {
  tokens->push_back(splittertext);
  return 1;
}

%%

static std::string toLower(const std::string & s)
{
  std::string out = s;
  for(unsigned int i = 0; i < out.size(); i ++){
    out[i] = tolower(out[i]);
  }
  return out;
}

/** does it end with "'string"? */
static int hasApostropheBlock(const std::string & s)
{
  for(int i = s.size() - 1; i > 0; i --){
    if(s[i] == '\'' && i < (int) s.size() - 1){
      return i;
    }

    if(! isalpha(s[i])){
      return -1;
    }
  }

  return -1;
}

static int splitterTransfer(char * buffer, int size)
{
  int howMuch = bufferEnd - bufferBegin;
                                       
  if(size < howMuch){
    howMuch = size;
  }
 
  /* The actual transfer */
  if(howMuch > 0){
    memcpy(buffer, bufferBegin, howMuch);
    bufferBegin += howMuch;
  }
                                             
  return howMuch;
}

void unitSplit(const char * inputBuffer, 
               unsigned int inputSize,
               std::vector<std::string> & output)
{
  bufferBegin = (char *) inputBuffer;
  bufferEnd = bufferBegin + inputSize;
  tokens = & output;

  while(splitterlex());
}

