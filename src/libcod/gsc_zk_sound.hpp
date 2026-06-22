#ifndef _GSC_ZK_SOUND_HPP_
#define _GSC_ZK_SOUND_HPP_
#ifdef LIBCOD
#include "gsc.hpp"

#ifndef COMPILE_CUSTOM_VOICE
#define COMPILE_CUSTOM_VOICE 1
#endif

// ---- custom sound-file playback (speex over the voice channel) ----
#define MAX_VOICEFRAMESIZE          160
#define MAX_VOICEPACKETDATALEN      256
#define MAX_VOICEPACKETSPERFRAME    2.56   // ~51.2 packets/sec @ 20 server fps
#define MAX_CUSTOMSOUNDDURATION     10     // minutes
#define MAX_STOREDVOICEPACKETS      (MAX_CUSTOMSOUNDDURATION * 3072)
#define MAX_CUSTOMSOUNDS            64
#define MAX_THREAD_RESULTS_BUFFER   64
// NOTE: rev's VoicePacket_t fields are { talker, data, dataSize } (zk used talkerNum/dataLen).

typedef enum
{
	ENCODER_OK,
	ENCODER_FILE_TOO_LONG,
	ENCODER_FILE_NOT_FOUND,
	ENCODER_FILE_READ_ERROR
} loadSoundFileThreadResult_t;

typedef struct
{
	int callback;
	int soundIndex;
	loadSoundFileThreadResult_t result;
	unsigned int levelId;
} loadSoundFileResult_t;

extern VoicePacket_t voiceDataStore[MAX_CUSTOMSOUNDS][MAX_STOREDVOICEPACKETS];
extern loadSoundFileResult_t loadSoundFileResults[MAX_THREAD_RESULTS_BUFFER];
extern int loadSoundFileResultsIndex;
extern int currentMaxSoundIndex;

extern unsigned short scr_const_sound_file_stop;

void SV_DrainSoundFileResults(void);   // per-frame: fire completed-encode callbacks
void SV_RunCustomSounds(void);         // per-frame: stream voice packets to players

// builtins
void gsc_utils_loadsoundfile();
void gsc_utils_loadspeexfile();
void gsc_utils_savespeexfile();
void gsc_utils_getsoundfileduration();
void gsc_player_playsoundfile(scr_entref_t ref);
void gsc_player_stopsoundfile(scr_entref_t ref);
void gsc_player_isplayingsoundfile(scr_entref_t ref);
void gsc_player_getremainingsoundfileduration(scr_entref_t ref);

#endif
#endif
