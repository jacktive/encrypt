#include <iostream>
#include <string>
#include <cstring>
#include <cstdio>
#include <unistd.h>

#ifdef __WIN32
#include <conio.h>
#include <Windows.h>
#endif // __WIN32
#ifdef __linux
#include <termios.h>
int getch(void) {
    struct termios oldattr, newattr;
    int ch;
    tcgetattr( STDIN_FILENO, &oldattr );
    newattr = oldattr;
    newattr.c_lflag &= ~( ICANON | ECHO );
    tcsetattr( STDIN_FILENO, TCSANOW, &newattr );
    ch = getchar();
    tcsetattr( STDIN_FILENO, TCSANOW, &oldattr );
    return ch;
}
int getche(void) {
    struct termios oldattr, newattr;
    int ch;
    tcgetattr( STDIN_FILENO, &oldattr );
    newattr = oldattr;
    newattr.c_lflag &= ~( ICANON );
    tcsetattr( STDIN_FILENO, TCSANOW, &newattr );
    ch = getchar();
    tcsetattr( STDIN_FILENO, TCSANOW, &oldattr );
    return ch;
}
bool kbhit(void) {
  struct termios oldt, newt;
  int ch;
  int oldf;
  tcgetattr(STDIN_FILENO, &oldt);
  newt = oldt;
  newt.c_lflag &= ~(ICANON | ECHO);
  tcsetattr(STDIN_FILENO, TCSANOW, &newt);
  oldf = fcntl(STDIN_FILENO, F_GETFL, 0);
  fcntl(STDIN_FILENO, F_SETFL, oldf | O_NONBLOCK);
  ch = getchar();
  tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
  fcntl(STDIN_FILENO, F_SETFL, oldf);
  if(ch != EOF)
  {
    ungetc(ch, stdin);
    return true;
  }
  return false;
}
#endif

#include "sha256.h"
#include "endetool.h"

using namespace std;

const char str_help[][128]={
    "Usage: ",
    " [options] [<key> <data>]",
    "<key>=key to be used in encryption/decryption.",
    "<data>=data to be encrypted/decrypted.",
    "Options:",
    "\t-e (--encrypt) to encrypt a data",
    "\t-d (--decrypt) to decrypt a data",
    "\t\tNote that if encrypt and decrypt are used at the same time, it'll basically encrypt and print original.",
    "\t<BROKEN> -w (--write-to-file) to write result to the file.",
    "\t<BROKEN> -r (--read-from-file) to read <key> and <data> from file.",
    "\t\tThese are three files that could be used. 'encrypted.txt','decrypted.txt','key.txt'",
    "\t\tNon-exiting files will be considered empty.",
    "\t-D (--dialog) Launch this program in dialog mode.",
    "\t-B (--no-blocking) to keep this program from blocking",
    "\t-h (--help) for this help page.",
    "You may also call this program with no parameter to launch in dialog mode.",
    "In dialog mode, you can NOT use any <key> or <data> with multiple lines."
};
const char str_dialog[][128]={
    "Input:",
    "encrypt",
    "decrypt",
    "encrypt and decrypt",
    "Welcome to dialog mode! I'll let you read the help before we process.\n",
    "\nPhew, That was the usage. Note: it's quite handy to use parameters. :D\n"
    "Anyway, let's get started.\n",
    "What Are you going to do? Encrypt or Decrypt.\n\tencrypt=1 | decrypt=2\n",
    "Opps... I have no idea what you're talking about...\n",
    "Right! You want to ",
    "Next, what data you want to ",
    "Okay, you wanna ",
    " this data!\n",
    "Then, next.\nWhat is your key?\n",
    "Okay! All's set! I'll give you the result soon enough!\n",
    "Thanks for using our dialog mode!\n"
};

string decrypted, encrypted;
string key_str;
string prgm_name;

namespace flags {
    bool dialogMode;
    bool encrypt;
    bool decrypt;
    bool writeToFile;
    bool readFromFile;
    bool noBlocking;
    bool helpPage;
}

