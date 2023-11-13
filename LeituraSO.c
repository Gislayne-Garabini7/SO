#define FUSE_USE_VERSION 31
#include <fuse.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

static const char *caminhoArquivo = "Caminho do Arquivo"; // Ex:C:\Users\Fulana\Desktop\SO\exemplo.txt

static int myfs_getattr(const char *caminho, struct stat *stbuf) {
    int resposta = 0;

    memset(stbuf, 0, sizeof(struct stat));

    if (strcmp(caminho, "/exemplo.txt") == 0) { //confere se é o arquivo correto, define as permissões, tamanho e tipo;
        stbuf->st_mode = S_IFREG | 0444; //código que define como somente leitura, 0666 para leitura e gravação, depende do que quer
/*Ex:   4: Permissão de leitura (Read)
        2: Permissão de gravação (Write)
        1: Permissão de execução (Execute)
        Os números 4, 2, e 1 correspondem aos bits de permissão, e você os soma para obter a combinação desejada. Portanto, 0444 significa:

        4 (leitura) para o usuário (dono do arquivo),
        4 (leitura) para o grupo,
        4 (leitura) para outros.
        Se você deseja adicionar permissões de escrita, você adiciona 2 (permissão de gravação). Se deseja permissões de leitura e gravação para o usuário, você usaria 0644:

        6 (leitura e gravação) para o usuário,
        4 (leitura) para o grupo,
        4 (leitura) para outros.
        Se quiser dar permissões de leitura e escrita para todos, você usaria 0666:

        6 (leitura e gravação) para o usuário,
        6 (leitura e gravação) para o grupo,
        6 (leitura e gravação) para outros.*/

        stbuf->st_nlink = 1; //Definição do número de links para o arquivo como 1
        //stbuf->st_size = 1024; // Substitua pelo tamanho real do seu arquivo
    } else {
        resposta = -ENOENT; // Arquivo não encontrado
    }

    return resposta;
}

static int myfs_ler_arquivo(const char *caminho, char *buf, size_t tamanho, off_t offset, struct fuse_file_info *fi) {
    (void) fi;

    if (strcmp(caminho, "/exemplo.txt") != 0) {// Verifica se o caminho corresponde ao arquivo
        return -ENOENT; // Arquivo não encontrado
    }

    FILE *arquivo = fopen(caminhoArquivo, "r");// Abre o arquivo em modo de leitura
    // "r" para leitura, "w" para escrita, precisa confirmar
    if (arquivo == NULL) {
        return -errno; // Erro ao abrir o arquivo
    }

    if (fseek(arquivo, offset, SEEK_SET) == -1) {// Move o cursor para a posição especificada pelo offset
        fclose(arquivo);
        return -errno; // Erro ao mover o cursor
    }

    size_t bytes_lidos = fread(buf, 1, tamanho, arquivo);// Lê até 'tamanho' bytes do arquivo para o buffer
    fclose(arquivo);

    return bytes_lidos;
}

static struct fuse_operations myfs_operacoes = {// Operações FUSE utilizadas
    .getattr = myfs_obter_atributos,
    .read = myfs_ler_arquivo,
    // Outras funções do FUSE aqui
};

int main(int argc, char *argv[]) {// Inicia o FUSE com as operações definidas
    return fuse_main(argc, argv, &myfs_operacoes, NULL);
}
