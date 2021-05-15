#ifndef PROJETO1SERVIDOR__SERVERFUNCS_H
#define PROJETO1SERVIDOR__SERVERFUNCS_H

// codigos de cores
#define CLLR "\033[1;31m"
#define CLLG "\033[1;32m"
#define CLLB "\033[1;34m"
#define CLLP "\033[1;35m"
#define CLLC "\033[1;36m"
#define CLW "\033[0m"
#define CLY "\033[0;33m"

#define CADS "ServerFiles/cadastros.txt" // arquivo de cadastros
#define CONFIG "ServerFiles/config.txt" // arquivo de configuracao


// string expecial de escrita dos arquivos (para controlar a leitura e escrita corretamente)
#define SPECCHAR "+=====+"

// escreve colorido
#define PRINTCL(COLOR,...) printf(COLOR), printf(__VA_ARGS__), printf(CLW)

// escreve com duas cores em sequencia
#define PRINT2C(A,COLOR1,COLOR2,...) printf(COLOR1), printf(A), printf(COLOR2), printf(__VA_ARGS__), printf(CLW)

/* ----------------------- FUNCOES DE OPERACOES DE DADOS AUXILIARES DO SERVIDOR ---------------------- */

/* atualiza o arquivo de cadastros com a lista atualizada */
void atualizaArquivo(NoPerfil *lista);

/* remove todas as quebras de linha de uma string (\n ou \r) */
void removeBreak(char *str);

/* logs referentes ao que se trata do socket (socket,bind,listen,accept) */
void socketLOG(char *x, int code, char *log, char *ip);

/* logs referentes as operacoes feitas pelo(s) cliente(s) */
void infoLOG(char *x, int code, char *log, char *ip);

/* ----------------------- FUNCOES DE OPERACOES DE DADOS PRINCIPAIS DO SERVIDOR ---------------------- */
/* ----- cada uma dessas funções corresponde à uma das 8 operações que o cliente pode requisitar ----- */

/* funcao para ler e retornar todos os cadastros em forma de lista */
/* entrada: nenhuma                                                */
/* retorno: referencia para lista de perfis do arquivo (NoPerfil*) */
NoPerfil *listarTodos();

/* funcao para encontrar os dados de um perfil a partir do identificador (email)                  */
/* entradas: email a encontrar (char*), lista (NoPerfil*) cabeça da lista que será feita a busca) */
/* saida: referencia para o perfil encontrado ou NULL caso o perfil nao exista (Perfil*)          */
Perfil *encontrarPerfil(char *email, NoPerfil *lista);

/* funcao para adicionar um novo perfil ao registro         */
/* entrada: referencia para o perfil à cadastrar (Perfil*)  */
/* retorno: 1 em sucesso ou 0 em falha (int)                */
int addPerfil(Perfil *p);

/* funcao para remover um perfil registrado, a partir de seu identificador (email) */
/* entrada: email (char*)                                                          */
/* saida: sucesso ou falha (int)                                                   */
int removerPerfil(char *email);

/* funcao para ler e retornar todos os cadastros de determinada formacao em forma de lista */
/* entrada: form = formacao a buscar (char*)                                               */
/* retorno: referencia para lista de informacoes (NoPerfilEmailNome*)                      */
NoPerfilEmailNome *listarPorFormacao(char *form);

/* funcao para ler e retornar todos os cadastros que possuem determinada habilidade em forma de lista */
/* entrada: hab = habilidade a buscar (char*)                                                         */
/* retorno: referencia para lista de informacoes (NoPerfilEmailNome*)                                 */
NoPerfilEmailNome *listarPorHabilidade(char *hab);

/* funcao para ler e retornar todos os cadastros formados em determinado ano, em forma de lista */
/* entrada: ano = ano a buscar (int)                                                            */
/* retorno: referencia para lista de informacoes (NoPerfilEmailNomeCurso*)                      */
NoPerfilEmailNomeCurso *listarPorAno(int ano);

/* funcao para adicionar uma experiencia em um determinado perfil                                 */
/* entrada: email para encontrar o perfil (char*)                                                 */
/* saida: 2 em sucesso, 1 caso a experiencia ja exista e 0 se nao existe perfil com o email (int) */
int addExperiencia(char *email, char *exp);

#endif
