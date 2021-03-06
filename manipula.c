#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "queue.h"

#define GRAUMINIMO 2
#define MIN_NO GRAUMINIMO - 1
#define MAX_NO 2 * (GRAUMINIMO)-1
#define END_POINT 4 * (GRAUMINIMO)-1

typedef struct {
    int chave;
    char nome[20];
    int idade;
} RegistroArquivoDados;

typedef struct {
    int chave;
    int apontador;
} No;

typedef struct {
    int ocupados;
    No nos[END_POINT];
} RegistroArquivoArvore;

typedef struct {
    int proximoDadosLivre;
    int proximoArvoreLivre;
    int deslocamentoRaiz;
} Controle;

void abreArquivo(FILE **dados, FILE **arvore, Controle *c) {
  *dados = fopen("dados", "r+");
  if (*dados == NULL) {
    *dados = fopen("dados", "w+");
    if (*dados == NULL) {
      c->proximoArvoreLivre = -1;
      return;
    }
  }

  *arvore = fopen("arvore", "r+");
  if (*arvore == NULL) {
    *arvore = fopen("arvore", "w+");
    if (*arvore == NULL) {
      c->proximoArvoreLivre = -1;
      return;
    } else {
      c->proximoArvoreLivre = 0;
      c->proximoDadosLivre = 0;
      c->deslocamentoRaiz = -1;
      fseek(*arvore, 0, SEEK_SET);
      if (!fwrite(c, sizeof(*c), 1, *arvore)) {
        c->proximoArvoreLivre = -1;
        return;
      }

      return;
    }
  }

  fseek(*arvore, 0, SEEK_SET);
  fread(c, sizeof(*c), 1, *arvore);
}

void setRaiz(Controle *c, FILE **arvore) {
  if (c->proximoDadosLivre == 0) {
    RegistroArquivoArvore raiz;
    raiz.ocupados = 0;
    for (int i = 0; i < END_POINT; i++) {
      raiz.nos[i].apontador = -1;
      raiz.nos[i].chave = -1;
    }

    fseek(*arvore, sizeof(*c), SEEK_SET);
    if (!fwrite(&raiz, sizeof(raiz), 1, *arvore)) {
      c->deslocamentoRaiz = -1;
      return;
    }

    c->deslocamentoRaiz = 0;
    c->proximoArvoreLivre = 1;
  }
}

void fechaArquivo(FILE **arquivo) { fclose(*arquivo); }

void imprimir(RegistroArquivoDados registro) {
  printf("chave: %d", registro.chave);
  printf("\n%s", registro.nome);
  printf("\n%d", registro.idade);
  printf("\n");
}

