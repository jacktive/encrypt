#include <iostream>
#include <cstring>
#include <cstdio>
#include <string>

#include "sha256.h"
#include "endetool.h"

using namespace std;

const char str_help[][128]={
    "Usage: encrypt [options] [<key> <data>]",
    "<key>=key to be used in encryption/decryption.",
    "<data>=data to be encrypted/decrypted.",
    "Options:",
    "\t-e (--encrypt) to encrypt a data",
    "\t-d (--decrypt) to decrypt a data",
    "\t\tNote that if encrypt and decrypt are used at the same time, it'll essentially encrypt.",
    "\t-f (--write-to-file) to write result to the file.",
    "\t-F (--read-from-file) to read <key> and <data> from file.",
    "\t\tThese are three files that could be used. 'encrypted.txt','decrypted.txt','key.txt'",
    "\t\tNon-exiting files will be considered empty.",
    "\t-B (--no-blocking) to keep this program from blocking",
    "\t-h (--help) for this help page.",
    "You may also call this program with no parameter to launch in dialog mode.",
    "In dialog mode, you can NOT use any <key> or <data> with spaces."
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
namespace flags {
    bool dialogMode;
    bool encrypt;
    bool decrypt;
    bool writeToFile;
    bool readFromFile;
    bool blocking=true;
    bool helpPage;
}

void help(bool tab=false) {
    for(int i=0; i<(sizeof(str_help)/128); i++)
        cout << (!tab?'\0':'\t') << str_help[i] << endl;
}

#define d_pause if(flags::blocking)getchar()
void dialog() {
    int select=0;
    cout << str_dialog[4];
    d_pause;
    help(true);
    cout << str_dialog[5];
    d_pause;
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
    d_pause;
    cout << str_dialog[9] << str_dialog[select] << '?' << '\n' << str_dialog[0];
    cin >> encrypted;
    decrypted=encrypted;
    cout << str_dialog[10] << str_dialog[select] << str_dialog[11];
    d_pause;
    cout << str_dialog[12] << str_dialog[0];
    cin >> key_str;
    cout << str_dialog[13];
    d_pause;
    cout << str_dialog[14];
    d_pause;
}

void pharse(int argc, char **argv) {
    for(int i=1; i<argc; i++) {
        if(strcmp(argv[i], "-e")==0 || strcmp(argv[i], "--encrypt")==0)
            flags::encrypt=true;
        else if(strcmp(argv[i], "-d")==0 || strcmp(argv[i], "--decrypt")==0)
            flags::decrypt=true;
        else if(strcmp(argv[i], "-f")==0 || strcmp(argv[i], "--write-to-file")==0)
            flags::writeToFile=true;
        else if(strcmp(argv[i], "-F")==0 || strcmp(argv[i], "--read-from-file")==0)
            flags::readFromFile=true;
        else if(strcmp(argv[i], "-B")==0 || strcmp(argv[i], "--no-blocking")==0)
            flags::blocking=false;
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
    cout << "Encrypted: " << encrypted << endl;
}

void decrypt() {
    EnDeTool endetool;
    SHA256 sha256;
    sha256(key_str);
    endetool.cryptkey(sha256.getHash().c_str());
    endetool.encodedtext(encrypted.c_str());
    decrypted=endetool.text();
    cout << "Decrypted: " << decrypted << endl;
}

int main(int argc, char **argv) {
    if(argc==1) {
        flags::dialogMode=true;
        dialog();
    }
    else {
        pharse(argc, argv);
        if(flags::helpPage) {
            help();
            return 1;
        }
        if(!flags::readFromFile) {
            key_str=argv[argc-2];
            encrypted=argv[argc-1];
            decrypted=argv[argc-1];
        }
    }
    if(flags::encrypt)
        encrypt();
    if(flags::decrypt)
        decrypt();
    return 0;
}
