#include <windows.h>
#include <shlwapi.h>
#include <d3d9.h>

#define SWRS_USES_HASH
#include "swrs.h"

#ifndef _DEBUG
extern "C"
int _fltused = 1;
#endif

// Vertex
struct SWRVERTEX
{
	float x, y, z;
	float rhw;
	D3DCOLOR color;
	float u, v;
};

#define FVF_SWRVERTEX (D3DFVF_XYZRHW | D3DFVF_DIFFUSE | D3DFVF_TEX1)


// POINT��float��
struct POINTF {
	float x, y;
};

// �v���t�@�C�������j�b�g
struct DNameUnit {
	SWRFONTDESC fontDesc;
	POINTF position;
	POINTF offset;
	bool backVisible;
};

// �����p�����o
struct CBattleManager_MyMember {
	bool m_enabled;
	int m_p1TexID;
	int m_p2TexID;
	int m_p1BkTexID;
	int m_p2BkTexID;
};

// �o�g���}�l�[�W��
#define CBattleManager_Create(p) \
	Ccall(p,s_origCBattleManager_OnCreate,void,())()
#define CBattleManager_Render(p) \
	Ccall(p,s_origCBattleManager_OnRender,bool,())()
#define CBattleManager_Destruct(p, dyn) \
	Ccall(p,s_origCBattleManager_OnDestruct,void*,(int))(dyn)

// �I���W�i���� CBattleManager ���
static DWORD s_origCBattleManager_OnCreate;
static DWORD s_origCBattleManager_OnDestruct;
static DWORD s_origCBattleManager_OnRender;
static DWORD s_origCBattleManager_Size;

// 1P/2P �v���t�@�C�������j�b�g�C���X�^���X
static DNameUnit s_p1Unit;
static DNameUnit s_p2Unit;

// �\���̉�
static bool s_enabledWatch;
static bool s_enabledBattle;

// ��`�v���~�e�B�u��`��
void DrawSprite(int texid, float x, float y, float cx, float cy)
{
	const SWRVERTEX vertices[] = {
		{ x,      y,      0.0f, 1.0f, 0xffffffff, 0.0f, 0.0f },
		{ x + cx, y,      0.0f, 1.0f, 0xffffffff, 1.0f, 0.0f },
		{ x + cx, y + cy, 0.0f, 1.0f, 0xffffffff, 1.0f, 1.0f },
		{ x,      y + cy, 0.0f, 1.0f, 0xffffffff, 0.0f, 1.0f },
	};

	CTextureManager_SetTexture(g_textureMgr, texid, 0);
	g_pd3dDev->DrawPrimitiveUP(D3DPT_TRIANGLEFAN, 2, vertices, sizeof(SWRVERTEX));
}

// �v���t�@�C�������j�b�g��`��
void DrawUnit(const DNameUnit &unit, int bktexid, int texid)
{
	// �w�i
	if(unit.backVisible) {
		DrawSprite(bktexid, 
			unit.position.x, unit.position.y,
			216.0f, 32.0f);
	}
	// �v���t�@�C����
	DrawSprite(texid,
		unit.position.x + unit.offset.x,
		unit.position.y + unit.offset.y,
		1024.0f,
		unit.fontDesc.Height + 18.0f);
}

