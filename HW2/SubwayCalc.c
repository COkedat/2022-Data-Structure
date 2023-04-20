/* 프로그램명: SubwayCalc.c 스택함수를 이용한 스택 계산기 프로그램*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <limits.h>

#define TRUE 1
#define FALSE 0
#define INF	9999
#define MAX_VERTICES 552 //정점(역)의 수

int Sub_Time=0; // 이동 시간 전역변수 (환승 시간 제외)
int Sub_Cnt=0; // 정거장 횟수 전역변수 (환승 시간 제외)
int IC_Time=0; // 환승 시간 전역변수
int option=1; // 우선방식 여부 전역변수 (1=최단거리, 2=최소환승)
int trans_done=0; // 랜덤 종료 여부 (1이면 종료)

const int R=MAX_VERTICES; // 행렬 상수
const int C=MAX_VERTICES; // 행렬 상수
const int IC=150; // 환승역 개수 상수

typedef struct sublist{ // 역명 구조체
    char name[30]; // 역명
    char code[10]; // 역 코드
    int num; // 호선 번호
} sublist;

typedef struct element{ // 인접행렬 구조체
    char from[10]; // 출발지
    char to[10]; // 목적지
    int data; // 가는 시간
    int datao; // 환승일 경우 가는 시간의 원본
    int ic; // 환승 여부
} element;

int distance[MAX_VERTICES]; // 시작 역으로부터의 최단 경로 거리
int found[MAX_VERTICES]; // 방문한 역 표시
int path[MAX_VERTICES]; // 최단거리 역까지 거치는 노드들을 저장
int check[MAX_VERTICES];// 한 역으로 가는 역을 표시


char *csvLists[20]={"1호선","1지선","2호선","2지선","3호선","4호선",
                    "5호선","5지선","6호선","7호선","8호선","9호선",
                    "분당선","인천1선","중앙선","경춘선","경의선","공항철도","역이름","환승정보"}; //역 파일명 모음


/*
* 함수명: makeArray()
* 인 자 : 없음
* 리턴형: element**
* 설 명 : 구조체 행렬을 동적 생성하는 함수
*/
element** makeArray(){
    element **arr = malloc(sizeof(element *) * R);   // 이중 포인터에 (element 포인터 크기 * row)만큼 동적 메모리 할당. 배열의 세로
    for (int i = 0; i < R; i++){            // 세로 크기만큼 반복
        arr[i] = malloc(sizeof(element) * C);    // (element의 크기 * col)만큼 동적 메모리 할당. 배열의 가로
    }
    return arr;
}


/*
* 함수명: initArray(arr)
* 인 자 : element**
* 리턴형: void
* 설 명 : 구조체 행렬 내부를 초기화하는 함수
*/
void initArray(element** arr){
    for (int i = 0; i < R; i++){
        for (int j = 0; j < C; j++){
                strcpy(arr[i][j].from,""); // 출발지 비움
                strcpy(arr[i][j].to,""); // 목적지 비움
                arr[i][j].data=9999; // 가중치 9999로 초기화
                arr[i][j].datao=9999; // 가중치 9999로 초기화
                arr[i][j].ic=FALSE; // 환승 X로 초기화
            }
    }
}
/*
* 함수명: killArray(arr)
* 인 자 : element**
* 리턴형: void
* 설 명 : 구조체 행렬을 할당 해제하는 함수
*/
void killArray(element** arr){
    for (int i = 0; i < R; i++) {
        free(arr[i]);
    }
    free(arr);
}

/*
* 함수명: readCSV(i)
* 인 자 : int
* 리턴형: FILE*
* 설 명 : i번째 파일을 읽어서 FILE*로 리턴하는 함수
*/
FILE* readCSV(int i){
        char fileDir[50] = "./data/"; // 하위 경로
        strcat(fileDir, csvLists[i]); // i번째 파일명을 하위 경로에 붙임
        strcat(fileDir, ".csv"); // .csv를 하위 경로에 붙임
        FILE *stream = fopen(fileDir, "r"); // 하위 경로의 파일을 불러옴
        if (stream != NULL){ //파일이 비어있지 않으면
            return stream;
        }
        else{ //파일이 비어있거나 문제가 있다면
            printf("Failed to load %s !!!", fileDir); //오류 출력
            exit(1); //종료
        }
}

