/*#define FUSE_USE_VERSION 31
#include <fuse.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/stat.h>*/

// Função de leitura de arquivo
static int myfs_read(const char *MyFS, char *buf, size_t size, off_t offset, struct fuse_file_info *fi){
    (void)fi;
    
    int fileIndex = -1; //Valor pré definido para não encontrado
    for (int i = 0; i < myFileSystem.numFiles; i++){ // Procuramos o índice do arquivo no sistema
        if (strcmp(myFileSystem.files[i].filename, MyFS) == 0)
        {
            fileIndex = i;
            break;
        }
    }

    if (fileIndex == -1){ // Se o arquivo não foi encontrado, retornamos erro
        return -ENOENT; // Arquivo não encontrado
    }

    if (offset >= myFileSystem.files[fileIndex].size){ // Se a posição de leitura está além do final do arquivo, não há nada para ler
        return 0; // Nada a ser lido além do final do arquivo
    }

    size_t bytesRead = size; // Inicialmente, lemos a quantidade total especificada (size), esse é o tamanho fornecido para o buffer

    if (offset + size > myFileSystem.files[fileIndex].size){ // Se a leitura ultrapassar o final do arquivo, ajustamos o número de bytes a ser lido
        bytesRead = myFileSystem.files[fileIndex].size - offset;
    }

    memcpy(buf, myFileSystem.files[fileIndex].content + offset, bytesRead); // Copiamos os dados do arquivo para o buffer fornecido

    return bytesRead; // Retornamos o número de bytes lidos
}
