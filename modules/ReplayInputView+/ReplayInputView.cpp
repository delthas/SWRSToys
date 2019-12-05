﻿#include <windows.h>
#include <shlwapi.h>
#include <d3d9.h>
#include <dinput.h>
#include <cstdio>
#include "fields.h"
#include "List.h"
#define SWRS_USES_HASH
#include "swrs.h"
#define INSTANTIATE
#include "common.h"

struct SWRCHARINPUT
{
  int lr;
  int ud;
  int a;
  int b;
  int c;
  int d;
  int ch;
  int s;
};

struct SWRVERTEX
{
  float x, y, z;
  float rhw;
  D3DCOLOR color;
  float u, v;

  void set(float _x, float _y, float _z) {
    x = _x;
    y = _y;
    z = _z;
  }

  void set_xy(float _x, float _y) {
    x = _x;
    y = _y;
  }
};

struct SWRCMDINFO
{
  bool enabled;
  int prev;
  int now;

  struct {
    bool enabled;
    int id[10];
    int base;
    int len;
  } record;
};

struct MYMEMBER {
  bool m_enabled;
  int m_texID;
  int m_forwardCount;
  int m_forwardStep;
  int m_forwardIndex;
  SWRCMDINFO m_cmdp1;
  SWRCMDINFO m_cmdp2;
  bool m_hitboxes;
  bool m_untech;
  bool m_show_debug;
  bool m_paused;
};

struct Hitbox {
  int x1, y1, x2, y2;

  int width() const { return abs(x2 - x1); }
  int not_abs_width() const { return (x2 - x1); }
  int height() const { return abs(y2 - y1); }
  int not_abs_height() const { return (y2 - y1); }
};

struct CustomVertex {
  float x, y, z;
};

struct CustomQuad {
  SWRVERTEX v[4];

  void translate(float tr_x, float tr_y, float tr_z) {
    for (int i = 0; i < 4; ++i) {
      v[i].x += tr_x;
      v[i].y += tr_y;
      v[i].z += tr_z;
    }

  }

  void scale(float scale_x, float scale_y) {
    for (int i = 0; i < 4; ++i) {
      v[i].x *= scale_x;
      v[i].y *= scale_y;
    }
  }

  void set_rect(float x, float y, float w, float h) {
    v[0].set_xy(x , y);
    v[0].rhw = 1.0f;
    v[0].u = 0.0f;
    v[0].v = 0.0f;

    v[1].set_xy(x + w, y);
    v[1].rhw = 1.0f;
    v[1].u = 1.0f;
    v[1].v = 0.0f;

    v[2].set_xy(x + w, y + h);
    v[2].rhw = 1.0f;
    v[2].u = 1.0f;
    v[2].v = 1.0f;

    v[3].set_xy(x, y + h);
    v[3].rhw = 1.0f;
    v[3].u = 0.0f;
    v[3].v = 1.0f;
  }

  void set_rect_rot(float x, float y, float vec1x, float vec1y, float vec2x, float vec2y) {
    v[0].set_xy(x , y);
    v[0].rhw = 1.0f;
    v[0].u = 0.0f;
    v[0].v = 0.0f;

    v[1].set_xy(x + vec1x, y + vec1y);
    v[1].rhw = 1.0f;
    v[1].u = 1.0f;
    v[1].v = 0.0f;

    v[2].set_xy(x + vec1x + vec2x, y + vec1y + vec2y);
    v[2].rhw = 1.0f;
    v[2].u = 1.0f;
    v[2].v = 1.0f;

    v[3].set_xy(x + vec2x, y + vec2y);
    v[3].rhw = 1.0f;
    v[3].u = 0.0f;
    v[3].v = 1.0f;
  }

  void debug_set_rect_rot(float x, float y, float x2, float y2, float vec1x, float vec1y, float vec2x, float vec2y) {
    v[0].set_xy(x , y);
    v[0].rhw = 1.0f;
    v[0].u = 0.0f;
    v[0].v = 0.0f;

    v[1].set_xy(x + vec1x, y + vec1y);
    v[1].rhw = 1.0f;
    v[1].u = 1.0f;
    v[1].v = 0.0f;

    v[2].set_xy(x2, y2);
    v[2].rhw = 1.0f;
    v[2].u = 1.0f;
    v[2].v = 1.0f;

    v[3].set_xy(x + vec2x, y + vec2y);
    v[3].rhw = 1.0f;
    v[3].u = 0.0f;
    v[3].v = 1.0f;
  }

  void set_colors(D3DCOLOR colors[4]) {
    for (int i = 0; i < 4; ++i) {
      v[i].color = colors[i];
    }
  }
};

typedef struct {
  unsigned char b, g, r, a;
} ARGB;

#define FVF_SWRVERTEX (D3DFVF_XYZRHW | D3DFVF_DIFFUSE | D3DFVF_TEX1)

HRESULT (__stdcall *s_D3DXCreateTextureFromResource)(
    LPDIRECT3DDEVICE9 pDevice,
    HMODULE hSrcModule,
    LPCTSTR pSrcResource,
    LPDIRECT3DTEXTURE9 *ppTexture
);

// ƒoƒgƒ‹ƒ}ƒl[ƒWƒƒ
#define CBattleManager_Create(p) \
  Ccall(p, s_origCBattleManager_OnCreate, void*, ())()
#define CBattleManager_Render(p) \
  Ccall(p, s_origCBattleManager_OnRender, void, ())()
#define CBattleManager_Process(p) \
  Ccall(p, s_origCBattleManager_OnProcess, int, ())()
