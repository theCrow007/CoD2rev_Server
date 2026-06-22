#include "gsc_zk_sound.hpp"
#ifdef LIBCOD

#if COMPILE_CUSTOM_VOICE == 1
#include "gsc_zk_custom_state.hpp"
#include <speex/speex.h>
#include <pthread.h>

// rev exposes these only as definitions (no header); declare locally like other libcod TUs.
#define MAX_STRINGLENGTH 1024
extern dvar_t *fs_homepath;
extern dvar_t *fs_debug;
extern dvar_t *sv_voiceQuality;
void FS_BuildOSPath(const char *base, const char *game, const char *qpath, char *ospath);

// ===========================================================================
// Custom sound-file playback (faithful port of zk_libcod's COMPILE_CUSTOM_VOICE).
// Audio input is RAW 16-bit signed PCM, mono, 8 kHz (pre-convert with ffmpeg/sox).
// rev's VoicePacket_t is { byte talker; byte data[256]; int dataSize; }
// (zk used talkerNum/dataLen -> mapped to talker/dataSize here).
// ===========================================================================

VoicePacket_t voiceDataStore[MAX_CUSTOMSOUNDS][MAX_STOREDVOICEPACKETS];

loadSoundFileResult_t loadSoundFileResults[MAX_THREAD_RESULTS_BUFFER];
int loadSoundFileResultsIndex = 0;
int currentMaxSoundIndex = 0;

struct encoder_async_task
{
	encoder_async_task *prev;
	encoder_async_task *next;
	char filePath[MAX_STRINGLENGTH];
	int callback;
	float volume;
	int soundIndex;
	unsigned int levelId;
};

static encoder_async_task *first_encoder_async_task = NULL;

extern dvar_t *g_voiceChatTalkingDuration;

static void Encode_FreeTask(encoder_async_task *task)
{
	Sys_EnterCriticalSection(CRITSECT_LOAD_SOUND_FILE);
	if ( task->next != NULL )
		task->next->prev = task->prev;
	if ( task->prev != NULL )
		task->prev->next = task->next;
	else
		first_encoder_async_task = task->next;
	Sys_LeaveCriticalSection(CRITSECT_LOAD_SOUND_FILE);
	delete task;
}

static void Encode_SetOptions(void *encoder)
{
	int samplerate = 8192;
	int quality = sv_voiceQuality ? sv_voiceQuality->current.integer : 4;
	int enabled = 0;
	speex_encoder_ctl(encoder, SPEEX_SET_SAMPLING_RATE, &samplerate);
	speex_encoder_ctl(encoder, SPEEX_SET_QUALITY, &quality);
	speex_encoder_ctl(encoder, SPEEX_SET_VAD, &enabled);
	speex_encoder_ctl(encoder, SPEEX_SET_DTX, &enabled);
}

