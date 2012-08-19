ndef __LW_MIDI_H__
#define	__LW_MIDI_H__

#include <Mmsystem.h>

namespace lw {

enum
{
	MIDI_NOTEON				= 0x90,
	MIDI_NOTEOFF			= 0x80,
	MIDI_AFTERTOUCH			= 0xA0,
	MIDI_CONTROLLER			= 0xB0,
	MIDI_PROGRAMCHANGE		= 0xC0,
	MIDI_CHANNELPRESSURE	= 0xD0,
	MIDI_PITCHWHEEL			= 0xE0,

	MIDI_RESET				= 0xFF,

	MIDI_CONTROLLER_VOLUME		= 7,	//coarse = 7, fine = 39
	MIDI_CONTROLLER_ALLNOTESOFF	= 123,
};

enum
{
	MIDI_OK					= 0,
	MIDIE_INVALID_CHANNEL	= 1,
	MIDIE_MIDI_MSG			= 2,		//midiOutShortMsg() 返回错误
};

class Midi
{
public:
	static void begin() {
		midiOutOpen(&getInstance()->_hMidi, 0, NULL, NULL, CALLBACK_NULL);
		//pitchBendRange(32);
	}

	static DWORD makeMsg(BYTE status, BYTE data1, BYTE data2)
	{
		union { 
			DWORD dwData; 
			BYTE bData[4]; 
		} u; 

		u.bData[0] = status;  // MIDI status byte 
		u.bData[1] = data1;   // first MIDI data byte 
		u.bData[2] = data2;   // second MIDI data byte 
		u.bData[3] = 0;

		return u.dwData;
	}

	static bool midiMsg(BYTE status, BYTE data1, BYTE data2) {
		union { 
			DWORD dwData; 
			BYTE bData[4]; 
		} u; 

		// Construct the MIDI message. 
		u.bData[0] = status;  // MIDI status byte 
		u.bData[1] = data1;   // first MIDI data byte 
		u.bData[2] = data2;   // second MIDI data byte 
		u.bData[3] = 0;

		if ( MMSYSERR_NOERROR == midiOutShortMsg(getInstance()->_hMidi, u.dwData) )
			return true;
		return false;
	}

	static bool midiMsg(DWORD data) {
		if ( MMSYSERR_NOERROR == midiOutShortMsg(getInstance()->_hMidi, data) )
			return true;
		return false;
	}

	static int reset() {
		if ( !midiMsg(MIDI_RESET, 0, 0) )
			return MIDIE_MIDI_MSG;
		return MIDI_OK;
	}

	static int noteOn(unsigned char note, unsigned char velocity, unsigned char channel = 0) {
		if ( channel > 15 )
			return MIDIE_INVALID_CHANNEL;
		if ( !midiMsg(MIDI_NOTEON + channel, note, velocity) )
			return MIDIE_MIDI_MSG;
		return MIDI_OK;
	}

	static int noteOff(unsigned char note, unsigned char velocity, unsigned char channel = 0) {
		if ( channel > 15 )
			return MIDIE_INVALID_CHANNEL;
		if ( !midiMsg(MIDI_NOTEOFF + channel, note, velocity) )
			return MIDIE_MIDI_MSG;
		return MIDI_OK;
	}

	static int allNotesOff(unsigned char channel = 0) {
		if ( channel > 15 )
			return MIDIE_INVALID_CHANNEL;
		if ( !midiMsg(MIDI_CONTROLLER + channel, MIDI_CONTROLLER_ALLNOTESOFF, 127) )
			return MIDIE_MIDI_MSG;
		return MIDI_OK;
	}

	static int programChange(unsigned char programNum, unsigned char channel = 0) {
		if ( channel > 15 )
			return MIDIE_INVALID_CHANNEL;
		if ( !midiMsg(MIDI_PROGRAMCHANGE + channel, programNum, 0) )
			return MIDIE_MIDI_MSG;
		return MIDI_OK;
	}

	static int aftertouch(unsigned char note, unsigned char pressure, unsigned char channel = 0) {
		if ( channel > 15 )
			return MIDIE_INVALID_CHANNEL;
		if ( !midiMsg(MIDI_AFTERTOUCH + channel, note, pressure) )
			return MIDIE_MIDI_MSG;
		return MIDI_OK;
	}

	static int channelPressure(unsigned char pressure, unsigned char channel = 0) {
		if ( channel > 15 )
			return MIDIE_INVALID_CHANNEL;
		if ( !midiMsg(MIDI_CHANNELPRESSURE + channel, pressure, 0) )
			return MIDIE_MIDI_MSG;
		return MIDI_OK;
	}

