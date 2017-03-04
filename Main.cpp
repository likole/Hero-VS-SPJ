//头文件
#include<windows.h>
#include<tchar.h>
#include<time.h>

//库文件
#pragma comment(lib,"winmm.lib")
#pragma comment(lib,"Msimg32.lib")

//宏定义
#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 600
#define WINDOW_TITILE L"勇者斗恶犬"

//结构体
struct CHARACTER
{
	int	NowHp;
	int	MaxHp;
	int	NowMp;
	int	MaxMp;
	int	Level;
	int	Strength;
	int	Intelligence;
	int	Agility;
};

//枚举体
//游戏状态
enum GameStateTypes
{
	GameStateTitle = 0,//标题
	GameStateBackground = 1,//背景介绍
	GameStatePlaying = 2,//进行中
	GameStateAttack = 3,//攻击
	GameStateOver = 4//游戏结束
};

//攻击状态
enum ActionTypes
{
	ACTION_TYPE_NORMAL = 0,//普通攻击
	ACTION_TYPE_CRITICAL = 1,//致命一击
	ACTION_TYPE_MAGIC = 2,//魔法攻击
	ACTION_TYPE_MISS = 3,//攻击落空
	ACTION_TYPE_RECOVER = 4,//回血
};


//全局变量声明
HDC	g_hdc = NULL, g_mdc = NULL, g_bufdc = NULL;
DWORD g_tPre = 0, g_tNow = 0;
RECT g_rect;
int g_FrameNum, g_iTxtNum;
wchar_t text[8][100];
CHARACTER SPJ, Hero;//恶犬，勇者
ActionTypes	spjActionType, HeroActionType;
GameStateTypes GameState;

//资源句柄
HBITMAP g_hTitle;//游戏标题
HBITMAP g_hSetting;//游戏背景
HBITMAP g_hBackGround;//战斗背景
HBITMAP	g_hGameOver, g_hVictory;//游戏结束，游戏胜利
HBITMAP g_hBossInfo, g_hHeroInfo;
HBITMAP	g_hMonsterBitmap, g_hHeroBitmap, g_hRecoverSkill, g_hRecoverSkill2;
//HBITMAP	g_hSkillButton1, g_hSkillButton2, g_hSkillButton3;  
HBITMAP	g_hSkillButtons;
HBITMAP	g_hHeroSkill1, g_hHeroSkill2, g_hHeroSkill3;
HBITMAP	g_hBossSkill1, g_hBossSkill2, g_hBossSkill3;


//全局函数声明
LRESULT CALLBACK WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);//窗口回调函数
BOOL Game_Init(HWND hwnd);//游戏初始化
VOID Game_Main(HWND hwnd);//游戏贴图
BOOL Game_ShutDown(HWND hwnd);//资源清理
VOID HeroLogic();//玩家攻击逻辑
VOID BossLogic();//怪物攻击逻辑
VOID Message_Insert(wchar_t* str);
VOID HeroPaint(HWND hwnd);//玩家攻击效果绘制
VOID BossPaint(HWND hwnd);//怪物攻击效果绘制
VOID Die_Check();


//主函数
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	//窗口类参数
	WNDCLASSEX wndclass = { 0 };
	wndclass.cbClsExtra = 0;
	wndclass.cbSize = sizeof(WNDCLASSEX);
	wndclass.cbWndExtra = 0;
	wndclass.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
	wndclass.hCursor = LoadCursor(NULL, IDC_ARROW);
	wndclass.hIcon = (HICON)::LoadImage(NULL, L"icon.ico", IMAGE_ICON, 0, 0, LR_DEFAULTCOLOR | LR_LOADFROMFILE);
	wndclass.hInstance = hInstance;
	wndclass.lpfnWndProc = WndProc;
	wndclass.lpszClassName = L"MainWindowClass";
	wndclass.lpszMenuName = NULL;
	wndclass.style = CS_HREDRAW | CS_VREDRAW;

	//注册窗口类
	if (!RegisterClassEx(&wndclass))return -1;

	//创建窗口
	HWND hwnd = CreateWindow(L"MainWindowClass", WINDOW_TITILE, WS_OVERLAPPEDWINDOW&(~WS_THICKFRAME)&(~WS_MAXIMIZEBOX), CW_USEDEFAULT, CW_USEDEFAULT, WINDOW_WIDTH, WINDOW_HEIGHT, NULL, NULL, hInstance, NULL);

	//显示窗口
	ShowWindow(hwnd, nCmdShow);
	UpdateWindow(hwnd);

	//资源初始化
	if (!Game_Init(hwnd)) {
		MessageBox(hwnd, L"资源初始化失败", L"消息窗口", 0);
		return FALSE;
	}
	PlaySound(L"GameMedia\\title.wav", NULL, SND_FILENAME | SND_ASYNC | SND_LOOP);

	//消息循环
	MSG msg = { 0 };
	while (msg.message != WM_QUIT) {
		if (PeekMessage(&msg, 0, 0, 0, PM_REMOVE)) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		else {
			g_tNow = GetTickCount();
			if (g_tNow - g_tPre >= 60) Game_Main(hwnd);
		}
	}

	//注销窗口类
	UnregisterClass(L"MainWindowClass", wndclass.hInstance);
	return 0;
}




