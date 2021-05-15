#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "listas.h"
#include "serverfuncs.h"

/* SOCKET ------------------------- */
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/unistd.h>
typedef struct sockaddr_in SockAddr_in;
typedef struct sockaddr SockAddr;

/* -------------------------- funcoes de recebimento e envio de informacoes -------------------------- */

/* funcao para receber um perfil do servidor */
Perfil *recebePerfil(int connectionFD)
{
    // auxilia retorno
    int res = 0;

    // para guardar informacoes do perfil
    char *values[5];
    int ano;
    NoString *auxexp, *auxhab;

    // recebe tamanhos das informacoes basicas do perfil
    char str[12];
    int tams[5];
    for(int i=0; i<5; i++)
    {
        // recebe tamanho
        res = (int)read(connectionFD, str, 12);
        tams[i] = (int)strtol(str,NULL,10);

        // quebra de conexao [log do servidor]
        if(res == -1)
            return NULL;
    }

    // aloca memoria para as informacoes
    for(int i=0; i<5; i++)
        values[i] = malloc(sizeof(char)*(tams[i]+1));

    // recebe dados basicos do perfil
    for(int i=0; i<5; i++)
        read(connectionFD, values[i], tams[i]+1);
    read(connectionFD, str, 12);
    ano = (int)strtol(str,NULL,10);

    // aloca lista dos dados mais complexos
    auxhab = newStringList();
    auxexp = newStringList();

    // recebe dados mais complexos
    for(int j=0; j<2; j++)
    {
        // recebe a qtde de habilidades/experiencias que serao lidas
        res = (int)read(connectionFD, str, 12);

        // quebra de conexao [log do servidor]
        if(res == -1)
            break;

        int qtd = (int)strtol(str,NULL,10);

        NoString *auxEH;
        auxEH = j ? auxhab : auxexp;

        // recebe habilidades/experiencias
        for(int i=0; i<qtd; i++)
        {
            // recebe tamanho da habilidade/experiencia
            res = (int)read(connectionFD, str, 12);

            // quebra de conexao [log do servidor]
            if(res == -1)
                break;

            int tam = (int)strtol(str,NULL,10);

            // aloca memoria para a habilidade/experiencia
            char *habexp = malloc(sizeof(char)*(tam+1));

            // recebe habilidade/experiencia e armazena na lista
            read(connectionFD,habexp,tam+1);
            stringListInsert(auxEH,habexp);
        }
    }
    // alloca perfil, atribui valores e o retorna (sucesso no recebimento)
    Perfil *p;
    p = createPerfil(values[0],values[1],values[2],ano,values[3],values[4],auxexp,auxhab);

    // retorno correto OU quebra de conexao [log do servidor]
    if(res)
        return p;
    else
    {
        freePerfil(p);
        return NULL;
    }
}

/* funcao que envia um perfil através do socket */
int enviaPerfil(int connectionFD, Perfil *p)
{
    // envia tamanho das informacoes basicas
    char str[12];
    int tams[5] = {(int)strlen(p->email),(int)strlen(p->nome),(int)strlen(p->sobrenome),(int)strlen(p->formacao),(int)strlen(p->cidade_residencia)};
    for(int i=0; i<5; i++)
    {
        sprintf(str, "%d", tams[i]);
        write(connectionFD, str, 12);
    }

    // envia informacoes basicas
    write(connectionFD, p->email, tams[0]+1);
    write(connectionFD, p->nome, tams[1]+1);
    write(connectionFD, p->sobrenome, tams[2]+1);
    write(connectionFD, p->formacao, tams[3]+1);
    write(connectionFD, p->cidade_residencia, tams[4]+1);

    sprintf(str, "%d", p->ano_formatura);
    int res = (int)write(connectionFD, str, 12);

    // quebra de conexao [log do servidor]
    if(res == -1)
        return 0;

    // percorre as listas de experiencia e habilidade, enviando as informacoes
    for(int i=0; i<2; i++)
    {
        int tamLS = (!i) ? (stringListLen(p->habilidades)) : (stringListLen(p->experiencia));

        // envia tamanho da lista (hab ou exp)
        sprintf(str, "%d", tamLS);
        res = (int)write(connectionFD, str, 12);

        // quebra de conexao [log do servidor]
        if(res == -1)
            return 0;

        if (tamLS > 0)
        {
            // escolhe qual lista esta sendo percorrida (hab ou exp)
            NoString *at;
            at = (!i) ?  (p->habilidades->prox) : (p->experiencia->prox);

            while(at != NULL)
            {
                // envia tamanho da habilidade/experiencia atual
                sprintf(str, "%d", (int)strlen(at->str));
                write(connectionFD, str, 12);

                // envia habilidade/experincia
                res = (int)write(connectionFD,at->str,(int)strlen(at->str)+1);

                // quebra de conexao [log do servidor]
                if(res == -1)
                    return 0;

                //passa para a proxima hab/exp
                at = at->prox;
            }
        }
    }
    return 1; // retorna
}

