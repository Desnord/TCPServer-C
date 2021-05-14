#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "listas.h"
#include "serverfuncs.h"

/* ------------------------------ funcoes auxiliares ----------------------------- */
void atualizaArquivo(NoPerfil *lista)
{
    remove(CADS); 							  // deleta o arquivo existente
    FILE *cadastros = fopen(CADS,"w"); // cria um novo arquivo (será atualizado)

    // percorre a lista de perfis e escreve os dados no arquivos
    for(NoPerfil *at = lista->prox; at != NULL; at=at->prox)
    {
        // perfil atual
        Perfil *p = at->pessoa;

        fprintf(cadastros,"%s\n", p->email);             // armazena email
        fprintf(cadastros,"%s\n", p->nome);              // armazena nome
        fprintf(cadastros,"%s\n", p->sobrenome);         // armazena sobrenome
        fprintf(cadastros,"%s\n", p->cidade_residencia); // armazena residencia
        fprintf(cadastros,"%s\n", p->formacao);          // armazena formacao academica
        fprintf(cadastros,"%d\n", p->ano_formatura);     // armazena ano de formatura

        // escreve lista de habilidades
        fputs(SPECCHAR,cadastros);
		fputs("\n",cadastros);
        for(NoString *strAT = p->habilidades->prox; strAT != NULL; strAT = strAT->prox)
            fprintf(cadastros,"%s\n",strAT->str);

        // escreve lista de experiencia
		fputs(SPECCHAR,cadastros);
		fputs("\n",cadastros);
        for(NoString *strAT = p->experiencia->prox; strAT != NULL; strAT = strAT->prox)
            fprintf(cadastros,"%s\n",strAT->str);
		fputs(SPECCHAR,cadastros);

		if(at->prox != NULL)
			fputs("\n",cadastros);
    }

    fclose(cadastros); // fecha arquivo
}
void removeBreak(char *str)
{
	int len = (int)strlen(str);
	for(int i = 0; i<len; i++)
	{
		if(str[i] == '\r' || str[i] == '\n')
			str[i] = '\0';
	}
}
void socketLOG(char *x, int code, char *log, char *ip)
{
	PRINTCL(CLW,"\u2192");

	if(!strcmp(x,"SBL"))
	{
		PRINTCL(CLW," %s ",log);

		if(code == -1)
		{
			PRINTCL(CLLR,"FAIL. \n");
			getchar();
			exit(0);
		}
		else
		{
			PRINTCL(CLLG,"OK. \n");
		}
	}
	else if(!strcmp(x,"A"))
	{
		if(code == 0)
		{
			PRINTCL(CLW," %s",log);
			PRINTCL(CLLR, " FAIL.\n");
			PRINTCL(CLW,"\u2192");
			PRINTCL(CLY, " [pid: %d]", getpid());
			PRINTCL(CLLR, " [ip: %s]", ip);
			PRINTCL(CLW, " falha ao se conectar ao cliente.\n");
		}
		else if(code == 1)
		{
			PRINTCL(CLW," %s",log);
			PRINTCL(CLLG, " OK.\n");
			PRINTCL(CLW,"\u2192");
			PRINTCL(CLY, " [pid: %d]", getpid());
			PRINTCL(CLLG, " [ip: %s]", ip);
			PRINTCL(CLW, " cliente se conectou.\n");
		}
		else if(code == 2)
		{
			PRINTCL(CLY, " [pid: %d]", getpid());
			PRINTCL(CLLP, " [ip: %s]", ip);
			PRINTCL(CLW, " cliente se desconectou.\n");
		}
		else if(code == 3)
		{
			PRINTCL(CLY, " [pid: %d]", getpid());
			PRINTCL(CLLR, " [ip: %s]", ip);
			PRINTCL(CLW, " conexão perdida.\n");
			int connectionFD = (int)strtol(log,NULL,10);
			close(connectionFD);
			exit(0);
		}

	}
}
void infoLOG(char *x, int code, char *log, char *ip)
{
	PRINTCL(CLW,"\u2192");
	PRINTCL(CLY, " [pid: %d]", getpid());

	if(!strcmp(x,"OP"))
	{
		PRINTCL(CLLB, " [ip: %s]", ip);
		PRINTCL(CLW, " operação escolhida pelo cliente:");
		PRINTCL(CLLP," %s\n",log);
	}
	else if(!strcmp(x,"1") || !strcmp(x,"2") || !strcmp(x,"3") || !strcmp(x,"4"))
	{
		if(code == 0)
		{
			PRINTCL(CLLR, " [ip: %s]", ip);
			PRINTCL(CLW," falha no envio.");
		}
		else if(code == 1)
		{
			PRINTCL(CLLG, " [ip: %s]", ip);
			PRINTCL(CLW," listagem enviada ao cliente.");
		}
		else if(code == 2)
		{
			PRINTCL(CLLP, " [ip: %s]", ip);
			PRINTCL(CLW," listagem realizada.");
		}

		char type[20];
		memset(type,'\0',20);

		if(!strcmp(x,"1"))
			strcpy(type,"curso:");
		else if(!strcmp(x,"2"))
			strcpy(type,"habilidade:");
		else if(!strcmp(x,"3"))
			strcpy(type,"ano:");

		PRINTCL(CLLP, "(%s%s)\n", type, log);
	}
	else if(!strcmp(x,"5") || strcpy(x,"6") || strcpy(x,"7") || !strcmp(x,"8"))
	{
		char type[20];
		memset(type,'\0',20);

		char msg[100];
		memset(msg,'\0',100);

		if(code == 0)
		{
			PRINTCL(CLLR, " [ip: %s]", ip);

			if(!strcmp(x,"5"))
				strcpy(msg," falha no envio.");
			else if(!strcmp(x,"6"))
				strcpy(msg," falha no recebimento.");
			else if(!strcmp(x,"7"))
				strcpy(msg," perfil não atualizado (experiencia duplicada).");
			else if(!strcmp(x,"8"))
				strcpy(msg," perfil não removido.");
		}
		else if(code == 1)
		{
			PRINTCL(CLLG, " [ip: %s]", ip);

			if(!strcmp(x,"5"))
				strcpy(msg," busca enviada ao cliente.");
			else if(!strcmp(x,"6"))
				strcpy(msg," perfil inserido.");
			else if(!strcmp(x,"7"))
				strcpy(msg, " perfil atualizado.");
			else if(!strcmp(x,"8"))
				strcpy(msg," perfil removido.");
		}
		else if(code == 2)
		{
			PRINTCL(CLLP, " [ip: %s]", ip);

			if(!strcmp(x,"5"))
				strcpy(msg," busca realizada.");
			else if(!strcmp(x,"6"))
				strcpy(msg," inserção requisitada.");
			else if(!strcmp(x,"7"))
				strcpy(msg, " atualização de perfil requisitada.");
			else if(!strcmp(x,"8"))
				strcpy(msg," remoção requisitada.");
		}
		else if(code == 3)
		{
			PRINTCL(CLLR, " [ip: %s]", ip);

			if(!strcmp(x,"5"))
				strcpy(msg," perfil inexistente.");
			if(!strcmp(x,"6"))
				strcpy(msg," perfil não inserido.");
			else if(!strcmp(x,"7"))
				strcpy(msg, "  perfil não atualizado (perfil inexistente).");
		}

		if(!strcmp(x,"5") || !strcmp(x,"8") || (!strcmp(x,"6") && code != 1))
			strcpy(type,"email:");

		PRINTCL(CLW,"%s",msg);
		PRINTCL(CLLP, "(%s%s)\n", type, log);
	}
}
/* ----------------------- funcoes de manipulacao dos dados ---------------------- */
NoPerfil *listarTodos()
{
    NoPerfil *listaGeral = newPerfilList();   // inicializa lista, alocando memoria
    FILE *cadastros = fopen(CADS,"r"); // Abre arquivo de cadastros

    // variaveis auxiliares
    char line[200];
	memset(line,'\0',200);
    int count_data = 0;

    Perfil *p = NULL;
    NoString *auxexp,*auxhab;

    while(fgets(line,200,cadastros))
    {
    	removeBreak(line);

    	if(!count_data && !strcmp(line,SPECCHAR))
    		break;

        if(count_data < 6)
        {
            // inicio da leitura de um perfil, logo aloca memoria para o perfil atual
            if(!count_data)
                p = (Perfil*)malloc(sizeof(Perfil));

			char *campo;

            if(count_data != 5) // casos em que o campo nao eh numerico, aloca memoria
            {
				campo = malloc(sizeof(char)*(strlen(line)+1));
				strcpy(campo,line);
            }

			switch (count_data) // atribui valores ao perfil
			{
				case 0:
					p->email = campo;
					break;
				case 1:
					p->nome = campo;
					break;
				case 2:
					p->sobrenome = campo;
					break;
				case 3:
					p->cidade_residencia = campo;
					break;
				case 4:
					p->formacao = campo;
					break;
				case 5:
					p->ano_formatura = (int)strtol(line,NULL,10);
					break;
				default:
					break;
			}
            count_data++;
        }
        else
        {
        	if(!strcmp(line,SPECCHAR))
			{
        		if(count_data == 6) // inicio da lista de habilidade
				{
					auxhab = newStringList();
					count_data++;
				}
        		else if(count_data == 7) // inicio da lista de experiencia
				{
					auxexp = newStringList();
					count_data++;
				}
        		else if(count_data == 8) // fim do perfil
				{
					// proximo perfil ==> reseta variaveis necessarias
					count_data = 0;

					// guarda listas de exp. e hab. no perfil
					p->experiencia = auxexp;
					p->habilidades = auxhab;

					// insere perfil na lista
					perfilListInsert(listaGeral,p);
				}
			}
        	else
			{
				char *str_at = malloc(sizeof(char) * ((int)strlen(line)+1));
				strcpy(str_at,line);

        		if(count_data == 7) // insere na lista de habilidades
				{
					stringListInsert(auxhab,str_at);
				}
        		else if(count_data == 8) // insere na lista de experiencia
				{
					stringListInsert(auxexp,str_at);
				}
			}
        }
		memset(line,'\0',200);
    }

    fclose(cadastros); // Fecha arquivo
    return listaGeral; // retorna lista criada
}
Perfil *encontrarPerfil(char *email, NoPerfil *lista)
{
    Perfil *p = NULL; // perfil padrao para retorno

    // percorre toda a lista em busca do perfil
    for(NoPerfil *at = lista->prox; at != NULL; at = at->prox)
    {
        if(!strcmp(at->pessoa->email,email))
        {
            p = at->pessoa;
            break;
        }
    }
    return p; // retorna perfil
}
int addPerfil(Perfil *p)
{
    NoPerfil *lista = listarTodos(); // cria lista com todos os perfis
    int adicionou = 0; 			     // variavel de retorno

    // se o email nao esta cadastrado
    if(!existePerfil(p->email,lista))
    {
        perfilListInsert(lista,p);  // insere perfil na lista
        atualizaArquivo(lista);     // atualiza o arquivo
        adicionou = 1;              // att retorno
    }
    else
        freePerfil(p);

    perfilListFree(lista); // libera memoria alocada na busca
    return adicionou;      // retorna flag
}
int removerPerfil(char *email)
{
    NoPerfil *lista = listarTodos(); // cria lista com todos os perfis
    int removeu = 0; 				 // variavel de retorno

    // se o email esta cadastrado, o perfil existe e deve ser removido
    if(existePerfil(email,lista))
    {
        perfilListRemove(lista,email); // remove perfil da lista
        atualizaArquivo(lista);        // atualiza o arquivo
        removeu = 1;                   // att retorno
    }

    perfilListFree(lista); // libera memoria alocada na busca
    return removeu;        // retorna flag
}
NoPerfilEmailNome *listarPorFormacao(char *form)
{
    NoPerfil *lista = listarTodos();			 // cria lista com todos os perfis
    NoPerfilEmailNome *listaRet = newNPENList(); // cria lista para retorno

    // percorre lista de cadastros
    for(NoPerfil *at = lista->prox; at != NULL; at = at->prox)
    {
        // se a pessoa for do curso buscado
        if(!strcmp(at->pessoa->formacao,form))
        {
            // aloca perfil aux
            PerfilEmailNome *penc = (PerfilEmailNome*)malloc(sizeof(PerfilEmailNome));

            // aloca memoria para o conteudo do perfil aux
            penc->email = malloc(sizeof(char) * ((int)strlen(at->pessoa->email)+1));
            penc->nome = malloc(sizeof(char) * ((int)strlen(at->pessoa->nome)+1));
            penc->sobrenome = malloc(sizeof(char) * ((int)strlen(at->pessoa->sobrenome)+1));

            // atribui copia valores ao perfil aux
            strcpy(penc->email,at->pessoa->email);
            strcpy(penc->nome,at->pessoa->nome);
            strcpy(penc->sobrenome,at->pessoa->sobrenome);

            // insere estrutura na lista de retorno
            NPENListInsert(listaRet,penc);
        }
    }

    perfilListFree(lista); // libera memoria alocada na busca
    return listaRet;       // retorna lista
}
NoPerfilEmailNome *listarPorHabilidade(char *hab)
{
    NoPerfil *lista = listarTodos();  			 // cria lista com todos os perfis
    NoPerfilEmailNome *listaRet = newNPENList(); // cria lista para retorno

    // percorre lista de cadastros
    for(NoPerfil *at = lista->prox; at != NULL; at = at->prox)
    {
        // se a pessoa for do curso buscado
        if(existeString(hab,at->pessoa->habilidades))
        {
            // aloca perfil aux
            PerfilEmailNome *penc = (PerfilEmailNome*)malloc(sizeof(PerfilEmailNome));

            // aloca memoria para o conteudo do perfil aux
            penc->email = malloc(sizeof(char) * ((int)strlen(at->pessoa->email)+1));
            penc->nome = malloc(sizeof(char) * ((int)strlen(at->pessoa->nome)+1));
            penc->sobrenome = malloc(sizeof(char) * ((int)strlen(at->pessoa->sobrenome)+1));

            // atribui copia valores ao perfil aux
            strcpy(penc->email,at->pessoa->email);
            strcpy(penc->nome,at->pessoa->nome);
            strcpy(penc->sobrenome,at->pessoa->sobrenome);

            // insere estrutura na lista de retorno
            NPENListInsert(listaRet,penc);
        }
    }

    perfilListFree(lista); // libera memoria alocada na busca
    return listaRet;       // retorna lista
}
NoPerfilEmailNomeCurso *listarPorAno(int ano)
{
    NoPerfil *lista = listarTodos(); 				   // cria lista com todos os perfis
    NoPerfilEmailNomeCurso *listaRet = newNPENCList(); // cria lista para retorno

    // percorre lista de cadastros
    for(NoPerfil *at = lista->prox; at != NULL; at = at->prox)
    {
        // se a pessoa formou no ano buscado
        if(at->pessoa->ano_formatura == ano)
        {
            // aloca perfil aux
            PerfilEmailNomeCurso *penc = (PerfilEmailNomeCurso*)malloc(sizeof(PerfilEmailNomeCurso));

            // aloca memoria para o conteudo do perfil aux
            penc->email = malloc(sizeof(char) * ((int)strlen(at->pessoa->email)+1));
            penc->nome = malloc(sizeof(char) * ((int)strlen(at->pessoa->nome)+1));
            penc->sobrenome = malloc(sizeof(char) * ((int)strlen(at->pessoa->sobrenome)+1));
            penc->formacao = malloc(sizeof(char) * ((int)strlen(at->pessoa->formacao)+1));

            // atribui copia valores ao perfil aux
            strcpy(penc->email,at->pessoa->email);
            strcpy(penc->nome,at->pessoa->nome);
            strcpy(penc->sobrenome,at->pessoa->sobrenome);
            strcpy(penc->formacao,at->pessoa->formacao);

            // insere estrutura na lista de retorno
            NPENCListInsert(listaRet,penc);
        }
    }
    perfilListFree(lista); // libera memoria alocada na busca
    return listaRet;       // retorna lista
}
int addExperiencia(char *email, char *exp)
{
    NoPerfil *lista = listarTodos(); 		  // cria lista com todos os perfis
    Perfil *p = encontrarPerfil(email,lista); // acha perfil (ou nao)
    int resultado = 0; 						  // variavel de retorno

    if(p != NULL)
    {
        // verifica se a experiencia ja existe no perfil
        for(NoString *at = p->experiencia->prox; at != NULL; at = at->prox)
        {
            if(!strcmp(exp,at->str))
            {
                resultado = 1;
                break;
            }
        }

        if(!resultado)
        {
            char *expins = malloc(sizeof(char)* (strlen(exp)+1));
            strcpy(expins,exp);
            stringListInsert(p->experiencia,expins); // insere experiencia
            atualizaArquivo(lista);                  // atualiza o arquivo
            resultado = 2;
        }
    }
    perfilListFree(lista);  // libera memoria alocada
    return resultado;       // retorna flag
}
