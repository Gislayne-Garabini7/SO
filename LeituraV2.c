/*#define FUSE_USE_VERSION 31
#include <fuse.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/stat.h>*/


struct MyFileSystem {
    // Definir aqui a estrutura do seu sistema de arquivos
    // ...
};


static int myfs_read(const char *path, char *buffer, size_t size, off_t offset, struct fuse_file_info *fi)
{
    (void)fi;

    // Encontra o índice do arquivo no sistema de arquivos
    int fileIndex = -1;

    for (int i = 0; i < myFileSystem.numFiles; i++)
    {
        if (strcmp(myFileSystem.files[i].filename, path) == 0)
        {
            fileIndex = i;
            break;
        }
    }

    if (fileIndex == -1) {  // Se o arquivo não for encontrado, retorna um erro
        return -ENOENT;  // Nenhum arquivo ou diretório encontrado
    }
    
    char *fileContent = myFileSystem.files[fileIndex].content; // Obtem o conteúdo do arquivo

    size_t remainingLength = strlen(fileContent) - offset; // Calcula o comprimento do conteúdo restante a ser lido

    if (offset >= strlen(fileContent)) {  // Verifica se a leitura não ultrapassa o final do arquivo
        return 0;  // Nada a ser lido, já atingiu o final do arquivo
    }

    size_t bytesToRead = (size < remainingLength) ? size : remainingLength;  // Determina a quantidade de bytes a serem lidos com base no tamanho solicitado

    memcpy(buffer, fileContent + offset, bytesToRead); // Copia a porção apropriada do conteúdo do arquivo para o buffer

    return bytesToRead; // Retorne a quantidade de bytes lidos
}







/*static int do_readdir( const char *path, void *buffer, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info *fi )
{
	printf( "--> Lista de arquivos no diretório: %s\n", path );
	
	filler( buffer, ".", NULL, 0 ); // Current Directory
	filler( buffer, "..", NULL, 0 ); // Parent Directory
	
	if ( strcmp( path, "/" ) == 0 ) // If the user is trying to show the files/directories of the root directory show the following
	{
		filler( buffer, "file54", NULL, 0 ); //file54 e file349 
		filler( buffer, "file349", NULL, 0 );
	}
	
	return 0;
}




static struct MyFileSystem myFileSystem;  // Supondo que você tenha uma instância global


Função de leitura de arquivo
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
*/