#define CBattleManager_Destruct(p, dyn) \
  Ccall(p, s_origCBattleManager_OnDestruct, void*, (int))(dyn)
// ƒoƒgƒ‹ƒV[ƒ“
#define CBattle_Process(p) \
  Ccall(p, s_origCBattle_OnProcess, int, ())()
#define CBattle_Destruct(p, dyn) \
  Ccall(p, s_origCBattle_OnDestruct, void*, (int))(dyn)

static DWORD s_origCBattleManager_OnCreate;
static DWORD s_origCBattleManager_OnDestruct;
static DWORD s_origCBattleManager_OnRender;
static DWORD s_origCBattleManager_OnProcess;
static DWORD s_origCBattleManager_Size;

static char s_profilePath[1024 + MAX_PATH];
static char s_msg[256];
static HMODULE s_hDllModule;
static int s_slowdown_method = 0;

#ifndef _DEBUG
extern "C"
int _fltused = 1;
#endif

#define SHOW_DEBUG_MSG() MessageBox(NULL, s_msg, "Debug", 0)
#define SHOW_MSG(text) MessageBox(NULL, (text), "Debug", 0)

typedef double (__thiscall *PTR_getlvl_height)(void *);
PTR_getlvl_height getlvl_height = (PTR_getlvl_height)ADDR_GETLVL_HEIGHT;

int * CTextureManager_LoadTextureFromResource(void *ptextureMgr, int *ret, HMODULE hSrcModule, LPCTSTR pSrcResource)
{
  int id = 0;
  LPDIRECT3DTEXTURE9 *pphandle = CTextureManager_Allocate(ptextureMgr, &id);

  *pphandle = NULL;
  if(SUCCEEDED(s_D3DXCreateTextureFromResource(g_pd3dDev, hSrcModule, pSrcResource, pphandle))) {
    *ret = id;
  } else {
    CTextureManager_Deallocate(ptextureMgr, id);
    *ret = 0;
  }
  return ret;
}

void __fastcall CBattleManager_RenderMyBack(float x, float y, int cx, int cy)
{
  const SWRVERTEX vertices[] = {
    { x,          y,      0.0f, 1.0f, 0xa0808080, 0.0f, 0.0f },
    { x + cx + 0, y,      0.0f, 1.0f, 0xa0808080, 1.0f, 0.0f },
    { x + cx + 5, y + cy, 0.0f, 1.0f, 0xa0202020, 1.0f, 1.0f },
    { x      + 5, y + cy, 0.0f, 1.0f, 0xa0202020, 0.0f, 1.0f },
  };
  CTextureManager_SetTexture(g_textureMgr, NULL, 0);
  g_pd3dDev->DrawPrimitiveUP(D3DPT_TRIANGLEFAN, 2, vertices, sizeof(SWRVERTEX));
}

void __fastcall CBattleManager_RenderQuad(SWRVERTEX quad[4])
{
  CTextureManager_SetTexture(g_textureMgr, NULL, 0);
  g_pd3dDev->DrawPrimitiveUP(D3DPT_TRIANGLEFAN, 2, quad, sizeof(SWRVERTEX));
}

void __fastcall CBattleManager_RenderQuadOutline(SWRVERTEX quad[4]) {
  SWRVERTEX v[5];

  for (int i = 0; i < 4; ++i) {
    v[i] = quad[i];
  }

  v[4] = quad[0];

  CTextureManager_SetTexture(g_textureMgr, NULL, 0);
  g_pd3dDev->DrawPrimitiveUP(D3DPT_LINESTRIP, 4, v, sizeof(SWRVERTEX));

//  v[1] = v[2];
//  g_pd3dDev->DrawPrimitiveUP(D3DPT_LINESTRIP, 1, v, sizeof(SWRVERTEX));
}

void CBattleManager_RenderTile(float x, float y, int u, int v, int a)
{
  int dif = (a << 24) | 0xFFFFFF;
  float fu = u / 256.0f;
  float fv = v / 64.0f;

  // ƒfƒoƒbƒOƒrƒ‹ƒh‚¾‚ÆƒOƒ_ƒOƒ_‚É‚È‚é‚Ì‚Í‰½‚È‚ÌH‰´Ž€‚Ê‚ÌH
  const SWRVERTEX vertices[] = {
    { x,         y,         0.0f, 1.0f, dif, fu,          fv },
    { x + 32.0f, y,         0.0f, 1.0f, dif, fu + 0.125f, fv },
    { x + 32.0f, y + 32.0f, 0.0f, 1.0f, dif, fu + 0.125f, fv + 0.5f },
    { x,         y + 32.0f, 0.0f, 1.0f, dif, fu,          fv + 0.5f },
  };
  g_pd3dDev->DrawPrimitiveUP(D3DPT_TRIANGLEFAN, 2, vertices, sizeof(SWRVERTEX));
}

