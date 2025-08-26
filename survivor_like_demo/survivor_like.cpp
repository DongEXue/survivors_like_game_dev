#include <graphics.h>
#include <string>
#include <vector>

#pragma comment(lib, "MSIMG32.LIB")
const int WINDOW_WIDTH = 1280;
const int WINDOW_HEIGHT = 720;


inline void putimage_alpha(int x, int y, IMAGE* img) {
    int w = img->getwidth();
    int h = img->getheight();
    AlphaBlend(GetImageHDC(NULL), x, y, w, h,
        GetImageHDC(img), 0, 0, w, h, { AC_SRC_OVER, 0, 255, AC_SRC_ALPHA });
}

class Animation {
public:
    Animation(LPCTSTR path, int num, int interval) {
        interval_ms = interval;

        //加载图片，因为素材中的命名规律遂将路径参数作为字符串格式化的模板
        TCHAR path_file[256];
        for (size_t i = 0; i < num; i++) {
            _stprintf_s(path_file, path, i);

            IMAGE* frame = new IMAGE();
            loadimage(frame, path_file);
            frame_list.push_back(frame);
        }
    }

    //动画对象的析构函数
    ~Animation() {
        for (size_t i = 0; i < frame_list.size(); i++) {
            delete(frame_list[i]);
        }
    }
    /*
    @param delta:距离上次调用Play函数过去了多久
    */
    void Play(int x, int y, int delta) {
        timer += delta;
        if (timer >= interval_ms) {
            index_frame = (index_frame + 1) % frame_list.size();
            timer = 0;
        }

        putimage_alpha(x, y, frame_list[index_frame]);
    }
private:
    int timer = 0;  //动画计时器
    int index_frame = 0;    //动画帧索引
    int interval_ms = 0;    //帧间隔
    std::vector<IMAGE*> frame_list;
};

class Player {
public:
    Player() {
        loadimage(&img_shadow, _T("img/shadow_player.png"));
		anim_left = new Animation(_T("img/player_left_%d.png"), 6, 45);
		anim_right = new Animation(_T("img/player_right_%d.png"), 6, 45);
    }
    ~Player() {
		delete anim_left;
		delete anim_right;
    }

    void ProcessEvent(const ExMessage& msg) {
        switch (msg.message) {
            case WM_KEYDOWN:
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
                break;

			case WM_KEYUP:
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
                break;
        }
    }

    void Move() {
        int dir_x = is_move_right - is_move_left;
        int dir_y = is_move_down - is_move_up;
        double len_dir = sqrt(dir_x * dir_x + dir_y * dir_y);
        if (len_dir != 0) {
            double normalized_x = dir_x / len_dir;
            double normalized_y = dir_y / len_dir;
            position.x += (int)(PLAYER_SPEED * normalized_x);
            position.y += (int)(PLAYER_SPEED * normalized_y);
        }
        if (position.x < 0) position.x = 0;
        if (position.y < 0) position.y = 0;
        if (position.x + PLAYER_WIDTH > WINDOW_WIDTH) position.x = WINDOW_WIDTH - PLAYER_WIDTH;
        if (position.y + PLAYER_HEIGHT > WINDOW_HEIGHT) position.y = WINDOW_HEIGHT - PLAYER_HEIGHT;
    }

    void Draw(int delta) {
        int pos_shadow_x = position.x + (PLAYER_WIDTH / 2 - SHADOW_WIDTH / 2);
        int pos_shadow_y = position.y + PLAYER_HEIGHT - 8;
        putimage_alpha(pos_shadow_x, pos_shadow_y, &img_shadow);

		int dir_x = is_move_right - is_move_left;

        if (dir_x < 0) {
            is_facing_left = true;
        }
        else if (dir_x > 0) {
            is_facing_left = false;
        }

        if (is_facing_left == true) {
            anim_left->Play(position.x, position.y, delta);
        }
        else {
            anim_right->Play(position.x, position.y, delta);
        }
    }

    const POINT& GetPosition() const {
        return position;
    }

private:
    const int PLAYER_SPEED = 5;
    const int PLAYER_WIDTH = 80;
    const int PLAYER_HEIGHT = 80;
    const int SHADOW_WIDTH = 32;

private:
    IMAGE img_shadow;
	Animation* anim_left;
	Animation* anim_right;
	POINT position = { 500, 500 };
    bool is_move_up = false;
    bool is_move_down = false;
    bool is_move_left = false;
    bool is_move_right = false;
	bool is_facing_left = false;
};