//窗口过程函数
LRESULT CALLBACK WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{

	case WM_KEYDOWN:
		//ESC退出游戏
		if (wParam == VK_ESCAPE) {
			PostQuitMessage(0);
		}

		//回车或空格进入游戏
		else if (wParam == VK_RETURN || wParam == VK_SPACE) {
			if (GameState == GameStateTitle)
			{
				GameState = GameStateBackground;
				PlaySound(NULL, 0, 0);
				PlaySound(L"GameMedia\\background.wav", NULL, SND_FILENAME | SND_ASYNC | SND_LOOP);
			}
			else if (GameState == GameStateBackground) {
				GameState = GameStatePlaying;
				PlaySound(NULL, 0, 0);
				PlaySound(L"GameMedia\\battle.wav", NULL, SND_FILENAME | SND_ASYNC | SND_LOOP);
			}
		}

		//释放技能（键盘）
		else if (GameState == GameStatePlaying) {
			if (wParam == 'J') {
				GameState = GameStateAttack;
				HeroActionType = ACTION_TYPE_NORMAL;
			}
			else if (wParam == 'K')
			{
				if (Hero.NowMp >= 30)
				{
					GameState = GameStateAttack;
					HeroActionType = ACTION_TYPE_MAGIC;
				}
				else
				{
					Message_Insert(L"魔法值不足，无法发动烈火剑法");
				}
			}
			else if (wParam == 'L')
			{
				if (Hero.NowMp >= 40)
				{
					GameState = GameStateAttack;
					HeroActionType = ACTION_TYPE_RECOVER;
				}
				else
				{
					Message_Insert(L"魔法值不足，无法使用恐鳌之心");
				}
			}
			else if (wParam == 'S')
			{
				SPJ.Level = 5;
				SPJ.Strength = 6;
				SPJ.Agility = 8;
				SPJ.Intelligence = 6;
			}
		}
		break;

	case WM_LBUTTONDOWN:
		//释放技能（鼠标）
		if (GameState == GameStatePlaying) {
			int x = LOWORD(lParam);
			int y = HIWORD(lParam);
			if (x > g_rect.right - 305 && x<g_rect.right - 5 && y>g_rect.bottom - 205 && y < g_rect.bottom - 145) {
				GameState = GameStateAttack;
				HeroActionType = ACTION_TYPE_NORMAL;
			}
			else if (x > g_rect.right - 305 && x<g_rect.right - 5 && y>g_rect.bottom - 135 && y < g_rect.bottom - 75)
			{
				if (Hero.NowMp >= 30)
				{
					GameState = GameStateAttack;
					HeroActionType = ACTION_TYPE_MAGIC;
				}
				else
				{
					Message_Insert(L"魔法值不足，无法发动烈火剑法");
				}
			}
			else if (x > g_rect.right - 305 && x<g_rect.right - 5 && y>g_rect.bottom - 65 && y < g_rect.bottom - 5)
			{
				if (Hero.NowMp >= 40)
				{
					GameState = GameStateAttack;
					HeroActionType = ACTION_TYPE_RECOVER;
				}
				else
				{
					Message_Insert(L"魔法值不足，无法使用恐鳌之心");
				}
			}
		}
		break;

	case WM_DESTROY:
		Game_ShutDown(hwnd);
		PostQuitMessage(0);
		break;

	default:
		return DefWindowProc(hwnd, message, wParam, lParam);
		break;
	}

	return 0;
}

