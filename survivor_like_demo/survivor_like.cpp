#include <graphics.h>
#include <string>

int index_current_anim = 0;

const int PLAYER_ANIM_NUM = 6;
const int PLAYER_SPEED = 5;

IMAGE img_player_left[PLAYER_ANIM_NUM];
IMAGE img_player_right[PLAYER_ANIM_NUM];

POINT player_pos = { 500, 500 };

#pragma comment(lib, "MSIMG32.LIB")

inline void putimage_alpha(int x, int y, IMAGE* img) {
    int w = img->getwidth();
    int h = img->getheight();
    AlphaBlend(GetImageHDC(NULL), x, y, w, h,
        GetImageHDC(img), 0, 0, w, h, { AC_SRC_OVER, 0, 255, AC_SRC_ALPHA });
}
void loadAnimation() {
    for (size_t i = 0; i < PLAYER_ANIM_NUM; i++) {
        std::wstring path = L"img/player_left_" + std::to_wstring(i) + L".png";
        loadimage(&img_player_left[i], path.c_str());
    }
    for (size_t i = 0; i < PLAYER_ANIM_NUM; i++) {
        std::wstring path = L"img/player_right_" + std::to_wstring(i) + L".png";
        loadimage(&img_player_right[i], path.c_str());
    }
}
//void loadAnimation() {
//    IMAGE img_player_sheet;
//    loadimage(&img_player_sheet, L"image/Idle-Sheet.png");  // 加载包含所有动画帧的图像
//
//    // 假设每帧的宽度为256，高度为80，且帧数为6
//    for (size_t i = 0; i < PLAYER_ANIM_NUM; i++) {
//        // 提取图像的每一帧
//        getimage(&img_player_left[i], i * 256, 0, 256, 80);  // 从图像中获取每一帧
//    }
//}
int main(void) {
    initgraph(1280, 720);

    bool running = true;
    ExMessage msg;
    IMAGE image_background;

    bool is_move_up = false;
    bool is_move_down = false;
    bool is_move_left = false;
    bool is_move_right = false;

    loadAnimation();
    // 加载背景图
    loadimage(&image_background, _T("img/background.png"));

    BeginBatchDraw();

    while (running) {
        DWORD start_time = GetTickCount();

        while (peekmessage(&msg)) {
            // 处理消息
            if (msg.message == WM_KEYDOWN) {
                switch (msg.vkcode) {
                case VK_UP:
                    is_move_up = true;
                    break;
                case VK_DOWN:
                    is_move_down = true;
                    break;
                case VK_LEFT:
                    is_move_left = true;
                    break;
                case VK_RIGHT:
                    is_move_right = true;
                    break;
                }
            }
			else if (msg.message = WM_KEYUP) {
				switch (msg.vkcode) {
				case VK_UP:
					is_move_up = false;
					break;
				case VK_DOWN:
					is_move_down = false;
					break;
				case VK_LEFT:
					is_move_left = false;
					break;
				case VK_RIGHT:
					is_move_right = false;
					break;
				}
			}
        }

		if (is_move_up) player_pos.y -= PLAYER_SPEED;
		if (is_move_down) player_pos.y += PLAYER_SPEED;
		if (is_move_left) player_pos.x -= PLAYER_SPEED;
		if (is_move_right) player_pos.x += PLAYER_SPEED;

        static int counter = 0;
        if (++counter % 5 == 0)
            index_current_anim++;

        // 动画的循环播放
        index_current_anim %= PLAYER_ANIM_NUM;

        cleardevice();

        putimage(0, 0, &image_background);
        putimage_alpha(player_pos.x, player_pos.y, &img_player_left[index_current_anim]);

        FlushBatchDraw();

        DWORD end_time = GetTickCount();
        DWORD delta_time = end_time - start_time;
        if (delta_time < 1000 / 144) {
            Sleep(1000 / 144 - delta_time);
        }
    }

    EndBatchDraw();

    return 0;
}


