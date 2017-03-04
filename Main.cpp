//ͷ�ļ�
#include<windows.h>
#include<tchar.h>
#include<time.h>

//���ļ�
#pragma comment(lib,"winmm.lib")
#pragma comment(lib,"Msimg32.lib")

//�궨��
#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 600
#define WINDOW_TITILE L"���߶���Ȯ"

//�ṹ��
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

//ö����
//��Ϸ״̬
enum GameStateTypes
{
	GameStateTitle = 0,//����
	GameStateBackground = 1,//��������
	GameStatePlaying = 2,//������
	GameStateAttack = 3,//����
	GameStateOver = 4//��Ϸ����
};

//����״̬
enum ActionTypes
{
	ACTION_TYPE_NORMAL = 0,//��ͨ����
	ACTION_TYPE_CRITICAL = 1,//����һ��
	ACTION_TYPE_MAGIC = 2,//ħ������
	ACTION_TYPE_MISS = 3,//�������
	ACTION_TYPE_RECOVER = 4,//��Ѫ
};


//ȫ�ֱ�������
HDC	g_hdc = NULL, g_mdc = NULL, g_bufdc = NULL;
DWORD g_tPre = 0, g_tNow = 0;
RECT g_rect;
int g_FrameNum, g_iTxtNum;
wchar_t text[8][100];
CHARACTER SPJ, Hero;//��Ȯ������
ActionTypes	spjActionType, HeroActionType;
GameStateTypes GameState;

//��Դ���
HBITMAP g_hTitle;//��Ϸ����
HBITMAP g_hSetting;//��Ϸ����
HBITMAP g_hBackGround;//ս������
HBITMAP	g_hGameOver, g_hVictory;//��Ϸ��������Ϸʤ��
HBITMAP g_hBossInfo, g_hHeroInfo;
HBITMAP	g_hMonsterBitmap, g_hHeroBitmap, g_hRecoverSkill, g_hRecoverSkill2;
//HBITMAP	g_hSkillButton1, g_hSkillButton2, g_hSkillButton3;  
HBITMAP	g_hSkillButtons;
HBITMAP	g_hHeroSkill1, g_hHeroSkill2, g_hHeroSkill3;
HBITMAP	g_hBossSkill1, g_hBossSkill2, g_hBossSkill3;


//ȫ�ֺ�������
LRESULT CALLBACK WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);//���ڻص�����
BOOL Game_Init(HWND hwnd);//��Ϸ��ʼ��
VOID Game_Main(HWND hwnd);//��Ϸ��ͼ
BOOL Game_ShutDown(HWND hwnd);//��Դ����
VOID HeroLogic();//��ҹ����߼�
VOID BossLogic();//���﹥���߼�
VOID Message_Insert(wchar_t* str);
VOID HeroPaint(HWND hwnd);//��ҹ���Ч������
VOID BossPaint(HWND hwnd);//���﹥��Ч������
VOID Die_Check();


//������
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	//���������
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

	//ע�ᴰ����
	if (!RegisterClassEx(&wndclass))return -1;

	//��������
	HWND hwnd = CreateWindow(L"MainWindowClass", WINDOW_TITILE, WS_OVERLAPPEDWINDOW&(~WS_THICKFRAME)&(~WS_MAXIMIZEBOX), CW_USEDEFAULT, CW_USEDEFAULT, WINDOW_WIDTH, WINDOW_HEIGHT, NULL, NULL, hInstance, NULL);

	//��ʾ����
	ShowWindow(hwnd, nCmdShow);
	UpdateWindow(hwnd);

	//��Դ��ʼ��
	if (!Game_Init(hwnd)) {
		MessageBox(hwnd, L"��Դ��ʼ��ʧ��", L"��Ϣ����", 0);
		return FALSE;
	}
	PlaySound(L"GameMedia\\title.wav", NULL, SND_FILENAME | SND_ASYNC | SND_LOOP);

	//��Ϣѭ��
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

	//ע��������
	UnregisterClass(L"MainWindowClass", wndclass.hInstance);
	return 0;
}




