/* ���α׷���: SubwayCalc.c �����Լ��� �̿��� ���� ���� ���α׷�*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <limits.h>

#define TRUE 1
#define FALSE 0
#define INF	9999
#define MAX_VERTICES 552 //����(��)�� ��

int Sub_Time=0; // �̵� �ð� �������� (ȯ�� �ð� ����)
int Sub_Cnt=0; // ������ Ƚ�� �������� (ȯ�� �ð� ����)
int IC_Time=0; // ȯ�� �ð� ��������
int option=1; // �켱��� ���� �������� (1=�ִܰŸ�, 2=�ּ�ȯ��)
int trans_done=0; // ���� ���� ���� (1�̸� ����)

const int R=MAX_VERTICES; // ��� ���
const int C=MAX_VERTICES; // ��� ���
const int IC=150; // ȯ�¿� ���� ���

typedef struct sublist{ // ���� ����ü
    char name[30]; // ����
    char code[10]; // �� �ڵ�
    int num; // ȣ�� ��ȣ
} sublist;

typedef struct element{ // ������� ����ü
    char from[10]; // �����
    char to[10]; // ������
    int data; // ���� �ð�
    int datao; // ȯ���� ��� ���� �ð��� ����
    int ic; // ȯ�� ����
} element;

int distance[MAX_VERTICES]; // ���� �����κ����� �ִ� ��� �Ÿ�
int found[MAX_VERTICES]; // �湮�� �� ǥ��
int path[MAX_VERTICES]; // �ִܰŸ� ������ ��ġ�� ������ ����
int check[MAX_VERTICES];// �� ������ ���� ���� ǥ��


char *csvLists[20]={"1ȣ��","1����","2ȣ��","2����","3ȣ��","4ȣ��",
                    "5ȣ��","5����","6ȣ��","7ȣ��","8ȣ��","9ȣ��",
                    "�д缱","��õ1��","�߾Ӽ�","���ἱ","���Ǽ�","����ö��","���̸�","ȯ������"}; //�� ���ϸ� ����


/*
* �Լ���: makeArray()
* �� �� : ����
* ������: element**
* �� �� : ����ü ����� ���� �����ϴ� �Լ�
*/
element** makeArray(){
    element **arr = malloc(sizeof(element *) * R);   // ���� �����Ϳ� (element ������ ũ�� * row)��ŭ ���� �޸� �Ҵ�. �迭�� ����
    for (int i = 0; i < R; i++){            // ���� ũ�⸸ŭ �ݺ�
        arr[i] = malloc(sizeof(element) * C);    // (element�� ũ�� * col)��ŭ ���� �޸� �Ҵ�. �迭�� ����
    }
    return arr;
}


/*
* �Լ���: initArray(arr)
* �� �� : element**
* ������: void
* �� �� : ����ü ��� ���θ� �ʱ�ȭ�ϴ� �Լ�
*/
void initArray(element** arr){
    for (int i = 0; i < R; i++){
        for (int j = 0; j < C; j++){
                strcpy(arr[i][j].from,""); // ����� ���
                strcpy(arr[i][j].to,""); // ������ ���
                arr[i][j].data=9999; // ����ġ 9999�� �ʱ�ȭ
                arr[i][j].datao=9999; // ����ġ 9999�� �ʱ�ȭ
                arr[i][j].ic=FALSE; // ȯ�� X�� �ʱ�ȭ
            }
    }
}
/*
* �Լ���: killArray(arr)
* �� �� : element**
* ������: void
* �� �� : ����ü ����� �Ҵ� �����ϴ� �Լ�
*/
void killArray(element** arr){
    for (int i = 0; i < R; i++) {
        free(arr[i]);
    }
    free(arr);
}