//游戏初始化
BOOL Game_Init(HWND hwnd)
{
	//随机数种子
	srand((unsigned)time(NULL));

	//三缓冲体系
	HBITMAP bitmap;
	g_hdc = GetDC(hwnd);
	g_mdc = CreateCompatibleDC(g_hdc);
	g_bufdc = CreateCompatibleDC(g_hdc);
	bitmap = CreateCompatibleBitmap(g_hdc, WINDOW_WIDTH, WINDOW_HEIGHT);
	SelectObject(g_mdc, bitmap);

	//加载图片
	g_hBackGround = (HBITMAP)LoadImage(NULL, L"GameMedia\\bg.bmp", IMAGE_BITMAP, 800, 600, LR_LOADFROMFILE);
	g_hTitle = (HBITMAP)LoadImage(NULL, L"GameMedia\\title.bmp", IMAGE_BITMAP, 800, 600, LR_LOADFROMFILE);
	g_hSetting = (HBITMAP)LoadImage(NULL, L"GameMedia\\background.bmp", IMAGE_BITMAP, 800, 600, LR_LOADFROMFILE);
	g_hMonsterBitmap = (HBITMAP)LoadImage(NULL, L"GameMedia\\monster.bmp", IMAGE_BITMAP, 360, 360, LR_LOADFROMFILE);
	g_hHeroBitmap = (HBITMAP)LoadImage(NULL, L"GameMedia\\hero.bmp", IMAGE_BITMAP, 360, 360, LR_LOADFROMFILE);
	g_hHeroSkill1 = (HBITMAP)LoadImage(NULL, L"GameMedia\\heroslash.bmp", IMAGE_BITMAP, 364, 140, LR_LOADFROMFILE);
	g_hHeroSkill2 = (HBITMAP)LoadImage(NULL, L"GameMedia\\heromagic.bmp", IMAGE_BITMAP, 960, 384, LR_LOADFROMFILE);
	g_hHeroSkill3 = (HBITMAP)LoadImage(NULL, L"GameMedia\\herocritical.bmp", IMAGE_BITMAP, 574, 306, LR_LOADFROMFILE);
	//g_hSkillButton1 = (HBITMAP)LoadImage(NULL, L"GameMedia\\skillbutton1.bmp", IMAGE_BITMAP, 50, 50, LR_LOADFROMFILE); 
	//g_hSkillButton2 = (HBITMAP)LoadImage(NULL, L"GameMedia\\skillbutton2.bmp", IMAGE_BITMAP, 50, 50, LR_LOADFROMFILE);
	//g_hSkillButton3 = (HBITMAP)LoadImage(NULL, L"GameMedia\\skillbutton3.bmp", IMAGE_BITMAP, 50, 50, LR_LOADFROMFILE);
	g_hSkillButtons = (HBITMAP)LoadImage(NULL, L"GameMedia\\skillbuttons.bmp", IMAGE_BITMAP, 600, 200, LR_LOADFROMFILE);
	g_hBossSkill1 = (HBITMAP)LoadImage(NULL, L"GameMedia\\monsterslash.bmp", IMAGE_BITMAP, 960, 384, LR_LOADFROMFILE);
	g_hBossSkill2 = (HBITMAP)LoadImage(NULL, L"GameMedia\\monstermagic.bmp", IMAGE_BITMAP, 960, 960, LR_LOADFROMFILE);
	g_hBossSkill3 = (HBITMAP)LoadImage(NULL, L"GameMedia\\monstercritical.bmp", IMAGE_BITMAP, 960, 576, LR_LOADFROMFILE);
	g_hRecoverSkill = (HBITMAP)LoadImage(NULL, L"GameMedia\\recover.bmp", IMAGE_BITMAP, 960, 1152, LR_LOADFROMFILE);
	g_hRecoverSkill2 = (HBITMAP)LoadImage(NULL, L"GameMedia\\recover2.bmp", IMAGE_BITMAP, 960, 1152, LR_LOADFROMFILE);
	g_hBossInfo = (HBITMAP)LoadImage(NULL, L"GameMedia\\bossinfo.bmp", IMAGE_BITMAP, 600, 100, LR_LOADFROMFILE);
	g_hHeroInfo = (HBITMAP)LoadImage(NULL, L"GameMedia\\heroinfo.bmp", IMAGE_BITMAP, 600, 100, LR_LOADFROMFILE);
	g_hVictory = (HBITMAP)LoadImage(NULL, L"GameMedia\\victory.bmp", IMAGE_BITMAP, 800, 600, LR_LOADFROMFILE);
	g_hGameOver = (HBITMAP)LoadImage(NULL, L"GameMedia\\gameover.bmp", IMAGE_BITMAP, 800, 600, LR_LOADFROMFILE);

	//获取客户端大小
	GetClientRect(hwnd, &g_rect);

	//设定玩家属性
	Hero.NowHp = Hero.MaxHp = 3000;
	Hero.Level = 6;
	Hero.NowMp = Hero.MaxMp = 60;
	Hero.Strength = 10;
	Hero.Agility = 20;
	Hero.Intelligence = 10;

	//设定BOSS属性
	SPJ.NowHp = SPJ.MaxHp = 5000;
	SPJ.Level = 10;
	SPJ.Strength = 10;
	SPJ.Agility = 10;
	SPJ.Intelligence = 10;

	//初始化提示消息条目
	g_iTxtNum = 0;

	//设定字体
	HFONT hFont;
	hFont = CreateFont(20, 0, 0, 0, 700, 0, 0, 0, GB2312_CHARSET, 0, 0, 0, 0, TEXT("微软雅黑"));
	SelectObject(g_mdc, hFont);
	SetBkMode(g_mdc, TRANSPARENT);

	//更改游戏状态
	GameState = GameStateTitle;

	Game_Main(hwnd);
	return TRUE;
}

