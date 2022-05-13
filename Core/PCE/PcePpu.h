#pragma once
#include "stdafx.h"
#include "Shared/Emulator.h"
#include "PCE/PceTypes.h"
#include "PCE/PceConstants.h"

class PceConsole;
class PceVce;

enum class PcePpuModeH
{
	Hds,
	Hdw,
	Hde,
	Hsw,
};

enum class PcePpuModeV
{
	Vds,
	Vdw,
	Vde,
	Vsw,
};

enum class PceVdcEvent
{
	None,
	LatchScrollY,
	LatchScrollX,
	HdsIrqTrigger,
	IncRcrCounter,
};

struct PceTileInfo
{
	uint16_t TileData[2];
	uint16_t TileAddr;
	uint8_t Palette;
};

struct PceSpriteInfo
{
	uint16_t TileData[4];
	int16_t X;
	uint16_t TileAddress;
	uint8_t Index;
	uint8_t Palette;
	bool HorizontalMirroring;
	bool ForegroundPriority;
	bool LoadSp23;
};

class PcePpu
{
private:
	PcePpuState _state = {};
	Emulator* _emu = nullptr;
	PceConsole* _console = nullptr;
	PceVce* _vce = nullptr;
	uint16_t* _vram = nullptr;
	uint16_t* _spriteRam = nullptr;

	uint16_t* _outBuffer[2] = {};
	uint16_t* _currentOutBuffer = nullptr;
	
	uint8_t _rowVceClockDivider[2][PceConstants::ScreenHeight] = {};
	uint8_t* _currentClockDividers = nullptr;

	uint16_t _rowBuffer[PceConstants::MaxScreenWidth] = {};

	uint16_t _vramOpenBus = 0;

	uint16_t _lastDrawHClock = 0;
	uint16_t _xStart = 0;

	PcePpuModeH _hMode = PcePpuModeH::Hds;
	int16_t _hModeCounter = 0;
	
	PcePpuModeV _vMode = PcePpuModeV::Vds;
	int16_t _vModeCounter = 0;

	uint16_t _screenOffsetX = 0;
	bool _needRcrIncrement = false;
	bool _needVertBlankIrq = false;
	bool _verticalBlankDone = false;

	uint8_t _spriteCount = 0;
	uint16_t _spriteRow = 0;
	uint16_t _evalStartCycle = 0;
	uint16_t _evalEndCycle = 0;
	int16_t _evalLastCycle = 0;
	bool _hasSpriteOverflow = false;

	uint16_t _loadBgStart = 0;
	uint16_t _loadBgEnd = 0;
	int16_t _loadBgLastCycle = 0;
	uint8_t _tileCount = 0;
	bool _allowVramAccess = false;

	bool _pendingMemoryRead = false;
	bool _pendingMemoryWrite = false;
	
	bool _vramDmaRunning = false;
	bool _vramDmaReadCycle = false;
	uint16_t _vramDmaBuffer = 0;
	uint16_t _vramDmaPendingCycles = 0;

	PceVdcEvent _nextEvent = PceVdcEvent::None;
	uint16_t _nextEventCounter = 0;

	uint8_t _drawSpriteCount = 0;
	uint8_t _totalSpriteCount = 0;
	bool _rowHasSprite0 = false;
	uint16_t _loadSpriteStart = 0;
	PceSpriteInfo _sprites[64] = {};
	PceSpriteInfo _drawSprites[64] = {};
	PceTileInfo _tiles[100] = {};

	template<uint16_t bitMask = 0xFFFF>
	void UpdateReg(uint16_t& reg, uint8_t value, bool msb)
	{
		if(msb) {
			reg = ((reg & 0xFF) | (value << 8)) & bitMask;
		} else {
			reg = ((reg & 0xFF00) | value) & bitMask;
		}
	}

	void ProcessVramRead();
	void ProcessVramWrite();
	__noinline void ProcessVramAccesses();

	void SendFrame();

	uint8_t GetClockDivider();
	uint16_t GetScanlineCount();
	uint16_t DotsToClocks(int dots);
	void TriggerHdsIrqs();

	__noinline void IncrementRcrCounter();
	__noinline void IncScrollY();
	__noinline void ProcessEndOfScanline();
	__noinline void ProcessEndOfVisibleFrame();
	__noinline void ProcessSatbTransfer();
	__noinline void ProcessVramDmaTransfer();
	__noinline void SetHorizontalMode(PcePpuModeH hMode);

	__noinline void ProcessVdcEvents();
	__noinline void ProcessEvent();

	__noinline void ProcessHorizontalSyncStart();
	__noinline void ProcessVerticalSyncStart();

	__forceinline uint8_t GetTilePixelColor(const uint16_t chrData[2], const uint8_t shift);
	__forceinline uint8_t GetSpritePixelColor(const uint16_t chrData[4], const uint8_t shift);

	__noinline void ProcessSpriteEvaluation();
	__noinline void LoadSpriteTiles();
	
	__noinline void LoadBackgroundTiles();
	__noinline void LoadBackgroundTilesWidth2(uint16_t end, uint16_t scrollOffset, uint16_t columnMask, uint16_t row);
	__noinline void LoadBackgroundTilesWidth4(uint16_t end, uint16_t scrollOffset, uint16_t columnMask, uint16_t row);
	
	__forceinline void LoadBatEntry(uint16_t scrollOffset, uint16_t columnMask, uint16_t row);
	__forceinline void LoadTileDataCg0(uint16_t row);
	__forceinline void LoadTileDataCg1(uint16_t row);

	__forceinline uint16_t ReadVram(uint16_t addr);

	void WaitForVramAccess();
	bool IsVramAccessBlocked();

public:
	PcePpu(Emulator* emu, PceConsole* console, PceVce* vce);
	~PcePpu();

	PcePpuState& GetState();
	uint16_t* GetScreenBuffer();
	uint16_t* GetPreviousScreenBuffer();
	uint8_t* GetRowClockDividers() { return _currentClockDividers; }
	uint8_t* GetPreviousRowClockDividers() { return _currentClockDividers == _rowVceClockDivider[0] ? _rowVceClockDivider[1] : _rowVceClockDivider[0]; }

	uint16_t GetHClock() { return _state.HClock; }
	uint16_t GetScanline() { return _state.Scanline; }
	uint16_t* GetRowBuffer() { return _rowBuffer; }
	uint16_t GetFrameCount() { return _state.FrameCount; }

	void Exec();
	void DrawScanline();

	uint8_t ReadVdc(uint16_t addr);
	void WriteVdc(uint16_t addr, uint8_t value);
};
