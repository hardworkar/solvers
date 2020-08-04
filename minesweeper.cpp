#include <iostream>
#include <cstdio>
#include <windows.h>
#define undefined 0
#define safe 1
#define mine 2
enum Color{basic, edge, set};
#define MINES 28
class Map{
public:
    int ns[9][9];    // 'digits' in cells: 0 - empty cell, x (-1) - undefined value
    int cells[9][9]; // 0 - undefined, 1 - safe, 2 - mine
    int ms[9][9];    // number of 'true-mine' around each cell
    int xs[9][9];    // number of 'undefined' around each cell
    Color colors[9][9];// blue - basic, green - edge, purple - set
    Map();
    void getStartNumsFromText(FILE * f);
    void printCells();
    void printCells(int i, int j, int color);
    bool step();
    void update();
    int simplify();
    bool isSolution();
    bool checkInvariant();
    bool findCertainInSet(int &i, int &j, int &what);
    bool finishSet();

    void discolor();
    void colorEdge();
    void colorSet();
};
bool checkBounds(int i, int j){
    return !(i < 0 || j < 0 || i >= 9 || j >= 9);
}
void Map::discolor() {
    for(int i = 0 ; i < 9 ; i++){
        for(int j = 0 ; j < 9 ; j++)
            colors[i][j] = basic;
    }
}

void Map::colorEdge() {     // edge - множество клеток с определенной цифрой + неудовлетворенным xs
    update();
    discolor();
    for(int i = 0 ; i < 9 ; i++){
        for(int j = 0 ; j < 9 ; j++){
            if(ns[i][j] != -1 && xs[i][j] != 0){
                colors[i][j] = edge;
            }
        }
    }
}

void Map::colorSet() {      // set - множество undefined клеток, соседствующих с edge
    colorEdge();
    for(int i = 0 ; i < 9 ; i++){
        for(int j = 0 ; j < 9 ; j++){
            if(cells[i][j] != undefined)
                continue;
            int dx[] = { 1, 1, 1, 0, 0,-1,-1,-1};
            int dy[] = { 1, 0,-1, 1,-1, 1, 0,-1};
            for(int cell = 0 ; cell < 8 ; cell++)
                if(checkBounds(i+dy[cell],j+dx[cell]) && colors[i+dy[cell]][j+dx[cell]] == edge)
                    colors[i][j] = set;
        }
    }
}
bool Map::finishSet() {     // false if we can't build the whole set
    printf("finishing set, that's start point:\n");
    printCells();
    simplify();
    printf("after simplify:\n");
    printCells();
    bool setHasUndefinedCells = false;
    for(int i = 0 ; i < 9 ; i++){
        for(int j = 0 ; j < 9 ; j++){
            if(colors[i][j] == set && cells[i][j] == undefined && ns[i][j] == -1){
                Map tmp = *this;
                printf("we put mine in (%d,%d)\n",i,j);
                cells[i][j] = mine;
                printCells(i,j, FOREGROUND_RED);
                bool mineResult = false;
                if(checkInvariant())
                    mineResult = finishSet();
                *this = tmp;
                if(!mineResult){
                    printf("putting mine in (%d,%d) didn't lead to solution\n",i,j);
                    cells[i][j] = safe;

                    printCells(i,j,FOREGROUND_RED);
                    bool res = finishSet();
                    if(res){
                        printf("looks like that's completed map\n");
                    }
                    *this = tmp;
                    return res;
                } else {
                    printf("looks like that's completed map\n");
                    *this = tmp;
                    return true;
                }
            }
        }
    }
    return checkInvariant();
}
bool Map::findCertainInSet(int &oi, int &oj, int &what) {   // как доказательство от противного:
    for(int i = 0 ; i < 9 ; i++){                           // пробуем поставить мину и достроить остаток сета -
        for(int j = 0 ; j < 9 ; j++){                       // если не получается достроить без нарушений инварианта -
            if(colors[i][j] == set){                        // в этой клетке не может стоять мина => она certain-safe
                printf("> now we hope (%d,%d) to be certain\n",i,j);
                Map tmp = *this;        // memorize map
                printf("try put Mine:\n");
                cells[i][j] = mine;     // trying break everything with putting mine
                if(!finishSet()){
                    oi = i, oj = j;
                    what = safe;
                    *this = tmp;
                    return true;
                }
                *this = tmp;
                printf("try put Safe:\n");
                cells[i][j] = safe;     // trying break everything with putting safe
                if(!finishSet()){
                    oi = i, oj = j;
                    what = mine;
                    *this = tmp;
                    return true;
                }
                *this = tmp;                                          // если для случаев мины/safe сеты достраивались =>
                printf("looks like (%d,%d) is not certain...\n",i,j); // ничего определенного про клетку сказать нельзя
            }
        }
    }
    return false;
}


