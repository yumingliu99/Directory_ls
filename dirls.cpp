//
// Created by Yuming Liu on 5/4/22.
//
#include <sys/stat.h> // include this to check the type of file.
#include <stdio.h>
#include <getopt.h>
#include<iostream>
#include <dirent.h>
#include <string>
#include <limits.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <pwd.h>
#include <grp.h>
#include <sys/types.h>


using namespace std;

//TODO: overall structure -> getopt updates the combination of flags.
// depending on the combination, choose the right function at the end.

//TODO: near the end,
// Make sure to create a struct/class to hold the various arguments in combination and pass in the struct to your function that navigates the directory recursion.
struct dirls{
    bool a, d, f, l, h;
};

void manual();
//Function to check if a specified file is a file or directory
bool isDirectory(const char* pathname);

bool symbolicLinks(const char * path, dirls &flags);

int count_chars(const char* array);

void longForm(const char* pathname);

void generalNavigation(const char* pathname, dirls &flags);

void fNavigation(const char* pathname, dirls &flags);

/*
[harazduk@storm cs3595]$ dirls -h
Usage: dirls [(-[adflh]+) (dir)]*
        -a: include dot files
        -f: follow symbolic links
        -d: only this directory
        -l: long form
        -h: prints this message
 */

int main(int argc, char *argv[]){

    dirls flags = {.a = false, .d = false, .f = false, .l = false, .h = false};
    char* dirPath = new char[256];
    int opt; //this is just a holder for return value of getopt
    int counter = 1;
    int size = argc;

    while (counter < size) {
        if (argv[counter][0] != '-'){
            dirPath = argv[counter];
        }
        counter++;
    }
    while ((opt = getopt(argc, argv, "afdlh")) !=
           -1) { //TODO: with this while loop, once the first flag ends, won't start again. need to write iteration for all of argv
        // TODO: cant figure this out rn, how to allow all the getopt options --> figure it out later after implementing the functions.
        // second flag after a folder name does not get picked up by this loop.
        switch (opt) {
            case 'a':
                flags.a = true;
                break;
            case 'f':
                flags.f = true;
                break;
            case 'd':
                flags.d = true;
                break;
            case 'l':
                flags.l = true;
                break;
            case 'h':
                manual();
                break;
        }
    }

    if (flags.f){
        fNavigation(dirPath, flags);
    }
    else{
        generalNavigation(dirPath, flags);
    }

}

void manual(){
    printf("Usage: dirls [(-[adflh]+) (dir)]* \n-a: include dot files \n-f: follow symbolic links \n-d: only this directory \n-l: long form \n-h: prints this message\n");
}

//Function to check if a specified file is a file or directory
bool isDirectory(const char* pathname){
    struct stat type;
    if(stat(pathname, &type) != 0){
        cout << "";
    }
    return S_ISDIR(type.st_mode);
}

//TODO: write -f function, which handles following symbolic links (to best of my knowledge
// this means to print out absolute path?
bool symbolicLinks(const char * path, dirls &flags){
    struct stat info;
    char *resolved_path = new char;
    if (lstat(path, &info) != 0){
        cout << "";
    }
    else {
        //entered here
        if (S_ISLNK(info.st_mode)) { // but not into this if statement
            printf("Symbolic Link: ");
            realpath(path, resolved_path);
            if (flags.l) {
                cout << path << " -> ";
                longForm(resolved_path);
            }
            else cout << path << " -> " << resolved_path << endl;
            return true;
        }
    }
    return false;
}
//count size of char
int count_chars(const char* array){
    int i = 0;
    while (array[i] != '\0'){
        i++;
    }
    return i;
}

// -l implemented by Roger
void longForm(const char* pathname){
    struct stat info;
    struct passwd pwd;
    struct passwd* pwdptr = &pwd;
    struct passwd* tempPwdPtr;
    char pwdbuffer[200];
    int pwdlinelen = sizeof(pwdbuffer);
    struct group grp;
    struct group *grpptr = &grp;
    struct group *tempGrpPtr;
    char grpbuffer[200];
    int grplinelen = sizeof(grpbuffer);

    if((getpwnam_r("MYUSER", pwdptr, pwdbuffer, pwdlinelen, &tempPwdPtr)) != 0)
        perror("getpwnam_r() error");

    if((getpwuid_r(info.st_uid, pwdptr, pwdbuffer, pwdlinelen, &tempPwdPtr)) != 0)
        perror("getpwuid_r() error");

    if((getgrnam_r("GROUP1", grpptr, grpbuffer, grplinelen, &tempGrpPtr)) != 0)
        perror("getgrnam_r() error");

    if((getgrgid_r(info.st_gid, grpptr, grpbuffer, grplinelen, &tempGrpPtr)) != 0)
        perror("getgrgid_r() error");

    if (lstat(pathname, &info) != 0)
        perror("");
    else {
        printf( (S_ISLNK(info.st_mode)) ? "l" : "-");
        printf( (info.st_mode & S_IRUSR) ? "r" : "-");
        printf( (info.st_mode & S_IWUSR) ? "w" : "-");
        printf( (info.st_mode & S_IXUSR) ? "x" : "-");
        printf( (info.st_mode & S_IRGRP) ? "r" : "-");
        printf( (info.st_mode & S_IWGRP) ? "w" : "-");
        printf( (info.st_mode & S_IXGRP) ? "x" : "-");
        printf( (info.st_mode & S_IROTH) ? "r" : "-");
        printf( (info.st_mode & S_IWOTH) ? "w" : "-");
        printf( (info.st_mode & S_IXOTH) ? "x" : "-");
        printf( (S_ISLNK(info.st_mode)) ? "l" : "-");
        printf(" %lo ", info.st_nlink);
        printf("%d ", pwd.pw_name);
        printf("%d ", pwd.pw_uid);
        printf("%d ", grp.gr_gid);
        printf("%d ", (int) info.st_size);
        char * time = new char[50];
        time = ctime(&info.st_mtime);
        printf("%s ", time);
    }
}