//游戏贴图
VOID Game_Main(HWND hwnd)
{
	wchar_t str[100];

	//游戏标题
	if (GameState == GameStateTitle)
	{
		SelectObject(g_mdc, g_hTitle);
		BitBlt(g_hdc, 0, 0, WINDOW_WIDTH, WINDOW_HEIGHT, g_mdc, 0, 0, SRCCOPY);
		return;
	}

	//游戏背景设定
	if (GameState == GameStateBackground) {
		SelectObject(g_mdc, g_hSetting);
		BitBlt(g_hdc, 0, 0, WINDOW_WIDTH, WINDOW_HEIGHT, g_mdc, 0, 0, SRCCOPY);
		return;
	}

	//游戏结束
	if (GameState == GameStateOver)
	{
		if (g_FrameNum < 30)
		{
			g_FrameNum++;
		}
		else
		{
			if (Hero.NowHp <= 0)
			{
				SelectObject(g_mdc, g_hGameOver);
				BitBlt(g_hdc, 0, 0, WINDOW_WIDTH, WINDOW_HEIGHT, g_mdc, 0, 0, SRCCOPY);
			}
			else
			{
				SelectObject(g_mdc, g_hVictory);
				BitBlt(g_hdc, 0, 0, WINDOW_WIDTH, WINDOW_HEIGHT, g_mdc, 0, 0, SRCCOPY);
			}
			return;
		}

	}
	//进行中
	//绘制背景图
	SelectObject(g_bufdc, g_hBackGround);
	BitBlt(g_mdc, 0, 0, WINDOW_WIDTH, WINDOW_HEIGHT, g_bufdc, 0, 0, SRCCOPY);

	//显示对战消息
	SetTextColor(g_mdc, RGB(255, 255, 255));
	for (int i = 0; i < g_iTxtNum; i++)
		TextOut(g_mdc, 20, 410 + i * 18, text[i], wcslen(text[i]));

	//贴上怪物信息
	SelectObject(g_bufdc, g_hBossInfo);
	BitBlt(g_mdc, 0, 0, 300, 100, g_bufdc, 300, 0, SRCAND);
	BitBlt(g_mdc, 0, 0, 300, 100, g_bufdc, 0, 0, SRCPAINT);

	//贴上玩家信息
	SelectObject(g_bufdc, g_hHeroInfo);
	BitBlt(g_mdc, g_rect.right - 300, 0, 300, 100, g_bufdc, 300, 0, SRCAND);
	BitBlt(g_mdc, g_rect.right - 300, 0, 300, 100, g_bufdc, 0, 0, SRCPAINT);

	//贴上怪物图
	if (SPJ.NowHp > 0)
	{
		SelectObject(g_bufdc, g_hMonsterBitmap);
		TransparentBlt(g_mdc, 0, 50, 360, 360, g_bufdc, 0, 0, 360, 360, RGB(255, 255, 0));

		swprintf_s(str, L"%d / %d", SPJ.NowHp, SPJ.MaxHp);
		SetTextColor(g_mdc, RGB(255, 10, 10));
		TextOut(g_mdc, 156, 41, str, wcslen(str));
	}

	//贴上玩家图
	if (Hero.NowHp > 0)
	{
		SelectObject(g_bufdc, g_hHeroBitmap);
		TransparentBlt(g_mdc, 400, 50, 360, 360, g_bufdc, 0, 0, 360, 360, RGB(0, 0, 0));

		swprintf_s(str, L"%d / %d", Hero.NowHp, Hero.MaxHp);
		SetTextColor(g_mdc, RGB(255, 10, 10));
		TextOut(g_mdc, WINDOW_WIDTH - 230, 26, str, wcslen(str));

		swprintf_s(str, L"%d / %d", Hero.NowMp, Hero.MaxMp);
		SetTextColor(g_mdc, RGB(10, 10, 255));
		TextOut(g_mdc, WINDOW_WIDTH - 230, 55, str, wcslen(str));
	}

	//非攻击状态
	if (GameState == GameStatePlaying)
	{
		//贴上技能图标
		SelectObject(g_bufdc, g_hSkillButtons);
		BitBlt(g_mdc, g_rect.right - 305, g_rect.bottom - 205, 300, 200, g_bufdc, 300, 0, SRCAND);
		BitBlt(g_mdc, g_rect.right - 305, g_rect.bottom - 205, 300, 200, g_bufdc, 0, 0, SRCPAINT);
	}
	else if (GameState == GameStateAttack)
	{
		g_FrameNum++;

		//玩家攻击逻辑处理
		if (g_FrameNum == 5)
		{
			HeroLogic();
		}

		//怪物攻击逻辑处理
		if (g_FrameNum == 35)
		{
			Die_Check();
			if (GameState == GameStateOver) goto outer;
			BossLogic();
		}

		//回合结束
		if (g_FrameNum == 60 && GameState != GameStateOver)
		{
			g_FrameNum = 0;
			Die_Check();
			if (GameState == GameStateOver) goto outer;
			GameState = GameStatePlaying;

			int MpRecover = 2 * (rand() % Hero.Intelligence) + 6;
			Hero.NowMp += MpRecover;
			if (Hero.NowMp >= Hero.MaxMp)
			{
				Hero.NowMp = Hero.MaxMp;
			}
			swprintf_s(str, L"回合结束，恢复【%d】点魔法值", MpRecover);
			Message_Insert(str);
		}

		//攻击效果绘制
		HeroPaint(hwnd);
		BossPaint(hwnd);
	}
outer:
	BitBlt(g_hdc, 0, 0, WINDOW_WIDTH, WINDOW_HEIGHT, g_mdc, 0, 0, SRCCOPY);
	g_tPre = GetTickCount();
}