void CBattleManager_RenderInputPanel(void *This, SWRCMDINFO &cmd, float x, float y)
{
  MYMEMBER &my = *(MYMEMBER *)((char *)g_pbattleMgr + s_origCBattleManager_Size);

  if(cmd.enabled) {
    // Œã‚ë
    CBattleManager_RenderMyBack(x, y, 24 * 6 + 24, 24 * 3 + 12);

    CTextureManager_SetTexture(g_textureMgr, my.m_texID, 0);

    // ƒWƒ‡ƒCƒXƒeƒBƒbƒN
    CBattleManager_RenderTile(x + 9,      y + 6,      128, 0, (cmd.now % 16 == 5 ? 255: 48)); /* LU */
    CBattleManager_RenderTile(x + 9 + 24, y + 6,        0, 0, (cmd.now % 16 == 1 ? 255: 48)); /* NU */
    CBattleManager_RenderTile(x + 9 + 48, y + 6,      160, 0, (cmd.now % 16 == 9 ? 255: 48)); /* RU */
    CBattleManager_RenderTile(x + 9,      y + 6 + 24,  64, 0, (cmd.now % 16 == 4 ? 255: 48)); /* LN */
    CBattleManager_RenderTile(x + 9 + 48, y + 6 + 24,  96, 0, (cmd.now % 16 == 8 ? 255: 48)); /* RN */
    CBattleManager_RenderTile(x + 9,      y + 6 + 48, 224, 0, (cmd.now % 16 == 6 ? 255: 48)); /* LD */
    CBattleManager_RenderTile(x + 9 + 24, y + 6 + 48,  32, 0, (cmd.now % 16 == 2 ? 255: 48)); /* ND */
    CBattleManager_RenderTile(x + 9 + 48, y + 6 + 48, 192, 0, (cmd.now % 16 ==10 ? 255: 48)); /* RD */
    // ƒ{ƒ^ƒ“—Þ
    CBattleManager_RenderTile(x + 9 + 72 + 3,      y + 6 + 12,   0, 32, (cmd.now & 16  ? 255: 48));
    CBattleManager_RenderTile(x + 9 + 72 + 3 + 27, y + 6 + 12,  32, 32, (cmd.now & 32  ? 255: 48));
    CBattleManager_RenderTile(x + 9 + 72 + 3 + 54, y + 6 + 12,  64, 32, (cmd.now & 64  ? 255: 48));
    CBattleManager_RenderTile(x + 9 + 72 + 6,      y + 6 + 36,  96, 32, (cmd.now & 128 ? 255: 48));
    CBattleManager_RenderTile(x + 9 + 72 + 6 + 27, y + 6 + 36, 128, 32, (cmd.now & 256 ? 255: 48));
    CBattleManager_RenderTile(x + 9 + 72 + 6 + 54, y + 6 + 36, 160, 32, (cmd.now & 512 ? 255: 48));
  }
}

void CBattleManager_RenderRecordPanel(void *This, SWRCMDINFO &cmd, float x, float y)
{
  MYMEMBER &my = *(MYMEMBER *)((char *)This + s_origCBattleManager_Size);

  if(cmd.record.enabled) {
    // Œã‚ë
    CBattleManager_RenderMyBack(x, y, 24 * 10 + 6, 24 + 6);

    CTextureManager_SetTexture(g_textureMgr, my.m_texID, 0);
    for(int i = 0; i < cmd.record.len; ++i) {
      int j = (i + cmd.record.base) % _countof(cmd.record.id);
      int id = cmd.record.id[j];
      CBattleManager_RenderTile(x + 3 + i * 24, y + 3, (id % 8) * 32, (id / 8) * 32, 255);
    }
  }
}

void CBattleManager_DetermineRecord(SWRCMDINFO &cmd, int mask, int flag, int id)
{
  if((cmd.prev & mask) != flag && (cmd.now & mask) == flag) {
    int index = (cmd.record.base + cmd.record.len) % _countof(cmd.record.id);
    cmd.record.id[index] = id;
    if(cmd.record.len == _countof(cmd.record.id)) {
      cmd.record.base = (cmd.record.base + 1) % _countof(cmd.record.id);
    } else {
      cmd.record.len++;
    }
  }
}

void CBattleManager_RefleshCommandInfo(SWRCMDINFO &cmd, void *Char)
{
  SWRCHARINPUT &input = *(SWRCHARINPUT*)((char*)Char + 0x754);

  cmd.prev = cmd.now;
  cmd.now = 0;
  if(input.ud < 0) cmd.now |= 1;
  if(input.ud > 0) cmd.now |= 2;
  if(input.lr < 0) cmd.now |= 4;
  if(input.lr > 0) cmd.now |= 8;
  if(input.a > 0)
    cmd.now |= 16;
  if(input.b > 0)  cmd.now |= 32;
  if(input.c > 0)  cmd.now |= 64;
  if(input.d > 0)  cmd.now |= 128;
  if(input.ch> 0)  cmd.now |= 256;
  if(input.s > 0)  cmd.now |= 512;

  if(cmd.record.enabled) {
    CBattleManager_DetermineRecord(cmd, 15, 5, 4);
    CBattleManager_DetermineRecord(cmd, 15, 1, 0);
    CBattleManager_DetermineRecord(cmd, 15, 9, 5);
    CBattleManager_DetermineRecord(cmd, 15, 4, 2);
    CBattleManager_DetermineRecord(cmd, 15, 8, 3);
    CBattleManager_DetermineRecord(cmd, 15, 6, 7);
    CBattleManager_DetermineRecord(cmd, 15, 2, 1);
    CBattleManager_DetermineRecord(cmd, 15,10, 6);

    CBattleManager_DetermineRecord(cmd, 16, 16, 8);
    CBattleManager_DetermineRecord(cmd, 32, 32, 9);
    CBattleManager_DetermineRecord(cmd, 64, 64, 10);
    CBattleManager_DetermineRecord(cmd, 128, 128, 11);
    CBattleManager_DetermineRecord(cmd, 256, 256, 12);
    CBattleManager_DetermineRecord(cmd, 512, 512, 13);
  }
}