Map::Map(){
    for(int i = 0 ; i < 9 ; i++){
        for(int j = 0 ; j < 9 ; j++){
            cells[i][j] = undefined;
            ms[i][j] = 0;
            xs[i][j] = 0;
            colors[i][j] = basic;
            int dx[] = { 1, 1, 1, 0, 0,-1,-1,-1};   // (-1, 1) ( 0, 1) ( 1, 1)
            int dy[] = { 1, 0,-1, 1,-1, 1, 0,-1};   // (-1, 0) ( 0, 0) ( 1, 0)
                                                    // (-1,-1) ( 0,-1) ( 1,-1)
            for(int cell = 0 ; cell < 8 ; cell++)
                if(checkBounds(i+dy[cell],j+dx[cell]))
                    xs[i][j]++;
        }
    }
}
void Map::getStartNumsFromText(FILE * f) {  // получение стартовой расстановки из файла
    for(int i = 0 ; i < 9 ; i ++){
        for(int j = 0 ; j < 10 ; j++){
            char c;
            fscanf(f,"%c",&c);
            if(j < 9) {
                if(c != 'x') {
                    cells[i][j] = safe;
                    ns[i][j] = c - '0';
                } else ns[i][j] = -1;
            }
        }
    }
}
// вывод поля с выделением одной клетки в color
void Map::printCells(int I, int J, int color) {
    HANDLE handle = GetStdHandle(STD_OUTPUT_HANDLE);
    SetConsoleTextAttribute(handle, 2);
    for(int i = 0; i < 9 ; i++) {
        for (int j = 0; j < 9; j++) {
            if(i == I && j == J)
                SetConsoleTextAttribute(handle, color);
            printf("%d ", cells[i][j]);
            if(i == I && j == J)
                SetConsoleTextAttribute(handle, 2);
        }
        printf("\n");
    }
    printf("\n");
}
void Map::printCells() {
    HANDLE handle = GetStdHandle(STD_OUTPUT_HANDLE);
    SetConsoleTextAttribute(handle, 1);
    for(int i = 0; i < 9 ; i++){
        for(int j = 0 ; j < 9 ; j++) {
            switch (colors[i][j]){
                case basic:
                    break;
                case edge:
                    SetConsoleTextAttribute(handle, 2);
                    break;
                case set:
                    SetConsoleTextAttribute(handle, 3);
                    break;
            }
            printf("%d ", cells[i][j]);
            SetConsoleTextAttribute(handle, 1);
        }
        printf("\n");
    }
}
void Map::update(){
    for(int i = 0 ; i < 9 ; i++){
        for(int j = 0 ; j < 9 ; j++){
            int dx[] = { 1, 1, 1, 0, 0,-1,-1,-1};
            int dy[] = { 1, 0,-1, 1,-1, 1, 0,-1};
            xs[i][j] = ms[i][j] = 0;
            for(int cell = 0 ; cell < 8 ; cell++)
                if(checkBounds(i+dy[cell],j+dx[cell])){
                    if(cells[i+dy[cell]][j+dx[cell]] == mine)
                        ms[i][j]++;
                    else if(cells[i+dy[cell]][j+dx[cell]] == undefined)
                        xs[i][j]++;
                }
        }
    }
}
// шаг алгоритма, когда есть 100% ход
// return value shows either there were changes or not
bool Map::step() {
    bool wasChange = false;
    for(int i = 0 ; i < 9 ; i++){
        for(int j = 0 ; j < 9 ; j++){
            if(ns[i][j] == -1)
                continue;
            int dx[] = { 1, 1, 1, 0, 0,-1,-1,-1};
            int dy[] = { 1, 0,-1, 1,-1, 1, 0,-1};
            if(ns[i][j] - ms[i][j] == 0){
                for(int cell = 0 ; cell < 8 ; cell++)
                    if(checkBounds(i+dy[cell], j+dx[cell]) && cells[i + dy[cell]][j + dx[cell]] == undefined) {
                        cells[i + dy[cell]][j + dx[cell]] = safe;
                        wasChange = true;
                    }
                xs[i][j] = 0;
            }
            if(ns[i][j] - ms[i][j] == xs[i][j]){
                for(int cell = 0 ; cell < 8 ; cell++)
                    if(checkBounds(i+dy[cell], j+dx[cell]) && cells[i + dy[cell]][j + dx[cell]] == undefined) {
                        cells[i + dy[cell]][j + dx[cell]] = mine;
                        wasChange = true;
                    }
                xs[i][j] = 0;
            }
        }
    }
    return wasChange;
}
int Map::simplify(){    // do steps while there are any changes
    int i = 0;
    do {
        update();
        i++;
    }while(step());
    return i;
}
bool Map::isSolution(){
    int mines = 0;
    update();
    for(int i = 0 ; i < 9 ; i++){
        for(int j = 0 ; j < 9 ; j++){
            if(cells[i][j] == undefined)
                return false;
            else if(cells[i][j] == mine)
                mines++;
            if(ns[i][j] != -1){
                if(ns[i][j] != ms[i][j])
                    return false;
            }
        }
    }
    return mines == MINES;
}
bool Map::checkInvariant() {    // true - everything is OK
    update();
    for(int i = 0 ; i < 9 ; i++){
        for(int j = 0 ; j < 9 ; j++){
            if(ns[i][j] == -1)
                continue;
            if(ms[i][j] > ns[i][j]) {
                printf("inv broke at (%d,%d) -> %d > %d\n",i,j,ms[i][j],ns[i][j]);
                return false;
            }
            if(ms[i][j] + xs[i][j] < ns[i][j]){
                printf("inv broke at (%d,%d) -> %d + %d < %d\n",i,j,ms[i][j],xs[i][j],ns[i][j]);
                return false;
            }
        }
    }
    return true;
}

int main(){
    FILE * f = fopen("./startMaps/map01.txt","r");
    Map map;
    map.getStartNumsFromText(f);
    fclose(f);
    while(!map.isSolution()) {
        map.simplify();
        map.discolor();
        map.printCells();
        printf("help me...\n");
        char command[200];
        std::cin.getline(command,200);
        if(!strcmp(command,"try set")){
            map.colorSet();
            printf("we colored map:\n");
            map.printCells();
            int oi,oj,what;
            Map tmp = map;
            if(map.findCertainInSet(oi,oj,what)){
                printf("we find %s cell: (%d,%d)\n", (what == mine ? "mine" : "safe"), oi, oj);
                map = tmp;
                map.cells[oi][oj] = what;
                map.printCells();
            } else printf("sorry, bro...\n");
        }
        if(!strcmp(command,"ok")){
            f = fopen("./startMaps/map01.txt","r");
            map.getStartNumsFromText(f);
            fclose(f);
        }
        if(strcmp(command,"exit") == 0){
            exit(0);
        }
    }
    return 0;
}