/*
* �Լ���: readCSV(i)
* �� �� : int
* ������: FILE*
* �� �� : i��° ������ �о FILE*�� �����ϴ� �Լ�
*/
FILE* readCSV(int i){
        char fileDir[50] = "./data/"; // ���� ���
        strcat(fileDir, csvLists[i]); // i��° ���ϸ��� ���� ��ο� ����
        strcat(fileDir, ".csv"); // .csv�� ���� ��ο� ����
        FILE *stream = fopen(fileDir, "r"); // ���� ����� ������ �ҷ���
        if (stream != NULL){ //������ ������� ������
            return stream;
        }
        else{ //������ ����ְų� ������ �ִٸ�
            printf("Failed to load %s !!!", fileDir); //���� ���
            exit(1); //����
        }
}

/*
* �Լ���: readSubInfo(subinfo[])
* �� �� : sublist
* ������: void
* �� �� : ����ö ���� ����ü ������ ������ �ڵ带 ä��� �Լ�
*/
void readSubInfo(sublist subinfo[]){
        char buf[2048];
        FILE *stream = readCSV(18);
        char *line, *tmp;
        line = fgets(buf, 2048, stream); // ���� �̸� ���� (�Ⱦ��� ĭ �Ÿ���)
        int idx=0;
        while ((line = fgets(buf, 2048, stream))!=NULL) {
            int i=0;
            tmp = strtok(buf, ","); //
            while (tmp != NULL){
                if (i == 0){
                    strcpy(subinfo[idx].code, tmp);
                }
                else if (i == 1){
                    if(idx!=R) // ���������� �ƴϸ�
                        tmp[strlen(tmp) - 1] = '\0'; // \n�� �������ش�.
                    strcpy(subinfo[idx].name, tmp);
                }
                i++;
                tmp = strtok(NULL, "");
            }
            idx++;
        }

        fclose(stream);
}

/*
* �Լ���: readSubArray(arr,subinfo[])
* �� �� : element** , sublist
* ������: void
* �� �� : ����ü ����� ������ ����ö ���� ����ü ������ ȣ�� �ε����� ä��� �Լ�
*/
void readSubArray(element** arr,sublist subinfo[]){
    int cRow=0; // ���� �࿭ ���� �����ϴ� ����
    char *tmp, *line; //�ٰ� �ܾ� ���� ����
    char buf[2048]; // ���� ����

    //������� ���� �Է�
    for (int i = 0; i < 18; i++){
        FILE *stream = readCSV(i);
        char *line, *tmp;
        line = fgets(buf, 2048, stream); // ���� �̸� ���� (�Ⱦ��� ĭ �Ÿ���)
        int tmpR = 0, tmpC = 0; //�ӽ� �� ��
        while ((line = fgets(buf, 2048, stream)) != NULL){ //���پ� ����
            tmpC = 0; //�ӽ� ���� 0���� �ʱ�ȭ
            tmp = strtok(line, ","); // ,�� ������ line�� ù �ܾ ����
            tmp = strtok(NULL, ","); //���� �ܾ ���� ( ù��° �� �ڵ� �پ�ѱ� )
            while (tmp != NULL){ // ���� �ܾ NULL�� �ƴϸ�
                subinfo[cRow + tmpR].num=i; // ~ȣ�� �ε����� ���� ����ü�� ����
                arr[cRow + tmpR][cRow + tmpC].data = atoi(tmp); // ����ü ��Ŀ� cRow + tmpR,C�� �ε����� �ش� �� ����
                tmp = strtok(NULL, ","); //���� �ܾ ����
                tmpC++; // �ӽ� �� �߰�
            }
            tmpR++; // �ӽ��� �߰�
        }
        cRow = cRow + tmpR; // ���� ���� �ӽ� �ุŭ �����༭ ����
        fclose(stream); //���� �ݱ�
    }

    //���⼭���ʹ� ����ü�� from to ����
    for (int i = 0; i < R; i++){
        for (int j = 0; j < R; j++){
            strcpy(arr[i][j].from,subinfo[i].code); //arr�� i�࿡ ��� ���ڵ� ����
            strcpy(arr[i][j].to,subinfo[j].code); //arr�� j�࿡ ���� ���ڵ� ����
        }
    }
    
    //���⼭���ʹ� ȯ���ϴ� �ڵ��� �ε��� ����
    FILE *stream = readCSV(19); // ȯ������ �б�
    int tmpCnt = 0;
    int tmpIC[IC];
    line = fgets(buf, 2048, stream);
    tmp = strtok(line, ",");
    //tmp = strtok(NULL, ","); //ȯ�� �ܾ� ��ŵ
    while (tmp != NULL){
        if(tmpCnt==IC-1) // ������ �ܾ��
                tmp[strlen(tmp) - 1] = '\0'; // \n�� �������ش�.
        for (int i = 0; i < R; i++){
            if (strcmp(tmp, subinfo[i].code) == 0){ // �� �ڵ尡 �����ϴٸ�
                tmpIC[tmpCnt] = i;// �ε��� ����
                //printf("%d) %s %d �ε��� [%s]\n", tmpCnt,tmp,i,subinfo[i].name);
                tmpCnt++; // ����
            }
        }
        tmp = strtok(NULL, ",");//���� �б�
    }

    //���⼭���ʹ� ȯ�������� ������Ŀ� ����
    tmpCnt=0;
    int i=0;
    int j=0;
    int m;
    int n;
    while ((line = fgets(buf, 2048, stream)) != NULL){ //���� �� �б�
        j=0;
        tmp = strtok(line, ",");
        tmp = strtok(NULL, ",");
        while (tmp != NULL){
            if(j==IC-1) // ������ �ܾ��
                tmp[strlen(tmp) - 1] = '\0'; // \n�� �������ش�.
            m = tmpIC[i]; // �� �ε��� �ҷ����� 
            n = tmpIC[j]; // �� �ε��� �ҷ����� 
            if (atoi(tmp) != 9999&&atoi(tmp) != 0){ // �ش� ����ġ�� �湮�� �� �ִ� ���̸�
                //printf("%d(%s), ",n,subinfo[n].name);
                arr[m][n].data = atoi(tmp); // �ش� �� ����
                arr[m][n].datao = atoi(tmp); // �ش� �� ����
                arr[m][n].ic = TRUE; // �ش� ������ ȯ���̹Ƿ� TRUE
            }
            if (j<IC)
                j++; // ���� ��
            tmp = strtok(NULL, ",");
        }
        //printf("[%d:%s]\n",m,subinfo[m].name);
        if (i<IC-2)
            i++; // ���� ��
    }
    fclose(stream);
}