/* funcao que envia todos os perfis através do socket */
int enviaTodos(int connectionFD)
{
    // flag do retorno
    int res;

    // lista com todos os perfis
    NoPerfil *lista = listarTodos();

    // envia quantidade de perfis
    char str[12];
    sprintf(str, "%d", perfilListLen(lista));
    res = (int)write(connectionFD, str, 12);

    // quebra de conexao [log do servidor]
    if(res == -1)
        return res+1;

    // percorre a lista, enviando cada um dos perfis
    for(NoPerfil *at = lista->prox; at!= NULL; at = at->prox)
    {
        res = enviaPerfil(connectionFD,at->pessoa);

        // quebra de conexao [log do servidor]
        if(!res)
            return res;
    }

    perfilListFree(lista); // libera memoria alocada
    return res;
}

/* funcoes para enviar todos os perfis reduzidos (de acordo com a selecao escolhida) */
int NPENenviaTodos(int connectionFD, NoPerfilEmailNome *lista)
{
    /* percorre a lista de perfis e envia cada um dos perfis ao cliente */
    for(NoPerfilEmailNome *at = lista->prox; at != NULL; at = at->prox)
    {
        /* tamanho dos 3 campos */
        char str[12];
        int tams[3] = {(int)strlen(at->perfil->email),(int)strlen(at->perfil->nome),(int)strlen(at->perfil->sobrenome)};

        /* envia tamanhos dos 3 campos */
        for(int i=0; i<3; i++)
        {
            sprintf(str, "%d", tams[i]);
            write(connectionFD, str, 12);
        }

        /* envia informacoes */
        write(connectionFD,at->perfil->email,tams[0]+1);
        write(connectionFD,at->perfil->nome,tams[1]+1);
        int res = (int)write(connectionFD,at->perfil->sobrenome,tams[2]+1);

        // quebra de conexao [log do servidor]
        if(res == -1)
            return res+1;
    }
    return 1;
}

int NPENCenviaTodos(int connectionFD, NoPerfilEmailNomeCurso *lista)
{
    /* percorre a lista de perfis e envia cada um dos perfis ao cliente */
    for(NoPerfilEmailNomeCurso *at = lista->prox; at != NULL; at = at->prox)
    {
        /* tamanho dos 4 campos */
        char str[12];
        int tams[4] = {(int)strlen(at->perfil->email),(int)strlen(at->perfil->nome),(int)strlen(at->perfil->sobrenome),(int)strlen(at->perfil->formacao)};

        /* envia tamanhos dos 4 campos */
        for(int i=0; i<4; i++)
        {
            sprintf(str, "%d", tams[i]);
            write(connectionFD, str, 12);
        }

        /* envia informacoes */
        write(connectionFD,at->perfil->email,tams[0]+1);
        write(connectionFD,at->perfil->nome,tams[1]+1);
        write(connectionFD,at->perfil->sobrenome,tams[2]+1);
        int res = (int)write(connectionFD,at->perfil->formacao,tams[3]+1);

        // quebra de conexao [log do servidor]
        if(res == -1)
            return res+1;
    }
    return 1;
}

