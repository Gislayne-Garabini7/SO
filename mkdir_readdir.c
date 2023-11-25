//cod criando e escrevendo
//gcc -o test remocao.c -lfuse
//./test ./test2
//comando para cria;'ao >      touch arq.txt

// echo arq.txt  "Olá, mundo!" 
//




#define FUSE_USE_VERSION 31
#define _FILE_OFFSET_BITS 64

#include <fuse.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <time.h>

// Criando a estrutura de dados do sistema
struct MyFile {
    char filename[50];
    size_t size;
    time_t timestamp;
    char *content;
    int isDirectory; // novo
};
struct MyFileSystem
{
    struct MyFile files[100];
    int numFiles;
};

// Inicializando o sistema
static struct MyFileSystem myFileSystem;

// Funções de sistema de arquivos
static int myfs_create(const char *MyFS, mode_t mode, struct fuse_file_info *fi)
{
    (void)mode;

    for (int n = 0; n < myFileSystem.numFiles; n++)
    {
        if (strcmp(myFileSystem.files[n].filename, MyFS) == 0)
        {
            return -EEXIST;
        }
    }

    if (myFileSystem.numFiles >= 100)
    {
        return -ENOSPC;
    }

    struct MyFile newFile;
    strcpy(newFile.filename, MyFS);
    newFile.size = 0;
    newFile.timestamp = time(NULL);
    newFile.content = NULL;

    ++myFileSystem.numFiles;

    return 0;
}

static int myfs_write(const char *MyFS, const char *buf, size_t size, off_t offset, struct fuse_file_info *fi)
{
    (void)fi;

    int fileIndex = -1;

    for (int i = 0; i < myFileSystem.numFiles; i++)
    {
        if (strcmp(myFileSystem.files[i].filename, MyFS) == 0)
        {
            fileIndex = i;
            break;
        }
    }

    if (fileIndex == -1)
    {
        return -ENOENT;
    }

    if (offset + size > myFileSystem.files[fileIndex].size)
    {
        myFileSystem.files[fileIndex].content = realloc(myFileSystem.files[fileIndex].content, offset + size);
        if (myFileSystem.files[fileIndex].content == NULL)
        {
            return -ENOMEM;
        }
        myFileSystem.files[fileIndex].size = offset + size;
    }

    memcpy(myFileSystem.files[fileIndex].content + offset, buf, size);

    myFileSystem.files[fileIndex].timestamp = time(NULL);

    return size;
}

static int myfs_unlink(const char *path)
{
    // fazer o remo;ao


    return 0; 
}

static int myfs_mkdir(const char *path, mode_t mode) {
    if (myFileSystem.numFiles >= 100) {
        return -ENOSPC; 
    }

    for (int n = 0; n < myFileSystem.numFiles; n++) {
        if (strcmp(myFileSystem.files[n].filename, path) == 0 && myFileSystem.files[n].isDirectory == 1) {
            return -EEXIST; 
        }
    }

struct MyFile newDirectory;
    strcpy(newDirectory.filename, path);
    newDirectory.size = 0;
    newDirectory.timestamp = time(NULL);
    newDirectory.content = NULL;
    newDirectory.isDirectory = 1; 

    myFileSystem.files[myFileSystem.numFiles] = newDirectory;
    myFileSystem.numFiles++;

    return 0; // Return success
}

static int myfs_readdir(const char *path, void *buf, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info *fi) {
    (void) offset;
    (void) fi;

    if (strcmp(path, "/") != 0) {
        return -ENOENT; 
    }

    for (int i = 0; i < myFileSystem.numFiles; i++) {
        if (myFileSystem.files[i].isDirectory == 1) {
            if (filler(buf, myFileSystem.files[i].filename, NULL, 0) != 0) {
                return -ENOMEM;
            }
        }
    }

    return 0; // Return success
}

// Definindo a estrutura fuse_operations
static struct fuse_operations myfs_operations = {
    .create = myfs_create,
    .write = myfs_write,
    .unlink = myfs_unlink,
    .mkdir = myfs_mkdir,
    .readdir = myfs_readdir,

};

int main(int argc, char *argv[])
{
    //inicialização do FUSE 

    return fuse_main(argc, argv, &myfs_operations, NULL);
}
