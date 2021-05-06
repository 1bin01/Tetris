#include "tetris.h"

static struct sigaction act, oact;

int main() {
	int exit = 0;

	initscr();
	noecho();
	keypad(stdscr, TRUE);

	createRankList();

	srand((unsigned int)time(NULL));

	while (!exit) {
		clear();
		switch (menu()) {
		case MENU_PLAY: play(); break;
		case MENU_RANK: rank(); break;
		case MENU_REC_PLAY: recommendedPlay(); break;
		case MENU_EXIT: exit = 1; break;
		default: break;
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
	DrawNextBlock(nextBlock);
	DrawBlockWithFeatures(blockY, blockX, nextBlock[0], blockRotate);
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
	case ' ':	/* space key*/
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
	if (drawFlag) DrawChange(field, command, nextBlock[0], blockRotate, blockY, blockX);
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
			}
			else printw(".");
		}
	}
}


void PrintScore(int score) {
	move(17, WIDTH + 11);
	printw("%8d", score);
}

void DrawNextBlock(int* nextBlock) {
	int i, j;

	// 블럭 위치를 추천
	if(check_recommend){
		if(root) free(root);
		root = (Node*) malloc(sizeof(Node));
		root->level = 0;
		for(int i = 0; i < HEIGHT; i++)
			for(int j = 0; j < WIDTH; j++)
				root->recField[i][j] = field[i][j];
		recommend(root);
	}

	for (i = 0; i < 4; i++) {
		// 다음 블럭 그리기
		move(4 + i, WIDTH + 13);
		for (j = 0; j < 4; j++) {
			if (block[nextBlock[1]][0][i][j] == 1) {
				attron(A_REVERSE);
				printw(" ");
				attroff(A_REVERSE);
			}
			else printw(" ");
		}

		// 다다음 블럭 그리기
		move(10 + i, WIDTH + 13);
		for (j = 0; j < 4; j++) {
			if (block[nextBlock[2]][0][i][j] == 1) {
				attron(A_REVERSE);
				printw(" ");
				attroff(A_REVERSE);
			}
			else printw(" ");
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
	// user code
	int ny, nx;
	for (int i = 0; i < 4; i++)
		for (int j = 0; j < 4; j++) {
			if (block[currentBlock][blockRotate][i][j]) {
				ny = blockY + i;
				nx = blockX + j;
				if (nx < 0 || ny < 0 || nx >= WIDTH || ny >= HEIGHT || f[ny][nx]) return 0;
			}
		}
	return 1;
}

void DrawChange(char f[HEIGHT][WIDTH], int command, int currentBlock, int blockRotate, int blockY, int blockX) {
	// user code
	int dy = blockY, ny = blockY, nx = blockX, nr = blockRotate;

	//1. 이전 블록 정보를 찾는다. ProcessCommand의 switch문을 참조할 것
	switch (command) {
	case KEY_UP:
		nr = (nr + 3) % 4; break;
	case KEY_DOWN:
		ny--; break;
	case KEY_RIGHT:
		nx--; break;
	case KEY_LEFT:
		nx++; break;
	default:
		break;
	}

	//2. 이전 블록 정보를 지운다. DrawBlock함수 참조할 것.
	while (CheckToMove(f, currentBlock, nr, dy + 1, nx)) dy++;

	for (int i = 0; i < 4; i++)
		for (int j = 0; j < 4; j++) {
			if (block[currentBlock][nr][i][j] && i + ny >= 0) {
				move(i + ny + 1, j + nx + 1);
				printw("%c", '.');
				move(i + dy + 1, j + nx + 1);
				printw("%c", '.');
			}
		}
	move(HEIGHT, WIDTH + 10);

	//3. 새로운 블록 정보를 그린다. 
	DrawBlockWithFeatures(blockY, blockX, currentBlock, blockRotate);
	return;
}

void BlockDown(int sig) {
	// user code
	//강의자료 p26-27의 플로우차트를 참고한다.

	if (CheckToMove(field, nextBlock[0], blockRotate, blockY + 1, blockX)) {
		blockY++;
		DrawChange(field, KEY_DOWN, nextBlock[0], blockRotate, blockY, blockX);
		timed_out = 0; return;
	}

	//gameOver = 1; return;


	// 블록의 y좌표가 -1일 경우 gameOver 변수를 1로 setting한다.
	if (blockY == -1) {
		gameOver = 1;
		timed_out = 0; return;
	}

	// 블록을 field에 합치고 점수를 갱신한다. 
	score += AddBlockToField(field, nextBlock[0], blockRotate, blockY, blockX);

	// 완전히 채워진 line을 지우고 score를 업데이트 한다. 
	score += DeleteLine(field);

	// nextBlock을 갱신한다. 
	nextBlock[0] = nextBlock[1];
	nextBlock[1] = nextBlock[2];

	// nextBlock[2]을 0~6 사이의 random 값으로 설정한다. 
	nextBlock[2] = rand() % 7;

	// blockRotate, blockY, blockX를 초기화 한다. ( InitTetris()에서와 같은 값 )
	blockRotate = 0;
	blockY = -1;
	blockX = WIDTH / 2 - 2;

	// Next 블록을 Next 블록 box안에 그려주고 갱신된 score를 함수를 통하여 화면에 출력한다. 
	DrawNextBlock(nextBlock);
	PrintScore(score);

	// field와 current block을 화면에 갱신하여 출력한다.	
	DrawField();
	timed_out = 0;
}

int AddBlockToField(char f[HEIGHT][WIDTH], int currentBlock, int blockRotate, int blockY, int blockX) {
	// user code
	int y, x;

	//현재 블록과 필드가 맞닿아 있는 필드의 면적을 count
	int touched = 0;

	//Block이 추가된 영역의 필드값을 바꾼다.
	for (int i = 0; i < 4; i++)
		for (int j = 0; j < 4; j++)
			if (block[currentBlock][blockRotate][i][j]) {
				y = blockY + i;
				x = blockX + j;
				if (y < 0 || x < 0 || y >= HEIGHT || x >= WIDTH) continue;
				f[y][x] = 1;
				// 아래의 필드가 채워져있다면 점수 증가
				if (y + 1 == HEIGHT || f[y + 1][x] == 1) touched++;
			}
	return touched * 10;
}

int DeleteLine(char f[HEIGHT][WIDTH]) {
	// user code
	int ret = 0, flag;

	for (int i = 0; i < HEIGHT; i++) {
		//1. 필드를 탐색하여, 꽉 찬 구간이 있는지 탐색한다.
		flag = 1;
		for (int j = 0; j < WIDTH; j++)
			if (!f[i][j]) flag = 0;

		//2. 꽉 찬 구간이 있으면 해당 구간을 지운다. 즉, 해당 구간으로 필드값을 한칸씩 내린다.
		if (flag) {
			for (int k = i; k; k--)
				for (int j = 0; j < WIDTH; j++)
					f[k][j] = f[k - 1][j];

			for (int j = 0; j < WIDTH; j++) f[0][j] = 0;
			ret++;
		}
	}
	return ret * ret * 100;
}

///////////////////////////////////////////////////////////////////////////

void DrawShadow(int y, int x, int blockID, int blockRotate) {
	// user code

	// 그림자의 위치(현재 블록을 가장 아래로 내렸을 때, 더 이상 내려갈 수 없는 위치)를 찾는다.
	while (CheckToMove(field, blockID, blockRotate, y + 1, x)) y++;

	// 그 위치에 현재의 블록을 ‘/’문자를 tile로 하여 그림자를 그린다.
	DrawBlock(y, x, blockID, blockRotate, '/');
	return;
}

void DrawBlockWithFeatures(int y, int x, int blockID, int blockRotate) {
	// DrawBlock(), DrawShadow() 함수를 호출하는 함수로, 기존의 DrawBlock() 함수의 위치에 삽입하여  움직임이 갱신될 때마다 현재 블록과 그림자를 함께 그리도록 한다.
	
	// 추천 위치를 그려준다.
	DrawRecommend(root->recBlockY, root->recBlockX, root->curBlockID, root->recBlockRotate);

	// 그림자를 그린다.
	DrawShadow(y, x, blockID, blockRotate);

	// 블록을 그린다.
	DrawBlock(y, x, blockID, blockRotate, ' ');
	return;
}


void createRankList() {
	// 목적: Input파일인 "rank.txt"에서 랭킹 정보를 읽어들임, 읽어들인 정보로 랭킹 목록 생성
	// 1. "rank.txt"열기
	// 2. 파일에서 랭킹정보 읽어오기
	// 3. LinkedList로 저장
	// 4. 파일 닫기
	FILE* fp;
	int i, j, score;
	char user_name[NAMELEN + 1];

	//1. 파일 열기
	fp = fopen("rank.txt", "r");

	// 2. 정보읽어오기
	/* int fscanf(FILE* stream, const char* format, ...);
	stream:데이터를 읽어올 스트림의 FILE 객체를 가리키는 파일포인터
	format: 형식지정자 등등
	변수의 주소: 포인터
	return: 성공할 경우, fscanf 함수는 읽어들인 데이터의 수를 리턴, 실패하면 EOF리턴 */
	// EOF(End Of File): 실제로 이 값은 -1을 나타냄, EOF가 나타날때까지 입력받아오는 if문
	head = malloc(sizeof(NODE));
	head->link = NULL;
	if (fscanf(fp, "%d", &score_number) != EOF) {
		NODE* cur = head;
		for (int i = 0; i < score_number; i++) {
			fscanf(fp, "%s %d", user_name, &score);
			NODE* nd = malloc(sizeof(NODE));
			nd->link = NULL;
			strcpy(nd->name, user_name);
			nd->score = score;
			cur->link = nd;
			cur = nd;
		}
	}
	else {
		score_number = 0;
	}
	// 4. 파일닫기
	fclose(fp);
}

void rank() {
	//목적: rank 메뉴를 출력하고 점수 순으로 X부터~Y까지 출력함
	//1. 문자열 초기화
	int ch, i, j, x = 1, y = score_number, count;
	clear();

	//2. printw()로 3개의 메뉴출력
	printw("1. list ranks from X to Y\n");
	printw("2. list ranks by a specific name\n");
	printw("3. delete a specific rank\n");

	//3. wgetch()를 사용하여 변수 ch에 입력받은 메뉴번호 저장
	ch = wgetch(stdscr);

	//4. 각 메뉴에 따라 입력받을 값을 변수에 저장
	//4-1. 메뉴1: X, Y를 입력받고 적절한 input인지 확인 후(X<=Y), X와 Y사이의 rank 출력
	if (ch == '1') {
		echo();
		printw("X: ");
		scanw("%d", &x);
		printw("Y: ");
		scanw("%d", &y);

		if (x > y || y > score_number || x < 1) {
			printw("search failure: no rank in the list\n");
		}
		else {
			NODE* cur = head;
			printw("       name       |   score   \n");
			printw("------------------------------\n");
			for (int i = 0; i < x; i++) cur = cur->link;
			for (int i = x; i <= y; i++) {
				printw(" %-17s| %-10d\n", cur->name, cur->score);
				cur = cur->link;
			}
		}
		noecho();
	}

	//4-2. 메뉴2: 문자열을 받아 저장된 이름과 비교하고 이름에 해당하는 리스트를 출력
	else if (ch == '2') {
		char str[NAMELEN + 1];
		int check = 0;
		echo();
		printw("Input the name: ");
		scanw("%s", str);

		// 일치하는 정보가 있는 지 확인
		count = 0;
		NODE* cur = head;
		while(cur){
			if(!strcmp(cur->name, str)){
				count++;
			}
			cur = cur->link;
		}

		if(!count){
			// 일치하는 정보가 없는 경우
			printw("\nsearch failure: no name in the list\n");
		}
		else{
			// 일치하는 정보를 출력
			printw("       name       |   score   \n");
			printw("------------------------------\n");

			cur = head;
			while(cur){
				if(!strcmp(cur->name, str)) {
					printw(" %-17s| %-10d\n", cur->name, cur->score);
				}
				cur = cur->link;
			}
		}
		noecho();
	}

	//4-3. 메뉴3: rank번호를 입력받아 리스트에서 삭제
	else if (ch == '3') {
		int num;
		echo();
		printw("Input the rank: ");
		scanw("%d", &num);
		if(num < 1 || num > score_number){
			printw("\nsearch failure: the rank not in the list\n");
		}
		else{
			count = 0;
			NODE* cur = head, *tmp;
			for(int i= 0; i < num - 1; i++) cur = cur->link;

			// 노드 삭제하기
			tmp = cur->link;
			cur->link = tmp->link;
			free(tmp);
			score_number--;
			writeRankFile();
			printw("\nresult: the rank deleted\n");
		}
		noecho();
	}
	getch();

}

void writeRankFile() {
	// 목적: 추가된 랭킹 정보가 있으면 새로운 정보를 "rank.txt"에 쓰고 없으면 종료
	int sn, i;
	//1. "rank.txt" 연다
	FILE* fpw = fopen("rank.txt", "w");

	//2. 랭킹 정보들의 수를 "rank.txt"에 기록
	fprintf(fpw, "%d\n", score_number);

	//3. 탐색할 노드가 더 있는지 체크하고 있으면 다음 노드로 이동, 없으면 종료
	NODE* cur = head->link;
	while (cur) {
		fprintf(fpw, "%s %d\n", cur->name, cur->score);
		cur = cur->link;
	}
	fclose(fpw);
	return;
}

void newRank(int score) {
	// 목적: GameOver시 호출되어 사용자 이름을 입력받고 score와 함께 리스트의 적절한 위치에 저장
	char str[NAMELEN + 1];
	int i, j;
	clear();
	//1. 사용자 이름을 입력받음
	echo();
	printw("your name: ");
	scanw("%s", str);
	noecho();

	//2. 새로운 노드를 생성해 이름과 점수를 저장, score_number가
	NODE* nd = malloc(sizeof(NODE));
	strcpy(nd->name, str);
	nd->score = score;
	nd->link = NULL;

	NODE* cur = head;
	while (1) {
		if (cur->link == NULL) {
			cur->link = nd;
			break;
		}
		else {
			if (score > cur->link->score) {
				nd->link = cur->link;
				cur->link = nd;
				break;
			}
			else {
				cur = cur->link;
			}
		}
	}
	score_number++;
	writeRankFile();
	return;
}

void recommend(Node* root){
	int y, x, score;
	root->accumulatedScore = -1;
	root->curBlockID = nextBlock[root->level];
	root->recBlockY = -1;

	root->child = (Node**) malloc(sizeof(Node*) * CHILDREN_MAX + 10); 
	//현재블록과 다음 2개의 블록을 고려해서 모든 play 시퀀스를 나타낼 수 있는 tree를 구성하고, tree의 정보를 바탕으로 사용자가 좋은 score를 얻을 수 있는현재 블록의 위치를 계산하는 기능을 한다.
	for(int rotate = 0; rotate < 4; rotate++){
		for(int j = 0; j <= WIDTH; j++){
			// 불가능한 경우인 경우 continue한다.
			x = j - 2;
			if(!CheckToMove(root->recField, root->curBlockID, rotate, 0, x)) continue;

			root->child[j] = (Node*) malloc(sizeof(Node));
			//블럭이 들어간 새로운 필드를 만든다.
			score = y = 0;
			while (CheckToMove(root->recField, root->curBlockID, rotate, y + 1, x)) y++;

			// score를 갱신한다.
			for(int i = 0; i < HEIGHT; i++)
				for(int k = 0; k < WIDTH; k++)
					root->child[j]->recField[i][k] = root->recField[i][k];
			score += AddBlockToField(root->child[j]->recField, root->curBlockID, rotate, y, x);
			score += DeleteLine(root->child[j]->recField);

			root->child[j]->level = root->level + 1;
			//재귀적으로 점수를 구한다.
			if(root->child[j]->level < BLOCK_NUM){
				recommend(root->child[j]);
				score += root->child[j]->accumulatedScore;
			}

			// 점수의 최댓값을 구한다.
			if(score > root->accumulatedScore || (score == root->accumulatedScore && (root->recBlockY == -1 || root->recBlockY < y))) {
				root->accumulatedScore = score;
				root->recBlockY = y;
				root->recBlockX = x;
				root->recBlockRotate = rotate;
			}
			free(root->child[j]);
		}	
	}
	return;
}

void DrawRecommend(int y, int x, int blockID, int blockRotate) {
	DrawBlock(y, x, blockID, blockRotate, 'R');
	return;
}


void recommendedPlay() {
	play();
	return;
}