static void *Encode_Async(void *newtask)
{
	encoder_async_task *task = (encoder_async_task*)newtask;
	loadSoundFileThreadResult_t result = ENCODER_OK;

	memset(&voiceDataStore[task->soundIndex - 1], 0, sizeof(voiceDataStore[0]));

	short in[MAX_VOICEFRAMESIZE];
	float input[MAX_VOICEFRAMESIZE];
	char data[MAX_VOICEPACKETDATALEN];
	size_t readLen;
	int dataLen;
	void *g_encoder;
	SpeexBits encodeBits;
	unsigned int i, packetIndex;
	VoicePacket_t *voicePacket;

	FILE *file = fopen(task->filePath, "r");
	if ( file )
	{
		g_encoder = speex_encoder_init(&speex_nb_mode);
		speex_bits_init(&encodeBits);
		Encode_SetOptions(g_encoder);

		for ( packetIndex = 0; packetIndex <= MAX_STOREDVOICEPACKETS; packetIndex++ )
		{
			memset(input, 0, sizeof(input));

			if ( packetIndex == MAX_STOREDVOICEPACKETS )
			{
				result = ENCODER_FILE_TOO_LONG;
				break;
			}

			readLen = fread(in, sizeof(short), MAX_VOICEFRAMESIZE, file);
			if ( readLen < MAX_VOICEFRAMESIZE && !feof(file) )
			{
				result = ENCODER_FILE_READ_ERROR;
				break;
			}

			for ( i = 0; i < readLen; i++ )
				input[i] = in[i] * task->volume;

			speex_bits_reset(&encodeBits);
			speex_encode(g_encoder, input, &encodeBits);
			dataLen = speex_bits_write(&encodeBits, data, MAX_VOICEPACKETDATALEN);
			voicePacket = &voiceDataStore[task->soundIndex - 1][packetIndex];
			memcpy(voicePacket->data, data, dataLen);
			voicePacket->dataSize = dataLen; // zk: dataLen

			if ( feof(file) )
				break;
		}
		if ( packetIndex != MAX_STOREDVOICEPACKETS )
			memset(&voiceDataStore[task->soundIndex - 1][packetIndex], 0, sizeof(voiceDataStore[0][0]));

		speex_encoder_destroy(g_encoder);
		speex_bits_destroy(&encodeBits);
		fclose(file);
	}
	else
	{
		result = ENCODER_FILE_NOT_FOUND;
	}

	Sys_EnterCriticalSection(CRITSECT_LOAD_SOUND_FILE);
	if ( loadSoundFileResultsIndex < MAX_THREAD_RESULTS_BUFFER )
	{
		loadSoundFileResults[loadSoundFileResultsIndex].result = result;
		loadSoundFileResults[loadSoundFileResultsIndex].soundIndex = task->soundIndex;
		loadSoundFileResults[loadSoundFileResultsIndex].callback = task->callback;
		loadSoundFileResults[loadSoundFileResultsIndex].levelId = task->levelId;
		loadSoundFileResultsIndex++;
	}
	Sys_LeaveCriticalSection(CRITSECT_LOAD_SOUND_FILE);

	Encode_FreeTask(task);
	return NULL;
}

// Per-frame: fire GSC callbacks for completed encode jobs (main thread).
void SV_DrainSoundFileResults(void)
{
	int i;
	if ( Scr_IsSystemActive() && loadSoundFileResultsIndex > 0 )
	{
		if ( Sys_TryEnterCriticalSection(CRITSECT_LOAD_SOUND_FILE) == 0 )
		{
			if ( loadSoundFileResultsIndex == MAX_THREAD_RESULTS_BUFFER )
				Com_Printf("WARNING: LoadSoundFile results buffer full\n");
			for ( i = 0; i < loadSoundFileResultsIndex; i++ )
			{
				// Skip callbacks from a previous levelId (scripts recompiled = stale reference).
				if ( scrVarPub.levelId == loadSoundFileResults[i].levelId )
				{
					stackPushInt(loadSoundFileResults[i].result);
					stackPushInt(loadSoundFileResults[i].soundIndex);
					short ret = Scr_ExecThread(loadSoundFileResults[i].callback, 2);
					Scr_FreeThread(ret);
				}
				else
				{
					Com_Printf("WARNING: LoadSoundFile result from previous map discarded\n");
				}
			}
			loadSoundFileResultsIndex = 0;
			Sys_LeaveCriticalSection(CRITSECT_LOAD_SOUND_FILE);
		}
	}
}