void * __fastcall CBattleManager_OnCreate(void *This)
{
  MYMEMBER &my = *(MYMEMBER *)((char *)This + s_origCBattleManager_Size);

  CBattleManager_Create(This);

  static char tmp[1024];

	if(g_subMode == 2 || (g_mainMode >= 1 && g_mainMode <= 5) || g_mainMode == 8) {
		// Replay
		my.m_enabled = true;
	} else {
		// それ以外
		my.m_enabled = false;
	}

  if(my.m_enabled) {
    CTextureManager_LoadTextureFromResource(g_textureMgr, &my.m_texID, s_hDllModule, MAKEINTRESOURCE(4));
    text::LoadSettings(s_profilePath, "Debug");
    text::OnCreate(This);

    if(my.m_texID != 0) {
      my.m_forwardCount = 1;
      my.m_forwardStep  = 1;
      my.m_forwardIndex = 0;

      my.m_cmdp1.enabled = ::GetPrivateProfileInt("Input", "p1.Enabled", 1, s_profilePath) != 0;
      my.m_cmdp1.prev = 0;
      my.m_cmdp1.record.base = my.m_cmdp1.record.len = 0;
      my.m_cmdp1.record.enabled = ::GetPrivateProfileInt("Record", "p1.Enabled", 0, s_profilePath) != 0;

      my.m_cmdp2.enabled = ::GetPrivateProfileInt("Input", "p2.Enabled", 1, s_profilePath) != 0;
      my.m_cmdp2.prev = 0;
      my.m_cmdp2.record.base = my.m_cmdp2.record.len = 0;
      my.m_cmdp2.record.enabled = ::GetPrivateProfileInt("Record", "p2.Enabled", 0, s_profilePath) != 0;

      my.m_hitboxes = ::GetPrivateProfileInt("HitboxDisplay", "Enabled", 0, s_profilePath) != 0;
      my.m_untech = ::GetPrivateProfileInt("JuggleMeter", "Enabled", 0, s_profilePath) != 0;

      my.m_show_debug = ::GetPrivateProfileInt("Debug", "Enabled", 0, s_profilePath) != 0;

      s_slowdown_method = ::GetPrivateProfileInt("Framerate", "AdjustmentMethod", 0, s_profilePath);
      if (s_slowdown_method != 0 && s_slowdown_method != 1) {
        s_slowdown_method = 0;
      }
    } else {
      // ‚â‚Á‚Ï‚è‚â‚ß‚½
      my.m_enabled = false;
    }
  }
  return This;
}

void process_frame(void *This, MYMEMBER& my) {
  int ret = CBattleManager_Process(This);
  if(ret > 0 && ret < 4) return;

  void *p1Obj = *(void**)((char *)This + 0xC);
  void *p2Obj = *(void**)((char *)This + 0x10);
  CBattleManager_RefleshCommandInfo(my.m_cmdp1, p1Obj);
  CBattleManager_RefleshCommandInfo(my.m_cmdp2, p2Obj);
}

bool check_key(int key, bool mod1, bool mod2, bool mod3) {
  //return CheckKeyOneshot(key, mod1, mod2, mod3);
  int *keytable = (int*)0x8998D8;
  return keytable[key] == 1;
}

int __fastcall CBattleManager_OnProcess(void *This)
{
  static int fps_steps[] = {
    16, 20, 24, 28, 33, 66, 100, 200
  };

  static int fps_index = 0;
  static int n_steps = sizeof(fps_steps) / sizeof(*fps_steps);

  MYMEMBER &my = *(MYMEMBER *)((char *)This + s_origCBattleManager_Size);
  int ret;

  int *delay = (int*)0x8A0FF8;

  if(my.m_enabled) {
    if(check_key(DIK_F4, 0, 0, 0)) {
      my.m_hitboxes = !my.m_hitboxes;
    } else if (check_key(DIK_F6, 0, 0, 0)) {
      my.m_show_debug = !my.m_show_debug;
    } else if (check_key(DIK_F7, 0, 0, 0)) {
			bool cmdEnabled = my.m_cmdp1.enabled;
			bool recordEnabled = my.m_cmdp1.record.enabled;
			my.m_cmdp1.enabled = !recordEnabled;
			my.m_cmdp1.record.enabled = cmdEnabled;
      my.m_cmdp2.enabled = my.m_cmdp1.enabled;
      my.m_cmdp2.record.enabled = my.m_cmdp1.record.enabled;
    } else if(check_key(DIK_F9, 0, 0, 0)) {
      if (s_slowdown_method) {
        if(my.m_forwardStep > 1) {
          my.m_forwardCount  = 1;
          my.m_forwardStep  -= 1;
        } else {
          my.m_forwardCount += 1;
          my.m_forwardStep   = 1;
        }
        my.m_forwardIndex = 0;
      } else {
        //*delay += 4;
        if (fps_index < (n_steps - 1)) {
          *delay = fps_steps[++fps_index];
        }
      }
    } else if(check_key(DIK_F10, 0, 0, 0)) {
      if (s_slowdown_method) {
        if(my.m_forwardCount > 1) {
          my.m_forwardCount -= 1;
          my.m_forwardStep   = 1;
        } else {
          my.m_forwardCount  = 1;
          my.m_forwardStep  += 1;
        }
        my.m_forwardIndex = 0;
      } else {
        //*delay -= 4;
        if (0 < fps_index) {
          *delay = fps_steps[--fps_index];
        }
      }
    } else if (check_key(DIK_F11, 0, 0, 0)) {
      if (!my.m_paused) {
        my.m_forwardCount  = -1;
        my.m_forwardStep   = 0;
        my.m_forwardIndex  = 0;
        my.m_paused = true;
      } else {
        my.m_forwardCount  = 1;
        my.m_forwardStep   = 1;
        my.m_forwardIndex  = 0;
        my.m_paused = false;
      }
    } else if (check_key(DIK_F12, 0, 0, 0)) {
      if (my.m_paused) {
        process_frame(This, my);
      }
    }

    my.m_forwardIndex += my.m_forwardStep;
    if(my.m_forwardIndex >= my.m_forwardCount) {
      for(int i = my.m_forwardIndex / my.m_forwardCount; i--;) {
        //process_frame(This, my);
        ret = CBattleManager_Process(This);
        if(ret > 0 && ret < 4) break;

        void *p1Obj = *(void**)((char *)This + 0xC);
        void *p2Obj = *(void**)((char *)This + 0x10);
        CBattleManager_RefleshCommandInfo(my.m_cmdp1, p1Obj);
        CBattleManager_RefleshCommandInfo(my.m_cmdp2, p2Obj);
      }
      my.m_forwardIndex = 0;
    }
  } else {
    ret = CBattleManager_Process(This);
  }
  return ret;
}