/* funcao de comunicação entre cliente-servidor [gerencia as 8 operações] */
void comunicacao(int connectionFD, char *ip)
{
	char opt[20];
    for (;;)
    {
      // le mensangem do cliente (operacao escolhida)
	    memset(opt,'\0',20);
      int resp = (int)read(connectionFD, opt, 20); // para tratar conexao quebrada

      // imprime número da operação a ser feita. (ou finaliza execucao do processo, se conn caiu)
      if(resp != -1 && strlen(opt) != 0)
      {
        infoLOG("OP",0, opt, ip);
      }
      else
      {
      	char tmpstr[12];
		    sprintf(tmpstr, "%d", connectionFD);
		    socketLOG("A", 3, tmpstr, ip);
      }

		  if (opt[0] == '0') // conexao com o cliente encerrada corretamente
			   break;


      if(opt[0] == '1' || opt[0] == '2')
      {
          // recebe informacao a ser filtrada
          char buffer[200];
		      memset(buffer,'\0',200);
          resp = (int)read(connectionFD, buffer, 200);

          // verifica quebra de conexao
          if(resp == -1)
              continue;

		      NoPerfilEmailNome *lista = newNPENList(); // aloca lista
		      infoLOG(opt, 2, buffer, ip); 		          // [log do servidor]

          if(opt[0] == '1') // lista dos perfis reduzidos que possuem a formacao requisitada
			       lista = listarPorFormacao(buffer);
          else if(opt[0] == '2') // lista dos perfis reduzidos que possuem a habilidade requisitada
			       lista = listarPorHabilidade(buffer);

    			// envia tamanho da lista
    			char str[12];
    			sprintf(str, "%d", NPENListLen(lista));
    			write(connectionFD, str, 12);

    			resp = NPENenviaTodos(connectionFD,lista); // envia lista
    			NPENListFree(lista);                       // libera memoria alocada

    			infoLOG(opt, resp, buffer, ip); // [log do servidor]
      }
      else if(opt[0] == '3')
      {
          // recebe ano a ser filtrado
          char str[12];
          resp = (int)read(connectionFD, str, 12);
          int ano = (int)strtol(str,NULL,10);

          // verifica quebra de conexao
          if(resp == -1)
              continue;

          NoPerfilEmailNomeCurso *lista = listarPorAno(ano); // encontra lista dos perfis reduzidos que formaram no ano requisitado
		      infoLOG(opt, 2, str, ip); 				                 // [log do servidor]

          // envia tamanho da lista
          int tam = NPENCListLen(lista);
          sprintf(str, "%d", tam);
          write(connectionFD, str, 12);

          NPENCenviaTodos(connectionFD,lista); // envia lista
          NPENCListFree(lista);                // libera memoria alocada

		      infoLOG(opt, resp, str, ip); // [log do servidor]
      }
      else if(opt[0] == '4')
      {
		    infoLOG(opt,2,"lista completa",ip); // [log do servidor]
        resp = enviaTodos(connectionFD); // [envia todos os perfis
		    infoLOG(opt,resp,"lista completa",ip); // [log do servidor]
      }
      else if(opt[0] == '5')
      {
        // recebe email a ser buscado
        char buffer[200];
        resp = (int)read(connectionFD, buffer, 200);

        // verifica quebra de conexao
        if(resp == -1)
            continue;

		    infoLOG(opt,2,buffer,ip); // [log do servidor]

        NoPerfil *listaGeral = listarTodos(); 		    // cria lista com todos os cadastros
        Perfil *p = encontrarPerfil(buffer,listaGeral); // procura perfil a partir do email

        // envia flag da busca ao cliente
        char existe = (p == NULL) ? '0' : '1';
        write(connectionFD, &existe, 1);

          // se o perfil existe, envia dados do perfil
          if (p != NULL)
          {
            resp = enviaPerfil(connectionFD, p);
            infoLOG(opt,resp,buffer,ip); // [log do servidor]
          }
          else
			      infoLOG(opt,3,buffer,ip); // [log do servidor]

          perfilListFree(listaGeral); // libera memoria alocada
      }
      else if(opt[0] == '6')
      {
          Perfil *p = recebePerfil(connectionFD); // recebe perfil do cliente

          // [log do servidor]
          if(p == NULL)
          {
			        infoLOG(opt,0,"",ip); // [log do servidor]
              continue;
          }

          char email[50];
          memset(email,'\0',50);
          strcpy(email,p->email);

		      infoLOG(opt,2,email,ip);
          int res = addPerfil(p);  // tenta inserir o perfil ao registro

		      // [log do servidor]
          if(!res)
			      infoLOG(opt,3,email,ip);
          else
			      infoLOG(opt,1,email,ip);

          res = res+48;
          write(connectionFD, &res, 1); // envia resultado da insercao ao cliente
      }
      else if(opt[0] == '7')
      {
        // recebe email
  			char buffer[200],buffer2[200];
        memset(buffer,'\0',200);
  			memset(buffer2,'\0',200);

  			read(connectionFD, buffer, 200);

        if(resp == -1)
          continue;

        // recebe experiencia
        resp = (int)read(connectionFD, buffer2, 200);

        infoLOG(opt,2,buffer,ip); // [log do servidor]

        if(resp == -1)
  				continue;

        int res = addExperiencia(buffer,buffer2); // tenta adicionar experiencia

        if(res == 2)
          infoLOG(opt,1,buffer2,ip); // [log do servidor]
        else if(res == 1)
          infoLOG(opt,3,buffer2,ip); // [log do servidor]
        else
          infoLOG(opt,0,buffer2,ip); // [log do servidor]

        res = res+48;                 // transforma em char
        write(connectionFD, &res, 1); // envia resultado ao cliente
      }
      else if(opt[0] == '8')
      {
  			// recebe email
  			char buffer[200];
  			memset(buffer,'\0',200);
  			resp = (int)read(connectionFD, buffer, 200);

  			if(resp == -1)
  				continue;

  			infoLOG(opt,2,buffer,ip);        // [log do servidor]
				int res = removerPerfil(buffer); // tenta remover perfil do registro
				infoLOG(opt,res,buffer,ip);      // [log do servidor]
        res = res+48;                    // transforma em char
		    write(connectionFD, &res, 1);    // envia resultado ao cliente
      }
    }
}