// Per-frame: stream the next voice packets for each player with an active custom sound.
void SV_RunCustomSounds(void)
{
	int i;
	client_t *client;
	gclient_t *gclient;
	int durationSinceLastTalk;
	qboolean aPlayerIsTalking = qfalse;

	// Pause custom sounds while a real player is talking (avoids audio collision).
	gclient = level.clients;
	for ( i = 0; i < sv_maxclients->current.integer; i++, gclient++ )
	{
		durationSinceLastTalk = level.time - gclient->lastVoiceTime;
		if ( durationSinceLastTalk >= 0 && g_voiceChatTalkingDuration && g_voiceChatTalkingDuration->current.integer > durationSinceLastTalk )
		{
			aPlayerIsTalking = qtrue;
			break;
		}
	}

	if ( aPlayerIsTalking )
		return;

	client = svs.clients;
	for ( i = 0; i < sv_maxclients->current.integer; i++, client++ )
	{
		if ( client->state < CS_CONNECTED )
			continue;

		if ( !customPlayerState[i].currentSoundIndex )
			continue;

		customPlayerState[i].pendingVoiceDataFrames += MAX_VOICEPACKETSPERFRAME;
		VoicePacket_t *voicePacket;

		for ( ; customPlayerState[i].pendingVoiceDataFrames > 1.0 && customPlayerState[i].sentVoiceDataIndex < MAX_STOREDVOICEPACKETS;
		      customPlayerState[i].sentVoiceDataIndex++, customPlayerState[i].pendingVoiceDataFrames -= 1.0 )
		{
			voicePacket = &voiceDataStore[customPlayerState[i].currentSoundIndex - 1][customPlayerState[i].sentVoiceDataIndex];
			if ( svs.clients[customPlayerState[i].currentSoundTalker].state < CS_CONNECTED || !voicePacket->dataSize )
			{
				customPlayerState[i].pendingVoiceDataFrames = 0.0;
				customPlayerState[i].currentSoundTalker = 0;
				customPlayerState[i].currentSoundIndex = 0;
				customPlayerState[i].sentVoiceDataIndex = 0;
				if ( Scr_IsSystemActive() )
					Scr_Notify(&g_entities[i], scr_const_sound_file_done, 0);
				break;
			}
			voicePacket->talker = customPlayerState[i].currentSoundTalker; // zk: talkerNum
			SV_QueueVoicePacket(voicePacket->talker, i, voicePacket);
		}
	}
}

// ===========================================================================
// builtins
// ===========================================================================

void gsc_utils_loadsoundfile()
{
	const char *filePath;
	int callback;
	float volume;
	int soundIndex;

	if ( !stackGetParamString(0, &filePath) )
	{
		stackError("gsc_utils_loadsoundfile() requires a file path (string) as first argument");
		stackPushUndefined();
		return;
	}
	if ( !stackGetParamFunction(1, &callback) )
	{
		stackError("gsc_utils_loadsoundfile() requires a callback function as second argument");
		stackPushUndefined();
		return;
	}
	if ( !stackGetParamFloat(2, &volume) )
		volume = 1.0;
	if ( volume > 1.0 ) volume = 1.0;
	if ( volume < 0.0 ) volume = 0.0;
	if ( !stackGetParamInt(3, &soundIndex) )
		soundIndex = ++currentMaxSoundIndex;

	if ( soundIndex < 1 || soundIndex > MAX_CUSTOMSOUNDS )
	{
		stackError("gsc_utils_loadsoundfile() invalid sound index, valid range is 1-%d", MAX_CUSTOMSOUNDS);
		stackPushUndefined();
		return;
	}

	char osPath[MAX_OSPATH];
	FS_BuildOSPath(fs_homepath->current.string, filePath, "", osPath);
	osPath[strlen(osPath) - 1] = '\0';
	if ( fs_debug->current.integer )
		Com_Printf("gsc_utils_loadsoundfile (fs_homepath) : %s\n", osPath);
	if ( strstr(osPath, "..") )
	{
		stackError("gsc_utils_loadsoundfile() invalid file path");
		stackPushUndefined();
		return;
	}

	FILE *file = fopen(osPath, "r");
	if ( !file )
	{
		stackError("gsc_utils_loadsoundfile() input file could not be opened");
		stackPushUndefined();
		return;
	}
	fclose(file);

	Sys_EnterCriticalSection(CRITSECT_LOAD_SOUND_FILE);
	encoder_async_task *current = first_encoder_async_task;
	while ( current != NULL && current->next != NULL )
		current = current->next;
	encoder_async_task *newtask = new encoder_async_task;
	newtask->prev = current;
	newtask->next = NULL;
	I_strncpyz(newtask->filePath, osPath, MAX_STRINGLENGTH);
	newtask->callback = callback;
	newtask->volume = volume;
	newtask->soundIndex = soundIndex;
	newtask->levelId = scrVarPub.levelId;
	if ( current != NULL )
		current->next = newtask;
	else
		first_encoder_async_task = newtask;
	Sys_LeaveCriticalSection(CRITSECT_LOAD_SOUND_FILE);

	pthread_t encoder_doer;
	if ( pthread_create(&encoder_doer, NULL, Encode_Async, newtask) != 0 )
	{
		Encode_FreeTask(newtask);
		stackError("gsc_utils_loadsoundfile() error creating encoder async handler thread");
		stackPushUndefined();
		return;
	}
	pthread_detach(encoder_doer);

	stackPushInt(soundIndex);
}

