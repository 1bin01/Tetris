#include "tetris.h"

static struct sigaction act, oact;
int B, count;

int main() {
    int exit = 0;

    // Create Linked List For Ranking System
    createRankList();

    initscr();
    noecho();
    keypad(stdscr, TRUE);

    srand((unsigned int)time(NULL));

    while (!exit) {
        clear();
        switch (menu()) {
            case MENU_PLAY:
                play();
                break;
            case MENU_RANK:
                rank();
                break;
            case MENU_REC_PLAY:
                recommendedPlay();
                break;
            case MENU_EXIT:
                exit = 1;
                break;
            default:
                break;
        }
    }

    endwin();
    system("clear");
    return 0;
}

void InitTetris() {
    int i, j;

    for (j = 0; j < HEIGHT; j++)
        for (i = 0; i < WIDTH; i++)
            field[j][i] = 0;

    nextBlock[0] = rand() % 7;
    nextBlock[1] = rand() % 7;
    nextBlock[2] = rand() % 7;
    blockRotate = 0;
    blockY = -1;
    blockX = WIDTH / 2 - 2;
    score = 0;
    gameOver = 0;
    timed_out = 0;

    DrawOutline();
    DrawField();
    DrawBlock(blockY, blockX, nextBlock[0], blockRotate, ' ');
    DrawNextBlock(nextBlock);
    PrintScore(score);
}

void DrawOutline() {
    int i, j;
    /* 블럭이 떨어지는 공간의 태두리를 그린다.*/
    DrawBox(0, 0, HEIGHT, WIDTH);

    /* next block을 보여주는 공간의 태두리를 그린다.*/
    move(2, WIDTH + 10);
    printw("NEXT BLOCK");
    DrawBox(3, WIDTH + 10, 4, 8);

    /* next next block을 보여주는 공간의 태두리를 그린다.*/
    move(8, WIDTH + 10);
    DrawBox(9, WIDTH + 10, 4, 8);

    /* score를 보여주는 공간의 태두리를 그린다.*/
    move(15, WIDTH + 10);
    printw("SCORE");
    DrawBox(16, WIDTH + 10, 1, 8);
}

int GetCommand() {
    int command;
    command = wgetch(stdscr);
    switch (command) {
        case KEY_UP:
            break;
        case KEY_DOWN:
            break;
        case KEY_LEFT:
            break;
        case KEY_RIGHT:
            break;
        case ' ': /* space key*/
            /*fall block*/
            break;
        case 'q':
        case 'Q':
            command = QUIT;
            break;
        default:
            command = NOTHING;
            break;
    }
    return command;
}

int ProcessCommand(int command) {
    int ret = 1;
    int drawFlag = 0;
    switch (command) {
        case QUIT:
            ret = QUIT;
            break;
        case KEY_UP:
            if ((drawFlag = CheckToMove(field, nextBlock[0], (blockRotate + 1) % 4, blockY, blockX)))
                blockRotate = (blockRotate + 1) % 4;
            break;
        case KEY_DOWN:
            if ((drawFlag = CheckToMove(field, nextBlock[0], blockRotate, blockY + 1, blockX)))
                blockY++;
            break;
        case KEY_RIGHT:
            if ((drawFlag = CheckToMove(field, nextBlock[0], blockRotate, blockY, blockX + 1)))
                blockX++;
            break;
        case KEY_LEFT:
            if ((drawFlag = CheckToMove(field, nextBlock[0], blockRotate, blockY, blockX - 1)))
                blockX--;
            break;
        default:
            break;
    }
    if (drawFlag)
        DrawChange(field, command, nextBlock[0], blockRotate, blockY, blockX);
    return ret;
}

void DrawField() {
    int i, j;
    for (j = 0; j < HEIGHT; j++) {
        move(j + 1, 1);
        for (i = 0; i < WIDTH; i++) {
            if (field[j][i] == 1) {
                attron(A_REVERSE);
                printw(" ");
                attroff(A_REVERSE);
            } else
                printw(".");
        }
    }
}

void PrintScore(int score) {
    move(17, WIDTH + 11);
    printw("%8d", score);
}

