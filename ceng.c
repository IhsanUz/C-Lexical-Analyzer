#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

// 2017510030 Ahmet Efendioglu
// 2017510078 İhsan Batuhan Uz
// 2017510100 Mesut Selim Serbes

bool EndOfLine = true; // true when new line occurs.
int leftParCounter = 0; // number of the left parentheses. 
int LeftSquareBracketCounter = 0; // number of left square brackets.
int LeftCurlyBracketCounter = 0; // number of left curly brackets.
bool keywordBracketCheck = false; // checks 'while', 'if' and 'for' keywords occurs.
bool keywordRightBracketCheck = false; // checks right brackets occurs after left bracket for keywords.
bool keywordCurlyBracketCheck = false; // checks right brackets occurs.


bool isKeyword(char lexeme[])
{
    bool check = false; // is string a keyword.
    // keywords
    char keywords[18][10] = {"break", "case", "char", "const", "continue", "do", "else", "enum", "float", "for", "goto",
    "if", "int", "long", "record", "return", "static", "while"};

    for (int i = 0; i < 18; i++)
    {
        if ((strcasecmp(keywords[i],lexeme)) == 0) // comparing string that comes from file and keywords.
        {
            // special keywords that require brackets after themself.
            if((strcasecmp("while",lexeme))==0 || (strcasecmp("for",lexeme))==0 || (strcasecmp("if",lexeme))==0){
                keywordBracketCheck = true; 
            }
            check = true;
            break;
        }
    }
    return check;
}

void isNumber(char ch, FILE *fp,FILE *fw) { 
    // given character is a number or not. If it is a number check the following characters.
    //until finding a character that is not number.
    char lexeme[15] = {0};
    int i = 0;

    while(isdigit(ch))
    {
        lexeme[i] = ch;
        ch = fgetc(fp);
        if(ch==EOF)
        {
            break;
        }
        if(strlen(lexeme) > 10)
        {
            fprintf(fw,"Error! Maximum integer size is not more than 10 digits.\n");
            break;
        }
        i++;
    }
    fseek(fp,-1,SEEK_CUR);
    
    if(strlen(lexeme) > 10) // if length of lexeme is over 10, read rest of the number.
    {
        while(isdigit(ch))
        {
            ch = fgetc(fp);
        }
    }
    if(isalpha(ch)){ // when an alphabetic character occurs after number. This is a error situation.
        fprintf(fw,"Error! Identifier cannot start with digit.\n");
        while((isalnum(ch) || ch == '_'))
        {
            ch=fgetc(fp);
            i++;
        }
    }
    if(strlen(lexeme) <= 10) // write the lexeme to output file if length of lexeme is under 10.
    {
        fprintf(fw,"IntConst(%s)\n", lexeme);
    }
}

int isIdentifier(char ch, FILE *fp,FILE *fw)
{
    char lexeme[25] = {0};
    int i = 0;

    while((isalnum(ch) || ch == '_'))
    {
        lexeme[i] = ch;
        ch=fgetc(fp);
        if(strlen(lexeme) > 20)
        {
            fprintf(fw, "Error! Maximum identifier size is not more than 20 digits.\n");
            break;
        }
        i++;
    }

    for(int j=0; j<strlen(lexeme);j++){
        lexeme[j]=tolower(lexeme[j]);
    }

    if(strlen(lexeme) > 20) // if length of lexeme is over 10, read rest of the string.
    {
        while((isalnum(ch) || ch == '_'))
        {
            ch = fgetc(fp);
        }
    }
    else if(isKeyword(lexeme)) // checks the given string is a keyword or not.
    {
        fprintf(fw,"Keyword(%s)\n", lexeme);
    }
    else
    {
        fprintf(fw,"Identifier(%s)\n", lexeme);
    }

}

void isBracket(char ch, FILE *fp,FILE *fw)
{
    if (ch == '(')
    {
        if(fgetc(fp)== '*')
        {
            isComment(fp, fw);
        }
        else
        {
            if(keywordBracketCheck){ // after a keyword occurs, first character should be '('.
                keywordBracketCheck = false;
                keywordRightBracketCheck = true; // right bracket should be next for keyword situation.
            }
            leftParCounter++;
            fprintf(fw,"LeftPar\n");
            fseek(fp,-1,SEEK_CUR);
        }
    }
    else if (ch == ')'){
        if(keywordRightBracketCheck){
            keywordCurlyBracketCheck = true; // when right bracket occurs. Next should be curly left bracket for keyword situation.
        }
        fprintf(fw,"RightPar\n");
        leftParCounter--;
    }
    else if (ch == '['){
        fprintf(fw,"LeftSquareBracket\n");
        LeftSquareBracketCounter++;
    }
    else if (ch == ']'){
        fprintf(fw,"RightSquareBracket\n");
        LeftSquareBracketCounter--;
    }
    else if (ch == '{'){
        if (keywordCurlyBracketCheck){
            keywordCurlyBracketCheck = false;
        }
        
        fprintf(fw,"LeftCurlyBracket\n");
        LeftCurlyBracketCounter++;
    }
    else if (ch == '}'){
        fprintf(fw,"RightCurlyBracket\n");
        LeftCurlyBracketCounter--;
    }

}