void gsc_utils_loadspeexfile()
{
	const char *filePath;
	int soundIndex;

	if ( !stackGetParamString(0, &filePath) )
	{
		stackError("gsc_utils_loadspeexfile() requires a file path (string) as first argument");
		stackPushBool(qfalse);
		return;
	}
	if ( !stackGetParamInt(1, &soundIndex) )
	{
		stackError("gsc_utils_loadspeexfile() requires a sound index (integer) as second argument");
		stackPushBool(qfalse);
		return;
	}
	if ( soundIndex < 1 || soundIndex > MAX_CUSTOMSOUNDS )
	{
		stackError("gsc_utils_loadspeexfile() invalid sound index, valid range is 1-%d", MAX_CUSTOMSOUNDS);
		stackPushBool(qfalse);
		return;
	}

	char ospath[MAX_OSPATH];
	FS_BuildOSPath(fs_homepath->current.string, filePath, "", ospath);
	ospath[strlen(ospath) - 1] = '\0';
	if ( fs_debug->current.integer )
		Com_Printf("gsc_utils_loadspeexfile (fs_homepath) : %s\n", ospath);
	if ( strstr(ospath, "..") )
	{
		stackError("gsc_utils_loadspeexfile() invalid file path");
		stackPushUndefined();
		return;
	}

	FILE *file = fopen(ospath, "rb");
	if ( !file )
	{
		stackError("gsc_utils_loadspeexfile() could not open the specified file");
		stackPushBool(qfalse);
		return;
	}

	int packetIndex;
	VoicePacket_t packet;
	for ( packetIndex = 0; packetIndex <= MAX_STOREDVOICEPACKETS; packetIndex++ )
	{
		if ( packetIndex == MAX_STOREDVOICEPACKETS )
			break;
		fread(&packet, sizeof(packet), 1, file);
		if ( feof(file) )
			break;
		memcpy(&voiceDataStore[soundIndex - 1][packetIndex], &packet, sizeof(packet));
	}
	if ( packetIndex != MAX_STOREDVOICEPACKETS )
		memset(&voiceDataStore[soundIndex - 1][packetIndex], 0, sizeof(packet));
	fclose(file);

	stackPushBool(qtrue);
}

void gsc_utils_savespeexfile()
{
	int soundIndex;
	const char *filePath;

	if ( !stackGetParamInt(0, &soundIndex) )
	{
		stackError("gsc_utils_savespeexfile() requires a sound index (integer) as first argument");
		stackPushBool(qfalse);
		return;
	}
	if ( soundIndex < 1 || soundIndex > MAX_CUSTOMSOUNDS )
	{
		stackError("gsc_utils_savespeexfile() invalid sound index, valid range is 1-%d", MAX_CUSTOMSOUNDS);
		stackPushBool(qfalse);
		return;
	}
	if ( !stackGetParamString(1, &filePath) )
	{
		stackError("gsc_utils_savespeexfile() requires a file path (string) as second argument");
		stackPushBool(qfalse);
		return;
	}

	VoicePacket_t *packet;
	packet = &voiceDataStore[soundIndex - 1][0];
	if ( !packet->dataSize )
	{
		stackError("gsc_utils_savespeexfile() no sound data in the specified slot");
		stackPushBool(qfalse);
		return;
	}

	char ospath[MAX_OSPATH];
	FS_BuildOSPath(fs_homepath->current.string, filePath, "", ospath);
	ospath[strlen(ospath) - 1] = '\0';
	if ( fs_debug->current.integer )
		Com_Printf("gsc_utils_savespeexfile (fs_homepath) : %s\n", ospath);
	if ( strstr(ospath, "..") )
	{
		stackError("gsc_utils_savespeexfile() invalid file path");
		stackPushUndefined();
		return;
	}

	FILE *file = fopen(ospath, "wb");
	if ( !file )
	{
		stackError("gsc_utils_savespeexfile() could not open the specified file");
		stackPushBool(qfalse);
		return;
	}

	for ( int packetIndex = 0; packetIndex < MAX_STOREDVOICEPACKETS; packetIndex++ )
	{
		packet = &voiceDataStore[soundIndex - 1][packetIndex];
		if ( !packet->dataSize )
			break;
		fwrite(packet, sizeof(VoicePacket_t), 1, file);
	}
	fclose(file);

	stackPushBool(qtrue);
}

