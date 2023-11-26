
#define FUSE_USE_VERSION 31
#define _FILE_OFFSET_BITS 64

#include <fuse.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <time.h>

// Estrutura de arquivos do sistema
struct MyFile {
    char *name; 
    char *content;
    size_t size;
    time_t timestamp;
    int isDirectory; 
};

// O QUE MUDOU? criacao da struct diretório; código antigo representava o fileSystem numa estrutura 
// própria, mudei para ser representado por um diretório "raiz"

// Estrutura de diretórios do sistema
struct MyDirectory{
    char *name;
    struct MyFile *arquivos; // arquivos do diretorio
    struct MyDirectory **diretorios; // diretorios no diretorio
    int num_arquivos;
    int num_diretorios;
};

// Inicializando o sistema
struct MyDirectory diretorio_raiz;

// Leitura de arquivo
static int myfs_read(const char *path, char *buffer, size_t size, off_t offset, struct fuse_file_info *fi) {
    (void)fi;

    int fileIndex = -1;
    // Procurando arquivo
    for (int i = 0; i < diretorio_raiz.num_arquivos; i++)
    {
        if (strcmp(diretorio_raiz.arquivos[i].name, path + 1) == 0) // Arquivo encontrado
        {
            if((offset + size) < diretorio_raiz.arquivos[i].size){
                size = diretorio_raiz.arquivos[i].size - offset;
                memcpy(buffer, diretorio_raiz.arquivos[i].content + offset, size);
            }
            else {
                size = 0;
            }

        }
        return size;
    }
    if (fileIndex == -1) // Arquivo não encontrado, retorna erro
    {
        return -ENOENT;
    }

}

// Criação de arquivo
// O QUE MUDOU? alterada a maneira que o arquivo é criada; o código anterior nao alocava espaço nem adicionava o arquivo na estrutura
static int myfs_create(const char *MyFS, mode_t mode, struct fuse_file_info *fi)
{
    (void)mode;
    // Checa todos os arquivos do diretório para ver se há repetição de nome
    for (int n = 0; n < diretorio_raiz.num_arquivos; n++)
    {
        if (strcmp(diretorio_raiz.arquivos[n].name, MyFS) == 0)
        {
            return -EEXIST;
        }
    }
    // Cria novo arquivo
    struct MyFile *novo_arquivo = &diretorio_raiz.arquivos[diretorio_raiz.num_arquivos++]; // Nova posição no array de arquivos
    char nome_semBarra[strlen(MyFS)];
    strcpy(nome_semBarra, MyFS + 1); // Remove o primeiro dígito da string (barra do path)
    novo_arquivo->name = nome_semBarra; // Nome do novo arquivo
    novo_arquivo->content = malloc(4096); // Alocação de memória
    novo_arquivo->size = 0;
    novo_arquivo->timestamp = time(NULL); // Mudando para o tempo atual

    diretorio_raiz.num_arquivos++; // Adiciona 1 ao número de arquivos existentes

    return 0;
}

// O QUE MUDOU? add comentários
// Escrita em arquivo
static int myfs_write(const char *MyFS, const char *buf, size_t size, off_t offset, struct fuse_file_info *fi)
{
    (void)fi;

    int fileIndex = -1;
    // Procurando arquivo
    for (int i = 0; i < diretorio_raiz.num_arquivos; i++)
    {
        if (strcmp(diretorio_raiz.arquivos[i].name, MyFS) == 0) // Arquivo encontrado
        {
            fileIndex = i; // Arquivo encontrado, armazenada sua posição
            break;
        }
    }
    if (fileIndex == -1) // Arquivo não encontrado, retorna erro
    {
        return -ENOENT;
    }

    // Verifica se o tamanho não suficiente
    if (offset + size > diretorio_raiz.arquivos[fileIndex].size)
    {
        diretorio_raiz.arquivos[fileIndex].content = realloc(diretorio_raiz.arquivos[fileIndex].content, offset + size); // Realocação do espaço
        if (diretorio_raiz.arquivos[fileIndex].content == NULL)
        {
            return -ENOMEM; // Falha na alocação
        }
        diretorio_raiz.arquivos[fileIndex].size = offset + size;
    }

    memcpy(diretorio_raiz.arquivos[fileIndex].content + offset, buf, size); // Copia os dados para o buffer
    diretorio_raiz.arquivos[fileIndex].timestamp = time(NULL); // Atualiza o timestamp
    return size;
}