void splitNode(FILE **arvore, Controle *c, int deslocamentoFilho, int deslocamentoPai, RegistroArquivoArvore *registro) {
  RegistroArquivoArvore pai;
  if (deslocamentoPai != -1) {
    fseek(*arvore, sizeof(*c) + deslocamentoPai * sizeof(pai), SEEK_SET);
    fread(&pai, sizeof(pai), 1, *arvore);
  } else {
    deslocamentoPai = c->proximoArvoreLivre;
    for (int i = 0; i < END_POINT; i++) {
      pai.nos[i].apontador = -1;
      pai.nos[i].chave = -1;
    }
    pai.ocupados = 0;
    c->deslocamentoRaiz = deslocamentoPai;
    c->proximoArvoreLivre++;
  }

  RegistroArquivoArvore filho;
  fseek(*arvore, sizeof(*c) + deslocamentoFilho * sizeof(filho), SEEK_SET);
  fread(&filho, sizeof(filho), 1, *arvore);

  double maxNo = MAX_NO;
  int mid = ceil(maxNo / 2) - 1;
  int deslocamentoNovoFilho = c->proximoArvoreLivre;
  c->proximoArvoreLivre++;

  No esquerda;
  esquerda.apontador = deslocamentoFilho;
  esquerda.chave = -1;

  No direita;
  direita.apontador = deslocamentoNovoFilho;
  direita.chave = -1;

  int i = (pai.ocupados * 2) - 1;
  int z = i;
  while (z >= 1) {
    if (filho.nos[(mid * 2) + 1].chave > pai.nos[z].chave) {
      break;
    }
    z -= 2;
    i = z;
  }

  No nomedio = filho.nos[(mid * 2) + 1];
  int chave;
  if (i == (pai.ocupados * 2) - 1) {
    // Novo maior numero
    chave = i + 2;
  } else {
    // Est?? no meio ou novo menor numero
    for (int j = (pai.ocupados * 2) - 1; j >= i; j -= 2) {
      if(j == (pai.ocupados * 2) - 1) {
        pai.nos[j + 3] = pai.nos[j + 1];
      }
      pai.nos[j + 2] = pai.nos[j];
      pai.nos[j + 1] = pai.nos[j - 1];
    }
    chave = (i < 0) ? 1 : i;
  }

  pai.nos[chave] = nomedio;
  pai.nos[chave - 1] = esquerda;
  pai.nos[chave + 1] = direita;
  pai.ocupados++;

  RegistroArquivoArvore novoFilho;
  novoFilho.ocupados = 0;
  for (int i = 0; i < END_POINT; i++) {
    novoFilho.nos[i].apontador = -1;
    novoFilho.nos[i].chave = -1;
  }

  int index = 0;
  for (int i = (mid + 1) * 2; i < END_POINT; i++) {
    novoFilho.nos[index] = filho.nos[i];
    if (filho.nos[i].chave != -1) {
      novoFilho.ocupados++;
    }
    index++;
  }

  for (int i = (mid * 2) + 1; i < END_POINT; i++) {
    if(filho.nos[i].chave != -1) {
      filho.ocupados--;
    }

    filho.nos[i].apontador = -1;
    filho.nos[i].chave = -1;
  }

  // Atualizando pai na mem??ria
  fseek(*arvore, sizeof(*c) + deslocamentoPai * sizeof(pai), SEEK_SET);
  fwrite(&pai, sizeof(pai), 1, *arvore);

  // Atualizando filho na mem??ria
  fseek(*arvore, sizeof(*c) + deslocamentoFilho * sizeof(filho), SEEK_SET);
  fwrite(&filho, sizeof(filho), 1, *arvore);

  // Criando novo filho na mem??ria
  fseek(*arvore, sizeof(*c) + deslocamentoNovoFilho * sizeof(novoFilho), SEEK_SET);
  fwrite(&novoFilho, sizeof(novoFilho), 1, *arvore);

  // Copiando informa????o do pai para o registro corrente
  for (int i = 0; i < END_POINT; i++) {
    registro->nos[i] = pai.nos[i];
  }
  registro->ocupados = pai.ocupados;
}

int buscaChaveCadastro(int chave, FILE **arvore, Controle *c) {
  RegistroArquivoArvore registro;
  fseek(*arvore, sizeof(*c) + c->deslocamentoRaiz * sizeof(registro), SEEK_SET);
  fread(&registro, sizeof(registro), 1, *arvore);

  if (registro.ocupados == 0) {
    registro.nos[1].chave = chave;
    registro.nos[1].apontador = c->proximoDadosLivre;
    registro.ocupados++;
    fseek(*arvore, sizeof(*c) + c->deslocamentoRaiz * sizeof(registro), SEEK_SET);
    fwrite(&registro, sizeof(registro), 1, *arvore);
    return 0;
  }

  int deslocamento = -2;
  int deslocamentoPai = -1;
  int noAtual = c->deslocamentoRaiz;
  while (deslocamento == -2) {
    if (registro.ocupados == MAX_NO) {
      splitNode(arvore, c, noAtual, deslocamentoPai, &registro);
    }

    int indice1 = (registro.ocupados * 2) - 1;
    int z = indice1;
    while (z >= 1) {
      if (registro.nos[z].chave <= chave) {
        break;
      }
      z -= 2;
      indice1 = z;
    }

    if (indice1 < 0) {
      indice1 = 1;
    }

    if (registro.nos[indice1].chave == chave) {
      deslocamento = -1;
      continue;
    }

    int proximoNo = registro.nos[indice1 + 1].apontador;
    if (registro.nos[indice1].chave > chave) {
      proximoNo = registro.nos[indice1 - 1].apontador;
    }

    if (proximoNo == -1) {
      deslocamento = c->proximoDadosLivre;

      int indice2 = (registro.ocupados * 2) - 1;
      int x = indice2;
      while (x >= 1) {
        if (registro.nos[x].chave < chave) {
          break;
        }
        x -= 2;
        indice2 = x;
      }

      int indice;
      if (indice2 == (registro.ocupados * 2) - 1) {
        // Novo maior numero
        indice = indice2 + 2;
      } else {
        // Est?? no meio ou ?? o novo menor n??mero
        for (int indice4 = (registro.ocupados * 2) - 1; indice4 > indice2; indice4 -= 2) {
          if(indice4 == (registro.ocupados * 2) - 1) {
            registro.nos[indice4 + 3] = registro.nos[indice4 + 1];
          }
          registro.nos[indice4 + 2] = registro.nos[indice4];
          registro.nos[indice4 + 1] = registro.nos[indice4 - 1];
        }
        indice = (indice2 < 0) ? 1 : indice2 + 2;
      }

      No novoNo;
      novoNo.apontador = deslocamento;
      novoNo.chave = chave;
      registro.nos[indice] = novoNo;
      registro.ocupados++;

      fseek(*arvore, sizeof(*c) + noAtual * sizeof(registro), SEEK_SET);
      fwrite(&registro, sizeof(registro), 1, *arvore);
      continue;
    }

    deslocamentoPai = noAtual;
    noAtual = proximoNo;
    fseek(*arvore, sizeof(*c) + proximoNo * sizeof(registro), SEEK_SET);
    fread(&registro, sizeof(registro), 1, *arvore);
  }

  return deslocamento;
}

