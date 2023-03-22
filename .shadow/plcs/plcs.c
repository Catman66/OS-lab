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
//需要一个全局注册表
int *progresses;
#define COND_CALCULAT(tid) (progresses[tid] <= progresses[tid+1] && progresses[tid] <= progresses[tid-1]) 
#define FINISH_ROUND(tid) (progresses[tid]++)
mutex_t lk = MUTEX_INIT();    //mutual exclusive lock
cond_t cv = COND_INIT();      //condition variable

void before_calculating(int tid)
{
  mutex_lock(&lk);

  while(!COND_CALCULAT(tid))
  {
    cond_wait(&cv, &lk);          //必须要等到条件满足时才会开始运算
  }
  //划分本次的顺序
  
  mutex_unlock(&lk);
}


void after_calculating(int tid)
{
  mutex_lock(&lk);
  
  FINISH_ROUND(tid);

  cond_broadcast(&cv);

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
  return round < N ? 0 : round - (N-1);
}

struct coordinate
{
  int i, j;
};

#define IS_VALID_IJ(i, j) ((i>=0) && (i<M) && (j>=0) && (j<N))
void first_ij(int round, int tid, struct coordinate* buff)
{
  int workload_of_round = workload(round),
  average = workload_of_round/T;

  buff->i = first_i(round) + (tid-1) * average;
  buff->j = round - buff->i;
  if(!IS_VALID_IJ(buff->i, buff->j))
    fprintf(stderr, "i = %d, j = %d\n", buff->i, buff->j);
  assert(IS_VALID_IJ(buff->i, buff->j));
}

#define RENEW_POSISTION(pos) (pos.i ++, pos.j --)
void calculate(int tid)
{
  int round = progresses[tid]+1;

  struct coordinate position;
  first_ij(round, tid, &position);

  for(int t = 0; t < workload_thread(tid, round); t++, RENEW_POSISTION(position))
  {
    int i = position.i;
    int j = position.j;

    int skip_a = DP(i-1, j);
    int skip_b = DP(i, j-1);
    int take_both = DP(i-1, j-1) + (A[i] == B[j]);

    dp[i][j] = MAX3(skip_a, skip_b, take_both);
  }
}



void Tworker(int id) {
  // if (id != 1) {
  //   // This is a serial implementation
  //   // Only one worker needs to be activated
  //   return;
  // }

  //问题是如何实现，工作量的分配
  // for (int i = 0; i < N; i++) {
  //   for (int j = 0; j < M; j++) {
  //     // Always try to make DP code more readable
  //     int skip_a = DP(i - 1, j);
  //     int skip_b = DP(i, j - 1);
  //     int take_both = DP(i - 1, j - 1) + (A[i] == B[j]);
  //     dp[i][j] = MAX3(skip_a, skip_b, take_both);
  //   }
  // }
  for (int round = 0; round < M + N - 1; round++) {
    before_calculating(id);

    calculate(id);

    after_calculating(id);
  }


  result = dp[N - 1][M - 1];
}


#define INIT_PROGRESSES() (progresses = malloc((T+2)*sizeof(int)), memset(progresses, 0xff, (T+2) * sizeof(int)), progresses[0] = progresses[T+1] = M+N)
#define FREE_PROGRESSES(vars) (free(vars))



int main(int argc, char *argv[]) {
  // No need to change
  assert(scanf("%s%s", A, B) == 2);
  N = strlen(B);
  M = strlen(A);
  T = !argv[1] ? 1 : atoi(argv[1]);

  INIT_PROGRESSES();
  assert(progresses != NULL);
  for(int i = 0; i < T+2; i++)
    assert(progresses[i] == -1);
  //thread id: 1, 2, 3, ..., T
  for (int i = 0; i < T; i++) {
    create(Tworker);
  }
  join();  // Wait for all workers

  printf("%d\n", result);

  FREE_PROGRESSES(progresses);

  return 0;
}
