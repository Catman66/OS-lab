#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "thread.h"
#include "thread-sync.h"

#define MAXN 10000
//ensured that the strlen of the given string wont exceed MAXN

int T, N, M;
char A[MAXN + 1], B[MAXN + 1];
int dp[MAXN][MAXN];
int result;

#define DP(x, y) (((x) >= 0 && (y) >= 0) ? dp[x][y] : 0)
#define MAX(x, y) (((x) > (y)) ? (x) : (y))
#define MAX3(x, y, z) MAX(MAX(x, y), z)
#define MIN(x, y) (((x) < (y)) ? (x) : (y))

mutex_t lk = MUTEX_INIT();//, lk_main = MUTEX_INIT();
cond_t cv = COND_INIT();//, cv_main = COND_INIT();      //condition variable

int* PROGRESSES;
#define INIT_PROGRESSES() PROGRESSES = malloc((T+1)* sizeof(int));\
                        memset(PROGRESSES, 0xff, (T+1)* sizeof(int)); \
                        PROGRESSES[0] = NUM_ROUNDS;
#define FREE_PROGRESSES() free(PROGRESSES)

int BLCOK_WIDTH = 100;
int BLOCK_HEIGHT, LAST_HEIGHT;
int NUM_ROUNDS;
#define INIT_BLOCK_INFO() \
BLOCK_HEIGHT = M / T, \
LAST_HEIGHT = M - (T-1) * BLOCK_HEIGHT,\
NUM_ROUNDS = (N + BLCOK_WIDTH - 1) / BLCOK_WIDTH

#define COND_CALCULAT(id, round) ((round) < PROGRESSES[(id)-1] )

void wait_for_other_thread(int tid, int round) {
  mutex_lock(&lk);
  while(!COND_CALCULAT(tid, round)) {
    printf("%d waiting", tid);
    cond_wait(&cv, &lk);          //必须要等到条件满足时才会开始运算
  }
  mutex_unlock(&lk);
}

void after_calculating(int tid) {
  mutex_lock(&lk);

  PROGRESSES[tid]++;
  cond_broadcast(&cv);

  mutex_unlock(&lk);
}

#define IS_VALID_IJ(i, j) ((i>=0) && (i<M) && (j>=0) && (j<N))

#define CALCULATE_GRID(i, j) int skip_a = DP(i-1, j);\
    int skip_b = DP(i, j-1);\
    int take_both = DP(i-1, j-1) + (A[i] == B[j]);\
    dp[i][j] = MAX3(skip_a, skip_b, take_both)


void finish_block(int m, int n, int height, int width){
  printf("finish %d %d %d %d \n", m ,n, height, width);
  for(int i = m; i < m + height; i++){
    for(int j = n; j < n + width; j++){
      assert(IS_VALID_IJ(i, j));
      CALCULATE_GRID(i, j);
    }
  }
}

void calculate(int tid, int round) {  
  
  int i = (tid-1) * BLOCK_HEIGHT;
  int j = round * BLCOK_WIDTH;
  int w = BLCOK_WIDTH;
  int h = BLOCK_HEIGHT;

  if(round == NUM_ROUNDS-1){
    w = N - round * BLCOK_WIDTH;
  }
  if(tid == T){
    h = LAST_HEIGHT;
  }
  finish_block(i, j, h, w);
}

#define limit_need_concurrent 500
#define CONCURRENT_CALCULATE(tid, round) wait_for_other_thread(tid, round);\
    calculate(tid, round);\
    after_calculating(round)
    

void Tworker(int id) {
  for (int round = 0; round < NUM_ROUNDS; round++) {  
      CONCURRENT_CALCULATE(id, round);
  }
  printf("all work of %d finished\n", id);
}

void single_worker_finish_all(){
  for (int i = 0; i < N; i++) {
    for (int j = 0; j < M; j++) {
      // Always try to make DP code more readable
      int skip_a = DP(i - 1, j);
      int skip_b = DP(i, j - 1);
      int take_both = DP(i - 1, j - 1) + (A[i] == B[j]);
      dp[i][j] = MAX3(skip_a, skip_b, take_both);
    }
  }
}

void display_dp_mtx(){
  for(int i = 0 ; i < M; i++){
    for(int j = 0; j < N; j++)
      printf("%d ", dp[i][j]);
    printf("\n");
  }
}

int main(int argc, char *argv[]) {
  assert(scanf("%s%s", A, B) == 2);
  N = strlen(B);
  M = strlen(A);
  T = !argv[1] ? 1 : atoi(argv[1]);


  INIT_BLOCK_INFO();
  INIT_PROGRESSES();

  for(int i = 1; i <= T; i ++){
    assert(PROGRESSES[i] == -1);
  }
  sleep(1000);
  for (int i = 0; i < T; i++) {   //thread id: 1, 2, 3, ..., T
    create(Tworker);
  }

  //no need to switch to main thread
  // mutex_lock(&lk_main);
  // cond_wait(&cv_main, &lk_main);
  // mutex_unlock(&lk_main);

  join();  // Wait for all workers
  
  result = dp[M - 1][N - 1];
  printf("%d\n", result);
  
  //display_dp_mtx();
  FREE_PROGRESSES();
  return 0;
}
