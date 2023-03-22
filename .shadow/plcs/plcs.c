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

//需要一个全局注册表
int *MAP;


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
  //for (int round = 0; round < 2 * N - 1; round++) {
  // 1. 计算出本轮能够计算的单元格
    //[0, round], [1, round-1], ... [round, 0]
    //一共round+1
    //每个线程可以完成 (round+1)/T个单元格
    
  // 2. 将任务分配给线程执行

  // 3. 等待线程执行完毕
    //线程执行完毕之后，能进入下一个周期，只要前面的都已经满足了即可
    //os会自动切换进程
  //}


  //result = dp[N - 1][M - 1];

  printf("%d\n", id);
}

void make_same_length()
{
  int A_len = strlen(A), B_len = strlen(B);

  if(A_len == B_len)
    return;

  int offset;
  char* small, *large;
  int small_len, large_len;

  if(A_len > B_len)
  {
    small = B, large = A;
    small_len = B_len, large_len = A_len;
  }
  else
  {
    small = A, large = B;
    small_len = A_len, large_len = B_len;
  }
  offset = large_len - small_len;

  for(int i = large_len; i >= offset; i--)
    small[i] = small[i-offset];
    
  for(int i = 0; i < offset; i++)
    small[i] = '$';//one letter that wont match large
  
  M = N = large_len;
  assert(strlen(small+offset) == small_len);
}



void show_thread_id(int id)
{

}
int main(int argc, char *argv[]) {
  // No need to change
  assert(scanf("%s%s", A, B) == 2);
  N = strlen(A);
  M = strlen(B);
  T = !argv[1] ? 1 : atoi(argv[1]);

  // Add preprocessing code here
  // make a and b of same length
  make_same_length();
  
  for (int i = 0; i < T; i++) {
    create(Tworker);
  }
  join();  // Wait for all workers

  printf("%d\n", result);
}