void draw_frame_boxes_helper(void *char_ptr, void *boxes, int n, D3DCOLOR colors[4], D3DCOLOR outline_colors[4]) {
  float char_x = ACCESS_FLOAT(char_ptr, CF_X_POS);
  float char_y = ACCESS_FLOAT(char_ptr, CF_Y_POS);

  for (int i = 0; i < n; ++i) {
    Hitbox box = {};

    box.x1 = ACCESS_SHORT(boxes, i * 8 + 0x0) + char_x;
    box.y1 = ACCESS_SHORT(boxes, i * 8 + 0x2) + char_y;
    box.x2 = ACCESS_SHORT(boxes, i * 8 + 0x4) + char_x;
    box.y2 = ACCESS_SHORT(boxes, i * 8 + 0x6) + char_y;

    CustomQuad quad = {};
    quad.set_rect(box.x1, box.y1, box.not_abs_width(), box.not_abs_height());
    quad.set_colors(colors);

    void *camera_transform = (void*)0x898600;
    float translate_x = ACCESS_FLOAT(camera_transform, 0x0C);
    float translate_y = ACCESS_FLOAT(camera_transform, 0x10);
    float scale = ACCESS_FLOAT(camera_transform, 0x14);

    quad.translate(translate_x, translate_y, 0.0f);
    quad.scale(scale, scale);

    CBattleManager_RenderQuad(quad.v);

    quad.set_colors(outline_colors);
    CBattleManager_RenderQuadOutline(quad.v);
  }
}

void draw_char_boxes_helper(void* char_ptr, int n, int boxes, D3DCOLOR colors[4], D3DCOLOR outline_colors[4]) {
  int char_addr = (int)char_ptr;

  for (int i = 0; i < n; ++i) {
    Hitbox box = {};
    box.x1 = ACCESS_INT(char_addr + boxes, i * 16 + 0x0);
    box.y1 = ACCESS_INT(char_addr + boxes, i * 16 + 0x4);
    box.x2 = ACCESS_INT(char_addr + boxes, i * 16 + 0x8);
    box.y2 = ACCESS_INT(char_addr + boxes, i * 16 + 0xC);

    CustomQuad quad = {};
    quad.set_rect(box.x1, box.y1, box.not_abs_width(), box.not_abs_height());
    quad.set_colors(colors);

    void *camera_transform = (void*)0x898600;
    float translate_x = ACCESS_FLOAT(camera_transform, 0x0C);
    float translate_y = ACCESS_FLOAT(camera_transform, 0x10);
    float scale = ACCESS_FLOAT(camera_transform, 0x14);

    quad.translate(translate_x, translate_y, 0.0f);
    quad.scale(scale, scale);

    CBattleManager_RenderQuad(quad.v);

    quad.set_colors(outline_colors);
    CBattleManager_RenderQuadOutline(quad.v);
  }
}

void draw_rotated_char_boxes_helper(void* char_ptr, int n, int boxes, int rotation, D3DCOLOR colors[4], D3DCOLOR outline_colors[4]) {
  int char_addr = (int)char_ptr;

  for (int i = 0; i < n; ++i) {
    void* rotptr = ACCESS_PTR(char_addr + rotation, i * 0x4);

    Hitbox box = {};
    box.x1 = ACCESS_INT(char_addr + boxes, i * 16 + 0x0);
    box.y1 = ACCESS_INT(char_addr + boxes, i * 16 + 0x4);
    box.x2 = ACCESS_INT(char_addr + boxes, i * 16 + 0x8);
    box.y2 = ACCESS_INT(char_addr + boxes, i * 16 + 0xC);

    CustomQuad quad = {};
    if (rotptr == NULL)
      quad.set_rect(box.x1, box.y1, box.width(), box.height());
    else
    {
      Hitbox rot = {};

      rot.x1 = *((int*)rotptr);
      rot.y1 = *((int*)rotptr + 1);
      rot.x2 = *((int*)rotptr + 2);
      rot.y2 = *((int*)rotptr + 3);

      quad.set_rect_rot(box.x1, box.y1, rot.x1, rot.y1, rot.x2, rot.y2);
    }

    quad.set_colors(colors);

    void *camera_transform = (void*)0x898600;
    float translate_x = ACCESS_FLOAT(camera_transform, 0x0C);
    float translate_y = ACCESS_FLOAT(camera_transform, 0x10);
    float scale = ACCESS_FLOAT(camera_transform, 0x14);

    quad.translate(translate_x, translate_y, 0.0f);
    quad.scale(scale, scale);

    CBattleManager_RenderQuad(quad.v);

    quad.set_colors(outline_colors);
    CBattleManager_RenderQuadOutline(quad.v);
  }
}