/*
* �Լ���: subChk(subinfo[],chk[])
* �� �� : sublist,char
* ������: int
* �� �� : ���� �ε����� Ž���ϴ� �Լ�
*/
int subChk(sublist subinfo[],char chk[]){
    for (int i=0;i<R;i++){
        if (strcmp(chk,subinfo[i].name)==0) //�ش��ϴ� ���ڿ��� ã����
            return i; //�ش� �ε��� ���
    }
    return -1; //��Ͽ� ������ -1 ���
}

/*
* �Լ���: rand_num(num)
* �� �� : int
* ������: int
* �� �� : 1~ �ش� ������ ����ȭ�ϴ� �Լ�
*/
int rand_num(int num){
    srand(time(NULL)); //���� ����ȭ
    return (rand() % num) + 1;
}

/*
* �Լ���: choose(distance[], n, found[])
* �� �� : int 3��
* ������: int
* �� �� : �ð��� ���� ���� �ε����� �����ϴ� �Լ�
*/
int choose(int distance[], int n, int found[]){
	int i, min, minpos;
	min = INT_MAX; // �ִ�� ����
	minpos = -1;
	for (i = 0; i<n; i++)
		if (distance[i]< min && !found[i]) { //�湮���� �ʾҰ� �ּҺ��� �� ������
			min = distance[i]; // �Ÿ� �Է�
			minpos = i; // �ε��� �Է�
		}
	return minpos;
}