// this general case handles all combinations without -f
void generalNavigation(const char *pathname, dirls &flags) {
    int size = count_chars(pathname);
    char* currentDir = new char[256];
    char * new_path = new char[256]; // this is just a non const variable to hold the name of the path ***
    DIR *dir; // these structures you dont have to worry about, just for opening directory
    struct dirent *en; // just a pointer to show where you are in directory

    if (flags.d || size <=1) {
        dir = opendir("."); //open current directory for -d
        getcwd(currentDir, 256);
    }
    else dir = opendir(pathname); //open directory specified by pathname (we enter this directory and look through all the files and directories inside)
    if (dir) {
        while ((en = readdir(dir)) != NULL) { // here we iterate through all of the listings inside of the larger directory "pathname"
            if (flags.d || size <= 1) strcpy(new_path, currentDir); //update pathname to given path
            else strcpy(new_path, pathname); //update pathname to working directory
            strcat(new_path, "/"); //these two lines creates new pathname for recursion
            strcat(new_path, en->d_name);
            if (!flags.a) { // disregard dot files without -a
                if (en->d_name[0] != '.') { // en->d_name is the directory names instead
                    if (flags.l){ // Implements -l
                        longForm(new_path);
                    }
                    cout << en->d_name << endl;
                }
            }
            else{
                if (flags.l){ // implements -l and -a
                    longForm(new_path);
                }
                cout << en->d_name << endl;
            }
            if (!flags.d) { //recursing TODO: with recursion, do not recurse the "." files
                if (en->d_name[0] != '.') {
                    if (isDirectory(en->d_name)) { //THIS FUNCTION RETURNS BOOL VALUE TELLING YOU IF ITS A FILE OR DIRECTORY *** you can leave it be
                        cout << "current new path: " << new_path << endl;
                        generalNavigation(new_path, flags); //opens the smaller directory, and recurses through.
                    }
                }
            }
        }

    }
    cout << endl;
    closedir(dir); //close all directory
}

//TODO: write navigation function for when -f is involved. see if you can manage the -d an -f combo along with everything else.
void fNavigation(const char *pathname, dirls &flags) {
    int size = 2 * count_chars(pathname);
    char* currentDir = new char[256];
    char* symlink = new char[size];
    DIR *dir;
    struct dirent *en;
    if (flags.d || size <=1) {
        dir = opendir("."); //open current directory for -d
        getcwd(currentDir, 256);
        symbolicLinks(currentDir, flags);
    }
    else {
        dir = opendir(pathname); //open directory specified by pathname (we enter this directory and look through all the files and directories inside)
        symbolicLinks(pathname, flags);
    }
    if (dir) {
        while ((en = readdir(dir)) != NULL) {
            if (flags.d || size <= 1) strcpy(symlink, currentDir); //handles flag d
            else strcpy(symlink, pathname);//no flag d

            if (!flags.a) {
                if (en->d_name[0] != '.') {
                    strcat(symlink, "/");
                    strcat(symlink, en->d_name);
                    if (!symbolicLinks(symlink, flags)) { //functions handles -l cases
                        if (flags.l) {
                            longForm(symlink); //-l case
                        }
                        cout<< en->d_name << endl;
                    }
                }
            }
            else{
                strcat(symlink, "/");
                strcat(symlink, en->d_name);
                if (!symbolicLinks(symlink, flags)) { //function handles -l cases
                    if (flags.l) {
                        longForm(symlink); //-l case
                    }
                    cout << en->d_name << endl;
                }
            }
        }
        cout << endl;
        closedir(dir); //close all directory
    }
}




//TODO: everything works except: -lf combo with symbolic links (no clue)
// also except flag address flag input.