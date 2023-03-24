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

mutex_t lk = MUTEX_INIT();
cond_t cv = COND_INIT();      //condition variable

volatile int ROUND = 0, DONE_WORK = 0;
#define ROUND_FINISHED (DONE_WORK == T)
#define COND_CALCULAT(round) (round == ROUND)

void before_calculating(int round) {
  mutex_lock(&lk);
  while(!COND_CALCULAT(round)) {
    cond_wait(&cv, &lk);          //必须要等到条件满足时才会开始运算
  }
  mutex_unlock(&lk);
}

void after_calculating() {
  mutex_lock(&lk);

  DONE_WORK++;
  if(ROUND_FINISHED) {
    DONE_WORK = 0;
    ROUND++;
    cond_broadcast(&cv);
  }
  mutex_unlock(&lk);
}


int workload(int round)
{
  int min_MN = MIN(M, N), max_MN = MAX(M, N);

  //logic from the function image
  if(round < min_MN)
    return round+1;
  else if(round < max_MN)
    return min_MN;
  else  
    return M+N-1 - round;
}
//round : [0, ... , M+N-2]

int workload_thread(int round, int tid)
{
  if(tid == T)
    return workload(round)/T + workload(round)%T;
  
  return workload(round)/T;
}

int first_i(int round)
{
  if(!(round < M+N-1)) {
    printf("error round : %d \n", round);
    assert(0);
  }
  return round < N ? 0 : round - (N-1);
}

struct coordinate
{
  int i, j;
};

#define IS_VALID_IJ(i, j) ((i>=0) && (i<M) && (j>=0) && (j<N))
void first_pos(int round, int tid, struct coordinate* buff)
{
  assert(round < M+N-1);

  int workload_of_round = workload(round),
  average = workload_of_round/T;

  buff->i = first_i(round) + (tid-1) * average;
  buff->j = round - buff->i;
}

#define RENEW_POSISTION(pos) (pos.i ++, pos.j --)
void calculate(int tid, int round) {  
  struct coordinate position;
  first_pos(round, tid, &position);

  for(int t = 0; t < workload_thread(round, tid); t++, RENEW_POSISTION(position)) {
    int i = position.i;
    int j = position.j;

    assert(IS_VALID_IJ(i, j));
    int skip_a = DP(i-1, j);
    int skip_b = DP(i, j-1);
    int take_both = DP(i-1, j-1) + (A[i] == B[j]);

    dp[i][j] = MAX3(skip_a, skip_b, take_both);
  }

  //printf(" %d id finished %d round\n", tid, round);
}

#define limit_need_concurrent 200
#define CONCURRENT_CALCULATE(tid, round) before_calculating(round);\
    calculate(tid, round);\
    after_calculating(round)
    

void Tworker(int id) {
  for (int round = 0; round < M + N - 1; round++) {
    if(workload(round) >= limit_need_concurrent) {        
      CONCURRENT_CALCULATE(id, round);
    }
  }
  printf("all work of %d finished\n", id);
}

void single_worker_finish_round(int round){
  struct coordinate position;
  first_pos(round, 1, &position);

  for(int step = 0; step < workload(round); step++, RENEW_POSISTION(position)){
    int i = position.i;
    int j = position.j;

    assert(IS_VALID_IJ(i, j));
    int skip_a = DP(i-1, j);
    int skip_b = DP(i, j-1);
    int take_both = DP(i-1, j-1) + (A[i] == B[j]);

    dp[i][j] = MAX3(skip_a, skip_b, take_both);
  }

  ROUND++;
}

void Tsuper_worker() {
  for (int round = 0; round < M + N - 1; round++) {
    if(workload(round) < limit_need_concurrent){
      single_worker_finish_round(round);
      continue;
    }
    assert(round == ROUND);
    break;
  }

  printf("ready to broadcast \n");
  cond_broadcast(&cv);
  for(int round = ROUND; round < M+N-1; round++) {
    if(workload(round) < limit_need_concurrent) {
      printf("concurrent stage end, currentround&ROUND is %d&%d \n",round, ROUND);
      mutex_lock(&lk);
      while(!COND_CALCULAT(round)) {
        cond_wait(&cv, &lk);
      }
      mutex_unlock(&lk);

      printf("ready to finish left alone\n");
      break;
    }

    CONCURRENT_CALCULATE(T, round);
  }


  for(int round = ROUND; round < M+N-1; round++) {
    single_worker_finish_round(round);
  }
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
  // No need to change
  assert(scanf("%s%s", A, B) == 2);
  N = strlen(B);
  M = strlen(A);
  T = !argv[1] ? 1 : atoi(argv[1]);

  for (int i = 0; i < T-1; i++) {   //thread id: 1, 2, 3, ..., T
    create(Tworker);
  }
  create(Tsuper_worker);
  join();  // Wait for all workers
  
  result = dp[M - 1][N - 1];
  printf("%d\n", result);
  
  //display_dp_mtx();
  return 0;
}