void DrawNextBlock(int *nextBlock) {
    int i, j;
    for (i = 0; i < 4; i++) {
        move(4 + i, WIDTH + 13);
        for (j = 0; j < 4; j++) {
            if (block[nextBlock[1]][0][i][j] == 1) {
                attron(A_REVERSE);
                printw(" ");
                attroff(A_REVERSE);
            } else
                printw(" ");
        }
    }

    for (int i = 0; i < 4; i++) {
        move(10 + i, WIDTH + 13);
        for (j = 0; j < 4; j++) {
            if (block[nextBlock[2]][0][i][j] == 1) {
                attron(A_REVERSE);
                printw(" ");
                attroff(A_REVERSE);
            } else
                printw(" ");
        }
    }
}

void DrawBlock(int y, int x, int blockID, int blockRotate, char tile) {
    int i, j;
    for (i = 0; i < 4; i++)
        for (j = 0; j < 4; j++) {
            if (block[blockID][blockRotate][i][j] == 1 && i + y >= 0) {
                move(i + y + 1, j + x + 1);
                attron(A_REVERSE);
                printw("%c", tile);
                attroff(A_REVERSE);
            }
        }

    move(HEIGHT, WIDTH + 10);
}

void DrawBox(int y, int x, int height, int width) {
    int i, j;
    move(y, x);
    addch(ACS_ULCORNER);
    for (i = 0; i < width; i++)
        addch(ACS_HLINE);
    addch(ACS_URCORNER);
    for (j = 0; j < height; j++) {
        move(y + j + 1, x);
        addch(ACS_VLINE);
        move(y + j + 1, x + width + 1);
        addch(ACS_VLINE);
    }
    move(y + j + 1, x);
    addch(ACS_LLCORNER);
    for (i = 0; i < width; i++)
        addch(ACS_HLINE);
    addch(ACS_LRCORNER);
}

void play() {
    int command;
    clear();
    act.sa_handler = BlockDown;
    sigaction(SIGALRM, &act, &oact);
    InitTetris();
    do {
        if (timed_out == 0) {
            alarm(1);
            timed_out = 1;
        }

        command = GetCommand();
        if (ProcessCommand(command) == QUIT) {
            alarm(0);
            DrawBox(HEIGHT / 2 - 1, WIDTH / 2 - 5, 1, 10);
            move(HEIGHT / 2, WIDTH / 2 - 4);
            printw("Good-bye!!");
            refresh();
            getch();

            return;
        }
    } while (!gameOver);

    alarm(0);
    getch();
    DrawBox(HEIGHT / 2 - 1, WIDTH / 2 - 5, 1, 10);
    move(HEIGHT / 2, WIDTH / 2 - 4);
    printw("GameOver!!");
    refresh();
    getch();
    newRank(score);
}

char menu() {
    printw("1. play\n");
    printw("2. rank\n");
    printw("3. recommended play\n");
    printw("4. exit\n");
    return wgetch(stdscr);
}

/////////////////////////첫주차 실습에서 구현해야 할 함수/////////////////////////
int CheckToMove(char f[HEIGHT][WIDTH], int currentBlock, int blockRotate, int blockY, int blockX) {
    for (int i = 0; i < 4; i++) {
        int y = blockY + i;
        for (int j = 0; j < 4; j++) {
            int x = blockX + j;
            if (block[currentBlock][blockRotate][i][j] == 1) {
                if (y >= HEIGHT)
                    return 0;
                else if (y < 0)
                    return 0;
                else if (x < 0)
                    return 0;
                else if (x >= WIDTH)
                    return 0;
                else if (f[y][x] == 1)
                    return 0;
            }
        }
    }
    return 1;
}

void DrawChange(char f[HEIGHT][WIDTH], int command, int currentBlock, int blockRotate, int blockY, int blockX) {
    // user code
    //1. 이전 블록 정보를 찾는다. ProcessCommand의 switch문을 참조할 것
    int prevBlockRotate = blockRotate;
    int prevBlockY = blockY;
    int prevBlockX = blockX;
    switch (command) {
        case KEY_UP:
            prevBlockRotate = (4 + (prevBlockRotate - 1)) % 4;
            break;
        case KEY_DOWN:
            prevBlockY -= 1;
            break;
        case KEY_RIGHT:
            prevBlockX -= 1;
            break;
        case KEY_LEFT:
            prevBlockX += 1;
            break;
        default:
            break;
    }

    //2. 이전 블록 정보를 지운다. DrawBlock함수 참조할 것.
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            if (block[currentBlock][prevBlockRotate][i][j] == 1 && i + prevBlockY >= 0) {
                move(i + prevBlockY + 1, j + prevBlockX + 1);
                printw("%c", '.');
            }
        }
    }

    //2-1. 이전 그림자 정보를 지운다.
    int can_Y = -1;
    for (int h = blockY; h < HEIGHT; h++) {
        if (!CheckToMove(field, currentBlock, prevBlockRotate, h, prevBlockX)) {
            can_Y = h - 1;
            break;
        }
    }

    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            if (block[currentBlock][prevBlockRotate][i][j] == 1 && i + can_Y >= 0) {
                move(i + can_Y + 1, j + prevBlockX + 1);
                printw("%c", '.');
            }
        }
    }

    //3. 새로운 블록 정보를 그린다.
    DrawBlockWithFeatures(blockY, blockX, currentBlock, blockRotate);
}

