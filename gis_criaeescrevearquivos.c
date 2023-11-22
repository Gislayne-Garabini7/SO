#define FUSE_USE_VERSION 31 
#include <fuse.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

//Criando a estrutura de dados do sistema
struct MyFile{
	char filename[50]; //O tamanho de caracteres que o nome de um arquivo pode ter
	size_t size; // Tamanho do arquivo
	time_t timestamp; // Data e Hora que o arquivo foi criado ou modificado
	char* content; //Armazena o conteúdo do arquivo
};

struct MyFileSystem{
	//Onde vai armazenar os arquivos do sistema
	struct MyFile files[100]; //Indica que o sistema pode armazenar até 100 arquivos
	int numFiles; //Variável que mantém o número de arquivos no sistema 
};

// Inicializando o sistema
static struct MyFileSystem myFileSystem;

static int myfs_create(const char* MyFS, mode_t mode, struct fuse_file_info* fi){
	(void)mode; //Evitar os avisos do compilador de variáveis não utilizadas.

	//Condição para verificar se esse arquivo já existe
	for(int n = 0; n < myFileSystem.numFiles; n++){
		if(strcmp(myFileSystem.files[n].filename, MyFS) == 0){
			return -EEXIST; // Se o arquivo existir, dá erro
		}
	}
	//verifica se tem e´paços a mais dno arquivo
	if (myFileSystem.numFiles >= 100){
		return -ENOSPC;
	}
	//Criando um novo arquivo
	struct MyFile newFile; 
	strcpy (newFile.filename, MyFS); //Faz uma cópia do nome 
	newFile.size = 0; // Inicia com 0 para indicar que está vazio o arquivo
	newFile.timestamp = time(NULL); //Obtem o timestamp atual
	newFile.content = NULL; // indica que o arquivo não tem contéudo incialmente

	//Atualiza o número de arquivos
	++myFileSystem.numFiles;

	//Retorna um ponteiro para o arquivo recém-criado
	return &newFile;
}

static int myfs_write (const char* MyFS, const char* buf, size_t size, off_t offset, struct fuse_file_info* fi){
	(void)fi; //Evitar avisos de variável não utilizada

	//Procura o arquivo em MyFileSystem pelo caminho dele
	int fileIndex = -1; //Não encontra o arquivo

	for(int i = 0; i < myFileSystem.numFiles; i++){
		//Verifica se o nome do arquivo de indice 'i' é igual o nome do arquivo especificado no path que aqui é 'MyFS'
		if(strcmp(myFileSystem.files[i].filename, MyFS) == 0){
			fileIndex = i;
			break;
		}
	}
	// Se o arquivo procurado não existir, retorna um erro
	if(fileIndex == -1){
		//Retorna o erro 'ENOENT' (quando o arquivo ou o diretório não existe)
		return ENOENT;
	}

	// Verificação se tem necessidade de redimensionar o conteúdo do arquivo
    if (offset + size > myFileSystem.files[fileIndex].size) {
    	// Se o redimensionamento for necessário o 'realloc', ajusta o tamanho do bloco na memória
        myFileSystem.files[fileIndex].content = realloc(myFileSystem.files[fileIndex].content, offset + size);
        //Verificando se o 'realloc' falhou na alocação
        if (myFileSystem.files[fileIndex].content == NULL) {
            //Se houver falha na alocação de memória retorna um erro ENOMEM (quando não há memória disponível)
            return -ENOMEM;
        }
        //Atualiza o tamanho do arquivo
        myFileSystem.files[fileIndex].size = offset + size;
    }

    //Escreve/copia os dados para o conteúdo do arquivo
    memcpy(myFileSystem.files[fileIndex].content + offset, buf, size);

    //Atualiza o timestamp
    myFileSystem.files[fileIndex].timestamp = time(NULL);

    //Retorna o tamnaho
    return size;


}

// Estrutura que define as operações de sistema de arquivo FUSE 
static struct fuse_operations myfs_operations = {
    .create = myfs_create,
    .write = myfs_write,
    
};