//死亡检查
void Die_Check()
{
	wchar_t str[100];

	if (Hero.NowHp <= 0)
	{
		GameState = GameStateOver;
		PlaySound(L"GameMedia\\gameover.wav", NULL, SND_FILENAME | SND_ASYNC);
		swprintf_s(str, L"胜败乃兵家常事，大侠请重新来过......");
		Message_Insert(str);
	}
	else if (SPJ.NowHp <= 0)
	{
		GameState = GameStateOver;
		PlaySound(L"GameMedia\\victory.wav", NULL, SND_FILENAME | SND_ASYNC);
		swprintf_s(str, L"已战胜恶犬，好样的，勇士！");
		Message_Insert(str);
	}
}

//玩家攻击逻辑处理
VOID HeroLogic()
{
	int damage = 0;
	wchar_t str[100];

	switch (HeroActionType)
	{
	case ACTION_TYPE_NORMAL:
		if (rand() % int(50 / Hero.Intelligence) == 1)
		{
			HeroActionType = ACTION_TYPE_CRITICAL;
			damage = (int)(4.5f*(3 * (rand() % Hero.Agility) + Hero.Level*Hero.Strength + 20));
			SPJ.NowHp -= (int)damage;
			swprintf_s(str, L"玩家发动暴击,对怪物照成了【%d】点伤害", damage);
		}
		else
		{
			damage = 3 * (rand() % Hero.Agility) + Hero.Level*Hero.Strength + 20;
			SPJ.NowHp -= (int)damage;
			swprintf_s(str, L"玩家使用了普通攻击“无敌斩”，对怪物照成了【%d】点伤害", damage);
		}
		Message_Insert(str);
		break;

	case ACTION_TYPE_MAGIC:
		damage = 5 * (2 * (rand() % Hero.Agility) + Hero.Level*Hero.Intelligence);
		SPJ.NowHp -= (int)damage;
		Hero.NowMp -= 30;
		swprintf_s(str, L"玩家释放烈火剑法,对怪物照成了【%d】点伤害", damage);
		Message_Insert(str);
		break;

	case ACTION_TYPE_RECOVER:
		Hero.NowMp -= 40;
		int HpRecover = 5 * (5 * (rand() % Hero.Intelligence) + 40);
		Hero.NowHp += HpRecover;
		if (Hero.NowHp >= Hero.MaxHp)
		{
			Hero.NowHp = Hero.MaxHp;
		}
		swprintf_s(str, L"玩家使用了气疗术，恢复了【%d】点生命值", HpRecover);
		Message_Insert(str);
		break;
	}
}