/*
* �Լ���: shortest_path(arr, start)
* �� �� : element**, int
* ������: void
* �� �� : �ִܰŸ��� Ž���ϴ� �Լ�
*/
void shortest_path(element** arr, int start){
	int i, u, w;
    
	for (i = 0; i<R; i++){  /* �ʱ�ȭ */
		distance[i] = arr[start][i].data;
		found[i] = FALSE;
		path[i] = start;
	}
	path[start] = -1;
    if (trans_done==0){// ����ȭ ���� ���� 0�ΰ��
        for (i = 0; i<R; i++){ // ������� Ž��
		    for (int j = 0; j<R; j++){
                if(arr[i][j].ic==1){// ȯ�¿��ϰ��
                    arr[i][j].data=rand_num(arr[i][j].datao); //ȯ���� ����ġ�� ����ȭ
                    if(option==2) arr[i][j].data+=1000; //�ּ�ȯ���� ��� ����ġ�� ũ�� ������Ŵ
                }
	        }
	    }
    }
    
	found[start] = TRUE;    /* ���� ���� �湮 ǥ�� */
	distance[start] = 0;
	for (i = 0; i<R-1; i++) {
		u = choose(distance, R, found);
		found[u] = TRUE; // �ش� ������ �湮 ǥ��
		for (w = 0; w<R; w++) 
			if (!found[w]) // �湮������ ���ٸ�
				if (distance[u] + arr[u][w].data < distance[w]){
					path[w]=u; //�ε��� ����
					distance[w] = distance[u] + arr[u][w].data; // ����ġ ����
				}
	}
}

/*
* �Լ���: print_path(int start, int end,sublist subinfo[],element** arr)
* �� �� : int, int, sublist, element**
* ������: void
* �� �� : path[]�� Ž���ϸ鼭 ������� �̵� �ð��� ����ϴ� �Լ�
*/
void print_path(int start, int end,sublist subinfo[],element** arr){
    //�ʱ�ȭ �ܰ�
    Sub_Time=0;
    IC_Time=0;
    Sub_Cnt=1;
	int i = end;
	int k = 0;
    int limit = 0;
    //�ʱ�ȭ �ܰ�

	int way[R];
    int Now;
	while (path[i] != -1){ // -1�� �����Ҷ����� �ε������� ����
		way[k++]=i; // �ε��� ����
		way[k++]=path[i]; // i �������� �̾����� �ε��� ����
		i = path[i]; // ����
	}
    //printf("(%d �ε���)", k-1);
    printf("\n<���>\n");
    //printf("(%d��)", Sub_Time);
    printf("-><%s> %s\n", csvLists[subinfo[way[k-1]].num],subinfo[way[k-1]].name);
	for(int q = k-1; q > limit; q=q-2){
        if(arr[way[q]][way[q-1]].ic==0){ 
            // ȯ���� �ƴҰ��
            Now=arr[way[q]][way[q-1]].data; //�ð��� �״�� ����
            Sub_Time+=Now;
            Sub_Cnt++;
            if(q<4||arr[way[q-2]][way[q-3]].ic==0){ // �������� ȯ�¿��� �ƴϸ� ����, ��Ʈ��Ŷ���� �߸��� ���� ����
                //printf("(%d��)", Sub_Time);
                printf("-><%s> %s\n", csvLists[subinfo[way[q-1]].num],subinfo[way[q-1]].name);
            }
        }
        else{
            // ȯ���� ���
            if(q<4||arr[way[q-2]][way[q-3]].ic==0){ // �������� ȯ�¿�(�ߺ�)�� �ƴϸ� ����, ��Ʈ��Ŷ���� �߸��� ���� ����
                Now=arr[way[q]][way[q-1]].data;
                if(option==2) Now=arr[way[q]][way[q-1]].data-1000; // �ּ�ȯ���ϰ�� ����ġ�� ������ �����Ƿ� 1000�� ����
                IC_Time+=Now;
                printf("-><ȯ�� : �ҿ�ð� %d ��> %s\n", Now,subinfo[way[q-1]].name);
            }
        }
	}
	printf("\n");
    if(option==1) printf("�ִܰŸ��� ����\n");
    if(option==2) printf("�ּ�ȯ������ ����\n");
    printf("�ҿ�ð� : %d (%d + ȯ�� �ҿ�ð� %d) ��\n",Sub_Time+IC_Time,Sub_Time,IC_Time);
    printf("������ �� : %d ��\n\n",Sub_Cnt);
}

