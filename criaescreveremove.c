#define FUSE_USE_VERSION 31
#define _FILE_OFFSET_BITS 64

#include <fuse.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <time.h>

// Criando a estrutura de dados do sistema
struct MyFile
{
    char filename[50]; // O tamanho de caracteres que um o nome de um arquivo pode ter
    size_t size;       // Tamanho do arquivo
    time_t timestamp;   // Data e Hora que o arquivo foi criado ou modificado
    char *content;      // Armazena o conteúdo do arquivo
};

struct MyFileSystem
{
    // Onde vai armazenar os arquivos do sistema
    struct MyFile files[100]; // Indica que o sistema pode armazenar até 100 arquivos
    int numFiles;             // Variável que mantém o número de arquivos no sistema
};

// Inicializando o sistema
static struct MyFileSystem myFileSystem;

// Funções de sistema de arquivos
static int myfs_create(const char *MyFS, mode_t mode, struct fuse_file_info *fi)
{
    (void)mode; // Evitar os avisos do compilador de variáveis não utilizadas.

    // Condição para verificar se esse arquivo já existe
    for (int n = 0; n < myFileSystem.numFiles; n++)
    {
        if (strcmp(myFileSystem.files[n].filename, MyFS) == 0)
        {
            return -EEXIST; // Se o arquivo existir, dá erro
        }
    }
    // verifica se tem espaços a mais no arquivo
    if (myFileSystem.numFiles >= 100)
    {
        return -ENOSPC;
    }
    // Criando um novo arquivo
    struct MyFile newFile;
    strcpy(newFile.filename, MyFS);
    newFile.size = 0;                           // Iniciando com 0 que indica que está vazio o arquivo
    newFile.timestamp = time(NULL);             // Obtém o timestamp atual
    newFile.content = NULL;                     // indica que o arquivo não tem conteúdo inicialmente

    // Atualiza o número de arquivos
    ++myFileSystem.numFiles;

    // Retorna o ponteiro para o arquivo recém-criado
    return 0;
}

static int myfs_write(const char *MyFS, const char *buf, size_t size, off_t offset, struct fuse_file_info *fi)
{
    (void)fi; // Evitar avisos de variável não utilizada

    // Procura o arquivo em MyFileSystem pelo caminho dele
    int fileIndex = -1; // Não encontra o arquivo

    for (int i = 0; i < myFileSystem.numFiles; i++)
    {
        // Verifica se o nome do arquivo de índice 'i' é igual o nome do arquivo especificado no path
        if (strcmp(myFileSystem.files[i].filename, MyFS) == 0)
        {
            fileIndex = i;
            break;
        }
    }
    // Se o arquivo procurado não existir, retorna um erro
    if (fileIndex == -1)
    {
        // Retorna o erro 'ENOENT' (quando o arquivo ou o diretório não existe)
        return -ENOENT;
    }

    // Verifique se é necessário redimensionar o conteúdo do arquivo
    if (offset + size > myFileSystem.files[fileIndex].size)
    {
        // Se necessário o 'realloc', ajusta o tamanho do bloco na memória
        myFileSystem.files[fileIndex].content = realloc(myFileSystem.files[fileIndex].content, offset + size);
        // Verificando se o 'realloc' falhou na alocação
        if (myFileSystem.files[fileIndex].content == NULL)
        {
            // Se houver falha na alocação de memória retorna um erro
            return -ENOMEM;
        }
        // Atualiza o tamanho do arquivo
        myFileSystem.files[fileIndex].size = offset + size;
    }

int myfs_unlink(const char *path) {
  // Verifica se o arquivo existe.
  struct myfs_object *object = myfs_find_object(path);
  if (object == NULL) {
    return -ENOENT;
  }

  // Exclui o arquivo do repositório Git.
  int ret = libcurl_delete(object->path);
  if (ret != 0) {
    return ret;
  }

  // Remove o objeto do MyFS.
  myfs_remove_object(object);

  return 0;
}


    // Escreve/copia os dados para o conteúdo do arquivo
    memcpy(myFileSystem.files[fileIndex].content + offset, buf, size);

    // Atualiza o timestamp
    myFileSystem.files[fileIndex].timestamp = time(NULL);

    return size;
}