	static int pitchWheel(float offset, unsigned char channel = 0) {
		if ( channel > 15 )
			return MIDIE_INVALID_CHANNEL;
		
		short s = (unsigned short)(0x2000*(1+offset*getInstance()->_rvStep));
		if ( s > 0x4000 )
			s = 0x4000;
		else if ( s < 0 )
			s = 0;

		if ( !midiMsg(MIDI_PITCHWHEEL + channel, s&0x7f, s>>7) )
			return MIDIE_MIDI_MSG;
		return MIDI_OK;
	}

	static int volume(unsigned char value, unsigned char channel = 0) {
		if ( channel > 15 )
			return MIDIE_INVALID_CHANNEL;
		if ( !midiMsg(MIDI_CONTROLLER + channel, MIDI_CONTROLLER_VOLUME, value) )
			return MIDIE_MIDI_MSG;
		return MIDI_OK;
	}

	static void pitchBendRange(unsigned char step) {
		midiMsg(MIDI_CONTROLLER, 101, 0);
		midiMsg(MIDI_CONTROLLER, 6, step);
		getInstance()->_rvStep = 1.0f / step;
	}

	static int readVarLen(char** pp)
	{
		char* p = *pp;
		int value = 0;
		while (true)
		{
			value = (value << 7) + (*p & 0x7F);
			if ( ((*p) & 0x80) == 0 )
			{
				++p;
				break;
			}
			++p;
		}

		*pp = p;
		return(value);
	}

	template<typename T>
	static void bitOrderReverse(T& value)
	{
		byte temp[sizeof(T)];
		for ( int i = 0; i < sizeof(T); ++i )
		{
			temp[i] = (byte)((value>>(i*8)) & 0xff);
		}
		value = 0;
		for ( int i = 0; i < sizeof(T); ++i )
		{
			value |= temp[i] << ((sizeof(T)-i-1)*8);
		}
	}


private:
	static Midi* getInstance() {
		static Midi inst;
		return &inst;
	}

