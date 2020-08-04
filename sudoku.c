#include <stdio.h>
#include <string.h>
char map[10][10];
int strs[10][10];
int cols[10][10];
int sqrs[10][10];
int stone[10][10];
int touched[10][10];
void printMap(){
    for(int i = 0 ; i < 9 ; i++) {
        for (int j = 0; j < 9; j++) {
            printf("%d",map[i][j]);
        }
        printf("\n");
    }
}
void put(int i, int j, int x){
    touched[i][j] = 1;
    map[i][j] = x;
    int sqr = i/3 * 3 + j/3;
    strs[i][x] = 1;
    cols[j][x] = 1;
    sqrs[sqr][x] = 1;
}
void cancel(int i, int j){
    int sqr = i/3 * 3 + j/3;
    strs[i][map[i][j]] = 0;
    cols[j][map[i][j]] = 0;
    sqrs[sqr][map[i][j]] = 0;
    map[i][j] = 0;
    touched[i][j] = 0;
}
int sol;
int possCnt(int i, int j){
    int res = 0;
    for(int k = 1 ; k <= 9 ; k++)
        if(!strs[i][k] && !cols[j][k] && !sqrs[i/3 * 3 + j/3][k])
            res++;
    return res;
}
int findNewMin(int * i, int * j){
    int min = 100;
    int hasProblem = 0;
    // maybe better search in the same str/col/sqr
    for(int ii = 0; ii < 9 ; ii++)
        for(int jj = 0 ; jj < 9 ; jj++)
            if(!touched[ii][jj]){
                int curr = possCnt(ii,jj);
                if(curr <= min) {
                    //printf("%d %d: %d\n",ii,jj,possCnt(ii, jj));
                    min = curr;
                    hasProblem = 1;
                    *i = ii;
                    *j = jj;
                }
            }
    return hasProblem;
}
int steps;
int solve(){
    steps++;
    int str,col;
    if(!findNewMin(&str,&col)) {
        sol = 1;
        return 1;
    }
    if(stone[str][col])
        return solve();
    //printf("in: %d %d\n",str,col);
    touched[str][col] = 1;
    //printf("come %d %d: %d posses: \n",str,col,possCnt(str,col));
    if(possCnt(str,col) == 0){
        touched[str][col] = 0;
        return 0;
    }
    int possExist = 0;
    for(int k = 1 ; k <= 9 ; k++){
        if(!strs[str][k] && !cols[col][k] && !sqrs[str/3 * 3 + col/3][k]){
            // printf("put %d in (%d,%d)\n",k,str,col);
            put(str,col,k);
            possExist = 1;
            if(!solve()) {
                cancel(str, col);
                //printf("cancel %d in (%d,%d)\n",k,str,col);
            }
            else
                break;
        }
    }
    if(!possExist || !sol)
        return 0;
    return 1;
}
int main() {
 
    freopen("input.txt","r",stdin);
    int N;
    scanf("%d\n",&N);
    for(int n = 0; n < N ; n++) {
        memset(map,0, sizeof(char)*100);
        memset(strs,0, sizeof(int)*100);
        memset(cols,0, sizeof(int)*100);
        memset(sqrs,0, sizeof(int)*100);
        memset(stone,0, sizeof(int)*100);
        memset(touched,0,sizeof(int)*100);
        sol = 0;
        for (int i = 0; i < 9; i++) {
            for (int j = 0; j < 9; j++) {
                char c;
                scanf("%c", &c);
                if (c != '.') {
                    map[i][j] = c - '0';
                    put(i, j, map[i][j]);
                    stone[i][j] = 1;
                    touched[i][j] = 1;
                }
            }
            scanf("\n");
        }
        solve();
        printMap();
        printf("\n");
    }
    //fprintf(stderr,"%d",steps);
    return 0;
}