void gsc_utils_getsoundfileduration()
{
	const char *filePath;
	int overrideLimit;

	if ( !stackGetParamString(0, &filePath) )
	{
		stackError("gsc_utils_getsoundfileduration() requires a file path (string) as argument");
		stackPushUndefined();
		return;
	}
	if ( !stackGetParamInt(1, &overrideLimit) )
		overrideLimit = 0;

	char ospath[MAX_OSPATH];
	FS_BuildOSPath(fs_homepath->current.string, filePath, "", ospath);
	ospath[strlen(ospath) - 1] = '\0';
	if ( fs_debug->current.integer )
		Com_Printf("gsc_utils_getsoundfileduration (fs_homepath) : %s\n", ospath);
	if ( strstr(ospath, "..") )
	{
		stackError("gsc_utils_getsoundfileduration() invalid file path");
		stackPushUndefined();
		return;
	}

	FILE *file = fopen(ospath, "rb");
	if ( file != NULL )
	{
		if ( fseek(file, 0, SEEK_END) == 0 )
		{
			int size = ftell(file);
			if ( size != -1 )
			{
				fclose(file);
				size -= size % (sizeof(short) * MAX_VOICEFRAMESIZE);
				float duration = size / ((((1.0 / FRAMETIME) * 1000) * MAX_VOICEPACKETSPERFRAME) * (sizeof(short) * MAX_VOICEFRAMESIZE));
				if ( overrideLimit && duration > (60 * MAX_CUSTOMSOUNDDURATION) )
					duration = 60 * MAX_CUSTOMSOUNDDURATION;
				stackPushFloat(duration);
			}
			else
			{
				fclose(file);
				stackError("gsc_utils_getsoundfileduration() error at ftell");
				stackPushUndefined();
			}
		}
		else
		{
			fclose(file);
			stackError("gsc_utils_getsoundfileduration() error at fseek");
			stackPushUndefined();
		}
	}
	else
	{
		stackError("gsc_utils_getsoundfileduration() file could not be opened");
		stackPushUndefined();
	}
}