	HMIDIOUT _hMidi;
	float _rvStep;
};

static const wchar_t* PROGRAM_NAME[] = {
	L"0 Acoustic Grand Piano   大钢琴（声学钢琴）",
	L"1 Bright Acoustic Piano         明亮的钢琴",
	L"2 Electric Grand Piano              电钢琴",
	L"3 Honky-tonk Piano                酒吧钢琴",
	L"4 Rhodes Piano                柔和的电钢琴",
	L"5 Chorused Piano        加合唱效果的电钢琴",
	L"6 Harpsichord        羽管键琴（拨弦古钢琴）",
	L"7 Clavichord     科拉维科特琴（击弦古钢琴）",

	L"8 Celesta                           钢片琴",
	L"9 Glockenspiel                        钟琴",
	L"10 Music box                        八音盒",
	L"11 Vibraphone                       颤音琴",
	L"12 Marimba                          马林巴",
	L"13 Xylophone                          木琴",
	L"14 Tubular Bells                      管钟",
	L"15 Dulcimer                         大扬琴",

	L"16 Hammond Organ                  击杆风琴",
	L"17 Percussive Organ             打击式风琴",
	L"18 Rock Organ                     摇滚风琴",
	L"19 Church Organ                   教堂风琴",
	L"20 Reed Organ                     簧管风琴",
	L"21 Accordian                        手风琴",
	L"22 Harmonica                          口琴",
	L"23 Tango Accordian              探戈手风琴",

	L"24 Acoustic Guitar (nylon)      尼龙弦吉他",
	L"25 Acoustic Guitar (steel)        钢弦吉他",
	L"26 Electric Guitar (jazz)       爵士电吉他",
	L"27 Electric Guitar (clean)      清音电吉他",
	L"28 Electric Guitar (muted)      闷音电吉他",
	L"29 Overdriven Guitar    加驱动效果的电吉他",
	L"30 Distortion Guitar    加失真效果的电吉他",
	L"31 Guitar Harmonics               吉他和音",

	L"32 Acoustic Bass         大贝司（声学贝司）",
	L"33 Electric Bass(finger)     电贝司（指弹）",
	L"34 Electric Bass (pick)      电贝司（拨片）",
	L"35 Fretless Bass                  无品贝司",
	L"36 Slap Bass 1                   掌击Bass 1",
	L"37 Slap Bass 2                  掌击Bass 2",
	L"38 Synth Bass 1             电子合成Bass 1",
	L"39 Synth Bass 2             电子合成Bass 2",

	L"40 Violin                           小提琴",
	L"41 Viola                            中提琴",
	L"42 Cello                            大提琴",
	L"43 Contrabass                   低音大提琴",
	L"44 Tremolo Strings          弦乐群颤音音色",
	L"45 Pizzicato Strings        弦乐群拨弦音色",
	L"46 Orchestral Harp                    竖琴",
	L"47 Timpani                          定音鼓",

	L"48 String Ensemble 1         弦乐合奏音色1",
	L"49 String Ensemble 2         弦乐合奏音色2",
	L"50 Synth Strings 1       合成弦乐合奏音色1",
	L"51 Synth Strings 2       合成弦乐合奏音色2",
	L"52 Choir Aahs               人声合唱“啊”",
	L"53 Voice Oohs                   人声“嘟”",
	L"54 Synth Voice                    合成人声",
	L"55 Orchestra Hit            管弦乐敲击齐奏",

	L"56 Trumpet                            小号",
	L"57 Trombone                           长号",
	L"58 Tuba                               大号",
	L"59 Muted Trumpet              加弱音器小号",
	L"60 French Horn               法国号（圆号）",
	L"61 Brass Section 铜管组（铜管乐器合奏音色）",
	L"62 Synth Brass 1             合成铜管音色1",
	L"63 Synth Brass 2             合成铜管音色2",

	L"64 Soprano Sax                高音萨克斯风",
	L"65 Alto Sax                 次中音萨克斯风",
	L"66 Tenor Sax                  中音萨克斯风",
	L"67 Baritone Sax               低音萨克斯风",
	L"68 Oboe                             双簧管",
	L"69 English Horn                     英国管",
	L"70 Bassoon                     巴松（大管）",
	L"71 Clarinet                  单簧管（黑管）",

	L"72 Piccolo                            短笛",
	L"73 Flute                               长笛",
	L"74 Recorder                           竖笛",
	L"75 Pan Flute                          排箫",
	L"76 Bottle Blow               [中文名称暂缺]",
	L"77 Shakuhachi                     日本尺八",
	L"78 Whistle                          口哨声",
	L"79 Ocarina                         奥卡雷那",

	L"80 Lead 1 (square)        合成主音1（方波）",
	L"81 Lead 2 (sawtooth)    合成主音2（锯齿波）",
	L"82 Lead 3 (caliope lead)         合成主音3",
	L"83 Lead 4 (chiff lead)           合成主音4",
	L"84 Lead 5 (charang)              合成主音5",
	L"85 Lead 6 (voice)         合成主音6（人声）",
	L"86 Lead 7 (fifths)    合成主音7（平行五度）",
	L"87 Lead 8 (bass+lead)合成主音8（贝司加主音）",

	L"88 Pad 1 (new age)      合成音色1（新世纪）",
	L"89 Pad 2 (warm)          合成音色2 （温暖）",
	L"90 Pad 3 (polysynth)              合成音色3",
	L"91 Pad 4 (choir)         合成音色4 （合唱）",
	L"92 Pad 5 (bowed)                 合成音色5",
	L"93 Pad 6 (metallic)    合成音色6 （金属声）",
	L"94 Pad 7 (halo)          合成音色7 （光环）",
	L"95 Pad 8 (sweep)                 合成音色8",

	L"96 FX 1 (rain)             合成效果 1 雨声",
	L"97 FX 2 (soundtrack)       合成效果 2 音轨",
	L"98 FX 3 (crystal)          合成效果 3 水晶",
	L"99 FX 4 (atmosphere)       合成效果 4 大气",
	L"100 FX 5 (brightness)      合成效果 5 明亮",
	L"101 FX 6 (goblins)         合成效果 6 鬼怪",
	L"102 FX 7 (echoes)          合成效果 7 回声",
	L"103 FX 8 (sci-fi)          合成效果 8 科幻",

	L"104 Sitar                    西塔尔（印度）",
	L"105 Banjo                    班卓琴（美洲）",
	L"106 Shamisen                 三昧线（日本）",
	L"107 Koto                   十三弦筝（日本）",
	L"108 Kalimba                         卡林巴",
	L"109 Bagpipe                           风笛",
	L"110 Fiddle                        民族提琴",
	L"111 Shanai                            山奈",

	L"112 Tinkle Bell                     叮当铃",
	L"113 Agogo                    [中文名称暂缺]",
	L"114 Steel Drums                       钢鼓",
	L"115 Woodblock                         木鱼",
	L"116 Taiko Drum                        太鼓",
	L"117 Melodic Tom                     通通鼓",
	L"118 Synth Drum                      合成鼓",
	L"119 Reverse Cymbal                    铜钹",

	L"120 Guitar Fret Noise         吉他换把杂音",
	L"121 Breath Noise                    呼吸声",
	L"122 Seashore                        海浪声",
	L"123 Bird Tweet                        鸟鸣",
	L"124 Telephone Ring                  电话铃",
	L"125 Helicopter                      直升机",
	L"126 Applause                        鼓掌声",
	L"127 Gunshot                           枪声",

};


} //namespace lw

#pragma comment(lib, "Winmm.lib")

#endif //__LW_MIDI_H__