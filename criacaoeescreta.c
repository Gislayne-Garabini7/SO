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
#include <sys/types.h>
#include <time.h>

// Criando a estrutura de dados do sistema
struct MyFile
{
    char MyFS[50]; 
    size_t size; 
    time_t timestamp; 
    char *content; 
};

struct MyFileSystem
{   // 
    struct MyFile files[100];
    int numFiles;
};

// Inicializando o sistema
static struct MyFileSystem myFileSystem;

// Funções de sistema de arquivos
static int myfs_create(const char *path, mode_t mode, struct fuse_file_info *fi)
{
    (void)mode;

    for (int n = 0; n < myFileSystem.numFiles; n++)
    {
        if (strcmp(myFileSystem.files[n].MyFS, path) == 0)
        {
            return -EEXIST;
        }
    }

    if (myFileSystem.numFiles >= 100)
    {
        return -ENOSPC;
    }

    struct MyFile newFile;
    strcpy(newFile.MyFS, path);
    newFile.size = 0;
    newFile.timestamp = time(NULL);
    newFile.content = NULL;

    ++myFileSystem.numFiles;

    return 0;
}

static int myfs_write(const char *path, const char *buf, size_t size, off_t offset, struct fuse_file_info *fi)
{
    (void)fi;

    int fileIndex = -1;

    for (int i = 0; i < myFileSystem.numFiles; i++)
    {
        if (strcmp(myFileSystem.files[i].MyFS, path) == 0)
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
   int fileIndex = find_file_index(path);

    if (fileIndex == -1) {
        return -ENOENT; 
    }

    // Libera a memória alocada para o conteúdo do arquivo
    free(myFileSystem.files[fileIndex].content);

    //Condição para remover o arquivo do sistema de arquivos
    for (int i = fileIndex; i < myFileSystem.numFiles - 1; i++) {
        myFileSystem.files[i] = myFileSystem.files[i + 1];
    }

    myFileSystem.numFiles--;

    return 0;
}

// Definindo a estrutura fuse_operations
static struct fuse_operations myfs_operations = {
    .create = myfs_create,
    .write = myfs_write,
    .unlink = myfs_unlink,

};

int main(int argc, char *argv[])
{
    //inicialização do FUSE 

    return fuse_main(argc, argv, &myfs_operations, NULL);
}