// O QUE MUDOU? foi implementada
// Remoção de arquivo
static int myfs_unlink(const char *path)
{
    // Procurando arquivo
    for (int i = 0; i < diretorio_raiz.num_arquivos; i++){
        if (strcmp(path + 1, diretorio_raiz.arquivos[i].name) == 0) { // Arquivo encontrado
            free(diretorio_raiz.arquivos[i].content); // Apagando memória
        
        // "Empurrando" arquivos para frente
        for (int j = i; j < diretorio_raiz.num_arquivos - 1; j++) {
                diretorio_raiz.arquivos[j] = diretorio_raiz.arquivos[j + 1];
            }

            diretorio_raiz.arquivos--; // Decresce número de arquivos
            return 0;
        }
    };
    return -ENOENT; // Arquivo não encontrado
}

// Criação de diretório
// O QUE MUDOU? Modo que o novo diretorio é criado
static int myfs_mkdir(const char *path, mode_t mode) {

    // Verifica se o nome já está sendo utilizado
    for (int n = 0; n < diretorio_raiz.num_diretorios; n++)
    {
        if (strcmp(diretorio_raiz.diretorios[n]->name, path + 1) == 0) // Já existe arquivo com esse nome
        {
            return -EEXIST;
        }
    }

    //Alocações de memória
    struct MyDirectory *novo_diretório = malloc(sizeof(struct MyDirectory)); // Alocando espaço para o diretório
    novo_diretório->arquivos = malloc(sizeof(struct MyFile) * 50); // Abrindo espaço para 50 arquivos
    novo_diretório->diretorios = malloc(sizeof(struct MyDirectory) * 50); // Abrindo espaço para 50 diretórios
    
    // Criação
    char nome_semBarra[strlen(path)];
    strcpy(nome_semBarra, path + 1); // Remove o primeiro dígito da string (barra do path)
    novo_diretório->name = nome_semBarra;
    novo_diretório->num_arquivos = 0;
    novo_diretório->diretorios = 0;
    
    return 0;
}

// Listar arquivos do diretório
static int myfs_readdir(const char *path, void *buf, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info *fi) {
    (void) offset;
    (void) fi;

    filler(buf, ".", NULL, 0);  // Nome do diretório

    // Exibindo arquivos
    for (int i = 0; i < diretorio_raiz.num_arquivos; i++) {
        filler(buf, diretorio_raiz.arquivos[i].name, NULL, 0);
    }
    // Exibindo diretórios
    for (int i = 0; i < diretorio_raiz.num_diretorios; i++) {
        filler(buf, diretorio_raiz.num_diretorios[i].name, NULL, 0);
    }

    return 0; 
}

// Obter atributos 
static int myfs_getattr(const char *path, struct stat *stbuf) {


    if (strcmp(path, "/") == 0) { 
        stbuf->st_mode = S_IFDIR | 0755; 
        stbuf->st_nlink = 2; 
        stbuf->st_uid = getuid(); 
        stbuf->st_gid = getgid(); 
        stbuf->st_atime = time(NULL); 
        stbuf->st_mtime = time(NULL);
        stbuf->st_ctime = time(NULL); 
    }
}

// Definindo a estrutura fuse_operations

static struct fuse_operations myfs_operations = {
    .getattr = myfs_getattr,
    .readdir = myfs_readdir,
    .read = myfs_read,
    .create = myfs_create,
    .write = myfs_write,
    .utimens = myfs_utimens,
    .open = myfs_open,  // Abre um arquivo
    .unlink = myfs_unlink,  // Remove um arquivo
    .mkdir = myfs_mkdir,
};

int main(int argc, char *argv[])
{
    // Criar diretório raiz
    diretorio_raiz.name = "/";
    diretorio_raiz.arquivos = malloc(sizeof(struct MyFile) * 50);
    diretorio_raiz.diretorios = malloc(sizeof(struct MyDirectory) * 50);
    diretorio_raiz.num_diretorios = 0;
    diretorio_raiz.num_arquivos = 0;

    // Iniciando FUSE
    return fuse_main(argc, argv, &myfs_operations, NULL);
}
