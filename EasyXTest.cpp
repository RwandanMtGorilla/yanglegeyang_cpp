#include <stdio.h>
#include <graphics.h>
#include <time.h>

#define WIN_W 600
#define WIN_H 800
#define BLOCK_W 50

#define BLOCK_KINDS_1 5

#define MAX_TOTAL_GROUPS 20 // 总组数上限

IMAGE imgBg;
IMAGE imgBlock[BLOCK_KINDS_1];

int map[MAX_TOTAL_GROUPS * 3];
int stack[7] = { 0 };

struct location {
    int  row;
    int col;
};
struct CardPosition {
    int x, y;
};

CardPosition cardPositions[MAX_TOTAL_GROUPS * 3];
void initCardPositions() {
    // 假设这里随机设置每个卡牌的位置
    srand(time(NULL));
    for (int i = 0; i < MAX_TOTAL_GROUPS * 3; i++) {
        cardPositions[i].x = rand() % (WIN_W - BLOCK_W-50); // 确保卡牌不会超出窗口右边界
        cardPositions[i].y = rand() % (WIN_H - BLOCK_W-250); // 确保卡牌不会超出窗口底部边界
    }
}

void initMap() {
    srand(time(NULL));
    int groupCount = 0;

    while (groupCount < MAX_TOTAL_GROUPS) {
        for (int blockType = 1; blockType <= BLOCK_KINDS_1; blockType++) {
            for (int i = 0; i < 3; i++) { // 每种类型生成3个
                map[groupCount * 3 + i] = blockType;
            }
            groupCount++;
            if (groupCount >= MAX_TOTAL_GROUPS) break; // 检查是否已达到总组数上限
        }
    }

    // 随机打乱 map 中的元素
    for (int i = 0; i < MAX_TOTAL_GROUPS * 3; i++) {
        int j = rand() % (MAX_TOTAL_GROUPS * 3);
        int temp = map[i];
        map[i] = map[j];
        map[j] = temp;
    }
}



void gameInit() {
    loadimage(&imgBg, "img/BG.png");

    char fileName[256];
    for (int i = 0; i < BLOCK_KINDS_1; i++) {
        sprintf_s(fileName, sizeof(fileName), "img/%d.png", i + 1);
        loadimage(&imgBlock[i], fileName);
    }

    initMap();

    initgraph( WIN_W, WIN_H, EW_SHOWCONSOLE);

    initCardPositions();

}
bool isCardCovered(int cardIndex) {
    for (int i = cardIndex + 1; i < MAX_TOTAL_GROUPS * 3; i++) {
        if (map[i] != 0) {
            // 检查卡牌位置是否重叠
            if (cardPositions[i].x < cardPositions[cardIndex].x + BLOCK_W &&
                cardPositions[i].x + BLOCK_W > cardPositions[cardIndex].x &&
                cardPositions[i].y < cardPositions[cardIndex].y + BLOCK_W &&
                cardPositions[i].y + BLOCK_W > cardPositions[cardIndex].y) {
                return true;
            }
        }
    }
    return false;
}


void update() {
    BeginBatchDraw();
    putimage(0, 0, &imgBg);

    for (int i = 0; i < MAX_TOTAL_GROUPS * 3; i++) {
        if (map[i] == 0) continue;
        int imgIndex = map[i] - 1;

        if (isCardCovered(i)) {
            // 加载灰色图像
            char greyFileName[256];
            sprintf_s(greyFileName, sizeof(greyFileName), "img/%d_grey.png", map[i]);
            IMAGE imgGrey;
            loadimage(&imgGrey, greyFileName);
            putimage(cardPositions[i].x, cardPositions[i].y, &imgGrey);
        }
        else {
            // 使用正常颜色的图像
            putimage(cardPositions[i].x, cardPositions[i].y, &imgBlock[imgIndex]);
        }
    }

    int marginX = 26;
    int marginY = 600;
    for (int i = 0; i < 7; i++) {
        if (stack[i]) {
            int x = marginX + i * 70;
            int y = marginY + 5;
            putimage(x, y, &imgBlock[stack[i]-1]);
        }
    }
    EndBatchDraw();
}

bool userClick(struct location* loc) {
    ExMessage msg;
    if (peekmessage(&msg, EM_MOUSE) && msg.message == WM_LBUTTONDOWN) {
        for (int i = 0; i < MAX_TOTAL_GROUPS * 3; i++) {
            if (!isCardCovered(i) &&
                msg.x >= cardPositions[i].x && msg.x < cardPositions[i].x + BLOCK_W &&
                msg.y >= cardPositions[i].y && msg.y < cardPositions[i].y + BLOCK_W) {
                // 用户点击在第i个未被遮挡的卡牌上
                loc->row = i; // 直接使用 i 作为 map 中的索引
                return true;
            }
        }
    }
    return false;
}





void removeMatchingBlocks() {
    int count[BLOCK_KINDS_1 + 1] = { 0 }; // 用于计数每种类型的 block 数量

    // 统计每种类型的 block 数量
    for (int i = 0; i < 7; i++) {
        if (stack[i] != 0) {
            count[stack[i]]++;
        }
    }

    // 检查是否有三个相同的 block
    for (int i = 1; i <= BLOCK_KINDS_1; i++) {
        if (count[i] >= 3) {
            // 移除三个相同的 block
            int removed = 0;
            for (int j = 0; j < 7 && removed < 3; j++) {
                if (stack[j] == i) {
                    stack[j] = 0;
                    removed++;
                }
            }

            // 重新排列 stack 填补空位
            int writeIndex = 0;
            for (int j = 0; j < 7; j++) {
                if (stack[j] != 0) {
                    stack[writeIndex++] = stack[j];
                }
            }
            for (int j = writeIndex; j < 7; j++) {
                stack[j] = 0;
            }

            break; // 执行一次消除后返回
        }
    }
}

void work(struct location* loc) {
    int index = loc->row; // 直接使用 loc->row 作为 index
    int blockType = map[index];
    map[index] = 0;

    // 查找 stack 中第一个空位
    int i = 0;
    for (; stack[i] && i < 7; i++);

    if (i < 7) {
        stack[i] = blockType; // 使用卡牌的类型
        removeMatchingBlocks();
    }
}


bool isStackFull() {
    for (int i = 0; i < 7; i++) {
        if (stack[i] == 0) {
            return false;
        }
    }
    return true;
}

bool isGameWon() {
    for (int i = 0; i < 7; i++) {
        if (stack[i] != 0) {
            return false;
        }
    }

    for (int i = 0; i < MAX_TOTAL_GROUPS * 3; i++) {
        if (map[i] != 0) {
            return false;
        }
    }

    return true;
}


int main(void) {
    gameInit();

    struct location loc;

    while (1) {
        if (userClick(&loc)) {
            work(&loc);
        }
        if (isStackFull()) {
            MessageBox(GetHWnd(), "游戏失败！", "提示", MB_OK);
            break;
        }

        if (isGameWon()) {
            MessageBox(GetHWnd(), "恭喜你，游戏胜利！", "提示", MB_OK);
            break;
        }
        update();
    }
    return 0;
}