void draw_char_boxes(void* char_addr, bool hurtboxes = true) {
  static D3DCOLOR gray[] = { 0x4f808080, 0x4f808080, 0x4fffffff, 0x4fffffff };
  static D3DCOLOR gray_outline[] = { 0xff808080, 0xff808080, 0xff808080, 0xff808080 };

  static D3DCOLOR red[] = { 0x4fc01010, 0x4fc01010, 0x4fff0000, 0x4fff0000 };
  static D3DCOLOR red_outline[] = { 0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000 };

  static D3DCOLOR green[] = { 0x4f10c010, 0x4f10c010, 0x4f00ff00, 0x4f00ff00 };
  static D3DCOLOR green_outline[] = { 0xff00ff00, 0xff00ff00, 0xff00ff00, 0xff00ff00 };

  // int n_hurt_boxes = ACCESS_CHAR(char_addr, CF_HURT_BOX_COUNT);
  // int n_attack_boxes = ACCESS_CHAR(char_addr, CF_ATTACK_BOX_COUNT);

  // auto frame = ACCESS_PTR(char_addr, CF_CURRENT_FRAME_DATA);
  // int n_hurt_boxes = ACCESS_INT(frame, FF_HURT_BOX_COUNT);
  // int n_attack_boxes = ACCESS_INT(frame, FF_ATTACK_BOX_COUNT);

  // auto collision_box = FIELD_ADDRESS(frame, FF_COLLISION_BOX);
  // auto hurt_boxes = FIELD_ADDRESS(frame, FF_HURT_BOXES);
  // auto attack_boxes = FIELD_ADDRESS(frame, FF_ATTACK_BOXES);

  // if (ACCESS_SHORT(char_addr, CF_CURRENT_SEQ) == 301) {
    // DPRINTF("n_hurt_boxes = %d\nn_attack_boxes = %d", n_hurt_boxes, n_attack_boxes);
    // draw_frame_boxes_helper(char_addr, collision_box, 1, red, red_outline);
    // draw_frame_boxes_helper(char_addr, hurt_boxes, n_hurt_boxes, green, green_outline);
    // draw_frame_boxes_helper(char_addr, attack_boxes, n_attack_boxes, red, red_outline);
  // }

  if (hurtboxes) {
    draw_rotated_char_boxes_helper(char_addr, ACCESS_CHAR(char_addr, CF_HURT_BOX_COUNT), CF_HURT_BOXES, CF_HURT_BOXES_ROT, green, green_outline);
  }

  draw_rotated_char_boxes_helper(char_addr, ACCESS_CHAR(char_addr, CF_ATTACK_BOX_COUNT), CF_ATTACK_BOXES, CF_ATTACK_BOXES_ROT, red, red_outline);
}

list::Node<void*>* get_char_objects(void *character) {
  //STLƒŠƒXƒgƒRƒ“ƒeƒi
  typedef struct{
    int alloc;
    void *head;
    int size;
  } OBJ_LIST;

  //STLƒŠƒXƒgƒRƒ“ƒeƒi‚Ìƒm[ƒh
  typedef struct{
    void *next;
    void *prev;
    void* val;
  } NODE;

  int sysObj;
  int objCharHead;
  int objCharTail;
  int objChar;
  int objCharNum;
  OBJ_LIST objProjList;
  NODE objProjIter;

  //‰Šú‰»
  list::Node<void*>* objects = NULL;

  char *objProjMgr = (char*)ACCESS_PTR(character, CF_OBJ_LIST_MGR);

  //ƒLƒƒƒ‰ƒNƒ^ƒIƒuƒWƒFƒNƒgƒŠƒXƒg‚ð‘–¸‚·‚é
  //(std::list<CharacterObject *>)
  objProjMgr = objProjMgr + 4;
  /*if(!ReadProcessMemory(ph, objProjMgr + ADDR_OBJPROJOFS, &objProjList, sizeof(objProjList))) {
    return;
  }*/
  memcpy_s(&objProjList, sizeof(objProjList), objProjMgr + PMF_OBJ_PROJ_OFS, sizeof(objProjList));
  /*if(!ReadProcessMemory(ph, objProjList.head, &objProjIter, sizeof(objProjIter))) {
    return;
  }*/
  memcpy_s(&objProjIter, sizeof(objProjIter), objProjList.head, sizeof(objProjIter));

  for(int k = 0; objProjList.head != objProjIter.next && objProjIter.next != 0 && objProjList.size > k; k++) {
    /*if(!ReadProcessMemory(ph, objProjIter.next, &objProjIter, sizeof(objProjIter))) {
      return;
    }*/

    memcpy_s(&objProjIter, sizeof(objProjIter), objProjIter.next, sizeof(objProjIter));
    objects = list::cons(objProjIter.val, objects);
  }

  return objects;
}

bool char_on_ground(void* char_addr) {
  float y = ACCESS_FLOAT(char_addr, CF_Y_POS);
  float v_inerc = ACCESS_FLOAT(char_addr, CF_Y_SPEED);

  if (getlvl_height(char_addr) < y) return false;
  if (v_inerc >= 0.0f) return false;
  return true;
}

bool is_airborne_frame(void* char_addr) {
  void *frame = ACCESS_PTR(char_addr, CF_CURRENT_FRAME_DATA);
  int fflags = ACCESS_SHORT(frame, FF_FFLAGS);
  return (fflags & FF_AIRBORNE) != 0;
}