void BlockDown(int sig) {
    // user code
    if (CheckToMove(field, nextBlock[0], blockRotate, blockY + 1, blockX)) {
        blockY++;
        DrawChange(field, KEY_DOWN, nextBlock[0], blockRotate, blockY, blockX);
    } else {
        if (blockY == -1) {
            gameOver = TRUE;
        } else {
            int touched = AddBlockToField(field, nextBlock[0], blockRotate, blockY, blockX);

            score += DeleteLine(field);
            score += touched;
            PrintScore(score);

            nextBlock[0] = nextBlock[1];
            nextBlock[1] = nextBlock[2];
            nextBlock[2] = rand() % 7;
            DrawNextBlock(nextBlock);

            blockRotate = 0;
            blockY = -1, blockX = WIDTH / 2 - 1;
        }
        DrawField();
    }

    // 타이머 시작
    timed_out = 0;

    //강의자료 p26-27의 플로우차트를 참고한다.
}

int AddBlockToField(char f[HEIGHT][WIDTH], int currentBlock, int blockRotate, int blockY, int blockX) {
    // 바닥에 닿은 면적 계산용
    int cnt = 0;
    //Block이 추가된 영역의 필드값을 바꾼다.
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            if (block[currentBlock][blockRotate][i][j] == 1) {
                if (0 <= blockY + i && blockY + i < HEIGHT && 0 <= blockX + j && blockX + j < WIDTH)
                    f[blockY + i][blockX + j] = 1;

                if (blockY + i == HEIGHT - 1)
                    cnt++;
                else if (blockY + i + 1 < HEIGHT && block[currentBlock][blockRotate][i + 1][j] == 0 && f[blockY + i + 1][blockX + j] == 1)
                    cnt++;
            }
        }
    }

    return cnt * 10;
}

int DeleteLine(char f[HEIGHT][WIDTH]) {
    // user code

    //1. 필드를 탐색하여, 꽉 찬 구간이 있는지 탐색한다.
    int *fullLines = (int *)malloc(sizeof(int) * HEIGHT);
    int cnt = 0;
    for (int i = 0; i < HEIGHT; i++) {
        int chk = 0;
        for (int j = 0; j < WIDTH; j++) {
            if (f[i][j] == 0) {
                chk = 1;
                break;
            }
        }
        if (!chk) {
            fullLines[cnt++] = i;
        }
    }

    //2. 꽉 찬 구간이 있으면 해당 구간을 지운다. 즉, 해당 구간으로 필드값을 한칸씩 내린다.
    for (int i = 0; i < cnt; i++) {
        int line = fullLines[i];
        for (int h = line - 1; h >= 0; h--) {
            if (h != 0)
                for (int j = 0; j < WIDTH; j++) {
                    f[h + 1][j] = f[h][j];
                }
            else
                for (int j = 0; j < WIDTH; j++) {
                    f[h][j] = 0;
                }
        }
    }
    free(fullLines);

    return cnt * cnt * 100;
}