//���ڹ��̺���
LRESULT CALLBACK WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{

	case WM_KEYDOWN:
		//ESC�˳���Ϸ
		if (wParam == VK_ESCAPE) {
			PostQuitMessage(0);
		}

		//�س���ո������Ϸ
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

		//�ͷż��ܣ����̣�
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
					Message_Insert(L"ħ��ֵ���㣬�޷������һ𽣷�");
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
					Message_Insert(L"ħ��ֵ���㣬�޷�ʹ�ÿ���֮��");
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
		//�ͷż��ܣ���꣩
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
					Message_Insert(L"ħ��ֵ���㣬�޷������һ𽣷�");
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
					Message_Insert(L"ħ��ֵ���㣬�޷�ʹ�ÿ���֮��");
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

//��Ϸ��ʼ��
BOOL Game_Init(HWND hwnd)
{
	//���������
	srand((unsigned)time(NULL));

	//��������ϵ
	HBITMAP bitmap;
	g_hdc = GetDC(hwnd);
	g_mdc = CreateCompatibleDC(g_hdc);
	g_bufdc = CreateCompatibleDC(g_hdc);
	bitmap = CreateCompatibleBitmap(g_hdc, WINDOW_WIDTH, WINDOW_HEIGHT);
	SelectObject(g_mdc, bitmap);

	//����ͼƬ
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

	//��ȡ�ͻ��˴�С
	GetClientRect(hwnd, &g_rect);

	//�趨�������
	Hero.NowHp = Hero.MaxHp = 3000;
	Hero.Level = 6;
	Hero.NowMp = Hero.MaxMp = 60;
	Hero.Strength = 10;
	Hero.Agility = 20;
	Hero.Intelligence = 10;

	//�趨BOSS����
	SPJ.NowHp = SPJ.MaxHp = 5000;
	SPJ.Level = 10;
	SPJ.Strength = 10;
	SPJ.Agility = 10;
	SPJ.Intelligence = 10;

	//��ʼ����ʾ��Ϣ��Ŀ
	g_iTxtNum = 0;

	//�趨����
	HFONT hFont;
	hFont = CreateFont(20, 0, 0, 0, 700, 0, 0, 0, GB2312_CHARSET, 0, 0, 0, 0, TEXT("΢���ź�"));
	SelectObject(g_mdc, hFont);
	SetBkMode(g_mdc, TRANSPARENT);

	//������Ϸ״̬
	GameState = GameStateTitle;

	Game_Main(hwnd);
	return TRUE;
}

//��Ϸ��ͼ
VOID Game_Main(HWND hwnd)
{
	wchar_t str[100];

	//��Ϸ����
	if (GameState == GameStateTitle)
	{
		SelectObject(g_mdc, g_hTitle);
		BitBlt(g_hdc, 0, 0, WINDOW_WIDTH, WINDOW_HEIGHT, g_mdc, 0, 0, SRCCOPY);
		return;
	}

	//��Ϸ�����趨
	if (GameState == GameStateBackground) {
		SelectObject(g_mdc, g_hSetting);
		BitBlt(g_hdc, 0, 0, WINDOW_WIDTH, WINDOW_HEIGHT, g_mdc, 0, 0, SRCCOPY);
		return;
	}

	//��Ϸ����
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
	//������
	//���Ʊ���ͼ
	SelectObject(g_bufdc, g_hBackGround);
	BitBlt(g_mdc, 0, 0, WINDOW_WIDTH, WINDOW_HEIGHT, g_bufdc, 0, 0, SRCCOPY);

	//��ʾ��ս��Ϣ
	SetTextColor(g_mdc, RGB(255, 255, 255));
	for (int i = 0; i < g_iTxtNum; i++)
		TextOut(g_mdc, 20, 410 + i * 18, text[i], wcslen(text[i]));

	//���Ϲ�����Ϣ
	SelectObject(g_bufdc, g_hBossInfo);
	BitBlt(g_mdc, 0, 0, 300, 100, g_bufdc, 300, 0, SRCAND);
	BitBlt(g_mdc, 0, 0, 300, 100, g_bufdc, 0, 0, SRCPAINT);

	//���������Ϣ
	SelectObject(g_bufdc, g_hHeroInfo);
	BitBlt(g_mdc, g_rect.right - 300, 0, 300, 100, g_bufdc, 300, 0, SRCAND);
	BitBlt(g_mdc, g_rect.right - 300, 0, 300, 100, g_bufdc, 0, 0, SRCPAINT);

	//���Ϲ���ͼ
	if (SPJ.NowHp > 0)
	{
		SelectObject(g_bufdc, g_hMonsterBitmap);
		TransparentBlt(g_mdc, 0, 50, 360, 360, g_bufdc, 0, 0, 360, 360, RGB(255, 255, 0));

		swprintf_s(str, L"%d / %d", SPJ.NowHp, SPJ.MaxHp);
		SetTextColor(g_mdc, RGB(255, 10, 10));
		TextOut(g_mdc, 156, 41, str, wcslen(str));
	}

	//�������ͼ
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

	//�ǹ���״̬
	if (GameState == GameStatePlaying)
	{
		//���ϼ���ͼ��
		SelectObject(g_bufdc, g_hSkillButtons);
		BitBlt(g_mdc, g_rect.right - 305, g_rect.bottom - 205, 300, 200, g_bufdc, 300, 0, SRCAND);
		BitBlt(g_mdc, g_rect.right - 305, g_rect.bottom - 205, 300, 200, g_bufdc, 0, 0, SRCPAINT);
	}
	else if (GameState == GameStateAttack)
	{
		g_FrameNum++;

		//��ҹ����߼�����
		if (g_FrameNum == 5)
		{
			HeroLogic();
		}

		//���﹥���߼�����
		if (g_FrameNum == 35)
		{
			Die_Check();
			if (GameState == GameStateOver) goto outer;
			BossLogic();
		}

		//�غϽ���
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
			swprintf_s(str, L"�غϽ������ָ���%d����ħ��ֵ", MpRecover);
			Message_Insert(str);
		}

		//����Ч������
		HeroPaint(hwnd);
		BossPaint(hwnd);
	}
outer:
	BitBlt(g_hdc, 0, 0, WINDOW_WIDTH, WINDOW_HEIGHT, g_mdc, 0, 0, SRCCOPY);
	g_tPre = GetTickCount();
}

