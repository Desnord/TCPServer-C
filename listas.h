#ifndef PROJETO1SERVIDOR__LISTAS_H
#define PROJETO1SERVIDOR__LISTAS_H

/* no de string (lista de string) */
typedef struct noString
{
	struct noString *prox;
	char *str;
} NoString;

/* -------------------------------- Structs Basicas -------------------------------- */
/* struct de perfil completo */
typedef struct perfil
{
	char  *email;
	char  *nome;
	char  *sobrenome;
	char  *cidade_residencia;
	char  *formacao;
	
	int ano_formatura;
	
	NoString *experiencia;
	NoString *habilidades;
}Perfil;

/* struct auxiliar (perfil simples 1) */
typedef struct perfilEmailNome
{
	char  *email;
	char  *nome;
	char  *sobrenome;
}PerfilEmailNome;

/* struct auxiliar (perfil simples 2) */
typedef struct perfilEmailNomeCurso
{
	char  *email;
	char  *nome;
	char  *sobrenome;
	char  *formacao;
}PerfilEmailNomeCurso;

/* ------------------------------- Structs de Listas ------------------------------- */
/* no de perfil (lista de perfil) */
typedef struct noPerfil
{
	Perfil *pessoa;
	struct noPerfil *prox;
} NoPerfil;

/* no de struct auxiliar (lista de perfil simples 1) */
typedef struct noPerfilEmailNome
{
	PerfilEmailNome *perfil;
	struct noPerfilEmailNome *prox;
} NoPerfilEmailNome;

/* no de struct auxiliar (lista de perfil simples 2) */
typedef struct noPerfilEmailNomeCurso
{
	PerfilEmailNomeCurso *perfil;
	struct noPerfilEmailNomeCurso *prox;
} NoPerfilEmailNomeCurso;

/* --------------------------- funcoes de inserir em lista ------------------------- */
void perfilListInsert(NoPerfil *lista, Perfil *pessoa);
void NPENListInsert(NoPerfilEmailNome *lista, PerfilEmailNome *info);
void NPENCListInsert(NoPerfilEmailNomeCurso *lista, PerfilEmailNomeCurso *info);
void stringListInsert(NoString *lista, char *str);

/* --------------------------- funcoes de liberar lista ---------------------------- */
void perfilListFree(NoPerfil *lista);
void NPENListFree(NoPerfilEmailNome *lista);
void NPENCListFree(NoPerfilEmailNomeCurso *lista);
void stringListFree(NoString *lista);

/* -------------------------- funcoes de criar nova lista -------------------------- */
NoPerfil *newPerfilList();
NoPerfilEmailNome *newNPENList();
NoPerfilEmailNomeCurso *newNPENCList();
NoString *newStringList();

/* ----------------------- funcoes de obter tamanho da lista ----------------------- */
int perfilListLen(NoPerfil *lista);
int NPENListLen(NoPerfilEmailNome *lista);
int NPENCListLen(NoPerfilEmailNomeCurso *lista);
int stringListLen(NoString *lista);

/* --------------------------------- outras funcoes -------------------------------- */
Perfil *createPerfil(char *email, char *nome, char *sn, int ano, char *form, char *rsd, NoString *lexp, NoString *lhab);
Perfil *newPerfil();
void freePerfil(Perfil *p);

int existeString(char *hab, NoString *lista);
int existePerfil(char *email, NoPerfil *lista);

void perfilListRemove(NoPerfil *lista, char *email);

#endif