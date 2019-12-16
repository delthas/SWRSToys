#include <windows.h>
#include <shlwapi.h>

#define SWRS_USES_HASH
#include "swrs.h"

#define CLogo_Process(p)   \
	Ccall(p, s_origCLogo_OnProcess, int, ())()
#define CBattle_Process(p) \
	Ccall(p, s_origCBattle_OnProcess, int, ())()

static DWORD s_origCLogo_OnProcess;
static DWORD s_origCBattle_OnProcess;

static bool s_swrapt;
static bool s_autoShutdown;

int __fastcall CLogo_OnProcess(void *This)
{
	int ret = CLogo_Process(This);
	if(ret == 2 && __argc == 2) {
		if(CInputManager_ReadReplay(g_inputMgr, __argv[1])) {
			s_swrapt = true;
			// ���͂��������悤�Ɍ���������BEND
			*(BYTE*)((DWORD)g_inputMgrs + 0x74) = 0xFF;
			// ���v���[�h�Ƀ`�F���W
			SetBattleMode(3, 2);
			ret = 6;
		}
	}
	return ret;
}

int __fastcall CBattle_OnProcess(void *This)
{
	int ret = CBattle_Process(This);
	if(s_swrapt && ret != 5) {
		s_swrapt = false;
		if(s_autoShutdown) {
			// ���Ƃ�
			ret = -1;
		}
	}
	return ret;
}

// �ݒ胍�[�h
void LoadSettings(LPCSTR profilePath)
{
	// �����V���b�g�_�E��
	s_autoShutdown = GetPrivateProfileInt("ReplayDnD", "AutoShutdown", 1, profilePath) != 0;
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

	GetModuleFileName(hMyModule, profilePath, 1024);
	PathRemoveFileSpec(profilePath);
	PathAppend(profilePath, "ReplayDnD.ini");
	LoadSettings(profilePath);

	DWORD old;
	::VirtualProtect((PVOID)rdata_Offset, rdata_Size, PAGE_EXECUTE_WRITECOPY, &old);
	s_origCLogo_OnProcess   = TamperDword(vtbl_CLogo   + 4, (DWORD)CLogo_OnProcess);
	s_origCBattle_OnProcess = TamperDword(vtbl_CBattle + 4, (DWORD)CBattle_OnProcess);
	::VirtualProtect((PVOID)rdata_Offset, rdata_Size, old, &old);

	::FlushInstructionCache(GetCurrentProcess(), NULL, 0);

	return true;
}

extern "C"
int APIENTRY DllMain(HMODULE hModule, DWORD fdwReason, LPVOID lpReserved)
{
	return TRUE;
}

