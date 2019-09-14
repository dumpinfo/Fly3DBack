#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <ctype.h>
#include <stdio.h>

#define DELIMITER 1
#define VARIABLE 2
#define NUMBER 3
#define FUNCTION 4

#define Float double

Float _X_=0.0,_Y_=0.0,_SX_=0.0,_SY_=0.0,_UX_=0.0,_UY_=0.0;

char *errors[]={ "Sintax error","Missing parentheses",
                 "No expression present","Undefined function",
                 "Negative number square root !","Divide by 0",
                 "Invalid LN function","Invalid ACOS funcion",
                 "Invalid ASIN function","Invalid '!' operator. Use '!='",
                 "Invalid assignment","Missing ':' operator in '?' statment"
               };


char *prog,token[128],tok_type,equal=0,noquestion=0,nocomp=0;

Float vars[26];

int get_exp(char *exp,Float *result);
char *strupr__(char *s);
void level1(Float *result);
void level2(Float *result);
void level3(Float *result);
void level4(Float *result);
void level5(Float *result);
void level6(Float *result);
void primitive(Float *result);
void get_token(void);
void arith(char o,Float *r,Float *h);
void unary(char o,Float *r);
void serror(int error);
void putback(void);
Float find_var(char *s);
int iswhite(char c);
int isdelim(char c);
Float exec_func(char *func,Float num);

char *strupr__(char *s)
{
     int a;
     for( a=0;s[a]!=0;s[a]=toupper(s[a]),a++ ); 
     return s;
} 

int get_exp(char *exp,Float *result)
{
   int done=0;
   prog=exp;

   get_token();
   if (!*token)
      {
       return 0;
      }
   do {
       level1(result);
       if (*token==',')
          get_token();
       else done=1;
      } while (!done);
   return 1;
}

void level1(Float *result)
{
    Float hold;
    int slot,ttok_type;
    char temp_token[128],op,t1;

    if (tok_type==VARIABLE)
       {
        strcpy(temp_token,token);
        ttok_type=tok_type;
        slot=toupper(*token)-'A';
        get_token();
        if (*token!='=')
           {
            putback();
            tok_type=ttok_type;
            strcpy(token,temp_token);
           }
        else if (*prog!='=')
             {
              get_token();
              level2(result);
              if (slot<0||slot>25)
                 serror(10);
              vars[slot]=*result;
              return;
             }
       }
    level2(result);
    while( ((op=*token)=='<' || op=='>' || op=='!' || op=='=')&&!nocomp )
           {
            if (*prog=='=')
               { equal=1;
                 prog++;
               }
            get_token();
            t1=noquestion;
            noquestion=1;
            level1(&hold);
            if (!t1)
               noquestion=0;
            arith(op,result,&hold);
            equal=0;
           }
    while( *token=='?'&&!noquestion )
           {
            Float hold1,hold2;
            get_token();
            level1(&hold1);
            if (*token!=':')
               serror(11);
            get_token();
            level1(&hold2);
            if (*result)
               *result=hold1;
            else *result=hold2;
           }

}

void level2(Float *result)
{
    char op,t1,t2;
    Float hold;

    level3(result);
    while( (op=*token)=='+' || op=='-' )
           {
            get_token();
            t1=noquestion;
            t2=nocomp;
            noquestion=1;
            nocomp=1;
            level1(&hold);
            if (!t1)
               noquestion=0;
            if (!t2)
               nocomp=0;
            arith(op,result,&hold);
            return;
           }
}

void level3(Float *result)
{
    char op;
    Float hold;

    level4(result);
    while( (op=*token)=='*' || op=='/' || op=='%' )
           {
            get_token();
            level4(&hold);
            arith(op,result,&hold);
           }
}

void level4(Float *result)
{
    Float hold;
    char op;

    level5(result);
    while( *token=='^' )
           {
            get_token();
            level5(&hold);
            arith('^',result,&hold);
           }
    while( *token=='|'||*token=='&' )
           {
            op=*token;
            get_token();
            level5(&hold);
            arith(op,result,&hold);
           }
}

void level5(Float *result)
{
    char op=0;

    if ( (tok_type==DELIMITER) && *token=='+' || *token=='-' )
       {
        op=*token;
        get_token();
       }
    level6(result);
    if (op)
       unary(op,result);
}

void level6(Float *result)
{
    char func[8];

    if ( (*token=='(') && (tok_type==DELIMITER) )
          {
           get_token();
           level1(result);
           if (*token!=')')
              serror(1);
           get_token();
          }
    else if (tok_type==FUNCTION)
            {
             strcpy(func,token);
             get_token();
             get_token();
             level1(result);
             if (*token!=')')
                serror(1);
             get_token();
             *result=exec_func(func,*result);
            }
         else primitive(result);
}