class Bullet {
public:
    POINT position = { 0, 0 };

public:
    Bullet() = default;
    ~Bullet() = default;

    void Draw() const {
        setlinecolor(RGB(255, 155, 10));
        setfillcolor(RGB(255, 75, 10));
        fillcircle(position.x, position.y, RADIUS);
    }

private:
    const int RADIUS = 10;
};

class Enemy {
public:
    Enemy() {
		loadimage(&img_shadow, _T("img/shadow_enemy.png"));
		anim_left = new Animation(_T("img/enemy_left_%d.png"), 6, 45);
		anim_right = new Animation(_T("img/enemy_right_%d.png"), 6, 45);

        enum class SpawnEdge {
			TOP = 0 ,
			BOTTOM,
			LEFT,
			RIGHT
        };

		SpawnEdge edge = (SpawnEdge)(rand() % 4);
		switch (edge) {
		case SpawnEdge::TOP:
			position.x = rand() % (WINDOW_WIDTH - ENEMY_WIDTH);
			position.y = -ENEMY_HEIGHT;
			break;
		case SpawnEdge::BOTTOM:
			position.x = rand() % (WINDOW_WIDTH - ENEMY_WIDTH);
			position.y = WINDOW_HEIGHT;
			break;
		case SpawnEdge::LEFT:
			position.x = -ENEMY_WIDTH;
			position.y = rand() % (WINDOW_HEIGHT - ENEMY_HEIGHT);
			break;
		case SpawnEdge::RIGHT:
			position.x = WINDOW_WIDTH;
			position.y = rand() % (WINDOW_HEIGHT - ENEMY_HEIGHT);
			break;
		}
    }

	bool CheckPlayerCollision(const Player& player) {
        return false;
	}

	bool CheckBulletCollision(const Bullet& bullet) {
		return false;
	}

	// Enemy朝向Player移动
    void Move(const Player& player) {
		const POINT& player_position = player.GetPosition();
		int dir_x = player_position.x - position.x;
		int dir_y = player_position.y - position.y;

        if (dir_x < 0) is_facing_left = true;
        else if (dir_x > 0) is_facing_left = false;

		double len_dir = sqrt(dir_x * dir_x + dir_y * dir_y);
        if (len_dir != 0) {
            double normalized_x = dir_x / len_dir;
            double normalized_y = dir_y / len_dir;
            position.x += (int)(ENEMY_SPEED * normalized_x);
            position.y += (int)(ENEMY_SPEED * normalized_y);
        }
    }

	void Draw(int delta) {
		int pos_shadow_x = position.x + (ENEMY_WIDTH / 2 - SHADOW_WIDTH / 2);
		int pos_shadow_y = position.y + ENEMY_HEIGHT - 35;
		putimage_alpha(pos_shadow_x, pos_shadow_y, &img_shadow);

		if (is_facing_left == true) {
			anim_left->Play(position.x, position.y, delta);
		}
		else {
			anim_right->Play(position.x, position.y, delta);
		}
	}

    ~Enemy() {
		delete anim_left;
        delete anim_right;
    };

private:
	const int ENEMY_SPEED = 2;
	const int ENEMY_WIDTH = 80;
	const int ENEMY_HEIGHT = 80;
	const int SHADOW_WIDTH = 48;

private:
	IMAGE img_shadow;
	Animation* anim_left;
	Animation* anim_right;
	POINT position = { 0, 0 };
	bool is_facing_left = false;
};

void TryGenerateEnemy(std::vector<Enemy*>& enemy_list) {
	const int GENERATE_INTERVAL = 100; // 每秒生成一个敌人
	static int timer = 0;
    if ((++timer) % GENERATE_INTERVAL == 0) {
		enemy_list.push_back(new Enemy());
    }
}

int main(void) {
    initgraph(1280, 720);

    bool running = true;
    ExMessage msg;
    IMAGE image_background;
    Player player;
	std::vector<Enemy*> enemy_list;

    // 加载背景图
    loadimage(&image_background, _T("img/background.png"));

    BeginBatchDraw();

    while (running) {
        DWORD start_time = GetTickCount();

        while (peekmessage(&msg)) {
            // 处理消息
			player.ProcessEvent(msg);
        }
        
		player.Move();
		TryGenerateEnemy(enemy_list);
        for (Enemy* enemy : enemy_list)
			enemy->Move(player);

        cleardevice();

        putimage(0, 0, &image_background);
        player.Draw(1000 / 144);
		for (Enemy* enemy : enemy_list)
			enemy->Draw(1000 / 144);

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