void getServerIP() // mostra ip da maquina (apenas para facilitar testes)
{
	system("curl --output index.txt --url https://icanhazip.com");
	system("clear");
	FILE *iptxt = fopen("index.txt","r");
	if(iptxt != NULL)
	{
		char myip[16];
		fscanf(iptxt,"%s",myip);
		fclose(iptxt);
		remove("index.txt");
		PRINT2C("IP do servidor: ",CLW,CLY,"<< %s >> \n", myip);
	}
}
int getServerInfo(char *ip) // le arquivo de configuracao do servidor
{
	// config [le ip e porta do arquivo]
	FILE *config = fopen(CONFIG,"r");
	int port = 9000;
	char str[12];

	if(config != NULL)
	{
		fscanf(config,"%s",ip);
		fscanf(config,"%s",str);
		port = (int)strtol(str,NULL,10);
		fclose(config);
	}
	PRINT2C("porta do servidor: ",CLW,CLY,"<< %d >> \n", port);
	return port;
}
/* ----------------------------------------------------- MAIN ------------------------------------------------------ */
int main()
{
	// apenas para ver o ip da maquina (servidor)
	getServerIP();

	// le ip e porta definidos no arquivo de configuracao
	char ip[16] = "0.0.0.0";        // valor (default)
	int port = getServerInfo(ip);   // valor default é 9000

	int socketFD = socket(AF_INET, SOCK_STREAM,IPPROTO_TCP); // cria file descriptor do socket TCP
	SockAddr_in server;												   // estrutura do socket
	socketLOG("SBL", socketFD, "socket ", ""); 		   // verifica criação do socket

	/* atribui valores à estrutura do socket */
	server.sin_family = AF_INET;            // AF_INET é a familia de protocolos do IPV4
	server.sin_addr.s_addr = inet_addr(ip); // 0.0.0.0 (qualquer ip na rede pode enviar ao servidor)
	server.sin_port = htons(port);          // porta do servidor

	int do_bind = bind(socketFD, (SockAddr*)&server, sizeof(server)); // vincula um nome ao socket
	socketLOG("SBL", do_bind, "bind   ", ""); 			  // verifica bind

	int do_listen = listen(socketFD, 5); 					// coloca socket à espera de uma conexão
	socketLOG("SBL", do_listen, "listen ", "");	// verifica listen

	int pid, connectionFD; // process id e fd da conexao
	SockAddr_in client;    // estrutura do socket

    for(;;)
    {
		    /* aceita cliente */
        int tam = sizeof(client);
        connectionFD = accept(socketFD, (SockAddr*)&client, &tam); // aceita conexão de um cliente

		    /* separa em dois processos */
        pid = fork();
        if(pid==0)
        {
			/* verifica accept */
            if (connectionFD == -1)
				socketLOG("A", 0, "accept ", inet_ntoa(client.sin_addr));
            else
            {
				socketLOG("A", 1, "accept ", inet_ntoa(client.sin_addr));
				comunicacao(connectionFD, inet_ntoa(client.sin_addr)); // Realiza comunicação cliente-servidor.
				socketLOG("A",2,"", inet_ntoa(client.sin_addr));
            }
            break;
        }
		close(connectionFD); // Após terminar a comunicacao, fecha o socket da conexao
    }
    return 0;
}