// CBattleWatch �R���X�g���N�^�n���h��
void * __fastcall CBattleManager_OnCreate(void *This)
{
	CBattleManager_MyMember &my = 
		*(CBattleManager_MyMember*)((char *)This + s_origCBattleManager_Size);
	// super
	CBattleManager_Create(This);
	
	// �ϐ킩�M�у��[�h�ŗL���ɂ���
	my.m_enabled = 
		(s_enabledWatch && g_sceneIdNew == 12) |
		(s_enabledBattle && (g_sceneIdNew == 8 || g_sceneIdNew == 9));

	if(my.m_enabled) {
		char font[0x1A4]; // �����Ƃ� 0x1A4

		// 1P���̃v���t�@�C�����e�N�X�`��
		SWRFont_Create(font);
		SWRFont_SetIndirect(font, &s_p1Unit.fontDesc);
		CTextureManager_CreateTextTexture(g_textureMgr,
			&my.m_p1TexID, g_pprofP1, font, 0x400, s_p1Unit.fontDesc.Height + 18, NULL, NULL);
		SWRFont_Destruct(font);
		// 2P���̃v���t�@�C�����e�N�X�`��
		SWRFont_Create(font);
		SWRFont_SetIndirect(font, &s_p2Unit.fontDesc);
		CTextureManager_CreateTextTexture(g_textureMgr,
			&my.m_p2TexID, g_pprofP2, font, 0x400, s_p2Unit.fontDesc.Height + 18, NULL, NULL);
		SWRFont_Destruct(font);

		// �w�i�e�N�X�`��
		CTextureManager_LoadTexture(g_textureMgr, &my.m_p1BkTexID,
			"data/scene/select/character/chr_profile1.bmp", NULL, NULL);
		CTextureManager_LoadTexture(g_textureMgr, &my.m_p2BkTexID,
			"data/scene/select/character/chr_profile2.bmp", NULL, NULL);
	}

	return This;
}

// CBattleManager �����_�����O�n���h��
// ��ʑw��Begin/End�����̂ł����ł͕s�K�v
int __fastcall CBattleManager_OnRender(void *This)
{
	int &m_mode = *(int*)((char *)This + 0x88);
	CBattleManager_MyMember &my = 
		*(CBattleManager_MyMember*)((char *)This + s_origCBattleManager_Size);
	// super
	int ret = CBattleManager_Render(This);

	if(my.m_enabled && m_mode < 6) {
		// CSprite�͍��R�X�g�Ȃ̂ŁA�ߖ�
		DrawUnit(s_p1Unit, my.m_p1BkTexID, my.m_p1TexID);
		DrawUnit(s_p2Unit, my.m_p2BkTexID, my.m_p2TexID);
	}
	return ret;
}

// CBattleWatch �f�X�g���N�^�n���h��
void * __fastcall CBattleManager_OnDestruct(void *This, int, int dyn)
{
	CBattleManager_MyMember &my = 
		*(CBattleManager_MyMember*)((char *)This + s_origCBattleManager_Size);
	if(my.m_enabled) {
		// �e�N�X�`����j��
		CTextureManager_Remove(g_textureMgr, my.m_p1TexID);
		CTextureManager_Remove(g_textureMgr, my.m_p2TexID);
		CTextureManager_Remove(g_textureMgr, my.m_p1BkTexID);
		CTextureManager_Remove(g_textureMgr, my.m_p2BkTexID);
	}
	return CBattleManager_Destruct(This, dyn);
}

// ���j�b�g�����v���t�@�C�����烍�[�h
void LoadUnitSettings(DNameUnit &unit, LPCSTR profilePath, LPCSTR sectionName, int defaultX, int defaultY)
{
	// �t�H���g��
	::GetPrivateProfileString(sectionName, "Font.Face", 
		g_defaultFontName, unit.fontDesc.FaceName, _countof(unit.fontDesc.FaceName), profilePath);
	// �F
	unit.fontDesc.R1 = ::GetPrivateProfileInt(sectionName, "Font.Color1.R", 255, profilePath);
	unit.fontDesc.R2 = ::GetPrivateProfileInt(sectionName, "Font.Color2.R", 255, profilePath);
	unit.fontDesc.G1 = ::GetPrivateProfileInt(sectionName, "Font.Color1.G", 255, profilePath);
	unit.fontDesc.G2 = ::GetPrivateProfileInt(sectionName, "Font.Color2.G", 255, profilePath);
	unit.fontDesc.B1 = ::GetPrivateProfileInt(sectionName, "Font.Color1.B", 255, profilePath);
	unit.fontDesc.B2 = ::GetPrivateProfileInt(sectionName, "Font.Color2.B", 255, profilePath);
	// �t�H���g����
	unit.fontDesc.Height = ::GetPrivateProfileInt(sectionName, "Font.Height", 14, profilePath);
	unit.fontDesc.Weight = ::GetPrivateProfileInt(sectionName, "Font.Bold", 0, profilePath) ? FW_BOLD : FW_NORMAL;
	unit.fontDesc.Italic = ::GetPrivateProfileInt(sectionName, "Font.Italic", 0, profilePath) ? 1 : 0;
	// �����C���A���̑�
	unit.fontDesc.Shadow = ::GetPrivateProfileInt(sectionName, "Font.Shadow", 1, profilePath) ? 1 : 0;
	unit.fontDesc.BufferSize = 1000000; // unknown buffer size
	unit.fontDesc.CharSpaceX = 0;
	unit.fontDesc.CharSpaceY = 2;
	// �\���ʒu
	unit.position.x = (float)(int)::GetPrivateProfileInt(sectionName, "Position.X", defaultX, profilePath);
	unit.position.y = (float)(int)::GetPrivateProfileInt(sectionName, "Position.Y", defaultY, profilePath);
	// ������\���I�t�Z�b�g
	unit.offset.x = (float)(int)::GetPrivateProfileInt(sectionName, "Offset.X", 90, profilePath);
	unit.offset.y = (float)(int)::GetPrivateProfileInt(sectionName, "Offset.Y", 25 - unit.fontDesc.Height, profilePath);
	// �w�i����
	unit.backVisible = ::GetPrivateProfileInt(sectionName, "Back.Visible", 1, profilePath) != 0;
}