/*
* 함수명: readSubInfo(subinfo[])
* 인 자 : sublist
* 리턴형: void
* 설 명 : 지하철 역명 구조체 정보의 역명과 코드를 채우는 함수
*/
void readSubInfo(sublist subinfo[]){
        char buf[2048];
        FILE *stream = readCSV(18);
        char *line, *tmp;
        line = fgets(buf, 2048, stream); // 한줄 미리 읽음 (안쓰는 칸 거르기)
        int idx=0;
        while ((line = fgets(buf, 2048, stream))!=NULL) {
            int i=0;
            tmp = strtok(buf, ","); //
            while (tmp != NULL){
                if (i == 0){
                    strcpy(subinfo[idx].code, tmp);
                }
                else if (i == 1){
                    if(idx!=R) // 마지막줄이 아니면
                        tmp[strlen(tmp) - 1] = '\0'; // \n을 제거해준다.
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
* 함수명: readSubArray(arr,subinfo[])
* 인 자 : element** , sublist
* 리턴형: void
* 설 명 : 구조체 행렬의 정보와 지하철 역명 구조체 정보의 호선 인덱스를 채우는 함수
*/
void readSubArray(element** arr,sublist subinfo[]){
    int cRow=0; // 현재 행열 값을 저장하는 벼수
    char *tmp, *line; //줄과 단어 변수 선언
    char buf[2048]; // 버퍼 선언

    //인접행렬 정보 입력
    for (int i = 0; i < 18; i++){
        FILE *stream = readCSV(i);
        char *line, *tmp;
        line = fgets(buf, 2048, stream); // 한줄 미리 읽음 (안쓰는 칸 거르기)
        int tmpR = 0, tmpC = 0; //임시 행 열
        while ((line = fgets(buf, 2048, stream)) != NULL){ //한줄씩 읽음
            tmpC = 0; //임시 열을 0으로 초기화
            tmp = strtok(line, ","); // ,로 나눠진 line의 첫 단어를 읽음
            tmp = strtok(NULL, ","); //다음 단어를 읽음 ( 첫번째 역 코드 뛰어넘기 )
            while (tmp != NULL){ // 읽은 단어가 NULL이 아니면
                subinfo[cRow + tmpR].num=i; // ~호선 인덱스를 역명 구조체에 저장
                arr[cRow + tmpR][cRow + tmpC].data = atoi(tmp); // 구조체 행렬에 cRow + tmpR,C의 인덱스에 해당 값 저장
                tmp = strtok(NULL, ","); //다음 단어를 읽음
                tmpC++; // 임시 열 추가
            }
            tmpR++; // 임시행 추가
        }
        cRow = cRow + tmpR; // 현재 열에 임시 행만큼 더해줘서 저장
        fclose(stream); //파일 닫기
    }

    //여기서부터는 구조체의 from to 저장
    for (int i = 0; i < R; i++){
        for (int j = 0; j < R; j++){
            strcpy(arr[i][j].from,subinfo[i].code); //arr의 i행에 출발 역코드 저장
            strcpy(arr[i][j].to,subinfo[j].code); //arr의 j행에 도착 역코드 저장
        }
    }
    
    //여기서부터는 환승하는 코드의 인덱스 저장
    FILE *stream = readCSV(19); // 환승파일 읽기
    int tmpCnt = 0;
    int tmpIC[IC];
    line = fgets(buf, 2048, stream);
    tmp = strtok(line, ",");
    //tmp = strtok(NULL, ","); //환승 단어 스킵
    while (tmp != NULL){
        if(tmpCnt==IC-1) // 마지막 단어면
                tmp[strlen(tmp) - 1] = '\0'; // \n을 제거해준다.
        for (int i = 0; i < R; i++){
            if (strcmp(tmp, subinfo[i].code) == 0){ // 역 코드가 동일하다면
                tmpIC[tmpCnt] = i;// 인덱스 저장
                //printf("%d) %s %d 인덱스 [%s]\n", tmpCnt,tmp,i,subinfo[i].name);
                tmpCnt++; // 다음
            }
        }
        tmp = strtok(NULL, ",");//다음 읽기
    }

    //여기서부터는 환승정보를 인접행렬에 저장
    tmpCnt=0;
    int i=0;
    int j=0;
    int m;
    int n;
    while ((line = fgets(buf, 2048, stream)) != NULL){ //다음 줄 읽기
        j=0;
        tmp = strtok(line, ",");
        tmp = strtok(NULL, ",");
        while (tmp != NULL){
            if(j==IC-1) // 마지막 단어면
                tmp[strlen(tmp) - 1] = '\0'; // \n을 제거해준다.
            m = tmpIC[i]; // 행 인덱스 불러오기 
            n = tmpIC[j]; // 열 인덱스 불러오기 
            if (atoi(tmp) != 9999&&atoi(tmp) != 0){ // 해당 가중치가 방문할 수 있는 값이면
                //printf("%d(%s), ",n,subinfo[n].name);
                arr[m][n].data = atoi(tmp); // 해당 값 저장
                arr[m][n].datao = atoi(tmp); // 해당 값 저장
                arr[m][n].ic = TRUE; // 해당 지점은 환승이므로 TRUE
            }
            if (j<IC)
                j++; // 다음 열
            tmp = strtok(NULL, ",");
        }
        //printf("[%d:%s]\n",m,subinfo[m].name);
        if (i<IC-2)
            i++; // 다음 행
    }
    fclose(stream);
}

/*
* 함수명: subChk(subinfo[],chk[])
* 인 자 : sublist,char
* 리턴형: int
* 설 명 : 역명 인덱스를 탐색하는 함수
*/
int subChk(sublist subinfo[],char chk[]){
    for (int i=0;i<R;i++){
        if (strcmp(chk,subinfo[i].name)==0) //해당하는 문자열을 찾으면
            return i; //해당 인덱스 출력
    }
    return -1; //목록에 없으면 -1 출력
}

/*
* 함수명: rand_num(num)
* 인 자 : int
* 리턴형: int
* 설 명 : 1~ 해당 값까지 난수화하는 함수
*/
int rand_num(int num){
    srand(time(NULL)); //완전 랜덤화
    return (rand() % num) + 1;
}

/*
* 함수명: choose(distance[], n, found[])
* 인 자 : int 3개
* 리턴형: int
* 설 명 : 시간이 가장 작은 인덱스를 리턴하는 함수
*/
int choose(int distance[], int n, int found[]){
	int i, min, minpos;
	min = INT_MAX; // 최대로 설정
	minpos = -1;
	for (i = 0; i<n; i++)
		if (distance[i]< min && !found[i]) { //방문하지 않았고 최소보다 더 작으면
			min = distance[i]; // 거리 입력
			minpos = i; // 인덱스 입력
		}
	return minpos;
}

/*
* 함수명: shortest_path(arr, start)
* 인 자 : element**, int
* 리턴형: void
* 설 명 : 최단거리를 탐색하는 함수
*/
void shortest_path(element** arr, int start){
	int i, u, w;
    
	for (i = 0; i<R; i++){  /* 초기화 */
		distance[i] = arr[start][i].data;
		found[i] = FALSE;
		path[i] = start;
	}
	path[start] = -1;
    if (trans_done==0){// 랜덤화 종료 값이 0인경우
        for (i = 0; i<R; i++){ // 인접행렬 탐색
		    for (int j = 0; j<R; j++){
                if(arr[i][j].ic==1){// 환승역일경우
                    arr[i][j].data=rand_num(arr[i][j].datao); //환승의 가중치를 난수화
                    if(option==2) arr[i][j].data+=1000; //최소환승일 경우 가중치를 크게 증가시킴
                }
	        }
	    }
    }
    
	found[start] = TRUE;    /* 시작 정점 방문 표시 */
	distance[start] = 0;
	for (i = 0; i<R-1; i++) {
		u = choose(distance, R, found);
		found[u] = TRUE; // 해당 정점을 방문 표시
		for (w = 0; w<R; w++) 
			if (!found[w]) // 방문한적이 없다면
				if (distance[u] + arr[u][w].data < distance[w]){
					path[w]=u; //인덱스 저장
					distance[w] = distance[u] + arr[u][w].data; // 가중치 저장
				}
	}
}

/*
* 함수명: print_path(int start, int end,sublist subinfo[],element** arr)
* 인 자 : int, int, sublist, element**
* 리턴형: void
* 설 명 : path[]를 탐색하면서 정거장과 이동 시간을 출력하는 함수
*/
void print_path(int start, int end,sublist subinfo[],element** arr){
    //초기화 단계
    Sub_Time=0;
    IC_Time=0;
    Sub_Cnt=1;
	int i = end;
	int k = 0;
    int limit = 0;
    //초기화 단계

	int way[R];
    int Now;
	while (path[i] != -1){ // -1에 도달할때까지 인덱스들을 저장
		way[k++]=i; // 인덱스 저장
		way[k++]=path[i]; // i 다음으로 이어지는 인덱스 저장
		i = path[i]; // 다음
	}
    //printf("(%d 인덱스)", k-1);
    printf("\n<출발>\n");
    //printf("(%d분)", Sub_Time);
    printf("-><%s> %s\n", csvLists[subinfo[way[k-1]].num],subinfo[way[k-1]].name);
	for(int q = k-1; q > limit; q=q-2){
        if(arr[way[q]][way[q-1]].ic==0){ 
            // 환승이 아닐경우
            Now=arr[way[q]][way[q-1]].data; //시간은 그대로 저장
            Sub_Time+=Now;
            Sub_Cnt++;
            if(q<4||arr[way[q-2]][way[q-3]].ic==0){ // 다음역이 환승역이 아니면 진행, 쇼트서킷으로 잘못된 참조 방지
                //printf("(%d분)", Sub_Time);
                printf("-><%s> %s\n", csvLists[subinfo[way[q-1]].num],subinfo[way[q-1]].name);
            }
        }
        else{
            // 환승일 경우
            if(q<4||arr[way[q-2]][way[q-3]].ic==0){ // 다음역이 환승역(중복)이 아니면 진행, 쇼트서킷으로 잘못된 참조 방지
                Now=arr[way[q]][way[q-1]].data;
                if(option==2) Now=arr[way[q]][way[q-1]].data-1000; // 최소환승일경우 가중치가 더해져 있으므로 1000분 제거
                IC_Time+=Now;
                printf("-><환승 : 소요시간 %d 분> %s\n", Now,subinfo[way[q-1]].name);
            }
        }
	}
	printf("\n");
    if(option==1) printf("최단거리로 도착\n");
    if(option==2) printf("최소환승으로 도착\n");
    printf("소요시간 : %d (%d + 환승 소요시간 %d) 분\n",Sub_Time+IC_Time,Sub_Time,IC_Time);
    printf("정거장 수 : %d 개\n\n",Sub_Cnt);
}

/*
* 함수명: calc_path(int start, int end,sublist subinfo[],element** arr)
* 인 자 : int, int, sublist, element**
* 리턴형: int
* 설 명 : path[]를 탐색하면서 총 이동 시간을 리턴하는 함수
*/
int calc_path(int start, int end,sublist subinfo[],element** arr){
    //초기화 단계
    shortest_path(arr, start); //해당 인덱스의 다익스트라를 돌림
    Sub_Time=0;
    IC_Time=0;
    Sub_Cnt=1;
	int i = end;
	int k = 0;
    int limit = 0;
    //초기화 단계

	int way[R];
    int Now;
	while (path[i] != -1){ // -1에 도달할때까지 인덱스들을 저장
		way[k++]=i; // 인덱스 저장
		way[k++]=path[i]; // i 다음으로 이어지는 인덱스 저장
		i = path[i]; // 다음
	}
	for(int q = k-1; q > limit; q=q-2){
        if(arr[way[q]][way[q-1]].ic==0){ 
            // 환승이 아닐경우
            // 가중치를 더하고 정거장 카운트 올림
            Now=arr[way[q]][way[q-1]].data;
            Sub_Time+=Now;
            Sub_Cnt++;
        }
        else{
            // 환승일 경우
            // 가중치를 더하고 정거장 카운트 올림
            if(q<4||arr[way[q-2]][way[q-3]].ic==0){ // 다음역이 환승역(중복)이 아니면 진행, 쇼트서킷으로 잘못된 참조 방지
                Now=arr[way[q]][way[q-1]].data;
                if(option==2) Now=arr[way[q]][way[q-1]].data; // 최소환승일경우 가중치가 더해져 있으나 비교를 위해 1000분 제거는 안함
                IC_Time+=Now;
            }
        }
	}
    return Sub_Time+IC_Time;
}

/*
* 함수명: sub_find(element** subarray, sublist subinfo[])
* 인 자 : element**, sublist
* 리턴형: void
* 설 명 : 출발역, 도착역과 최단거리, 최소환승을 입력받고 관련 함수들을 돌리는 함수 (사실상 main)
*/
void sub_find(element** subarray,sublist subinfo[]){
    char sub1[100]; // 출발역 입력
    char sub2[100]; // 도착역 입력
    int sub1_idx; // 출발역 인덱스
    int sub2_idx; // 도착역 인덱스
    int ov_idx[R]; // 출발역 환승 중복 인덱스 저장 배열
    int ov_idx2[R]; // 도착역 환승 중복 인덱스 저장 배열
    int ov_time[R]; // 출발역 환승 중복 시간 저장 배열
    int curIdx=0; // 출발역 환승 중복 인덱스 배열 인덱스
    int min_time=9999; // 출발역이 환승일경우 환승 역중 가장 최단인 시간
    int ov_ic[R][R]; // 출발역 임시 환승시간 저장 배열

    while(1){ // 역 이름을 입력하는 부분 
        printf("출발역을 입력해주세요: ");
        fgets(sub1,sizeof(sub1),stdin); //출발역 입력
        //sub1이 비어있지 않고 0번째가 \0이 아니면서 끝부분이 \n일경우
        if(sub1 !=NULL && sub1[0]!='\0' && sub1[strlen(sub1)-1]=='\n')
            sub1[strlen(sub1)-1]='\0';

        printf("도착역을 입력해주세요: ");
        fgets(sub2,sizeof(sub2),stdin); //도착역 입력 
        //sub2가 비어있지 않고 0번째가 \0이 아니면서 끝부분이 \n일경우
        if(sub2 !=NULL && sub2[0]!='\0' && sub2[strlen(sub2)-1]=='\n') 
            sub2[strlen(sub2)-1]='\0';

        if(strcmp(sub1,sub2)==0){ // 출발역과 도착역이 동일하면 에러 1
            printf("에러1 : 출발역과 도착역이 동일합니다! \n");
            printf("다시 입력해주세요! \n");
            continue;
        }
        sub1_idx=subChk(subinfo,sub1);
        sub2_idx=subChk(subinfo,sub2);
        if (sub1_idx==-1||sub2_idx==-1){ // 인덱스가 존재하지 않으면 에러 2
            printf("에러2 : 출발역또는 도착역이 정확하지 않습니다! \n");
            printf("다시 입력해주세요! \n");
            continue;
        }
        break;
    }
    while(1){ // 우선 방식을 입력하는 부분 
        printf("방식? 1. 최단경로 2. 최소환승\n");
        printf(": ");
        char Chk_num[5]; // 우선방식 여부
        fgets(Chk_num,sizeof(Chk_num),stdin); //출발역 입력
        //sub1이 비어있지 않고 0번째가 \0이 아니면서 끝부분이 \n일경우
        if(Chk_num !=NULL && Chk_num[0]!='\0' && Chk_num[strlen(Chk_num)-1]=='\n')
            Chk_num[strlen(Chk_num)-1]='\0';

        if(atoi(Chk_num)!=1&&atoi(Chk_num)!=2){ // 값이 정해진 값이 아니면 에러 3
            printf("에러3 : 올바르지 않은 값입니다! \n");
            printf("다시 입력해주세요! \n");
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

    for(int i=0;i<R;i++){ // 출발, 도착역이 여러개인지 확인
        for(int h=0;h<R;h++){
            if(strcmp(sub1,subinfo[i].name)==0&&strcmp(sub2,subinfo[h].name)==0){ // 이름이 동일하다면
                ov_idx[curIdx]=i; // 해당 출발역 인덱스를 저장
                ov_idx2[curIdx]=h; // 해당 도착역 인덱스 저장
                ov_time[curIdx]=calc_path(i, h,subinfo,subarray); // 해당 출발역 소요시간 저장
                if (ov_time[curIdx] < min_time){ //최소보다 작을경우
                    // 최소일 경우
                    min_time = ov_time[curIdx]; // i번째 시간 저장
                    sub1_idx = ov_idx[curIdx]; // i번째 출발역 인덱스 저장
                    sub2_idx = ov_idx2[curIdx]; // i번째 도착역 인덱스 저장
                    for(int j=0;j<R;j++){
                        for(int k=0;k<R;k++){
                            ov_ic[j][k]=subarray[j][k].data; //해당 최소 가중치 갱신
                        }
                    }
                }
                curIdx++;
            }
        }
    }
    if(curIdx>1){ // 경우의 수가 여러개일경우
        for(int j=0;j<R;j++){
            for(int k=0;k<R;k++){
                subarray[j][k].data=ov_ic[j][k]; //저장했던 최소 가중치를 다시 불러와서 저장
            }
        }
    }
    trans_done=1; //랜덤화 멈춤
    shortest_path(subarray, sub1_idx); // 다익스트라
	print_path(sub1_idx, sub2_idx, subinfo, subarray); // 최종 출력 함수
}

int main(){
    element** subarray= makeArray();
    initArray(subarray);
    
    sublist subinfo[R];
    readSubInfo(subinfo);
    readSubArray(subarray,subinfo);
    
    sub_find(subarray,subinfo);

    killArray(subarray); //인접행렬 free
    printf("프로그램이 종료되었습니다 \n");

    return 0;
}