//玩家攻击效果绘制
VOID HeroPaint(HWND hwnd)
{
	switch (HeroActionType)
	{
	case ACTION_TYPE_NORMAL:
		if (g_FrameNum >= 5 && g_FrameNum < 15) {
			SelectObject(g_bufdc, g_hHeroSkill1);
			TransparentBlt(g_mdc, 50, 170, 364, 140, g_bufdc, 0, 0, 364, 140, RGB(0, 0, 0));
		}
		break;

	case ACTION_TYPE_CRITICAL:
		if (g_FrameNum >= 5 && g_FrameNum < 15) {
			SelectObject(g_bufdc, g_hHeroSkill3);
			TransparentBlt(g_mdc, 20, 60, 574, 306, g_bufdc, 0, 0, 574, 306, RGB(0, 0, 0));
		}
		break;

	case ACTION_TYPE_MAGIC:
		if (g_FrameNum >= 5 && g_FrameNum < 15) {
			SelectObject(g_bufdc, g_hHeroSkill2);
			TransparentBlt(g_mdc, 10, 80, 300, 300, g_bufdc, ((g_FrameNum - 5) % 5) * 192, (g_FrameNum / 5 - 1) * 192, 192, 192, RGB(0, 0, 0));
		}
		break;

	case ACTION_TYPE_RECOVER:
		if (g_FrameNum >= 5 && g_FrameNum < 30) {
			SelectObject(g_bufdc, g_hRecoverSkill);
			TransparentBlt(g_mdc, 485, 120, 192, 192, g_bufdc, ((g_FrameNum - 5) % 5) * 192, (g_FrameNum / 5 - 1) * 192, 192, 192, RGB(0, 0, 0));
		}
		break;
	}
}

//怪物逻辑处理
VOID BossLogic()
{
	int damage = 0, recover = 0;
	wchar_t str[100];

	if (SPJ.NowHp > (SPJ.MaxHp / 2))
	{
		//血多时
		switch (rand() % 3)
		{
		case 0:
			spjActionType = ACTION_TYPE_NORMAL;
			break;
		case 1:
			spjActionType = ACTION_TYPE_CRITICAL;
			break;
		case 2:
			spjActionType = ACTION_TYPE_MAGIC;
			break;
		}
	}
	else
	{
		//血少时
		switch (rand() % 3)
		{
		case 0:
			spjActionType = ACTION_TYPE_MAGIC;
			break;
		case 1:
			spjActionType = ACTION_TYPE_CRITICAL;
			break;
		case 2:
			spjActionType = ACTION_TYPE_RECOVER;
			break;
		}
	}

	//显示消息并计算血量
	switch (spjActionType)
	{
	case ACTION_TYPE_NORMAL:
		damage = rand() % SPJ.Agility + SPJ.Level*SPJ.Strength;
		Hero.NowHp -= (int)damage;
		swprintf_s(str, L"沈鹏杰使用狗刨，对玩家照成【%d】点伤害", damage);
		Message_Insert(str);
		break;

	case ACTION_TYPE_MAGIC:
		damage = 2 * (2 * (rand() % SPJ.Agility) + SPJ.Strength*SPJ.Intelligence);
		Hero.NowHp -= damage;
		recover = (int)((float)damage*0.2f);
		SPJ.NowHp += recover;
		swprintf_s(str, L"鹏杰释放嗜血咒，对玩家照成【%d】点伤害,自身恢复【%d】点生命值", damage, recover);
		Message_Insert(str);
		break;

	case ACTION_TYPE_CRITICAL:
		damage = 2 * (rand() % SPJ.Agility + SPJ.Level*SPJ.Strength);
		Hero.NowHp -= (int)damage;
		swprintf_s(str, L"鹏杰致命一击，对玩家照成【%d】点伤害.", damage);
		Message_Insert(str);
		break;

	case ACTION_TYPE_RECOVER:

		recover = 2 * SPJ.Intelligence*SPJ.Intelligence;
		SPJ.NowHp += recover;
		swprintf_s(str, L"鹏杰使用梅肯斯姆...恢复了【%d】点生命值", recover);
		Message_Insert(str);
		break;
	}
}