void help(bool tab=false) {
    cout << (!tab?'\0':'\t') << str_help[0] << prgm_name << str_help[1] << endl;
    for(int i=2; i<(sizeof(str_help)/128); i++)
        cout << (!tab?'\0':'\t') << str_help[i] << endl;
}

void d_pause() {
    const char paused[]="Press 'A' to continue";
    static int blink=0;
    while(!kbhit()) {
        for(int i=0; i<sizeof(paused); i++) {
            if(i!=blink)
                putchar(paused[i]);
            else
                putchar(' ');
            if(blink>=sizeof(paused)-1)
                blink=0;
        }
        putchar('\r');
        blink++;
        Sleep(66);
    }
    getch();
    putchar('\r');
    for(int i=0; i<sizeof(paused); i++)
        putchar(' ');
    putchar('\r');
}

void dialog() {
    int select=0;
    cout << str_dialog[4];
    d_pause();
    help(true);
    cout << str_dialog[5];
    d_pause();
again:
    cout << str_dialog[6] << str_dialog[0];
    cin >> select;
    if(select!=1 && select!=2 && select!=3) {
        cout << str_dialog[7];
        getchar();
        goto again;
    }
    if(select==1 || select==3)
        flags::encrypt=true;
    if(select==2 || select==3)
        flags::decrypt=true;
    cout << str_dialog[8] << str_dialog[select] << '!' << '\n';
    d_pause();
    cout << str_dialog[9] << str_dialog[select] << '?' << '\n' << str_dialog[0];
    cin.sync();
    getline(cin, encrypted);
    decrypted=encrypted;
    cout << str_dialog[10] << str_dialog[select] << str_dialog[11];
    d_pause();
    cout << str_dialog[12] << str_dialog[0];
    cin.sync();
    getline(cin, key_str);
    cout << str_dialog[13];
    d_pause();
    cout << str_dialog[14];
    d_pause();
}

void pharse(int argc, char **argv) {
    for(int i=1; i<argc-2; i++) {
        if(argv[i][0]=='-' && argv[i][1]!='-') {
            for(int j=0; argv[i][j]!=0; j++) {
                switch(argv[i][j]) {
                case 'e':
                    flags::encrypt=true;
                    break;
                case 'd':
                    flags::decrypt=true;
                    break;
                case 'w':
                    flags::writeToFile=true;
                    break;
                case 'r':
                    flags::readFromFile=true;
                    break;
                case 'D':
                    flags::dialogMode=true;
                    break;
                case 'B':
                    flags::noBlocking=true;
                    break;
                default:
                    flags::helpPage=true;
                }
            }
        }
        else if(strcmp(argv[i], "--encrypt")==0)
            flags::encrypt=true;
        else if(strcmp(argv[i], "--decrypt")==0)
            flags::decrypt=true;
        else if(strcmp(argv[i], "--write-to-file")==0)
            flags::writeToFile=true;
        else if(strcmp(argv[i], "--read-from-file")==0)
            flags::readFromFile=true;
        else if(strcmp(argv[i], "-dialog")==0)
            flags::dialogMode=true;
        else if(strcmp(argv[i], "--no-blocking")==0)
            flags::noBlocking=true;
        else
            flags::helpPage=true;
    }
    if(flags::readFromFile) {
        if(strcmp(argv[0], "--encrypt")==0)
            flags::encrypt=true;
        else if(strcmp(argv[0], "--decrypt")==0)
            flags::decrypt=true;
        else if(strcmp(argv[0], "--write-to-file")==0)
            flags::writeToFile=true;
        else if(strcmp(argv[0], "--read-from-file")==0)
            flags::readFromFile=true;
        else if(strcmp(argv[0], "-dialog")==0)
            flags::dialogMode=true;
        else if(strcmp(argv[0], "--no-blocking")==0)
            flags::noBlocking=true;
        else
            flags::helpPage=true;
        if(strcmp(argv[1], "--encrypt")==0)
            flags::encrypt=true;
        else if(strcmp(argv[1], "--decrypt")==0)
            flags::decrypt=true;
        else if(strcmp(argv[1], "--write-to-file")==0)
            flags::writeToFile=true;
        else if(strcmp(argv[1], "--read-from-file")==0)
            flags::readFromFile=true;
        else if(strcmp(argv[1], "-dialog")==0)
            flags::dialogMode=true;
        else if(strcmp(argv[1], "--no-blocking")==0)
            flags::noBlocking=true;
        else
            flags::helpPage=true;
    }
}