bool is_melee_frame(void* object) {
  void *frame = ACCESS_PTR(object, CF_CURRENT_FRAME_DATA);
  int aflags = ACCESS_SHORT(frame, FF_AFLAGS);
  return (aflags & AF_GRAZABLE) == 0;
}

bool is_active(void *object) {
  return ACCESS_CHAR(object, CF_HIT_COUNT) > 0 && ACCESS_INT(object, PF_IS_ACTIVE) != 0;
}

void draw_char_bullets(void* char_addr, bool draw_all_objects = true) {
  auto objects = get_char_objects((void*)char_addr);

  for (auto e = objects; e; e = e->next) {
    //if (!is_active(e->data)) continue;

    if (is_melee_frame(e->data) || draw_all_objects) {
      draw_char_boxes(e->data, false);
    }
  }

  list::free(objects);
}

void draw_quad(float x, float y, float w, float h, D3DCOLOR color[4]) {
  CustomQuad quad = {};
  quad.set_rect(x, -y, w, h);
  quad.set_colors(color);

  void *camera_transform = (void*)0x898600;
  float translate_x = ACCESS_FLOAT(camera_transform, 0x0C);
  float translate_y = ACCESS_FLOAT(camera_transform, 0x10);
  float scale = ACCESS_FLOAT(camera_transform, 0x14);

  quad.translate(translate_x, translate_y, 0.0f);
  quad.scale(scale, scale);

  CBattleManager_RenderQuad(quad.v);
}

void draw_char_untech_bar(void* char_addr) {
  int untech = ACCESS_SHORT(char_addr, 0x4BA);
  if (untech > 100) {
    untech = 100;
  }

  float x = ACCESS_FLOAT(char_addr, CF_X_POS);
  float y = ACCESS_FLOAT(char_addr, CF_Y_POS);

  void *character = (void*)char_addr;

  if (char_on_ground(character)) {
    return;
  }

  if (!is_airborne_frame(character)) {
    return;
  }

  if (ACCESS_CHAR(char_addr, CF_DAMAGE_LIMITED)) {
    return;
  }

  int seq = ACCESS_SHORT(char_addr, CF_CURRENT_SEQ);
  if (seq < 50 || 150 <= seq) {
    return;
  }

  float w_max = 300.0f;
  float w = untech * (w_max / 100.0f);
  float h = 5.0f;

  D3DCOLOR black[4] = { 0xff000000, 0xff000000, 0xff000000, 0xff000000 };
  D3DCOLOR color[4] = { 0xffffff00, 0xffffff00, 0xffffff00, 0xffffff00 };

  if (untech > 50) {
    unsigned char value = (unsigned char)(((100 - untech) / 50.0f) * 255.0f);
    for (int i = 0; i < 4; ++i) {
      ((ARGB*)color)[i].r = value;
    }
  } else {
    unsigned char value = (unsigned char)((untech / 50.0f) * 255.0f);
    for (int i = 0; i < 4; ++i) {
      ((ARGB*)color)[i].g = value;
    }
  }

  draw_quad(x - w_max / 2.0f, y, w, h, black);
  draw_quad(x - w_max / 2.0f, y, w, h - 2.0f, color);
}

void iterate_over_chars(void *This) {
  MYMEMBER &my = *(MYMEMBER *)((char *)This + s_origCBattleManager_Size);

  int scene_addr = ACCESS_INT(ADDR_BATTLE_MANAGER, 0);
  auto p1 = ACCESS_PTR(scene_addr, 0x0C);
  auto p2 = ACCESS_PTR(scene_addr, 0x10);

  if (my.m_hitboxes) {
    draw_char_boxes(p1);
    draw_char_boxes(p2);
    draw_char_bullets(p1);
    draw_char_bullets(p2);
  }

  if (my.m_untech) {
    draw_char_untech_bar(p1);
    draw_char_untech_bar(p2);
  }
}

static void draw_debug_info(void *This) {
  static char buffer[1024];

  int scene_addr = ACCESS_INT(ADDR_BATTLE_MANAGER, 0);
  int p1 = ACCESS_INT(scene_addr, 0x0C);
  int p2 = ACCESS_INT(scene_addr, 0x10);

  int p1_seq = ACCESS_SHORT(p1, CF_CURRENT_SEQ);
  int p1_sub = ACCESS_SHORT(p1, CF_CURRENT_SUBSEQ);
  int p1_frm = ACCESS_SHORT(p1, CF_CURRENT_FRAME);
  int p2_seq = ACCESS_SHORT(p2, CF_CURRENT_SEQ);
  int p2_sub = ACCESS_SHORT(p2, CF_CURRENT_SUBSEQ);
  int p2_frm = ACCESS_SHORT(p2, CF_CURRENT_FRAME);

  sprintf_s(
    buffer, sizeof(buffer),
    "1f %03d %03d %03d 1l %04d\n"
    "2f %03d %03d %03d 2l %04d\n"
	  "1stop %03d untech %04d 1hst %01d 1mhc %02d\n"
	  "2stop %03d untech %04d 2hst %01d 2mhc %02d\n"
    "1pos %0.2f %0.2f 1vel %0.2f %0.2f 1g %0.2f\n"
    "2pos %0.2f %0.2f 2vel %0.2f %0.2f 2g %0.2f\n",
    ACCESS_SHORT(p1, CF_CURRENT_SEQ),
    ACCESS_SHORT(p1, CF_CURRENT_SUBSEQ),
    ACCESS_SHORT(p1, CF_CURRENT_FRAME),
    ACCESS_INT(p1, CF_ELAPSED_IN_SUBSEQ),
    ACCESS_SHORT(p2, CF_CURRENT_SEQ),
    ACCESS_SHORT(p2, CF_CURRENT_SUBSEQ),
    ACCESS_SHORT(p2, CF_CURRENT_FRAME),
    ACCESS_INT(p2, CF_ELAPSED_IN_SUBSEQ),
    ACCESS_SHORT(p1, CF_HIT_STOP), ACCESS_SHORT(p1, CF_UNTECH),
    ACCESS_INT(p1, CF_HIT_STATE), ACCESS_SHORT(p1, 0x7D0),
	  ACCESS_SHORT(p2, CF_HIT_STOP), ACCESS_SHORT(p2, CF_UNTECH),
    ACCESS_INT(p2, CF_HIT_STATE), ACCESS_SHORT(p2, 0x7D0),
    ACCESS_FLOAT(p1, CF_X_POS), ACCESS_FLOAT(p1, CF_Y_POS),
    ACCESS_FLOAT(p1, CF_X_SPEED), ACCESS_FLOAT(p1, CF_Y_SPEED), ACCESS_FLOAT(p1, CF_GRAVITY),
    ACCESS_FLOAT(p2, CF_X_POS), ACCESS_FLOAT(p2, CF_Y_POS),
    ACCESS_FLOAT(p2, CF_X_SPEED), ACCESS_FLOAT(p2, CF_Y_SPEED), ACCESS_FLOAT(p2, CF_GRAVITY)
  );

  text::SetText(buffer);
  text::OnRender(This);
}