/*
* �Լ���: calc_path(int start, int end,sublist subinfo[],element** arr)
* �� �� : int, int, sublist, element**
* ������: int
* �� �� : path[]�� Ž���ϸ鼭 �� �̵� �ð��� �����ϴ� �Լ�
*/
int calc_path(int start, int end,sublist subinfo[],element** arr){
    //�ʱ�ȭ �ܰ�
    shortest_path(arr, start); //�ش� �ε����� ���ͽ�Ʈ�� ����
    Sub_Time=0;
    IC_Time=0;
    Sub_Cnt=1;
	int i = end;
	int k = 0;
    int limit = 0;
    //�ʱ�ȭ �ܰ�

	int way[R];
    int Now;
	while (path[i] != -1){ // -1�� �����Ҷ����� �ε������� ����
		way[k++]=i; // �ε��� ����
		way[k++]=path[i]; // i �������� �̾����� �ε��� ����
		i = path[i]; // ����
	}
	for(int q = k-1; q > limit; q=q-2){
        if(arr[way[q]][way[q-1]].ic==0){ 
            // ȯ���� �ƴҰ��
            // ����ġ�� ���ϰ� ������ ī��Ʈ �ø�
            Now=arr[way[q]][way[q-1]].data;
            Sub_Time+=Now;
            Sub_Cnt++;
        }
        else{
            // ȯ���� ���
            // ����ġ�� ���ϰ� ������ ī��Ʈ �ø�
            if(q<4||arr[way[q-2]][way[q-3]].ic==0){ // �������� ȯ�¿�(�ߺ�)�� �ƴϸ� ����, ��Ʈ��Ŷ���� �߸��� ���� ����
                Now=arr[way[q]][way[q-1]].data;
                if(option==2) Now=arr[way[q]][way[q-1]].data; // �ּ�ȯ���ϰ�� ����ġ�� ������ ������ �񱳸� ���� 1000�� ���Ŵ� ����
                IC_Time+=Now;
            }
        }
	}
    return Sub_Time+IC_Time;
}