unsigned int filesize(FILE *file) {
    unsigned int ret;
    fseek(file, 0, SEEK_END);
    ret=ftell(file);
    fseek(file, 0, SEEK_SET);
    return ret;
}

void fileOperation(bool write=false) {
    uint8_t *buffer;
    FILE *file;
    if(!write) {
        if(file=fopen("encrypted.txt", "rb")) {
            unsigned int size=filesize(file);
            buffer=new uint8_t[size+1];
            size=fread((void*)&buffer, 1, size+1, file);
            for(int i=0; i<size; i++)
                encrypted.push_back(buffer[i]);
            delete buffer;
            fclose(file);
        }
        if(file=fopen("decrypted.txt", "rb")) {
            unsigned int size=filesize(file);
            buffer=new uint8_t[size+1];
            size=fread((void*)&buffer, 1, size+1, file);
            for(int i=0; i<size; i++)
                decrypted.push_back(buffer[i]);
            delete buffer;
            fclose(file);
        }
        if(file=fopen("key.txt", "rb")) {
            unsigned int size=filesize(file);
            buffer=new uint8_t[size+1];
            size=fread((void*)&buffer, 1, size+1, file);
            for(int i=0; i<size; i++)
                key_str.push_back(buffer[i]);
            delete buffer;
            fclose(file);
        }
    } else {
        file=fopen("encrypted.txt", "wb");
        fwrite(encrypted.c_str(), 1, encrypted.size(), file);
        fclose(file);
        file=fopen("decrypted.txt", "wb");
        fwrite(decrypted.c_str(), 1, decrypted.size(), file);
        fclose(file);
    }
}

void encrypt() {
    EnDeTool endetool;
    SHA256 sha256;
    sha256(key_str);
    endetool.cryptkey(sha256.getHash().c_str());
    endetool.text(decrypted.c_str());
    encrypted=endetool.encodedtext();
    encrypted.erase(encrypted.end()-1);
    encrypted.erase(encrypted.end()-1);
    cout << "Encrypted: " << encrypted << endl;
}

void decrypt() {
    encrypted.push_back('=');
    encrypted.push_back('=');
    EnDeTool endetool;
    SHA256 sha256;
    sha256(key_str);
    endetool.cryptkey(sha256.getHash().c_str());
    endetool.encodedtext(encrypted.c_str());
    decrypted=endetool.text();
    cout << "Decrypted: " << decrypted << endl;
}

int main(int argc, char **argv) {
    cout << "Loading...";
    encrypted="";
    decrypted="";
    key_str="";
    #ifdef __WIN32
    prgm_name="encrypt.exe";
    #else
    prgm_name="encrypt";
    #endif
    if(argc==1)
        flags::dialogMode=true;
    else {
        pharse(argc, argv);
        if(flags::helpPage) {
            help();
            return 0;
        }
        if(!flags::readFromFile) {
            key_str=argv[argc-2];
            encrypted=argv[argc-1];
            decrypted=argv[argc-1];
        }
    }
    cout << '\r' << "          " << '\r';
    if(flags::dialogMode)
        dialog();
    if(flags::encrypt)
        encrypt();
    if(flags::decrypt)
        decrypt();
    if(flags::writeToFile)
        fileOperation(true);
    if(!flags::noBlocking)
        d_pause();
    return 0;
}