void __fastcall CBattleManager_OnRender(void *This)
{
  MYMEMBER &my = *(MYMEMBER *)((char *)This + s_origCBattleManager_Size);

  CBattleManager_Render(This);

  if(my.m_enabled) {
    CBattleManager_RenderInputPanel(This, my.m_cmdp1,  60, 340);
    CBattleManager_RenderInputPanel(This, my.m_cmdp2, 400, 340);
    CBattleManager_RenderRecordPanel(This, my.m_cmdp1,   0, 300);
    CBattleManager_RenderRecordPanel(This, my.m_cmdp2, 390, 300);

    iterate_over_chars(This);

    if (my.m_show_debug) {
      draw_debug_info(This);
    }
  }
}

void * __fastcall CBattleManager_OnDestruct(void *This, int mystery, int dyn)
{
  MYMEMBER &my = *(MYMEMBER *)((char *)This + s_origCBattleManager_Size);

  if(my.m_enabled) {
    CTextureManager_Remove(g_textureMgr, my.m_texID);
    text::OnDestruct(This, mystery, dyn);

    ::WritePrivateProfileString("Input", "p1.Enabled", my.m_cmdp1.enabled ? "1" : "0", s_profilePath);
    ::WritePrivateProfileString("Input", "p2.Enabled", my.m_cmdp2.enabled ? "1" : "0", s_profilePath);
    ::WritePrivateProfileString("Record", "p1.Enabled", my.m_cmdp1.record.enabled ? "1" : "0", s_profilePath);
    ::WritePrivateProfileString("Record", "p2.Enabled", my.m_cmdp2.record.enabled ? "1" : "0", s_profilePath);
  }

  return CBattleManager_Destruct(This, dyn);
}

extern "C"
__declspec(dllexport) bool CheckVersion(const BYTE hash[16])
{
  return ::memcmp(TARGET_HASH, hash, sizeof TARGET_HASH) == 0;
}

extern "C"
__declspec(dllexport) bool Initialize(HMODULE hMyModule, HMODULE hParentModule)
{
  s_hDllModule = hMyModule;

  HMODULE d3dx = LoadLibrary("D3DX9_33.DLL");
  if(d3dx == NULL) return false;
  *(FARPROC*)&s_D3DXCreateTextureFromResource =
    GetProcAddress(d3dx, "D3DXCreateTextureFromResourceA");
  if(s_D3DXCreateTextureFromResource == NULL) return false;

  GetModuleFileName(hMyModule, s_profilePath, 1024);
  PathRemoveFileSpec(s_profilePath);
  PathAppend(s_profilePath, "ReplayInputView.ini");

  DWORD old;
  ::VirtualProtect((PVOID)text_Offset, text_Size, PAGE_EXECUTE_WRITECOPY, &old);
  s_origCBattleManager_Size =
    TamperDwordAdd((DWORD)&CBattleManager_Size, sizeof(MYMEMBER));
  s_origCBattleManager_OnCreate =
    TamperNearJmpOpr(CBattleManager_Creater, union_cast<DWORD>(CBattleManager_OnCreate));

  // combo counter
  TamperByte(0x4792FB + 2, 1);

  ::VirtualProtect((PVOID)text_Offset, text_Size, old, &old);

  ::VirtualProtect((PVOID)rdata_Offset, rdata_Size, PAGE_WRITECOPY, &old);
  s_origCBattleManager_OnDestruct =
    TamperDword(vtbl_CBattleManager + 0x00, union_cast<DWORD>(CBattleManager_OnDestruct));
  s_origCBattleManager_OnRender =
    TamperDword(vtbl_CBattleManager + 0x38, union_cast<DWORD>(CBattleManager_OnRender));
  s_origCBattleManager_OnProcess =
    TamperDword(vtbl_CBattleManager + 0x0c, union_cast<DWORD>(CBattleManager_OnProcess));
  ::VirtualProtect((PVOID)rdata_Offset, rdata_Size, old, &old);

  ::FlushInstructionCache(GetCurrentProcess(), NULL, 0);
  
  return true;
}

extern "C"
int APIENTRY DllMain(HMODULE hModule, DWORD fdwReason, LPVOID lpReserved)
{
  return TRUE;
}