void isComment(FILE *fp, FILE *fw) // for comments.
{
    int flag = 1;
    while(flag)
    {
        char ch = fgetc(fp);
        if(fgetc(fp) == ')' && ch == '*')
        {
            flag = 0;
            fgetc(fp);
        }
        else if (ch == EOF)
        {
            fprintf(fw, "Error! End of file for comment.");
            flag = 0;
        }
        fseek(fp,-1,SEEK_CUR);
    }
}

void isStringConst(FILE *fp,FILE *fw)
{
    char *lexeme = (char *)malloc(100);

    int i = 0;
    char ch = fgetc(fp);
    while(ch != '"') // String constants starts with '"' and tries to find other '"'.
    {
        lexeme[i] = ch;
        ch = fgetc(fp);
        i++;
        if (ch == EOF)
        {
            fprintf(fw, "Error! End of file for string.");
            break;
        }
    }
    if(ch != EOF)
    {
        fprintf(fw, "StringConst(%s)\n", lexeme);
    }
}

void isOperator(char ch, FILE *fp,FILE *fw)
{
    if(ch=='-' || ch=='+'|| ch=='/'|| ch=='*'|| ch==':')
    {
        if(ch=='+')
        {
            if(fgetc(fp)=='+')
                fprintf(fw,"Operator(++)\n");
            else
            {
                if(EndOfLine){ // after ';', new line cannot start with operator except ++ and --.
                    fprintf(fw,"Error! Line cannot start with operator except ++ and --.\n");
                }else{
                    fprintf(fw, "Operator(%c)\n",ch);
                }
                EndOfLine= false;
                fseek(fp,-1,SEEK_CUR);
            }
        }

        else if(ch=='-')
        {
            if(fgetc(fp)=='-')
                fprintf(fw,"Operator(--)\n");
            else
            {
                if(EndOfLine){
                    fprintf(fw,"Error! Line cannot start with operator except ++ and --.\n");
                }else{
                    fprintf(fw, "Operator(%c)\n",ch);
                }
                EndOfLine= false;
                fseek(fp,-1,SEEK_CUR);
            }
        }
        else if(ch==':')
        {
            if(EndOfLine){
                fprintf(fw,"Error! Line cannot start with operator except ++ and --.\n");
            }else{
                if(fgetc(fp)=='=')
                    fprintf(fw,"Operator(:=)\n");
                else
                {
                    fseek(fp,-1,SEEK_CUR);
                }
            }
            
        }
        else if(ch=='/' || ch== '*')
        {
            if(EndOfLine){
                fprintf(fw,"Error! Line cannot start with operator except ++ and --.\n");
            }else{
                fprintf(fw,"Operator(%c)\n",ch);
            }
        }
        else
        {
            fseek(fp,-1,SEEK_CUR);
        }
    }
}

int main()
{
    FILE *fp; //file pointer for read file.
    FILE *fw; //file pointer for write file.

    fw=fopen("code.lex","w+"); // write file

    if((fp = fopen("code.Ceng", "r")) == NULL) // open the file and check if file exists.
    {
        fprintf(fw,"Could not open the file.");
        exit(0);

    }
    else
    {
        while(!feof(fp))
        {

            char ch = fgetc(fp); 
            if(keywordBracketCheck && ch != '(' && ch != ' '){ // checks next character is '(' for keywords.
                fprintf(fw,"Error! for, while and if keywords should continue with left parentheses.\n");
                keywordBracketCheck = false;
            }
            if(keywordCurlyBracketCheck && ch != '{' && ch != ' '){ // checks next character is '{' for keywords.
                fprintf(fw,"Error! for, while and if statements should have left curly brackets.\n");
                keywordCurlyBracketCheck = false;
            }
            if (isalpha(ch))
            {
                EndOfLine = false;
                isIdentifier(ch, fp,fw);
                fseek(fp,-1,SEEK_CUR);
            }
            else if(isdigit(ch))
            {
                EndOfLine = false;
                isNumber(ch, fp,fw);
            }
            else if(ch == '"')
            {
                isStringConst(fp,fw);
                EndOfLine = false;
            }
            else if (ch == ';')
            {
                fprintf(fw,"EndOfLine\n");
                EndOfLine = true;
            }
            else
            {
                isBracket(ch, fp,fw);
                isOperator(ch,fp,fw);
            }
            if(fgetc(fp)==EOF){
                // Control of number of parentheses.
                if(leftParCounter != 0){ 
                    fprintf(fw,"Error! Number of left parentheses and right parentheses do not match.\n");
                }
                if(LeftSquareBracketCounter != 0){
                    fprintf(fw,"Error! Number of left square bracket and right square bracket do not match.\n");
                }
                if(LeftCurlyBracketCounter != 0){
                    fprintf(fw,"Error! Number of left curly bracket and right curly bracket do not match.\n");
                }
                break;
            }
            else
            {
                fseek(fp,-1,SEEK_CUR);
            }

        }
    }
    fclose(fw);
    fclose(fp);
    return 0;
}