//�������
void Die_Check()
{
	wchar_t str[100];

	if (Hero.NowHp <= 0)
	{
		GameState = GameStateOver;
		PlaySound(L"GameMedia\\gameover.wav", NULL, SND_FILENAME | SND_ASYNC);
		swprintf_s(str, L"ʤ���˱��ҳ��£���������������......");
		Message_Insert(str);
	}
	else if (SPJ.NowHp <= 0)
	{
		GameState = GameStateOver;
		PlaySound(L"GameMedia\\victory.wav", NULL, SND_FILENAME | SND_ASYNC);
		swprintf_s(str, L"��սʤ��Ȯ�������ģ���ʿ��");
		Message_Insert(str);
	}
}

//��ҹ����߼�����
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
			swprintf_s(str, L"��ҷ�������,�Թ����ճ��ˡ�%d�����˺�", damage);
		}
		else
		{
			damage = 3 * (rand() % Hero.Agility) + Hero.Level*Hero.Strength + 20;
			SPJ.NowHp -= (int)damage;
			swprintf_s(str, L"���ʹ������ͨ�������޵�ն�����Թ����ճ��ˡ�%d�����˺�", damage);
		}
		Message_Insert(str);
		break;

	case ACTION_TYPE_MAGIC:
		damage = 5 * (2 * (rand() % Hero.Agility) + Hero.Level*Hero.Intelligence);
		SPJ.NowHp -= (int)damage;
		Hero.NowMp -= 30;
		swprintf_s(str, L"����ͷ��һ𽣷�,�Թ����ճ��ˡ�%d�����˺�", damage);
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
		swprintf_s(str, L"���ʹ�������������ָ��ˡ�%d��������ֵ", HpRecover);
		Message_Insert(str);
		break;
	}
}

//��ҹ���Ч������
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

//�����߼�����
VOID BossLogic()
{
	int damage = 0, recover = 0;
	wchar_t str[100];

	if (SPJ.NowHp > (SPJ.MaxHp / 2))
	{
		//Ѫ��ʱ
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
		//Ѫ��ʱ
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

	//��ʾ��Ϣ������Ѫ��
	switch (spjActionType)
	{
	case ACTION_TYPE_NORMAL:
		damage = rand() % SPJ.Agility + SPJ.Level*SPJ.Strength;
		Hero.NowHp -= (int)damage;
		swprintf_s(str, L"������ʹ�ù��٣�������ճɡ�%d�����˺�", damage);
		Message_Insert(str);
		break;

	case ACTION_TYPE_MAGIC:
		damage = 2 * (2 * (rand() % SPJ.Agility) + SPJ.Strength*SPJ.Intelligence);
		Hero.NowHp -= damage;
		recover = (int)((float)damage*0.2f);
		SPJ.NowHp += recover;
		swprintf_s(str, L"�����ͷ���Ѫ�䣬������ճɡ�%d�����˺�,����ָ���%d��������ֵ", damage, recover);
		Message_Insert(str);
		break;

	case ACTION_TYPE_CRITICAL:
		damage = 2 * (rand() % SPJ.Agility + SPJ.Level*SPJ.Strength);
		Hero.NowHp -= (int)damage;
		swprintf_s(str, L"��������һ����������ճɡ�%d�����˺�.", damage);
		Message_Insert(str);
		break;

	case ACTION_TYPE_RECOVER:

		recover = 2 * SPJ.Intelligence*SPJ.Intelligence;
		SPJ.NowHp += recover;
		swprintf_s(str, L"����ʹ��÷��˹ķ...�ָ��ˡ�%d��������ֵ", recover);
		Message_Insert(str);
		break;
	}
}

//���﹥��Ч������
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

//�����ս��Ϣ
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

//��Դ����
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
