#ifndef _TERMINAL_H
#define _TERMINAL_H

#include <cstdio>
#include <cstdint>
#include <termios.h>

#include <iostream>
#include <vector>
#include <set>
#include <string>

#include "cmd_line_history.h"

using namespace std;


/*
//清除屏幕
#define CLEAR() fprintf(stderr," 033[2J")

//上移光标
#define MOVEUP(x) printf("\033[%dA", (x))

//下移光标
#define MOVEDOWN(x) printf("\033[%dB", (x))

//定位光标
#define MOVETO(x,y) fprintf(stderr," 033[%d;%dH", (x), (y))

//光标复位
#define RESET_CURSOR() fprintf(stderr," 033[H")

//隐藏光标
#define HIDE_CURSOR() fprintf(stderr," 033[?25l")

//显示光标
#define SHOW_CURSOR() fprintf(stderr," 033[?25h")

//反显
#define HIGHT_LIGHT() fprintf(stderr," 033[7m")
#define UN_HIGHT_LIGHT() fprintf(stderr," 033[27m")

#define KEY_ESC     (27)
// ->  279165
// <-  279166
*/

class Terminal
{
public:
    static void ClearLine()
    {
        printf("\33[2K\r");
        rewind(stdout);
    }

    //左移光标
    static void MoveCursorLeft(int bit)
    {
        printf("\033[%dD", bit);
    }

    //右移光标
    static void MoveCursorRight(int bit)
    {
        printf("\033[%dC", bit);
    }

    void SetPromptStr(const char* text)
    {
        _prompt_str = text;
    }

    std::string ReadLine()
    {
        Reset();

        while( (ch = Getchar()) != '\n' ){
            //printf("%d", ch);

            if(ch == 127) {
                DelKeyEvent();
            } else if(ch == 27 ) {
                is_direct_key = true;
                escnt = 0;
            } else if(is_direct_key){
                escnt++;
                DirectKeyEvent();
            } else{
                NormalKeyEvent(ch);
            }

            chold = ch;
        }

        printf("\n");
        _history.Append(input_string);
        return input_string;
    }

private:
    int Getchar(void)
    {
        //return getchar();
        struct termios oldt,newt;
        int ch;
        tcgetattr( STDIN_FILENO, &oldt );
        newt = oldt;
        newt.c_lflag &= ~( ICANON | ECHO );
        tcsetattr( STDIN_FILENO, TCSANOW, &newt );
        ch = getchar();
        tcsetattr( STDIN_FILENO, TCSANOW, &oldt );
        return ch;
    }

    void Reset()
    {
        is_direct_key = false;
        escnt = 0;

        memset(input_string, 0, sizeof(input_string));
        chold = 0;
        ch_index = max_index = 0;
    }

    void ReshowHistory(const std::string& text)
    {
        memset(input_string, 0, sizeof(input_string));  
        text.copy(input_string, text.size());

        ch_index = text.size();
        max_index = text.size();  

        ClearLine();
        Echo();
    }

    void Echo()
    {
        printf("%s%s", _prompt_str.c_str(), input_string);  
    }

    void DirectKeyEvent() 
    {
        if(escnt >= 2)  {  
            if(chold == 91)  {
                switch(ch)  {
                case 65: // KEY_UP
                    {
                        const std::string& prev_cmd = _history.Pre();
                        if(!prev_cmd.empty())
                            ReshowHistory(prev_cmd);
                    }
                    is_direct_key = false;
                    break;
                case 66: // KEY_DOWN
                    {
                        const std::string& next_cmd = _history.Next();
                        if(!next_cmd.empty())
                            ReshowHistory(next_cmd);
                    }
                    is_direct_key = false;
                    break;
                case 67:  // KEY_RIGHT
                    if(ch_index < max_index)  {
                        MoveCursorRight(1);
                        ++ch_index;
                    }

                    is_direct_key = false;
                    break;
                case 68:  // KEY_LEFT
                    if(ch_index > 0)  {
                        MoveCursorLeft(1);
                        --ch_index;
                    }

                    is_direct_key = false;
                    break;
                default:
                    break;
                }
            }
            else if( chold == 49 && ch == 126){
                MoveToHeader();
                is_direct_key = false;
            }
            else if( chold == 52 && ch == 126){
                EndKeyEvent();
                is_direct_key = false;
            }
        }
    }

    void DelKeyEvent()
    {
        if(ch_index > 0) {
            for(int i = ch_index-1; i <= max_index-1; i++) 
                input_string[i] = input_string[i+1]; 

            ch_index--;
            input_string[--max_index] = '\0';
        }

        ClearLine();
        Echo();

        // reset cursor
        if( max_index > ch_index )
            MoveCursorLeft(max_index - ch_index);
    }

    void EndKeyEvent()
    {
        MoveToEnd();
    }

    void MoveToHeader()
    {
        if( ch_index !=0 && max_index != 0){
            MoveCursorLeft(ch_index);
            ch_index = 0;
        }
    }

    void MoveToEnd()
    {
        if( max_index > ch_index )
            MoveCursorRight(max_index - ch_index);

        ch_index = max_index;
    }

    void NormalKeyEvent(int ch)
    {
        printf("%c",ch);

        if(ch_index >= max_index)  {
            input_string[ch_index] = ch;
            max_index = ++ch_index;
        } else {
            for(int i = max_index; i > ch_index; i--)  
                input_string[i] = input_string[i-1];  

            input_string[ch_index] = ch;  
            max_index++;  
            ch_index++;  

            ClearLine();
            Echo();
            MoveCursorLeft(max_index-ch_index);  
        }
    }

private:
    CommandLineHistory _history;

    char input_string[256];
    int  ch,chold;
    int ch_index = 0,max_index = 0;

    bool is_direct_key = false;
    int escnt = 0;

    std::string _prompt_str;
};

#endif 
