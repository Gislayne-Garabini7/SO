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
#include <fcntl.h>
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
static int find_file_index(const char *path);

// Funções de sistema de arquivos
static int myfs_create(const char *path, mode_t mode, struct fuse_file_info *fi)
{
    (void)mode;
    
    printf("Chamando myfs_create para o caminho: %s\n", path);

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
    
    myFileSystem.files[myFileSystem.numFiles] = newFile;
    ++myFileSystem.numFiles;

    return 0;
}

static int myfs_open(const char *path, struct fuse_file_info *fi)
{
    printf("Chamando myfs_open para o caminho: %s\n", path);

    // Encontrar o índice do arquivo no sistema de arquivos
    int fileIndex = find_file_index(path);

    if (fileIndex == -1)
    {
        return -ENOENT;  // Arquivo não encontrado
    }

    // Verificar se o arquivo está sendo aberto para escrita
    if ((fi->flags & O_ACCMODE) == O_WRONLY || (fi->flags & O_ACCMODE) == O_RDWR)
    {
        // Se estiver aberto para escrita, liberar a memória antiga do conteúdo
        free(myFileSystem.files[fileIndex].content);
        myFileSystem.files[fileIndex].content = NULL;
        myFileSystem.files[fileIndex].size = 0;
    }

    // Armazenar informações do arquivo para uso posterior
    fi->fh = fileIndex;

    return 0;  // Sucesso na abertura do arquivo
}

static int myfs_write(const char *path, const char *buf, size_t size, off_t offset, struct fuse_file_info *fi)
{
    (void)fi;
    printf("Chamando myfs_write para o caminho: %s\n", path);

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


// Função para encontrar o índice de um arquivo pelo nome
static int find_file_index(const char *path) {
    for (int i = 0; i < myFileSystem.numFiles; i++) {
        if (strcmp(myFileSystem.files[i].MyFS, path) == 0) {
            return i;
        }
    }
    return -1;
}

static int myfs_unlink(const char *path) {
    int fileIndex = find_file_index(path);

    if (fileIndex == -1) {
        return -ENOENT;  // Arquivo não encontrado
    }

    // Liberar a memória alocada para o conteúdo do arquivo
    free(myFileSystem.files[fileIndex].content);

    // Remover o arquivo do sistema de arquivos
    for (int i = fileIndex; i < myFileSystem.numFiles - 1; i++) {
        myFileSystem.files[i] = myFileSystem.files[i + 1];
    }

    myFileSystem.numFiles--;

    return 0;  // Sucesso na remoção
}

static int myfs_utime(const char *path, struct utimbuf *ubuf) {
    (void)path;

    printf("Chamando myfs_utime para o caminho: %s\n", path);

    int fileIndex = find_file_index(path);

    if (fileIndex == -1) {
        return -ENOENT;  // Arquivo não encontrado
    }

    // Atualizar o carimbo de data e hora do arquivo
    myFileSystem.files[fileIndex].timestamp = ubuf->modtime;

    return 0;  // Sucesso
}

// Definindo a estrutura fuse_operations
static struct fuse_operations myfs_operations = {
    .create = myfs_create,
    .open = myfs_open,
    .write = myfs_write,
    .unlink = myfs_unlink,
    .utime = myfs_utime,
 

};

int main(int argc, char *argv[])
{
    //inicialização do FUSE 

    return fuse_main(argc, argv, &myfs_operations, NULL);
}