void DrawBlockWithFeatures(int y, int x, int blockID, int blockRotate) {
    parent = malloc(sizeof(RecNode));
    memcpy(parent->recField, field, sizeof(char) * HEIGHT * WIDTH);
    parent->accumulatedScore = 0;
    parent->level = 0;

    max_ = -1;
    Max_ptr = NULL;
    recommend(parent);

    while (Max_ptr && Max_ptr->level != 1) {
        Max_ptr = Max_ptr->parent;
    }

    if (Prev_Rec)
        DeleteRecommend(Prev_Rec->recBlockY, Prev_Rec->recBlockX, Prev_Rec->curBlockID, Prev_Rec->recBlockRotate);
    // Prev_Rec = Max_ptr;
    Prev_Rec = parent;

    DrawRecommend(parent->recBlockY, parent->recBlockX, parent->curBlockID, parent->recBlockRotate);
    // DrawRecommend(Max_ptr->recBlockY, Max_ptr->recBlockX, Max_ptr->curBlockID, Max_ptr->recBlockRotate);

    DrawShadow(y, x, blockID, blockRotate);
    DrawBlock(blockY, blockX, blockID, blockRotate, ' ');

    // Free Used Rec-Tree
    // int dep1_cnt = parent->child_cnt;
    // while (dep1_cnt--) {
    //     RecNode *depth1 = parent->child[dep1_cnt];
    //     while (depth1) {
    //         int dep2_cnt = depth1->child_cnt;
    //         while (dep2_cnt--) {
    //             free(depth1->child[dep2_cnt]);
    //         }
    //     }
    //     free(depth1);
    // }

    // free(parent);
}

void DrawShadow(int y, int x, int blockID, int blockRotate) {
    int can_Y = -1;

    for (int h = blockY; h < HEIGHT; h++) {
        if (!CheckToMove(field, blockID, blockRotate, h, x)) {
            can_Y = h - 1;
            break;
        }
    }

    DrawBlock(can_Y, x, blockID, blockRotate, '/');
}

void createRankList() {
    // 목적: Input파일인 "rank.txt"에서 랭킹 정보를 읽어들임, 읽어들인 정보로 랭킹 목록 생성
    // 1. "rank.txt"열기
    // 2. 파일에서 랭킹정보 읽어오기
    // 3. LinkedList로 저장
    // 4. 파일 닫기
    FILE *fp;
    int i, j;

    //1. 파일 열기
    fp = fopen("rank.txt", "r");

    // 파일이 존재하지 않는다면
    if (!fp) {
        printf("rank.txt does not exist!\n");
        exit(0);
    }

    // 2. 정보읽어오기
    /* int fscanf(FILE* stream, const char* format, ...);
	stream: 데이터를 읽어올 스트림의 FILE 객체를 가리키는 파일포인터
	format: 형식지정자 등등
	변수의 주소: 포인터
	return: 성공할 경우, fscanf 함수는 읽어들인 데이터의 수를 리턴, 실패하면 EOF리턴 */
    // EOF(End Of File): 실제로 이 값은 -1을 나타냄, EOF가 나타날때까지 입력받아오는 if문
    fscanf(fp, "%d", &rank_cnt);
    char ch[NAMELEN];
    int n;

    Rank_Head = NULL;
    Node *end_of_link = Rank_Head;

    while (fscanf(fp, "%s %d", ch, &n) != EOF) {
        Node *tmp = (Node *)malloc(sizeof(Node));

        strcpy(tmp->name, ch);
        tmp->score = n;
        tmp->np = NULL;

        if (Rank_Head == NULL) {
            end_of_link = tmp;
            Rank_Head = tmp;
        } else {
            end_of_link->np = tmp;
            end_of_link = tmp;
        }
    }

    // 4. 파일닫기
    fclose(fp);
}