int buscaChaveConsulta(int chave, FILE **arvore, Controle *c) {
  RegistroArquivoArvore registro;
  fseek(*arvore, sizeof(*c) + c->deslocamentoRaiz * sizeof(registro), SEEK_SET);
  fread(&registro, sizeof(registro), 1, *arvore);

  if (registro.ocupados == 0) {
    return -1;
  }

  int deslocamento = -2;
  int deslocamentoPai = -1;
  int noAtual = c->deslocamentoRaiz;
  while (deslocamento == -2) {
    if (registro.ocupados == MAX_NO) {
      splitNode(arvore, c, noAtual, deslocamentoPai, &registro);
    }

    int i = (registro.ocupados * 2) - 1;
    int z = i;
    while (z >= 1) {
      if (registro.nos[z].chave <= chave) {
        i = z;
        break;
      }
      z -= 2;
    }

    if (i < 0) {
      i = 1;
    }

    if (registro.nos[i].chave == chave) {
      deslocamento = registro.nos[i].apontador;
      continue;
    }

    int proximoNo = registro.nos[i + 1].apontador;
    if (registro.nos[i].chave > chave) {
      proximoNo = registro.nos[i - 1].apontador;
    }

    if (proximoNo == -1) {
      deslocamento = -1;
      continue;
    }

    deslocamentoPai = noAtual;
    noAtual = proximoNo;
    fseek(*arvore, sizeof(*c) + proximoNo * sizeof(registro), SEEK_SET);
    fread(&registro, sizeof(registro), 1, *arvore);
  }

  return deslocamento;
}

void cadastrar(FILE **dados, FILE **arvore, Controle *c) {
  RegistroArquivoDados r;
  scanf("%d%*c", &r.chave);

  fgets(r.nome, 20, stdin);
  // Limpando enter final do buffer
  size_t ln = strlen(r.nome) - 1;
  if (r.nome[ln] == '\n') r.nome[ln] = '\0';

  scanf("%d%*c", &r.idade);

  int deslocamento = buscaChaveCadastro(r.chave, arvore, c);
  if (deslocamento == -1) {
    printf("chave ja existente: %d\n", r.chave);
    return;
  }

  fseek(*dados, deslocamento * sizeof(r), SEEK_SET);
  if (fwrite(&r, sizeof(r), 1, *dados)) {
    c->proximoDadosLivre++;
    printf("insercao com sucesso: %d\n", r.chave);
  }
}

void consultar(FILE **dados, FILE **arvore, Controle *c) {
  int chave;
  scanf("%d%*c", &chave);

  int deslocamento = buscaChaveConsulta(chave, arvore, c);
  if (deslocamento == -1) {
    printf("chave nao encontrada: %d\n", chave);
    return;
  }

  RegistroArquivoDados r;
  fseek(*dados, deslocamento * sizeof(r), SEEK_SET);
  fread(&r, sizeof(r), 1, *dados);
  imprimir(r);
}

void imprimeNo(int deslocamento, FILE **arvore){
  RegistroArquivoArvore no;

  fseek(*arvore, sizeof(Controle) + deslocamento * sizeof(no), SEEK_SET);
  fread(&no, sizeof(no), 1, *arvore);

  for (int i = 0; i < END_POINT; i++) {
    if (no.nos[i].apontador != -1 && no.nos[i].chave == -1) {
      imprimeNo(no.nos[i].apontador, arvore);
      continue;
    }
    
    if (no.nos[i].chave != -1) {
      printf("%d\n", no.nos[i].chave);
    }
  }
}

