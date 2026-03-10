/***********************************************************************
 *  File name   : sample.c
 *  Description : a sample C program to demonstrate
 *                the working of the lexical analyser.
************************************************************************/

#include<stdio.h>
void sample()
{
    char option;
    int num1, num2;
    int result;
    printf("Enter 2 nos: ");
    scanf("%d %d", &num1, &num2);
    printf("Enter '+' for addition\n '-' for subraction\n '*' for multiplication\n '/' for division: ");
    scanf(" %c", &option);
    switch (option)//This is a single line comment
    {/*MUlti
         line
         commnet */
        case '+':
        result = num1 + num2;
        printf("Ans: %d",result);
        break;
        case '-':
        result = num1 - num2;
        printf("Ans: %d", result);
        break;
        case '*':
        result = num1 * num2;
        printf("Ans: %d",result);
        break;
        case '/':
        result = num1 / num2;
        printf("Ans: %d", result);
        break;
        default:
        printf("None of the options");
    }
    return;

}