//怪物攻击效果绘制
VOID BossPaint(HWND hwnd)
{
	switch (spjActionType)
	{
	case ACTION_TYPE_NORMAL:
		if (g_FrameNum >= 35 && g_FrameNum < 51) {
			SelectObject(g_bufdc, g_hBossSkill1);
			TransparentBlt(g_mdc, 490, 135, 220, 220, g_bufdc, ((g_FrameNum - 35) % 5) * 192, (g_FrameNum / 5 - 7) * 192, 192, 192, RGB(0, 0, 0));
		}
		break;

	case ACTION_TYPE_MAGIC:
		if (g_FrameNum >= 35 && g_FrameNum < 60) {
			SelectObject(g_bufdc, g_hBossSkill2);
			TransparentBlt(g_mdc, 467, 130, 250, 250, g_bufdc, ((g_FrameNum - 35) % 5) * 192, (g_FrameNum / 5 - 7) * 192, 192, 192, RGB(0, 0, 0));
		}
		break;

	case ACTION_TYPE_CRITICAL:
		if (g_FrameNum >= 35 && g_FrameNum < 46) {
			SelectObject(g_bufdc, g_hBossSkill3);
			TransparentBlt(g_mdc, 485, 130, 220, 220, g_bufdc, ((g_FrameNum - 35) % 5) * 192, (g_FrameNum / 5 - 7) * 192, 192, 192, RGB(0, 0, 0));
		}
		break;

	case ACTION_TYPE_RECOVER:
		if (g_FrameNum >= 35 && g_FrameNum < 60) {
			SelectObject(g_bufdc, g_hRecoverSkill2);
			TransparentBlt(g_mdc, 30, 80, 250, 300, g_bufdc, ((g_FrameNum - 35) % 5) * 192, (g_FrameNum / 5 - 7) * 192, 192, 192, RGB(0, 0, 0));
		}
		break;
	}
}

//插入对战消息
void Message_Insert(wchar_t* str)
{
	if (g_iTxtNum < 8)
	{
		swprintf_s(text[g_iTxtNum], str);
		g_iTxtNum++;
	}
	else
	{
		for (int i = 0; i < g_iTxtNum; i++)
			swprintf_s(text[i], text[i + 1]);
		swprintf_s(text[7], str);
	}
}

//资源清理
BOOL Game_ShutDown(HWND hwnd)
{
	DeleteObject(g_hBackGround);
	DeleteObject(g_hTitle);
	DeleteObject(g_hSetting);
	DeleteObject(g_hMonsterBitmap);
	DeleteObject(g_hHeroBitmap);
	DeleteObject(g_hHeroSkill1);
	DeleteObject(g_hHeroSkill2);
	DeleteObject(g_hHeroSkill3);
	//DeleteObject(g_hSkillButton1);
	//DeleteObject(g_hSkillButton2);
	//DeleteObject(g_hSkillButton3);
	DeleteObject(g_hSkillButtons);
	DeleteObject(g_hBossSkill1);
	DeleteObject(g_hBossSkill2);
	DeleteObject(g_hBossSkill3);
	DeleteObject(g_hRecoverSkill);
	DeleteObject(g_hBossInfo);
	DeleteObject(g_hHeroInfo);
	DeleteObject(g_hVictory);
	DeleteObject(g_hGameOver);
	DeleteDC(g_bufdc);
	DeleteDC(g_mdc);
	ReleaseDC(hwnd, g_hdc);
	return TRUE;
}