void primitive(Float *result)
{
    switch( tok_type )
            {
             case VARIABLE:
                  *result=find_var(token);
                  get_token();
                  return;
             case NUMBER:
                  {
                   sscanf(token,"%lf",result);
                   get_token();
                  }
                  return;
             default:
                  serror(0);
            }
}

Float exec_func(char *func,Float num)
{
    strupr__(func);

    if (!strcmp(func,"NOT"))
       return !num;

    if (!strcmp(func,"COS"))
       return cos(num);

    if (!strcmp(func,"RND"))
       return (Float)rand()/RAND_MAX*num;

    if (!strcmp(func,"SIN"))
       return sin(num);

    if (!strcmp(func,"TAN"))
       return tan(num);

    if (!strcmp(func,"ACOS"))
       if (num>1.0||num<-1.0)
          { serror(8); return 0; }
       else return acos(num);

    if (!strcmp(func,"ASIN"))
       if (num>1.0||num<-1.0)
          { serror(8); return 0; }
       else return asin(num);

    if (!strcmp(func,"ATAN"))
       return atan(num);

    if (!strcmp(func,"EXP"))
       return exp(num);

    if (!strcmp(func,"LN"))
       if (num<=0.0)
          { serror(6); return 0; }
       else return log(num);

    if (!strcmp(func,"LOG"))
       if (num<=0.0)
          { serror(6); return 0; }
       else return log10(num);

    if (!strcmp(func,"ABS"))
       return fabs(num);

    if (!strcmp(func,"SQR"))
        return num*num; 

    if (!strcmp(func,"SQRT"))
       if (num<0.0)
          { serror(4); return 0; }
       else return sqrt(num);

    serror(3);

    return 0.0;
}

void arith(char o,Float *r,Float *h)
{
    switch( o )
            {
             case '-':
                  *r=*r - *h;
                  break;
             case '+':
                  *r=*r + *h;
                  break;
             case '*':
                  *r=*r * *h;
                  break;
             case '/':
                  if (fabs(*h)<1.0e-10)
                     { *r=*h; break; }
                  *r=*r / *h;
                  break;
             case '%':
                  *r=fmod(*r,*h);
                  break;
             case '^':
                  *r=pow(*r,*h);
                  break;
             case '<':
                  if (equal)
                     *r=(Float)(*r<=*h);
                  else *r=(Float)(*r<*h);
                  break;
             case '>':
                  if (equal)
                     *r=(Float)(*r>=*h);
                  else *r=(Float)(*r>*h);
                  break;
             case '!':
                  if (equal)
                     *r=(Float)(*r!=*h);
                  else serror(9);
                  break;
             case '=':
                  if (equal)
                     *r=(Float)(*r==*h);
                  else serror(10);
                  break;
             case '|':
                  *r=(Float)(((int)*r)||((int)*h));
                  break;
             case '&':
                  *r=(Float)(((int)*r)&&((int)*h));
                  break;
             case ':':
                  serror(11);
                  break;
            }
}

void unary(char o,Float *r)
{
    if (o=='-')
       *r=-(*r);
}

void putback(void)
{
    char *t;

    t=token;
    for( ;*t;t++ )
         prog--;
}

Float find_var(char *s)
{
    strupr__(s);
    if (!isalpha(*s))
       {
        serror(1);
        return 0.0;
       }
    if (!strcmp(s,"X"))
       return _X_;
    if (!strcmp(s,"Y"))
       return _Y_;
    if (!strcmp(s,"SX"))
       return _SX_;
    if (!strcmp(s,"SY"))
       return _SY_;
    if (!strcmp(s,"UX"))
       return _UX_;
    if (!strcmp(s,"UY"))
       return _UY_;
    if (!strcmp(s,"PI"))
       return 3.141592653589793;

    return vars[*token-'A'];
}

void serror(int error)
{
}

void get_token(void)
{
    char *temp;

    tok_type=0;
    temp=token;

    while( iswhite(*prog) )
           ++prog;

    if ( isdelim(*prog) )
       {
        tok_type=DELIMITER;
        *temp++=*prog++;
       }
    else if (isalpha(*prog))
            {
             while( !isdelim(*prog) )
                    *temp++=*prog++;
             if (*prog=='(')
                tok_type=FUNCTION;
             else tok_type=VARIABLE;
            }
         else if (isdigit(*prog)||*prog=='.')
                 {
                  while( !isdelim(*prog) )
                         *temp++=*prog++;
                  tok_type=NUMBER;
                 }
    *temp=0;
}

int iswhite(char c)
{
   if (c==' '||c==9)
      return 1;
   return 0;
}

int isdelim(char c)
{
   if ( strchr("+-*/%^=()<>!?:|&,",c) || c==9 || c=='\r' || c==0 )
      return 1;
   return 0;
}