void rank() {
    //목적: rank 메뉴를 출력하고 점수 순으로 X부터~Y까지 출력함
    //1. 문자열 초기화
    int X = 1, Y = rank_cnt, ch, i, j;
    clear();

    //2. printw()로 3개의 메뉴출력
    printw("1. list ranks from X to Y\n");
    printw("2. list ranks by a specific name\n");
    printw("3. delete a specific rank\n");

    //3. wgetch()를 사용하여 변수 ch에 입력받은 메뉴번호 저장
    while (1) {
        ch = wgetch(stdscr);

        if ('1' <= ch && ch <= '3')
            break;
    }

    //4. 각 메뉴에 따라 입력받을 값을 변수에 저장
    //4-1. 메뉴1: X, Y를 입력받고 적절한 input인지 확인 후(X<=Y), X와 Y사이의 rank 출력
    if (ch == '1') {
        char ch_x[10] = {0}, ch_y[10] = {0};
        int y, x;

        echo();
        printw("X: ");
        scanw("%[^\n]", ch_x);
        printw("Y: ");
        scanw("%[^\n]", ch_y);
        noecho();

        if (strcmp(ch_x, "") != 0)
            X = atoi(ch_x);
        if (strcmp(ch_y, "") != 0)
            Y = atoi(ch_y);

        printw("\tname\t|   Score\n");
        printw("------------------------------\n");
        if (!rank_cnt) {
            printw("search failure: No Data In Rank.txt");
        } else if (X == 0 || Y == 0) {
            printw("search failure: input cannot contain zero");
        } else if (Y > rank_cnt || X < 1) {
            printw("search failure: input out of range");
        } else if (X > Y) {
            printw("search failure: X cannot be bigger than Y");
        } else {
            Node *it = Rank_Head;
            int cnt_to_meet_X = X - 1;
            int cnt = Y - X + 1;
            while (cnt_to_meet_X--) {
                it = it->np;
            }

            for (int i = 0; i < cnt; i++) {
                printw("%-17s| %d\n", it->name, it->score);
                it = it->np;
            }
        }
    }

    //4-2. 메뉴2: 문자열을 받아 저장된 이름과 비교하고 이름에 해당하는 리스트를 출력
    else if (ch == '2') {
        char str[NAMELEN + 1];
        int check = 0;

        printw("Input the name: ");
        echo();
        scanw("%s", str);
        noecho();

        Node *it = Rank_Head;

        int cnt = 0;
        for (int i = 0; i < rank_cnt; i++) {
            if (strcmp(it->name, str) == 0) {
                if (cnt++ == 0) {
                    printw("\tname\t|   Score\n");
                    printw("------------------------------\n");
                }

                printw("%-17s| %d\n", it->name, it->score);
            }
            it = it->np;
        }

        if (!cnt)
            printw("search failure: no name in the list\n");
    }
    //4-3. 메뉴3: rank번호를 입력받아 리스트에서 삭제
    else if (ch == '3') {
        int num;

        printw("Input the rank: ");
        echo();
        scanw("%d", &num);
        noecho();

        if (num > rank_cnt) {
            printw("search failure: the rank not in the list\n");

            getch();
            return;
        } else if (num == 1) {
            Node *head = Rank_Head;
            Rank_Head = Rank_Head->np;
            free(head);

            rank_cnt--;
            writeRankFile();

            printw("result: the rank deleted\n");

            getch();
            return;
        }

        Node *it = Rank_Head;
        Node *to_del;

        int cnt = 0;
        for (int i = 0; i < rank_cnt; i++) {
            if (++cnt == num - 1) {
                to_del = it->np;
                it->np = to_del->np;
                free(to_del);

                rank_cnt--;
                writeRankFile();

                printw("result: the rank deleted\n");

                break;
            }

            it = it->np;
        }
    }
    getch();
}

void writeRankFile() {
    // 목적: 추가된 랭킹 정보가 있으면 새로운 정보를 "rank.txt"에 쓰고 없으면 종료
    int sn, i;
    //1. "rank.txt" 연다
    FILE *fp = fopen("rank.txt", "w");

    //2. 랭킹 정보들의 수를 "rank.txt"에 기록
    fprintf(fp, "%d\n", rank_cnt);
    Node *it = Rank_Head;

    //3. 탐색할 노드가 더 있는지 체크하고 있으면 다음 노드로 이동, 없으면 종료
    while (it) {
        fprintf(fp, "%s %d\n", it->name, it->score);
        it = it->np;
    }

    fclose(fp);
}

void newRank(int score) {
    // 목적: GameOver시 호출되어 사용자 이름을 입력받고 score와 함께 리스트의 적절한 위치에 저장
    char str[NAMELEN + 1];
    int i, j;
    clear();
    //1. 사용자 이름을 입력받음
    printw("your name: ");
    echo();
    scanw("%s", str);
    noecho();

    //2. 새로운 노드를 생성해 이름과 점수를 저장, score_number가
    Node *new_rank = (Node *)malloc(sizeof(Node));
    strcpy(new_rank->name, str);
    new_rank->score = score;
    new_rank->np = NULL;

    if (Rank_Head == NULL) {
        Rank_Head = new_rank;
    } else {
        Node *cur_ptr = Rank_Head->np;
        Node *prev_ptr = Rank_Head;

        // 한 개의 원소만 존재
        if (cur_ptr == NULL) {
            if (prev_ptr->score < score) {
                Rank_Head = new_rank;
                new_rank->np = prev_ptr;
            } else {
                Rank_Head->np = new_rank;
            }
        } else  // 두 개 이상의 원소가 존재
        {
            if (prev_ptr->score < score) {
                Rank_Head = new_rank;
                new_rank->np = prev_ptr;
            } else {
                while (cur_ptr) {
                    if (cur_ptr->score <= score && score <= prev_ptr->score) {
                        new_rank->np = cur_ptr;
                        prev_ptr->np = new_rank;
                        break;
                    }

                    prev_ptr = cur_ptr;
                    cur_ptr = cur_ptr->np;

                    if (cur_ptr == NULL) {
                        prev_ptr->np = new_rank;
                        break;
                    }
                }
            }
        }
    }
    rank_cnt++;

    writeRankFile();
}