/*
* �Լ���: sub_find(element** subarray, sublist subinfo[])
* �� �� : element**, sublist
* ������: void
* �� �� : ��߿�, �������� �ִܰŸ�, �ּ�ȯ���� �Է¹ް� ���� �Լ����� ������ �Լ� (��ǻ� main)
*/
void sub_find(element** subarray,sublist subinfo[]){
    char sub1[100]; // ��߿� �Է�
    char sub2[100]; // ������ �Է�
    int sub1_idx; // ��߿� �ε���
    int sub2_idx; // ������ �ε���
    int ov_idx[R]; // ��߿� ȯ�� �ߺ� �ε��� ���� �迭
    int ov_idx2[R]; // ������ ȯ�� �ߺ� �ε��� ���� �迭
    int ov_time[R]; // ��߿� ȯ�� �ߺ� �ð� ���� �迭
    int curIdx=0; // ��߿� ȯ�� �ߺ� �ε��� �迭 �ε���
    int min_time=9999; // ��߿��� ȯ���ϰ�� ȯ�� ���� ���� �ִ��� �ð�
    int ov_ic[R][R]; // ��߿� �ӽ� ȯ�½ð� ���� �迭

    while(1){ // �� �̸��� �Է��ϴ� �κ� 
        printf("��߿��� �Է����ּ���: ");
        fgets(sub1,sizeof(sub1),stdin); //��߿� �Է�
        //sub1�� ������� �ʰ� 0��°�� \0�� �ƴϸ鼭 ���κ��� \n�ϰ��
        if(sub1 !=NULL && sub1[0]!='\0' && sub1[strlen(sub1)-1]=='\n')
            sub1[strlen(sub1)-1]='\0';

        printf("�������� �Է����ּ���: ");
        fgets(sub2,sizeof(sub2),stdin); //������ �Է� 
        //sub2�� ������� �ʰ� 0��°�� \0�� �ƴϸ鼭 ���κ��� \n�ϰ��
        if(sub2 !=NULL && sub2[0]!='\0' && sub2[strlen(sub2)-1]=='\n') 
            sub2[strlen(sub2)-1]='\0';

        if(strcmp(sub1,sub2)==0){ // ��߿��� �������� �����ϸ� ���� 1
            printf("����1 : ��߿��� �������� �����մϴ�! \n");
            printf("�ٽ� �Է����ּ���! \n");
            continue;
        }
        sub1_idx=subChk(subinfo,sub1);
        sub2_idx=subChk(subinfo,sub2);
        if (sub1_idx==-1||sub2_idx==-1){ // �ε����� �������� ������ ���� 2
            printf("����2 : ��߿��Ǵ� �������� ��Ȯ���� �ʽ��ϴ�! \n");
            printf("�ٽ� �Է����ּ���! \n");
            continue;
        }
        break;
    }
    while(1){ // �켱 ����� �Է��ϴ� �κ� 
        printf("���? 1. �ִܰ�� 2. �ּ�ȯ��\n");
        printf(": ");
        char Chk_num[5]; // �켱��� ����
        fgets(Chk_num,sizeof(Chk_num),stdin); //��߿� �Է�
        //sub1�� ������� �ʰ� 0��°�� \0�� �ƴϸ鼭 ���κ��� \n�ϰ��
        if(Chk_num !=NULL && Chk_num[0]!='\0' && Chk_num[strlen(Chk_num)-1]=='\n')
            Chk_num[strlen(Chk_num)-1]='\0';

        if(atoi(Chk_num)!=1&&atoi(Chk_num)!=2){ // ���� ������ ���� �ƴϸ� ���� 3
            printf("����3 : �ùٸ��� ���� ���Դϴ�! \n");
            printf("�ٽ� �Է����ּ���! \n");
            continue;
        }
        else if(atoi(Chk_num)==1){
            option=1;
        }
        else{
            option=2;
        }
        break;
    }

    for(int i=0;i<R;i++){ // ���, �������� ���������� Ȯ��
        for(int h=0;h<R;h++){
            if(strcmp(sub1,subinfo[i].name)==0&&strcmp(sub2,subinfo[h].name)==0){ // �̸��� �����ϴٸ�
                ov_idx[curIdx]=i; // �ش� ��߿� �ε����� ����
                ov_idx2[curIdx]=h; // �ش� ������ �ε��� ����
                ov_time[curIdx]=calc_path(i, h,subinfo,subarray); // �ش� ��߿� �ҿ�ð� ����
                if (ov_time[curIdx] < min_time){ //�ּҺ��� �������
                    // �ּ��� ���
                    min_time = ov_time[curIdx]; // i��° �ð� ����
                    sub1_idx = ov_idx[curIdx]; // i��° ��߿� �ε��� ����
                    sub2_idx = ov_idx2[curIdx]; // i��° ������ �ε��� ����
                    for(int j=0;j<R;j++){
                        for(int k=0;k<R;k++){
                            ov_ic[j][k]=subarray[j][k].data; //�ش� �ּ� ����ġ ����
                        }
                    }
                }
                curIdx++;
            }
        }
    }
    if(curIdx>1){ // ����� ���� �������ϰ��
        for(int j=0;j<R;j++){
            for(int k=0;k<R;k++){
                subarray[j][k].data=ov_ic[j][k]; //�����ߴ� �ּ� ����ġ�� �ٽ� �ҷ��ͼ� ����
            }
        }
    }
    trans_done=1; //����ȭ ����
    shortest_path(subarray, sub1_idx); // ���ͽ�Ʈ��
	print_path(sub1_idx, sub2_idx, subinfo, subarray); // ���� ��� �Լ�
}

int main(){
    element** subarray= makeArray();
    initArray(subarray);
    
    sublist subinfo[R];
    readSubInfo(subinfo);
    readSubArray(subarray,subinfo);
    
    sub_find(subarray,subinfo);

    killArray(subarray); //������� free
    printf("���α׷��� ����Ǿ����ϴ� \n");

    return 0;
}