// �����v���t�@�C�����烍�[�h
void LoadSettings(LPCSTR profilePath)
{
	LoadUnitSettings(s_p1Unit, profilePath, "1P", 0, 0);
	LoadUnitSettings(s_p2Unit, profilePath, "2P", 424, 0);

	s_enabledWatch  = ::GetPrivateProfileInt("General", "Enabled.Watch", 1, profilePath) != 0;
	s_enabledBattle = ::GetPrivateProfileInt("General", "Enabled.Battle", 1, profilePath) != 0;
}

extern "C"
__declspec(dllexport) bool CheckVersion(const BYTE hash[16])
{
	return ::memcmp(TARGET_HASH, hash, sizeof TARGET_HASH) == 0;
}

extern "C"
__declspec(dllexport) bool Initialize(HMODULE hMyModule, HMODULE hParentModule)
{
	char profilePath[1024 + MAX_PATH];
	// �v���t�@�C�������[�h
	GetModuleFileName(hMyModule, profilePath, 1024);
	PathRemoveFileSpec(profilePath);
	PathAppend(profilePath, "NetProfileView.ini");
	LoadSettings(profilePath);

	// ����������
	DWORD old;
	::VirtualProtect((PVOID)text_Offset, text_Size, PAGE_EXECUTE_WRITECOPY, &old);
	// CBattleManager �̃T�C�Y���g��
	s_origCBattleManager_Size = TamperDwordAdd(
		(DWORD)&CBattleManager_Size, sizeof(CBattleManager_MyMember));
	// CBattleManager ctor �� call ����������
	s_origCBattleManager_OnCreate = TamperNearJmpOpr(
		CBattleManager_Creater, union_cast<DWORD>(CBattleManager_OnCreate));
	// ��������������
	TamperNearJmp(ProfileNamePrintCode1, ProfileNamePrintCode1End);
	TamperNearJmp(ProfileNamePrintCode2, ProfileNamePrintCode2End);
	::VirtualProtect((PVOID)text_Offset, text_Size, old, &old);

	::VirtualProtect((PVOID)rdata_Offset, rdata_Size, PAGE_EXECUTE_WRITECOPY, &old);
	// CBattleManager vtbl ����������
	s_origCBattleManager_OnDestruct = TamperDword(
		vtbl_CBattleManager + 0x00, union_cast<DWORD>(CBattleManager_OnDestruct));
	s_origCBattleManager_OnRender = TamperDword(
		vtbl_CBattleManager + 0x38, union_cast<DWORD>(CBattleManager_OnRender));
	::VirtualProtect((PVOID)rdata_Offset, rdata_Size, old, &old);

	// ����������flush
	::FlushInstructionCache(GetCurrentProcess(), NULL, 0);
	return true;
}

// �G���g���|�C���g
extern "C"
int APIENTRY DllMain(HMODULE hModule, DWORD fdwReason, LPVOID lpReserved)
{
	return TRUE;
}