void DeleteRecommend(int y, int x, int blockID, int blockRotate) {
    int i, j;
    for (i = 0; i < 4; i++)
        for (j = 0; j < 4; j++) {
            if (block[blockID][blockRotate][i][j] == 1 && i + y >= 0 && field[y + i][x + j] == 0) {
                move(i + y + 1, j + x + 1);
                // attron(A_REVERSE);
                printw("%c", '.');
                // attroff(A_REVERSE);
            }
        }
}

void DrawRecommend(int y, int x, int blockID, int blockRotate) {
    // user code
    int i, j;
    for (i = 0; i < 4; i++)
        for (j = 0; j < 4; j++) {
            if (block[blockID][blockRotate][i][j] == 1 && i + y >= 0) {
                move(i + y + 1, j + x + 1);
                attron(A_REVERSE);
                // printw("%d", Max_ptr->accumulatedScore);
                printw("R");
                attroff(A_REVERSE);
            }
        }
}

void FieldWithRecBlock(char ret[HEIGHT][WIDTH], char f[HEIGHT][WIDTH], int blockY, int blockX, int blockID, int blockRotateID) {
    memcpy(ret, f, sizeof(char) * HEIGHT * WIDTH);

    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            ret[blockY + i][blockX + j] = block[blockID][blockRotateID][i][j];
        }
    }
}

int CalTouched(char f[HEIGHT][WIDTH], int y, int x, int currentBlock, int blockRotate) {
    int cnt = 0;

    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            if (block[currentBlock][blockRotate][i][j] == 1) {
                if (y + i == HEIGHT - 1)
                    cnt++;
                else if (y + i + 1 < HEIGHT && block[currentBlock][blockRotate][i + 1][j] == 0 && f[y + i + 1][x + j] == 1)
                    cnt++;
            }
        }
    }

    return cnt;
}

int recommend(RecNode *root) {
    if (root->level == BLOCK_NUM - 1) {
        return 0;
    }

    int lv = root->level;
    int sz = 0;
    int max = 0;  // 미리 보이는 블럭의 추천 배치까지 고려했을 때 얻을 수 있는 최대 점수

    // user code
    for (int i = 0; i < NUM_OF_ROTATE; i++) {
        for (int x = -5; x < WIDTH + 5; x++) {
            for (int y = blockY; y < HEIGHT; y++) {
                if (!CheckToMove(root->recField, nextBlock[lv], i, y, x)) continue;

                while (y < HEIGHT && CheckToMove(root->recField, nextBlock[lv], i, y, x)) {
                    y++;
                }

                y--;

                RecNode *tmp = malloc(sizeof(RecNode));

                FieldWithRecBlock(tmp->recField, root->recField, y, x, nextBlock[lv], i);

                tmp->parent = root;
                tmp->level = lv + 1;

                tmp->accumulatedScore = root->accumulatedScore + CalTouched(tmp->recField, y, x, nextBlock[lv], i) * 10 + DeleteLine(tmp->recField);
                tmp->curBlockID = nextBlock[lv];
                tmp->recBlockRotate = i;
                tmp->recBlockX = x;
                tmp->recBlockY = y;

                if (tmp->accumulatedScore > max_) {
                    if (tmp->level == 1) {
                        // move(HEIGHT, WIDTH + 20);
                        // printw("%d ", tmp->accumulatedScore);
                        parent->curBlockID = nextBlock[0];
                        parent->recBlockRotate = i;
                        parent->recBlockX = x;
                        parent->recBlockY = y;
                        max_ = tmp->accumulatedScore;
                        Max_ptr = tmp;
                    }
                }

                if (sz == 0)
                    root->child = malloc(sizeof(RecNode *) * ++sz);
                else
                    root->child = realloc(root->child, sizeof(RecNode *) * ++sz);
                root->child[sz - 1] = tmp;
                root->child_cnt = sz;

                recommend(root->child[sz - 1]);
                // recommend(tmp);

                break;
            }
        }
    }

    return max;
}

void recommendedPlay() {
    // user code
    is_RecMode = 1;
    play();
    is_RecMode = 0;
}