int imprimeNoLargura(int index, int offset, int deslocamento, Queue *queue, FILE **arvore) {
RegistroArquivoArvore no;

  fseek(*arvore, sizeof(Controle) + deslocamento * sizeof(no), SEEK_SET);
  fread(&no, sizeof(no), 1, *arvore);

  printf("No: %d:", index);
  int usados = 0;
  int apontadores = index != 1 ? index + (offset-1) : index + offset;
  for (int i = 0; i <= no.ocupados * 2; i++) {
    if (no.nos[i].chave == -1) {
      char *apontador = (char*) malloc(100);
      
      if (no.nos[i].apontador != -1) {
        apontadores++;
        usados++;
        snprintf(apontador, 100, "%d", apontadores);
        enqueue(queue, no.nos[i].apontador);
      } else {
        strcpy(apontador, "null");
      }

      printf(" apontador: %s", apontador);
      free(apontador);
    }

    if (no.nos[i].chave != -1) {
      printf(" chave: %d", no.nos[i].chave);
    }
  }
  printf("\n");
  return usados;
}

void imprimeArvore(FILE **arvore, Controle *c) {
  Queue* queue = createQueue(c->proximoArvoreLivre+1);
  enqueue(queue, c->deslocamentoRaiz);

  int index = 1;
  int usados = 0;
  while (!isEmpty(queue)) {
    int deslocamento = dequeue(queue);

    int tst = 0;
    if (deslocamento != c->deslocamentoRaiz) {
      tst = 1;
    }

    usados = imprimeNoLargura(index, usados, deslocamento, queue, arvore) + tst;
    index++;
  }
}

void imprimeOrdem(FILE **arvore, Controle *c) {
  RegistroArquivoArvore arvoreRaiz;

  fseek(*arvore, sizeof(*c) + c->deslocamentoRaiz * sizeof(arvoreRaiz), SEEK_SET);
  fread(&arvoreRaiz, sizeof(arvoreRaiz), 1, *arvore);

  for (int i = 0; i < END_POINT; i++) {
    if (arvoreRaiz.nos[i].apontador != -1 && arvoreRaiz.nos[i].chave == -1) {
      imprimeNo(arvoreRaiz.nos[i].apontador, arvore);
      continue;
    }

    if (arvoreRaiz.nos[i].chave != -1) {
      printf("%d\n", arvoreRaiz.nos[i].chave);
    }
  }
}

void taxaDeOcupacao(FILE **arvore, Controle *c) {
  RegistroArquivoArvore arvoreRaiz;

  fseek(*arvore, sizeof(*c) + c->deslocamentoRaiz * sizeof(arvoreRaiz), SEEK_SET);
  fread(&arvoreRaiz, sizeof(arvore), 1, *arvore);

  if (arvoreRaiz.ocupados == 0) {
    printf("??rvore vazia\n");
    return;
  }

  float qntRegistros = (float) c->proximoDadosLivre;
  float qntNos = (float) c->proximoArvoreLivre;
  printf("taxa de ocupacao: %.1f\n", qntRegistros / (qntNos * MAX_NO));
}

int main(void) {
  FILE *pont_dados;
  FILE *pont_arvore;
  char opcao;

  Controle controle;
  abreArquivo(&pont_dados, &pont_arvore, &controle);
  if (controle.proximoArvoreLivre == -1) {
    printf("Erro na abertura dos arquivos!");
    exit(-1);
  }

  setRaiz(&controle, &pont_arvore);
  if (controle.deslocamentoRaiz == -1) {
    printf("Erro ao ler a raiz!");
    exit(-1);
  }

  do {
    scanf("%c%*c", &opcao);
    switch (opcao) {
      case 'e': {
        break;
      }
      case 'c': {
        // Deve consultar a chave
        consultar(&pont_dados, &pont_arvore, &controle);
        break;
      }
      case 'i': {
        // Deve inserir o registro
        cadastrar(&pont_dados, &pont_arvore, &controle);
        break;
      }
      case 'p': {
        // Deve imprimir a arvore
        imprimeArvore(&pont_arvore, &controle);
        break;
      }
      case 'o': {
        // Deve imprimir as chaves em ordem crescente
        imprimeOrdem(&pont_arvore, &controle);
        break;
      }
      case 't': {
        // Deve imprimir a taxa de ocupa????o da arvore
        taxaDeOcupacao(&pont_arvore, &controle);
        break;
      }
      default:
        break;
    }
  } while (opcao != 'e');
  // Salvando altera????es da struct de controle
  fseek(pont_arvore, 0, SEEK_SET);
  fwrite(&controle, sizeof(controle), 1, pont_arvore);

  fechaArquivo(&pont_dados);
  fechaArquivo(&pont_arvore);
  return (0);
}