void gsc_player_playsoundfile(scr_entref_t ref)
{
	int id = ref.entnum;
	int args;
	qboolean error;
	int soundIndex;
	float offset;
	int source;

	args = Scr_GetNumParam();
	error = qfalse;
	switch ( args )
	{
		case 1:
			offset = 0.0;
			source = id;
			if ( !stackGetParams("i", &soundIndex) )
				error = qtrue;
			break;
		case 2:
			source = id;
			if ( !stackGetParams("if", &soundIndex, &offset) )
				error = qtrue;
			break;
		case 3:
			if ( !stackGetParams("ifi", &soundIndex, &offset, &source) )
				error = qtrue;
			break;
		default:
			stackError("gsc_player_playsoundfile() incorrect number of parameters");
			stackPushBool(qfalse);
			return;
	}

	if ( error )
	{
		stackError("gsc_player_playsoundfile() one or more arguments is undefined or has a wrong type");
		stackPushBool(qfalse);
		return;
	}
	if ( id >= MAX_CLIENTS || source >= MAX_CLIENTS )
	{
		stackError("gsc_player_playsoundfile() entity %i is not a player", id);
		stackPushBool(qfalse);
		return;
	}
	if ( svs.clients[source].state < CS_CONNECTED )
	{
		stackError("gsc_player_playsoundfile() entity %i is not connected", source);
		stackPushBool(qfalse);
		return;
	}
	if ( soundIndex < 1 || soundIndex > MAX_CUSTOMSOUNDS )
	{
		stackError("gsc_player_playsoundfile() invalid sound index, valid range is 1-%d", MAX_CUSTOMSOUNDS);
		stackPushBool(qfalse);
		return;
	}

	int packetOffset = (int)(offset * (((1.0 / FRAMETIME) * 1000) * MAX_VOICEPACKETSPERFRAME));
	if ( packetOffset >= MAX_STOREDVOICEPACKETS )
	{
		stackError("gsc_player_playsoundfile() too large offset for sound with index %d", soundIndex);
		stackPushBool(qfalse);
		return;
	}

	VoicePacket_t *voicePacket = &voiceDataStore[soundIndex - 1][packetOffset];
	if ( !*voicePacket->data )
	{
		stackError("gsc_player_playsoundfile() too large offset for sound with index %d", soundIndex);
		stackPushBool(qfalse);
		return;
	}

	if ( customPlayerState[id].currentSoundIndex )
		Scr_Notify(&g_entities[id], scr_const_sound_file_stop, 0);

	customPlayerState[id].pendingVoiceDataFrames = 0.0;
	customPlayerState[id].currentSoundTalker = source;
	customPlayerState[id].sentVoiceDataIndex = packetOffset;
	customPlayerState[id].currentSoundIndex = soundIndex;

	stackPushBool(qtrue);
}

void gsc_player_stopsoundfile(scr_entref_t ref)
{
	int id = ref.entnum;
	if ( id >= MAX_CLIENTS )
	{
		stackError("gsc_player_stopsoundfile() entity %i is not a player", id);
		stackPushUndefined();
		return;
	}

	if ( customPlayerState[id].currentSoundIndex )
	{
		customPlayerState[id].pendingVoiceDataFrames = 0.0;
		customPlayerState[id].currentSoundTalker = 0;
		customPlayerState[id].currentSoundIndex = 0;
		customPlayerState[id].sentVoiceDataIndex = 0;
		Scr_Notify(&g_entities[id], scr_const_sound_file_stop, 0);
		stackPushBool(qtrue);
	}
	else
	{
		stackPushBool(qfalse);
	}
}

void gsc_player_isplayingsoundfile(scr_entref_t ref)
{
	int id = ref.entnum;
	if ( id >= MAX_CLIENTS )
	{
		stackError("gsc_player_isplayingsoundfile() entity %i is not a player", id);
		stackPushUndefined();
		return;
	}
	stackPushInt(customPlayerState[id].currentSoundIndex);
}

void gsc_player_getremainingsoundfileduration(scr_entref_t ref)
{
	int id = ref.entnum;
	if ( id >= MAX_CLIENTS )
	{
		stackError("gsc_player_getremainingsoundfileduration() entity %i is not a player", id);
		stackPushUndefined();
		return;
	}

	if ( customPlayerState[id].currentSoundIndex )
	{
		int remainingPackets;
		int endIndex = customPlayerState[id].sentVoiceDataIndex;
		while ( endIndex < MAX_STOREDVOICEPACKETS && voiceDataStore[customPlayerState[id].currentSoundIndex - 1][endIndex].dataSize > 0 )
			endIndex++;
		remainingPackets = endIndex - customPlayerState[id].sentVoiceDataIndex;
		if ( remainingPackets <= 0 )
			stackPushFloat(0.0);
		else
			stackPushFloat(remainingPackets / (((1.0 / FRAMETIME) * 1000) * MAX_VOICEPACKETSPERFRAME));
	}
	else
	{
		stackPushFloat(0.0);
	}
}

#else
// Built without speex (NOSPEEX=1): no-op frame hooks so SV_SendClientMessages still links.
void SV_DrainSoundFileResults(void) {}
void SV_RunCustomSounds(void) {}
#endif // COMPILE_CUSTOM_VOICE

#endif
